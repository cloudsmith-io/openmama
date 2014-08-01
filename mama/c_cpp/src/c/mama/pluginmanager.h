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

#ifndef MamaPluginManagerH__
#define MamaPluginManagerH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief Load a specific plugin bridge (if not already loaded).
 */
MAMAExpDLL
extern mama_status
mamaPluginManager_loadBridge (const char* pluginName,
                              const char* path,
                              mamaPlugin* plugin);
/**
 * @brief Unload a specific plugin library.
 *
 * Method to unload a specific plugin library. 
 *
 * @param[in] library The library to be unloaded.
 * 
 *@return A mama_status indicating the success or failure of the unload.
 */
MAMAExpDLL
extern mama_status
mamaPluginManager_unloadLib (mamaPluginLibrary library);

/**
 * @brief Retrieve the name of a library.
 *
 * Method to retrieve the name of a library.
 *
 * @param[in] library A mamaPluginLibrary from which to get the name.
 *
 * @return A null terminated string of the name - do NOT free this..
 */
MAMAExpDLL
extern const char* 
mamaPluginManager_getName (mamaPluginLibrary library);

/**
 * @brief Retrieve the path from where a library was loaded.
 *
 * Method to retrieve the path from where a library was loaded.
 *
 * @param[in] library A mamaPluginLibrary.
 *
 * @return A null terminated string of the path - do NOT free this.
 */
MAMAExpDLL
extern const char*
mamaPluginManager_getPath (mamaPluginLibrary library);

/*
 * @brief Set a property programmatically for a specific library.
 * 
 * Method to set a library property programmatically.
 *
 * @param[in] libraryName Name of the library for which 
              the property will be set.
 * @param[in] propertyName Property name e.g. ignore for 
 *            mama.library.<library-name>.ignore.
 * @param[in] value Value of the property
 */

MAMAExpDLL
extern mama_status
mamaPluginManager_setProperty (const char* libraryName,
                               const char* propertyName,
                               const char* value);


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif

