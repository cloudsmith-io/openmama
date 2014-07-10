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

#include <mama/status.h>
#include <mama/mama.h>
#include "assert.h"
#include "oeaentitlementbridge.h"
#include "oeautil.h"

#include <OeaClient.h>
#include <OeaSubscription.h>

#define SERVERS_PROPERTY "entitlement.servers"
#define MAX_ENTITLEMENT_SERVERS 32
#define MAX_USER_NAME_STR_LEN 64
#define MAX_HOST_NAME_STR_LEN 64

oeaClient *               gEntitlementClient = NULL;
mamaEntitlementCallbacks  gEntitlementCallbacks;
static const char*        gServerProperty     = NULL;
static const char*        gServers[MAX_ENTITLEMENT_SERVERS];

#if (OEA_MAJVERSION == 2 && OEA_MINVERSION < 11) || OEA_MAJVERSION < 2
#undef MAMACALLTYPE
#define MAMACALLTYPE
#endif

void MAMACALLTYPE 
entitlementDisconnectCallback (oeaClient*                  client,
                               const OEA_DISCONNECT_REASON reason,
                               const char * const          userId,
                               const char * const          host,
                               const char * const          appName);

void MAMACALLTYPE 
entitlementUpdatedCallback (oeaClient* client,
                            int        forbidden);

void MAMACALLTYPE 
entitlementCheckingSwitchCallback (oeaClient* client,
                                   int        entitlementsDisabled);

mama_status
parseServersProperty (void);

void MAMACALLTYPE 
entitlementDisconnectCallback (oeaClient*                  client,
                               const OEA_DISCONNECT_REASON reason,
                               const char * const          userId,
                               const char * const          host,
                               const char * const          appName)

{
    if (gEntitlementCallbacks.onSessionDisconnect != NULL)
    {
        gEntitlementCallbacks.onSessionDisconnect (reason, userId, host, appName);
    }
}


void MAMACALLTYPE 
entitlementUpdatedCallback (oeaClient* client,
                            int        forbidden)
{
    if (gEntitlementCallbacks.onEntitlementUpdate != NULL)
    {
        gEntitlementCallbacks.onEntitlementUpdate();
    }
}

void MAMACALLTYPE 
entitlementCheckingSwitchCallback (oeaClient* client,
                                   int        entitlementsDisabled)
{
    if (gEntitlementCallbacks.onEntitlementCheckingSwitch != NULL)
    {
        gEntitlementCallbacks.onEntitlementCheckingSwitch(entitlementsDisabled);
    }
}

mama_status
parseServersProperty()
{
    char *ptr;
    int idx = 0;

    if (gServerProperty == NULL)
    {
        memset (gServers, 0, sizeof(gServers)*MAX_ENTITLEMENT_SERVERS);

        const char* server = mama_getProperty (SERVERS_PROPERTY);
        if(NULL == server)
        {
            mama_log( MAMA_LOG_LEVEL_WARN,
                      "Failed to open properties file "
                      "or no entitlement.servers property." );
            return MAMA_STATUS_NOT_ENTITLED;
        }

        gServerProperty = strdup (server);

        mama_log (MAMA_LOG_LEVEL_NORMAL,
                  "entitlement.servers=%s", gServerProperty);

        while( idx < MAX_ENTITLEMENT_SERVERS - 1 )
        {
            gServers[idx] = strtok_r (idx == 0 ? (char *)gServerProperty : NULL
                                      , ",",
                                      &ptr);


            if (gServers[idx++] == NULL) /* last server parsed */
            {
                break;
            }

            mama_log (MAMA_LOG_LEVEL_NORMAL,
                      "Parsed entitlement server: %s",
                      gServers[idx-1]);
        }
    }
    return MAMA_STATUS_OK;
}

mama_status 
oeaEntitlement_load ()
{
    return MAMA_STATUS_OK;
}

mama_status
oeaEntitlement_unload ()
{
    return MAMA_STATUS_OK;
}

mama_status 
oeaEntitlement_setup ()
{    
    if (NULL != gEntitlementClient)
    {
        oeaClient_destroy (gEntitlementClient);
        gEntitlementClient = NULL;
    }

    mama_status status = MAMA_STATUS_OK;
    if (NULL == gServers[0])
    {
        status = parseServersProperty();
        if (MAMA_STATUS_OK != status)
            return MAMA_STATUS_OK;
    }

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "Attempting to connect to entitlement server");

    const char* portLowStr  = 
        mama_getProperty ("mama.entitlement.portlow");

    int portLow = 8000;
    if (NULL != portLowStr)
        portLow  = atoi (portLowStr);

    const char* portHighStr = 
        mama_getProperty ("mama.entitlement.porthigh");

    int portHigh = 8001;
    if (NULL != portHighStr)
        portHigh = atoi (portHighStr);

    const char* altUserId   = 
        mama_getProperty ("mama.entitlement.altuserid");

    const char* site = 
        mama_getProperty ("mama.entitlement.site");

    const char* altIp = 
        mama_getProperty ("mama.entitlement.effective_ip_address");

    oeaCallbacks entitlementCallbacks;
    entitlementCallbacks.onDisconnect                 = entitlementDisconnectCallback;
    entitlementCallbacks.onEntitlementsUpdated        = entitlementUpdatedCallback;
    entitlementCallbacks.onSwitchEntitlementsChecking = entitlementCheckingSwitchCallback;

    int size = 0; 
    while (gServers[size])
        size++;

    oeaStatus   entitlementStatus = OEA_STATUS_OK;

    gEntitlementClient = oeaClient_create (&entitlementStatus,
                                site,
                                portLow,
                                portHigh,
                                gServers,
                                size);

    if (entitlementStatus != OEA_STATUS_OK)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Error creating OEA entitlement " 
            "client status %s", oeaStatusToStr (entitlementStatus));
        return MAMA_STATUS_NOT_ENTITLED;
    }

    if (NULL != gEntitlementClient)
    {
        entitlementStatus = oeaClient_setCallbacks (gEntitlementClient, 
                                                    &entitlementCallbacks);

        if (OEA_STATUS_OK != entitlementStatus)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "Error setting callbacks for OEA " 
                    "client. Status %s", oeaStatusToStr (entitlementStatus));
            return MAMA_STATUS_NOT_ENTITLED;
        }

        entitlementStatus = oeaClient_setAlternativeUserId (gEntitlementClient, 
                                                            altUserId);

        if (OEA_STATUS_OK != entitlementStatus)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "Error setting alternative userID "
                "for OEA client. Status %s", oeaStatusToStr (entitlementStatus));
            return MAMA_STATUS_NOT_ENTITLED;
        }

        entitlementStatus = oeaClient_setEffectiveIpAddress (gEntitlementClient, 
                                                             altIp);

        if (OEA_STATUS_OK != entitlementStatus)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "Error setting effective IP address for "
                "OEA client. Status %s", oeaStatusToStr (entitlementStatus));
            return MAMA_STATUS_NOT_ENTITLED;
        }

        const char* name = NULL;
        status = mama_getApplicationName (&name);

        entitlementStatus = oeaClient_setApplicationId (gEntitlementClient, 
                                                        name);

        if (OEA_STATUS_OK != entitlementStatus)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "Error setting application ID for "
                "OEA client. Status %s", oeaStatusToStr (entitlementStatus));
            return MAMA_STATUS_NOT_ENTITLED;
        }

        entitlementStatus = oeaClient_downloadEntitlements (
                            (oeaClient*const)gEntitlementClient);

        if (OEA_STATUS_OK != entitlementStatus)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "Error downloading entitlements "
                "for OEA client. Status %s", oeaStatusToStr (entitlementStatus));
            return MAMA_STATUS_NOT_ENTITLED;
        }
    }
    return MAMA_STATUS_OK;
}

mama_status
oeaEntitlement_tearDown ()
{
    oeaClient_destroy (gEntitlementClient);
    gEntitlementClient = NULL;
    free ((void*)gServerProperty);
    return MAMA_STATUS_OK;
}

mama_status 
oeaEntitlement_createSubscription (mamaEntitlement entitlement)
{
    assert (entitlement);
    
    oeaStatus   entitlementStatus = OEA_STATUS_OK;

    entitlement->mSubscription = (void*)oeaClient_newSubscription (
                                    &entitlementStatus, gEntitlementClient);
 
    if (OEA_STATUS_OK != entitlementStatus)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Error cannot create subscription "
            "- status %s", oeaStatusToStr (entitlementStatus));
        return MAMA_STATUS_NOT_ENTITLED;
    }

    return MAMA_STATUS_OK;
}

mama_status
oeaEntitlement_deleteSubscription (mamaEntitlement entitlement)
{
    assert (entitlement);

    if (!entitlement->mSubscription)
        return MAMA_STATUS_OK;

    oeaSubscription_destroy ((oeaSubscription*)entitlement->mSubscription);
    entitlement->mSubscription = NULL;
    return MAMA_STATUS_OK;
}

mama_status
oeaEntitlement_setSubscriptionType (mamaEntitlement entitlement,
                                    mamaServiceLevel level)
{
    assert (entitlement);
    
    oeaSubscription_setIsSnapshot ((oeaSubscription*)entitlement->mSubscription, 
        level == MAMA_SERVICE_LEVEL_SNAPSHOT?1:0);
    return MAMA_STATUS_OK;
}

mama_status
oeaEntitlement_checkEntitledWithSubject (mamaEntitlement entitlement, 
                                         const char*     subject)
{
    assert (entitlement);
    assert (subject);

    oeaSubscription_setSubject ((oeaSubscription*)entitlement->mSubscription, 
                                                                     subject);

    oeaStatus  entitlementStatus =
        oeaSubscription_isAllowed ((oeaSubscription*)entitlement->mSubscription); 

    return ( entitlementStatus == OEA_STATUS_OK ? 
        MAMA_STATUS_OK : MAMA_STATUS_NOT_ENTITLED );
}

mama_status
oeaEntitlement_checkEntitledWithCode (mamaEntitlement entitlement, 
                                      int32_t         entitleCode)
{
    assert (entitlement);
    
    oeaSubscription_addEntitlementCode (
        (oeaSubscription*)entitlement->mSubscription, entitleCode);

    oeaSubscription_open ((oeaSubscription*)entitlement->mSubscription);

    int result = 
        oeaSubscription_isOpen ((oeaSubscription*)entitlement->mSubscription);

    return ( result ? MAMA_STATUS_OK : MAMA_STATUS_NOT_ENTITLED);
}
