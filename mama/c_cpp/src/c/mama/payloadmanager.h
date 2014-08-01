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
(*mamaPayloadCb) (mamaPayloadBridge bridge,
                  void*             closure);

/**
 * @brief Load a specific payload bridge (if not already loaded).
 *
 * Method to locate a shared library of a specific name and try 
 * and load it and its functions as a payload bridge.
 *
 * @param[in]  payloadName The name of the payload we want to load.
 * @param[in]  path The path to the location of the library if known,
               otherwise NULL.
 * @param[out] bridge A pointer to a mamaPayloadBridge structure where
 *             the payload bridge will be put, if found.
 * 
 *@return A mama_status indicating the success or failure of the load.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_loadBridgeWithPath (const char*        payloadName,
                                       const char*        path,
                                       mamaPayloadBridge* bridge);
/**
 * @brief Unload a specific payload bridge.
 *
 * Method to unload a specific payload bridge. 
 *
 * @param[in] bridge The bridge to be unloaded.
 * 
 *@return A mama_status indicating the success or failure of the unload.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_unloadBridge (mamaPayloadBridge bridge);

/**
 * @brief Retrieve a specific payload bridge.
 *
 * Method to return a specific payload based on name.
 *
 * @param[in]  payloadName The name of the payload we want to retrieve.
 * @param[out] bridge A pointer to the mamaPayloadBridge structure where
 *             the payload bridge will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_getBridge (const char*        payloadName,
                              mamaPayloadBridge* bridge);

/*
 * @brief Retrieve the default (usually first loaded) payload bridge.
 *
 * Method to return the default payload bridge.
 *
 * @param[out] bridge A pointer to the mamaPayloadBridge structure where
 *                     the payload library will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_getDefaultBridge (mamaPayloadBridge* bridge);

/**
 * @brief Retrieve a specific payload bridge by Id.
 *
 * Method to retrieve a specific payload bridge by Id.
 *
 * @param[in]  payloadId The ID of the payload we want to retrieve.
 * @param[out] bridge A pointer to the mamaPayloadBridge structure where
 *             the payload bridge will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_getBridgeById (char               payloadId,
                                 mamaPayloadBridge* bridge);
/**
 * @brief Retrieve all loaded payload bridges.
 *
 * Method to return all loaded payload bridges.
 *
 * @param[out] bridges A pointer to an array of mamaPayloadBridges structures
 *             where the return bridges will be put.
 *
 * @param[in/out] size On calling the function this should be the size
 *                of the array where we are to put the libraries, on the 
 *                function return this is the number of elements in the array.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_getBridges (mamaPayloadBridge* libraries,
                               mama_size_t*       size);


/**
 * @brief Register a callback to be triggered when a new payload library is loaded.
 *
 * Method to register a callback to be triggered on a new payload load.
 *
 * @param[in] cb A mamaPayloadBridgeCb function pointer to be trigger 
 *               on load of a new payload.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_registerLoadCallback (mamaPayloadCb cb,
                                         void*         closure);

/**
 * @brief Register a callback to be triggered when a payload library is unloaded.
 *
 * Method to register a callback to be triggered when a payload is unloaded.
 *
 * @param[in] cb A mamaPayloadBridgeCb function pointer to be trigger 
 *               on unload of a payload.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_registerUnloadCallback (mamaPayloadCb cb,
                                           void*         closure);

/**
 * @brief Deregister a callback to be triggered when a new payload 
 *        library is loaded.
 *
 * Method to deregister a callback to be triggered on a new payload 
 * library load.
 *
 * @param[in] cb A mamaPayloadBridgeCb function pointer to be trigger on 
 *               load of a new payload.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_deregisterLoadCallback (mamaPayloadCb cb);

/**
 * @brief Deregister a callback to be triggered when a payload 
 * library is unloaded.
 *
 * Method to deregister a callback to be triggered when a payload 
 * library is unloaded.
 *
 * @param[in] cb A mamaPayloadBridgeCb function pointer that was previously
 *               registered.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_deregisterUnloadCallback (mamaPayloadCb cb);

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
mamaPayloadManager_setProperty (const char* libraryName,
                                const char* propertyName,
                                const char* value);
/**
 * @brief Retrieve the name of a library.
 *
 * Method to retrieve the name of a library.
 *
 * @param[in] library A mamaPayloadBridge from which to get the name.
 *
 * @return A null terminated string of the name - do NOT free this..
 */
MAMAExpDLL
extern const char* 
mamaPayloadManager_getName (mamaPayloadBridge bridge);

/**
 * @brief Retrieve the Id of a library.
 *
 * Method to retrieve the Id of a library.
 *
 * @param[in] library A mamaPayloadBridge from which to get the name.
 *
 * @return A character representing the ID.
 */
MAMAExpDLL
extern char 
mamaPayloadManager_getId (mamaPayloadBridge bridge);

/**
 * @brief Retrieve the path from where a library was loaded.
 *
 * Method to retrieve the path from where a library was loaded.
 *
 * @param[in] library A mamaPayloadBridge.
 *
 * @return A null terminated string of the path - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaPayloadManager_getPath (mamaPayloadBridge bridge);

/**
 * @brief set the default payload by ID.
 *
 * Method to set the default payload by ID.
 *
 * @param[in] id ID of the payload to set as the default.
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaPayloadManager_setDefaultPayloadbyId (char id);

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
mamaPayloadManager_payloadIdToString (char         payloadId,
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
mamaPayloadManager_stringToPayloadId (const char* str, 
                                      char*       payloadId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaPayloadManagerH__ */
