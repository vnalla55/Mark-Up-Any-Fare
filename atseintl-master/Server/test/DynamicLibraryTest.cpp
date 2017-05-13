#include "test/include/CppUnitHelperMacros.h"

#include "Server/DynamicLibrary.h"

class DynamicLibraryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DynamicLibraryTest);
  CPPUNIT_TEST(testLoadUnloadSym);
  CPPUNIT_TEST_SUITE_END();

public:
  void testLoadUnloadSym()
  {
    tse::DynamicLibrary lib;

    // Check for bad input parms
    bool rc = lib.load("xx");
    CPPUNIT_ASSERT(rc == false);
    CPPUNIT_ASSERT(lib.loaded() == false);
    CPPUNIT_ASSERT(lib.error() == true);
    CPPUNIT_ASSERT(lib.errorMsg().size() > 0);

    // Check with a valid lib
    // rc = lib.load(TSE_VOB_DIR "/lib/" TSE_BUILD_TYPE "/libTO.so");
    #ifdef MOCK_MODULE_PATH
    rc = lib.load(MOCK_MODULE_PATH);
    #else
    rc = lib.load(TSE_VOB_DIR "/Server/test/" TSE_BUILD_TYPE "/libMockModule.so");
    #endif
    CPPUNIT_ASSERT_MESSAGE(lib.errorMsg(), rc == true);
    CPPUNIT_ASSERT(lib.loaded() == true);
    CPPUNIT_ASSERT(lib.loaded() != lib.error());

    // Now try to resolve a couple functions
    void* fnc = lib.symbol("mockFunction");
    bool (*fncPtr)() = (bool (*)())fnc;
    CPPUNIT_ASSERT(fnc != 0);
    CPPUNIT_ASSERT_MESSAGE(lib.errorMsg(), lib.error() == false);
    CPPUNIT_ASSERT_EQUAL(true, (*fncPtr)());

    fnc = lib.symbol("mockFunction2");
    fncPtr = (bool (*)())fnc;
    CPPUNIT_ASSERT(fnc != 0);
    CPPUNIT_ASSERT_MESSAGE(lib.errorMsg(), lib.error() == false);
    CPPUNIT_ASSERT_EQUAL(false, (*fncPtr)());

    // rc = lib.unload();
    // CPPUNIT_ASSERT_MESSAGE(lib.errorMsg(), rc == true);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DynamicLibraryTest);
