/* $Id$
 *
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

#include "wombat/port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#include "platform.h"
#include "wombat/wincompat.h"

/**
 * Functions relating to DLLs/shared objects
 */
char errorBuf[25];
LIB_HANDLE openSharedLib (const char* libName, const char* path)
{
    char fileName[FILENAME_MAX];

    if (path)   
    {
        snprintf (fileName, FILENAME_MAX-1, "%s%slib%s%s", 
                  path, PATHSEP, libName, LIB_EXTENSION);
    }
    else
    {
        snprintf (fileName, FILENAME_MAX-1, "lib%s%s", 
                  libName, LIB_EXTENSION);
    }

    return dlopen (fileName, RTLD_NOW | RTLD_GLOBAL);
}

int closeSharedLib (LIB_HANDLE handle)
{
    return dlclose (handle);
}

void* loadLibFunc (LIB_HANDLE handle, const char* funcName)
{
    return dlsym (handle, funcName);  
}

char* getLibError (void)
{
    return dlerror();
}
