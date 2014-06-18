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

#include <mama/middleware.h>
#include <mama/middlewaremanager.h>
#include <middlewaremanager.h>

mamaMiddleware
mamaMiddleware_convertFromString (const char*  str)
{
    if (!str)
        return MAMA_MIDDLEWARE_UNKNOWN;

    char middlewareId  = '\0';
    mama_status status =
        mamaMiddlewareLibraryManager_stringToMiddlewareId(str, &middlewareId);  

    if (MAMA_STATUS_OK != status)
        return mamaMiddlewareLibraryManager_convertFromString(str);

    return middlewareId;
}

/* Returns lowercase for use in library and function names */
const char*
mamaMiddleware_convertToString (mamaMiddleware  middleware)
{
    const char* str    = NULL;
    mama_status status = 
        mamaMiddlewareLibraryManager_middlewareIdToString(middleware, &str);

    if (MAMA_STATUS_OK != status)
        return mamaMiddlewareLibraryManager_convertToString(middleware);

    return str;
}
