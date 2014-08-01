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
 * Default Payload
 */
static mamaPayloadLibrary gDefaultPayload = NULL;

/*
 * Private types
 */

typedef struct mamaPayloadManagerImpl* mamaPayloadManager;

typedef struct mamaPayloadLibraryImpl_
{
    mamaLibrary               mParent;
    mamaPayloadBridge         mBridge;
    wInterlockedInt           mActiveCount;
    char                      mPayloadId;
    mamaPayloadManager mManager;
} mamaPayloadLibraryImpl;

typedef struct mamaPayloadManagerImpl
{
    mamaLibraryTypeManager mParent;
    mamaPayloadLibrary     mPayloads [MAMA_MAX_LIBRARIES];
} mamaPayloadManagerImpl;

/*
 * Private declarations
 */

static mama_status
mamaPayloadManagerImpl_getInstance (
        mamaPayloadManager* plManager);

static mama_status
mamaPayloadManagerImpl_getLibraries (mamaPayloadLibrary*    libraries,
                                            mama_size_t*           size,
                                            mamaLibraryPredicateCb predicate);

static Payload_createImpl
mamaPayloadManagerImpl_getCreateImpl (const char* libraryName,
                                             LIB_HANDLE  libraryHandle);

static Payload_destroyImpl
mamaPayloadManagerImpl_getDestroyImpl (const char* libraryName,
                                              LIB_HANDLE  libraryHandle);

static Payload_load
mamaPayloadManagerImpl_getLoad (const char* libraryName,
                                       LIB_HANDLE  libraryHandle);
static Payload_unload
mamaPayloadManagerImpl_getUnload (const char* libraryName,
                                         LIB_HANDLE  libraryHandle);
static Payload_createImpl
mamaPayloadManagerImpl_getLibraryCreateImpl (mamaLibrary library);

static Payload_destroyImpl
mamaPayloadManagerImpl_getLibraryDestroyImpl (mamaLibrary library);

static Payload_load
mamaPayloadManagerImpl_getLibraryLoad (mamaLibrary library);

static Payload_unload
mamaPayloadManagerImpl_getLibraryUnload (mamaLibrary library);

static mama_status
mamaPayloadManagerImpl_createBridge (mamaLibrary        library,
                                            mamaPayloadBridge* bridge0);

static mama_status
mamaPayloadManagerImpl_loadBridge (mamaPayloadLibrary plLibrary);

static void
mamaPayloadManager_destroyBridge (mamaPayloadBridge bridge);

static void
mamaPayloadManager_deactivateLibrary (mamaPayloadLibrary library);

static mama_status
mamaPayloadManagerImpl_createLibrary (
        mamaLibrary library, mamaPayloadLibrary* plLibrary0);

static void
mamaPayloadManagerImpl_destroyLibrary (mamaPayloadLibrary plLibrary);

static mama_status
mamaPayloadManagerImpl_getPayloadId (mamaPayloadLibrary plLibrary,
                                            char*              payloadId);

/*
 * Private implementation
 */

static mama_status
mamaPayloadManagerImpl_getInstance (
        mamaPayloadManager* plManager)
{
     if (!plManager)
         return MAMA_STATUS_NULL_ARG;

     mamaLibraryTypeManager manager = NULL;
     mama_status status =
         mamaLibraryManager_getTypeManager (MAMA_PAYLOAD_LIBRARY, &manager);

     if (MAMA_STATUS_OK == status)
         *plManager = (mamaPayloadManager) manager->mClosure;
     else
         *plManager = NULL;

     return status;
}

static Payload_createImpl
mamaPayloadManagerImpl_getCreateImpl (const char* libraryName,
                                             LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Payload_createImpl");
   return *(Payload_createImpl*) &func;
}

static Payload_destroyImpl
mamaPayloadManagerImpl_getDestroyImpl (const char* libraryName,
                                              LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Payload_destroyImpl");
 
    return *(Payload_destroyImpl*)&func;
}

static Payload_load
mamaPayloadManagerImpl_getLoad (const char* libraryName,
                                       LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Payload_load");
    return *(Payload_load*)&func;
}

static Payload_unload
mamaPayloadManagerImpl_getUnload (const char* libraryName,
                                         LIB_HANDLE  libraryHandle)
{
    void* func = 
        mamaLibraryManager_loadLibraryFunction (libraryName,
                                                libraryHandle,
                                                "Payload_unload");
    return *(Payload_unload*)&func;
}

