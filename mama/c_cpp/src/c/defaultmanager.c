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

#define MAX_PROPERTY_LEN 300

/*
 * Private implementation
 */

static const char*
mamaDefaultLibraryManagerImpl_getProperty (mamaLibrary library,
                                           const char* property)
{
    /* Delegate back to type manager to retrieve text-based property. */
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, property);
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaDefaultLibraryManager_create (mamaLibraryTypeManager manager)
{
    return MAMA_STATUS_OK;
}

void
mamaDefaultLibraryManager_destroy (void)
{
}

void
mamaDefaultLibraryManager_dump (mamaLibraryTypeManager manager)
{
}

void
mamaDefaultLibraryManager_dumpLibrary (mamaLibrary library)
{
}

mama_status
mamaDefaultLibraryManager_loadLibrary (mamaLibrary library)
{
    return MAMA_STATUS_OK;
}

void
mamaDefaultLibraryManager_unloadLibrary (mamaLibrary library)
{
}

mamaLibraryType
mamaDefaultLibraryManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib)
{
    return MAMA_UNKNOWN_LIBRARY;
}

const char*
mamaDefaultLibraryManager_getLibraryProperty (mamaLibrary library,
                                              const char* property)
{
    return mamaLibraryManager_getProperty (library->mName, property,
                                           library->mManager->mType);
}

mama_bool_t
mamaDefaultLibraryManager_getLibraryBoolProperty (mamaLibrary library,
                                                  const char* property)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryProperty (library, property);

    if (!prop)
        return 0;
        
    return strtobool (prop);
}

mama_bool_t
mamaDefaultLibraryManager_getLibraryIgnore (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryBoolProperty (library, "ignore");
}

const char*
mamaDefaultLibraryManager_getLibraryName (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "name");
}

const char*
mamaDefaultLibraryManager_getLibraryDescription (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "description");
}

const char*
mamaDefaultLibraryManager_getLibraryAuthor (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "author");
}

const char*
mamaDefaultLibraryManager_getLibraryUri (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "uri");
}

const char*
mamaDefaultLibraryManager_getLibraryLicense (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "license");
}

const char*
mamaDefaultLibraryManager_getLibraryVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "version");
}

const char*
mamaDefaultLibraryManager_getLibraryMamaVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    return manager->mFuncs->getLibraryProperty (library, "mama_version");
}

const char*
mamaDefaultLibraryManager_getLibraryBridgeAuthor (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryProperty (library, "bridge_author");

    return (prop ? prop : manager->mFuncs->getLibraryAuthor (library));
}

const char*
mamaDefaultLibraryManager_getLibraryBridgeUri (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryProperty (library, "bridge_uri");

    return (prop ? prop : manager->mFuncs->getLibraryUri (library));
}

const char*
mamaDefaultLibraryManager_getLibraryBridgeLicense (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
       manager->mFuncs->getLibraryProperty (library, "bridge_license");

    return (prop ? prop : manager->mFuncs->getLibraryLicense (library));
}

const char*
mamaDefaultLibraryManager_getLibraryBridgeVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryProperty (library, "bridge_version");

    return (prop ? prop : manager->mFuncs->getLibraryVersion (library));
}

const char*
mamaDefaultLibraryManager_getLibraryBridgeMamaVersion (mamaLibrary library)
{
    mamaLibraryTypeManager manager = library->mManager;
    const char* prop =
        manager->mFuncs->getLibraryProperty (library, "bridge_mama_version");

    return (prop ? prop : manager->mFuncs->getLibraryMamaVersion (library));
}
