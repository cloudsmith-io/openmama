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

#include "entitlementmanager.h"
#include "mama/entitlementmanager.h"

#include <mama/mama.h>
#include <assert.h>
#include <platform.h>
#include <wombat/wInterlocked.h>

#include "entitlement.h"
#include "librarymanager.h"
#include "defaultentitlementbridge.h"

static mamaEntitlementBridgeImpl defaultBridge = {
    defaultEntitlement_setup,    
    defaultEntitlement_tearDown,
    defaultEntitlement_createSubscription,
    defaultEntitlement_deleteSubscription,
    defaultEntitlement_setSubscriptionType,
    defaultEntitlement_checkEntitledWithSubject,
    defaultEntitlement_checkEntitledWithCode, 
    NULL
};

/*
 * Private types
 */

typedef struct mamaEntitlementLibraryManagerImpl* mamaEntitlementLibraryManager;

typedef struct mamaEntitlementLibraryImpl_
{
    mamaLibrary                     mParent;
    mamaEntitlementBridge           mBridge;
    wInterlockedInt                 mActiveCount;
    char                            mEntitlementId;
    mamaEntitlementLibraryManager   mManager;
} mamaEntitlementLibraryImpl;

typedef struct mamaEntitlementLibraryManagerImpl
{
    mamaLibraryTypeManager mParent;
    mamaEntitlementLibrary mEntitlements [MAMA_MAX_LIBRARIES];
} mamaEntitlementLibraryManagerImpl;

/*
 * Private declarations
 */
static mama_status
mamaEntitlementLibraryManagerImpl_getInstance (
        mamaEntitlementLibraryManager* entManager);

static mama_status
mamaEntitlementLibraryManagerImpl_getLibraries (mamaEntitlementLibrary* libraries,
                                                mama_size_t*            size,
                                                mamaLibraryPredicateCb  predicate);
static Entitlement_load
mamaEntitlementLibraryManagerImpl_getLoad (const char* libraryName,
                                           LIB_HANDLE  libraryHandle);
static Entitlement_unload
mamaEntitlementLibraryManagerImpl_getUnload (const char* libraryName,
                                             LIB_HANDLE  libraryHandle);
static Entitlement_load
mamaEntitlementLibraryManagerImpl_getLibraryLoad (mamaLibrary library);

static Entitlement_unload
mamaEntitlementLibraryManagerImpl_getLibraryUnload (mamaLibrary library);

static mama_status
mamaEntitlementLibraryManagerImpl_createBridge (mamaEntitlementBridge* bridge0);

static mama_status
mamaEntitlementLibraryManagerImpl_loadBridge (mamaEntitlementLibrary plLibrary);

static void
mamaEntitlementLibraryManagerImpl_destroyBridge (mamaEntitlementBridge bridge);

static mama_status
mamaEntitlementLibraryManagerImpl_activateLibrary (mamaEntitlementLibrary entLibrary);

static void
mamaEntitlementLibraryManagerImpl_deactivateLibrary (mamaEntitlementLibrary library);

static mama_status
mamaEntitlementLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaEntitlementLibrary* plLibrary0);

static void
mamaEntitlementLibraryManagerImpl_destroyLibrary (mamaEntitlementLibrary plLibrary);

static mama_status
mamaEntitlementLibraryManagerImpl_getEntitlementId (
        mamaEntitlementLibrary library, char* entitlementId);

static mama_status
mamaEntitlementLibraryManagerImpl_getDefaultBridge (
        mamaEntitlementBridge* bridge);

/*
 * Private implementation
 */

static mama_status
mamaEntitlementLibraryManagerImpl_getInstance (
        mamaEntitlementLibraryManager* entManager)
{
     if (!entManager)
         return MAMA_STATUS_NULL_ARG;

     mamaLibraryTypeManager manager = NULL;
     mama_status status =
         mamaLibraryManager_getTypeManager (MAMA_ENTITLEMENT_LIBRARY, &manager);

     if (MAMA_STATUS_OK == status)
         *entManager = (mamaEntitlementLibraryManager) manager->mClosure;
     else
         *entManager = NULL;

     return status;
}

static Entitlement_load
mamaEntitlementLibraryManagerImpl_getLoad (const char* libraryName,
                                           LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Entitlement_load");
    return *(Entitlement_load*)&func;
}

static Entitlement_unload
mamaEntitlementLibraryManagerImpl_getUnload (const char* libraryName,
                                             LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Entitlement_unload");
    return *(Entitlement_unload*)&func;
}

static Entitlement_load
mamaEntitlementLibraryManagerImpl_getLibraryLoad (mamaLibrary library)
{
    return (Entitlement_load)
        mamaEntitlementLibraryManagerImpl_getLoad (library->mName,
                                               library->mHandle);
}

static Entitlement_unload
mamaEntitlementLibraryManagerImpl_getLibraryUnload (mamaLibrary library)
{
    return (Entitlement_unload)
        mamaEntitlementLibraryManagerImpl_getUnload (library->mName,
                                                     library->mHandle);
}

static mama_status
mamaEntitlementLibraryManagerImpl_getLibraries (mamaEntitlementLibrary* entLibraries,
                                                mama_size_t*            size,
                                                mamaLibraryPredicateCb  predicate)
{
    if (!entLibraries || !size)
        return MAMA_STATUS_NULL_ARG;

    mamaLibrary libraries [MAMA_MAX_LIBRARIES];
    mama_size_t librariesSize = MAMA_MAX_LIBRARIES;

    mama_status status =
        mamaLibraryManager_getLibraries (libraries,
                                         &librariesSize,
                                         MAMA_ENTITLEMENT_LIBRARY,
                                         predicate);

    for (mama_size_t k = 0; k < librariesSize && k < *size; ++k)
        entLibraries[k] = (mamaEntitlementLibrary)libraries[k]->mClosure;

    if (librariesSize < *size)
        *size = librariesSize;

    return status;
}

#define REGISTER_ENTITLEMENT_FUNCTION(funcName, FuncName)\
do {\
    if (MAMA_STATUS_OK == status)\
    {\
        void* func = \
            mamaLibraryManager_loadLibraryFunction\
                    (library->mName, library->mHandle, #funcName);\
        entBridge->FuncName = *(funcName*) &func;\
        if (!entBridge->FuncName)\
        {\
            mama_log (MAMA_LOG_LEVEL_ERROR,\
                      "mamaLibraryEntitlementManager_loadLibrary(): "\
                      "Could not load %s library %s because "\
                      "required function [%s] is missing in bridge.",\
                      library->mTypeName, library->mName,\
                      #funcName);\
            status = MAMA_STATUS_NO_BRIDGE_IMPL;\
        }\
    }\
} while (0);

mama_status
mamaEntitlementLibraryManagerImpl_loadBridge (mamaEntitlementLibrary entLibrary)
{
    mama_status status = 
        mamaEntitlementLibraryManagerImpl_createBridge (&entLibrary->mBridge);

    if (MAMA_STATUS_OK != status)    
        return status;

    mamaLibrary           library    = entLibrary->mParent;
    mamaEntitlementBridge entBridge  = entLibrary->mBridge;
    entBridge->mLibrary              = entLibrary;

    /* Once the bridge has been successfully loaded, and the initialization
     * function called, we register each of the required bridge functions.
     */

    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_setup,
                                   entitlementSetup);

    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_tearDown,
                                   entitlementTearDown);

    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_createSubscription,
                                   entitlementCreateSubscription);
    
    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_deleteSubscription,
                                   entitlementDeleteSubscription);
    
    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_setSubscriptionType,
                                   entitlementSetSubscriptionType);
    
    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_checkEntitledWithSubject,
                                   entitlementCheckEntitledWithSubject);
    
    REGISTER_ENTITLEMENT_FUNCTION (Entitlement_checkEntitledWithCode,
                                   entitlementCheckEntitledWithCode);

    if (MAMA_STATUS_OK != status)
    {
        free (entBridge);
        return status;
    }
    return MAMA_STATUS_OK;
}

static mama_status
mamaEntitlementLibraryManagerImpl_createBridge (mamaEntitlementBridge* bridge0)
{
    assert (bridge0);
    *bridge0 = NULL;

    mamaEntitlementBridge bridge = (mamaEntitlementBridge)
        calloc (1, sizeof (mamaEntitlementBridgeImpl));

    if (!bridge)
        return MAMA_STATUS_NOMEM;

    *bridge0 = bridge;
    return MAMA_STATUS_OK;
}

static void
mamaEntitlementLibraryManager_destroyBridge (mamaEntitlementBridge bridge)
{
    free (bridge);
}

mama_status
mamaEntitlementLibraryManagerImpl_activateLibrary (mamaEntitlementLibrary entLibrary)
{
    if (0 == wInterlocked_read (&entLibrary->mActiveCount))
    {
        mamaLibrary library = entLibrary->mParent;
        wlock_lock (library->mLock);
        if (0 == wInterlocked_read (&entLibrary->mActiveCount))
        {
            mama_status status = 
                mamaEntitlementLibraryManagerImpl_loadBridge (entLibrary);

            if (MAMA_STATUS_OK != status)
                return status;

            status = 
                entLibrary->mBridge->entitlementSetup ();

            if (MAMA_STATUS_OK != status)
                return status;

            wInterlocked_increment (&entLibrary->mActiveCount);
        }        
        wlock_unlock (library->mLock);
    }
    return MAMA_STATUS_OK;
}

static void
mamaEntitlementLibraryManager_deactivateLibrary (mamaEntitlementLibrary entLibrary)
{
    wlock_lock (entLibrary->mParent->mLock);
    if (0 != wInterlocked_read (&entLibrary->mActiveCount))
    {
        entLibrary->mBridge->entitlementTearDown ();

        mamaEntitlementLibraryManager_destroyBridge (entLibrary->mBridge); 

        wInterlocked_decrement (&entLibrary->mActiveCount);    
    } 
    wlock_unlock (entLibrary->mParent->mLock);
}

static mama_status
mamaEntitlementLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaEntitlementLibrary* entLibrary0)
{
    assert (entLibrary0);
    *entLibrary0 = NULL;

    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaEntitlementLibrary entLibrary =
        (mamaEntitlementLibrary) calloc (1, sizeof (mamaEntitlementLibraryImpl));

    if (!entLibrary)
        return MAMA_STATUS_NOMEM;

    Entitlement_load load = 
        mamaEntitlementLibraryManagerImpl_getLibraryLoad (library);

    if (load)
    {
        status = load ();

        if (MAMA_STATUS_OK != status)
        {
             mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaEntitlementLibraryManagerImpl_createLibrary(): "
                      "Could not initialise %s library %s bridge using "
                      "new-style initialisation.",
                      library->mTypeName, library->mName);

            free (entLibrary);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        }
        
        status = mamaLibraryManager_compareMamaVersion (library);
        
        if (MAMA_STATUS_OK != status)
        {
            free (entLibrary);
            return status;
        }
    }

    library->mClosure = entLibrary;

    entLibrary->mBridge    = NULL;
    entLibrary->mParent    = library;
    entLibrary->mManager   = entManager;

    wInterlocked_initialize (&entLibrary->mActiveCount);
    wInterlocked_set (0, &entLibrary->mActiveCount);

    *entLibrary0 = entLibrary;
    return MAMA_STATUS_OK;
}

static void
mamaEntitlementLibraryManagerImpl_destroyLibrary (
        mamaEntitlementLibrary entLibrary)
{
    mamaLibrary library = entLibrary->mParent;

    mamaEntitlementLibraryManager_deactivateLibrary (entLibrary);

    Entitlement_unload unload = 
        mamaEntitlementLibraryManagerImpl_getLibraryUnload (library);

    if (unload)
    {
        if (MAMA_STATUS_OK != unload ())
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, 
                      "mamaEntitlementLibraryManagerImpl_destroyLibrary(): "
                      "Error unloading %s library %s", library->mTypeName, 
                      library->mName);
        }
    }

    library->mClosure = NULL;
    free (entLibrary);
}

static mama_status
mamaEntitlementLibraryManagerImpl_getEntitlementId (
        mamaEntitlementLibrary entLibrary, char* entitlementId)
{
    assert (entLibrary);
    assert (entitlementId);
    *entitlementId = 0;

    /* Attempting to find the entitlement ID takes a hierarchial approach:
     * first check for a configured ID, then check the old method and
     * finally just take a free slot in the array. */

    mamaLibrary library = entLibrary->mParent;
    const char* prop =
        mamaLibraryManager_getLibraryProperty (library, "id");

    if (prop)
    {
        *entitlementId = prop[0];
        return MAMA_STATUS_OK;
    }

    char tmpId;
    mama_status status =
            mamaEntitlementLibraryManager_stringToEntitlementId (library->mName, &tmpId);

    if (tmpId != MAMA_ENTITLEMENT_LIBRARY)
    {
        *entitlementId = tmpId;
        return MAMA_STATUS_OK;
    }

    mamaEntitlementLibraryManager entManager = entLibrary->mManager;
    status = MAMA_STATUS_SYSTEM_ERROR;

    /* Select next free slot */
    for (mama_size_t k = 0; k < MAMA_MAX_LIBRARIES; ++k)
    {
        if (!entManager->mEntitlements[k])
        {
            *entitlementId = k;
            status = MAMA_STATUS_OK;
            break;
        }
    }

    return status;
}

