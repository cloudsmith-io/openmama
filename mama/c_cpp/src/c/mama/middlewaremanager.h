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

#ifndef MamaMiddlewareManagerH__
#define MamaMiddlewareManagerH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Generic callback defintion for any middleware library callback*/

typedef mama_bool_t
(*mamaMiddlewareCb) (mamaBridge library,
                     void*      closure);

/**
 * @brief Load a specific middleware bridge (if not already loaded).
 *
 * Method to locate a shared library of a specific name and try 
 * and load it and its functions as a middleware bridge.
 *
 * @param[in]  middlewareName The name of the middleware we want to load.
 * @param[in]  path The path to the location of the library if known,
               otherwise NULL.
 * @param[out] bridge A pointer to a mamaBridge structure where
 *             the middleware bridge will be put, if found.
 * 
 *@return A mama_status indicating the success or failure of the load.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_loadBridgeWithPath (const char* middlewareName,
                                          const char* path,
                                          mamaBridge* bridge);

/**
 * @brief Unload a specific middleware bridge.
 *
 * Method to unload a specific middleware bridge. 
 *
 * @param[in] bridge The library to be unloaded.
 * 
 *@return A mama_status indicating the success or failure of the unload.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_unloadBridge (mamaBridge bridge);

/**
 * @brief Retrieve a specific middleware bridge.
 *
 * Method to return a specific middleware bridge based on name.
 *
 * @param[in]  middlewareName The name of the payload we want to retrieve.
 * @param[out] bridge A pointer to the mamaBridge structure where
 *             the middleware bridge will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getBridge (const char* middlewareName,
                                 mamaBridge* bridge);

/*
 * @brief Retrieve the default (usually first loaded) middleware bridge.
 *
 * Method to return the default middleware bridge.
 *
 * @param[out] bridge A pointer to the mamaBridge structure where
 *                     the middleware bridge will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getDefaultBridge (mamaBridge* bridge);

/**
 * @brief Retrieve all loaded middleware bridges.
 *
 * Method to get all the loaded middleware bridges.
 * 
 * @param[out] libraries A pointer to an array of mamaBridge structures
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
mamaMiddlewareManager_getBridges (mamaBridge*  bridges,
                                  mama_size_t* size);

/**
 * @brief Retrieve all loaded middleware bridges, which represent open bridges.
 *
 * Method to retrieve all bridges representing open middleware bridges.
 *
 * @param[out] bridges A pointer to an array of mamaBridge structures
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
mamaMiddlewareManager_getOpenedBridges (mamaBridge*   bridge,
                                        mama_size_t*  size);
/**
 * @brief Retrieve all loaded middleware bridges, which represent active bridges.
 *
 * Method to retrieve all bridges representing active middleware bridges.
 *
 * @param[out] bridges A pointer to an array of mamaBridge structures
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
mamaMiddlewareManager_getActiveBridges (mamaBridge*  bridges,
                                        mama_size_t* size);

/**
 * @brief Retrieve all loaded middleware bridges, which represent inactive bridges.
 *
 * Method to retrieve all bridges representing inactive middleware bridges.
 *
 * @param[out] libraries A pointer to an array of mamaBridge structures
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
mamaMiddlewareManager_getInactiveBridges (mamaBridge*  bridges,
                                          mama_size_t* size);

/**
 * @brief Retrieve all loaded middleware bridges, which represent closed bridges.
 *
 * Method to retrieve all bridges representing closed middleware bridges.
 *
 * @param[out] bridges A pointer to an array of mamaBridge structures
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
mamaMiddlewareManager_getClosedBridges (mamaBridge*  bridges,
                                        mama_size_t* size);
/**
 * @brief Get the number of middleware bridges loaded.
 *
 * Method to retrieve the number of loaded middleware bridges.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getNumBridges (mama_size_t* size);

/**
 * @brief Get the number of middleware libraries representing open bridges.
 *
 * Method to retrieve the number of loaded middleware libraries, which represent
 * open middleware bridges.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries will be put. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getNumOpenedBridges (mama_size_t* size);

/**
 * @brief Get the number of middleware libraries representing active bridges.
 *
 * Method to retrieve the number of loaded middleware libraries, which represent
 * active middleware bridges.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries will be put. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getNumActiveBridges (mama_size_t* size);

/**
 * @brief Get the number of middleware libraries representing inactive bridges.
 *
 * Method to retrieve the number of loaded middleware libraries, which represent
 * inactive middleware bridges.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries will be put. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getNumInactiveBridges (mama_size_t* size);

/**
 * @brief Get the number of middleware libraries representing closed bridges.
 *
 * Method to retrieve the number of loaded middleware libraries, which represent
 * closed middleware bridges.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries will be put. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getNumClosedBridges (mama_size_t* size);

/**
 * @brief Open the bridge represented by the mamaBridge handle.
 *
 * Method to open the bridge associated with the provided mamaBridge,
 * defers the opening to the middleware library's specific implementation of the 
 * open function.
 *
 * @param[in] library A mamaBridge handle to the bridge to be opened.
 *
 * @return A mama_status indicating the success or failure of the bridge open.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_openBridge (mamaBridge bridge);

/**
 * @brief Close the bridge represented by the mamaBridge handle.
 *
 * Method to close the bridge associated with the provided mamaBridge,
 * defers the closing to the middleware library's specific implementation of the 
 * close function.
 *
 * @param[in] library A mamaBridge handle to the bridge to be closed.
 *
 * @return A mama_status indicating the success or failure of the bridge close.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_closeBridge (mamaBridge bridge);

/**
 * @brief Start the middleware bridge represented by the mamaBridge handle.
 *
 * Method to start the middleware bridge associated with the provided 
 * mamaBridge, defers the start to the middleware library's 
 * specific implementation of the close function.
 *
 * @param[in] library A mamaBridge handle to the middleware bridge 
 *                    to be started.
 *
 * @return A mama_status indicating the success or failure of the bridge start.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_startBridge (mamaBridge bridge);

/**
 * @brief Start the middleware bridge represented by the mamaBridge handle
 *        on another thread.
 *
 * Method to start (in the background) the middleware bridge associated with the 
 * provided mamaBridge, defers the start to the middleware library's 
 * specific implementation of the close function.
 *
 * @param[in] library A mamaBridge handle to the middleware bridge 
 *                    to be started.
 *
 * @param[in] cb A callback to be fired if the background call to the bridge specific 
 *              start fails for any reason.
 *
 * @param[in] closure A pointer to any data to be passed to the structure.
 *
 * @return A mama_status indicating the success or failure of the bridge start.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_startBridgeBackground (mamaBridge       library,
                                             mamaMiddlewareCb cb,
                                             void*            closure);

/**
 * @brief Stop the middleware bridge represent by the mamaBridge handle.
 *
 * Method to stop the middleware bridge associated with the provided 
 * mamaBridge, defers the start to the middleware library's 
 * specific implementation of the close function.
 *
 * @param[in] library A mamaBridge handle to the middleware bridge 
 *                    to be stopped.
 *
 * @return A mama_status indicating the success or failure of the bridge stop.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_stopBridge (mamaBridge bridge);

/**
 * @brief Stops all middleware bridges.
 *
 * Method to stop all the middleware bridges. 
 *
 * @return A mama_status indicating the success or failure of the bridge opening.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_stopAllBridges (void);

/**
 * Get a reference to the internal default event queue in use for the specified
 * middleware.
 *
 * @param bridgeImpl The middleware for which the default event queue is being
 * obtained.
 * @param defaultQueue Address to which the defaultQueue is to be written.
 *
 * @return MAMA_STATUS_OK if the function returns successfully.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_getDefaultEventQueue (mamaBridge bridge,
                                            mamaQueue* defaultQueue);

/**
 * @brief Gets a string representing the library version.
 *
 * Method to return a string library version.
 *
 * @param[in] library A mamaBridge pointer of which to 
 *                    get the version.
 *
 * @return A const char* pointing to the library version - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaMiddlewareManager_getBridgeVersion (mamaBridge bridge);

/**
 * @brief Gets a string representing the minimum supported MAMA version 
 *        of the library.
 *
 * Method to return a string representing the minimum supported MAMA 
 * version for the library.
 *
 * @param[in] library A mamaBridge pointer of which to 
 *                    get the MAMA version.
 *
 * @return A const char* pointing to the MAMA version - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaMiddlewareManager_getBridgeMamaVersion (mamaBridge bridge);

/**
 * @brief Register a callback to be triggered when a new middleware 
 *        library is loaded.
 *
 * Method to register a callback to be triggered on a new middleware 
 * library load.
 *
 * @param[in] cb A mamaBridgeCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_registerLoadCallback (mamaMiddlewareCb cb,
                                            void*            closure);

/**
 * @brief Register a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * Method to register a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * @param[in] cb A mamaBridgeCb function pointer to be trigger on 
 *               unload of a middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_registerUnloadCallback (mamaMiddlewareCb cb,
                                              void*            closure);
/**
 * @brief Register a callback to be triggered when start is called
 *        on an a middleware bridge.
 *
 * Method to register a callback to be triggered when start is called
 * on a middleware bridge.
 *
 * @param[in] cb A mamaBridgeCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_registerStartCallback (mamaMiddlewareCb cb,
                                             void*            closure);
/**
 * @brief Register a callback to be triggered when stop is called
 *        on an a middleware bridge.
 *
 * Method to register a callback to be triggered when stop is called
 * on a middleware bridge.
 *
 * @param[in] cb A mamaBridgeCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_registerStopCallback (mamaMiddlewareCb cb,
                                            void*            closure);

/**
 * @brief Deregister a callback to be triggered when a new middleware 
 *        library is loaded.
 *
 * Method to deregister a callback to be triggered on a new middleware 
 * library load.
 *
 * @param[in] cb A mamaBridgeCb function pointer to be trigger on 
 *               load of a new middleware.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_deregisterLoadCallback (mamaMiddlewareCb cb);

/**
 * @brief Deregister a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * Method to deregister a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * @param[in] cb A mamaBridgeCb function pointer that was previously
 *               registered.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_deregisterUnloadCallback (mamaMiddlewareCb cb);

/**
 * @brief Deregister a callback to be triggered when start is called
 *        on an a middleware bridge.
 *
 * Method to deregister a callback to be triggered when start is called
 * on a middleware bridge.
 *
 * @param[in] cb A mamaBridgeCb function pointer that was previously
 *               registered. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_deregisterStartCallback (mamaMiddlewareCb cb);

/**
 * @brief Remove a callback to be triggered when stop is called
 *        on an a middleware bridge.
 *
 * Method to remove a callback to be triggered when stop is called
 * on a middleware bridge.
 *
 * @param[in] cb An existing mamaBridgeCb that was previosuly registered. 
 *
 * @return A mama_status indicating the success or failure of the deregistration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_deregisterStopCallback (mamaMiddlewareCb cb);

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
mamaMiddlewareManager_setProperty (const char* libraryName,
                                   const char* propertyName,
                                   const char* value);

/**
 * @brief Retrieve the name of a library.
 *
 * Method to retrieve the name of a library.
 *
 * @param[in] library A mamaBridge from which to get the name.
 *
 * @return A null terminated string of the name - do NOT free this..
 */
