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

#include "defaultmanager.h"
#include <wombat/strutils.h>

/*
 * Private implementation
 */

static const char*
mamaDefaultManagerImpl_getProperty (mamaLibrary library,
                                           const char* property)
{
    /* Delegate back to type manager to retrieve text-based property. */
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, property);
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaDefaultManager_create (mamaLibraryTypeManager manager)
{
    return MAMA_STATUS_OK;
}

void
mamaDefaultManager_destroy (void)
{
}

void
mamaDefaultManager_dump (mamaLibraryTypeManager manager)
{
}

void
mamaDefaultManager_dumpLibrary (mamaLibrary library)
{
}

mama_status
mamaDefaultManager_loadLibrary (mamaLibrary library)
{
    return MAMA_STATUS_OK;
}

void
mamaDefaultManager_unloadLibrary (mamaLibrary library)
{
}

mamaLibraryType
mamaDefaultManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib)
{
    return MAMA_UNKNOWN_LIBRARY;
}

mama_bool_t
mamaDefaultManager_forwardCallback (mamaLibraryCb cb,
                                           mamaLibrary   library,
                                           void*         closure)
{
    return 0;
}

const char*
mamaDefaultManager_getLibraryStringProperty (mamaLibrary library,
                                                    const char* property)
{
    return mamaLibraryManager_getProperty (library->mName, property,
                                           library->mManager->mType);
}

mama_bool_t
mamaDefaultManager_getLibraryBoolProperty (mamaLibrary library,
                                                  const char* property)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryStringProperty (library, property);

    if (!prop)
        return 0;
        
    return strtobool (prop);
}

mama_bool_t
mamaDefaultManager_getLibraryIgnore (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryBoolProperty (library, "ignore");
}

const char*
mamaDefaultManager_getLibraryName (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "name");
}

const char*
mamaDefaultManager_getLibraryDescription (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "description");
}

const char*
mamaDefaultManager_getLibraryAuthor (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "author");
}

const char*
mamaDefaultManager_getLibraryUri (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "uri");
}

const char*
mamaDefaultManager_getLibraryLicense (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "license");
}

const char*
mamaDefaultManager_getLibraryVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "version");
}

const char*
mamaDefaultManager_getLibraryMamaVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryStringProperty (library, "mama_version");
}

const char*
mamaDefaultManager_getLibraryBridgeAuthor (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryStringProperty (library, "bridge_author");

    return (prop ? prop : manager->mFuncs->getLibraryAuthor (library));
}

const char*
mamaDefaultManager_getLibraryBridgeUri (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryStringProperty (library, "bridge_uri");

    return (prop ? prop : manager->mFuncs->getLibraryUri (library));
}

const char*
mamaDefaultManager_getLibraryBridgeLicense (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
       manager->mFuncs->getLibraryStringProperty (library, "bridge_license");

    return (prop ? prop : manager->mFuncs->getLibraryLicense (library));
}

const char*
mamaDefaultManager_getLibraryBridgeVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryStringProperty (library, "bridge_version");

    return (prop ? prop : manager->mFuncs->getLibraryVersion (library));
}

const char*
mamaDefaultManager_getLibraryBridgeMamaVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryStringProperty (library, "bridge_mama_version");

    return (prop ? prop : manager->mFuncs->getLibraryMamaVersion (library));
}
