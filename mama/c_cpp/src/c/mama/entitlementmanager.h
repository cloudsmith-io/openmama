/*
 * OpenMAMA: The open entitlement agnostic messaging API
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

#ifndef MamaEntitlementManagerH__
#define MamaEntitlementManagerH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef mama_bool_t
(*mamaEntitlementLibraryCb) (mamaEntitlementLibrary library,
                             void*                  closure);

/**
 * @brief Load a specific entitlement bridge (if not already loaded).
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_loadLibraryWithPath (const char*             entitlementName,
                                                   const char*             path,
                                                   mamaEntitlementLibrary* entitlement);
/**
 * @brief Unload a specific entitlement library.
 *
 * Method to unload a specific entitlement library. 
 *
 * @param[in] library The library to be unloaded.
 * 
 *@return A mama_status indicating the success or failure of the unload.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_unloadLib (mamaEntitlementLibrary library);

/**
 * @brief Retrieve a specific entitlement library.
 *
 * Method to return a specific entitlement based on name.
 *
 * @param[in]  entitlementName The name of the entitlement we want to retrieve.
 * @param[out] library A pointer to the mamaEntitlementLibrary structure where
 *             the entitlement library will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_getLibrary (const char*         entitlementName,
                                      mamaEntitlementLibrary* library);

/*
 * @brief Retrieve the default (usually first loaded) entitlement library.
 *
 * Method to return the default entitlement library.
 *
 * @param[out] library A pointer to the mamaEntitlementLibrary structure where
 *                     the entitlement library will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_getDefaultLibrary (mamaEntitlementLibrary* library);

/**
 * @brief Retrieve a specific entitlement library by Id.
 *
 * Method to retrieve a specific entitlement library by Id.
 *
 * @param[in]  entitlementId The ID of the entitlement we want to retrieve.
 * @param[out] library A pointer to the mamaEntitlementLibrary structure where
 *             the entitlement library will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_getLibraryById (char                entitlementId,
                                              mamaEntitlementLibrary* library);
/**
 * @brief Retrieve all loaded entitlement libraries.
 *
 * Method to return all loaded entitlement libraries.
 *
 * @param[out] libraries A pointer to an array of mamaEntitlementLibrary structures
 *             where the return libraries will be put.
 *
 * @param[in/out] size On calling the function this should be the size
 *                of the array where we are to put the libraries, on the 
 *                function return this is the number of elements in the array.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_getLibraries (mamaEntitlementLibrary* libraries,
                                            mama_size_t*        size);

/**
 * @brief Register a callback to be triggered when a new entitlement library is loaded.
 *
 * Method to register a callback to be triggered on a new entitlement load.
 *
 * @param[in] cb A mamaEntitlementLibraryCb function pointer to be trigger 
 *               on load of a new entitlement.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_registerLoadCallback (mamaEntitlementLibraryCb cb,
                                                    void*           closure);

/**
 * @brief Register a callback to be triggered when a entitlement library is unloaded.
 *
 * Method to register a callback to be triggered when a entitlement is unloaded.
 *
 * @param[in] cb A mamaEntitlementLibraryCb function pointer to be trigger 
 *               on unload of a entitlement.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_registerUnloadCallback (mamaEntitlementLibraryCb cb,
                                                      void*           closure);

/**
 * @brief Deregister a callback to be triggered when a new entitlement 
 *        library is loaded.
 *
 * Method to deregister a callback to be triggered on a new entitlement 
 * library load.
 *
 * @param[in] cb A mamaEntitlementLibraryCb function pointer to be trigger on 
 *               load of a new entitlement.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_deregisterLoadCallback (mamaEntitlementLibraryCb cb);

/**
 * @brief Deregister a callback to be triggered when a entitlement 
 * library is unloaded.
 *
 * Method to deregister a callback to be triggered when a entitlement 
 * library is unloaded.
 *
 * @param[in] cb A mamaEntitlementLibraryCb function pointer that was previously
 *               registered.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_deregisterUnloadCallback (mamaEntitlementLibraryCb cb);

/**
 * @brief Retrieve the name of a library.
 *
 * Method to retrieve the name of a library.
 *
 * @param[in] library A mamaEntitlementLibrary from which to get the name.
 *
 * @return A null terminated string of the name - do NOT free this..
 */
MAMAExpDLL
extern const char* 
mamaEntitlementLibraryManager_getName (mamaEntitlementLibrary library);

/**
 * @brief Retrieve the path from where a library was loaded.
 *
 * Method to retrieve the path from where a library was loaded.
 *
 * @param[in] library A mamaEntitlementLibrary.
 *
 * @return A null terminated string of the path - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaEntitlementLibraryManager_getPath (mamaEntitlementLibrary library);

/*
 * @brief Set a property programmatically for a specific library.
 * 
 * Method to set a library property programmatically.
 *
 * @param[in] libraryName Name of the library for which 
              the property will be set.
 * @param[in] propertyName Property name e.g. ignore for 
 *            mama.library.<library-name>.ignore.
 * @param[in] value Value of the property
 */

MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_setProperty (const char* libraryName,
                                           const char* propertyName,
                                           const char* value);

/**
 * @brief Get a string representation of a entitlement Id.
 *
 * Method to get a string representation of a entitlement Id.
 *
 * @param[in] entitlementId The Id to be converted to a string.
 * @param[out] str A pointer where the string representation will be put.
 *             Note: Do NOT free this memory.  
 *              
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_entitlementIdToString (char         entitlementId,
                                                     const char** str);
/**
 * @brief Get the entitlement Id from the string representation of the library name.
 *
 * Method to get the entitlement Id from the library name.
 *
 * @param[in] str library name to be looked up to find the entitlementId.
 * @param[out] entitlementId Pointer to a character where the ID will be put.
 *             Note: Do NOT free this. 
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaEntitlementLibraryManager_stringToEntitlementId (const char*  str, 
                                                     char*        entitlementId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif

