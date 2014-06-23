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

#include "middlewaremanager.h"
#include "mama/middlewaremanager.h"

#include <assert.h>
#include <platform.h>
#include <wombat/wInterlocked.h>

#include "bridge.h"
#include "defaultmanager.h"
#include "librarymanager.h"
#include "mama/payloadmanager.h"

/*
 * Private types
 */

typedef struct mamaMiddlewareLibraryManagerImpl_* mamaMiddlewareLibraryManager;

typedef struct mamaMiddlewareLibraryImpl_
{
    mamaLibrary                  mParent;
    wInterlockedInt              mOpenCount;
    wInterlockedInt              mStartCount;
    mamaBridge                   mBridge;
    char                         mMiddlewareId;
    mamaMiddlewareLibraryManager mManager;
} mamaMiddlewareLibraryImpl;

typedef struct mamaMiddlewareLibraryManagerImpl_
{
    mamaLibraryTypeManager mParent;
    mamaMiddlewareLibrary  mMiddleware [MAX_LIBRARIES];
    wInterlockedInt        mNumOpen;
    wInterlockedInt        mNumActive;
    mama_size_t            mStartSignal;
    mama_size_t            mStopSignal;
} mamaMiddlewareLibraryManagerImpl;

/*
 * Private declarations
 */

static mama_status
mamaMiddlewareLibraryManagerImpl_getInstance (
        mamaMiddlewareLibraryManager* mwManager);

static bridge_createImpl
mamaMiddlewareLibraryManagerImpl_getCreateImpl (const char* libraryName,
                                                LIB_HANDLE  libraryHandle);

static bridge_init
mamaMiddlewareLibraryManager_getInit (const char* libraryName,
                                      LIB_HANDLE  libraryHandle);

static bridge_createImpl
mamaMiddlewareLibraryManagerImpl_getLibraryCreateImpl (mamaLibrary library);

static bridge_init
mamaMiddlewareLibraryManagerImpl_getLibraryInit (mamaLibrary library);

static mama_status
mamaMiddlewareLibraryManagerImpl_createBridge (mamaLibrary library,
                                               mamaBridge* bridge0);

static void
mamaMiddlewareLibraryManagerImpl_destroyBridge (mamaBridge bridge);

static mama_status
mamaMiddlewareLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaMiddlewareLibrary* mwLibrary0);

static void
mamaMiddlewareLibraryManagerImpl_destroyLibrary (
        mamaMiddlewareLibrary mwLibrary);

static mama_status
mamaMiddlewareLibraryManagerImpl_loadDefaultPayloads (mamaLibrary library,
                                                      mamaBridge  bridge);

static mama_status
mamaMiddlewareLibraryManagerImpl_getMiddlewareId (
        mamaMiddlewareLibrary library, char* middlewareId);

static mama_status
mamaMiddlewareLibraryManagerImpl_closeFull (mamaMiddlewareLibrary mwLibrary);

static mama_status
mamaMiddlewareLibraryManagerImpl_stopFull (mamaMiddlewareLibrary mwLibrary);

/*
 * Private implementation
 */

static mama_status
mamaMiddlewareLibraryManagerImpl_getInstance (
        mamaMiddlewareLibraryManager* mwManager)
{
     if (!mwManager)
         return MAMA_STATUS_NULL_ARG;

     mamaLibraryTypeManager manager = NULL;
     mama_status status =
         mamaLibraryManager_getTypeManager (MAMA_MIDDLEWARE_LIBRARY, &manager);

     if (MAMA_STATUS_OK == status)
         *mwManager = (mamaMiddlewareLibraryManager) manager->mClosure;
     else
         *mwManager = NULL;

     return status;
}

static bridge_createImpl
mamaMiddlewareLibraryManagerImpl_getCreateImpl (const char* libraryName,
                                                LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Bridge_createImpl");
    return *(bridge_createImpl*)&func;
}

static bridge_createImpl
mamaMiddlewareLibraryManagerImpl_getLibraryCreateImpl (mamaLibrary library)
{
    return mamaMiddlewareLibraryManagerImpl_getCreateImpl (library->mName,
                                                           library->mHandle);
}

static bridge_init
mamaMiddlewareLibraryManagerImpl_getInit (const char* libraryName,
                                          LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Bridge_init");
    return *(bridge_init*)&func;
}

static bridge_init
mamaMiddlewareLibraryManagerImpl_getLibraryInit (mamaLibrary library)
{
    return mamaMiddlewareLibraryManagerImpl_getInit (library->mName,
                                                     library->mHandle);
}

/*
 * @brief Initialise library bridge.
 *
 * Backwards compatible (old-style) bridges, which initialises themselves,
 * pose an interesting compatibility issue.  If we let the * bridge allocate
 * itself, then it is compile-time bound to the size * of the bridge structure,
 * so it is not forwards compatible with any future changes.  Yet we still
 * need to let it do it in order to be backwards compatible (and because they
 * set other things on the structure, like closures).  The solution is to let
 * them and then copy the relevant parts before deallocating theirs again.
 */
static mama_status
mamaMiddlewareLibraryManagerImpl_createBridge (mamaLibrary library,
                                               mamaBridge* bridge0)
{
    assert (bridge0);
    *bridge0 = NULL;

    mamaBridge bridge = (mamaBridge)
        calloc (1, sizeof (mamaBridgeImpl));

    if (!bridge)
        return MAMA_STATUS_NOMEM;

    bridge_init init = 
        mamaMiddlewareLibraryManagerImpl_getLibraryInit (library);
    
    if (init)
    {
       mama_status status = init ();

        if (MAMA_STATUS_OK != status)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaPayloadLibraryManager_createBridge(): "
                      "Could not initialise %s library %s bridge using "
                      "new-style initialisation.",
                      library->mTypeName, library->mName);

            free (bridge);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        } 
    }
    else
    {
        bridge_createImpl createImpl =
            mamaMiddlewareLibraryManagerImpl_getLibraryCreateImpl (library);

        if (!createImpl)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaPayloadLibraryManager_createBridge(): "
                      "Could not find either init or createImpl "
                      "initialise method for %s library %s bridge",
                      library->mTypeName, library->mName);
            
            free (bridge);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        }

        /* FIXME: We might need to make a carbon copy of mamaPayloadBridge
         * and make it mamaPayloadOldBridge if we make any changes that
         * would affect binary-compatibility between structures, other than
         * adding things on to the end of the structure. */
        mamaBridge oldBridge = NULL;
        createImpl (&oldBridge);

        if (!oldBridge)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaMiddlewareLibraryManager_loadLibrary(): "
                      "Could not initialise %s library %s bridge using "
                      "old-style allocation.",
                      library->mTypeName, library->mName);

            free (bridge);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        }

        /* Copy relevant parts of bridge.
         * FIXME: Should we have a copy/move function?
         * FIXME: Are all of these actually used?
         */
        bridge->mRefCount           = oldBridge->mRefCount;
        bridge->mInternalEventQueue = oldBridge->mInternalEventQueue;
        bridge->mInternalDispatcher = oldBridge->mInternalDispatcher;
        bridge->mClosure            = oldBridge->mClosure;
        bridge->mNativeMsgBridge    = oldBridge->mNativeMsgBridge;
        bridge->mBridgeInfoCallback = oldBridge->mBridgeInfoCallback;
        bridge->mCppCallback        = oldBridge->mCppCallback;

        /* FIXME: Might need to hold on to memory here */
        free (oldBridge->mLock);
        free (oldBridge);
    }

    *bridge0 = bridge;
    return MAMA_STATUS_OK;
}

static void
mamaMiddlewareLibraryManagerImpl_destroyBridge (mamaBridge bridge)
{
    free (bridge);
}

static mama_status
mamaMiddlewareLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaMiddlewareLibrary* mwLibrary0)
{
    assert (mwLibrary0);
    *mwLibrary0 = NULL;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) calloc (1, sizeof (mamaMiddlewareLibraryImpl));

    if (!mwLibrary)
        return MAMA_STATUS_NOMEM;

    mamaBridge bridge = NULL;
    status = mamaMiddlewareLibraryManagerImpl_createBridge (library, &bridge);

    if (MAMA_STATUS_OK != status)
    {
        free (mwLibrary);
        return status;
    }

    /* Establish bi-directional links */
    bridge->mLibrary = mwLibrary;
    library->mClosure = mwLibrary;

    mwLibrary->mBridge  = bridge;
    mwLibrary->mParent  = library;
    mwLibrary->mManager = mwManager;

    wInterlocked_initialize (&mwLibrary->mOpenCount);
    wInterlocked_set (0, &mwLibrary->mOpenCount);
    wInterlocked_initialize (&mwLibrary->mStartCount);
    wInterlocked_set (0, &mwLibrary->mStartCount);

    *mwLibrary0 = mwLibrary;
    return MAMA_STATUS_OK;
}

static mama_status
mamaMiddlewareLibraryManagerImpl_closeFull (mamaMiddlewareLibrary mwLibrary)
{
    mama_status status = MAMA_STATUS_OK;

    while (MAMA_STATUS_OK == status &&
           wInterlocked_read (&mwLibrary->mOpenCount) > 0)
    {
        status = mamaMiddlewareLibraryManager_closeBridge (mwLibrary);
    }

    if (MAMA_STATUS_OK != status)
    {
        mama_log (MAMA_LOG_LEVEL_WARN,
                  "mamaMiddlewareLibraryManagerImpl_closeFull(): "
                  "Failed to fully close %s library %s bridge.",
                  mwLibrary->mParent->mTypeName, mwLibrary->mParent->mName);
    }

    return status;
}

static mama_status
mamaMiddlewareLibraryManagerImpl_stopFull (mamaMiddlewareLibrary mwLibrary)
{
    mama_status status = MAMA_STATUS_OK;

    while (MAMA_STATUS_OK == status &&
           wInterlocked_read (&mwLibrary->mOpenCount) > 0)
    {
        status = mamaMiddlewareLibraryManager_stopBridge (mwLibrary);
    }

    if (MAMA_STATUS_OK != status)
    {
        mama_log (MAMA_LOG_LEVEL_WARN,
                  "mamaMiddlewareLibraryManagerImpl_closeFull(): "
                  "Failed to fully stop %s library %s bridge.",
                  mwLibrary->mParent->mTypeName, mwLibrary->mParent->mName);
    }

    return status;
}

static void
mamaMiddlewareLibraryManagerImpl_destroyLibrary (
        mamaMiddlewareLibrary mwLibrary)
{
    if (!mwLibrary)
        return;

    mamaLibrary library = (mamaLibrary) mwLibrary;

    /* Complain if bridge is still open */
    if (0 != wInterlocked_read (&mwLibrary->mOpenCount))
    {
        mama_log (MAMA_LOG_LEVEL_FINER,
                  "mamaMiddlewareLibraryManagerImpl_destroyLibrary(): "
                  "Destroying %s library %s but bridge was still open, "
                  "so forcing a bridge close.",
                  library->mTypeName, library->mName);

        mamaMiddlewareLibraryManagerImpl_closeFull (mwLibrary);
    }

    mamaMiddlewareLibraryManagerImpl_destroyBridge (mwLibrary->mBridge);

    wInterlocked_destroy (&mwLibrary->mStartCount);
    wInterlocked_destroy (&mwLibrary->mOpenCount);

    mwLibrary->mParent->mClosure = NULL;
    free (mwLibrary);
}

static mama_status
mamaMiddlewareLibraryManagerImpl_loadDefaultPayloads (mamaLibrary library,
                                                      mamaBridge  bridge)
{
    mama_status status = MAMA_STATUS_OK;

    /* Attempt to load the default payload(s) for the library */
    char** payloadNames = NULL;
    char*  payloadIds   = NULL;

    bridge->bridgeGetDefaultPayloadId (&payloadNames, &payloadIds);

    if (payloadNames && payloadIds)
    {
        while (*payloadNames && *payloadIds)
        {
            mamaPayloadLibrary payloadLibrary = NULL;

            const char* paths [2] = {
                library->mPath,
                NULL
            };

            /* Try to load with same path first */
            for (mama_size_t k = 0; k < sizeof (paths)/sizeof (paths[0]); ++k)
            {
                status =
                    mamaPayloadLibraryManager_loadLibraryWithPath (*payloadNames,
                                                                   paths[k],
                                                                   &payloadLibrary);

                if (MAMA_STATUS_OK == status)
                    break;
            }

            /* Failed to load a required payload? */
            if (MAMA_STATUS_OK != status)
                break;

            ++payloadNames;
            ++payloadIds;
        }
    }

    return status;
}

static mama_status
mamaMiddlewareLibraryManagerImpl_getMiddlewareId (
        mamaMiddlewareLibrary mwLibrary, char* middlewareId)
{
    assert (mwLibrary);
    assert (middlewareId);
    *middlewareId = 0;

    /* Attempting to find the payload ID takes a hierarchial approach:
     * first check for a configured ID, then check the old method and
     * finally just take a free slot in the array. */

    mamaLibrary library = mwLibrary->mParent;
    const char* prop =
        mamaLibraryManager_getLibraryProperty (library, "id");

    if (prop)
    {
        *middlewareId = prop[0];
        return MAMA_STATUS_OK;
    }

    mamaMiddleware tmpId =
            mamaMiddlewareLibraryManager_convertFromString (library->mName);

    if (tmpId != MAMA_MIDDLEWARE_UNKNOWN)
    {
        *middlewareId = tmpId;
        return MAMA_STATUS_OK;
    }

    mamaMiddlewareLibraryManager mwManager = mwLibrary->mManager;
    mama_status status = MAMA_STATUS_SYSTEM_ERROR;

    /* Select next free slot */
    for (mama_size_t k = 0; k < MAX_LIBRARIES; ++k)
    {
        if (!mwManager->mMiddleware[k])
        {
            *middlewareId = k;
            status = MAMA_STATUS_OK;
            break;
        }
    }

    return status;
}

static
mama_bool_t mamaMiddlewareLibraryManagerImpl_isOpenedBridge (mamaLibrary lib)
{
    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) lib->mClosure;
    return (mwLibrary && wInterlocked_read (&mwLibrary->mOpenCount) > 0);
}

static
mama_bool_t mamaMiddlewareLibraryManagerImpl_isActiveBridge (mamaLibrary lib)
{
    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) lib->mClosure;
    return (mwLibrary && wInterlocked_read (&mwLibrary->mStartCount) > 0);
}

static
mama_bool_t mamaMiddlewareLibraryManagerImpl_isInactiveBridge (mamaLibrary lib)
{
    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) lib->mClosure;
    return (mwLibrary && mamaMiddlewareLibraryManagerImpl_isOpenedBridge (lib) &&
            !mamaMiddlewareLibraryManagerImpl_isActiveBridge (lib));
}

static
mama_bool_t mamaMiddlewareLibraryManagerImpl_isClosedBridge (mamaLibrary lib)
{
    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) lib->mClosure;
    return (mwLibrary &&
            !mamaMiddlewareLibraryManagerImpl_isOpenedBridge (lib));
}

static mama_status
mamaMiddlewareLibraryManagerImpl_getLibraries (mamaMiddlewareLibrary* mwLibraries,
                                               mama_size_t*           size,
                                               mamaLibraryPredicateCb predicate)
{
    if (!mwLibraries || !size)
        return MAMA_STATUS_NULL_ARG;
 
    mamaLibrary libraries [MAX_LIBRARIES];
    mama_size_t librariesSize = MAX_LIBRARIES;

    mama_status status =
        mamaLibraryManager_getLibraries (libraries,
                                         &librariesSize,
                                         MAMA_MIDDLEWARE_LIBRARY,
                                         predicate);

    for (mama_size_t k = 0; k < librariesSize && k < *size; ++k)
        mwLibraries[k] = (mamaMiddlewareLibrary)libraries[k]->mClosure;

    if (librariesSize < *size)
        *size = librariesSize;

    return status;
}

static mama_bool_t
mamaMiddlewareLibraryManagerImpl_stopAllCb (mamaLibrary library,
                                            void*       closure)
{
    mamaMiddlewareLibrary mwLibrary =
        (mamaMiddlewareLibrary) library->mClosure;

    mamaMiddlewareLibraryManagerImpl_stopFull (mwLibrary);
    return 1;
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaMiddlewareLibraryManager_create (mamaLibraryTypeManager manager)
{
    mamaMiddlewareLibraryManager mwManager = (mamaMiddlewareLibraryManager)
        calloc (1, sizeof (mamaMiddlewareLibraryManagerImpl));

    if (!mwManager)
        return MAMA_STATUS_NOMEM;

    mamaLibraryManager_createCallbackSignal (manager,
                                             &mwManager->mStartSignal);

    mamaLibraryManager_createCallbackSignal (manager,
                                             &mwManager->mStopSignal);

    wInterlocked_initialize (&mwManager->mNumOpen);
    wInterlocked_set (0, &mwManager->mNumOpen);
    wInterlocked_initialize (&mwManager->mNumActive);
    wInterlocked_set (0, &mwManager->mNumActive);

    /* Establish bi-directional link */
    manager->mClosure = mwManager;
    mwManager->mParent = manager;

    return MAMA_STATUS_OK;
}

void
mamaMiddlewareLibraryManager_destroy (void)
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return;

    wInterlocked_destroy (&mwManager->mNumActive);
    wInterlocked_destroy (&mwManager->mNumOpen);

    mamaLibraryManager_destroyCallbackSignal (mwManager->mParent,
                                              mwManager->mStartSignal);

    mamaLibraryManager_destroyCallbackSignal (mwManager->mParent,
                                              mwManager->mStopSignal);

    mwManager->mParent->mClosure = NULL;
    free (mwManager);
}

#define REGISTER_BRIDGE_FUNCTION(funcName, bridgeFuncName, funcSig)\
do {\
    if (MAMA_STATUS_OK == status)\
    {\
        void* func =\
            mamaLibraryManager_loadLibraryFunction\
                    (library->mName, library->mHandle, #funcName);\
        bridge->bridgeFuncName = *(funcSig*) &func;\
        if (!bridge->bridgeFuncName)\
        {\
            mama_log (MAMA_LOG_LEVEL_ERROR,\
                      "mamaMiddlewareLibraryManager_loadLibrary(): "\
                      "Could not load %s library %s because "\
                      "required function [%s] is missing in bridge.",\
                      library->mTypeName, library->mName,\
                      #funcName);\
            status = MAMA_STATUS_NO_BRIDGE_IMPL;\
        }\
    }\
} while (0);

mama_status
mamaMiddlewareLibraryManager_loadLibrary (mamaLibrary library)
{
    mamaMiddlewareLibrary mwLibrary = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_createLibrary (library,
                                                        &mwLibrary);

    if (MAMA_STATUS_OK != status)
    {
        return status;
    }

    /* Once the bridge has been successfully loaded, and the initialization
     * function called, we register each of the required bridge functions.
     */
    mamaBridge bridge = mwLibrary->mBridge;

    REGISTER_BRIDGE_FUNCTION (Bridge_open,  bridgeOpen,  bridge_open);
    REGISTER_BRIDGE_FUNCTION (Bridge_close, bridgeClose, bridge_close);
    REGISTER_BRIDGE_FUNCTION (Bridge_start, bridgeStart, bridge_start);
    REGISTER_BRIDGE_FUNCTION (Bridge_stop,  bridgeStop,  bridge_stop);

    /*General purpose functions*/
    REGISTER_BRIDGE_FUNCTION (Bridge_getVersion,
                              bridgeGetVersion,
                              bridge_getVersion);

    REGISTER_BRIDGE_FUNCTION (Bridge_getName,
                              bridgeGetName,
                              bridge_getName);

    REGISTER_BRIDGE_FUNCTION (Bridge_getDefaultPayloadId,
                              bridgeGetDefaultPayloadId,
                              bridge_getDefaultPayloadId);

    /*Queue functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_create,
                              bridgeMamaQueueCreate,
                              bridgeMamaQueue_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_create_usingNative,
                              bridgeMamaQueueCreateUsingNative,
                              bridgeMamaQueue_create_usingNative);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_destroy,
                              bridgeMamaQueueDestroy,
                              bridgeMamaQueue_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_getEventCount,
                              bridgeMamaQueueGetEventCount,
                              bridgeMamaQueue_getEventCount);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_dispatch,
                              bridgeMamaQueueDispatch,
                              bridgeMamaQueue_dispatch);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_timedDispatch,
                              bridgeMamaQueueTimedDispatch,
                              bridgeMamaQueue_timedDispatch);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_dispatchEvent,
                              bridgeMamaQueueDispatchEvent,
                              bridgeMamaQueue_dispatchEvent);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_enqueueEvent,
                              bridgeMamaQueueEnqueueEvent,
                              bridgeMamaQueue_enqueueEvent);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_stopDispatch,
                              bridgeMamaQueueStopDispatch,
                              bridgeMamaQueue_stopDispatch);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_setEnqueueCallback,
                              bridgeMamaQueueSetEnqueueCallback,
                              bridgeMamaQueue_setEnqueueCallback);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_removeEnqueueCallback,
                              bridgeMamaQueueRemoveEnqueueCallback,
                              bridgeMamaQueue_removeEnqueueCallback);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_getNativeHandle,
                              bridgeMamaQueueGetNativeHandle,
                              bridgeMamaQueue_getNativeHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_setLowWatermark,
                              bridgeMamaQueueSetLowWatermark,
                              bridgeMamaQueue_setLowWatermark);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_setHighWatermark,
                              bridgeMamaQueueSetHighWatermark,
                              bridgeMamaQueue_setHighWatermark);

    /*Transport functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_isValid,
                              bridgeMamaTransportIsValid,
                              bridgeMamaTransport_isValid);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_destroy,
                              bridgeMamaTransportDestroy,
                              bridgeMamaTransport_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_create,
                              bridgeMamaTransportCreate,
                              bridgeMamaTransport_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_forceClientDisconnect,
                              bridgeMamaTransportForceClientDisconnect,
                              bridgeMamaTransport_forceClientDisconnect);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_findConnection,
                              bridgeMamaTransportFindConnection,
                              bridgeMamaTransport_findConnection);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getAllConnections,
                              bridgeMamaTransportGetAllConnections,
                              bridgeMamaTransport_getAllConnections);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getAllConnectionsForTopic,
                              bridgeMamaTransportGetAllConnectionsForTopic,
                              bridgeMamaTransport_getAllConnectionsForTopic);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_freeAllConnections,
                              bridgeMamaTransportFreeAllConnections,
                              bridgeMamaTransport_freeAllConnections);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getAllServerConnections,
                              bridgeMamaTransportGetAllServerConnections,
                              bridgeMamaTransport_getAllServerConnections);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_freeAllServerConnections,
                              bridgeMamaTransportFreeAllServerConnections,
                              bridgeMamaTransport_freeAllServerConnections);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getNumLoadBalanceAttributes,
                              bridgeMamaTransportGetNumLoadBalanceAttributes,
                              bridgeMamaTransport_getNumLoadBalanceAttributes);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getLoadBalanceSharedObjectName,
                              bridgeMamaTransportGetLoadBalanceSharedObjectName,
                              bridgeMamaTransport_getLoadBalanceSharedObjectName);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getLoadBalanceScheme,
                              bridgeMamaTransportGetLoadBalanceScheme,
                              bridgeMamaTransport_getLoadBalanceScheme);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_sendMsgToConnection,
                              bridgeMamaTransportSendMsgToConnection,
                              bridgeMamaTransport_sendMsgToConnection);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_isConnectionIntercepted,
                              bridgeMamaTransportIsConnectionIntercepted,
                              bridgeMamaTransport_isConnectionIntercepted);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_installConnectConflateMgr,
                              bridgeMamaTransportInstallConnectConflateMgr,
                              bridgeMamaTransport_installConnectConflateMgr);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_uninstallConnectConflateMgr,
                              bridgeMamaTransportUninstallConnectConflateMgr,
                              bridgeMamaTransport_uninstallConnectConflateMgr);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_startConnectionConflation,
                              bridgeMamaTransportStartConnectionConflation,
                              bridgeMamaTransport_startConnectionConflation);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_requestConflation,
                              bridgeMamaTransportRequestConflation,
                              bridgeMamaTransport_requestConflation);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_requestEndConflation,
                              bridgeMamaTransportRequestEndConflation,
                              bridgeMamaTransport_requestEndConflation);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getNativeTransport,
                              bridgeMamaTransportGetNativeTransport,
                              bridgeMamaTransport_getNativeTransport);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTransport_getNativeTransportNamingCtx,
                              bridgeMamaTransportGetNativeTransportNamingCtx,
                              bridgeMamaTransport_getNativeTransportNamingCtx);

    /*Subscription functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_create,
                              bridgeMamaSubscriptionCreate,
                              bridgeMamaSubscription_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_createWildCard,
                              bridgeMamaSubscriptionCreateWildCard,
                              bridgeMamaSubscription_createWildCard);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_mute,
                              bridgeMamaSubscriptionMute,
                              bridgeMamaSubscription_mute);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_destroy,
                              bridgeMamaSubscriptionDestroy,
                              bridgeMamaSubscription_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_isValid,
                              bridgeMamaSubscriptionIsValid,
                              bridgeMamaSubscription_isValid);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_hasWildcards,
                              bridgeMamaSubscriptionHasWildcards,
                              bridgeMamaSubscription_hasWildcards);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_getPlatformError,
                              bridgeMamaSubscriptionGetPlatformError,
                              bridgeMamaSubscription_getPlatformError);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_setTopicClosure,
                              bridgeMamaSubscriptionSetTopicClosure,
                              bridgeMamaSubscription_setTopicClosure);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_muteCurrentTopic,
                              bridgeMamaSubscriptionMuteCurrentTopic,
                              bridgeMamaSubscription_muteCurrentTopic);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaSubscription_isTportDisconnected,
                              bridgeMamaSubscriptionIsTportDisconnected,
                              bridgeMamaSubscription_isTportDisconnected);

    /*Timer functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaTimer_create,
                              bridgeMamaTimerCreate,
                              bridgeMamaTimer_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTimer_destroy,
                              bridgeMamaTimerDestroy,
                              bridgeMamaTimer_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTimer_reset,
                              bridgeMamaTimerReset,
                              bridgeMamaTimer_reset);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTimer_setInterval,
                              bridgeMamaTimerSetInterval,
                              bridgeMamaTimer_setInterval);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaTimer_getInterval,
                              bridgeMamaTimerGetInterval,
                              bridgeMamaTimer_getInterval);

    /*IO functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaIo_create,
                              bridgeMamaIoCreate,
                              bridgeMamaIo_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaIo_getDescriptor,
                              bridgeMamaIoGetDescriptor,
                              bridgeMamaIo_getDescriptor);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaIo_destroy,
                              bridgeMamaIoDestroy,
                              bridgeMamaIo_destroy);

    /*Publisher functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_create,
                              bridgeMamaPublisherCreate,
                              bridgeMamaPublisher_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_createByIndex,
                              bridgeMamaPublisherCreateByIndex,
                              bridgeMamaPublisher_createByIndex);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_destroy,
                              bridgeMamaPublisherDestroy,
                              bridgeMamaPublisher_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_send,
                              bridgeMamaPublisherSend,
                              bridgeMamaPublisher_send);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_sendFromInbox,
                              bridgeMamaPublisherSendFromInbox,
                              bridgeMamaPublisher_sendFromInbox);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_sendFromInboxByIndex,
                              bridgeMamaPublisherSendFromInboxByIndex,
                              bridgeMamaPublisher_sendFromInboxByIndex);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_sendReplyToInbox,
                              bridgeMamaPublisherSendReplyToInbox,
                              bridgeMamaPublisher_sendReplyToInbox);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaPublisher_sendReplyToInboxHandle,
                              bridgeMamaPublisherSendReplyToInboxHandle,
                              bridgeMamaPublisher_sendReplyToInboxHandle);

    /*Inbox functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaInbox_create,
                              bridgeMamaInboxCreate,
                              bridgeMamaInbox_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaInbox_createByIndex,
                              bridgeMamaInboxCreateByIndex,
                              bridgeMamaInbox_createByIndex);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaInbox_destroy,
                              bridgeMamaInboxDestroy,
                              bridgeMamaInbox_destroy);

    /*Mama Msg functions*/
    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_create,
                              bridgeMamaMsgCreate,
                              bridgeMamaMsg_create);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_isFromInbox,
                              bridgeMamaMsgIsFromInbox,
                              bridgeMamaMsg_isFromInbox);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_getPlatformError,
                              bridgeMamaMsgGetPlatformError,
                              bridgeMamaMsg_getPlatformError);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_detach,
                              bridgeMamaMsgDetach,
                              bridgeMamaMsg_detach);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_destroy,
                              bridgeMamaMsgDestroy,
                              bridgeMamaMsg_destroy);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_destroyMiddlewareMsg,
                              bridgeMamaMsgDestroyMiddlewareMsg,
                              bridgeMamaMsg_destroyMiddlewareMsg);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_setSendSubject,
                              bridgeMamaMsgSetSendSubject,
                              bridgeMamaMsg_setSendSubject);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_getNativeHandle,
                              bridgeMamaMsgGetNativeHandle,
                              bridgeMamaMsg_getNativeHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_duplicateReplyHandle,
                              bridgeMamaMsgDuplicateReplyHandle,
                              bridgeMamaMsg_duplicateReplyHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_copyReplyHandle,
                              bridgeMamaMsgCopyReplyHandle,
                              bridgeMamaMsg_copyReplyHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsgImpl_setReplyHandle,
                              bridgeMamaMsgSetReplyHandle,
                              bridgeMamaMsgImpl_setReplyHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsgImpl_setReplyHandle,
                              bridgeMamaMsgSetReplyHandleAndIncrement,
                              bridgeMamaMsgImpl_setReplyHandle);

    REGISTER_BRIDGE_FUNCTION (BridgeMamaMsg_destroyReplyHandle,
                              bridgeMamaMsgDestroyReplyHandle,
                              bridgeMamaMsg_destroyReplyHandle);

    if (MAMA_STATUS_OK != status)
    {
        free (bridge);
        return status;
    }

    char middlewareId = 0;
    status =
        mamaMiddlewareLibraryManagerImpl_getMiddlewareId (mwLibrary,
                                                          &middlewareId);

    if (status != MAMA_STATUS_OK)
    {
        free (bridge);
        return status;
    }

    mamaMiddlewareLibraryManager mwManager = mwLibrary->mManager;
    mamaMiddlewareLibrary dupLibrary = mwManager->mMiddleware [middlewareId];

    if (dupLibrary)
    {
       mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaMiddlewareLibraryManager_loadLibrary(): "
                  "Middleware id [%d] for %s library %s duplicates library %s.",
                  middlewareId, library->mTypeName,
                  library->mName,
                  dupLibrary->mParent->mName);

        free (bridge);
        return MAMA_STATUS_SYSTEM_ERROR;
    }

    status =
        mamaMiddlewareLibraryManagerImpl_loadDefaultPayloads (library, bridge);

    if (MAMA_STATUS_OK != status)
    {
        free (bridge);
        return status;
    }

    mwLibrary->mMiddlewareId = middlewareId;
    mwManager->mMiddleware [middlewareId] = mwLibrary;
    return status;
}

