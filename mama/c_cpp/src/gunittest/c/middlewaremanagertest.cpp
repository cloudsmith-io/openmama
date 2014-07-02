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
        mamaMiddlewareLibrary* library) = 0;

    virtual void unload (mamaMiddlewareLibrary library) = 0;
};

class NewMethodLoader : public MiddlewareLibraryLoader
{
public:
    void load (const char* path, 
        mamaMiddlewareLibrary* library)
    {
        mamaMiddlewareLibraryManager_loadLibraryWithPath (getMiddleware(),
                                                          NULL,
                                                          library);
    
        mamaMiddlewareLibraryManager_openBridge (*library);
    
        mama_open ();
    }

    void unload (mamaMiddlewareLibrary library)
    {
        mama_close ();
    }
};

class OldMethodLoader : public MiddlewareLibraryLoader
{
public:
    void load (const char* path, 
        mamaMiddlewareLibrary* library)
    {
        mamaBridge bridge;
        mama_loadBridge (&bridge, getMiddleware ());
        *library = bridge->mLibrary;
        mama_open ();
    }

    void unload (mamaMiddlewareLibrary library)
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
    mamaMiddlewareLibrary  mLibrary;
};

template <typename T> void MiddlewareManagerTestC<T>::SetUp(void)
{
    loader.load(NULL, &this->mLibrary);
}

template <typename T> void MiddlewareManagerTestC<T>::TearDown(void)
{
    loader.unload(this->mLibrary);
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
    mama_size_t           numBridges = 256;
    mamaMiddlewareLibrary bridges [256];

    mamaMiddlewareLibraryManager_getLibraries (bridges, &numBridges);

    EXPECT_EQ (1,        numBridges);
    EXPECT_EQ (this->mLibrary, bridges[0]);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumLoadedMiddlewares)
{
    mama_size_t           numBridges = 256;

    mamaMiddlewareLibraryManager_getNumLibraries (&numBridges);

    EXPECT_EQ (1,        numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetOpenMiddlewares)
{
    mama_size_t           numBridges = 256;
    mamaMiddlewareLibrary bridges [256];

    mamaMiddlewareLibraryManager_getOpenedBridges (bridges, &numBridges);
    EXPECT_EQ (1, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumOpenMiddlewares)
{
    mama_size_t           numBridges = 256;

    mamaMiddlewareLibraryManager_getNumOpenedBridges (&numBridges);
    EXPECT_EQ (1, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mamaMiddlewareLibrary bridges[256];
    mama_size_t           numBridges = 256;

    mamaMiddlewareLibraryManager_getActiveBridges (bridges, &numBridges);

    EXPECT_EQ (0, numBridges);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetNumActiveMiddlewares)
{
    /*We havent called mama_start on the bridge therefore it should not
     * be dispatching*/
    mama_size_t           numBridges = 0;

    mamaMiddlewareLibraryManager_getNumActiveBridges (&numBridges);

    EXPECT_EQ (0, numBridges);
}

static mama_bool_t stopCb (mamaMiddlewareLibrary library, 
                           void*                 closure)
{
    mama_log (MAMA_LOG_LEVEL_ERROR, "Failed to start %s library",
              mamaMiddlewareLibraryManager_getName (library));
    return 1;
}

TYPED_TEST_P (MiddlewareManagerTestC, ActiveGetActiveMiddlewares)
{
    mama_size_t size   = 0;
    mama_status status = 
        mamaMiddlewareLibraryManager_startBridgeBackground (this->mLibrary, 
                                                             stopCb, NULL);

    if (status == MAMA_STATUS_OK)
    {
        status = mamaMiddlewareLibraryManager_getNumActiveBridges (&size);
        while (!size && status == MAMA_STATUS_OK)
        {
            sleep(1);
            status = mamaMiddlewareLibraryManager_getNumActiveBridges (&size);
        }

        mamaMiddlewareLibrary libraries[256];
        mama_size_t           numBridges = 256;

        mamaMiddlewareLibraryManager_getActiveBridges (libraries, &numBridges);

        EXPECT_EQ (1, numBridges);
        EXPECT_EQ (libraries[0], this->mLibrary);

        mamaMiddlewareLibraryManager_stopBridge (libraries[0]);

    }
}

TYPED_TEST_P (MiddlewareManagerTestC, GetInactiveMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareLibraryManager_getInactiveBridges (bridges, &numBridges);

    ASSERT_EQ (1, numBridges);
    ASSERT_EQ (this->mLibrary, bridges[0]);
}

TYPED_TEST_P (MiddlewareManagerTestC, GetClosedMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t numBridges = 256;

    mamaMiddlewareLibraryManager_getClosedBridges (bridges, &numBridges);

    ASSERT_EQ (0, numBridges);

    mamaMiddlewareLibraryManager_closeBridge (this->mLibrary); 
 
    numBridges = 256;
    mamaMiddlewareLibraryManager_getClosedBridges (bridges, &numBridges);
 
    ASSERT_EQ (1, numBridges);
    ASSERT_EQ (this->mLibrary, bridges[0]);
}
  
TYPED_TEST_P (MiddlewareManagerTestC, GetMiddleware)
{
    mamaMiddlewareLibrary bridge   = NULL;
    mama_status status = 
        mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), &bridge);

    ASSERT_EQ (this->mLibrary, bridge);
    ASSERT_EQ (MAMA_STATUS_OK, status);

    bridge = NULL;
    status = 
        mamaMiddlewareLibraryManager_getLibrary ("foobar", &bridge); 

    ASSERT_TRUE (NULL == bridge);
    ASSERT_EQ   (MAMA_STATUS_NOT_FOUND, status);  
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetLoadedMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    mamaMiddlewareLibraryManager_getLibraries (bridges, &numBridges);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibraries (bridges, NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibraries (NULL, &numBridges));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumLoadedMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getNumLibraries (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetOpenMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getOpenedBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getOpenedBridges (bridges, NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumOpenMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getNumOpenedBridges (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetActiveMiddlewares)
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

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetNumActiveMiddlewares)
{
    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getNumActiveBridges (NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetInactiveMiddlewares)
{
    mamaMiddlewareLibrary  bridges[256];
    mama_size_t        numBridges = 256;

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getInactiveBridges (NULL, &numBridges));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getInactiveBridges (bridges, NULL));
}

TYPED_TEST_P (MiddlewareManagerTestC, NullTestGetMiddleware)
{
    mamaMiddlewareLibrary bridge    = NULL;

    mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), &bridge);

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibrary (getMiddleware(), NULL));

    ASSERT_EQ (MAMA_STATUS_NULL_ARG,
        mamaMiddlewareLibraryManager_getLibrary (NULL, &bridge));
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
    mamaMiddlewareLibrary library = NULL;

    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareLibraryManager_loadLibraryWithPath (getMiddleware(),
                                                          NULL,
                                                          &library));
    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareLibraryManager_openBridge (library));

    ASSERT_EQ (MAMA_STATUS_OK, 
        mamaMiddlewareLibraryManager_openBridge (library));
 
    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareLibraryManager_closeBridge (library));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareLibraryManager_closeBridge (library));
}

TEST_F (MamaOpenCloseMiddlewareManagerTestC, closeBridge)
{
    mamaMiddlewareLibrary library = NULL;
    
    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareLibraryManager_loadLibraryWithPath (getMiddleware(),
                                                          NULL,
                                                          &library));
    ASSERT_EQ (MAMA_STATUS_SYSTEM_ERROR,
        mamaMiddlewareLibraryManager_closeBridge (library));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareLibraryManager_openBridge (library));

    ASSERT_EQ (MAMA_STATUS_OK,
        mamaMiddlewareLibraryManager_closeBridge (library));

    ASSERT_EQ (MAMA_STATUS_SYSTEM_ERROR,
        mamaMiddlewareLibraryManager_closeBridge (library));    
}

class MamaPropertiesMiddlewareManagerTestC : public ::testing::Test
{
protected:
    MamaPropertiesMiddlewareManagerTestC() {};
    virtual ~MamaPropertiesMiddlewareManagerTestC() {};

    void SetUp();
    void TearDown ();
public:
    mamaMiddlewareLibrary                 mLibrary;
};

void MamaPropertiesMiddlewareManagerTestC::SetUp()
{
    mamaBridge bridge;
    mama_loadBridge (&bridge, getMiddleware ());
    mLibrary = bridge->mLibrary;
    mama_open ();
}

void MamaPropertiesMiddlewareManagerTestC::TearDown ()
{
    mama_close ();
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, setProperty)
{
    ASSERT_EQ (MAMA_STATUS_OK,    
        mamaMiddlewareLibraryManager_setProperty  (
            mamaMiddlewareLibraryManager_getName (mLibrary),
            "description", "TEST"));

    char property[256];
    snprintf (property, 256, "mama.library.%s.%s", 
        mamaMiddlewareLibraryManager_getName (mLibrary),
        "description");

    ASSERT_STREQ ("TEST", mama_getProperty (property));
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, getName)
{
    ASSERT_STREQ (getMiddleware (),
        mamaMiddlewareLibraryManager_getName (mLibrary));
}

TEST_F (MamaPropertiesMiddlewareManagerTestC, getPath)
{
    ASSERT_EQ (NULL,
        mamaMiddlewareLibraryManager_getPath (mLibrary));
}
