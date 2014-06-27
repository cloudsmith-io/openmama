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

#ifndef MamaPayloadLibraryManagerH__
#define MamaPayloadLibraryManagerH__

#include <mama/types.h>
#include "librarymanager.h"
#include <mama/msg.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status
mamaPayloadLibraryManager_create (mamaLibraryTypeManager manager);

extern void
mamaPayloadLibraryManager_destroy (void);

extern mama_status
mamaPayloadLibraryManager_loadLibrary (mamaLibrary library);

extern void
mamaPayloadLibraryManager_unloadLibrary (mamaLibrary library);

extern mamaLibraryType
mamaPayloadLibraryManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib);

extern mama_bool_t
mamaPayloadLibraryManager_forwardCallback (mamaLibraryCb cb, 
                                           mamaLibrary   library, 
                                           void*         closure);

extern void 
mamaPayloadLibraryManager_dump (mamaLibraryTypeManager manager);

extern void 
mamaPayloadLibraryManager_dumpLibrary (mamaLibrary library);

extern mamaPayloadBridge
mamaPayloadLibraryManager_findPayload (char id);

extern mamaPayloadBridge
mamaPayloadLibraryManager_getDefaultPayload (void);

extern void
mamaPayloadLibraryManager_setDefaultPayload (mamaPayloadLibrary library);

extern mama_status
mamaPayloadLibraryManager_activateLibrary (mamaPayloadLibrary library);

/*Deprecated function for converting the payload enum to a string representation*/
extern const char*
mamaPayloadLibraryManager_convertToString (mamaPayloadType payloadType);

/*Tempory method to convert payload library to payload bridge*/
extern mama_status
mamaPayloadLibraryManager_convertLibraryToPayload (mamaPayloadLibrary library,
                                                   mamaPayloadBridge* bridge);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaPayloadLibraryManagerH__ */

