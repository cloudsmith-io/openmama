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

#ifndef MamaLibraryH__
#define MamaLibraryH__

#include "mama/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Maximum number of same-type libraries that can be loaded */
#define MAX_LIBRARIES 256  

/**
 * Enum representing the different types of libraries which may be loaded by
 * OpenMAMA.
 */
typedef enum mamaLibraryType_ {
    MAMA_MIDDLEWARE_LIBRARY,
    MAMA_PAYLOAD_LIBRARY,
    MAMA_PLUGIN_LIBRARY,
    MAMA_UNKNOWN_LIBRARY,
    MAX_LIBRARY_TYPE = MAMA_UNKNOWN_LIBRARY
} mamaLibraryType;

typedef mama_bool_t
(*mamaLibraryCb) (mamaLibrary library,
                  void*       closure);

MAMAExpDLL
extern const char* 
mamaLibrary_getName (mamaLibrary library);

MAMAExpDLL
extern mamaLibraryType
mamaLibrary_getType (mamaLibrary library);

MAMAExpDLL
extern const char*
mamaLibrary_getTypeName (mamaLibrary library);

MAMAExpDLL
extern const char*
mamaLibrary_getPath (mamaLibrary library);

/* FIXME: These should be internal only */
MAMAExpDLL
extern void
mamaLibrary_lock (mamaLibrary library);

MAMAExpDLL
extern void
mamaLibrary_unlock (mamaLibrary library);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaLibraryH__ */