mama_status
mamaEntitlementLibraryManagerImpl_getDefaultBridge (mamaEntitlementBridge* bridge)
{
    *bridge = &defaultBridge;
    return MAMA_STATUS_OK;
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaEntitlementLibraryManager_create (mamaLibraryTypeManager manager)
{
    mamaEntitlementLibraryManager entManager = (mamaEntitlementLibraryManager)
        calloc (1, sizeof (mamaEntitlementLibraryManagerImpl));

    if (!entManager)
        return MAMA_STATUS_NOMEM;

    /* Establish bi-directional link */
    manager->mClosure = entManager;
    entManager->mParent = manager;

    return MAMA_STATUS_OK;
}

void
mamaEntitlementLibraryManager_destroy (void)
{
    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK == status)
    {
        entManager->mParent->mClosure = NULL;
        free (entManager);
    }
}

mama_status
mamaEntitlementLibraryManager_loadLibrary (mamaLibrary library)
{
    mamaEntitlementLibrary entLibrary = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_createLibrary (library,
                                                        &entLibrary);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaEntitlementLibraryManager entManager = NULL;
    status = mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    entLibrary->mManager = entManager;

    status =
        mamaEntitlementLibraryManagerImpl_getEntitlementId (entLibrary,
                                                        &entLibrary->mEntitlementId);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaEntitlementLibrary dupLibrary = entManager->mEntitlements [entLibrary->mEntitlementId];

    if (dupLibrary)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaEntitlementLibraryManager_loadLibrary(): "
                  "Entitlement id [%d] for %s library %s duplicates library %s.",
                  entLibrary->mEntitlementId, library->mTypeName,
                  library->mName,
                  dupLibrary->mParent->mName);

        return MAMA_STATUS_PLATFORM;
    }

    /* Store indexed lookup */
    entManager->mEntitlements [entLibrary->mEntitlementId] = entLibrary;
    return MAMA_STATUS_OK;
}

mama_status
mamaEntitlementLibraryManager_unloadLib (mamaEntitlementLibrary library)
{
    return mamaLibraryManager_unloadLibrary (
        mamaEntitlementLibraryManager_getName(library), MAMA_ENTITLEMENT_LIBRARY);
}

mama_status
mamaEntitlementLibraryManager_getLibrary (const char*         entitlementName,
                                          mamaEntitlementLibrary* entLibrary)
{
    if (!entLibrary || !entitlementName)
        return MAMA_STATUS_NULL_ARG;
    
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_getLibrary (entitlementName,
                                       MAMA_ENTITLEMENT_LIBRARY,
                                       &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *entLibrary =
        (mamaEntitlementLibrary) library->mClosure;

    return MAMA_STATUS_OK;
}

mama_status
mamaEntitlementLibraryManager_getLibraryById (char                entitlementId,
                                              mamaEntitlementLibrary* entLibrary)
{
    if (!entLibrary)
        return MAMA_STATUS_NULL_ARG;    

    *entLibrary = NULL;

    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    *entLibrary =
        entManager->mEntitlements[entitlementId];

    return (*entLibrary ? MAMA_STATUS_OK : MAMA_STATUS_NO_BRIDGE_IMPL);
}

mama_status
mamaEntitlementLibraryManager_getLibraries (mamaEntitlementLibrary* libraries,
                                            mama_size_t*        size)
{
    return mamaEntitlementLibraryManagerImpl_getLibraries (libraries, size,
                                                       NULL);
}

mama_status
mamaEntitlementLibraryManager_registerLoadCallback (mamaEntitlementLibraryCb cb,
                                                    void*           closure)
{
    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerLoadCallback (entManager->mParent,
                                                    (mamaLibraryCb)cb, closure);
}

mama_status
mamaEntitlementLibraryManager_registerUnloadCallback (mamaEntitlementLibraryCb cb,
                                                      void*           closure)
{
    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerUnloadCallback (entManager->mParent,
                                                      (mamaLibraryCb)cb, closure);
}

mama_status
mamaEntitlementLibraryManager_deregisterLoadCallback (mamaEntitlementLibraryCb cb)
{
    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_deregisterLoadCallback (entManager->mParent,
                                                      (mamaLibraryCb)cb);
}

mama_status
mamaEntitlementLibraryManager_deregisterUnloadCallback (mamaEntitlementLibraryCb cb)
{
    mamaEntitlementLibraryManager entManager = NULL;
    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_deregisterUnloadCallback (entManager->mParent,
                                                        (mamaLibraryCb)cb);
}

void
mamaEntitlementLibraryManager_unloadLibrary (mamaLibrary library)
{
    mamaEntitlementLibrary entLibrary = ((mamaEntitlementLibrary) library->mClosure);
    mamaEntitlementLibraryManager entManager = entLibrary->mManager;

    char entitlementId = entLibrary->mEntitlementId;
    mamaEntitlementLibraryManagerImpl_destroyLibrary (entLibrary);
    entManager->mEntitlements[entitlementId] = NULL;
}

mamaLibraryType
mamaEntitlementLibraryManager_classifyLibraryType (const char* libraryName,
                                                   LIB_HANDLE  libraryLib)
{
    Entitlement_load load =  
        mamaEntitlementLibraryManagerImpl_getLoad (libraryName, 
                                                   libraryLib);
    if (load)
        return MAMA_ENTITLEMENT_LIBRARY;

    Entitlement_unload unload =  
        mamaEntitlementLibraryManagerImpl_getUnload (libraryName, 
                                                     libraryLib);
 
    if (unload)
        return MAMA_ENTITLEMENT_LIBRARY;

    return MAMA_UNKNOWN_LIBRARY;
}

void mamaEntitlementLibraryManager_dump (mamaLibraryTypeManager manager)
{
}

void mamaEntitlementLibraryManager_dumpLibrary (mamaLibrary library)
{
    mamaEntitlementLibrary entLibrary = 
        (mamaEntitlementLibrary) library->mClosure;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Entitlement Id [%c]", 
                                entLibrary->mEntitlementId);
}

