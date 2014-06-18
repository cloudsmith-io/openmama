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

#ifndef MamaLibraryManagerInternalH__
#define MamaLibraryManagerInternalH__

#include <wlock.h>
#include <wombat/wtable.h>
#include <wombat/wInterlocked.h>
#include "mama/library.h"
#include "mama/status.h"
#include "mama/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

#define MAX_LIBRARY_FUNCTION_NAME  256
#define MAX_LIBRARY_SIGNALS        256
#define MAX_LIBRARY_SIGNAL_SLOTS   256

/* Forward declarations */
typedef struct mamaLibraryTypeManagerImpl_*       mamaLibraryTypeManager;
typedef struct mamaLibraryTypeManagerBridgeImpl_* mamaLibraryTypeManagerBridge;
typedef struct mamaLibraryManagerImpl_*           mamaLibraryManager;

/*********************************************************
 * Predicate typedef for "get" functions 
 *      e.g. get open bridges
 *********************************************************/

typedef mama_bool_t
(*mamaLibraryPredicateCb) (mamaLibrary library);

typedef mama_bool_t
(*mamaLibraryIterateCb) (mamaLibrary library,
                         void*       closure);

/*********************************************************
 * Library Type Manager Bridge Functions
 *********************************************************/

typedef mama_status
(*mamaLibraryTypeManager_create) (mamaLibraryTypeManager manager);

typedef void
(*mamaLibraryTypeManager_destroy) (void);

typedef mama_status
(*mamaLibraryTypeManager_loadLibrary) (mamaLibrary library);

typedef void
(*mamaLibraryTypeManager_unloadLibrary) (mamaLibrary library);

typedef mamaLibraryType
(*mamaLibraryTypeManager_classifyLibraryType) (const char* libraryName,
                                               LIB_HANDLE  libraryHandle);

typedef const char*
(*mamaLibraryTypeManager_getLibraryProperty) (mamaLibrary library,
                                              const char* property);

typedef mama_bool_t
(*mamaLibraryTypeManager_getLibraryBoolProperty) (mamaLibrary library,
                                                  const char* property);

typedef const char*
(*mamaLibraryTypeManager_getLibraryIgnore) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryName) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryDescription) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryAuthor) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryUri) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryLicense) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryVersion) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryMamaVersion) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryBridgeAuthor) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryBridgeUri) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryBridgeLicense) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryBridgeVersion) (mamaLibrary library);

typedef const char*
(*mamaLibraryTypeManager_getLibraryBridgeMamaVersion) (mamaLibrary library);


/*********************************************************
 * Library Manager Data Structures.
 *********************************************************/

typedef struct mamaLibraryCallbackSlot
{
    mamaLibraryCb mCb;
    void*         mClosure;
} mamaLibraryCallbackSlot;


typedef struct mamaLibraryCallbackSignal
{
    mama_size_t             mSize;
    mama_size_t             mNextId;
    mamaLibraryCallbackSlot mSlots [MAX_LIBRARY_SIGNAL_SLOTS];
} mamaLibraryCallbackSignal;


typedef struct mamaLibraryCallbackRegister
{
    mama_size_t               mSize;
    mama_size_t               mNextId;
    mamaLibraryCallbackSignal mSignals [MAX_LIBRARY_SIGNALS];
} mamaLibraryCallbackRegister;


typedef struct mamaLibraryImpl_
{
    const char*                 mName;      /**< Library name */
    mamaLibraryType             mType;      /**< Library type (enum) */
    const char*                 mTypeName;  /**< Library type (string) */
    LIB_HANDLE                  mHandle;    /**< Shared library handle */
    void*                       mClosure;   /**< Library type specific closure */
    wLock                       mLock;      /**< Library specific lock */
    const char*                 mPath;      /**< Path library was loaded from */
    mamaLibraryTypeManager      mManager;   /**< Reference to owning manager */
} mamaLibraryImpl;