MAMAExpDLL
extern const char* 
mamaMiddlewareManager_getName (mamaBridge bridge);

/**
 * @brief Retrieve the Id of a library.
 *
 * Method to retrieve the Id of a library.
 *
 * @param[in] library A mamaBridge from which to get the name.
 *
 * @return A character representing the ID.
 */
MAMAExpDLL
extern char 
mamaMiddlewareManager_getId (mamaBridge bridge);

/**
 * @brief Retrieve the path from where a library was loaded.
 *
 * Method to retrieve the path from where a library was loaded.
 *
 * @param[in] library A mamaBridge.
 *
 * @return A null terminated string of the path - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaMiddlewareManager_getPath (mamaBridge bridge);

/**
 * @brief Get a string representation of a middleware Id.
 *
 * Method to get a string representation of a middleware Id.
 *
 * @param[in] middlewareId The Id to be converted to a string.
 * @param[out] str A pointer where the string representation will be put.
 *             Note: Do NOT free this memory.  
 *              
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_middlewareIdToString (char         middlewareId,
                                            const char** str);
/**
 * @brief Get the middleware Id from the string representation of the library name.
 *
 * Method to get the middleware Id from the library name.
 *
 * @param[in] str library name to be looked up to find the middlewareId.
 * @param[out] middlewareId Pointer to a character where the ID will be put.
 *             Note: Do NOT free this. 
 *
 * @return A mama_status indicating success or otherwise.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareManager_stringToMiddlewareId (const char*  str, 
                                            char*        middlewareId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaMiddlewareManagerH__ */

