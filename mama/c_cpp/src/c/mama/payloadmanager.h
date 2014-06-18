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

#include <mama/library.h>
#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief Load a specific payload bridge (if not already loaded).
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_loadBridge (const char* payloadName,
                                      const char* path,
                                      mamaPayloadBridge* bridge);

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
mamaPayloadLibraryManager_getBridge (const char*        payloadName,
                                     mamaPayloadBridge* bridge);

/*
 * @brief Retrieve the default (usually first loaded) payload bridge.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getDefaultBridge (mamaPayloadBridge* bridge);

/**
 * @brief Retrieve a specific payload bridge by id.
 *
 * @param[in]  payloadId The ID of the payload we want to retrieve.
 * @param[out] bridge A pointer to the mamaPayloadBridge structure where
 *             the payload bridge will be put, if found.
 *
 * @return A mama_status indicating the success or failure of the retrieval.
 */
MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getBridgeById (char               payloadId,
                                         mamaPayloadBridge* bridge);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getLibraries (mamaPayloadLibrary* libraries,
                                        mama_size_t*        size);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_getBridges (mamaPayloadBridge*  bridges,
                                      mama_size_t*        size);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_registerLoadCallback (mamaLibraryCb cb,
                                                void*         closure);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_registerUnloadCallback (mamaLibraryCb cb,
                                                  void*         closure);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_libraryToPayloadLibrary (
        mamaLibrary         library,
        mamaPayloadLibrary* plLibrary);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_payloadIdToString (char         payloadId,
                                             const char** str);

MAMAExpDLL
extern mama_status
mamaPayloadLibraryManager_stringToPayloadId (const char* str, 
                                             char*       payloadId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaPayloadManagerH__ */

