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

#ifndef MamaDefaultEntitlementBridgeH__
#define MamaDefaultEntitlementBridgeH__

#include "entitlement.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

extern mama_status 
defaultEntitlement_setup (void);

extern mama_status
defaultEntitlement_tearDown (void);

extern mama_status 
defaultEntitlement_createSubscription (mamaEntitlement entitlement);

extern mama_status
defaultEntitlement_deleteSubscription (mamaEntitlement entitlement);

extern mama_status
defaultEntitlement_setSubscriptionType (mamaEntitlement  entitlement,
                                        mamaServiceLevel level);

extern mama_status
defaultEntitlement_checkEntitledWithSubject (mamaEntitlement entitlement, 
                                             const char*     subject);
extern mama_status
defaultEntitlement_checkEntitledWithCode (mamaEntitlement entitlement, 
                                          int32_t         entitleCode);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
