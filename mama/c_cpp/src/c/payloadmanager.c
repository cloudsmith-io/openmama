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

#include "payloadmanager.h"
#include "mama/payloadmanager.h"

#include <assert.h>
#include <platform.h>
#include <wombat/wInterlocked.h>

#include "payloadbridge.h"
#include "librarymanager.h"

/*
 * Private types
 */

typedef struct mamaPayloadLibraryManagerImpl* mamaPayloadLibraryManager;

typedef struct mamaPayloadLibraryImpl_
{
    mamaLibrary               mParent;
    mamaPayloadBridge         mBridge;
    char                      mPayloadId;
    mamaPayloadLibraryManager mManager;
} mamaPayloadLibraryImpl;

typedef struct mamaPayloadLibraryManagerImpl
{
    mamaLibraryTypeManager mParent;
    mamaPayloadLibrary     mPayloads [MAX_LIBRARIES];
} mamaPayloadLibraryManagerImpl;

/*
 * Private declarations
 */

static mama_status
mamaPayloadLibraryManagerImpl_getInstance (
        mamaPayloadLibraryManager* plManager);

static mama_status
mamaPayloadLibraryManagerImpl_getLibraries (mamaPayloadLibrary*    libraries,
                                            mama_size_t*           size,
                                            mamaLibraryPredicateCb predicate);

static void*
mamaPayloadLibraryManagerImpl_loadFunction (const char* libraryName,
                                            LIB_HANDLE  libraryHandle,
                                            const char* funcName);

static void*
mamaPayloadLibraryManagerImpl_loadLibraryFunction (mamaLibrary library,
                                                   const char* funcName);

static msgPayload_createImpl
mamaPayloadLibraryManagerImpl_getCreateImpl (const char* libraryName,
                                             LIB_HANDLE  libraryHandle);


static msgPayload_destroyImpl
mamaPayloadLibraryManagerImpl_getDestroyImpl (const char* libraryName,
                                              LIB_HANDLE  libraryHandle);

static msgPayload_createImpl
mamaPayloadLibraryManagerImpl_getLibraryCreateImpl (mamaLibrary library);

static msgPayload_destroyImpl
mamaPayloadLibraryManagerImpl_getLibraryDestroyImpl (mamaLibrary library);

static mama_status
mamaPayloadLibraryManagerImpl_createBridge (mamaLibrary        library,
                                            mamaPayloadBridge* bridge0,
                                            char*              payloadId);

static void
mamaPayloadLibraryManager_destroyBridge (mamaPayloadBridge bridge);

static mama_status
mamaPayloadLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaPayloadLibrary* plLibrary0);

static void
mamaPayloadLibraryManagerImpl_destroyLibrary (mamaPayloadLibrary plLibrary);

static mama_status
mamaPayloadLibraryManagerImpl_getPayloadId (mamaPayloadLibrary plLibrary,
                                            char*              payloadId);

/*
 * Private implementation
 */

static mama_status
mamaPayloadLibraryManagerImpl_getInstance (
        mamaPayloadLibraryManager* plManager)
{
     if (!plManager)
         return MAMA_STATUS_NULL_ARG;

     mamaLibraryTypeManager manager = NULL;
     mama_status status =
         mamaLibraryManager_getTypeManager (MAMA_PAYLOAD_LIBRARY, &manager);

     if (MAMA_STATUS_OK == status)
         *plManager = (mamaPayloadLibraryManager) manager->mClosure;
     else
         *plManager = NULL;

     return status;
}

static void*
mamaPayloadLibraryManagerImpl_loadFunction (const char* libraryName,
                                            LIB_HANDLE  libraryHandle,
                                            const char* funcName)
{
    /* Build target function spec */
    char funcSpec [MAX_LIBRARY_FUNCTION_NAME];
    if (snprintf (funcSpec, MAX_LIBRARY_FUNCTION_NAME-1, "%s%%s",
                  libraryName) < 0)
    {
        return NULL;
    }

    const char* funcAltSpec = "default%s";

    return mamaLibraryManager_loadFunction (libraryHandle,
                                            funcSpec,
                                            funcName,
                                            funcAltSpec,
                                            NULL, /* funcAltName */
                                            NULL  /* default */);
}

static void*
mamaPayloadLibraryManagerImpl_loadLibraryFunction (mamaLibrary library,
                                                   const char* funcName)
{
    return
        mamaPayloadLibraryManagerImpl_loadFunction (library->mName,
                                                    library->mHandle,
                                                    funcName);
}

static msgPayload_createImpl
mamaPayloadLibraryManagerImpl_getCreateImpl (const char* libraryName,
                                             LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaPayloadLibraryManagerImpl_loadFunction (libraryName,
                                                    libraryHandle,
                                                    "Payload_createImpl");
   return *(msgPayload_createImpl*) &func;
}

static msgPayload_destroyImpl
mamaPayloadLibraryManagerImpl_getDestroyImpl (const char* libraryName,
                                              LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaPayloadLibraryManagerImpl_loadFunction (libraryName,
                                                    libraryHandle,
                                                    "Payload_destroyImpl");
 

    return *(msgPayload_destroyImpl*)&func;
}

static msgPayload_createImpl
mamaPayloadLibraryManagerImpl_getLibraryCreateImpl (mamaLibrary library)
{
    return (msgPayload_createImpl)
        mamaPayloadLibraryManagerImpl_getCreateImpl (library->mName,
                                                     library->mHandle);
}

static msgPayload_destroyImpl
mamaPayloadLibraryManagerImpl_getLibraryDestroyImpl (mamaLibrary library)
{
    return (msgPayload_destroyImpl)
        mamaPayloadLibraryManagerImpl_getDestroyImpl (library->mName,
                                                      library->mHandle);
}

static mama_status
mamaPayloadLibraryManagerImpl_getLibraries (mamaPayloadLibrary* libraries,
                                            mama_size_t*           size,
                                            mamaLibraryPredicateCb predicate)
{
    if (!libraries || !size)
        return MAMA_STATUS_NULL_ARG;

    return mamaLibraryManager_getLibraries ((mamaLibrary*) libraries, size,
                                            MAMA_PAYLOAD_LIBRARY, predicate);
}

static mama_status
mamaPayloadLibraryManagerImpl_getBridges (mamaPayloadBridge*     bridges,
                                          mama_size_t*           size,
                                          mamaLibraryPredicateCb predicate)
{
    if (!bridges || !size)
        return MAMA_STATUS_NULL_ARG;

    mamaPayloadLibrary libraries [MAX_LIBRARIES];
    mama_size_t librariesSize = MAX_LIBRARIES;

    mama_status status =
        mamaPayloadLibraryManagerImpl_getLibraries (libraries,
                                                    &librariesSize,
                                                    predicate);

    for (mama_size_t k = 0; k < librariesSize && k < *size; ++k)
        bridges[k] = libraries[k]->mBridge;

    if (librariesSize < *size)
        *size = librariesSize;

    return status;
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
mamaPayloadLibraryManagerImpl_createBridge (mamaLibrary        library,
                                            mamaPayloadBridge* bridge0,
                                            char*              payloadId)
{
    assert (bridge0);
    assert (payloadId);
    *bridge0 = NULL;

    mamaPayloadBridge bridge = (mamaPayloadBridge)
        calloc (1, sizeof (mamaPayloadBridgeImpl));

    if (!bridge)
        return MAMA_STATUS_NOMEM;

    msgPayload_createImpl createImpl =
        mamaPayloadLibraryManagerImpl_getLibraryCreateImpl (library);

    if (createImpl)
    {
        /* FIXME: We might need to make a carbon copy of mamaPayloadBridge
         * and make it mamaPayloadOldBridge if we make any changes that
         * would affect binary-compatibility between structures, other than
         * adding things on to the end of the structure. */
        mamaPayloadBridge oldBridge = NULL;
        mama_status status = createImpl (&oldBridge, payloadId);

        if (MAMA_STATUS_OK != status || !oldBridge)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaPayloadLibraryManager_createBridge(): "
                      "Could not initialise %s library %s bridge using "
                      "old-style allocation.",
                      library->mTypeName, library->mName);

            free (bridge);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        }

        /* Copy relevant parts of bridge.
         * FIXME: Should we have a copy function? */
        bridge->mClosure = oldBridge->mClosure;
        bridge->closure  = oldBridge->closure;

        /* Unload the old-style bridge.
         * FIXME: We possibly should keep hold of the memory until
         * we unload the new-style bridge, in case this does more
         * than just freeing the bridge struct. */
        msgPayload_destroyImpl destroyImpl =
            mamaPayloadLibraryManagerImpl_getLibraryDestroyImpl (library);

        if (destroyImpl)
            destroyImpl (oldBridge);
        else
            free (oldBridge);
    }

    *bridge0 = bridge;

    return MAMA_STATUS_OK;
}

static void
mamaPayloadLibraryManager_destroyBridge (mamaPayloadBridge bridge)
{
    free (bridge);
}

static mama_status
mamaPayloadLibraryManagerImpl_createLibrary (
        mamaLibrary library, mamaPayloadLibrary* plLibrary0)
{
    assert (plLibrary0);
    *plLibrary0 = NULL;

    mamaPayloadLibraryManager plManager = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary plLibrary =
        (mamaPayloadLibrary) calloc (1, sizeof (mamaPayloadLibraryImpl));

    if (!plLibrary)
        return MAMA_STATUS_NOMEM;

    mamaPayloadBridge bridge = NULL;
    char payloadId = '\0';
    status = mamaPayloadLibraryManagerImpl_createBridge (library, &bridge,
                                                         &payloadId);

    if (MAMA_STATUS_OK != status)
    {
        free (plLibrary);
        return status;
    }

    /* Establish bi-directional links */
    bridge->mLibrary = plLibrary;
    library->mClosure = plLibrary;

    plLibrary->mPayloadId = payloadId;
    plLibrary->mBridge    = bridge;
    plLibrary->mParent    = library;
    plLibrary->mManager   = plManager;

    *plLibrary0 = plLibrary;
    return MAMA_STATUS_OK;
}

static void
mamaPayloadLibraryManagerImpl_destroyLibrary (
        mamaPayloadLibrary plLibrary)
{
    mamaPayloadLibraryManager_destroyBridge (plLibrary->mBridge);

    plLibrary->mParent->mClosure = NULL;
    free (plLibrary);
}

static mama_status
mamaPayloadLibraryManagerImpl_getPayloadId (mamaPayloadLibrary plLibrary,
                                            char*              payloadId)
{
    assert (plLibrary);
    assert (payloadId);
    *payloadId = 0;

    mamaLibrary library = plLibrary->mParent;

    const char* prop =
        mamaLibraryManager_getLibraryProperty (library, "id");

    if (prop)
    {
        if (*payloadId)
        {
            mama_log (MAMA_LOG_LEVEL_FINE,
                      "mamaPayloadLibraryManagerImpl_getPayloadId(): "
                      "Overriding payload bridge ID [%c] "
                      "with configured ID [%c]",
                      *payloadId, prop[0]);
        }

        *payloadId = prop[0];
        return MAMA_STATUS_OK;
    }

    *payloadId = plLibrary->mBridge->msgPayloadGetType ();
    return (*payloadId ? MAMA_STATUS_OK : MAMA_STATUS_SYSTEM_ERROR);
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaPayloadLibraryManager_create (mamaLibraryTypeManager manager)
{
    mamaPayloadLibraryManager plManager = (mamaPayloadLibraryManager)
        calloc (1, sizeof (mamaPayloadLibraryManagerImpl));

    if (!plManager)
        return MAMA_STATUS_NOMEM;

    /* Establish bi-directional link */
    manager->mClosure = plManager;
    plManager->mParent = manager;

    return MAMA_STATUS_OK;
}

void
mamaPayloadLibraryManager_destroy (void)
{
    mamaPayloadLibraryManager plManager = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK == status)
    {
        plManager->mParent->mClosure = NULL;
        free (plManager);
    }
}

