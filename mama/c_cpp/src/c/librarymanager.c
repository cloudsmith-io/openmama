/*
 * OpenMAMA: The open middleware agnostic messaging API
 * Copyright (C) 2011 NYSE Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "librarymanager.h"

#include <assert.h>
#include <stdio.h>

#include <mama/mama.h>
#include <mamainternal.h>
#include <platform.h>
#include <wombat/directory.h>
#include <wombat/port.h>
#include <wombat/strutils.h>
#include <wombat/wtable.h>
#include <wombat/wInterlocked.h>

#include "bridge.h"
#include "payloadbridge.h"
#include "plugin.h"

#define LIB_PREFIX          "mama"
#define LIB_POSTFIX         "impl"
#define LIB_PATTERN         "*mama*impl"LIB_EXTENSION
#define PROPERTIES_PATTERN  "*mama.properties"
#define MAX_PROPERTY_LEN    300
#define DEFAULT_BUCKET_SIZE 64

/*
 * Private types
 */

typedef struct mamaLibraryTypeManagerInfoImpl_
{
    mamaLibraryType        mType;
    const char*            mTypeName;
    const char*            mTypeNameTitle;
    mamaLibraryTypeManager mImpl;
} mamaLibraryTypeManagerInfoImpl;
typedef mamaLibraryTypeManagerInfoImpl* mamaLibraryTypeManagerInfo;

typedef struct mamaLibraryManagerImpl_
{
    wInterlockedInt         mInit;
    wthread_static_mutex_t  mLock;
    mamaLibraryTypeManager  mManagers [MAX_LIBRARY_TYPE];
} mamaLibraryManagerImpl;

typedef struct GetLibrariesCbClosure_
{
    mamaLibrary*           mLibraries;
    mama_size_t            mIndex;
    mama_size_t            mMaxSize;
    mamaLibraryPredicateCb mPredicate;
} GetLibrariesCbClosure;

typedef struct IterateLibrariesCbClosure_
{
    mama_bool_t          mOk;
    mamaLibraryIterateCb mCb;
    void*                mClosure;
} IterateLibrariesCbClosure;

/*
 * Instantiation
 */

static mamaLibraryTypeManagerInfoImpl gManagers [] = {
    {MAMA_MIDDLEWARE_LIBRARY, "middleware", "Middleware", NULL},
    {MAMA_PAYLOAD_LIBRARY,    "payload",    "Payload",    NULL},
    {MAMA_PLUGIN_LIBRARY,     "plugin",     "Plugin",     NULL},
    {MAMA_UNKNOWN_LIBRARY,    NULL,         NULL,         NULL}
};

static mamaLibraryManagerImpl gImpl = {
    0,                           /* mInit           */
    WSTATIC_MUTEX_INITIALIZER    /* mLock           */
};

/*
 * Private declarations
 */

static mama_status
mamaLibraryManagerImpl_create (void);

static void
mamaLibraryManagerImpl_destroy (void);

static mama_status
mamaLibraryManagerImpl_getInstance (mamaLibraryManager* manager);

static const char*
mamaLibraryManagerImpl_getProperty (const char* library,
                                    const char* property);

static const char*
mamaLibraryManagerImpl_getPathProperty (const char*     library,
                                        mamaLibraryType libraryType);

static void
mamaLibraryManagerImpl_determineLibraryName (const char* filename,
                                             char*       buf,
                                             mama_size_t bufSize);

static void*
mamaLibraryManagerImpl_loadTypeManagerFunction (mamaLibraryTypeManager manager,
                                                const char* funcName);

static mama_status
mamaLibraryManagerImpl_createTypeManager (mamaLibraryTypeManagerInfo info);

static void
mamaLibraryManagerImpl_destroyTypeManager (mamaLibraryTypeManagerInfo info);

static mama_bool_t
mamaLibraryManagerImpl_cleanupLibrariesCb (mamaLibrary library,
                                           void*       closure);

static mama_bool_t 
mamaLibraryManagerImpl_dumpLibrariesCb (mamaLibrary library,
                                        void*       closure);

static mama_status
mamaLibraryManagerImpl_dumpTypeManager (mamaLibraryType type);

static mama_status
mamaLibraryManagerImpl_classifyLibraryType (const char*      libraryName,
                                            LIB_HANDLE       libraryHandle,
                                            mamaLibraryType* libraryType);
static void*
mamaLibraryManager_loadFunction (LIB_HANDLE  lib,
                                 const char* funcSpec,
                                 const char* funcName,
                                 const char* funcAltSpec,
                                 const char* funcAltName,
                                 void*       funcDefault);

static mama_status
mamaLibraryManagerImpl_createLibrary (mamaLibraryTypeManager manager,
                                      const char*            libraryName,
                                      LIB_HANDLE             libraryHandle,
                                      const char*            libraryPath,
                                      mamaLibrary*           library0);

static void
mamaLibraryManagerImpl_destroyLibrary (mamaLibrary library);

static mama_status
mamaLibraryManagerImpl_loadLibrary (const char*     libraryName,
                                    mamaLibraryType libraryType,
                                    const char*     path,
                                    mamaLibrary*    library,
                                    mama_bool_t     direct);

static mama_status
mamaLibraryManagerImpl_unloadLibrary (mamaLibrary library);

static mama_status
mamaLibraryManagerImpl_findAndLoad (const char*     path,
                                    mamaLibraryType libraryType);

/*
 * Private implementation
 */

static mama_status
mamaLibraryManagerImpl_create (void)
{
    mamaLibraryManager manager = &gImpl;
    mama_status        status  = MAMA_STATUS_OK;

    wthread_static_mutex_lock (&manager->mLock);
    if (0 == wInterlocked_read (&manager->mInit))
    {
        for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
        {
            status = mamaLibraryManagerImpl_createTypeManager (&gManagers[k]);

            if (MAMA_STATUS_OK != status)
            {
                break;
            }

            manager->mManagers[k] = gManagers[k].mImpl;
        }

        if (MAMA_STATUS_OK == status)
        {
            wInterlocked_increment (&manager->mInit);
        }
    }

    wthread_static_mutex_unlock (&manager->mLock);
    return status;
}

static void
mamaLibraryManagerImpl_destroy (void)
{
    mamaLibraryManager manager = &gImpl;
    wthread_static_mutex_lock (&manager->mLock);

    if (0 != wInterlocked_read (&gImpl.mInit))
    {
        for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
        {
            mamaLibraryManagerImpl_destroyTypeManager (&gManagers[k]);
            manager->mManagers[k] = NULL;
        }

        wInterlocked_decrement (&manager->mInit);
    }

    wthread_static_mutex_unlock (&manager->mLock);
}

static mama_status
mamaLibraryManagerImpl_getInstance (mamaLibraryManager* manager)
{
    if (0 == wInterlocked_read (&gImpl.mInit))
    {
        *manager = NULL;
        mama_status status = mamaLibraryManagerImpl_create ();

        if (MAMA_STATUS_OK != status)
        {
            return status;
        }

        /* FIXME: Should mama itself call this explicitly instead? */
        mamaLibraryManager_loadAll ();
    }

    *manager = &gImpl;
    return MAMA_STATUS_OK;
}

static const char*
mamaLibraryManagerImpl_getProperty (const char* library,
                                    const char* property)
{
    if (!property)
        return NULL;

    char buf [MAX_PROPERTY_LEN];
    int  res = 0;

    if (!(library && *library))
        library = "default";

    if (snprintf (buf, MAX_PROPERTY_LEN-1,
                 "mama.library.%s.%s", library, property) < 0)
    {
        return NULL;
    }

    const char* prop = mama_getProperty (buf);
    if (prop && *prop)
        return prop;

    return NULL;
}

static const char*
mamaLibraryManagerImpl_getPathProperty (const char*     library,
                                        mamaLibraryType libraryType)
{
    return mamaLibraryManager_getProperty (library, "path", libraryType);
}

static const char*
mamaLibraryManagerImpl_getDefaultPathProperty (void)
{
    return mamaLibraryManagerImpl_getPathProperty (NULL, MAMA_UNKNOWN_LIBRARY);
}

static void
mamaLibraryManagerImpl_determineLibraryName (const char* filename,
                                             char*       buf,
                                             mama_size_t bufSize)
{
    char*       token      = NULL;
    mama_size_t token_size = 0;

    if ((token = strstr (filename, LIB_PREFIX)))
    {
        /* File is most likely an OpenMAMA library */
        char* prefix  = token;
        char* postfix = strstr (filename, LIB_POSTFIX);

        if (postfix && postfix > prefix)
        {
            token += strlen (LIB_PREFIX);
            token_size = postfix - token;
        }
    }

    if (buf && bufSize > 0)
    {
        if (token && token_size > 0)
        {
            /* Copy token back to caller if determined */
            int n = token_size < bufSize ? token_size : bufSize;
            strncpy (buf, token, bufSize);
            buf[n] = '\0';
        }
        else
        {
            buf[0] = '\0';
        }
    }

    return;
}

static void*
mamaLibraryManagerImpl_loadTypeManagerFunction (mamaLibraryTypeManager manager,
                                                const char* funcName)
{
    /* Build target function spec */
    char funcSpec [MAX_LIBRARY_FUNCTION_NAME];
    if (snprintf (funcSpec, MAX_LIBRARY_FUNCTION_NAME-1,
                  "mama%sLibraryManager_%%s",
                  manager->mTypeNameTitle) < 0)
    {
        return NULL;
    }

    const char* funcAltSpec = "mamaDefaultLibraryManager_%s";

    return mamaLibraryManager_loadFunction (NULL /* handle */,
                                            funcSpec,
                                            funcName,
                                            funcAltSpec,
                                            NULL /* funcAltName */,
                                            NULL /* default */);
}

#define REGISTER_TYPE_MANAGER_FUNCTION(funcName)\
do {\
    void* func = mamaLibraryManagerImpl_loadTypeManagerFunction (manager, #funcName);\
    manager->mFuncs->funcName = *(mamaLibraryTypeManager_ ## funcName*) &func;\
    assert (manager->mFuncs->funcName);\
} while (0);

static mama_status
mamaLibraryManagerImpl_createTypeManager (mamaLibraryTypeManagerInfo info)
{
    mamaLibraryTypeManager manager = (mamaLibraryTypeManager)
        calloc (1, sizeof (mamaLibraryTypeManagerImpl));
    info->mImpl = manager;

    if (!manager)
    {
        mamaLibraryManagerImpl_destroyTypeManager (info);
        return MAMA_STATUS_NOMEM;
    }

    manager->mLibraries = wtable_create ("Libraries", DEFAULT_BUCKET_SIZE);
    if (!manager->mLibraries)
    {
        mamaLibraryManagerImpl_destroyTypeManager (info);
        return MAMA_STATUS_NOMEM;
    }

    manager->mLock = wlock_create ();
    if (!manager->mLock)
    {
        mamaLibraryManagerImpl_destroyTypeManager (info);
        return MAMA_STATUS_NOMEM;
    }

    manager->mFuncs = (mamaLibraryTypeManagerBridge)
        calloc (1, sizeof (mamaLibraryTypeManagerBridgeImpl));

    if (!manager->mFuncs)
    {
        mamaLibraryManagerImpl_destroyTypeManager (info);
        return MAMA_STATUS_NOMEM;
    }

    manager->mType          = info->mType;
    manager->mTypeName      = info->mTypeName;
    manager->mTypeNameTitle = info->mTypeNameTitle;

    wInterlocked_initialize (&manager->mNumLibraries);
    wInterlocked_set (0, &manager->mNumLibraries);

    mamaLibraryManager_createCallbackSignal (manager, &manager->mLoadSignal);
    mamaLibraryManager_createCallbackSignal (manager, &manager->mUnloadSignal);

    /* Register all available type manager functions */
    REGISTER_TYPE_MANAGER_FUNCTION (create);
    REGISTER_TYPE_MANAGER_FUNCTION (destroy);
    REGISTER_TYPE_MANAGER_FUNCTION (loadLibrary);
    REGISTER_TYPE_MANAGER_FUNCTION (unloadLibrary);
    REGISTER_TYPE_MANAGER_FUNCTION (dump);
    REGISTER_TYPE_MANAGER_FUNCTION (dumpLibrary);
    REGISTER_TYPE_MANAGER_FUNCTION (classifyLibraryType);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryProperty);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBoolProperty);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryIgnore);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryName);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryAuthor);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryUri);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryLicense);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryVersion);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryMamaVersion);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBridgeAuthor);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBridgeUri);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBridgeLicense);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBridgeVersion);
    REGISTER_TYPE_MANAGER_FUNCTION (getLibraryBridgeMamaVersion);

    /* Library type specific initialisation */
    mama_status status = manager->mFuncs->create (manager);
    if (MAMA_STATUS_OK != status)
        mamaLibraryManagerImpl_destroyTypeManager (info);

    return status;
}

static void
mamaLibraryManagerImpl_destroyTypeManager (mamaLibraryTypeManagerInfo info)
{
    mamaLibraryTypeManager manager = info->mImpl;

    if (!manager)
        return;

    if (0 != wInterlocked_read (&manager->mNumLibraries))
    {
        mamaLibraryManager_iterateLibraries (manager->mType,
            mamaLibraryManagerImpl_cleanupLibrariesCb, NULL);
    }

    manager->mFuncs->destroy ();

    mamaLibraryManager_destroyCallbackSignal (manager, manager->mLoadSignal);
    mamaLibraryManager_destroyCallbackSignal (manager, manager->mUnloadSignal);

    if (manager->mLock)
    {
        wlock_destroy (manager->mLock);
        manager->mLock = NULL;
    }

    if (manager->mLibraries)
    {
        wtable_free_all (manager->mLibraries);
        wtable_destroy (manager->mLibraries);
        manager->mLibraries = NULL;
    }

    wInterlocked_destroy (&manager->mNumLibraries);
    free (manager->mFuncs);
    free (manager);
    info->mImpl = NULL;
}

static mama_bool_t
mamaLibraryManagerImpl_cleanupLibrariesCb (mamaLibrary library,
                                           void*       closure)
{
    mamaLibraryManagerImpl_unloadLibrary (library);
    return 1;
}

static mama_status
mamaLibraryManagerImpl_classifyLibraryType (const char*      libraryName,
                                            LIB_HANDLE       libraryHandle,
                                            mamaLibraryType* libraryType0)
{
    *libraryType0 = MAMA_UNKNOWN_LIBRARY;

    /* Fast path, check if property has been specified */
    const char* prop =
        mamaLibraryManager_getProperty ("type", libraryName,
                                        MAMA_UNKNOWN_LIBRARY);

    if (prop)
    {
        mamaLibraryType libraryType =
            mamaLibraryManager_stringToLibraryType (prop);

        if (MAMA_UNKNOWN_LIBRARY != libraryType)
        {
            *libraryType0 = libraryType;
            return MAMA_STATUS_OK;
        }
    }

    mamaLibraryManager manager = NULL;
    mama_status status = mamaLibraryManagerImpl_getInstance (&manager);

    if (MAMA_STATUS_OK != status)
        return status;

    /* Attempt to determine library type using type managers */
    for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
    {
        mamaLibraryTypeManager typeManager = manager->mManagers[k];

        mamaLibraryType libraryType =
            typeManager->mFuncs->classifyLibraryType (libraryName,
                                                      libraryHandle);

        if (MAMA_UNKNOWN_LIBRARY != libraryType)
        {
            *libraryType0 = libraryType;
            return MAMA_STATUS_OK;
        }
    }

    return MAMA_STATUS_OK;
}

static mama_status
mamaLibraryManagerImpl_createLibrary (mamaLibraryTypeManager manager,
                                      const char*            libraryName,
                                      LIB_HANDLE             libraryHandle,
                                      const char*            libraryPath,
                                      mamaLibrary*           library0)
{
    *library0 = NULL;

    mamaLibrary library = (mamaLibrary)
        wtable_lookup (manager->mLibraries, libraryName);

    if (!library)
    {
        library = calloc (1, sizeof (mamaLibraryImpl));

        library->mName   = strdup (libraryName);
        library->mHandle = libraryHandle;
        library->mPath   = libraryPath ? strdup (libraryPath) : NULL;

        if (!wtable_insert (manager->mLibraries, libraryName, library))
            return MAMA_STATUS_NOMEM;

        library->mLock = wlock_create ();

        if (!library->mLock)
            return MAMA_STATUS_NOMEM;
    }

    *library0 = library;

    return MAMA_STATUS_OK;
}

static void*
mamaLibraryManager_loadFunction (LIB_HANDLE  lib,
                                 const char* funcSpec,
                                 const char* funcName,
                                 const char* funcAltSpec,
                                 const char* funcAltName,
                                 void*       funcDefault)
{
    if (!funcName)
        return NULL;

    char buf [MAX_LIBRARY_FUNCTION_NAME];

    /* Try primary function name */
    if (snprintf (buf, MAX_LIBRARY_FUNCTION_NAME-1, funcSpec, funcName) < 0)
        return NULL;

    void* func = loadLibFunc (lib, buf);
    if (func)
        return func;

    if (funcAltName || funcAltSpec)
    {
        /* Try alternative function name */

        if (!funcAltName)
            funcAltName = funcName;

        if (!funcAltSpec)
            funcAltSpec = funcSpec;

        if (snprintf (buf, MAX_LIBRARY_FUNCTION_NAME-1, funcAltSpec, funcAltName) < 0)
            return NULL;

        /* FIXME: Alternative function loaded from process space, but if we 
         * want to load from the same (or different) library, a new parameter
         * is required. */
        func = loadLibFunc (NULL, buf);
        if (func)
            return func;
    }

    /* Unresolved, assign default address */
    return funcDefault;
}

static void
mamaLibraryManagerImpl_destroyLibrary (mamaLibrary library)
{
    closeSharedLib (library->mHandle);
    wtable_remove (library->mManager->mLibraries, library->mName);
    wlock_destroy (library->mLock);
    free ((void*) library->mPath);
    free ((void*) library->mName);
    free (library);
}

static mama_status
mamaLibraryManagerImpl_loadLibrary (const char*     libraryName,
                                    mamaLibraryType libraryType,
                                    const char*     path,
                                    mamaLibrary*    library0,
                                    mama_bool_t     direct)
{
    *library0 = NULL;
    if (!libraryName)
        return MAMA_STATUS_NULL_ARG;

    char buf [FILENAME_MAX];
    snprintf (buf, FILENAME_MAX-1, "%s%s%s",
              LIB_PREFIX, libraryName, LIB_POSTFIX);

    LIB_HANDLE libraryHandle = openSharedLib (buf, path);
    if (!libraryHandle)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManager_loadLibrary(): "
                  "Could not open %s library %s [%s] (path: %s)",
                  mamaLibraryManager_libraryTypeToString (libraryType),
                  libraryName, getLibError (), path);
        return MAMA_STATUS_NOT_FOUND;
    }

    /* Attempt to classify the library if the type isn't known */
    if (MAMA_UNKNOWN_LIBRARY == libraryType)
    {
        mama_status status =
            mamaLibraryManagerImpl_classifyLibraryType (libraryName,
                                                        libraryHandle,
                                                        &libraryType);

        if (MAMA_STATUS_OK != status)
            return status;

        if (MAMA_UNKNOWN_LIBRARY == libraryType)
        {
            /* Still couldn't determine the library type */
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaLibraryManager_loadLibrary(): "
                      "Opened %s library %s but could not determine type (path: %s)",
                      mamaLibraryManager_libraryTypeToString (libraryType),
                      libraryName, path);
            return MAMA_STATUS_SYSTEM_ERROR;
        }
    }

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return status;

    wlock_lock (manager->mLock);

    /* Allocate memory for the library and store in type manager */
    mamaLibrary library = NULL;
    status = mamaLibraryManagerImpl_createLibrary (manager,
                                                   libraryName,
                                                   libraryHandle,
                                                   path,
                                                   &library);

    mama_bool_t ignore = 0;

    if (MAMA_STATUS_OK == status)
    {
        /* Library was already loaded */
        if (library->mManager)
        {
            *library0 = library;
            closeSharedLib (libraryHandle);
            wlock_unlock (manager->mLock);
            return status;
        }

        library->mManager  = manager;
        library->mType     = manager->mType;
        library->mTypeName = manager->mTypeName;

        /* Give libraries the chance to declare themselves as ignorable */
        if (!direct && manager->mFuncs->getLibraryIgnore (library))
        {
            mama_log (MAMA_LOG_LEVEL_FINEST,
                      "mamaLibraryManager_loadLibrary(): "
                      "Skipped ignored %s library %s during load "
                      "all (path: %s)",
                      mamaLibraryManager_libraryTypeToString (libraryType),
                      libraryName, path);

            /* FIXME: Need a better status here, MAMA_STATUS_IGNORED? */
            status = MAMA_STATUS_NO_BRIDGE_IMPL;
        }
        else
        {
            /* Inform type manager of library loading */
            status = manager->mFuncs->loadLibrary (library);
        }
    }

    if (MAMA_STATUS_OK == status)
    {
        wInterlocked_increment (&manager->mNumLibraries);
        *library0 = library;

        mama_log (MAMA_LOG_LEVEL_FINE,
                  "mamaLibraryManager_loadLibrary(): "
                  "Successfully loaded %s library %s (path: %s)",
                  mamaLibraryManager_libraryTypeToString (libraryType),
                  libraryName, path);

        /* Signal to interested parties that the library was loaded */
        mamaLibraryManager_raiseCallbackSignal (library, manager->mLoadSignal);
    }
    else
    {
        mamaLibraryManagerImpl_destroyLibrary (library);
    }

    wlock_unlock (manager->mLock);
    return status;
}

static mama_status
mamaLibraryManagerImpl_unloadLibrary (mamaLibrary library)
{
    if (!library)
        return MAMA_STATUS_NULL_ARG;

    mamaLibraryTypeManager manager = library->mManager;
    wlock_lock (manager->mLock);

    /* Signal to interested parties that the library was unloaded */
    mamaLibraryManager_raiseCallbackSignal (library, manager->mLoadSignal);

    wInterlocked_decrement (&manager->mNumLibraries);
    manager->mFuncs->unloadLibrary (library);
    mamaLibraryManagerImpl_destroyLibrary (library);

    wlock_unlock (manager->mLock);
    return MAMA_STATUS_OK;
}

static
void mamaLibraryManagerImpl_loadLibrariesCb (const char* path,
                                             const char* filename,
                                             void*       closure)
{
    char name [FILENAME_MAX];

    mamaLibraryManagerImpl_determineLibraryName (filename, name,
                                                 FILENAME_MAX-1);

    mamaLibraryType libraryType = *(mamaLibraryType*) closure;
    mamaLibrary     library     = NULL;

    mamaLibraryManagerImpl_loadLibrary (name, libraryType,
                                        path, &library, 0);
}

static
void mamaLibraryManagerImpl_loadPropertiesCb (const char* path,
                                              const char* filename,
                                              void*       closure)
{
    mama_setPropertiesFromFile (path, filename);
}

static mama_status
mamaLibraryManagerImpl_findAndLoad (const char*     path,
                                    mamaLibraryType libraryType)
{
    /* First load any properties files - it is important this is
     * done first as these may contain configuration relevant to the
     * bridge loading such as whether a certain functionality (e.g. IO)
     * should use a default version provided by MAMA or the bridge version
     */
    if (0 != enumerateDirectory (path, PROPERTIES_PATTERN,
                                 mamaLibraryManagerImpl_loadPropertiesCb,
                                 (void*) &libraryType))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManagerImpl_findAndLoad(): "
                  "Error looking for %s libraries files in path [%s]",
                  mamaLibraryManager_libraryTypeToString (libraryType),
                  path);
        return MAMA_STATUS_SYSTEM_ERROR;
    }

    /* Now load the library files */
    if (0 != enumerateDirectory (path, LIB_PATTERN,
                                 mamaLibraryManagerImpl_loadLibrariesCb,
                                 (void*) &libraryType))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManagerImpl_findAndLoad(): "
                  "Error looking for %s libraries files in path [%s]",
                  mamaLibraryManager_libraryTypeToString (libraryType),
                  path);
        return MAMA_STATUS_SYSTEM_ERROR;
    }

    return MAMA_STATUS_OK;
}

static void
mamaLibraryManagerImpl_getLibrariesCb (wtable_t    table,
                                       void*       data,
                                       const char* key,
                                       void*       closure)
{
    GetLibrariesCbClosure* cl = (GetLibrariesCbClosure*) closure;

    if (cl->mIndex < cl->mMaxSize)
    {
        mamaLibrary library = (mamaLibrary) data;

        if (!cl->mPredicate || cl->mPredicate (library))
        {
            cl->mLibraries[cl->mIndex++] = library;
        }
    }
}

static void
mamaLibraryManagerImpl_iterateLibrariesCb (wtable_t    table,
                                           void*       data,
                                           const char* key,
                                           void*       closure)
{
    IterateLibrariesCbClosure* cl = (IterateLibrariesCbClosure*) closure;
    mamaLibrary library = (mamaLibrary) data;

    /* FIXME: No current method to stop iteration in wtable? */
    if (cl->mOk)
        cl->mOk = cl->mCb (library, cl->mClosure);
}

mama_status
mamaLibraryManagerImpl_dumpTypeManager (mamaLibraryType libraryType)
{
    mamaLibraryTypeManager manager = NULL;
    mama_status status             = 
            mamaLibraryManager_getTypeManager(libraryType, &manager);
 
    if (MAMA_STATUS_OK != status)
        return status;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "%s Libraries: No. Libraries[%d] ",
              manager->mTypeName, wInterlocked_read(&manager->mNumLibraries));
    
    manager->mFuncs->dump(manager);
}

/*
 * Internal implementation (accessible from type managers)
 */

void
mamaLibraryManager_destroy (void)
{
    mamaLibraryManagerImpl_destroy ();
}

mama_status
mamaLibraryManager_loadAll (void)
{
    const char* default_path = mamaLibraryManagerImpl_getDefaultPathProperty ();
    mama_size_t loaded       = 0;

    mamaLibraryManager manager = NULL;
    mama_status status = mamaLibraryManagerImpl_getInstance (&manager);

    if (MAMA_STATUS_OK != status)
        return status;

    for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
    {
        mamaLibraryTypeManager typeManager = manager->mManagers[k];

        const char* path =
            mamaLibraryManagerImpl_getPathProperty (
                typeManager->mTypeName, MAMA_UNKNOWN_LIBRARY);

        /* Path not set for specific library type */
        if (!path)
            continue;

        /* Ignore paths equivalent to the default_path */
        if (default_path && 0 == strcmp (path, default_path))
            continue;

        status = mamaLibraryManagerImpl_findAndLoad (path, typeManager->mType);
        if (MAMA_STATUS_OK != status)
            continue;

        /* Successfully loaded libraries from path */
        loaded += 1;
    }

    if (MAX_LIBRARY_TYPE != loaded)
    {
        /* Not all library type paths were specified or loaded, so attempt a
         * general load from the top-level path. */
        if (!default_path)
        {
            /* FIXME: Should default path default to system LD_LIBRARY_PATH,
             * or whatever the platform equivalent is? */
            mama_log (MAMA_LOG_LEVEL_WARN,
                      "mamaLibraryManager_loadAll():  "
                      "Cannot load all libraries without a path being "
                      "specified (e.g. mama.library.default.path).");
            return MAMA_STATUS_OK;
        }

        status = mamaLibraryManagerImpl_findAndLoad (default_path,
                                                     MAMA_UNKNOWN_LIBRARY);
    }

    return status;
}

