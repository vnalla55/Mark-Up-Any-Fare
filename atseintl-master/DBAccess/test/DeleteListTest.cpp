//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/DeleteList.h"

using namespace std;

namespace tse
{
class DeleteListTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DeleteListTest);
  CPPUNIT_TEST(testDefaultConstructorDestructor);
  CPPUNIT_TEST(testConstructorDestructorWithMultipleContainers);
  CPPUNIT_TEST(testAdopt);
  CPPUNIT_TEST(testCopy);
  CPPUNIT_TEST(testImport);
  CPPUNIT_TEST(testImportWithOutDebugingOn);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    // Debug flags must be set to 15 in order to set count
    // variables for validation
    DeleteList::setDebugFlags(15);
    clearVariables();
    // Set level to debug because we need to test duplicated logic
    // ran with and without debugging turned on.
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
  }

  void tearDown()
  {
    // Reset flags on exit.
    DeleteList::setDebugFlags(0);
  }

  void testDefaultConstructorDestructor() { CPPUNIT_ASSERT_NO_THROW(DeleteList dl); }

  void testConstructorDestructorWithMultipleContainers()
  {
    CPPUNIT_ASSERT_NO_THROW(DeleteList dl(5));
    // Depending on the environment, each count may very, but the total should be 10
    CPPUNIT_ASSERT_EQUAL(size_t(10),
                         DeleteList::_largeCount + DeleteList::_mediumCount +
                             DeleteList::_smallCount + DeleteList::_singleCount);
  }

  void testAdopt()
  {
    string* data = new string("testing");
    {
      DeleteList dl;
      dl.adopt<string>(data);
    }
    CPPUNIT_ASSERT(DeleteList::_destroyCount == 1);
  }

  void testCopy()
  {
    std::shared_ptr<string> data1(new string("testing"));
    std::shared_ptr<string> data2(new string("testing"));
    std::shared_ptr<string> data3(new string("testing"));
    {
      DeleteList dl(5);
      dl.copy<std::shared_ptr<string>>(data1);
      dl.copy<std::shared_ptr<string>>(data2);
      dl.copy<std::shared_ptr<string>>(data3);
    }
    CPPUNIT_ASSERT(DeleteList::_destroyCount == 1);
    // Depending on the environment, each count may very, but the total should be 10
    // printVariables();
    CPPUNIT_ASSERT_EQUAL(size_t(10),
                         DeleteList::_largeCount + DeleteList::_mediumCount +
                             DeleteList::_smallCount + DeleteList::_singleCount);
  }

  void testImport()
  {
    std::shared_ptr<string> data(new string("testing"));
    string* data2 = new string("testing");
    {
      DeleteList dl1;
      dl1.copy<std::shared_ptr<string>>(data);
      dl1.adopt<string>(data2);
      DeleteList dl2;
      dl2.import(dl1);
      CPPUNIT_ASSERT(DeleteList::_destroyCount == 0);
      CPPUNIT_ASSERT(DeleteList::_importCount == 1);
      CPPUNIT_ASSERT_EQUAL(size_t(0), DeleteList::_singleCount);
    }
    CPPUNIT_ASSERT(DeleteList::_destroyCount == 2);
    CPPUNIT_ASSERT_EQUAL(size_t(4), DeleteList::_singleCount);
  }

  void testImportWithOutDebugingOn()
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getInfo());
    std::shared_ptr<string> data(new string("testing"));
    string* data2 = new string("testing");
    {
      DeleteList dl1;
      dl1.copy<std::shared_ptr<string>>(data);
      dl1.adopt<string>(data2);
      DeleteList dl2;
      dl2.import(dl1);
      CPPUNIT_ASSERT(DeleteList::_destroyCount == 0);
      CPPUNIT_ASSERT(DeleteList::_importCount == 1);
    }
    CPPUNIT_ASSERT(DeleteList::_destroyCount == 2);
  }

  void clearVariables()
  {
    DeleteList::_destroyCount = 0;
    DeleteList::_importCount = 0;
    DeleteList::_singleCount = 0;
    DeleteList::_smallCount = 0;
    DeleteList::_mediumCount = 0;
    DeleteList::_largeCount = 0;
  }

  void printVariables()
  {
    cout << "destroyCount = " << DeleteList::_destroyCount << endl;
    cout << "importCount = " << DeleteList::_importCount << endl;
    cout << "singleCount = " << DeleteList::_singleCount << endl;
    cout << "smallCount = " << DeleteList::_smallCount << endl;
    cout << "mediumCount = " << DeleteList::_mediumCount << endl;
    cout << "largeCount = " << DeleteList::_largeCount << endl;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(tse::DeleteListTest);
} // namespace tse