#define REGISTER_BRIDGE_FUNCTION(funcName, bridgeFuncName)\
do {\
    if (MAMA_STATUS_OK == status)\
    {\
        void* func = \
            mamaPayloadLibraryManagerImpl_loadLibraryFunction\
                    (library, #funcName);\
        bridge->bridgeFuncName = *(msg ## funcName*) &func;\
        if (!bridge->bridgeFuncName)\
        {\
            mama_log (MAMA_LOG_LEVEL_ERROR,\
                      "mamaLibraryMiddlewareManager_loadLibrary(): "\
                      "Could not load %s library %s because "\
                      "required function [%s] is missing in bridge.",\
                      library->mTypeName, library->mName,\
                      #funcName);\
            status = MAMA_STATUS_NO_BRIDGE_IMPL;\
        }\
    }\
} while (0);

mama_status
mamaPayloadLibraryManager_loadLibrary (mamaLibrary library)
{
    mamaPayloadLibrary plLibrary = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_createLibrary (library,
                                                     &plLibrary);

    if (MAMA_STATUS_OK != status)
    {
        return status;
    }

    mamaPayloadBridge bridge = plLibrary->mBridge;

    mamaPayloadLibraryManager plManager = NULL;
    status = mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
    {
        free (bridge);
        return status;
    }

    plLibrary->mManager = plManager;

    /* Once the bridge has been successfully loaded, and the initialization
     * function called, we register each of the required bridge functions.
     */
    REGISTER_BRIDGE_FUNCTION (Payload_create,
                              msgPayloadCreate);

    REGISTER_BRIDGE_FUNCTION (Payload_createForTemplate,
                              msgPayloadCreateForTemplate);

    REGISTER_BRIDGE_FUNCTION (Payload_getType,
                              msgPayloadGetType);

    REGISTER_BRIDGE_FUNCTION (Payload_copy,
                              msgPayloadCopy);

    REGISTER_BRIDGE_FUNCTION (Payload_clear,
                              msgPayloadClear);

    REGISTER_BRIDGE_FUNCTION (Payload_destroy,
                              msgPayloadDestroy);

    REGISTER_BRIDGE_FUNCTION (Payload_setParent,
                              msgPayloadSetParent);

    REGISTER_BRIDGE_FUNCTION (Payload_getByteSize,
                              msgPayloadGetByteSize);

    REGISTER_BRIDGE_FUNCTION (Payload_getNumFields,
                              msgPayloadGetNumFields);

    REGISTER_BRIDGE_FUNCTION (Payload_getSendSubject,
                              msgPayloadGetSendSubject);

    REGISTER_BRIDGE_FUNCTION (Payload_toString,
                              msgPayloadToString);

    REGISTER_BRIDGE_FUNCTION (Payload_iterateFields,
                              msgPayloadIterateFields);

    REGISTER_BRIDGE_FUNCTION (Payload_serialize,
                              msgPayloadSerialize);

    REGISTER_BRIDGE_FUNCTION (Payload_unSerialize,
                              msgPayloadUnSerialize);

    REGISTER_BRIDGE_FUNCTION (Payload_getByteBuffer,
                              msgPayloadGetByteBuffer);

    REGISTER_BRIDGE_FUNCTION (Payload_setByteBuffer,
                              msgPayloadSetByteBuffer);

    REGISTER_BRIDGE_FUNCTION (Payload_createFromByteBuffer,
                              msgPayloadCreateFromByteBuffer);

    REGISTER_BRIDGE_FUNCTION (Payload_apply,
                              msgPayloadApply);


    /*Msg payload get/update functions*/
    REGISTER_BRIDGE_FUNCTION (Payload_getNativeMsg,
                              msgPayloadGetNativeMsg);

    REGISTER_BRIDGE_FUNCTION (Payload_getFieldAsString,
                              msgPayloadGetFieldAsString);

    REGISTER_BRIDGE_FUNCTION (Payload_addBool,
                              msgPayloadAddBool);

    REGISTER_BRIDGE_FUNCTION (Payload_addChar,
                              msgPayloadAddChar);

    REGISTER_BRIDGE_FUNCTION (Payload_addI8,
                              msgPayloadAddI8);

    REGISTER_BRIDGE_FUNCTION (Payload_addU8,
                              msgPayloadAddU8);

    REGISTER_BRIDGE_FUNCTION (Payload_addI16,
                              msgPayloadAddI16);

    REGISTER_BRIDGE_FUNCTION (Payload_addU16,
                              msgPayloadAddU16);

    REGISTER_BRIDGE_FUNCTION (Payload_addI32,
                              msgPayloadAddI32);

    REGISTER_BRIDGE_FUNCTION (Payload_addU32,
                              msgPayloadAddU32);

    REGISTER_BRIDGE_FUNCTION (Payload_addI64,
                              msgPayloadAddI64);

    REGISTER_BRIDGE_FUNCTION (Payload_addU64,
                              msgPayloadAddU64);

    REGISTER_BRIDGE_FUNCTION (Payload_addF32,
                              msgPayloadAddF32);

    REGISTER_BRIDGE_FUNCTION (Payload_addF64,
                              msgPayloadAddF64);

    REGISTER_BRIDGE_FUNCTION (Payload_addString,
                              msgPayloadAddString);

    REGISTER_BRIDGE_FUNCTION (Payload_addOpaque,
                              msgPayloadAddOpaque);

    REGISTER_BRIDGE_FUNCTION (Payload_addDateTime,
                              msgPayloadAddDateTime);

    REGISTER_BRIDGE_FUNCTION (Payload_addPrice,
                              msgPayloadAddPrice);

    REGISTER_BRIDGE_FUNCTION (Payload_addMsg,
                              msgPayloadAddMsg);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorBool,
                              msgPayloadAddVectorBool);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorChar,
                              msgPayloadAddVectorChar);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorI8,
                              msgPayloadAddVectorI8);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorU8,
                              msgPayloadAddVectorU8);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorI16,
                              msgPayloadAddVectorI16);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorU16,
                              msgPayloadAddVectorU16);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorI32,
                              msgPayloadAddVectorI32);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorU32,
                              msgPayloadAddVectorU32);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorI64,
                              msgPayloadAddVectorI64);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorU64,
                              msgPayloadAddVectorU64);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorF32,
                              msgPayloadAddVectorF32);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorF64,
                              msgPayloadAddVectorF64);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorString,
                              msgPayloadAddVectorString);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorMsg,
                              msgPayloadAddVectorMsg);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorDateTime,
                              msgPayloadAddVectorDateTime);

    REGISTER_BRIDGE_FUNCTION (Payload_addVectorPrice,
                              msgPayloadAddVectorPrice);

    REGISTER_BRIDGE_FUNCTION (Payload_updateBool,
                              msgPayloadUpdateBool);

    REGISTER_BRIDGE_FUNCTION (Payload_updateChar,
                              msgPayloadUpdateChar);

    REGISTER_BRIDGE_FUNCTION (Payload_updateU8,
                              msgPayloadUpdateU8);

    REGISTER_BRIDGE_FUNCTION (Payload_updateI8,
                              msgPayloadUpdateI8);

    REGISTER_BRIDGE_FUNCTION (Payload_updateI16,
                              msgPayloadUpdateI16);

    REGISTER_BRIDGE_FUNCTION (Payload_updateU16,
                              msgPayloadUpdateU16);

    REGISTER_BRIDGE_FUNCTION (Payload_updateI32,
                              msgPayloadUpdateI32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateU32,
                              msgPayloadUpdateU32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateI64,
                              msgPayloadUpdateI64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateU64,
                              msgPayloadUpdateU64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateF32,
                              msgPayloadUpdateF32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateF64,
                              msgPayloadUpdateF64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateString,
                              msgPayloadUpdateString);

    REGISTER_BRIDGE_FUNCTION (Payload_updateOpaque,
                              msgPayloadUpdateOpaque);

    REGISTER_BRIDGE_FUNCTION (Payload_updateDateTime,
                              msgPayloadUpdateDateTime);

    REGISTER_BRIDGE_FUNCTION (Payload_updatePrice,
                              msgPayloadUpdatePrice);

    REGISTER_BRIDGE_FUNCTION (Payload_updateSubMsg,
                              msgPayloadUpdateSubMsg);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorMsg,
                              msgPayloadUpdateVectorMsg);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorString,
                              msgPayloadUpdateVectorString);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorBool,
                              msgPayloadUpdateVectorBool);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorChar,
                              msgPayloadUpdateVectorChar);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorI8,
                              msgPayloadUpdateVectorI8);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorU8,
                              msgPayloadUpdateVectorU8);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorI16,
                              msgPayloadUpdateVectorI16);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorU16,
                              msgPayloadUpdateVectorU16);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorI32,
                              msgPayloadUpdateVectorI32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorU32,
                              msgPayloadUpdateVectorU32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorI64,
                              msgPayloadUpdateVectorI64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorU64,
                              msgPayloadUpdateVectorU64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorF32,
                              msgPayloadUpdateVectorF32);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorF64,
                              msgPayloadUpdateVectorF64);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorPrice,
                              msgPayloadUpdateVectorPrice);

    REGISTER_BRIDGE_FUNCTION (Payload_updateVectorTime,
                              msgPayloadUpdateVectorTime);


    /*Msg payload get functions*/
    REGISTER_BRIDGE_FUNCTION (Payload_getBool,
                              msgPayloadGetBool);

    REGISTER_BRIDGE_FUNCTION (Payload_getChar,
                              msgPayloadGetChar);

    REGISTER_BRIDGE_FUNCTION (Payload_getI8,
                              msgPayloadGetI8);

    REGISTER_BRIDGE_FUNCTION (Payload_getU8,
                              msgPayloadGetU8);

    REGISTER_BRIDGE_FUNCTION (Payload_getI16,
                              msgPayloadGetI16);

    REGISTER_BRIDGE_FUNCTION (Payload_getU16,
                              msgPayloadGetU16);

    REGISTER_BRIDGE_FUNCTION (Payload_getI32,
                              msgPayloadGetI32);

    REGISTER_BRIDGE_FUNCTION (Payload_getU32,
                              msgPayloadGetU32);

    REGISTER_BRIDGE_FUNCTION (Payload_getI64,
                              msgPayloadGetI64);

    REGISTER_BRIDGE_FUNCTION (Payload_getU64,
                              msgPayloadGetU64);

    REGISTER_BRIDGE_FUNCTION (Payload_getF32,
                              msgPayloadGetF32);

    REGISTER_BRIDGE_FUNCTION (Payload_getF64,
                              msgPayloadGetF64);

    REGISTER_BRIDGE_FUNCTION (Payload_getString,
                              msgPayloadGetString);

    REGISTER_BRIDGE_FUNCTION (Payload_getOpaque,
                              msgPayloadGetOpaque);

    REGISTER_BRIDGE_FUNCTION (Payload_getField,
                              msgPayloadGetField);

    REGISTER_BRIDGE_FUNCTION (Payload_getDateTime,
                              msgPayloadGetDateTime);

    REGISTER_BRIDGE_FUNCTION (Payload_getPrice,
                              msgPayloadGetPrice);

    REGISTER_BRIDGE_FUNCTION (Payload_getMsg,
                              msgPayloadGetMsg);


    /*Msg payload get vector functions*/
    REGISTER_BRIDGE_FUNCTION (Payload_getVectorBool,
                              msgPayloadGetVectorBool);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorChar,
                              msgPayloadGetVectorChar);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorI8,
                              msgPayloadGetVectorI8);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorU8,
                              msgPayloadGetVectorU8);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorI16,
                              msgPayloadGetVectorI16);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorU16,
                              msgPayloadGetVectorU16);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorI32,
                              msgPayloadGetVectorI32);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorU32,
                              msgPayloadGetVectorU32);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorI64,
                              msgPayloadGetVectorI64);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorU64,
                              msgPayloadGetVectorU64);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorF32,
                              msgPayloadGetVectorF32);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorF64,
                              msgPayloadGetVectorF64);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorString,
                              msgPayloadGetVectorString);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorDateTime,
                              msgPayloadGetVectorDateTime);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorPrice,
                              msgPayloadGetVectorPrice);

    REGISTER_BRIDGE_FUNCTION (Payload_getVectorMsg,
                              msgPayloadGetVectorMsg);


    /*Field payload get/update functions*/
    REGISTER_BRIDGE_FUNCTION (FieldPayload_create,
                              msgFieldPayloadCreate);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_destroy,
                              msgFieldPayloadDestroy);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getName,
                              msgFieldPayloadGetName);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getFid,
                              msgFieldPayloadGetFid);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getDescriptor,
                              msgFieldPayloadGetDescriptor);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getType,
                              msgFieldPayloadGetType);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateBool,
                              msgFieldPayloadUpdateBool);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateChar,
                              msgFieldPayloadUpdateChar);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateU8,
                              msgFieldPayloadUpdateU8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateI8,
                              msgFieldPayloadUpdateI8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateI16,
                              msgFieldPayloadUpdateI16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateU16,
                              msgFieldPayloadUpdateU16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateI32,
                              msgFieldPayloadUpdateI32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateU32,
                              msgFieldPayloadUpdateU32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateI64,
                              msgFieldPayloadUpdateI64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateU64,
                              msgFieldPayloadUpdateU64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateF32,
                              msgFieldPayloadUpdateF32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateF64,
                              msgFieldPayloadUpdateF64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateString,
                              msgFieldPayloadUpdateString);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updateDateTime,
                              msgFieldPayloadUpdateDateTime);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_updatePrice,
                              msgFieldPayloadUpdatePrice);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getBool,
                              msgFieldPayloadGetBool);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getChar,
                              msgFieldPayloadGetChar);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getI8,
                              msgFieldPayloadGetI8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getU8,
                              msgFieldPayloadGetU8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getI16,
                              msgFieldPayloadGetI16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getU16,
                              msgFieldPayloadGetU16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getI32,
                              msgFieldPayloadGetI32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getU32,
                              msgFieldPayloadGetU32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getI64,
                              msgFieldPayloadGetI64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getU64,
                              msgFieldPayloadGetU64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getF32,
                              msgFieldPayloadGetF32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getF64,
                              msgFieldPayloadGetF64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getString,
                              msgFieldPayloadGetString);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getOpaque,
                              msgFieldPayloadGetOpaque);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getDateTime,
                              msgFieldPayloadGetDateTime);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getPrice,
                              msgFieldPayloadGetPrice);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getMsg,
                              msgFieldPayloadGetMsg);


    /*Field payload vector related functions*/
    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorBool,
                              msgFieldPayloadGetVectorBool);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorChar,
                              msgFieldPayloadGetVectorChar);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorI8,
                              msgFieldPayloadGetVectorI8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorU8,
                              msgFieldPayloadGetVectorU8);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorI16,
                              msgFieldPayloadGetVectorI16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorU16,
                              msgFieldPayloadGetVectorU16);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorI32,
                              msgFieldPayloadGetVectorI32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorU32,
                              msgFieldPayloadGetVectorU32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorI64,
                              msgFieldPayloadGetVectorI64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorU64,
                              msgFieldPayloadGetVectorU64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorF32,
                              msgFieldPayloadGetVectorF32);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorF64,
                              msgFieldPayloadGetVectorF64);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorString,
                              msgFieldPayloadGetVectorString);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorDateTime,
                              msgFieldPayloadGetVectorDateTime);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorPrice,
                              msgFieldPayloadGetVectorPrice);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getVectorMsg,
                              msgFieldPayloadGetVectorMsg);

    REGISTER_BRIDGE_FUNCTION (FieldPayload_getAsString,
                              msgFieldPayloadGetAsString);


    /*Iterator functions*/
    REGISTER_BRIDGE_FUNCTION (PayloadIter_create,
                              msgPayloadIterCreate);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_next,
                              msgPayloadIterNext);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_hasNext,
                              msgPayloadIterHasNext);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_begin,
                              msgPayloadIterBegin);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_end,
                              msgPayloadIterEnd);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_associate,
                              msgPayloadIterAssociate);

    REGISTER_BRIDGE_FUNCTION (PayloadIter_destroy,
                              msgPayloadIterDestroy);

    if (MAMA_STATUS_OK != status)
    {
        free (bridge);
        return status;
    }

    status =
        mamaPayloadLibraryManagerImpl_getPayloadId (plLibrary,
                                                    &plLibrary->mPayloadId);

    if (MAMA_STATUS_OK != status)
    {
        free (bridge);
        return status;
    }

    mamaPayloadLibrary dupLibrary = plManager->mPayloads[plLibrary->mPayloadId];

    if (dupLibrary)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaPayloadLibraryManager_loadLibrary(): "
                  "Payload id [%d] for %s library %s duplicates library %s.",
                  plLibrary->mPayloadId, library->mTypeName,
                  library->mName,
                  dupLibrary->mParent->mName);

        free (bridge);
        return MAMA_STATUS_PLATFORM;
    }

    /* Store indexed lookup */
    plManager->mPayloads[plLibrary->mPayloadId] = plLibrary;

    if (!mamaInternal_getDefaultPayload ())
        mamaInternal_setDefaultPayload (bridge);

    return status;
}

void
mamaPayloadLibraryManager_unloadLibrary (mamaLibrary library)
{
    mamaPayloadLibrary plLibrary = ((mamaPayloadLibrary) library->mClosure);
    mamaPayloadLibraryManager plManager = plLibrary->mManager;

    char payloadId = plLibrary->mPayloadId;
    mamaPayloadLibraryManagerImpl_destroyLibrary (plLibrary);
    plManager->mPayloads[payloadId] = NULL;
}

mamaLibraryType
mamaPayloadLibraryManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib)
{
    msgPayload_createImpl createImpl =
        mamaPayloadLibraryManagerImpl_getCreateImpl (libraryName,
                                                     libraryLib);

    msgPayload_destroyImpl destroyImpl =
        mamaPayloadLibraryManagerImpl_getDestroyImpl (libraryName,
                                                      libraryLib);

    if (createImpl || destroyImpl)
        return MAMA_PAYLOAD_LIBRARY;

    return MAMA_UNKNOWN_LIBRARY;
}

/*
 * Public implementation
 */

mama_status
mamaPayloadLibraryManager_loadBridge (const char*        payloadName,
                                      const char*        path,
                                      mamaPayloadBridge* bridge)
{
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_loadLibrary (payloadName,
                                        MAMA_PAYLOAD_LIBRARY,
                                        path, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary plLibrary =
        (mamaPayloadLibrary) library->mClosure;

    *bridge = plLibrary->mBridge;
    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadLibraryManager_getBridge (const char*        payloadName,
                                     mamaPayloadBridge* bridge)
{
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_getLibrary (payloadName,
                                       MAMA_PAYLOAD_LIBRARY,
                                       &library);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary plLibrary =
        (mamaPayloadLibrary) library->mClosure;

    *bridge = plLibrary->mBridge;
    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadLibraryManager_getDefaultBridge (mamaPayloadBridge* bridge)
{
    if (!bridge)
        return MAMA_STATUS_NULL_ARG;

    *bridge = mamaInternal_getDefaultPayload ();
    return (*bridge ? MAMA_STATUS_OK : MAMA_STATUS_NOT_FOUND);
}

mama_status
mamaPayloadLibraryManager_getBridgeById (char               payloadId,
                                         mamaPayloadBridge* bridge)
{
    *bridge = NULL;

    mamaPayloadLibraryManager plManager = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary library =
        plManager->mPayloads[payloadId];

    if (!library)
        return MAMA_STATUS_NO_BRIDGE_IMPL;

    *bridge = library->mBridge;
    return (*bridge ? MAMA_STATUS_OK : MAMA_STATUS_NO_BRIDGE_IMPL);
}

mama_status
mamaPayloadLibraryManager_getLibraries (mamaPayloadLibrary* libraries,
                                        mama_size_t*        size)
{
    return mamaPayloadLibraryManagerImpl_getLibraries (libraries, size,
                                                       NULL);
}

mama_status
mamaPayloadLibraryManager_getBridges (mamaPayloadBridge*     bridges,
                                      mama_size_t*           size)
{
    return mamaPayloadLibraryManagerImpl_getBridges (bridges, size, NULL);
}

mama_status
mamaPayloadLibraryManager_registerLoadCallback (mamaLibraryCb cb,
                                                void*         closure)
{
    mamaPayloadLibraryManager plManager = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerLoadCallback (plManager->mParent,
                                                    cb, closure);
}

mama_status
mamaPayloadLibraryManager_registerUnloadCallback (mamaLibraryCb cb,
                                                  void*         closure)
{
    mamaPayloadLibraryManager plManager = NULL;
    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerUnloadCallback (plManager->mParent,
                                                      cb, closure);
}

mama_status
mamaPayloadLibraryManager_libraryToPayloadLibrary (
        mamaLibrary         library,
        mamaPayloadLibrary* plLibrary)
{
    if (!library || !plLibrary)
        return MAMA_STATUS_NULL_ARG;

    if (MAMA_PAYLOAD_LIBRARY != library->mType)
        return MAMA_STATUS_INVALID_ARG;

    *plLibrary = (mamaPayloadLibrary) library->mClosure;
    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadLibraryManager_payloadIdToString (char         payloadId,
                                             const char** str)
{
    if (!str)
        return MAMA_STATUS_NULL_ARG;

    *str = NULL;
    mamaPayloadLibraryManager plManager = NULL;

    mama_status status =
        mamaPayloadLibraryManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary library = plManager->mPayloads[payloadId];

    if (!library)
        return MAMA_STATUS_NOT_FOUND;

    *str = library->mParent->mName;
    return status;
}

mama_status
mamaPayloadLibraryManager_stringToPayloadId (const char* str, 
                                             char*       payloadId)
{
    if (!str || !payloadId)
        return MAMA_STATUS_NULL_ARG;

    *payloadId = 0;
    mamaPayloadBridge bridge = NULL;

    mama_status status =
        mamaPayloadLibraryManager_getBridge (str, &bridge);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary library = bridge->mLibrary;
    *payloadId = library->mPayloadId;
    return status;
}