typedef struct mamaLibraryTypeManagerBridgeImpl_
{
    mamaLibraryTypeManager_create                      create;
    mamaLibraryTypeManager_destroy                     destroy;
    mamaLibraryTypeManager_loadLibrary                 loadLibrary;
    mamaLibraryTypeManager_unloadLibrary               unloadLibrary;
    mamaLibraryTypeManager_classifyLibraryType         classifyLibraryType;
    mamaLibraryTypeManager_getLibraryProperty          getLibraryProperty;
    mamaLibraryTypeManager_getLibraryBoolProperty      getLibraryBoolProperty;
    mamaLibraryTypeManager_getLibraryIgnore            getLibraryIgnore;
    mamaLibraryTypeManager_getLibraryName              getLibraryName;
    mamaLibraryTypeManager_getLibraryAuthor            getLibraryAuthor;
    mamaLibraryTypeManager_getLibraryUri               getLibraryUri;
    mamaLibraryTypeManager_getLibraryLicense           getLibraryLicense;
    mamaLibraryTypeManager_getLibraryVersion           getLibraryVersion;
    mamaLibraryTypeManager_getLibraryMamaVersion       getLibraryMamaVersion;
    mamaLibraryTypeManager_getLibraryBridgeAuthor      getLibraryBridgeAuthor;
    mamaLibraryTypeManager_getLibraryBridgeUri         getLibraryBridgeUri;
    mamaLibraryTypeManager_getLibraryBridgeLicense     getLibraryBridgeLicense;
    mamaLibraryTypeManager_getLibraryBridgeVersion     getLibraryBridgeVersion;
    mamaLibraryTypeManager_getLibraryBridgeMamaVersion getLibraryBridgeMamaVersion;
} mamaLibraryTypeManagerBridgeImpl;

typedef struct mamaLibraryTypeManagerImpl_
{
    mamaLibraryType              mType;
    const char*                  mTypeName;
    const char*                  mTypeNameTitle;
    wtable_t                     mLibraries;
    wInterlockedInt              mNumLibraries;
    wLock                        mLock;
    void*                        mClosure;
    mamaLibraryTypeManagerBridge mFuncs;
    mamaLibraryCallbackRegister  mCallbacks;
    mama_size_t                  mLoadSignal;
    mama_size_t                  mUnloadSignal;
} mamaLibraryTypeManagerImpl;

/*
 * Library manager functions
 */

/**
 * @brief Destroys the library manager .
 *
 * Takes a libraryName, an optional path, a void* structure, and a library
 * type, and loads the appropriate shared library. It then searches the library
 * for a function which is used to initialise the library, before finally
 * calling the bridge registration methods.
 *
 * @return A mama_status denoting the success or failure of the bridge loading.
 *         MAMA_STATUS_OK denotes a successful load.
 */
extern void
mamaLibraryManager_destroy (void);

/**
 * @brief Load and initialise libraries from a specific path or paths.
 *
 * This function tries to load all the libraries from a specific path or paths.
 * First it checks whether there is a global path for all libraries, if it is
 * not present then it checks for individual paths for middleware/payload and
 * plugins.
 *
 * @return A mama_status denoting the success or failure of the bridge loading.
 *         MAMA_STATUS_OK denotes a successful load.
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates a failure to load the shared library.
 *         MAMA_STATUS_NOMEM indicates a failure to allocate memeory.
 *
 */
extern mama_status
mamaLibraryManager_loadAll (void);

/**
 * @brief Load and initialise libraries of specific types.
 *
 * Takes a libraryName, an optional path, a void* structure, and a library
 * type, and loads the appropriate shared library. It then searches the library
 * for a function which is used to initialise the library, before finally
 * calling bridge registration method.
 *
 * @param libraryName The string name for a library e.g. wmw, qpid.
 * @param libraryType enum mamaLibraryType value denoting the type of library
 *                    to be loaded.
 * @param path An optional path on which the library may be found.
 * @param[out] library Library pointer to store result in.
 *
 * @return A mama_status denoting the success or failure of the bridge loading.
 *         MAMA_STATUS_OK denotes a successful load.
 *         MAMA_STATUS_NULL_ARG indicates a NULL argument was passed to the function.
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates a failure to load the shared library.
 *         MAMA_STATUS_NOMEM indicates a failure to allocate memeory.
 *           system (wTable failures in particular).
 *
 */
extern mama_status
mamaLibraryManager_loadLibrary (const char*     libraryName,
                                mamaLibraryType libraryType,
                                const char*     path,
                                mamaLibrary*    library);

extern mama_status
mamaLibraryManager_unloadLibrary (const char*     libraryName,
                                  mamaLibraryType libraryType);

/**
 * @brief Load a function from the specified library.
 *
 * Attempts to load a function from the library utilising the
 * function specification, which is in the form of "func_%s",
 * of which function name is interpolated into it.  If the
 * function cannot be found then the alternative is tried, and
 * if that isn't found the specified default is returned.
 *
 * @param lib The handle to the loaded shared library (or current process
 *            if the handle is NULL).
 * @param funcSpec String specification for function name.
 * @param funcName String to be interpolated into spec.
 * @param funcAltSpec Secondary string specification for function name.
 * @param funcAltName Secondary string to be interpolated into spec.
 * @param funcDefault Default function pointer to return if function not found.
 * @return A pointer to the loaded function, or default if not found.
 */
extern void*
mamaLibraryManager_loadFunction (LIB_HANDLE  lib,
                                 const char* funcSpec,
                                 const char* funcName,
                                 const char* funcAltSpec,
                                 const char* funcAltName,
                                 void*       funcDefault);

/*
 * @brief Lookup a library type manager.
 *
 * @param libraryType Enumeration value for library type.
 * @param[out] typeManager Library type manager pointer to store result in.
 */
extern mama_status
mamaLibraryManager_getTypeManager (mamaLibraryType         libraryType,
                                   mamaLibraryTypeManager* typeManager);

/*
 * @brief Lookup a library of a specific type.
 *
 * @param name NULL-terminated string of the library name.
 * @param libraryType Enumeration value for library type.
 * @param[out] library Library pointer to store result in.
 */
extern mama_status
mamaLibraryManager_getLibrary (const char*     libraryName,
                               mamaLibraryType libraryType,
                               mamaLibrary*    library);


/*
 * @brief Retrieve an array of all available/loaded libraries.
 */
extern mama_status
mamaLibraryManager_getLibraries (mamaLibrary*           libraries,
                                 mama_size_t*           size,
                                 mamaLibraryType        libraryType,
                                 mamaLibraryPredicateCb predicate);

extern mama_status
mamaLibraryManager_iterateLibraries (mamaLibraryType      libraryType,
                                     mamaLibraryIterateCb cb,
                                     void*                closure);

extern mama_status
mamaLibraryManager_createCallbackSignal (mamaLibraryTypeManager manager,
                                         mama_size_t*           signalId);

extern mama_status
mamaLibraryManager_destroyCallbackSignal (mamaLibraryTypeManager manager,
                                          mama_size_t            signalId);

extern mama_status
mamaLibraryManager_createCallbackSlot (mamaLibraryTypeManager manager,
                                       mama_size_t            signalId,
                                       mamaLibraryCb          cb,
                                       void*                  closure);

extern mama_status
mamaLibraryManager_destroyCallbackSlot (mamaLibraryTypeManager manager,
                                        mama_size_t            signalId,
                                        mamaLibraryCb          cb);

extern mama_status
mamaLibraryManager_raiseCallbackSignal (mamaLibrary library,
                                        mama_size_t signalId);

extern mama_status
mamaLibraryManager_registerLoadCallback (mamaLibraryTypeManager manager,
                                         mamaLibraryCb          cb,
                                         void*                  closure);

extern mama_status
mamaLibraryManager_registerUnloadCallback (mamaLibraryTypeManager manager,
                                           mamaLibraryCb          cb,
                                           void*                  closure);

/*
 * @brief Retrieve a library property based on hierarchy.
 *
 * 1. Try library-specific property: mama.library.<libname>.<property>
 * 2. Try library-type property:     mama.library.<libtype>.<property>
 * 3. Try default library property:  mama.library.default.<property>
 *
 * @return A null-terminated string of the property value, or NULL if
 *         the property could not be found in the hierarchy.
 */
extern const char*
mamaLibraryManager_getProperty (const char*     library,
                                const char*     property,
                                mamaLibraryType libraryType);

/*
 * @brief Retrieve a library bool property based on hierarchy.
 *
 * @see mamaLibraryManager_getProperty
 *
 * @return 0 if property not found, or equates to false, 1 if found and true.
 */
extern mama_bool_t
mamaLibraryManager_getBoolProperty (const char*     library,
                                    const char*     property,
                                    mamaLibraryType libraryType);

extern const char*
mamaLibraryManager_getLibraryProperty (mamaLibrary library,
                                       const char* property);

extern mama_bool_t
mamaLibraryManager_getLibraryBoolProperty (mamaLibrary library,
                                           const char* property);

/**
 * @brief Return the library type enumeration value converted from string.
 *
 * The return value will be MAMA_UNKNOWN_LIBRARY if the conversion is not valid.
 *
 * @param name The string to be converted into the library type enum.
 * @return     An enumeration value from mamaLibraryType.
 */
extern mamaLibraryType
mamaLibraryManager_stringToLibraryType (const char* name);

/**
 * @brief Return the library type string converted from library type enum.
 *
 * @param libraryType The library type enum to be converted into string.
 * @return     A null-terminated string representing the library type name.
 */
extern const char*
mamaLibraryManager_libraryTypeToString (mamaLibraryType libraryType);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaLibraryManagerInternalH__ */