mama_status
mamaLibraryManager_loadLibrary (const char*     libraryName,
                                mamaLibraryType libraryType,
                                const char*     path,
                                mamaLibrary*    library)
{
    *library = NULL;
    if (!libraryName)
        return MAMA_STATUS_NULL_ARG;

    if (0 == strcmp ("default", libraryName) ||
        MAMA_UNKNOWN_LIBRARY !=
            mamaLibraryManager_stringToLibraryType (libraryName))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManager_loadLibrary(): "
                  "Failed to load %s library %s because it duplicates "
                  "a reserved name.",
                  mamaLibraryManager_libraryTypeToString (libraryType),
                  libraryName);
        return MAMA_STATUS_INVALID_ARG;
    }

    /* Attempt to retrieve path for specific library (if overridden). */
    if (!path)
    {
        path = mamaLibraryManagerImpl_getPathProperty (libraryName,
                                                       libraryType);
    }

    return mamaLibraryManagerImpl_loadLibrary (libraryName, libraryType,
                                               path, library, 1 /* direct */);
}

mama_status
mamaLibraryManager_unloadLibrary (const char*     libraryName,
                                  mamaLibraryType libraryType)
{
    if (!libraryName)
        return MAMA_STATUS_NULL_ARG;

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return status;

    wlock_lock (manager->mLock);
    mamaLibrary library = (mamaLibrary)
        wtable_lookup (manager->mLibraries, libraryName);

    if (library)
        status = mamaLibraryManagerImpl_unloadLibrary (library);
    else
        status = MAMA_STATUS_NOT_FOUND;

    wlock_unlock (manager->mLock);
    return MAMA_STATUS_OK;
}

void* 
mamaLibraryManager_loadLibraryFunction (const char* libraryName,
                                        LIB_HANDLE  libraryHandle,
                                        const char* funcName)
{
    char funcSpec [MAX_LIBRARY_FUNCTION_NAME];
    if (snprintf (funcSpec, MAX_LIBRARY_FUNCTION_NAME-1, "%s%%s",
                  libraryName) < 0)
    {
        return NULL;
    }

    const char* funcAltSpec = "default%s";

    return mamaLibraryManager_loadFunction (libraryHandle,
                                            funcSpec,
                                            funcName,
                                            funcAltSpec,
                                            NULL, /* funcAltName */
                                            NULL  /* default */);
}

mama_status
mamaLibraryManager_getTypeManager (mamaLibraryType         type,
                                   mamaLibraryTypeManager* typeManager)
{
    if (!typeManager)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_UNKNOWN_LIBRARY == type)
        return MAMA_STATUS_INVALID_ARG;

    mamaLibraryManager manager = NULL;
    mama_status status = mamaLibraryManagerImpl_getInstance (&manager);

    if (MAMA_STATUS_OK != status)
        return status;

    *typeManager = manager->mManagers[type];
    return MAMA_STATUS_OK;
}

mama_status
mamaLibraryManager_getLibrary (const char*     libraryName,
                               mamaLibraryType libraryType,
                               mamaLibrary*    library)
{
    if (!(libraryName && *libraryName) || !library)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_UNKNOWN_LIBRARY == libraryType)
        return MAMA_STATUS_INVALID_ARG;

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return status;

    if (0 == wInterlocked_read (&manager->mNumLibraries))
        return MAMA_STATUS_NOT_FOUND;

    wlock_lock (manager->mLock);
    *library = (mamaLibrary)
        wtable_lookup (manager->mLibraries, libraryName);

    if (!*library)
    {
        /* Library hasn't been loaded yet, attempt to load it */
        status = mamaLibraryManager_loadLibrary (libraryName, libraryType,
                                                 NULL, library);
    }

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_getLibraries (mamaLibrary*           libraries,
                                 mama_size_t*           size,
                                 mamaLibraryType        libraryType,
                                 mamaLibraryPredicateCb predicate)
{
    if (!libraries || !size)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_UNKNOWN_LIBRARY == libraryType || 0 == *size)
        return MAMA_STATUS_INVALID_ARG;

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return status;

    wlock_lock (manager->mLock);

    GetLibrariesCbClosure closure;
    closure.mLibraries = libraries;
    closure.mIndex     = 0;
    closure.mMaxSize   = *size;
    closure.mPredicate = predicate;

    wtable_for_each (manager->mLibraries,
                     mamaLibraryManagerImpl_getLibrariesCb,
                     (void*) &closure);

    *size = closure.mIndex;

    if (0 == *size)
        *libraries = NULL;

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_iterateLibraries (mamaLibraryType      libraryType,
                                     mamaLibraryIterateCb cb,
                                     void*                closure)
{
    if (!cb)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_UNKNOWN_LIBRARY == libraryType)
        return MAMA_STATUS_INVALID_ARG;

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return status;

    wlock_lock (manager->mLock);
    IterateLibrariesCbClosure cl;
    cl.mOk      = 1;
    cl.mCb      = cb;
    cl.mClosure = closure;

    wtable_for_each (manager->mLibraries,
                     mamaLibraryManagerImpl_iterateLibrariesCb,
                     (void*) &cl);

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_createCallbackSignal (mamaLibraryTypeManager manager,
                                         mama_size_t*           signalId)
{
    if (!manager || !signalId)
        return MAMA_STATUS_NULL_ARG;

    *signalId = 0;

    wlock_lock (manager->mLock);
    mamaLibraryCallbackRegister* callbacks = &manager->mCallbacks;

    if (MAX_LIBRARY_SIGNALS == callbacks->mNextId)
    {
        wlock_unlock (manager->mLock);
        return MAMA_STATUS_NOMEM;  /**< FIXME: Not quite right */
    }

    *signalId = callbacks->mNextId;
    callbacks->mSize += 1;
    callbacks->mNextId += 1;

    /* Find next available slot */
    for (; callbacks->mNextId < MAX_LIBRARY_SIGNALS; ++callbacks->mNextId)
        if (0 == callbacks->mSignals[callbacks->mNextId].mSize)
            break;

    wlock_unlock (manager->mLock);
    return MAMA_STATUS_OK;
}

