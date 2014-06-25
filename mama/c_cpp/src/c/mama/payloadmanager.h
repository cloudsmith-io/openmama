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

#ifndef MamaPayloadManagerH__
#define MamaPayloadManagerH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef mama_bool_t
(*mamaPayloadLibraryCb) (mamaPayloadLibrary library,
                         void*              closure);

/**
 * @brief Load a specific payload bridge (if not already loaded).
 *
 * Method to locate a shared library of a specific name and try 
 * and it and its functions as a payload library.
 *
 * @param[in]  payloadName The name of the payload we want to load.
 * @param[in]  path The path to the location of the library if known,
               otherwise NULL.
 * @param[out] library A pointer to a mamaPayloadLibrary structure where
 *             the payload bridge will be put, if found.
 * 
 *@return A mama_status indicating the success or failure of the load.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_loadLibraryWithPath (const char* payloadName,
                                               const char* path,
                                               mamaPayloadLibrary* library);
/**
 * @brief Unload a specific payload bridge.
 *
 * Method to unload a specific payload bridge. 
 *
 * @param[in] library The library to be unloaded.
 * 
 *@return A mama_status indicating the success or failure of the unload.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_unloadLib (mamaPayloadLibrary library);

/**
 * @brief Retrieve a specific payload library.
 *
 * Method to return a specific payload based on name.
 *
 * @param[in]  payloadName The name of the payload we want to retrieve.
 * @param[out] library A pointer to the mamaPayloadLibrary structure where
 *             the payload library will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getLibrary (const char*         payloadName,
                                      mamaPayloadLibrary* library);

/*
 * @brief Retrieve the default (usually first loaded) payload library.
 *
 * Method to return the default payload library.
 *
 * @param[out] library A pointer to the mamaPayloadLibrary structure where
 *                     the payload library will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getDefaultLibrary (mamaPayloadLibrary* library);

/**
 * @brief Retrieve a specific payload library by Id.
 *
 * Method to retrieve a specific payload library by Id.
 *
 * @param[in]  payloadId The ID of the payload we want to retrieve.
 * @param[out] library A pointer to the mamaPayloadLibrary structure where
 *             the payload library will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getLibraryById (char                payloadId,
                                          mamaPayloadLibrary* library);
/**
 * @brief Retrieve all loaded payload libraries.
 *
 * Method to return all loaded payload libraries.
 *
 * @param[out] libraries A pointer to an array of mamaPayloadLibrary structures
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
mamaPayloadLibraryManager_getLibraries (mamaPayloadLibrary* libraries,
                                        mama_size_t*        size);


/**
 * @brief Register a callback to be triggered when a new payload library is loaded.
 *
 * Method to register a callback to be triggered on a new payload load.
 *
 * @param[in] cb A mamaPayloadLibraryCb function pointer to be trigger 
 *               on load of a new payload.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_registerLoadCallback (mamaPayloadLibraryCb cb,
                                                void*           closure);

/**
 * @brief Register a callback to be triggered when a payload library is unloaded.
 *
 * Method to register a callback to be triggered when a payload is unloaded.
 *
 * @param[in] cb A mamaPayloadLibraryCb function pointer to be trigger 
 *               on unload of a payload.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_registerUnloadCallback (mamaPayloadLibraryCb cb,
                                                  void*           closure);

/**
 * @brief Deregister a callback to be triggered when a new payload 
 *        library is loaded.
 *
 * Method to deregister a callback to be triggered on a new payload 
 * library load.
 *
 * @param[in] cb A mamaPayloadLibraryCb function pointer to be trigger on 
 *               load of a new payload.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_deregisterLoadCallback (mamaPayloadLibraryCb cb);

/**
 * @brief Deregister a callback to be triggered when a payload 
 * library is unloaded.
 *
 * Method to deregister a callback to be triggered when a payload 
 * library is unloaded.
 *
 * @param[in] cb A mamaPayloadLibraryCb function pointer that was previously
 *               registered.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_deregisterUnloadCallback (mamaPayloadLibraryCb cb);

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
mamaPaylaodLibraryManager_setProperty (const char* libraryName,
                                       const char* propertyName,
                                       const char* value);

/**
 * @brief Retrieve the name of a library.
 *
 * Method to retrieve the name of a library.
 *
 * @param[in] library A mamaPayloadLibrary from which to get the name.
 *
 * @return A null terminated string of the name - do NOT free this..
 */
MAMAExpDLL
extern const char* 
mamaPayloadLibraryManager_getName (mamaPayloadLibrary library);

/**
 * @brief Retrieve the path from where a library was loaded.
 *
 * Method to retrieve the path from where a library was loaded.
 *
 * @param[in] library A mamaPayloadLibrary.
 *
 * @return A null terminated string of the path - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaPayloadLibraryManager_getPath (mamaPayloadLibrary library);

/**
 * @brief Get a string representation of a payload Id.
 *
 * Method to get a string representation of a payload Id.
 *
 * @param[in] payloadId The Id to be converted to a string.
 * @param[out] str A pointer where the string representation will be put.
 *             Note: Do NOT free this memory.  
 *              
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_payloadIdToString (char         payloadId,
                                             const char** str);
/**
 * @brief Get the payload Id from the string representation of the library name.
 *
 * Method to get the payload Id from the library name.
 *
 * @param[in] str library name to be looked up to find the payloadId.
 * @param[out] payloadId Pointer to a character where the ID will be put.
 *             Note: Do NOT free this. 
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_stringToPayloadId (const char* str, 
                                             char*       payloadId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaPayloadManagerH__ */