void
mamaMiddlewareLibraryManager_unloadLibrary (mamaLibrary library)
{
    mamaMiddlewareLibrary mwLibrary = (mamaMiddlewareLibrary) library->mClosure;
    mamaMiddlewareLibraryManager mwManager = mwLibrary->mManager;

    char middlewareId = mwLibrary->mMiddlewareId;
    mamaMiddlewareLibraryManagerImpl_destroyLibrary (mwLibrary);
    mwManager->mMiddleware[middlewareId] = NULL;
}

void mamaMiddlewareLibraryManager_dump (mamaLibraryTypeManager manager)
{
    mamaMiddlewareLibraryManager mwManager = 
        (mamaMiddlewareLibraryManager) manager->mClosure;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Num Open [%d] Num Active [%d]", 
        wInterlocked_read(&mwManager->mNumOpen), 
        wInterlocked_read(&mwManager->mNumActive));
}

void mamaMiddlewareLibraryManager_dumpLibrary (mamaLibrary library)
{
    mamaMiddlewareLibrary mwLibrary = 
        (mamaMiddlewareLibrary) library->mClosure;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Middleware Id [%c]", 
                                 mwLibrary->mMiddlewareId);    
}

mamaLibraryType
mamaMiddlewareLibraryManager_classifyLibraryType (const char* libraryName,
                                                  LIB_HANDLE  libraryLib)
{
    bridge_createImpl createImpl =
        mamaMiddlewareLibraryManagerImpl_getCreateImpl (libraryName,
                                                        libraryLib);

    bridge_init init = 
        mamaMiddlewareLibraryManagerImpl_getInit (libraryName,
                                                  libraryLib);

    if (createImpl || init)
        return MAMA_MIDDLEWARE_LIBRARY;

    return MAMA_UNKNOWN_LIBRARY;
}

/*
 * Public implementation
 */

mama_status
mamaMiddlewareLibraryManager_loadLibraryWithPath (const char*            middlewareName,
                                                  const char*            path,
                                                  mamaMiddlewareLibrary* mwLibrary)
{
    if (!mwLibrary || !middlewareName)
        return MAMA_STATUS_NULL_ARG;

    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_loadLibrary (middlewareName,
                                        MAMA_MIDDLEWARE_LIBRARY,
                                        path, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *mwLibrary =
        (mamaMiddlewareLibrary) library->mClosure;

    return MAMA_STATUS_OK;
}

mama_status
mamaMiddlewareLibraryManager_getLibrary (const char*            middlewareName,
                                         mamaMiddlewareLibrary* mwLibrary)
{
    if (!mwLibrary || !middlewareName)
        return MAMA_STATUS_NULL_ARG;

    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_getLibrary (middlewareName,
                                       MAMA_MIDDLEWARE_LIBRARY,
                                       &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *mwLibrary =
        (mamaMiddlewareLibrary) library->mClosure;

    return MAMA_STATUS_OK;
}

mama_status
mamaMiddlewareLibraryManager_getDefaultLibrary (mamaMiddlewareLibrary* library)
{
    if (!library)
        return MAMA_STATUS_NULL_ARG;

    *library = NULL;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    if (0 == mwManager->mParent->mNumLibraries)
        return MAMA_STATUS_NOT_FOUND;

    /* Find first bridge */
    status = MAMA_STATUS_NOT_FOUND;
    for (mama_size_t k = 0; k < MAX_LIBRARIES; ++k)
    {
        mamaMiddlewareLibrary mwLibrary = mwManager->mMiddleware[k];

        if (mwLibrary)
        {
            *library = mwLibrary;
            return MAMA_STATUS_OK;
        }
    }

    return status;
}

mama_status
mamaMiddlewareLibraryManager_getLibraries (mamaMiddlewareLibrary* libraries,
                                           mama_size_t*           size)
{
    return mamaMiddlewareLibraryManagerImpl_getLibraries (libraries, size,
                                                          NULL);
}

mama_status
mamaMiddlewareLibraryManager_getOpenedBridges (mamaMiddlewareLibrary*  libraries,
                                             mama_size_t*            size)
{
    mamaLibraryPredicateCb predicate =
        mamaMiddlewareLibraryManagerImpl_isOpenedBridge;
    return mamaMiddlewareLibraryManagerImpl_getLibraries (libraries, size,
                                                          predicate);
}

mama_status
mamaMiddlewareLibraryManager_getActiveBridges (mamaMiddlewareLibrary*  libraries,
                                               mama_size_t*            size)
{
    mamaLibraryPredicateCb predicate =
        mamaMiddlewareLibraryManagerImpl_isActiveBridge;
    return mamaMiddlewareLibraryManagerImpl_getLibraries (libraries, size,
                                                          predicate);
}

mama_status
mamaMiddlewareLibraryManager_getInactiveBridges (mamaMiddlewareLibrary*  libraries,
                                                 mama_size_t*            size)
{
    mamaLibraryPredicateCb predicate =
        mamaMiddlewareLibraryManagerImpl_isInactiveBridge;
    return mamaMiddlewareLibraryManagerImpl_getLibraries (libraries, size,
                                                          predicate);
}

mama_status
mamaMiddlewareLibraryManager_getClosedBridges (mamaMiddlewareLibrary*  bridges,
                                               mama_size_t*            size)
{
    mamaLibraryPredicateCb predicate =
        mamaMiddlewareLibraryManagerImpl_isClosedBridge;
    return mamaMiddlewareLibraryManagerImpl_getLibraries (bridges, size,
                                                          predicate);
}

mama_status
mamaMiddlewareLibraryManager_getNumLibraries (mama_size_t* size)
{
    if (!size)
        return MAMA_STATUS_NULL_ARG;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK == status)
        *size = mwManager->mParent->mNumLibraries;
    else
        *size = 0;

    return status;
}

mama_status
mamaMiddlewareLibraryManager_getNumOpenedBridges (mama_size_t* size)
{
    if (!size)
        return MAMA_STATUS_NULL_ARG;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK == status)
        *size = wInterlocked_read (&mwManager->mNumOpen);
    else
        *size = 0;

    return status;
}

mama_status
mamaMiddlewareLibraryManager_getNumActiveBridges (mama_size_t* size)
{
    if (!size)
        return MAMA_STATUS_NULL_ARG;

    mamaMiddlewareLibraryManager mwManager = NULL;

    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK == status)
        *size = wInterlocked_read (&mwManager->mNumActive);
    else
        *size = 0;

    return status;
}

mama_status
mamaMiddlewareLibraryManager_getNumInactiveBridges (mama_size_t* size)
{
    if (!size)
        return MAMA_STATUS_NULL_ARG;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK == status)
        *size = wInterlocked_read (&mwManager->mNumOpen) -
                wInterlocked_read (&mwManager->mNumActive);
    else
        *size = 0;

    return status;
}

mama_status
mamaMiddlewareLibraryManager_getNumClosedBridges (mama_size_t* size)
{
    if (!size)
        return MAMA_STATUS_NULL_ARG;

    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK == status)
        *size = wInterlocked_read (&mwManager->mParent->mNumLibraries) -
                wInterlocked_read (&mwManager->mNumOpen);
    else
        *size = 0;

    return status;
}

