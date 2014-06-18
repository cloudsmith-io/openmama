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

#include "librarymanager.h"
#include <wlock.h>

/*
 * Public implementation
 */

const char*
mamaLibrary_getName (mamaLibrary library)
{
    return (library ? library->mName : NULL);
}

mamaLibraryType
mamaLibrary_getType (mamaLibrary library)
{
    return (library ? library->mType : MAMA_UNKNOWN_LIBRARY);
}

const char*
mamaLibrary_getTypeName (mamaLibrary library)
{
    return (library ? library->mTypeName : NULL);
}

const char*
mamaLibrary_getPath (mamaLibrary library)
{
    return (library ? library->mPath : NULL);
}

void
mamaLibrary_lock (mamaLibrary library)
{
    if (library)
        wlock_lock (library->mLock);
}

void
mamaLibrary_unlock (mamaLibrary library)
{
    if (library)
        wlock_unlock (library->mLock);
}

