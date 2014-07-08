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

#ifndef MamaStatusH__
#define MamaStatusH__

#if defined(__cplusplus)
extern "C" {
#endif

#include "wombat/port.h"

typedef enum
{
    /* Status OK */
    MAMA_STATUS_OK                          = 0,
    /* Out of memory */
    MAMA_STATUS_NOMEM                       = 1,
    /* Messaging platform specific error */
    MAMA_STATUS_PLATFORM                    = 2,
    /* General system error */
    MAMA_STATUS_SYSTEM_ERROR                = 3,
    /* Invalid argument */
    MAMA_STATUS_INVALID_ARG                 = 4,
    /* Null argument */
    MAMA_STATUS_NULL_ARG                    = 5,
    /* Not found */
    MAMA_STATUS_NOT_FOUND                   = 6,
    /* Timer failure */
    MAMA_STATUS_TIMER_FAILURE               = 7,
    /* IP address not found */
    MAMA_STATUS_IP_NOT_FOUND                = 8,
    /* Timeout e.g. when subscribing to a symbol */
    MAMA_STATUS_TIMEOUT                     = 9,
    /* Not entitled to the symbol being subscribed to */
    MAMA_STATUS_NOT_ENTITLED                = 10,
    /* Property too long */
    MAMA_STATUS_PROPERTY_TOO_LONG           = 11,
    /* MD Not opened */
    MAMA_STATUS_MD_NOT_OPENED               = 12,
    /* Publish/subscribe not opened */
    MAMA_STATUS_PUB_SUB_NOT_OPENED          = 13,
    /* Entitlements not enabled */
    MAMA_STATUS_ENTITLEMENTS_NOT_ENABLED    = 14,
    /* Bad transport type */
    MAMA_STATUS_BAD_TRANSPORT_TYPE          = 15,
    /* Using unsupported I/O type */
    MAMA_STATUS_UNSUPPORTED_IO_TYPE         = 16,
    /* Too many dispatchers */
    MAMA_STATUS_TOO_MANY_DISPATCHERS        = 17,
    /* Not implemented */
    MAMA_STATUS_NOT_IMPLEMENTED             = 18,
    /* Wrong field type */
    MAMA_STATUS_WRONG_FIELD_TYPE            = 19,
    /* Invalid symbol */
    MAMA_STATUS_BAD_SYMBOL                  = 20,
    /* I/O error */
    MAMA_STATUS_IO_ERROR                    = 21,
    /* Not installed */
    MAMA_STATUS_NOT_INSTALLED               = 22,
    /* Conflation error */
    MAMA_STATUS_CONFLATE_ERROR              = 23,
    /* Event dispatch queue full */
    MAMA_STATUS_QUEUE_FULL                  = 24,
    /* End of event queue reached */
    MAMA_STATUS_QUEUE_END                   = 25,
    /* No bridge */
    MAMA_STATUS_NO_BRIDGE_IMPL              = 26,
    /* Invalid queue */
    MAMA_STATUS_INVALID_QUEUE               = 27,
     /* Not modifiable  */
    MAMA_STATUS_NOT_MODIFIABLE              = 28,
     /* Message Type DELETE  */
    MAMA_STATUS_DELETE                      = 29,
	/* Not permissioned for the subject */
    MAMA_STATUS_NOT_PERMISSIONED			= 4001,
    /* Subscription is in an invalid state. */
    MAMA_STATUS_SUBSCRIPTION_INVALID_STATE  = 5001,
    /* Queue has open objects. */
    MAMA_STATUS_QUEUE_OPEN_OBJECTS          = 5002,
    /* The function isn't supported for this type of subscription. */
    MAMA_STATUS_SUBSCRIPTION_INVALID_TYPE   = 5003,
    /* The underlying transport saw a gap. */
    MAMA_STATUS_SUBSCRIPTION_GAP            = 5004,
    /* A resource has not been initialised. */
    MAMA_STATUS_NOT_INITIALISED             = 5005,
    /* A symbol has no subscribers. */
    MAMA_STATUS_NO_SUBSCRIBERS              = 5006,
    /* The symbol has expired. */
    MAMA_STATUS_EXPIRED                     = 5007,
    /* The application's bandwidth limit has been exceeded. */
    MAMA_STATUS_BANDWIDTH_EXCEEDED          = 5008
} mama_status;

MAMAExpDLL 
extern const char*     
mamaStatus_stringForStatus (mama_status status);

#if defined(NDEBUG) && !defined(WITH_UNITTESTS)

#define NULLARG_STATUS_CHECK
#define NULLARG_STATUS_CHECK_STR

#else

#define NULLARG_STATUS_CHECK(x) \
    do { \
        if (!(x)) return MAMA_STATUS_NULL_ARG; \
	} while(0);

#define NULLARG_STATUS_CHECK_STR(x) \
    do { \
        if (!(x) || (strlen((x))==0) ) return MAMA_STATUS_NULL_ARG; \
    } while(0);

#endif

#define NOMEM_STATUS_CHECK(x) \
    do { \
        if ((x==NULL))  \
        {    \
            mama_log (MAMA_LOG_LEVEL_SEVERE, "Could not allocate memory");   \
            return MAMA_STATUS_NOMEM;      \
         } \
	} while(0);

#if defined(__cplusplus)
} /*extern "C" { */
#endif
#endif /* MamaStatusH__*/