mama_status
mamaMiddlewareLibraryManager_openBridge (mamaMiddlewareLibrary mwLibrary)
{
    if (NULL == mwLibrary)
        return MAMA_STATUS_NULL_ARG;

    mamaBridge  bridge  = mwLibrary->mBridge;
    mamaLibrary library = mwLibrary->mParent;

    wlock_lock (library->mLock);
    mama_status status = MAMA_STATUS_OK;

    if (0 == wInterlocked_read (&mwLibrary->mOpenCount))
    {
        status = bridge->bridgeOpen (bridge);

        if (MAMA_STATUS_OK == status)
            wInterlocked_increment (&mwLibrary->mManager->mNumOpen);
    }

    if (MAMA_STATUS_OK == status)
        wInterlocked_increment (&mwLibrary->mOpenCount);

    wlock_unlock (library->mLock);
    return status;
}

mama_status
mamaMiddlewareLibraryManager_closeBridge (mamaMiddlewareLibrary mwLibrary)
{
    if (NULL == mwLibrary)
        return MAMA_STATUS_NULL_ARG;

    mamaBridge  bridge  = mwLibrary->mBridge;
    mamaLibrary library = mwLibrary->mParent;

    wlock_lock (library->mLock);
    mama_status status = MAMA_STATUS_OK;

    if (wInterlocked_read (&mwLibrary->mOpenCount) > 0)
    {
        if (0 == wInterlocked_decrement (&mwLibrary->mOpenCount))
        {
            /* Complain if bridge is still running (active) */
            if (0 != wInterlocked_read (&mwLibrary->mStartCount))
            {
                mama_log (MAMA_LOG_LEVEL_WARN,
                          "mamaMiddlewareLibraryManager_closeBridge(): "
                          "Closing %s library %s bridge but bridge was still "
                          "running, so forcing a bridge stop.",
                          library->mTypeName, library->mName);

                mamaMiddlewareLibraryManagerImpl_stopFull (mwLibrary);
            }

            /* For the old style bridges where create allocates the bridge
             * and close destroys we need to create a copy of the bridge
             * and pass it to it - we actually want to keep the bridge around
             * the user wants to open the bridge again.
             */

            mamaBridge tmpBridge =
                (mamaBridge)calloc (1, sizeof (mamaBridgeImpl));
            memcpy (tmpBridge, bridge, sizeof (mamaBridgeImpl));

            mamaBridgeImpl_stopInternalEventQueue (tmpBridge);
            status = bridge->bridgeClose (tmpBridge);
            wInterlocked_decrement (&mwLibrary->mManager->mNumOpen);
        }
    }
    else
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaMiddlewareLibraryManager_closeBridge(): "
                  "Attempted to close %s library %s bridge but it has "
                  "already been closed (or was never opened).",
                  library->mTypeName,
                  library->mName);
        status = MAMA_STATUS_SYSTEM_ERROR;
    }

    wlock_unlock (library->mLock);
    return status;
}

mama_status
mamaMiddlewareLibraryManager_startBridge (mamaMiddlewareLibrary mwLibrary)
{
    if (NULL == mwLibrary)
        return MAMA_STATUS_NULL_ARG;

    mamaBridge  bridge  = mwLibrary->mBridge;
    mamaLibrary library = mwLibrary->mParent;

    if (!bridge->mDefaultEventQueue)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaMiddlewareLibraryManager_startBridge(): "
                  "Cannot start %s library %s bridge because it "
                  "hasn't been opened.",
                  library->mTypeName, library->mName);
        return MAMA_STATUS_INVALID_QUEUE;
    }

    wlock_lock (library->mLock);
    mama_status status = MAMA_STATUS_OK;

    if (0 == wInterlocked_read (&mwLibrary->mStartCount))
    {
        wInterlocked_increment (&mwLibrary->mStartCount);
        wInterlocked_increment (&mwLibrary->mManager->mNumActive);
        wlock_unlock (library->mLock);

        /* FIXME: Possible race condition here - Unfortunately we
         * can't hold the lock here due to bridgeStart blocking. */
        if (1 == wInterlocked_read (&mwLibrary->mStartCount))
            status = bridge->bridgeStart (bridge->mDefaultEventQueue);

        wlock_lock (library->mLock);
        if (MAMA_STATUS_OK != status)
        {
            wInterlocked_decrement (&mwLibrary->mStartCount);
            wInterlocked_decrement (&mwLibrary->mManager->mNumActive);
        }
    }

    wlock_unlock (library->mLock);
    return status;
}

mama_status
mamaMiddlewareLibraryManager_stopBridge (mamaMiddlewareLibrary mwLibrary)
{
    if (NULL == mwLibrary)
        return MAMA_STATUS_NULL_ARG;

    mamaBridge  bridge  = mwLibrary->mBridge;
    mamaLibrary library = mwLibrary->mParent;

    wlock_lock (library->mLock);
    mama_status status = MAMA_STATUS_OK;

    if (wInterlocked_read (&mwLibrary->mStartCount) > 0)
    {
        if (1 == wInterlocked_read (&mwLibrary->mStartCount))
        {
            status = bridge->bridgeStop (bridge->mDefaultEventQueue);
            mamaBridgeImpl_stopInternalEventQueue (bridge);
            wInterlocked_decrement (&mwLibrary->mStartCount);
            wInterlocked_decrement (&mwLibrary->mManager->mNumActive);
        }
    }
    else
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaMiddlewareLibraryManager_stopBridge(): "
                  "Attempted to stop %s library %s bridge but it has "
                  "already been stopped (or was never started).",
                  library->mTypeName,
                  library->mName);
        status = MAMA_STATUS_SYSTEM_ERROR;
    }

    wlock_unlock (library->mLock);
    return status;
}

mama_status
mamaMiddlewareLibraryManager_stopAllBridges ()
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    if (0 == wInterlocked_read (&mwManager->mNumActive))
        return MAMA_STATUS_OK;

    mamaLibraryIterateCb cb = mamaMiddlewareLibraryManagerImpl_stopAllCb;
    mamaLibraryManager_iterateLibraries (MAMA_MIDDLEWARE_LIBRARY, cb, NULL);

    return MAMA_STATUS_OK;
}

const char*
mamaMiddlewareLibraryManager_getLibraryVersion (mamaMiddlewareLibrary mwLibrary)
{
    if (!mwLibrary)
        return NULL;

    mamaLibrary library = mwLibrary->mParent;

    const char* prop =
        mamaDefaultLibraryManager_getLibraryBridgeVersion (library);

    if (!(prop && *prop))
        prop = mwLibrary->mBridge->bridgeGetVersion ();

    return prop;
}

const char*
mamaMiddlewareLibraryManager_getLibraryMamaVersion (mamaMiddlewareLibrary mwLibrary)
{
    if (!mwLibrary)
        return NULL;

    mamaLibrary library = mwLibrary->mParent;

    return mamaDefaultLibraryManager_getLibraryBridgeMamaVersion (library);
}