static Payload_createImpl
mamaPayloadManagerImpl_getLibraryCreateImpl (mamaLibrary library)
{
    return (Payload_createImpl)
        mamaPayloadManagerImpl_getCreateImpl (library->mName,
                                                     library->mHandle);
}

static Payload_destroyImpl
mamaPayloadManagerImpl_getLibraryDestroyImpl (mamaLibrary library)
{
    return (Payload_destroyImpl)
        mamaPayloadManagerImpl_getDestroyImpl (library->mName,
                                                      library->mHandle);
}

static Payload_load
mamaPayloadManagerImpl_getLibraryLoad (mamaLibrary library)
{
    return (Payload_load)
        mamaPayloadManagerImpl_getLoad (library->mName,
                                               library->mHandle);
}

static Payload_unload
mamaPayloadManagerImpl_getLibraryUnload (mamaLibrary library)
{
    return (Payload_unload)
        mamaPayloadManagerImpl_getUnload (library->mName,
                                                 library->mHandle);
}

static mama_status
mamaPayloadManagerImpl_getBridges (mamaPayloadBridge* plBridges,
                                   mama_size_t*           size,
                                   mamaLibraryPredicateCb predicate)
{
    if (!plBridges || !size)
        return MAMA_STATUS_NULL_ARG;

    mamaLibrary libraries [MAMA_MAX_LIBRARIES];
    mama_size_t librariesSize = MAMA_MAX_LIBRARIES;

    mama_status status =
        mamaLibraryManager_getLibraries (libraries,
                                         &librariesSize,
                                         MAMA_PAYLOAD_LIBRARY,
                                         predicate);

    for (mama_size_t k = 0; k < librariesSize && k < *size; ++k)
        plBridges[k] = (mamaPayloadBridge)((mamaPayloadLibrary)libraries[k]->
                        mClosure)->mBridge;

    if (librariesSize < *size)
        *size = librariesSize;

    return status;
}

mama_status
mamaPayloadManagerImpl_loadBridge (mamaPayloadLibrary plLibrary)
{
    mama_status       status  = MAMA_STATUS_OK;
    mamaLibrary       library = plLibrary->mParent;
    mamaPayloadBridge bridge  = plLibrary->mBridge;
    
    if (!bridge)
    {
        status = 
            mamaPayloadManagerImpl_createBridge (plLibrary->mParent,
                                             &plLibrary->mBridge);
        if (MAMA_STATUS_OK != status)    
            return status;

        bridge->mLibrary          = plLibrary;
    }

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
    return MAMA_STATUS_OK;
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
mamaPayloadManagerImpl_createBridge (mamaLibrary        library,
                                     mamaPayloadBridge* bridge0)
{
    assert (bridge0);
    *bridge0 = NULL;

    mamaPayloadBridge bridge = (mamaPayloadBridge)
        calloc (1, sizeof (mamaPayloadBridgeImpl));

    if (!bridge)
        return MAMA_STATUS_NOMEM;

    mama_status status = MAMA_STATUS_OK;
    
    Payload_createImpl createImpl =
        mamaPayloadManagerImpl_getLibraryCreateImpl (library);

    if (createImpl)
    {        
        /* FIXME: We might need to make a carbon copy of mamaPayloadBridge
        * and make it mamaPayloadOldBridge if we make any changes that
        * would affect binary-compatibility between structures, other than
        * adding things on to the end of the structure. */
        mamaPayloadBridge oldBridge = NULL;
        char payloadId = '\0';
        status = createImpl (&oldBridge, &payloadId);

        if (MAMA_STATUS_OK != status || !oldBridge)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaPayloadManager_createBridge(): "
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
        Payload_destroyImpl destroyImpl =
            mamaPayloadManagerImpl_getLibraryDestroyImpl (library);

        if (destroyImpl)
            destroyImpl (oldBridge);
        else
            free (oldBridge);
    }

    *bridge0 = bridge;

    return MAMA_STATUS_OK;
}

static void
mamaPayloadManager_destroyBridge (mamaPayloadBridge bridge)
{
    free (bridge);
}

mama_status
mamaPayloadManager_activateLibrary (mamaPayloadLibrary plLibrary)
{
    if (0 == wInterlocked_read (&plLibrary->mActiveCount))
    {
        mamaLibrary library = plLibrary->mParent;
        wlock_lock (library->mLock);
        if (0 == wInterlocked_read (&plLibrary->mActiveCount))
        {
            mama_status status = 
                mamaPayloadManagerImpl_loadBridge (plLibrary);

            if (MAMA_STATUS_OK == status)
                wInterlocked_increment (&plLibrary->mActiveCount);
        }        
        wlock_unlock (library->mLock);
    }
    return MAMA_STATUS_OK;
}

static void
mamaPayloadManager_deactivateLibrary (mamaPayloadLibrary plLibrary)
{
    wlock_lock (plLibrary->mParent->mLock);
    if (0 != wInterlocked_read (&plLibrary->mActiveCount))
    {
        mamaPayloadManager_destroyBridge (plLibrary->mBridge); 

        wInterlocked_decrement (&plLibrary->mActiveCount);    
    } 
    wlock_unlock (plLibrary->mParent->mLock);
}

static mama_status
mamaPayloadManagerImpl_createLibrary (
        mamaLibrary library, mamaPayloadLibrary* plLibrary0)
{
    assert (plLibrary0);
    *plLibrary0 = NULL;

    mamaPayloadManager plManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary plLibrary =
        (mamaPayloadLibrary) calloc (1, sizeof (mamaPayloadLibraryImpl));

    if (!plLibrary)
        return MAMA_STATUS_NOMEM;

    Payload_load load = 
        mamaPayloadManagerImpl_getLibraryLoad (library);

    if (load)
    {
        status = load (&plLibrary->mPayloadId);

        if (MAMA_STATUS_OK != status)
        {
             mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaPayloadManagerImpl_createLibrary(): "
                      "Could not initialise %s library %s bridge using "
                      "new-style initialisation.",
                      library->mTypeName, library->mName);

            free (plLibrary);
            return MAMA_STATUS_NO_BRIDGE_IMPL;
        }
        
        status = mamaLibraryManager_compareMamaVersion (library);
        
        if (MAMA_STATUS_OK != status)
        {
            free (plLibrary);
            return status;
        }
    }

    library->mClosure = plLibrary;

    plLibrary->mBridge    = NULL;
    plLibrary->mParent    = library;
    plLibrary->mManager   = plManager;

    wInterlocked_initialize (&plLibrary->mActiveCount);
    wInterlocked_set (0, &plLibrary->mActiveCount);
 
    status = 
            mamaPayloadManagerImpl_createBridge (plLibrary->mParent,
                                                &plLibrary->mBridge);
    if (MAMA_STATUS_OK != status)    
        return status;

    plLibrary->mBridge->mLibrary          = plLibrary;
    *plLibrary0 = plLibrary;
    return MAMA_STATUS_OK;
}

static void
mamaPayloadManagerImpl_destroyLibrary (
        mamaPayloadLibrary plLibrary)
{
    mamaLibrary library = plLibrary->mParent;

    mamaPayloadManager_deactivateLibrary (plLibrary);

    Payload_unload unload = 
        mamaPayloadManagerImpl_getLibraryUnload (library);

    if (unload)
    {
        if (MAMA_STATUS_OK != unload ())
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, 
                      "mamaMiddlewareLibraryManagerImpl_destroyLibrary(): "
                      "Error unloading %s library %s", library->mTypeName, 
                      library->mName);
        }
    }

    library->mClosure = NULL;
    free (plLibrary);
}

static mama_status
mamaPayloadManagerImpl_getPayloadId (mamaPayloadLibrary plLibrary,
                                            char*              payloadId)
{
    assert (plLibrary);
    assert (payloadId);
    *payloadId = 0;

    mamaLibrary library = plLibrary->mParent;

    /*Give the properties a chance to override the programmtically set value*/ 
    const char* prop =
        mamaLibraryManager_getLibraryProperty (library, "id");

    if (prop)
    {
        if (*payloadId)
        {
            mama_log (MAMA_LOG_LEVEL_FINE,
                      "mamaPayloadManagerImpl_getPayloadId(): "
                      "Overriding payload bridge ID [%c] "
                      "with configured ID [%c]",
                      *payloadId, prop[0]);
        }

        *payloadId = prop[0];
        return MAMA_STATUS_OK;
    }

    void* func = 
        mamaLibraryManager_loadLibraryFunction (library->mName, 
                                                library->mHandle, 
                                                "Payload_getType");

    if (!func)
        return MAMA_STATUS_SYSTEM_ERROR;

    Payload_getType typeFunc = (*(Payload_getType*)&func); 

    char tmpId = typeFunc();
    if (*payloadId && tmpId != *payloadId)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaPayloadManagerImpl_getPayloadId(): "
                  "Received payload bridge ID [%c] from load "
                  "and different payload ID [%c] from getType",
                   *payloadId, tmpId);
        return MAMA_STATUS_PLATFORM;
    }

    *payloadId = tmpId;
    return MAMA_STATUS_OK;
}

/*
 * Internal implementation (accessible from library manager)
 */

mama_status
mamaPayloadManager_create (mamaLibraryTypeManager manager)
{
    mamaPayloadManager plManager = (mamaPayloadManager)
        calloc (1, sizeof (mamaPayloadManagerImpl));

    if (!plManager)
        return MAMA_STATUS_NOMEM;

    /* Establish bi-directional link */
    manager->mClosure = plManager;
    plManager->mParent = manager;

    return MAMA_STATUS_OK;
}

void
mamaPayloadManager_destroy (void)
{
    mamaPayloadManager plManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK == status)
    {
        plManager->mParent->mClosure = NULL;
        free (plManager);
    }
}

mama_status
mamaPayloadManager_loadLibrary (mamaLibrary library)
{
    mamaPayloadLibrary plLibrary = NULL;
    mama_status status =
        mamaPayloadManagerImpl_createLibrary (library,
                                                     &plLibrary);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadManager plManager = NULL;
    status = mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    plLibrary->mManager = plManager;

    status =
        mamaPayloadManagerImpl_getPayloadId (plLibrary,
                                                    &plLibrary->mPayloadId);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary dupLibrary = plManager->mPayloads[plLibrary->mPayloadId];

    if (dupLibrary)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaPayloadManager_loadLibrary(): "
                  "Payload id [%d] for %s library %s duplicates library %s.",
                  plLibrary->mPayloadId, library->mTypeName,
                  library->mName,
                  dupLibrary->mParent->mName);

        return MAMA_STATUS_PLATFORM;
    }

    /* Store indexed lookup */
    plManager->mPayloads[plLibrary->mPayloadId] = plLibrary;

    if (!mamaPayloadManager_getDefaultPayload ())
        mamaPayloadManager_setDefaultPayload (plLibrary);

    return status;
}

void
mamaPayloadManager_unloadLibrary (mamaLibrary library)
{
    mamaPayloadLibrary plLibrary = ((mamaPayloadLibrary) library->mClosure);
    mamaPayloadManager plManager = plLibrary->mManager;

    char payloadId = plLibrary->mPayloadId;
    mamaPayloadManagerImpl_destroyLibrary (plLibrary);
    plManager->mPayloads[payloadId] = NULL;
    gDefaultPayload = NULL;
}

mamaLibraryType
mamaPayloadManager_classifyLibraryType (const char* libraryName,
                                               LIB_HANDLE  libraryLib)
{
    Payload_load load = 
        mamaPayloadManagerImpl_getLoad (libraryName,
                                               libraryLib);

    Payload_unload unload = 
        mamaPayloadManagerImpl_getUnload (libraryName,
                                                 libraryLib);

    if (load || unload)
        return MAMA_PAYLOAD_LIBRARY;

    Payload_createImpl createImpl =
        mamaPayloadManagerImpl_getCreateImpl (libraryName,
                                                     libraryLib);

    Payload_destroyImpl destroyImpl =
        mamaPayloadManagerImpl_getDestroyImpl (libraryName,
                                                      libraryLib);

    if (createImpl || destroyImpl)
        return MAMA_PAYLOAD_LIBRARY;

    return MAMA_UNKNOWN_LIBRARY;
}

mama_bool_t
mamaPayloadManager_forwardCallback (mamaLibraryCb cb, 
                                    mamaLibrary   library, 
                                    void*         closure)
{
    mamaPayloadCb plCb             = (mamaPayloadCb)cb;
    mamaPayloadLibrary   plLibrary = 
        (mamaPayloadLibrary)library->mClosure;

    return plCb (plLibrary->mBridge, closure);
}

void mamaPayloadManager_dump (mamaLibraryTypeManager manager)
{
    mamaPayloadManager mwManager = 
        (mamaPayloadManager) manager->mClosure;
}

void mamaPayloadManager_dumpLibrary (mamaLibrary library)
{
    mamaPayloadLibrary plLibrary = 
        (mamaPayloadLibrary) library->mClosure;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Payload Id [%c]", 
                                plLibrary->mPayloadId);
}

/*
 * Public implementation
 */

mama_status
mamaPayloadManager_loadBridgeWithPath (const char*        payloadName,
                                       const char*        path,
                                       mamaPayloadBridge* bridge)
{
    if (!bridge || !payloadName)
        return MAMA_STATUS_NULL_ARG;
    
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_loadLibrary (payloadName,
                                        MAMA_PAYLOAD_LIBRARY,
                                        path, &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *bridge =
        (mamaPayloadBridge)((mamaPayloadLibrary) library->mClosure)->mBridge;

    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadManager_unloadBridge (mamaPayloadBridge bridge)
{
    return mamaLibraryManager_unloadLibrary (
        mamaPayloadManager_getName(bridge), MAMA_PAYLOAD_LIBRARY);
}

mama_status
mamaPayloadManager_getBridge (const char*        payloadName,
                               mamaPayloadBridge* plBridge)
{
    if (!plBridge || !payloadName)
        return MAMA_STATUS_NULL_ARG;
    
    mamaLibrary library = NULL;
    mama_status status =
        mamaLibraryManager_getLibrary (payloadName,
                                       MAMA_PAYLOAD_LIBRARY,
                                       &library);

    if (MAMA_STATUS_OK != status)
        return status;

    *plBridge =
        (mamaPayloadBridge)((mamaPayloadLibrary) library->mClosure)->mBridge;

    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadManager_getDefaultBridge (mamaPayloadBridge* plBridge)
{
    if (!gDefaultPayload)
        return MAMA_STATUS_NOT_FOUND;
    
    mama_status status = 
        mamaPayloadManager_activateLibrary (gDefaultPayload);

    *plBridge = gDefaultPayload->mBridge;
    return status;
}

mama_status
mamaPayloadManager_getBridgeById (char                payloadId,
                                  mamaPayloadBridge* plBridge)
{
    if (!plBridge)
        return MAMA_STATUS_NULL_ARG;    

    *plBridge = NULL;

    mamaPayloadManager plManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary plLibrary =
        plManager->mPayloads[payloadId];

    *plBridge = plLibrary->mBridge; 
    return (*plBridge ? MAMA_STATUS_OK : MAMA_STATUS_NO_BRIDGE_IMPL);
}

mama_status
mamaPayloadManager_getBridges (mamaPayloadBridge* bridges,
                                 mama_size_t*       size)
{
    return mamaPayloadManagerImpl_getBridges (bridges, size,
                                                      NULL);
}

mama_status
mamaPayloadManager_registerLoadCallback (mamaPayloadCb cb,
                                         void*         closure)
{
    mamaPayloadManager plManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerLoadCallback (plManager->mParent,
                                                    (mamaLibraryCb)cb, closure);
}

mama_status
mamaPayloadManager_registerUnloadCallback (mamaPayloadCb cb,
                                           void*           closure)
{
    mamaPayloadManager plManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_registerUnloadCallback (plManager->mParent,
                                                      (mamaLibraryCb)cb, closure);
}

mama_status
mamaPayloadManager_deregisterLoadCallback (mamaPayloadCb cb)
{
    mamaPayloadManager mwManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_deregisterLoadCallback (mwManager->mParent,
                                                      (mamaLibraryCb)cb);
}

mama_status
mamaPayloadManager_deregisterUnloadCallback (mamaPayloadCb cb)
{
    mamaPayloadManager mwManager = NULL;
    mama_status status =
        mamaPayloadManagerImpl_getInstance (&mwManager);

    if (MAMA_STATUS_OK != status)
        return status;

    return mamaLibraryManager_deregisterUnloadCallback (mwManager->mParent,
                                                        (mamaLibraryCb)cb);
}

mama_status
mamaPayloadManager_setProperty (const char* libraryName,
                                const char* propertyName,
                                const char* value)
{
    return mamaLibraryManager_setProperty (libraryName,
                                           propertyName,
                                           value);
}

const char* 
mamaPayloadManager_getName (mamaPayloadBridge bridge)
{
    if (!bridge)
        return NULL;

    return mamaLibraryManager_getName(bridge->mLibrary->mParent);
}

char 
mamaPayloadManager_getId (mamaPayloadBridge bridge)
{
    return bridge->mLibrary->mPayloadId;
}

const char*
mamaPayloadManager_getPath (mamaPayloadBridge bridge)
{
    if (!bridge)
        return NULL;

    return mamaLibraryManager_getPath(bridge->mLibrary->mParent);
}

mamaPayloadBridge
mamaPayloadManager_findPayload (char id)
{
    mamaPayloadBridge bridge = NULL;
    mamaPayloadManager_getBridgeById (id, &bridge);
   
    if (!bridge)
        return NULL;

    mama_status status = 
        mamaPayloadManager_activateLibrary (bridge->mLibrary);

    if (MAMA_STATUS_OK != status)
        return NULL;
 
    return bridge;
}

mamaPayloadBridge
mamaPayloadManager_getDefaultPayload (void)
{
    if (!gDefaultPayload)
        return NULL;

     mama_status status = 
        mamaPayloadManager_activateLibrary (gDefaultPayload);

    if (MAMA_STATUS_OK != status)
        return NULL;
 
    return gDefaultPayload->mBridge;
}

void
mamaPayloadManager_setDefaultPayload (mamaPayloadLibrary library)
{
    gDefaultPayload = library; 
}

mama_status
mamaPayloadManager_setDefaultPayloadbyId (char id)
{
    if (id == '\0')
        return MAMA_STATUS_NULL_ARG;

    mamaPayloadBridge bridge = NULL;

    mama_status status =
        mamaPayloadManager_getBridgeById (id, &bridge);

    gDefaultPayload = bridge->mLibrary;

    if (status != MAMA_STATUS_OK)
        return status;

    return MAMA_STATUS_OK;
}

mama_status
mamaPayloadManager_payloadIdToString (char         payloadId,
                                      const char** str)
{
    if (!str)
        return MAMA_STATUS_NULL_ARG;

    *str = NULL;
    mamaPayloadManager plManager = NULL;

    mama_status status =
        mamaPayloadManagerImpl_getInstance (&plManager);

    if (MAMA_STATUS_OK != status)
        return status;

    mamaPayloadLibrary library = plManager->mPayloads[payloadId];

    if (!library)
        return MAMA_STATUS_NOT_FOUND;

    *str = library->mParent->mName;
    return status;
}

mama_status
mamaPayloadManager_stringToPayloadId (const char* str, 
                                      char*       payloadId)
{
    if (!str || !payloadId)
        return MAMA_STATUS_NULL_ARG;

    *payloadId = 0;
    mamaPayloadBridge bridge = NULL;

    mama_status status =
        mamaPayloadManager_getBridge (str, &bridge);

    if (MAMA_STATUS_OK != status)
        return status;

    *payloadId = bridge->mLibrary->mPayloadId;
    return status;
}

const char*
mamaPayloadManager_convertToString (mamaPayloadType payloadType)
{
    switch (payloadType)
    {
        case MAMA_PAYLOAD_SOLACE:
            return "solacemsg";
        case MAMA_PAYLOAD_V5:
            return "V5";
        case MAMA_PAYLOAD_AVIS:
            return "AVIS";
        case MAMA_PAYLOAD_TICK42BLP:
            return "TICK42BLP";
        case MAMA_PAYLOAD_FAST:
            return "FAST";
        case MAMA_PAYLOAD_RAI:
            return "rai";
        case MAMA_PAYLOAD_UMS:
            return "UMS";
        case MAMA_PAYLOAD_TICK42RMDS:
            return "TICK42RMDS";
        case MAMA_PAYLOAD_QPID:
            return "QPID";
        case MAMA_PAYLOAD_TIBRV:
            return "TIBRV";
        case MAMA_PAYLOAD_IBMWFO:
            return "ibmwfo";
        case MAMA_PAYLOAD_ACTIV:
            return "activ";
        case MAMA_PAYLOAD_VULCAN:
            return "Vulcan";
        case MAMA_PAYLOAD_WOMBAT_MSG:
            return "WombatMsg";
        case MAMA_PAYLOAD_EXEGY:
            return "EXEGY";
        case MAMA_PAYLOAD_INRUSH:
            return "INRUSH";
        case MAMA_PAYLOAD_KWANTUM:
            return "KWANTUM";
        default:
            return "unknown";
    }
}

mama_status
mamaPayloadManager_convertLibraryToPayload (mamaPayloadLibrary library,
                                            mamaPayloadBridge* bridge)
{
    assert (library);
    assert (bridge);

    *bridge = library->mBridge;
    return MAMA_STATUS_OK;
}