mama_status
mamaLibraryManager_destroyCallbackSignal (mamaLibraryTypeManager manager,
                                          mama_size_t            signalId)
{
    if (!manager)
        return MAMA_STATUS_NULL_ARG;

    if (signalId > MAX_LIBRARY_SIGNALS)
        return MAMA_STATUS_INVALID_ARG;

    wlock_lock (manager->mLock);
    mamaLibraryCallbackRegister* callbacks = &manager->mCallbacks;
    mamaLibraryCallbackSignal* signal = &callbacks->mSignals[signalId];

    mama_status status = MAMA_STATUS_NOT_FOUND;
    if (signalId < callbacks->mNextId)
    {
        callbacks->mNextId = signalId;
        callbacks->mSize -= 1;

        if (0 != signal->mSize)
            memset (signal, 0, sizeof (mamaLibraryCallbackSignal));

        status = MAMA_STATUS_OK;
    }

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_createCallbackSlot (mamaLibraryTypeManager manager,
                                       mama_size_t            signalId,
                                       mamaLibraryCb          cb,
                                       void*                  closure)
{
    if (!manager || !cb)
        return MAMA_STATUS_NULL_ARG;

    if (signalId >= MAX_LIBRARY_SIGNALS)
        return MAMA_STATUS_INVALID_ARG;

    wlock_lock (manager->mLock);
    mamaLibraryCallbackRegister* callbacks = &manager->mCallbacks;
    mamaLibraryCallbackSignal* signal = &callbacks->mSignals[signalId];

    if (MAX_LIBRARY_SIGNAL_SLOTS == signal->mNextId)
    {
        wlock_unlock (manager->mLock);
        return MAMA_STATUS_NOMEM;  /**< FIXME: Not quite right */
    }

    /* Ensure callback isn't already registered */
    mamaLibraryManager_destroyCallbackSlot (manager, signalId, cb);

    mamaLibraryCallbackSlot* slot = &signal->mSlots[signal->mNextId];

    slot->mCb      = cb;
    slot->mClosure = closure;
    signal->mSize += 1;
    signal->mNextId += 1;

    /* Find next available slot */
    for (; signal->mNextId < MAX_LIBRARY_SIGNAL_SLOTS; ++signal->mNextId)
        if (!signal->mSlots[signal->mNextId].mCb)
            break;

    wlock_unlock (manager->mLock);
    return MAMA_STATUS_OK;
}

mama_status
mamaLibraryManager_destroyCallbackSlot (mamaLibraryTypeManager manager,
                                        mama_size_t            signalId,
                                        mamaLibraryCb          cb)
{
    if (!manager || !cb)
        return MAMA_STATUS_NULL_ARG;

    if (signalId >= MAX_LIBRARY_SIGNALS)
        return MAMA_STATUS_INVALID_ARG;

    wlock_lock (manager->mLock);
    mamaLibraryCallbackRegister* callbacks = &manager->mCallbacks;
    mamaLibraryCallbackSignal* signal = &callbacks->mSignals[signalId];

    mama_status status = MAMA_STATUS_NOT_FOUND;
    mama_size_t slotId = 0;

    for (; slotId < MAX_LIBRARY_SIGNAL_SLOTS; ++slotId)
    {
        mamaLibraryCallbackSlot* slot = &signal->mSlots[slotId];

        if (slot->mCb == cb)
        {
            signal->mSize -= 1;
            signal->mNextId = slotId;
            status = MAMA_STATUS_OK;
            break;
        }
    } 

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_raiseCallbackSignal (mamaLibrary library,
                                        mama_size_t signalId)
{
    if (!library)
        return MAMA_STATUS_NULL_ARG;

    if (signalId > MAX_LIBRARY_SIGNALS)
        return MAMA_STATUS_INVALID_ARG;

    mamaLibraryTypeManager manager = library->mManager;

    wlock_lock (manager->mLock);
    mamaLibraryCallbackRegister* callbacks = &manager->mCallbacks;

    mama_status status = MAMA_STATUS_NOT_FOUND;
    if (signalId < callbacks->mNextId)
    {
        mamaLibraryCallbackSignal* signal = &callbacks->mSignals[signalId];

        mama_bool_t ok = 1;
        for (mama_size_t k = 0; k < signal->mSize && 0 != ok; ++k)
        {
            mamaLibraryCallbackSlot* slot = &signal->mSlots[k];
            ok = slot->mCb (library, slot->mClosure);
        }

        status = MAMA_STATUS_OK;
    }

    wlock_unlock (manager->mLock);
    return status;
}

mama_status
mamaLibraryManager_registerLoadCallback (mamaLibraryTypeManager manager,
                                         mamaLibraryCb          cb,
                                         void*                  closure)
{
    return mamaLibraryManager_createCallbackSlot (manager, 
                                                  manager->mLoadSignal,
                                                  cb, closure);
}

mama_status
mamaLibraryManager_registerUnloadCallback (mamaLibraryTypeManager manager,
                                           mamaLibraryCb          cb,
                                           void*                  closure)
{
    return mamaLibraryManager_createCallbackSlot (manager, 
                                                  manager->mUnloadSignal,
                                                  cb, closure);
}

void
mamaLibraryManager_dump ()
{
    mama_log (MAMA_LOG_LEVEL_NORMAL, 
        "mamaLibraryManager_dump(): Loaded Libraries:");

    for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
    {
        mamaLibraryType libraryType    = gManagers[k].mType;
        mama_status status             = 
            mamaLibraryManagerImpl_dumpTypeManager (libraryType);
 
        if (MAMA_STATUS_OK != status)
            continue;

        mamaLibraryManager_iterateLibraries (libraryType, 
                                             mamaLibraryManagerImpl_dumpLibrariesCb,
                                             NULL);               
    }
}

mama_bool_t
mamaLibraryManagerImpl_dumpLibrariesCb (mamaLibrary library,
                                        void*       closure)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL, "%s:\n\tName [%s] Path[%s]", 
        library->mName, library->mName, library->mPath);    

    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (library->mType, &manager);

    if (MAMA_STATUS_OK != status)
        return 0;

    manager->mFuncs->dumpLibrary (library);

    static const char* properties[] = {"ignore", "name" , "description",
                                       "author", "uri", "license", "version",
                                       "mama_version", "bridge_author", 
                                       "bridge_uri", "bridge_license", 
                                       "bridge_version", "bridge_mama_version"};

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Bridge Properties: ");

    for (mama_size_t i = 0; 
        i < sizeof(properties)/sizeof(properties[0]); ++i)
    {
        const char* prop = 
            manager->mFuncs->getLibraryProperty (library,
                                                 properties[i]);
    
        if (prop)
        {
            mama_log (MAMA_LOG_LEVEL_NORMAL, "Property [%s] Value [%s]", 
                properties[i], prop);
        }
    }
    return 1;
}

mama_status 
mamaLibraryManager_setProperty (const char* libraryName,
                                const char* propertyName,
                                const char* value)
{
    char buf [MAX_PROPERTY_LEN];
    int  res = 0;

    if (!(libraryName && *libraryName))
        libraryName = "default";

    if (snprintf (buf, MAX_PROPERTY_LEN-1,
                 "mama.library.%s.%s", libraryName, propertyName) < 0)
    {
        return MAMA_STATUS_NOMEM;
    }

    return mama_setProperty(buf, value); 
}

const char*
mamaLibraryManager_getProperty (const char*     library,
                                const char*     property,
                                mamaLibraryType type)
{
    if (library && *library)
    {
        /* Lookup property based on specified name */
        const char* prop =
            mamaLibraryManagerImpl_getProperty (library, property);

        if (prop)
            return prop;
    }

    if (MAMA_UNKNOWN_LIBRARY != type)
    {
        /* Look up property based on type, if known */
        library = mamaLibraryManager_libraryTypeToString (type);
        return mamaLibraryManager_getProperty (library, property,
                                               MAMA_UNKNOWN_LIBRARY);
    }

    /* Still not found, so look up default (if any) */
    return mamaLibraryManagerImpl_getProperty (NULL, property);
}

mama_bool_t
mamaLibraryManager_getBoolProperty (const char*     library,
                                    const char*     property,
                                    mamaLibraryType type)
{
    const char* prop =
        mamaLibraryManager_getProperty (library, property, type);

    if (!prop)
        return 0;

    return strtobool (prop);
}

const char*
mamaLibraryManager_getLibraryProperty (mamaLibrary library,
                                       const char* property)
{
    return mamaLibraryManager_getProperty (library->mName,
                                           property,
                                           library->mType);
}

mama_bool_t
mamaLibraryManager_getLibraryBoolProperty (mamaLibrary library,
                                           const char* property)
{
    return mamaLibraryManager_getBoolProperty (library->mName,
                                               property,
                                               library->mType);
}

mamaLibraryType
mamaLibraryManager_stringToLibraryType (const char* name)
{
    if (!name)
        return MAMA_UNKNOWN_LIBRARY;

    mamaLibraryManager manager = NULL;
    mama_status status = mamaLibraryManagerImpl_getInstance (&manager);

    if (MAMA_STATUS_OK != status)
        return MAMA_UNKNOWN_LIBRARY;

    for (mama_size_t k = 0; k < MAX_LIBRARY_TYPE; ++k)
    {
        mamaLibraryTypeManager typeManager = manager->mManagers[k];

        if (0 == strcmp (name, typeManager->mTypeName))
            return typeManager->mType;
    }

    return MAMA_UNKNOWN_LIBRARY;
}

const char*
mamaLibraryManager_libraryTypeToString (const mamaLibraryType libraryType)
{
    mamaLibraryTypeManager manager = NULL;
    mama_status status =
        mamaLibraryManager_getTypeManager (libraryType, &manager);

    if (MAMA_STATUS_OK != status)
        return "unknown";

    return manager->mTypeName;
}

