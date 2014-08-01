/* $Id: librarymanagertest.cpp,v 1.1.2.1.2.4 2012/08/24 16:12:00 rossgeddis Exp $
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

#include <gtest/gtest.h>
#include "MainUnitTestC.h"
#include "mama/payloadmanager.h"
#include "mama/mama.h"
#include "payloadbridge.h"

class PayloadLibraryLoader
{
public:
    virtual void load (const char* path, 
        mamaPayloadBridge* bridge) = 0;

    virtual void unload (mamaPayloadBridge bridge) = 0;
};

class NewMethodPayloadLoader : public PayloadLibraryLoader
{
public:
    void load (const char* path, 
        mamaPayloadBridge* bridge)
    {
        mamaPayloadManager_loadBridgeWithPath (getPayload(),
                                               NULL,
                                               bridge);
    }

    void unload (mamaPayloadBridge bridge)
    {
        mamaPayloadManager_unloadBridge (bridge);
    }
};

class OldMethodPayloadLoader : public PayloadLibraryLoader
{
public:
    void load (const char* path, 
        mamaPayloadBridge* bridge)
    {
        mama_loadPayloadBridge (bridge, getPayload ());
    }

    void unload (mamaPayloadBridge bridge)
    {
    }
};

template <typename T>
class PayloadManagerTestC : public ::testing::Test
{
protected:
    PayloadManagerTestC() {};
    virtual ~PayloadManagerTestC() {};

    virtual void SetUp();
    virtual void TearDown ();

public:
    T loader;
    mamaPayloadBridge  mBridge;
};

template <typename T> void PayloadManagerTestC<T>::SetUp(void)
{
    loader.load(NULL, &this->mBridge);
}

template <typename T> void PayloadManagerTestC<T>::TearDown(void)
{
    loader.unload(this->mBridge);
}

TYPED_TEST_CASE_P (PayloadManagerTestC);

/* ************************************************************************* */
/* Test Functions */
/* ************************************************************************* */

/*  Description: Ensure the library manager contains and return the correct information.
 *
 */

TYPED_TEST_P (PayloadManagerTestC, GetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadBridge payloads[256];
    mama_size_t       numPayloads = 256;

    mamaPayloadManager_getBridges (payloads, &numPayloads);

    EXPECT_EQ (1, numPayloads);
    EXPECT_EQ (this->mBridge, payloads[0]);
}

TYPED_TEST_P (PayloadManagerTestC, GetDefaultPayload)
{
    mamaPayloadBridge payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;

    status = mamaPayloadManager_getDefaultBridge (&payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TYPED_TEST_P (PayloadManagerTestC, GetPayload)
{
    mamaPayloadBridge payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;

    status = mamaPayloadManager_getBridge (getPayload(), &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);

    payload = NULL;
    status = mamaPayloadManager_getBridge ("foobar", &payload);

    ASSERT_TRUE (NULL == payload);
    ASSERT_EQ   (MAMA_STATUS_NOT_FOUND, status);
}

TYPED_TEST_P (PayloadManagerTestC, GetPayloadById)
{
    mamaPayloadBridge payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;

    status = mamaPayloadManager_getBridge (getPayload(), &payload);
    
    ASSERT_EQ   (MAMA_STATUS_OK, status);
    ASSERT_TRUE (NULL != payload);

    char payloadId = 
        mamaPayloadManager_getId (payload);

    mamaPayloadBridge payload0 = NULL;
    
    status = mamaPayloadManager_getBridgeById (payloadId, &payload0);

    ASSERT_EQ (payload, payload0);
    ASSERT_EQ (MAMA_STATUS_OK, status);

    status = mamaPayloadManager_getBridgeById ('\r', &payload0);

    ASSERT_TRUE (NULL == payload0);
    ASSERT_EQ (MAMA_STATUS_NO_BRIDGE_IMPL, status);
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadBridge payloads[256];
    mama_size_t    numPayloads = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadManager_getBridges (payloads, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadManager_getBridges (NULL, &numPayloads));
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetPayload)
{
    mamaPayloadBridge payload = NULL;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadManager_getBridge (NULL, &payload));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadManager_getBridge (getPayload (), NULL));
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetPayloadById)
{
    char payloadId = 'A';
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadManager_getBridgeById (payloadId, NULL));
}

REGISTER_TYPED_TEST_CASE_P (PayloadManagerTestC, GetLoadedPayloads, 
                            GetPayload,          GetDefaultPayload,
                            GetPayloadById,      NullTestGetLoadedPayloads, 
                            NullTestGetPayload,  NullTestGetPayloadById);

typedef ::testing::Types<OldMethodPayloadLoader, NewMethodPayloadLoader> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P (Mama, PayloadManagerTestC, MyTypes);

class MamaPropertiesPayloadManagerTestC : public ::testing::Test
{
protected:
    MamaPropertiesPayloadManagerTestC() {};
    virtual ~MamaPropertiesPayloadManagerTestC() {};

    void SetUp();
    void TearDown ();
public:
    mamaPayloadBridge                 mBridge;
};

void MamaPropertiesPayloadManagerTestC::SetUp()
{
    mama_loadPayloadBridge (&mBridge, getPayload ());
}

void MamaPropertiesPayloadManagerTestC::TearDown ()
{
}

TEST_F (MamaPropertiesPayloadManagerTestC, setProperty)
{
    ASSERT_EQ (MAMA_STATUS_OK,    
        mamaPayloadManager_setProperty  (
            mamaPayloadManager_getName (mBridge),
            "description", "TEST"));

    char property[256];
    snprintf (property, 256, "mama.library.%s.%s", 
        mamaPayloadManager_getName (mBridge),
        "description");

    ASSERT_STREQ ("TEST", mama_getProperty (property));
}

TEST_F (MamaPropertiesPayloadManagerTestC, getName)
{
    ASSERT_STREQ (getPayload (),
        mamaPayloadManager_getName (mBridge));
}

TEST_F (MamaPropertiesPayloadManagerTestC, getPath)
{
    ASSERT_EQ (NULL,
        mamaPayloadManager_getPath (mBridge));
}
