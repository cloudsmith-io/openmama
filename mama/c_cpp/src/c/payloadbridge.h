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

#ifndef PayloadBridgeH__
#define PayloadBridgeH__


#if defined(__cplusplus)
extern "C" {
#endif

#include <mama/mama.h>
#include <mamainternal.h>
#include <mama/subscmsgtype.h>

#define XSTR(s) STR(s)
#define STR(s) #s

typedef void* msgPayload;
typedef void* msgFieldPayload;
typedef void* msgPayloadIter;

/* Set all all the msg payload releated functions */
#define INITIALIZE_PAYLOAD_BRIDGE(msgPayloadImpl, identifier)               \
do                                                                          \
{                                                                           \
    msgPayloadImpl->mClosure            =   NULL;                           \
    msgPayloadImpl->msgPayloadCreate                                        \
                        = identifier ## Payload_create;                     \
    msgPayloadImpl->msgPayloadGetType                                       \
                        = identifier ## Payload_getType;                    \
    msgPayloadImpl->msgPayloadCreateForTemplate                             \
                        = identifier ## Payload_createForTemplate;          \
    msgPayloadImpl->msgPayloadCopy                                          \
                        = identifier ## Payload_copy;                       \
    msgPayloadImpl->msgPayloadClear                                         \
                        = identifier ## Payload_clear;                      \
    msgPayloadImpl->msgPayloadDestroy                                       \
                        = identifier ## Payload_destroy;                    \
    msgPayloadImpl->msgPayloadSetParent                                     \
                        = identifier ## Payload_setParent;                  \
    msgPayloadImpl->msgPayloadGetByteSize                                   \
                        = identifier ## Payload_getByteSize;                \
    msgPayloadImpl->msgPayloadGetNumFields                                  \
                        = identifier ## Payload_getNumFields;               \
    msgPayloadImpl->msgPayloadGetSendSubject                                \
                        = identifier ## Payload_getSendSubject;             \
    msgPayloadImpl->msgPayloadToString                                      \
                        = identifier ## Payload_toString;                   \
    msgPayloadImpl->msgPayloadIterateFields                                 \
                        = identifier ## Payload_iterateFields;              \
    msgPayloadImpl->msgPayloadGetByteBuffer                                 \
                       = identifier ## Payload_getByteBuffer;               \
   msgPayloadImpl->msgPayloadUnSerialize                                    \
                       = identifier ## Payload_unSerialize;                 \
    msgPayloadImpl->msgPayloadSerialize                                     \
                        = identifier ## Payload_serialize;                  \
    msgPayloadImpl->msgPayloadSetByteBuffer                                 \
                        = identifier ## Payload_setByteBuffer;              \
    msgPayloadImpl->msgPayloadCreateFromByteBuffer                          \
                        = identifier ## Payload_createFromByteBuffer;       \
    msgPayloadImpl->msgPayloadApply                                         \
                        = identifier ## Payload_apply;                      \
    msgPayloadImpl->msgPayloadGetNativeMsg                                  \
                        = identifier ## Payload_getNativeMsg;               \
    msgPayloadImpl->msgPayloadGetFieldAsString                              \
                        = identifier ## Payload_getFieldAsString;           \
    /*Add methods */                                                        \
    msgPayloadImpl->msgPayloadAddBool                                       \
                        = identifier ## Payload_addBool;                    \
    msgPayloadImpl->msgPayloadAddChar                                       \
                        = identifier ## Payload_addChar;                    \
    msgPayloadImpl->msgPayloadAddI8                                         \
                        = identifier ## Payload_addI8;                      \
    msgPayloadImpl->msgPayloadAddU8                                         \
                        = identifier ## Payload_addU8;                      \
    msgPayloadImpl->msgPayloadAddI16                                        \
                        = identifier ## Payload_addI16;                     \
    msgPayloadImpl->msgPayloadAddU16                                        \
                        = identifier ## Payload_addU16;                     \
    msgPayloadImpl->msgPayloadAddI32                                        \
                        = identifier ## Payload_addI32;                     \
    msgPayloadImpl->msgPayloadAddU32                                        \
                        = identifier ## Payload_addU32;                     \
    msgPayloadImpl->msgPayloadAddI64                                        \
                        = identifier ## Payload_addI64;                     \
    msgPayloadImpl->msgPayloadAddU64                                        \
                        = identifier ## Payload_addU64;                     \
    msgPayloadImpl->msgPayloadAddF32                                        \
                        = identifier ## Payload_addF32;                     \
    msgPayloadImpl->msgPayloadAddF64                                        \
                        = identifier ## Payload_addF64;                     \
    msgPayloadImpl->msgPayloadAddString                                     \
                        = identifier ## Payload_addString;                  \
    msgPayloadImpl->msgPayloadAddOpaque                                     \
                        = identifier ## Payload_addOpaque;                  \
    msgPayloadImpl->msgPayloadAddDateTime                                   \
                        = identifier ## Payload_addDateTime;                \
    msgPayloadImpl->msgPayloadAddPrice                                      \
                        = identifier ## Payload_addPrice;                   \
    msgPayloadImpl->msgPayloadAddMsg                                        \
                        = identifier ## Payload_addMsg;                     \
    msgPayloadImpl->msgPayloadAddVectorBool                                 \
                        = identifier ## Payload_addVectorBool;              \
    msgPayloadImpl->msgPayloadAddVectorChar                                 \
                        = identifier ## Payload_addVectorChar;              \
    msgPayloadImpl->msgPayloadAddVectorI8                                   \
                        = identifier ## Payload_addVectorI8;                \
    msgPayloadImpl->msgPayloadAddVectorU8                                   \
                        = identifier ## Payload_addVectorU8;                \
    msgPayloadImpl->msgPayloadAddVectorI16                                  \
                        = identifier ## Payload_addVectorI16;               \
    msgPayloadImpl->msgPayloadAddVectorU16                                  \
                        = identifier ## Payload_addVectorU16;               \
    msgPayloadImpl->msgPayloadAddVectorI32                                  \
                        = identifier ## Payload_addVectorI32;               \
    msgPayloadImpl->msgPayloadAddVectorU32                                  \
                        = identifier ## Payload_addVectorU32;               \
    msgPayloadImpl->msgPayloadAddVectorI64                                  \
                        = identifier ## Payload_addVectorI64;               \
    msgPayloadImpl->msgPayloadAddVectorU64                                  \
                        = identifier ## Payload_addVectorU64;               \
    msgPayloadImpl->msgPayloadAddVectorF32                                  \
                        = identifier ## Payload_addVectorF32;               \
    msgPayloadImpl->msgPayloadAddVectorF64                                  \
                        = identifier ## Payload_addVectorF64;               \
    msgPayloadImpl->msgPayloadAddVectorString                               \
                        = identifier ## Payload_addVectorString;            \
    msgPayloadImpl->msgPayloadAddVectorMsg                                  \
                        = identifier ## Payload_addVectorMsg;               \
    msgPayloadImpl->msgPayloadAddVectorDateTime                             \
                        = identifier ## Payload_addVectorDateTime;          \
    msgPayloadImpl->msgPayloadAddVectorPrice                                \
                        = identifier ## Payload_addVectorPrice;             \
    /*Update methods */                                                     \
    msgPayloadImpl->msgPayloadUpdateBool                                    \
                        = identifier ## Payload_updateBool;                 \
    msgPayloadImpl->msgPayloadUpdateChar                                    \
                        = identifier ## Payload_updateChar;                 \
    msgPayloadImpl->msgPayloadUpdateU8                                      \
                        = identifier ## Payload_updateU8;                   \
    msgPayloadImpl->msgPayloadUpdateI8                                      \
                        = identifier ## Payload_updateI8;                   \
    msgPayloadImpl->msgPayloadUpdateI16                                     \
                        = identifier ## Payload_updateI16;                  \
    msgPayloadImpl->msgPayloadUpdateU16                                     \
                        = identifier ## Payload_updateU16;                  \
    msgPayloadImpl->msgPayloadUpdateI32                                     \
                        = identifier ## Payload_updateI32;                  \
    msgPayloadImpl->msgPayloadUpdateU32                                     \
                        = identifier ## Payload_updateU32;                  \
    msgPayloadImpl->msgPayloadUpdateI64                                     \
                        = identifier ## Payload_updateI64;                  \
    msgPayloadImpl->msgPayloadUpdateU64                                     \
                        = identifier ## Payload_updateU64;                  \
    msgPayloadImpl->msgPayloadUpdateF32                                     \
                        = identifier ## Payload_updateF32;                  \
    msgPayloadImpl->msgPayloadUpdateF64                                     \
                        = identifier ## Payload_updateF64;                  \
    msgPayloadImpl->msgPayloadUpdateString                                  \
                        = identifier ## Payload_updateString;               \
    msgPayloadImpl->msgPayloadUpdateOpaque                                  \
                        = identifier ## Payload_updateOpaque;               \
    msgPayloadImpl->msgPayloadUpdateDateTime                                \
                        = identifier ## Payload_updateDateTime;             \
    msgPayloadImpl->msgPayloadUpdatePrice                                   \
                        = identifier ## Payload_updatePrice;                \
    msgPayloadImpl->msgPayloadUpdateSubMsg                                  \
                        = identifier ## Payload_updateSubMsg;               \
    msgPayloadImpl->msgPayloadUpdateVectorMsg                               \
                        = identifier ## Payload_updateVectorMsg;            \
    msgPayloadImpl->msgPayloadUpdateVectorString                            \
                        = identifier ## Payload_updateVectorString;         \
    msgPayloadImpl->msgPayloadUpdateVectorBool                              \
                        = identifier ## Payload_updateVectorBool;           \
    msgPayloadImpl->msgPayloadUpdateVectorChar                              \
                        = identifier ## Payload_updateVectorChar;           \
    msgPayloadImpl->msgPayloadUpdateVectorI8                                \
                        = identifier ## Payload_updateVectorI8;             \
    msgPayloadImpl->msgPayloadUpdateVectorU8                                \
                        = identifier ## Payload_updateVectorU8;             \
    msgPayloadImpl->msgPayloadUpdateVectorI16                               \
                        = identifier ## Payload_updateVectorI16;            \
    msgPayloadImpl->msgPayloadUpdateVectorU16                               \
                        = identifier ## Payload_updateVectorU16;            \
    msgPayloadImpl->msgPayloadUpdateVectorI32                               \
                        = identifier ## Payload_updateVectorI32;            \
    msgPayloadImpl->msgPayloadUpdateVectorU32                               \
                        = identifier ## Payload_updateVectorU32;            \
    msgPayloadImpl->msgPayloadUpdateVectorI64                               \
                        = identifier ## Payload_updateVectorI64;            \
    msgPayloadImpl->msgPayloadUpdateVectorU64                               \
                        = identifier ## Payload_updateVectorU64;            \
    msgPayloadImpl->msgPayloadUpdateVectorF32                               \
                        = identifier ## Payload_updateVectorF32;            \
    msgPayloadImpl->msgPayloadUpdateVectorF64                               \
                        = identifier ## Payload_updateVectorF64;            \
    msgPayloadImpl->msgPayloadUpdateVectorPrice                             \
                        = identifier ## Payload_updateVectorPrice;          \
    msgPayloadImpl->msgPayloadUpdateVectorTime                              \
                        = identifier ## Payload_updateVectorTime;           \
    /*Get methods */                                                        \
    msgPayloadImpl->msgPayloadGetBool                                       \
                        = identifier ## Payload_getBool;                    \
    msgPayloadImpl->msgPayloadGetChar                                       \
                        = identifier ## Payload_getChar;                    \
    msgPayloadImpl->msgPayloadGetI8                                         \
                        = identifier ## Payload_getI8;                      \
    msgPayloadImpl->msgPayloadGetU8                                         \
                        = identifier ## Payload_getU8;                      \
    msgPayloadImpl->msgPayloadGetI16                                        \
                        = identifier ## Payload_getI16;                     \
    msgPayloadImpl->msgPayloadGetU16                                        \
                        = identifier ## Payload_getU16;                     \
    msgPayloadImpl->msgPayloadGetI32                                        \
                        = identifier ## Payload_getI32;                     \
    msgPayloadImpl->msgPayloadGetU32                                        \
                        = identifier ## Payload_getU32;                     \
    msgPayloadImpl->msgPayloadGetI64                                        \
                        = identifier ## Payload_getI64;                     \
    msgPayloadImpl->msgPayloadGetU64                                        \
                        = identifier ## Payload_getU64;                     \
    msgPayloadImpl->msgPayloadGetF32                                        \
                        = identifier ## Payload_getF32;                     \
    msgPayloadImpl->msgPayloadGetF64                                        \
                        = identifier ## Payload_getF64;                     \
    msgPayloadImpl->msgPayloadGetString                                     \
                        = identifier ## Payload_getString;                  \
    msgPayloadImpl->msgPayloadGetOpaque                                     \
                        = identifier ## Payload_getOpaque;                  \
    msgPayloadImpl->msgPayloadGetField                                      \
                        = identifier ## Payload_getField;                   \
    msgPayloadImpl->msgPayloadGetDateTime                                   \
                        = identifier ## Payload_getDateTime;                \
    msgPayloadImpl->msgPayloadGetPrice                                      \
                        = identifier ## Payload_getPrice;                   \
    msgPayloadImpl->msgPayloadGetMsg                                        \
                        = identifier ## Payload_getMsg;                     \
    msgPayloadImpl->msgPayloadGetVectorBool                                 \
                        = identifier ## Payload_getVectorBool;              \
    msgPayloadImpl->msgPayloadGetVectorChar                                 \
                        = identifier ## Payload_getVectorChar;              \
    msgPayloadImpl->msgPayloadGetVectorI8                                   \
                        = identifier ## Payload_getVectorI8;                \
    msgPayloadImpl->msgPayloadGetVectorU8                                   \
                        = identifier ## Payload_getVectorU8;                \
    msgPayloadImpl->msgPayloadGetVectorI16                                  \
                        = identifier ## Payload_getVectorI16;               \
    msgPayloadImpl->msgPayloadGetVectorU16                                  \
                        = identifier ## Payload_getVectorU16;               \
    msgPayloadImpl->msgPayloadGetVectorI32                                  \
                        = identifier ## Payload_getVectorI32;               \
    msgPayloadImpl->msgPayloadGetVectorU32                                  \
                        = identifier ## Payload_getVectorU32;               \
    msgPayloadImpl->msgPayloadGetVectorI64                                  \
                        = identifier ## Payload_getVectorI64;               \
    msgPayloadImpl->msgPayloadGetVectorU64                                  \
                        = identifier ## Payload_getVectorU64;               \
    msgPayloadImpl->msgPayloadGetVectorF32                                  \
                        = identifier ## Payload_getVectorF32;               \
    msgPayloadImpl->msgPayloadGetVectorF64                                  \
                        = identifier ## Payload_getVectorF64;               \
    msgPayloadImpl->msgPayloadGetVectorString                               \
                        = identifier ## Payload_getVectorString;            \
    msgPayloadImpl->msgPayloadGetVectorDateTime                             \
                        = identifier ## Payload_getVectorDateTime;          \
    msgPayloadImpl->msgPayloadGetVectorPrice                                \
                        = identifier ## Payload_getVectorPrice;             \
    msgPayloadImpl->msgPayloadGetVectorMsg                                  \
                        = identifier ## Payload_getVectorMsg;               \
    /*msgIter methods */                                                    \
    msgPayloadImpl->msgPayloadIterCreate                                    \
                        = identifier ## PayloadIter_create;                 \
    msgPayloadImpl->msgPayloadIterNext                                      \
                        = identifier ## PayloadIter_next;                   \
    msgPayloadImpl->msgPayloadIterHasNext                                   \
                        = identifier ## PayloadIter_hasNext;                \
    msgPayloadImpl->msgPayloadIterBegin                                     \
                        = identifier ## PayloadIter_begin;                  \
    msgPayloadImpl->msgPayloadIterEnd                                       \
                        = identifier ## PayloadIter_end;                    \
    msgPayloadImpl->msgPayloadIterAssociate                                 \
                        = identifier ## PayloadIter_associate;              \
    msgPayloadImpl->msgPayloadIterDestroy                                   \
                        = identifier ## PayloadIter_destroy;                \
   /*msgField methods */                                                    \
    msgPayloadImpl->msgFieldPayloadCreate                                   \
                        = identifier ## FieldPayload_create;                \
    msgPayloadImpl->msgFieldPayloadDestroy                                  \
                        = identifier ## FieldPayload_destroy;               \
    msgPayloadImpl->msgFieldPayloadGetName                                  \
                        = identifier ## FieldPayload_getName;               \
    msgPayloadImpl->msgFieldPayloadGetFid                                   \
                        = identifier ## FieldPayload_getFid;                \
    msgPayloadImpl->msgFieldPayloadGetDescriptor                            \
                        = identifier ## FieldPayload_getDescriptor;         \
    msgPayloadImpl->msgFieldPayloadGetType                                  \
                        = identifier ## FieldPayload_getType;               \
    msgPayloadImpl->msgFieldPayloadUpdateBool                               \
                        = identifier ## FieldPayload_updateBool;            \
    msgPayloadImpl->msgFieldPayloadUpdateChar                               \
                        = identifier ## FieldPayload_updateChar;            \
    msgPayloadImpl->msgFieldPayloadUpdateU8                                 \
                        = identifier ## FieldPayload_updateU8;              \
    msgPayloadImpl->msgFieldPayloadUpdateI8                                 \
                        = identifier ## FieldPayload_updateI8;              \
    msgPayloadImpl->msgFieldPayloadUpdateI16                                \
                        = identifier ## FieldPayload_updateI16;             \
    msgPayloadImpl->msgFieldPayloadUpdateU16                                \
                        = identifier ## FieldPayload_updateU16;             \
    msgPayloadImpl->msgFieldPayloadUpdateI32                                \
                        = identifier ## FieldPayload_updateI32;             \
    msgPayloadImpl->msgFieldPayloadUpdateU32                                \
                        = identifier ## FieldPayload_updateU32;             \
    msgPayloadImpl->msgFieldPayloadUpdateI64                                \
                        = identifier ## FieldPayload_updateI64;             \
    msgPayloadImpl->msgFieldPayloadUpdateU64                                \
                        = identifier ## FieldPayload_updateU64;             \
    msgPayloadImpl->msgFieldPayloadUpdateF32                                \
                        = identifier ## FieldPayload_updateF32;             \
    msgPayloadImpl->msgFieldPayloadUpdateF64                                \
                        = identifier ## FieldPayload_updateF64;             \
    msgPayloadImpl->msgFieldPayloadUpdateDateTime                           \
                        = identifier ## FieldPayload_updateDateTime;        \
    msgPayloadImpl->msgFieldPayloadUpdatePrice                              \
                        = identifier ## FieldPayload_updatePrice;           \
    msgPayloadImpl->msgFieldPayloadUpdateString                             \
                        = identifier ## FieldPayload_updateString;          \
    msgPayloadImpl->msgFieldPayloadGetBool                                  \
                        = identifier ## FieldPayload_getBool;               \
    msgPayloadImpl->msgFieldPayloadGetChar                                  \
                        = identifier ## FieldPayload_getChar;               \
    msgPayloadImpl->msgFieldPayloadGetI8                                    \
                        = identifier ## FieldPayload_getI8;                 \
    msgPayloadImpl->msgFieldPayloadGetU8                                    \
                        = identifier ## FieldPayload_getU8;                 \
    msgPayloadImpl->msgFieldPayloadGetI16                                   \
                        = identifier ## FieldPayload_getI16;                \
    msgPayloadImpl->msgFieldPayloadGetU16                                   \
                        = identifier ## FieldPayload_getU16;                \
    msgPayloadImpl->msgFieldPayloadGetI32                                   \
                        = identifier ## FieldPayload_getI32;                \
    msgPayloadImpl->msgFieldPayloadGetU32                                   \
                        = identifier ## FieldPayload_getU32;                \
    msgPayloadImpl->msgFieldPayloadGetI64                                   \
                        = identifier ## FieldPayload_getI64;                \
    msgPayloadImpl->msgFieldPayloadGetU64                                   \
                        = identifier ## FieldPayload_getU64;                \
    msgPayloadImpl->msgFieldPayloadGetF32                                   \
                        = identifier ## FieldPayload_getF32;                \
    msgPayloadImpl->msgFieldPayloadGetF64                                   \
                        = identifier ## FieldPayload_getF64;                \
    msgPayloadImpl->msgFieldPayloadGetString                                \
                        = identifier ## FieldPayload_getString;             \
    msgPayloadImpl->msgFieldPayloadGetOpaque                                \
                        = identifier ## FieldPayload_getOpaque;             \
    msgPayloadImpl->msgFieldPayloadGetDateTime                              \
                        = identifier ## FieldPayload_getDateTime;           \
    msgPayloadImpl->msgFieldPayloadGetPrice                                 \
                        = identifier ## FieldPayload_getPrice;              \
    msgPayloadImpl->msgFieldPayloadGetMsg                                   \
                        = identifier ## FieldPayload_getMsg;                \
    msgPayloadImpl->msgFieldPayloadGetVectorBool                            \
                        = identifier ## FieldPayload_getVectorBool;         \
    msgPayloadImpl->msgFieldPayloadGetVectorChar                            \
                        = identifier ## FieldPayload_getVectorChar;         \
    msgPayloadImpl->msgFieldPayloadGetVectorI8                              \
                        = identifier ## FieldPayload_getVectorI8;           \
    msgPayloadImpl->msgFieldPayloadGetVectorU8                              \
                        = identifier ## FieldPayload_getVectorU8;           \
    msgPayloadImpl->msgFieldPayloadGetVectorI16                             \
                        = identifier ## FieldPayload_getVectorI16;          \
    msgPayloadImpl->msgFieldPayloadGetVectorU16                             \
                        = identifier ## FieldPayload_getVectorU16;          \
    msgPayloadImpl->msgFieldPayloadGetVectorI32                             \
                        = identifier ## FieldPayload_getVectorI32;          \
    msgPayloadImpl->msgFieldPayloadGetVectorU32                             \
                        = identifier ## FieldPayload_getVectorU32;          \
    msgPayloadImpl->msgFieldPayloadGetVectorI64                             \
                        = identifier ## FieldPayload_getVectorI64;          \
    msgPayloadImpl->msgFieldPayloadGetVectorU64                             \
                        = identifier ## FieldPayload_getVectorU64;          \
    msgPayloadImpl->msgFieldPayloadGetVectorF32                             \
                        = identifier ## FieldPayload_getVectorF32;          \
    msgPayloadImpl->msgFieldPayloadGetVectorF64                             \
                        = identifier ## FieldPayload_getVectorF64;          \
    msgPayloadImpl->msgFieldPayloadGetVectorString                          \
                        = identifier ## FieldPayload_getVectorString;       \
    msgPayloadImpl->msgFieldPayloadGetVectorDateTime                        \
                        = identifier ## FieldPayload_getVectorDateTime;     \
    msgPayloadImpl->msgFieldPayloadGetVectorPrice                           \
                        = identifier ## FieldPayload_getVectorPrice;        \
    msgPayloadImpl->msgFieldPayloadGetVectorMsg                             \
                        = identifier ## FieldPayload_getVectorMsg;          \
    msgPayloadImpl->msgFieldPayloadGetAsString                              \
                        = identifier ## FieldPayload_getAsString;           \
}                                                                           \
while(0)                                                                    \

/*===================================================================
 =             general bridge function pointers                 =
 ====================================================================*/
/*Called when loading/creating a bridge */
typedef mama_status
(*Payload_createImpl)      (mamaPayloadBridge* result, char* identifier);

typedef void
(*Payload_destroyImpl)     (mamaPayloadBridge bridge);

typedef mamaPayloadType
(*Payload_getType)         (void);

typedef mama_status 
(*Payload_load)            (char* identifier);

typedef mama_status 
(*Payload_unload)          (void);

/*===================================================================
 =              msgPayload bridge function pointers                 =
 ====================================================================*/
typedef mama_status
(*Payload_create)           (msgPayload*         msg);
typedef mama_status
(*Payload_createForTemplate)(msgPayload*         msg,
                             mamaPayloadBridge       bridge,
                             mama_u32_t          templateId);
typedef mama_status
(*Payload_copy)             (const msgPayload    msg,
                             msgPayload*         copy);
typedef mama_status
(*Payload_clear)            (msgPayload          msg);
typedef mama_status
(*Payload_destroy)          (msgPayload          msg);
typedef mama_status
(*Payload_setParent)        (msgPayload          msg,
                             const mamaMsg       parent);
typedef mama_status
(*Payload_getByteSize)      (msgPayload          msg,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getNumFields)     (const msgPayload    msg,
                             mama_size_t*        numFields);

typedef mama_status
(*Payload_getSendSubject)   (const msgPayload    msg,
                             const char**        subject);
typedef const char*
(*Payload_toString)         (const msgPayload    msg);
typedef mama_status
(*Payload_iterateFields)    (const msgPayload    msg,
                             const mamaMsg       parent,
                             mamaMsgField        field,
                             mamaMsgIteratorCb   cb,
                             void*               closure);

typedef mama_status
(*Payload_serialize)        (const msgPayload    msg,
                             const void**        buffer,
                             mama_size_t*        bufferLength);

typedef mama_status
(*Payload_unSerialize)      (const msgPayload    msg,
                             const void**        buffer,
                             mama_size_t         bufferLength);

typedef mama_status
(*Payload_getByteBuffer)    (const msgPayload    msg,
                             const void**        buffer,
                             mama_size_t*        bufferLength);

typedef mama_status
(*Payload_setByteBuffer)    (const msgPayload    msg,
                             mamaPayloadBridge   bridge,
                             const void*         buffer,
                             mama_size_t         bufferLength);

typedef mama_status
(*Payload_createFromByteBuffer) (
                                msgPayload*         msg,
                                mamaPayloadBridge       bridge,
                                const void*         buffer,
                                mama_size_t         bufferLength);
typedef mama_status
(*Payload_apply)            (msgPayload          dest,
                             const msgPayload    src);
typedef mama_status
(*Payload_getNativeMsg)     (const msgPayload    msg,
                             void**              nativeMsg);

typedef mama_status
(*Payload_getFieldAsString) (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             char*               buffer,
                             mama_size_t         len);
typedef mama_status
(*Payload_addBool)          (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_bool_t         value);
typedef mama_status
(*Payload_addChar)          (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             char                value);
typedef mama_status
(*Payload_addI8)            (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i8_t           value);
typedef mama_status
(*Payload_addU8)            (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u8_t           value);
typedef mama_status
(*Payload_addI16)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i16_t          value);
typedef mama_status
(*Payload_addU16)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u16_t          value);
typedef mama_status
(*Payload_addI32)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i32_t          value);
typedef mama_status
(*Payload_addU32)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u32_t          value);
typedef mama_status
(*Payload_addI64)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i64_t          value);
typedef mama_status
(*Payload_addU64)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u64_t          value);
typedef mama_status
(*Payload_addF32)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f32_t          value);
typedef mama_status
(*Payload_addF64)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f64_t          value);
typedef mama_status
(*Payload_addString)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char*         value);
typedef mama_status
(*Payload_addOpaque)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const void*         value,
                             mama_size_t         size);
typedef mama_status
(*Payload_addDateTime)      (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaDateTime  value);
typedef mama_status
(*Payload_addPrice)         (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaPrice     value);
typedef mama_status
(*Payload_addMsg)           (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             msgPayload          value);
typedef mama_status
(*Payload_addVectorBool)    (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_bool_t   value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorChar)    (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char          value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorI8)      (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i8_t     value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorU8)      (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u8_t     value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorI16)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i16_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorU16)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u16_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorI32)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i32_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorU32)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u32_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorI64)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i64_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorU64)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u64_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorF32)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_f32_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorF64)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_f64_t    value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorString)  (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char*         value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorMsg)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaMsg       value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorDateTime)(msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaDateTime  value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_addVectorPrice)   (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaPrice     value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateBool)       (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_bool_t         value);
typedef mama_status
(*Payload_updateChar)       (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             char                value);
typedef mama_status
(*Payload_updateU8)         (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u8_t           value);
typedef mama_status
(*Payload_updateI8)         (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i8_t           value);
typedef mama_status
(*Payload_updateI16)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i16_t          value);
typedef mama_status
(*Payload_updateU16)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u16_t          value);
typedef mama_status
(*Payload_updateI32)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i32_t          value);
typedef mama_status
(*Payload_updateU32)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u32_t          value);
typedef mama_status
(*Payload_updateI64)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i64_t          value);
typedef mama_status
(*Payload_updateU64)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u64_t          value);
typedef mama_status
(*Payload_updateF32)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f32_t          value);
typedef mama_status
(*Payload_updateF64)        (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f64_t          value);
typedef mama_status
(*Payload_updateString)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char*         value);
typedef mama_status
(*Payload_updateOpaque)     (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const void*         value,
                             mama_size_t         size);
typedef mama_status
(*Payload_updateDateTime)   (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaDateTime  value);
typedef mama_status
(*Payload_updatePrice)      (msgPayload          msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaPrice     value);
typedef mama_status
(*Payload_getBool)          (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_bool_t*        result);
typedef mama_status
(*Payload_updateSubMsg)     (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const msgPayload    subMsg);
typedef mama_status
(*Payload_updateVectorMsg)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mamaMsg       value[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorString)(msgPayload         msg,
                              const char*         fname,
                              mama_fid_t          fid,
                              const char*         strList[],
                              mama_size_t         size);
typedef mama_status
(*Payload_updateVectorBool) (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_bool_t   boolList[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorChar) (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const char          charList[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorI8)   (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_i8_t     i8List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorU8)   (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_u8_t     u8List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorI16)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_i16_t    i16List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorU16)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_u16_t    u16List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorI32)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_i32_t    i32List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorU32)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_u32_t    u32List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorI64)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_i64_t    i64List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorU64)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_u64_t    u64List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorF32)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_f32_t    f32List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorF64)  (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mama_f64_t    f64List[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorPrice)(msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mamaPrice*    priceList[],
                             mama_size_t         size);
typedef mama_status
(*Payload_updateVectorTime) (msgPayload          msg,
                             const char*         fname,
                             mama_fid_t          fid,
                             const mamaDateTime  timeList[],
                             mama_size_t         size);
typedef mama_status
(*Payload_getChar)          (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             char*               result);
typedef mama_status
(*Payload_getI8)            (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i8_t*          result);
typedef mama_status
(*Payload_getU8)            (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u8_t*          result);
typedef mama_status
(*Payload_getI16)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i16_t*         result);
typedef mama_status
(*Payload_getU16)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u16_t*         result);
typedef mama_status
(*Payload_getI32)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i32_t*         result);
typedef mama_status
(*Payload_getU32)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u32_t*         result);
typedef mama_status
(*Payload_getI64)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_i64_t*         result);
typedef mama_status
(*Payload_getU64)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_u64_t*         result);
typedef mama_status
(*Payload_getF32)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f32_t*         result);
typedef mama_status
(*Payload_getF64)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mama_f64_t*         result);
typedef mama_status
(*Payload_getString)        (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char**        result);
typedef mama_status
(*Payload_getOpaque)        (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const void**        result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getField)         (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             msgFieldPayload*    result);
typedef mama_status
(*Payload_getDateTime)      (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mamaDateTime        result);
typedef mama_status
(*Payload_getPrice)         (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             mamaPrice           result);
typedef mama_status
(*Payload_getMsg)           (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             msgPayload*         result);
typedef mama_status
(*Payload_getVectorBool)    (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_bool_t** result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorChar)    (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char**        result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorI8)      (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i8_t**   result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorU8)      (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u8_t**   result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorI16)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i16_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorU16)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u16_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorI32)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i32_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorU32)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u32_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorI64)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_i64_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorU64)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_u64_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorF32)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_f32_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorF64)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mama_f64_t**  result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorString)  (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const char***       result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorDateTime)(const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaDateTime* result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorPrice)   (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const mamaPrice*    result,
                             mama_size_t*        size);
typedef mama_status
(*Payload_getVectorMsg)     (const msgPayload    msg,
                             const char*         name,
                             mama_fid_t          fid,
                             const msgPayload**  result,
                             mama_size_t*        size);
/*===================================================================
 =              msgFieldPayload bridge function pointers             =
 ====================================================================*/
typedef mama_status
(*FieldPayload_create)      (msgFieldPayload*        field);

typedef mama_status
(*FieldPayload_destroy)     (msgFieldPayload         field);

typedef mama_status
(*FieldPayload_getType)     (const msgFieldPayload   field,
                             mamaFieldType*          result);
typedef mama_status
(*FieldPayload_getName)     (msgFieldPayload         field,
                             mamaDictionary          dict,
                             mamaFieldDescriptor     desc,
                             const char**            result);
typedef mama_status
(*FieldPayload_getFid)      (const msgFieldPayload   field,
                             mamaDictionary          dict,
                             mamaFieldDescriptor     desc,
                             uint16_t*               result);
typedef mama_status
(*FieldPayload_getDescriptor)(const msgFieldPayload  field,
                              mamaDictionary          dict,
                              mamaFieldDescriptor*    result);
typedef mama_status
(*FieldPayload_updateBool)  (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_bool_t             value);
typedef mama_status
(*FieldPayload_updateChar)  (msgFieldPayload         field,
                             msgPayload              msg,
                             char                    value);
