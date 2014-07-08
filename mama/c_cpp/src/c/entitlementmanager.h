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

#ifndef MamaEntitlementLibraryManagerH__
#define MamaEntitlementLibraryManagerH__

#include "librarymanager.h"
#include "entitlement.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status
mamaEntitlementLibraryManager_create (mamaLibraryTypeManager manager);

extern void
mamaEntitlementLibraryManager_destroy (void);

extern mama_status
mamaEntitlementLibraryManager_loadLibrary (mamaLibrary library);

extern void
mamaEntitlementLibraryManager_unloadLibrary (mamaLibrary library);

extern mamaLibraryType
mamaEntitlementLibraryManager_classifyLibraryType (const char* libraryName,
                                                   LIB_HANDLE  libraryLib);

extern void 
mamaEntitlementLibraryManager_dump (mamaLibraryTypeManager manager);

extern void 
mamaEntitlementLibraryManager_dumpLibrary (mamaLibrary library);

extern mama_status
mamaEntitlementLibraryManager_getEntitlementBridge (mamaEntitlementBridge* bridge);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaEntitlementLibraryManagerH__ */

