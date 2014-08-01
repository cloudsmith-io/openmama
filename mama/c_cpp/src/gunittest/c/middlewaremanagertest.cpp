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
#include "mama/mama.h"
#include "middlewaremanager.h"
#include "bridge.h"

class MiddlewareLibraryLoader
{
public:
    virtual void load (const char* path, 
        mamaBridge* bridge) = 0;

    virtual void unload (mamaBridge bridge) = 0;
};

class NewMethodLoader : public MiddlewareLibraryLoader
{
public:
    void load (const char* path, 
             mamaBridge* bridge)
    {
        mamaMiddlewareManager_loadBridgeWithPath (getMiddleware(),
                                                  NULL,
                                                  bridge);
    
        mamaMiddlewareManager_openBridge (*bridge);
    
        mama_open ();
    }

    void unload (mamaBridge library)
    {
        mama_close ();
    }
};

class OldMethodLoader : public MiddlewareLibraryLoader
{
public:
    void load (const char* path, 
        mamaBridge* bridge)
    {
        mama_loadBridge (bridge, getMiddleware ());
        mama_open ();
    }

    void unload (mamaBridge bridge)
    {
        mama_close ();
    }
};

template <typename T>
class MiddlewareManagerTestC : public ::testing::Test
{
protected:
    MiddlewareManagerTestC() {};
    virtual ~MiddlewareManagerTestC() {};

    virtual void SetUp();
    virtual void TearDown ();

public:
    T loader;
    mamaBridge  mBridge;
};

template <typename T> void MiddlewareManagerTestC<T>::SetUp(void)
{
    loader.load(NULL, &this->mBridge);
}

template <typename T> void MiddlewareManagerTestC<T>::TearDown(void)
{
    loader.unload(this->mBridge);
}

TYPED_TEST_CASE_P (MiddlewareManagerTestC);

/* ************************************************************************* */
/* Test Functions */
/* ************************************************************************* */

/*  Description: Ensure the library manager contains and return the correct information.
 *
 */

TYPED_TEST_P (MiddlewareManagerTestC, GetLoadedMiddlewares)
{
    mama_size_t numBridges = 256;
    mamaBridge  bridges [256];

    mamaMiddlewareManager_getBridges (bridges, &numBridges);

    EXPECT_EQ (1,        numBridges);
    EXPECT_EQ (this->mBridge, bridges[0]);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumLoadedMiddlewares)
{
    mama_size_t           numBridges = 256;

    mamaMiddlewareManager_getNumBridges (&numBridges);

    EXPECT_EQ (1,        numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetOpenMiddlewares)
{
    mama_size_t numBridges = 256;
    mamaBridge  bridges [256];

    mamaMiddlewareManager_getOpenedBridges (bridges, &numBridges);
    EXPECT_EQ (1, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumOpenMiddlewares)
{
    mama_size_t           numBridges = 256;

    mamaMiddlewareManager_getNumOpenedBridges (&numBridges);
    EXPECT_EQ (1, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareManager_getActiveBridges (bridges, &numBridges);

    EXPECT_EQ (0, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mama_size_t           numBridges = 0;

    mamaMiddlewareManager_getNumActiveBridges (&numBridges);

    EXPECT_EQ (0, numBridges);
}

static mama_bool_t stopCb (mamaBridge bridge, 
                           void*      closure)
{
    mama_log (MAMA_LOG_LEVEL_ERROR, "Failed to start %s library",
              mamaMiddlewareManager_getName (bridge));
    return 1;
}

TYPED_TEST_P (MiddlewareManagerTestC, ActiveGetActiveMiddlewares)
{
    mama_size_t size   = 0;
    mama_status status = 
        mamaMiddlewareManager_startBridgeBackground (this->mBridge, 
                                                      stopCb, NULL);

    if (status == MAMA_STATUS_OK)
    {
        status = mamaMiddlewareManager_getNumActiveBridges (&size);
        while (!size && status == MAMA_STATUS_OK)
        {
            sleep(1);
            status = mamaMiddlewareManager_getNumActiveBridges (&size);
        }

        mamaBridge  bridges[256];
        mama_size_t numBridges = 256;

        mamaMiddlewareManager_getActiveBridges (bridges, &numBridges);

        EXPECT_EQ (1, numBridges);
        EXPECT_EQ (bridges[0], this->mBridge);

        mamaMiddlewareManager_stopBridge (bridges[0]);

    }
}

TYPED_TEST_P (MiddlewareManagerTestC, GetInactiveMiddlewares)
{
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareManager_getInactiveBridges (bridges, &numBridges);

    ASSERT_EQ (1, numBridges);
    ASSERT_EQ (this->mBridge, bridges[0]);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetClosedMiddlewares)
{
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareManager_getClosedBridges (bridges, &numBridges);

    ASSERT_EQ (0, numBridges);

    mamaMiddlewareManager_closeBridge (this->mBridge); 
 
    numBridges = 256;
    mamaMiddlewareManager_getClosedBridges (bridges, &numBridges);
 
    ASSERT_EQ (1, numBridges);
    ASSERT_EQ (this->mBridge, bridges[0]);
}
  
TYPED_TEST_P (MiddlewareManagerTestC, GetMiddleware)
{
    mamaBridge bridge   = NULL;
    mama_status status = 
        mamaMiddlewareManager_getBridge (getMiddleware(), &bridge);

    ASSERT_EQ (this->mBridge, bridge);
    ASSERT_EQ (MAMA_STATUS_OK, status);

    bridge = NULL;
    status = 
        mamaMiddlewareManager_getBridge ("foobar", &bridge); 

    ASSERT_TRUE (NULL == bridge);
    ASSERT_EQ   (MAMA_STATUS_NOT_FOUND, status);  
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetLoadedMiddlewares)
{
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareManager_getBridges (bridges, &numBridges);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getBridges (bridges, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getBridges (NULL, &numBridges));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumLoadedMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getNumBridges (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetOpenMiddlewares)
{
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getOpenedBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getOpenedBridges (bridges, NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumOpenMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getNumOpenedBridges (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaBridge  bridges[256];
    mama_size_t numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getActiveBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getActiveBridges (bridges, NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumActiveMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getNumActiveBridges (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetInactiveMiddlewares)
{
    mamaBridge   bridges[256];
    mama_size_t  numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getInactiveBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getInactiveBridges (bridges, NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetMiddleware)
{
    mamaBridge bridge    = NULL;

    mamaMiddlewareManager_getBridge (getMiddleware(), &bridge);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getBridge (getMiddleware(), NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareManager_getBridge (NULL, &bridge));
}

REGISTER_TYPED_TEST_CASE_P (MiddlewareManagerTestC,          GetLoadedMiddlewares, 
                            GetNumLoadedMiddlewares,         GetOpenMiddlewares,         
                            GetNumOpenMiddlewares,           GetActiveMiddlewares,       
                            GetNumActiveMiddlewares,         ActiveGetActiveMiddlewares,
                            GetInactiveMiddlewares,          GetClosedMiddlewares,           
                            GetMiddleware,                   NullTestGetLoadedMiddlewares,   
                            NullTestGetNumLoadedMiddlewares, NullTestGetOpenMiddlewares,     
                            NullTestGetNumOpenMiddlewares,   NullTestGetActiveMiddlewares,   
                            NullTestGetNumActiveMiddlewares, NullTestGetInactiveMiddlewares, 
                            NullTestGetMiddleware);

typedef ::testing::Types<OldMethodLoader, NewMethodLoader> MyTypes;
INSTANTIATE_TYPED_TEST_CASE_P (Mama, MiddlewareManagerTestC, MyTypes);

class MamaOpenCloseMiddlewareManagerTestC : public ::testing::Test
{
protected:
    MamaOpenCloseMiddlewareManagerTestC() {};
    virtual ~MamaOpenCloseMiddlewareManagerTestC() {};

    void SetUp() {};
    void TearDown () {};
};

TEST_F (MamaOpenCloseMiddlewareManagerTestC, openBridge)
{
    mamaBridge bridge = NULL;

    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareManager_loadBridgeWithPath (getMiddleware(),
                                                  NULL,
                                                  &bridge));
    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareManager_openBridge (bridge));

    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareManager_openBridge (bridge));
 
    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareManager_closeBridge (bridge));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareManager_closeBridge (bridge));
}

TEST_F (MamaOpenCloseMiddlewareManagerTestC, closeBridge)
{
    mamaBridge bridge = NULL;
    
    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareManager_loadBridgeWithPath (getMiddleware(),
                                                  NULL,
                                                  &bridge));
    ASSERT_EQ (MAMA_STATUS_SYSTEM_ERROR,
        mamaMiddlewareManager_closeBridge (bridge));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareManager_openBridge (bridge));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareManager_closeBridge (bridge));

    ASSERT_EQ (MAMA_STATUS_SYSTEM_ERROR,
        mamaMiddlewareManager_closeBridge (bridge));    
}

class MamaPropertiesMiddlewareManagerTestC : public ::testing::Test
{
protected:
    MamaPropertiesMiddlewareManagerTestC() {};
    virtual ~MamaPropertiesMiddlewareManagerTestC() {};

    void SetUp();
    void TearDown ();
public:
    mamaBridge                 mBridge;
};

void MamaPropertiesMiddlewareManagerTestC::SetUp()
{
    mama_loadBridge (&mBridge, getMiddleware ());
    mama_open ();
}

void MamaPropertiesMiddlewareManagerTestC::TearDown ()
{
    mama_close ();
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, setProperty)
{
    ASSERT_EQ (MAMA_STATUS_OK,    
        mamaMiddlewareManager_setProperty  (
            mamaMiddlewareManager_getName (mBridge),
            "description", "TEST"));

    char property[256];
    snprintf (property, 256, "mama.library.%s.%s", 
        mamaMiddlewareManager_getName (mBridge),
        "description");

    ASSERT_STREQ ("TEST", mama_getProperty (property));
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, getName)
{
    ASSERT_STREQ (getMiddleware (),
        mamaMiddlewareManager_getName (mBridge));
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, getPath)
{
    ASSERT_EQ (NULL,
        mamaMiddlewareManager_getPath (mBridge));
}
