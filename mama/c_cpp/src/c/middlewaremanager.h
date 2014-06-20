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

#ifndef MamaMiddlewareLibraryManagerH__
#define MamaMiddlewareLibraryManagerH__

#include "librarymanager.h"
#include <mama/middleware.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status
mamaMiddlewareLibraryManager_create (mamaLibraryTypeManager manager);

extern void
mamaMiddlewareLibraryManager_destroy (void);

extern mama_status
mamaMiddlewareLibraryManager_loadLibrary (mamaLibrary library);

extern void
mamaMiddlewareLibraryManager_unloadLibrary (mamaLibrary library);

extern mamaLibraryType
mamaMiddlewareLibraryManager_classifyLibraryType (const char* libraryName,
                                                  LIB_HANDLE  libraryLib);

/*Deprecated convert from middleware name to mamaMiddleware enum string*/
extern mamaMiddleware
mamaMiddlewareLibraryManager_convertFromString (const char*  str);

/*Deprecated convert from mamaMiddleware to string representation of middleware name*/
extern const char*
mamaMiddlewareLibraryManager_convertToString (mamaMiddleware middleware);

/* FIXME - these are just temporary functions to allow us to still use functions taking mamaBridge
 * which should be converted to mamaMiddlewareLibrary functions*/

extern mama_status
mamaMiddlewareLibraryManager_convertLibraryToBridge (mamaMiddlewareLibrary library, 
                                                     mamaBridge*           bridge);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaMiddlewareLibraryManagerH__ */

