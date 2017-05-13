#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
#define FILLER_SIZE 50

class FillerRec
{
public:
  std::string typeName() { return typeid(*this).name(); }
  char buf[FILLER_SIZE];
};

class DummyRec
{
public:
  std::string typeName() { return typeid(*this).name(); }
};

class HandyDataHandle : public DataHandle
{
public:
  std::map<std::string, DeleteList::Entry> allItems;

  int initItemMetrics()
  {
    deleteList().collectMetrics(allItems);
    return allItems.size();
  }

  void assertItem(const std::string& name, unsigned int size, int numInHeap, int numInPool)
  {
    std::map<std::string, DeleteList::Entry>::const_iterator iter(allItems.find(name));
    if (iter == allItems.end())
      return CPPUNIT_ASSERT_MESSAGE(false, "expected allocated item not found");
    CPPUNIT_ASSERT_EQUAL(name, iter->first);
    CPPUNIT_ASSERT_EQUAL((size_t)size, iter->second.size);
    CPPUNIT_ASSERT_EQUAL((size_t)numInHeap, iter->second.nheap);
    CPPUNIT_ASSERT_EQUAL((size_t)numInPool, iter->second.npool);
  }
};

class DataHandleTest_mem : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(DataHandleTest_mem);
  CPPUNIT_TEST(testEmptyMetrics);
  CPPUNIT_TEST(testOneAlloc);
  CPPUNIT_TEST(testManyAlloc);
  CPPUNIT_TEST(testManyAllocTwoHandles);
  CPPUNIT_TEST(testImport);
  CPPUNIT_TEST_SUITE_END();

public:
  HandyDataHandle* _dh;
  void setUp() { _dh = new HandyDataHandle; }
  void tearDown() { delete _dh; }

protected:
  void testEmptyMetrics() { CPPUNIT_ASSERT_EQUAL(0, _dh->initItemMetrics()); }

  void testOneAlloc()
  {
    FillerRec* filler;
    _dh->get(filler);

    CPPUNIT_ASSERT_EQUAL(1, _dh->initItemMetrics());
    _dh->assertItem(filler->typeName(), sizeof(FillerRec), 0, 1); // one item in pool
  }
  void testManyAlloc()
  {
    FillerRec* filler;
    _dh->get(filler);
    _dh->get(filler);
    _dh->get(filler);

    DummyRec* dummy;
    _dh->get(dummy);

    CPPUNIT_ASSERT_EQUAL(2, _dh->initItemMetrics());
    // assertItem works as FIFO
    _dh->assertItem(filler->typeName(), sizeof(FillerRec), 0, 3); // three items in pool
    _dh->assertItem(dummy->typeName(), sizeof(DummyRec), 0, 1); // one item in pool
  }

  void testManyAllocTwoHandles()
  {
    FillerRec* filler;
    _dh->get(filler);
    _dh->get(filler);
    _dh->get(filler);

    HandyDataHandle dummyDH;
    DummyRec* dummy;
    dummyDH.get(dummy);

    // verify allocs are kept seperate
    CPPUNIT_ASSERT_EQUAL(1, _dh->initItemMetrics());
    _dh->assertItem(filler->typeName(), sizeof(FillerRec), 0, 3);
    CPPUNIT_ASSERT_EQUAL(1, dummyDH.initItemMetrics());
    dummyDH.assertItem(dummy->typeName(), sizeof(DummyRec), 0, 1);
  }

  void testImport()
  {
    FillerRec* filler;
    _dh->get(filler);
    _dh->get(filler);
    _dh->get(filler);

    HandyDataHandle dummyDH;
    dummyDH.import(*_dh); // moves the 3 allocs into dummyDH...
    _dh->get(filler); // ... but doesn't affect future allocs
    _dh->get(filler);

    DummyRec* dummy;
    dummyDH.get(dummy);

    // verify allocs are together when DeletList is imported
    CPPUNIT_ASSERT_EQUAL(2, dummyDH.initItemMetrics());
    dummyDH.assertItem(filler->typeName(), sizeof(FillerRec), 0, 3);
    dummyDH.assertItem(dummy->typeName(), sizeof(DummyRec), 0, 1);

    CPPUNIT_ASSERT_EQUAL(1, _dh->initItemMetrics());
    _dh->assertItem(filler->typeName(), sizeof(FillerRec), 0, 2); // allocs after import
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DataHandleTest_mem);
} // tse
