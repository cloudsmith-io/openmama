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
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_loadBridge (const char* middlewareName,
                                         const char* path,
                                         mamaBridge* bridge);

/**
 * @brief Retrieve a specific middleware bridge.
 *
 * @param middlewareName The name of the middleware we want to retrieve.
 * @param[out] bridge A pointer to the mamaBridge structure where
 *                    the middleware bridge will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getBridge (const char* middlewareName,
                                        mamaBridge* bridge);

/*
 * @brief Retrieve the default (usually first loaded) middleware bridge.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getDefaultBridge (mamaBridge* bridge);

/**
 * FIXME:
 * @brief Method to return all middleware.
 *
 * @param[out] middlewares A pointer for an array of mamaBridges
 *                         where we allocate memory to hold the returned
 *                         mamaBridge structures.
 * @param[out] size A pointer mama_size_t where the number of bridges
 *                  will be put.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getLibraries (mamaMiddlewareLibrary* libraries,
                                           mama_size_t*           size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getBridges (mamaBridge*  bridges,
                                         mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getOpenedBridges (mamaBridge*  bridges,
                                               mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getActiveBridges (mamaBridge*  bridges,
                                               mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getInactiveBridges (mamaBridge*  bridges,
                                                 mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getClosedBridges (mamaBridge*  bridges,
                                               mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumBridges (mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumOpenedBridges (mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumActiveBridges (mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumInactiveBridges (mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_getNumClosedBridges (mama_size_t* size);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_openBridge (mamaBridge bridge);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_closeBridge (mamaBridge bridge);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_startBridge (mamaBridge bridge);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_stopBridge (mamaBridge bridge);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_stopAllBridges (void);

MAMAExpDLL
extern const char*
mamaMiddlewareLibraryManager_getBridgeVersion (mamaBridge bridge);

MAMAExpDLL
extern const char*
mamaMiddlewareLibraryManager_getBridgeMamaVersion (mamaBridge bridge);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerLoadCallback (mamaLibraryCb cb,
                                                   void*         closure);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerUnloadCallback (mamaLibraryCb cb,
                                                     void*         closure);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerStartCallback (mamaLibraryCb cb,
                                                    void*         closure);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_registerStopCallback (mamaLibraryCb cb,
                                                   void*         closure);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_libraryToMiddlewareLibrary (
        mamaLibrary            library,
        mamaMiddlewareLibrary* mwLibrary);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_middlewareIdToString (char         middlewareId,
                                                   const char** str);

MAMAExpDLL
extern mama_status
mamaMiddlewareLibraryManager_stringToMiddlewareId (const char*  str, 
                                                   char*        middlewareId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaMiddlewareManagerH__ */