mama_status
mamaMiddlewareLibraryManager_registerLoadCallback (mamaLibraryCb cb,
                                                   void*         closure)
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerLoadCallback (mwManager->mParent,
                                                    cb, closure);
}

mama_status
mamaMiddlewareLibraryManager_registerUnloadCallback (mamaLibraryCb cb,
                                                     void*         closure)
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerUnloadCallback (mwManager->mParent,
                                                      cb, closure);
}

mama_status
mamaMiddlewareLibraryManager_registerStartCallback (mamaLibraryCb cb,
                                                    void*         closure)
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_createCallbackSlot (mwManager->mParent,
                                                  mwManager->mStartSignal,
                                                  cb, closure);
}

mama_status
mamaMiddlewareLibraryManager_registerStopCallback (mamaLibraryCb cb,
                                                   void*         closure)
{
    mamaMiddlewareLibraryManager mwManager = NULL;
    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_createCallbackSlot (mwManager->mParent,
                                                  mwManager->mStopSignal,
                                                  cb, closure);
}

mama_status
mamaMiddlewareLibraryManager_libraryToMiddlewareLibrary (
        mamaLibrary            library,
        mamaMiddlewareLibrary* mwLibrary)
{
    if (!library || !mwLibrary)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_MIDDLEWARE_LIBRARY != library->mType)
        return MAMA_STATUS_INVALID_ARG;

    *mwLibrary = (mamaMiddlewareLibrary) library->mClosure;
    return MAMA_STATUS_OK;
}

mama_status
mamaMiddlewareLibraryManager_middlewareIdToString (char         middlewareId,
                                                   const char** str)
{
    if (!str)
        return MAMA_STATUS_NULL_ARG;

    *str = NULL;
    mamaMiddlewareLibraryManager mwManager = NULL;

    mama_status status =
        mamaMiddlewareLibraryManagerImpl_getInstance (&mwManager);

    if (status != MAMA_STATUS_OK)
        return status;

    mamaMiddlewareLibrary library = mwManager->mMiddleware[middlewareId];

    if (!library)
        return MAMA_STATUS_NOT_FOUND;

    *str = library->mParent->mName;
    return status;
}

mama_status
mamaMiddlewareLibraryManager_stringToMiddlewareId (const char*  str, 
                                                   char*        middlewareId)
{
    if (!str || !middlewareId)
        return MAMA_STATUS_NULL_ARG;

    *middlewareId = 0;
    mamaMiddlewareLibrary library = NULL;
    
    mama_status status =
        mamaMiddlewareLibraryManager_getLibrary (str, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *middlewareId = library->mMiddlewareId;
    return status;
}

mamaMiddleware
mamaMiddlewareLibraryManager_convertFromString (const char*  str)
{
    if (!str)
        return MAMA_MIDDLEWARE_UNKNOWN;
    
    if (strcasecmp (str, "wmw") == 0)
        return MAMA_MIDDLEWARE_WMW;
        
    if (strcasecmp (str, "lbm") == 0)
        return MAMA_MIDDLEWARE_LBM;
        
    if (strcasecmp (str, "tibrv") == 0)
        return MAMA_MIDDLEWARE_TIBRV;

    if (strcasecmp (str, "avis") == 0)
        return MAMA_MIDDLEWARE_AVIS;

    if (strcasecmp (str, "tick42blp") == 0)
        return MAMA_MIDDLEWARE_TICK42BLP;

    if (strcasecmp (str, "solace") == 0)
        return MAMA_MIDDLEWARE_SOLACE;
    
    if (strcasecmp (str, "rai") == 0)
        return MAMA_MIDDLEWARE_RAI;

    if (strcasecmp (str, "qpid") == 0)
        return MAMA_MIDDLEWARE_QPID;

    if (strcasecmp (str, "exegy") == 0)
        return MAMA_MIDDLEWARE_EXEGY;

    if (strcasecmp (str, "ibmwfo") == 0)
        return MAMA_MIDDLEWARE_IBMWFO;

    if (strcasecmp (str, "activ") == 0)
        return MAMA_MIDDLEWARE_ACTIV;

    if (strcasecmp (str, "tick42rmds") == 0)
        return MAMA_MIDDLEWARE_TICK42RMDS;

    if (strcasecmp (str, "ums") == 0)
        return MAMA_MIDDLEWARE_UMS;

    if (strcasecmp (str, "vulcan") == 0)
        return MAMA_MIDDLEWARE_VULCAN;

    if (strcasecmp (str, "inrush") == 0)
        return MAMA_MIDDLEWARE_INRUSH;

    if (strcasecmp (str, "lbmkomodo") == 0)
        return MAMA_MIDDLEWARE_LBMKOMODO;

    return MAMA_MIDDLEWARE_UNKNOWN;
}

const char*
mamaMiddlewareLibraryManager_convertToString (mamaMiddleware middleware)
{
    switch (middleware)
    {
        case MAMA_MIDDLEWARE_WMW:
            return "wmw";
        case MAMA_MIDDLEWARE_LBM:
            return "lbm";
        case MAMA_MIDDLEWARE_TIBRV:
            return "tibrv";
        case MAMA_MIDDLEWARE_AVIS:
            return "AVIS";
        case MAMA_MIDDLEWARE_TICK42BLP:
            return "tick42blp";
        case MAMA_MIDDLEWARE_SOLACE:
            return "SOLACE";
        case MAMA_MIDDLEWARE_RAI:
            return "rai";
        case MAMA_MIDDLEWARE_QPID:
            return "QPID";
        case MAMA_MIDDLEWARE_EXEGY:
            return "exegy";
        case MAMA_MIDDLEWARE_IBMWFO:
            return "ibmwfo";
        case MAMA_MIDDLEWARE_ACTIV:
            return "activ";
	    case MAMA_MIDDLEWARE_TICK42RMDS:
            return "tick42rmds";
        case MAMA_MIDDLEWARE_UMS:
            return "ums";
        case MAMA_MIDDLEWARE_VULCAN:
            return "vulcan";
        case MAMA_MIDDLEWARE_INRUSH:
            return "inrush";
        case MAMA_MIDDLEWARE_LBMKOMODO:
            return "lbmkomodo";
        default:
            return "unknown";
    }
}

mama_status
mamaMiddlewareLibraryManager_convertLibraryToBridge (mamaMiddlewareLibrary library, 
                                                     mamaBridge*           bridge)
{
    assert (library);
    assert (bridge);

    *bridge = library->mBridge;
    return MAMA_STATUS_OK;
}
