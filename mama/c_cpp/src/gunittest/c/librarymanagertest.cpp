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
#include "mama/middlewaremanager.h"
#include "mama/payloadmanager.h"
#include "mama/mama.h"
#include "middlewaremanager.h"
#include "bridge.h"
#include "payloadbridge.h"

class MamaLibraryManagerTestC : public ::testing::Test
{
protected:
    MamaLibraryManagerTestC() {};
    virtual ~MamaLibraryManagerTestC() {};

    virtual void SetUp();
    virtual void TearDown ();
public:
    MamaLibraryManagerTestC *m_this;
    mamaBridge mBridge;

};

void MamaLibraryManagerTestC::SetUp(void)
{
    m_this = this;

    mama_loadBridge (&mBridge, getMiddleware());
    mama_open ();
}

void MamaLibraryManagerTestC::TearDown(void)
{
    mama_close ();
    m_this = NULL;
}

/* ************************************************************************* */
/* Test Functions */
/* ************************************************************************* */

/*  Description: Ensure the library manager contains and return the correct information.
 *
 */

TEST_F (MamaLibraryManagerTestC, GetLoadedMiddlewares)
{
    mama_size_t           numBridges = 256;
    mamaMiddlewareLibrary bridges [256];

    mamaMiddlewareLibraryManager_getLibraries (bridges, &numBridges);

    EXPECT_EQ (1, numBridges);
}

TEST_F (MamaLibraryManagerTestC, GetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadLibrary payloads[256];
    mama_size_t        numPayloads = 256;

    mamaPayloadLibraryManager_getLibraries (payloads, &numPayloads);

    EXPECT_EQ (1, numPayloads);
}

TEST_F (MamaLibraryManagerTestC, GetOpenMiddlewares)
{
    mama_size_t           numBridges = 256;
    mamaMiddlewareLibrary bridges [256];

    mamaMiddlewareLibraryManager_getOpenedBridges (bridges, &numBridges);
    EXPECT_EQ (1, numBridges);
}

TEST_F (MamaLibraryManagerTestC, GetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaMiddlewareLibrary bridges[256];
    mama_size_t           numBridges = 256;

    mamaMiddlewareLibraryManager_getActiveBridges (bridges, &numBridges);

    EXPECT_EQ (0, numBridges);
}

#if 0
static void stopCb (mama_status status)
{
}

TEST_F (MamaLibraryManagerTestC, ActiveMiddlewares)
{
    int         active = 0;
    mama_status status = MAMA_STATUS_OK;

    mama_startBackground(mBridge, stopCb);

    if (status == MAMA_STATUS_OK)
    {
        status = mamaLibraryManager_isActive(mBridge, &active);
        while (!active && status == MAMA_STATUS_OK)
        {
            sleep(1);
            status = mamaLibraryManager_isActive(mBridge, &active);
        }

        mamaBridge* bridges    = NULL;
        mama_size_t numBridges = 0;

        mamaLibraryManager_getActiveMiddlewares (&bridges, &numBridges);

        EXPECT_EQ (1, numBridges);
        EXPECT_EQ (mBridge, bridges[0]);

        mama_stop(mBridge);

    }
}
#endif

TEST_F (MamaLibraryManagerTestC, GetInactiveMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareLibraryManager_getInactiveBridges (bridges, &numBridges);

    EXPECT_EQ (1, numBridges);
}

TEST_F (MamaLibraryManagerTestC, GetMiddleware)
{
    mamaMiddlewareLibrary bridge   = NULL;

    mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), &bridge);

    //ASSERT_EQ (mBridge, bridge);
}

TEST_F (MamaLibraryManagerTestC, GetPayload)
{
    mamaPayloadLibrary payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;
    char** payloadName = NULL;
    char*  payloadId   = NULL;

    mBridge->bridgeGetDefaultPayloadId (&payloadName, &payloadId);

    status = mamaPayloadLibraryManager_getLibrary (payloadName[0], &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TEST_F (MamaLibraryManagerTestC, GetPayloadById)
{
    mamaPayloadLibrary payload = NULL;
    mama_status        status  = MAMA_STATUS_OK;
    char** payloadName         = NULL;
    char*  payloadId           = NULL;

    mBridge->bridgeGetDefaultPayloadId(&payloadName, &payloadId);

    status = mamaPayloadLibraryManager_getLibraryById (payloadId[0], &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TEST_F (MamaLibraryManagerTestC, NullTestGetLoadedMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    mamaMiddlewareLibraryManager_getLibraries (bridges, &numBridges);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibraries (bridges, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibraries (NULL, &numBridges));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetLoadedPayloads)
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

TEST_F (MamaLibraryManagerTestC, NullTestGetOpenMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getOpenedBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getOpenedBridges (bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getActiveBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getActiveBridges (bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetInactiveMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getInactiveBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getInactiveBridges (bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetMiddleware)
{
    mamaMiddlewareLibrary bridge    = NULL;

    mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), &bridge);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibrary (NULL, &bridge));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetPayload)
{
    mamaPayloadLibrary payload = NULL;
    const char* payloadName[] = {"ABC", "DEF"};

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibrary (NULL, &payload));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaPayloadLibraryManager_getLibrary (payloadName[0], NULL));
}
