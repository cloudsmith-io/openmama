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

#include <mama/library.h>
#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief Load a specific middleware bridge (if not already loaded).
 *
 * Method to locate a shared library of a specific name and try 
 * and it and its functions as a middleware library.
 *
 * @param[in]  middlewareName The name of the middleware we want to load.
 * @param[in]  path The path to the location of the library if known,
               otherwise NULL.
 * @param[out] library A pointer to a mamaMiddlewareLibrary structure where
 *             the middleware bridge will be put, if found.
 * 
 *@return A mama_status indicating the success or failure of the load.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_loadLibraryWithPath (const char* middlewareName,
                                                  const char* path,
                                                  mamaMiddlewareLibrary* library);

/**
 * @brief Retrieve a specific middleware library.
 *
 * Method to return a specific middleware library based on name.
 *
 * @param[in]  middlewareName The name of the payload we want to retrieve.
 * @param[out] library A pointer to the mamaMiddlewareLibrary structure where
 *             the middleware library will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getLibrary (const char* middlewareName,
                                         mamaMiddlewareLibrary* library);

/*
 * @brief Retrieve the default (usually first loaded) middleware library.
 *
 * Method to return the default middleware library.
 *
 * @param[out] library A pointer to the mamaMiddlewareLibrary structure where
 *                     the payload library will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getDefaultLibrary (mamaMiddlewareLibrary* library);

/**
 * @brief Retrieve all loaded middleware libraries.
 *
 * Method to get all the loaded payload libraries.
 * 
 * @param[out] libraries A pointer to an array of mamaMiddlewareLibrary structures
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
mamaMiddlewareLibraryManager_getLibraries (mamaMiddlewareLibrary* libraries,
                                           mama_size_t*           size);

/**
 * @brief Retrieve all loaded middleware libraries, which represent open bridges.
 *
 * Method to retrieve all libraries representing open middleware bridges.
 *
 * @param[out] libraries A pointer to an array of mamaMiddlewareLibrary structures
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
mamaMiddlewareLibraryManager_getOpenedBridges (mamaMiddlewareLibrary* libraries,
                                               mama_size_t*           size);
/**
 * @brief Retrieve all loaded middleware libraries, which represent active bridges.
 *
 * Method to retrieve all libraries representing active middleware bridges.
 *
 * @param[out] libraries A pointer to an array of mamaMiddlewareLibrary structures
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
mamaMiddlewareLibraryManager_getActiveBridges (mamaMiddlewareLibrary* libraries,
                                               mama_size_t*           size);

/**
 * @brief Retrieve all loaded middleware libraries, which represent inactive bridges.
 *
 * Method to retrieve all libraries representing inactive middleware bridges.
 *
 * @param[out] libraries A pointer to an array of mamaMiddlewareLibrary structures
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
mamaMiddlewareLibraryManager_getInactiveBridges (mamaMiddlewareLibrary* libraries,
                                                 mama_size_t*           size);

/**
 * @brief Retrieve all loaded middleware libraries, which represent closed bridges.
 *
 * Method to retrieve all libraries representing closed middleware bridges.
 *
 * @param[out] libraries A pointer to an array of mamaMiddlewareLibrary structures
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
mamaMiddlewareLibraryManager_getClosedBridges (mamaMiddlewareLibrary* libraries,
                                               mama_size_t*           size);
/**
 * @brief Get the number of middleware libraries loaded.
 *
 * Method to retrieve the number of loaded middleware libraries.
 *
 * @param[out] size A pointer to a mama_size_t where we will put the number
 *                  of libraries. 
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumLibraries (mama_size_t* size);

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
mamaMiddlewareLibraryManager_getNumOpenedBridges (mama_size_t* size);

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
mamaMiddlewareLibraryManager_getNumActiveBridges (mama_size_t* size);

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
mamaMiddlewareLibraryManager_getNumInactiveBridges (mama_size_t* size);

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
mamaMiddlewareLibraryManager_getNumClosedBridges (mama_size_t* size);

/**
 * @brief Open the bridge represent by the mamaMiddlewareLibrary handle.
 *
 * Method to open the bridge associated with the provided mamaMiddlewareLibrary,
 * defers the opening to the middleware library's specific implementation of the 
 * open function.
 *
 * @param[in] library A mamaMiddlewareLibrary handle to the bridge to be opened.
 *
 * @return A mama_status indicating the success or failure of the bridge open.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_openBridge (mamaMiddlewareLibrary library);

/**
 * @brief Close the bridge represent by the mamaMiddlewareLibrary handle.
 *
 * Method to close the bridge associated with the provided mamaMiddlewareLibrary,
 * defers the closing to the middleware library's specific implementation of the 
 * close function.
 *
 * @param[in] library A mamaMiddlewareLibrary handle to the bridge to be closed.
 *
 * @return A mama_status indicating the success or failure of the bridge close.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_closeBridge (mamaMiddlewareLibrary library);

/**
 * @brief Start the middleware bridge represent by the mamaMiddlewareLibrary handle.
 *
 * Method to start the middleware bridge associated with the provided 
 * mamaMiddlewareLibrary, defers the start to the middleware library's 
 * specific implementation of the close function.
 *
 * @param[in] library A mamaMiddlewareLibrary handle to the middleware bridge 
 *                    to be started.
 *
 * @return A mama_status indicating the success or failure of the bridge start.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_startBridge (mamaMiddlewareLibrary library);

/**
 * @brief Stop the middleware bridge represent by the mamaMiddlewareLibrary handle.
 *
 * Method to stop the middleware bridge associated with the provided 
 * mamaMiddlewareLibrary, defers the start to the middleware library's 
 * specific implementation of the close function.
 *
 * @param[in] library A mamaMiddlewareLibrary handle to the middleware bridge 
 *                    to be stopped.
 *
 * @return A mama_status indicating the success or failure of the bridge stop.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_stopBridge (mamaMiddlewareLibrary library);

/**
 * @brief Stops all middleware bridges.
 *
 * Method to stop all the middleware bridges. 
 *
 * @return A mama_status indicating the success or failure of the bridge opening.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_stopAllBridges (void);

/**
 * @brief Gets a string representing the library version.
 *
 * Method to return a string library version.
 *
 * @param[in] library A mamaMiddlewareLibrary pointer of which to 
 *                    get the version.
 *
 * @return A const char* pointing to the library version - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaMiddlewareLibraryManager_getLibraryVersion (mamaMiddlewareLibrary library);

/**
 * @brief Gets a string representing the minimum supported MAMA version 
 *        of the library.
 *
 * Method to return a string representing the minimum supported MAMA 
 * version for the library.
 *
 * @param[in] library A mamaMiddlewareLibrary pointer of which to 
 *                    get the MAMA version.
 *
 * @return A const char* pointing to the MAMA version - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaMiddlewareLibraryManager_getLibraryMamaVersion (mamaMiddlewareLibrary library);

/**
 * @brief Register a callback to be triggered when a new middleware 
 *        library is loaded.
 *
 * Method to register a callback to be triggered on a new middleware 
 * library load.
 *
 * @param[in] cb A mamaLibraryCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerLoadCallback (mamaLibraryCb cb,
                                                   void*         closure);

/**
 * @brief Register a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * Method to register a callback to be triggered when a middleware 
 * library is unloaded.
 *
 * @param[in] cb A mamaLibraryCb function pointer to be trigger on 
 *               unload of a middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerUnloadCallback (mamaLibraryCb cb,
                                                     void*         closure);
/**
 * @brief Register a callback to be triggered when start is called
 *        on an a middleware bridge.
 *
 * Method to register a callback to be triggered when start is called
 * on a middleware bridge.
 *
 * @param[in] cb A mamaLibraryCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerStartCallback (mamaLibraryCb cb,
                                                    void*         closure);

/**
 * @brief Register a callback to be triggered when stop is called
 *        on an a middleware bridge.
 *
 * Method to register a callback to be triggered when stop is called
 * on a middleware bridge.
 *
 * @param[in] cb A mamaLibraryCb function pointer to be trigger on 
 *               load of a new middleware.
 * @param[in] closure Data to pass to the callback function. 
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerStopCallback (mamaLibraryCb cb,
                                                   void*         closure);

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
mamaMiddlewareLibraryManager_setProperty (const char* libraryName,
                                          const char* propertyName,
                                          const char* value);
/**
 * @brief Convert a mamaLibrary to a mamaMiddlewareLibrary.
 *
 * Method to convert a mamaLibrary to a mamaMiddlewareLibrary.
 *
 * @param[in] library A mamaLibrary to be converted to a mamaMiddlewareLibrary.
 * @param[out] plLibrary Pointer to mamaMiddlewareLibrary where the converted 
 *             library should be put. 
 *
 * @return A mama_status indicating the success of the conversion.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_libraryToMiddlewareLibrary (
        mamaLibrary            library,
        mamaMiddlewareLibrary* mwLibrary);

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
mamaMiddlewareLibraryManager_middlewareIdToString (char         middlewareId,
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
mamaMiddlewareLibraryManager_stringToMiddlewareId (const char*  str, 
                                                   char*        middlewareId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaMiddlewareManagerH__ */

