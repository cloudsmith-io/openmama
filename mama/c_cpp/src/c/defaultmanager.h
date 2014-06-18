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

#ifndef MamaDefaultLibraryManagerH__
#define MamaDefaultLibraryManagerH__

#include "librarymanager.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status
mamaDefaultLibraryManager_create (mamaLibraryTypeManager manager);

extern void
mamaDefaultLibraryManager_destroy (void);

extern mama_status
mamaDefaultLibraryManager_loadLibrary (mamaLibrary library);

extern void
mamaDefaultLibraryManager_unloadLibrary (mamaLibrary library);

extern mamaLibraryType
mamaDefaultLibraryManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib);

extern const char*
mamaDefaultLibraryManager_getLibraryProperty (mamaLibrary library,
                                              const char* property);

extern mama_bool_t
mamaDefaultLibraryManager_getLibraryBoolProperty (mamaLibrary library,
                                                  const char* property);

extern mama_bool_t
mamaDefaultLibraryManager_getLibraryIgnore (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryName (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryDescription (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryAuthor (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryUri (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryLicense (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryVersion (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryMamaVersion (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryBridgeAuthor (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryBridgeUri (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryBridgeLicense (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryBridgeVersion (mamaLibrary library);

extern const char*
mamaDefaultLibraryManager_getLibraryBridgeMamaVersion (mamaLibrary library);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaDefaultLibraryManagerH__ */

