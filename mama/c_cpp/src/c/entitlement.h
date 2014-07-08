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

#ifndef MamaEntitlementH__
#define MamaEntitlementH__

#include "mama/status.h"
#include "mama/types.h"
#include "mama/servicelevel.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct mamaEntitlementBridgeImpl_* mamaEntitlementBridge;
typedef struct mamaEntitlementImpl_*       mamaEntitlement;

/*===================================================================
 =             general bridge function pointers                 =
 ====================================================================*/
/*Called when loading/creating a bridge */
typedef mama_status 
(*Entitlement_load)    (void);

typedef mama_status 
(*Entitlement_unload)  (void);

/*===================================================================
 =             Entitlement specific function pointers               =
 ====================================================================*/
/*Called during start to initialise the entitlements server */
typedef mama_status 
(*Entitlement_setup)   (void);

/*Called during shut down to give the bridge a chance to clean up*/
typedef mama_status
(*Entitlement_tearDown) (void);

/*Creates a new subscription*/
typedef mama_status 
(*Entitlement_createSubscription) (mamaEntitlement entitlement);

/*Deletes a subscription*/
typedef mama_status
(*Entitlement_deleteSubscription) (mamaEntitlement entitlement);

/*Sets the subscription type*/
typedef mama_status
(*Entitlement_setSubscriptionType) (mamaEntitlement  entitlement,
                                    mamaServiceLevel level);

typedef mama_status
(*Entitlement_checkEntitledWithSubject) (mamaEntitlement entitlement, 
                                         const char*     subject);
typedef mama_status
(*Entitlement_checkEntitledWithCode) (mamaEntitlement entitlement, 
                                      int32_t         entitleCode);

typedef struct mamaEntitlementImpl_
{
    const char*  mSubject;         /*Subject associated with this entitlement*/
    int32_t      mEntitleCode;     /*Entitlement code associated with this subscription*/  
    mama_bool_t  mEntitlementAlreadyVerified; /*Has this entitlement already been checked?*/
    void*        mSubscription;    /*Generic pointer to subscription object*/ 
    void*        mClosure;         /*Generic pointer for any other information*/
}mamaEntitlementImpl;

typedef struct mamaEntitlementBridgeImpl_
{
    Entitlement_setup                    entitlementSetup;
    Entitlement_tearDown                 entitlementTearDown;
    Entitlement_createSubscription       entitlementCreateSubscription;
    Entitlement_deleteSubscription       entitlementDeleteSubscription;
    Entitlement_setSubscriptionType      entitlementSetSubscriptionType;
    Entitlement_checkEntitledWithSubject entitlementCheckEntitledWithSubject;
    Entitlement_checkEntitledWithCode    entitlementCheckEntitledWithCode;

    mamaEntitlementLibrary mLibrary;
}mamaEntitlementBridgeImpl; 

#if defined(__cplusplus)
}
#endif

#endif 
