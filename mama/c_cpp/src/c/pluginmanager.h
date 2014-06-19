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

#ifndef MamaPluginLibraryManagerH__
#define MamaPluginLibraryManagerH__

#include "librarymanager.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status
mamaPluginLibraryManager_create (mamaLibraryTypeManager manager);

extern void
mamaPluginLibraryManager_destroy (void);

extern mama_status
mamaPluginLibraryManager_loadLibrary (mamaLibrary library);

extern void
mamaPluginLibraryManager_unloadLibrary (mamaLibrary library);

extern mamaLibraryType
mamaPluginLibraryManager_classifyLibraryType (const char* libraryName,
                                              LIB_HANDLE  libraryLib);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaPluginLibraryManagerH__ */
