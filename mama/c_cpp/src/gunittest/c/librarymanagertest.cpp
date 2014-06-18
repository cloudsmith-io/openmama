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
    mama_size_t numBridges = 256;
    mamaBridge bridges [256];

    mamaMiddlewareLibraryManager_getBridges (bridges, &numBridges);

    EXPECT_EQ (1, numBridges);
}

#if 0
TEST_F (MamaLibraryManagerTestC, GetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadBridge* payloads    = NULL;
    mama_size_t        numPayloads = 0;

    char** payloadName = NULL;
    char*  payloadId   = NULL;

    mamaLibraryManager_getLoadedPayloads (&payloads, &numPayloads);
    mBridge->bridgeGetDefaultPayloadId(&payloadName, &payloadId);

    EXPECT_EQ (1, numPayloads);
    EXPECT_EQ (payloadId[0], payloads[0]->msgPayloadGetType());

    free (payloads);
}
#endif

TEST_F (MamaLibraryManagerTestC, GetOpenMiddlewares)
{
    mama_size_t numBridges = 256;
    mamaBridge bridges [256];

    mamaMiddlewareLibraryManager_getOpenedBridges (bridges, &numBridges);
    EXPECT_EQ (1, numBridges);
}

#if 0
TEST_F (MamaLibraryManagerTestC, GetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    mamaLibraryManager_getActiveMiddlewares (&bridges, &numBridges);

    EXPECT_EQ (0, numBridges);

    free (bridges);
}

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

        free (bridges);
    }
}

TEST_F (MamaLibraryManagerTestC, GetInactiveMiddlewares)
{
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    mamaLibraryManager_getInactiveMiddlewares (&bridges, &numBridges);

    EXPECT_EQ (1, numBridges);
    EXPECT_EQ (mBridge, bridges[0]);

    free (bridges);
}
TEST_F (MamaLibraryManagerTestC, GetMiddleware)
{
    mamaBridge bridge    = NULL;

    mamaLibraryManager_getMiddleware (getMiddleware(), &bridge);

    ASSERT_EQ (mBridge, bridge);
}

TEST_F (MamaLibraryManagerTestC, GetPayload)
{
    mamaPayloadBridge payload = NULL;
    mama_status       status  = MAMA_STATUS_OK;
    char** payloadName = NULL;
    char*  payloadId   = NULL;

    mBridge->bridgeGetDefaultPayloadId (&payloadName, &payloadId);

    status = mamaLibraryManager_getPayload (payloadName[0], &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TEST_F (MamaLibraryManagerTestC, GetPayloadById)
{
    mamaPayloadBridge payload = NULL;
    mama_status       status  = MAMA_STATUS_OK;
    char** payloadName        = NULL;
    char*  payloadId          = NULL;

    mBridge->bridgeGetDefaultPayloadId(&payloadName, &payloadId);

    status = mamaLibraryManager_getPayloadById (payloadId[0], &payload);

    ASSERT_TRUE (NULL != payload);
    ASSERT_EQ (MAMA_STATUS_OK, status);
}

TEST_F (MamaLibraryManagerTestC, isActive)
{
    int active = 0;

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaLibraryManager_isActive(mBridge, &active));

    ASSERT_EQ(0, active);
}

TEST_F (MamaLibraryManagerTestC, isOpen)
{
    int open = 0;
    ASSERT_EQ (MAMA_STATUS_OK,
        mamaLibraryManager_isActive(mBridge, &open));

    ASSERT_EQ(0, open);
}

TEST_F (MamaLibraryManagerTestC, NullTestGetLoadedMiddlewares)
{
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    mamaLibraryManager_getLoadedMiddlewares (&bridges, &numBridges);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getLoadedMiddlewares (&bridges, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getLoadedMiddlewares (NULL, &numBridges));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetLoadedPayloads)
{
    /*mama_open loads the default middleware for each bridge
     * so although we have not loaded one explicitly there
     * should be one payload bridge loaded at the very least.*/

    mamaPayloadBridge* payloads    = NULL;
    mama_size_t        numPayloads = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getLoadedPayloads (&payloads, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getLoadedPayloads (NULL, &numPayloads));

}

TEST_F (MamaLibraryManagerTestC, NullTestGetOpenMiddlewares)
{
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getOpenMiddlewares (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getOpenMiddlewares (&bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getActiveMiddlewares (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getActiveMiddlewares (&bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetInactiveMiddlewares)
{
    mamaBridge* bridges    = NULL;
    mama_size_t numBridges = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getInactiveMiddlewares (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getInactiveMiddlewares (&bridges, NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetMiddleware)
{
    mamaBridge bridge    = NULL;

    mamaLibraryManager_getMiddleware (getMiddleware(), &bridge);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getMiddleware (getMiddleware(), NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getMiddleware (NULL, &bridge));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetPayload)
{
    mamaPayloadBridge payload = NULL;
    const char* payloadName[] = {"ABC", "DEF"};

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getPayload (NULL, &payload));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getPayload (payloadName[0], NULL));
}

TEST_F (MamaLibraryManagerTestC, NullTestGetPayloadById)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaLibraryManager_getPayloadById ('A', NULL));
}

TEST_F (MamaLibraryManagerTestC, NullResultisActive)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
     mamaLibraryManager_isActive(mBridge, NULL));

    int result = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
     mamaLibraryManager_isActive(NULL, &result));
}

TEST_F (MamaLibraryManagerTestC, NullResultisOpen)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
     mamaLibraryManager_isOpen(mBridge, NULL));

    int result = 0;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
     mamaLibraryManager_isOpen(NULL, &result));
}
#endif