mama_status
mamaEntitlementLibraryManager_getEntitlementBridge (mamaEntitlementBridge* bridge)
{
#ifndef WITH_ENTITLEMENTS
    return mamaEntitlementLibraryManagerImpl_getDefaultBridge (bridge);
#else
    
    const char* manager = 
        mama_getProperty ("mama.entitlement.manager");   

    if (!manager)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Entitlement manager is not configured - "
            "this must be set for an entitled build");
        return MAMA_STATUS_NOT_ENTITLED;
    } 

    mamaEntitlementLibrary library = NULL;
    mama_status status = 
        mamaEntitlementLibraryManager_getLibrary (&library, manager);

    if (MAMA_STATUS_OK != status)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Cannot find library %s for entitlements - "
            "this must be present when configured for an entitled build", manager);
        return status;
    }

    mamaEntitlementLibraryManagerImpl_activateLibrary (library);

    *bridge = library->mBridge; 
#endif
    return MAMA_STATUS_OK;
}

/*
 * Public implementation
 */

mama_status
mamaEntitlementLibraryManager_loadLibraryWithPath (const char*             entName,
                                                   const char*             path,
                                                   mamaEntitlementLibrary* entLibrary0)
{
    if (!entLibrary0 || !entName)
        return MAMA_STATUS_NULL_ARG;
    
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_loadLibrary (entName,
                                        MAMA_ENTITLEMENT_LIBRARY,
                                        path, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *entLibrary0 =
        (mamaEntitlementLibrary) library->mClosure;

    return MAMA_STATUS_OK;
}

mama_status
mamaEntitlementLibraryManager_setProperty (const char* libraryName,
                                           const char* propertyName,
                                           const char* value)
{
    return mamaLibraryManager_setProperty (libraryName,
                                           propertyName,
                                           value);
}

const char* 
mamaEntitlementLibraryManager_getName (mamaEntitlementLibrary library)
{
    return mamaLibraryManager_getName(library->mParent);
}

const char*
mamaEntitlementLibraryManager_getPath (mamaEntitlementLibrary library)
{
    return mamaLibraryManager_getPath(library->mParent);
}

mama_status
mamaEntitlementLibraryManager_entitlementIdToString (char         entitlementId,
                                                     const char** str)
{
    if (!str)
        return MAMA_STATUS_NULL_ARG;

    *str = NULL;
    mamaEntitlementLibraryManager entManager = NULL;

    mama_status status =
        mamaEntitlementLibraryManagerImpl_getInstance (&entManager);

    if (status != MAMA_STATUS_OK)
        return status;

    mamaEntitlementLibrary library = entManager->mEntitlements[entitlementId];

    if (!library)
        return MAMA_STATUS_NOT_FOUND;

    *str = library->mParent->mName;
    return status;
}

mama_status
mamaEntitlementLibraryManager_stringToEntitlementId (const char*  str, 
                                                     char*        entitlementId)
{
    if (!str || !entitlementId)
        return MAMA_STATUS_NULL_ARG;

    *entitlementId = 0;
    mamaEntitlementLibrary library = NULL;
    
    mama_status status =
        mamaEntitlementLibraryManager_getLibrary (str, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *entitlementId = library->mEntitlementId;
    return status;
}