typedef mama_status
(*FieldPayload_updateU8)    (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_u8_t               value);
typedef mama_status
(*FieldPayload_updateI8)    (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_i8_t               value);
typedef mama_status
(*FieldPayload_updateI16)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_i16_t              value);
typedef mama_status
(*FieldPayload_updateU16)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_u16_t              value);
typedef mama_status
(*FieldPayload_updateI32)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_i32_t              value);
typedef mama_status
(*FieldPayload_updateU32)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_u32_t              value);
typedef mama_status
(*FieldPayload_updateI64)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_i64_t              value);
typedef mama_status
(*FieldPayload_updateU64)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_u64_t              value);
typedef mama_status
(*FieldPayload_updateF32)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_f32_t              value);
typedef mama_status
(*FieldPayload_updateF64)   (msgFieldPayload         field,
                             msgPayload              msg,
                             mama_f64_t              value);
typedef mama_status
(*FieldPayload_updateString)(msgFieldPayload         field,
                             msgPayload              msg,
                             const char*             value);
typedef mama_status
(*FieldPayload_updateDateTime)
                               (msgFieldPayload         field,
                                msgPayload              msg,
                                const mamaDateTime      value);
typedef mama_status
(*FieldPayload_updatePrice) (msgFieldPayload         field,
                             msgPayload              msg,
                             const mamaPrice         value);
typedef mama_status
(*FieldPayload_updateSubMsg)(msgFieldPayload         field,
                             msgPayload              msg,
                             const msgPayload        subMsg);
typedef mama_status
(*FieldPayload_getBool)     (const msgFieldPayload   field,
                             mama_bool_t*            result);
typedef mama_status
(*FieldPayload_getChar)     (const msgFieldPayload   field,
                             char*                   result);
typedef mama_status
(*FieldPayload_getI8)       (const msgFieldPayload   field,
                             mama_i8_t*              result);
typedef mama_status
(*FieldPayload_getU8)       (const msgFieldPayload   field,
                                mama_u8_t*              result);
typedef mama_status
(*FieldPayload_getI16)      (const msgFieldPayload   field,
                             mama_i16_t*             result);
typedef mama_status
(*FieldPayload_getU16)      (const msgFieldPayload   field,
                             mama_u16_t*            result);
typedef mama_status
(*FieldPayload_getI32)      (const msgFieldPayload   field,
                             mama_i32_t*             result);
typedef mama_status
(*FieldPayload_getU32)      (const msgFieldPayload   field,
                             mama_u32_t*             result);
typedef mama_status
(*FieldPayload_getI64)      (const msgFieldPayload   field,
                             mama_i64_t*             result);
typedef mama_status
(*FieldPayload_getU64)      (const msgFieldPayload   field,
                             mama_u64_t*             result);
typedef mama_status
(*FieldPayload_getF32)      (const msgFieldPayload   field,
                             mama_f32_t*             result);
typedef mama_status
(*FieldPayload_getF64)      (const msgFieldPayload   field,
                             mama_f64_t*             result);
typedef mama_status
(*FieldPayload_getString)   (const msgFieldPayload   field,
                             const char**            result);
typedef mama_status
(*FieldPayload_getOpaque)   (const msgFieldPayload   field,
                             const void**            result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getDateTime) (const msgFieldPayload   field,
                             mamaDateTime            result);
typedef mama_status
(*FieldPayload_getPrice)    (const msgFieldPayload   field,
                             mamaPrice               result);
typedef mama_status
(*FieldPayload_getMsg)      (const msgFieldPayload   field,
                             msgPayload*             result);
typedef mama_status
(*FieldPayload_getVectorBool)
                               (const msgFieldPayload   field,
                                const mama_bool_t**     result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorChar)
                               (const msgFieldPayload   field,
                                const char**            result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorI8)
                               (const msgFieldPayload   field,
                                const mama_i8_t**       result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorU8) (const msgFieldPayload   field,
                             const mama_u8_t**       result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorI16)(const msgFieldPayload   field,
                             const mama_i16_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorU16)(const msgFieldPayload   field,
                             const mama_u16_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorI32)(const msgFieldPayload   field,
                             const mama_i32_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorU32)(const msgFieldPayload   field,
                             const mama_u32_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorI64)(const msgFieldPayload   field,
                             const mama_i64_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorU64)(const msgFieldPayload   field,
                             const mama_u64_t**      result,
                             mama_size_t*            size);
 typedef mama_status
(*FieldPayload_getVectorF32)(const msgFieldPayload   field,
                             const mama_f32_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorF64)(const msgFieldPayload   field,
                             const mama_f64_t**      result,
                             mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorString)
                               (const msgFieldPayload   field,
                                const char***           result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorDateTime)
                               (const msgFieldPayload   field,
                                const mamaDateTime*     result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorPrice)
                               (const msgFieldPayload   field,
                                const mamaPrice*        result,
                                mama_size_t*            size);
typedef mama_status
(*FieldPayload_getVectorMsg)(const msgFieldPayload   field,
                             const msgPayload**      result,
                             mama_size_t*            size);

typedef mama_status
(*FieldPayload_getAsString) (const msgFieldPayload   field,
                             const msgPayload        msg,
                             char*                   buffer,
                             mama_size_t             len);

/*===================================================================
 =              msgPayloadIter bridge function pointers             =
 ====================================================================*/
typedef mama_status
(*PayloadIter_create)       (msgPayloadIter*         iter,
                             msgPayload              msg);
typedef msgFieldPayload
(*PayloadIter_next)         (msgPayloadIter          iter,
                             msgFieldPayload         field,
                             msgPayload              msg);
typedef mama_bool_t
(*PayloadIter_hasNext)      (msgPayloadIter          iter,
                             msgPayload              msg);
typedef msgFieldPayload
(*PayloadIter_begin)        (msgPayloadIter          iter,
                             msgFieldPayload         field,
                             msgPayload              msg);
typedef msgFieldPayload
(*PayloadIter_end)          (msgPayloadIter          iter,
                             msgPayload              msg);
typedef mama_status
(*PayloadIter_associate)    (msgPayloadIter          iter,
                             msgPayload              msg);
typedef mama_status
(*PayloadIter_destroy)      (msgPayloadIter          iter);

typedef struct mamaPayloadBridgeImpl_
{
    /*Associate arbitrary data. Used for the wfast state */
    void*     mClosure;

    Payload_create                   msgPayloadCreate;
    Payload_createForTemplate        msgPayloadCreateForTemplate;
    Payload_getType                  msgPayloadGetType;
    Payload_copy                     msgPayloadCopy;
    Payload_clear                    msgPayloadClear;
    Payload_destroy                  msgPayloadDestroy;
    Payload_setParent                msgPayloadSetParent;
    Payload_getByteSize              msgPayloadGetByteSize;
    Payload_getNumFields             msgPayloadGetNumFields;
    Payload_getSendSubject           msgPayloadGetSendSubject;
    Payload_toString                 msgPayloadToString;
    Payload_iterateFields            msgPayloadIterateFields;
    Payload_serialize                msgPayloadSerialize;
    Payload_unSerialize              msgPayloadUnSerialize;
    Payload_getByteBuffer            msgPayloadGetByteBuffer;
    Payload_setByteBuffer            msgPayloadSetByteBuffer;
    Payload_createFromByteBuffer     msgPayloadCreateFromByteBuffer;
    Payload_apply                    msgPayloadApply;
    Payload_getNativeMsg             msgPayloadGetNativeMsg;
    Payload_getFieldAsString         msgPayloadGetFieldAsString;
    Payload_addBool                  msgPayloadAddBool;
    Payload_addChar                  msgPayloadAddChar;
    Payload_addI8                    msgPayloadAddI8;
    Payload_addU8                    msgPayloadAddU8;
    Payload_addI16                   msgPayloadAddI16;
    Payload_addU16                   msgPayloadAddU16;
    Payload_addI32                   msgPayloadAddI32;
    Payload_addU32                   msgPayloadAddU32;
    Payload_addI64                   msgPayloadAddI64;
    Payload_addU64                   msgPayloadAddU64;
    Payload_addF32                   msgPayloadAddF32;
    Payload_addF64                   msgPayloadAddF64;
    Payload_addString                msgPayloadAddString;
    Payload_addOpaque                msgPayloadAddOpaque;
    Payload_addDateTime              msgPayloadAddDateTime;
    Payload_addPrice                 msgPayloadAddPrice;
    Payload_addMsg                   msgPayloadAddMsg;
    Payload_addVectorBool            msgPayloadAddVectorBool;
    Payload_addVectorChar            msgPayloadAddVectorChar;
    Payload_addVectorI8              msgPayloadAddVectorI8;
    Payload_addVectorU8              msgPayloadAddVectorU8;
    Payload_addVectorI16             msgPayloadAddVectorI16;
    Payload_addVectorU16             msgPayloadAddVectorU16;
    Payload_addVectorI32             msgPayloadAddVectorI32;
    Payload_addVectorU32             msgPayloadAddVectorU32;
    Payload_addVectorI64             msgPayloadAddVectorI64;
    Payload_addVectorU64             msgPayloadAddVectorU64;
    Payload_addVectorF32             msgPayloadAddVectorF32;
    Payload_addVectorF64             msgPayloadAddVectorF64;
    Payload_addVectorString          msgPayloadAddVectorString;
    Payload_addVectorMsg             msgPayloadAddVectorMsg;
    Payload_addVectorDateTime        msgPayloadAddVectorDateTime;
    Payload_addVectorPrice           msgPayloadAddVectorPrice;
    Payload_updateBool               msgPayloadUpdateBool;
    Payload_updateChar               msgPayloadUpdateChar;
    Payload_updateU8                 msgPayloadUpdateU8;
    Payload_updateI8                 msgPayloadUpdateI8;
    Payload_updateI16                msgPayloadUpdateI16;
    Payload_updateU16                msgPayloadUpdateU16;
    Payload_updateI32                msgPayloadUpdateI32;
    Payload_updateU32                msgPayloadUpdateU32;
    Payload_updateI64                msgPayloadUpdateI64;
    Payload_updateU64                msgPayloadUpdateU64;
    Payload_updateF32                msgPayloadUpdateF32;
    Payload_updateF64                msgPayloadUpdateF64;
    Payload_updateString             msgPayloadUpdateString;
    Payload_updateOpaque             msgPayloadUpdateOpaque;
    Payload_updateDateTime           msgPayloadUpdateDateTime;
    Payload_updatePrice              msgPayloadUpdatePrice;
    Payload_updateSubMsg             msgPayloadUpdateSubMsg;
    Payload_updateVectorMsg          msgPayloadUpdateVectorMsg;
    Payload_updateVectorString       msgPayloadUpdateVectorString;
    Payload_updateVectorBool         msgPayloadUpdateVectorBool;
    Payload_updateVectorChar         msgPayloadUpdateVectorChar;
    Payload_updateVectorI8           msgPayloadUpdateVectorI8;
    Payload_updateVectorU8           msgPayloadUpdateVectorU8;
    Payload_updateVectorI16          msgPayloadUpdateVectorI16;
    Payload_updateVectorU16          msgPayloadUpdateVectorU16;
    Payload_updateVectorI32          msgPayloadUpdateVectorI32;
    Payload_updateVectorU32          msgPayloadUpdateVectorU32;
    Payload_updateVectorI64          msgPayloadUpdateVectorI64;
    Payload_updateVectorU64          msgPayloadUpdateVectorU64;
    Payload_updateVectorF32          msgPayloadUpdateVectorF32;
    Payload_updateVectorF64          msgPayloadUpdateVectorF64;
    Payload_updateVectorPrice        msgPayloadUpdateVectorPrice;
    Payload_updateVectorTime         msgPayloadUpdateVectorTime;
    Payload_getBool                  msgPayloadGetBool;
    Payload_getChar                  msgPayloadGetChar;
    Payload_getI8                    msgPayloadGetI8;
    Payload_getU8                    msgPayloadGetU8;
    Payload_getI16                   msgPayloadGetI16;
    Payload_getU16                   msgPayloadGetU16;
    Payload_getI32                   msgPayloadGetI32;
    Payload_getU32                   msgPayloadGetU32;
    Payload_getI64                   msgPayloadGetI64;
    Payload_getU64                   msgPayloadGetU64;
    Payload_getF32                   msgPayloadGetF32;
    Payload_getF64                   msgPayloadGetF64;
    Payload_getString                msgPayloadGetString;
    Payload_getOpaque                msgPayloadGetOpaque;
    Payload_getField                 msgPayloadGetField;
    Payload_getDateTime              msgPayloadGetDateTime;
    Payload_getPrice                 msgPayloadGetPrice;
    Payload_getMsg                   msgPayloadGetMsg;
    Payload_getVectorBool            msgPayloadGetVectorBool;
    Payload_getVectorChar            msgPayloadGetVectorChar;
    Payload_getVectorI8              msgPayloadGetVectorI8;
    Payload_getVectorU8              msgPayloadGetVectorU8;
    Payload_getVectorI16             msgPayloadGetVectorI16;
    Payload_getVectorU16             msgPayloadGetVectorU16;
    Payload_getVectorI32             msgPayloadGetVectorI32;
    Payload_getVectorU32             msgPayloadGetVectorU32;
    Payload_getVectorI64             msgPayloadGetVectorI64;
    Payload_getVectorU64             msgPayloadGetVectorU64;
    Payload_getVectorF32             msgPayloadGetVectorF32;
    Payload_getVectorF64             msgPayloadGetVectorF64;
    Payload_getVectorString          msgPayloadGetVectorString;
    Payload_getVectorDateTime        msgPayloadGetVectorDateTime;
    Payload_getVectorPrice           msgPayloadGetVectorPrice;
    Payload_getVectorMsg             msgPayloadGetVectorMsg;
    FieldPayload_create              msgFieldPayloadCreate;
    FieldPayload_destroy             msgFieldPayloadDestroy;
    FieldPayload_getName             msgFieldPayloadGetName;
    FieldPayload_getFid              msgFieldPayloadGetFid;
    FieldPayload_getDescriptor       msgFieldPayloadGetDescriptor;
    FieldPayload_getType             msgFieldPayloadGetType;
    FieldPayload_updateBool          msgFieldPayloadUpdateBool;
    FieldPayload_updateChar          msgFieldPayloadUpdateChar;
    FieldPayload_updateU8            msgFieldPayloadUpdateU8;
    FieldPayload_updateI8            msgFieldPayloadUpdateI8;
    FieldPayload_updateI16           msgFieldPayloadUpdateI16;
    FieldPayload_updateU16           msgFieldPayloadUpdateU16;
    FieldPayload_updateI32           msgFieldPayloadUpdateI32;
    FieldPayload_updateU32           msgFieldPayloadUpdateU32;
    FieldPayload_updateI64           msgFieldPayloadUpdateI64;
    FieldPayload_updateU64           msgFieldPayloadUpdateU64;
    FieldPayload_updateF32           msgFieldPayloadUpdateF32;
    FieldPayload_updateF64           msgFieldPayloadUpdateF64;
    FieldPayload_updateString        msgFieldPayloadUpdateString;
    FieldPayload_updateDateTime      msgFieldPayloadUpdateDateTime;
    FieldPayload_updatePrice         msgFieldPayloadUpdatePrice;
    FieldPayload_getBool             msgFieldPayloadGetBool;
    FieldPayload_getChar             msgFieldPayloadGetChar;
    FieldPayload_getI8               msgFieldPayloadGetI8;
    FieldPayload_getU8               msgFieldPayloadGetU8;
    FieldPayload_getI16              msgFieldPayloadGetI16;
    FieldPayload_getU16              msgFieldPayloadGetU16;
    FieldPayload_getI32              msgFieldPayloadGetI32;
    FieldPayload_getU32              msgFieldPayloadGetU32;
    FieldPayload_getI64              msgFieldPayloadGetI64;
    FieldPayload_getU64              msgFieldPayloadGetU64;
    FieldPayload_getF32              msgFieldPayloadGetF32;
    FieldPayload_getF64              msgFieldPayloadGetF64;
    FieldPayload_getString           msgFieldPayloadGetString;
    FieldPayload_getOpaque           msgFieldPayloadGetOpaque;
    FieldPayload_getDateTime         msgFieldPayloadGetDateTime;
    FieldPayload_getPrice            msgFieldPayloadGetPrice;
    FieldPayload_getMsg              msgFieldPayloadGetMsg;
    FieldPayload_getVectorBool       msgFieldPayloadGetVectorBool;
    FieldPayload_getVectorChar       msgFieldPayloadGetVectorChar;
    FieldPayload_getVectorI8         msgFieldPayloadGetVectorI8;
    FieldPayload_getVectorU8         msgFieldPayloadGetVectorU8;
    FieldPayload_getVectorI16        msgFieldPayloadGetVectorI16;
    FieldPayload_getVectorU16        msgFieldPayloadGetVectorU16;
    FieldPayload_getVectorI32        msgFieldPayloadGetVectorI32;
    FieldPayload_getVectorU32        msgFieldPayloadGetVectorU32;
    FieldPayload_getVectorI64        msgFieldPayloadGetVectorI64;
    FieldPayload_getVectorU64        msgFieldPayloadGetVectorU64;
    FieldPayload_getVectorF32        msgFieldPayloadGetVectorF32;
    FieldPayload_getVectorF64        msgFieldPayloadGetVectorF64;
    FieldPayload_getVectorString     msgFieldPayloadGetVectorString;
    FieldPayload_getVectorDateTime   msgFieldPayloadGetVectorDateTime;
    FieldPayload_getVectorPrice      msgFieldPayloadGetVectorPrice;
    FieldPayload_getVectorMsg        msgFieldPayloadGetVectorMsg;
    FieldPayload_getAsString         msgFieldPayloadGetAsString;

    PayloadIter_create               msgPayloadIterCreate;
    PayloadIter_next                 msgPayloadIterNext;
    PayloadIter_hasNext              msgPayloadIterHasNext;
    PayloadIter_begin                msgPayloadIterBegin;
    PayloadIter_end                  msgPayloadIterEnd;
    PayloadIter_associate            msgPayloadIterAssociate;
    PayloadIter_destroy              msgPayloadIterDestroy;

    void*                               closure;

    mamaPayloadLibrary mLibrary;   /**< Back-reference to parent library */
} mamaPayloadBridgeImpl;


#if defined (__cplusplus)
}
#endif

#endif /* mamaPayloadBridgeH__ */
