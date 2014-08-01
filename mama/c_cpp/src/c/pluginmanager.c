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

#include "pluginmanager.h"
#include "mama/pluginmanager.h"

#include <mama/mama.h>
#include <assert.h>
#include <platform.h>
#include <wombat/wInterlocked.h>

#include "plugin.h"
#include "librarymanager.h"

/*
 * Private types
 */

typedef struct mamaPluginManagerImpl* mamaPluginManager;

typedef struct mamaPluginLibraryImpl_
{
    mamaLibrary                 mParent;
    mamaPlugin                  mBridge;
    mamaPluginManager    mManager;
} mamaPluginLibraryImpl;

typedef struct mamaPluginManagerImpl
{
    mamaLibraryTypeManager mParent;
    mamaLibrary            mPlugins [MAMA_MAX_LIBRARIES];
} mamaPluginManagerImpl;

/*
 * Private declarations
 */

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaPluginManager_create (mamaLibraryTypeManager manager)
{
    return MAMA_STATUS_OK;
}

void
mamaPluginManager_destroy (void)
{
}

mama_status
mamaPluginManager_loadLibrary (mamaLibrary library)
{
    return MAMA_STATUS_OK;
}

mama_status
mamaPluginManager_unloadLib (mamaPluginLibrary library)
{
    return MAMA_STATUS_OK;
}

void
mamaPluginManager_unloadLibrary (mamaLibrary library)
{
}

mamaLibraryType
mamaPluginManager_classifyLibraryType (const char* libraryName,
                                              LIB_HANDLE  libraryLib)
{
#if 0
    if (strstr (libraryName, "plugin"))
    {
        *libraryType0 = MAMA_PLUGIN_LIBRARY;
        return MAMA_STATUS_OK;
    }
#endif

    return MAMA_UNKNOWN_LIBRARY;
}

void mamaPluginManager_dump (mamaLibraryTypeManager manager)
{
}

void mamaPluginManager_dumpLibrary (mamaLibrary library)
{
}

/*
 * Public implementation
 */

mama_status
mamaPluginManager_loadBridge (const char*  pluginName,
                                     const char*  path,
                                     mamaPlugin*  plugin)
{
    return MAMA_STATUS_OK;
}

mama_status
mamaPluginManager_setProperty (const char* libraryName,
                                      const char* propertyName,
                                      const char* value)
{
    return mamaLibraryManager_setProperty (libraryName,
                                           propertyName,
                                           value);
}

const char* 
mamaPluginManager_getName (mamaPluginLibrary library)
{
    return mamaLibraryManager_getName(library->mParent);
}

const char*
mamaPluginManager_getPath (mamaPluginLibrary library)
{
    return mamaLibraryManager_getPath(library->mParent);
}

