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
        mamaPayloadLibrary* library) = 0;

    virtual void unload (mamaPayloadLibrary library) = 0;
};

class NewMethodPayloadLoader : public PayloadLibraryLoader
{
public:
    void load (const char* path, 
        mamaPayloadLibrary* library)
    {
        mamaPayloadLibraryManager_loadLibraryWithPath (getPayload(),
                                                       NULL,
                                                       library);
    }

    void unload (mamaPayloadLibrary library)
    {
        mamaPayloadLibraryManager_unloadLib (library);
    }
};

class OldMethodPayloadLoader : public PayloadLibraryLoader
{
public:
    void load (const char* path, 
        mamaPayloadLibrary* library)
    {
        mamaPayloadBridge bridge;
        mama_loadPayloadBridge (&bridge, getPayload ());
        *library = bridge->mLibrary;
    }

    void unload (mamaPayloadLibrary library)
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
    mamaPayloadLibrary  mLibrary;
};

template <typename T> void PayloadManagerTestC<T>::SetUp(void)
{
    loader.load(NULL, &this->mLibrary);
}

template <typename T> void PayloadManagerTestC<T>::TearDown(void)
{
    loader.unload(this->mLibrary);
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

    mamaPayloadLibrary payloads[256];
    mama_size_t        numPayloads = 256;

    mamaPayloadLibraryManager_getLibraries (payloads, &numPayloads);

    EXPECT_EQ (1, numPayloads);
    EXPECT_EQ (this->mLibrary, payloads[0]);
}

TYPED_TEST_P (PayloadManagerTestC, GetPayload)
{
    mamaPayloadLibrary payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;

    status = mamaPayloadLibraryManager_getLibrary (getPayload(), &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TYPED_TEST_P (PayloadManagerTestC, GetPayloadById)
{
    mamaPayloadLibrary payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;

    status = mamaPayloadLibraryManager_getLibrary (getPayload(), &payload);
    
    ASSERT_EQ   (MAMA_STATUS_OK, status);
    ASSERT_TRUE (NULL != payload);

    char payloadId = 
        mamaPayloadLibraryManager_getId (payload);

    mamaPayloadLibrary payload0 = NULL;
    
    status = mamaPayloadLibraryManager_getLibraryById (payloadId, &payload0);

    ASSERT_EQ (payload, payload0);
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadLibrary payloads[256];
    mama_size_t    numPayloads = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibraries (payloads, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibraries (NULL, &numPayloads));
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetPayload)
{
    mamaPayloadLibrary payload = NULL;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibrary (NULL, &payload));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibrary (getPayload (), NULL));
}

TYPED_TEST_P (PayloadManagerTestC, NullTestGetPayloadById)
{
    char payloadId = 'A';
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibraryById (payloadId, NULL));
}

REGISTER_TYPED_TEST_CASE_P (PayloadManagerTestC,       GetLoadedPayloads, 
                            GetPayload,                GetPayloadById,            
                            NullTestGetLoadedPayloads, NullTestGetPayload,
                            NullTestGetPayloadById);

typedef ::testing::Types<OldMethodPayloadLoader, NewMethodPayloadLoader> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P (Mama, PayloadManagerTestC, MyTypes);
