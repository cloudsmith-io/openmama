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

    char Id = 
        mamaMiddlewareManager_convertFromString (str);

    if (MAMA_MIDDLEWARE_UNKNOWN == Id)
    {
        mama_status status =
            mamaMiddlewareManager_stringToMiddlewareId (str, &Id);  

        if (MAMA_STATUS_OK != status)
            return MAMA_MIDDLEWARE_UNKNOWN; 
    }
    return Id;
}

const char*
mamaMiddleware_convertToString (mamaMiddleware  middleware)
{
    const char* str = 
        mamaMiddlewareManager_convertToString (middleware);

    if (NULL == str)
    {
        mama_status status = 
            mamaMiddlewareManager_middlewareIdToString (middleware, &str);

        if (MAMA_STATUS_OK != status)
            return NULL;
    }
    return str;
}
