//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <boost/assign/std/set.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/GroupsFilter.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <memory>

using namespace boost::assign;

namespace tse
{
namespace flexFares
{

class GroupsFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GroupsFilterTest);

  CPPUNIT_TEST(testFilterOutPublicFares);
  CPPUNIT_TEST(testFilterOutAccCodes);
  CPPUNIT_TEST(testFilterOut);
  CPPUNIT_TEST(testFilterOutPublicFaresNew);
  CPPUNIT_TEST(testFilterOutAccCodesNew);
  CPPUNIT_TEST(testFilterOutNew);


  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;

  ValidationStatusPtr _status;
  TotalAttrs* _totalAttrs;

public:
  void setUp()
  {
    _status = std::make_shared<ValidationStatus>();
    _totalAttrs = _memHandle.create<TotalAttrs>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFilterOutPublicFares()
  {
    _status->setStatus<flexFares::PUBLIC_FARES>(false);

    _totalAttrs->addGroup<flexFares::PUBLIC_FARES>(2);

    GroupsIds validGroups;
    validGroups += 1, 2, 3;

    GroupsFilter filter(*_totalAttrs);
    filter.filterOutInvalidGroups(_status, validGroups);

    CPPUNIT_ASSERT_EQUAL(size_t(2), validGroups.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(1));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(3));
  }

  void testFilterOutAccCodes()
  {
    _status->setValidAccCode("A");
    _status->setValidAccCode("B");

    _totalAttrs->addAccCode("A", 1);
    _totalAttrs->addAccCode("B", 2);
    _totalAttrs->addAccCode("A", 3);
    _totalAttrs->addAccCode("B", 3);
    _totalAttrs->addAccCode("C", 4);
    _totalAttrs->addAccCode("A", 5);
    _totalAttrs->addAccCode("C", 5);

    GroupsIds validGroups;
    validGroups += 1, 2, 3, 4, 5, 6;

    GroupsFilter filter(*_totalAttrs);
    filter.filterOutInvalidGroups(_status, validGroups);

    CPPUNIT_ASSERT_EQUAL(size_t(5), validGroups.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(1));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(2));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(3));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(5));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(6));
  }

  void testFilterOut()
  {
    _status->setStatus<NO_MIN_MAX_STAY>(FAIL);
    _status->setValidCorpId("A");
    _status->setValidCorpId("C");

    _totalAttrs->addGroup<NO_PENALTIES>(1);
    _totalAttrs->addGroup<NO_PENALTIES>(2);
    _totalAttrs->addGroup<NO_MIN_MAX_STAY>(1);

    _totalAttrs->addCorpId("A", 1);
    _totalAttrs->addCorpId("C", 1);
    _totalAttrs->addCorpId("A", 3);
    _totalAttrs->addCorpId("B", 4);

    GroupsIds validGroups;
    validGroups += 1, 2, 3, 4;

    GroupsFilter filter(*_totalAttrs);
    filter.filterOutInvalidGroups(_status, validGroups);

    CPPUNIT_ASSERT_EQUAL(size_t(2), validGroups.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(2));
    CPPUNIT_ASSERT_EQUAL(size_t(1), validGroups.count(3));
  }

  void testFilterOutPublicFaresNew()
  {
    _status->setStatus<flexFares::PUBLIC_FARES>(false);
    _totalAttrs->addGroup<flexFares::PUBLIC_FARES>(7);

    GroupId validGroup = 7;

    GroupsFilter filter(*_totalAttrs);
    bool rc = filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(false, rc);

    _status->setStatus<flexFares::PUBLIC_FARES>(true);
    _totalAttrs->addGroup<flexFares::PUBLIC_FARES>(11);

    validGroup = 8;
    rc = filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testFilterOutAccCodesNew()
  {
    _status->setValidAccCode("A");
    _status->setValidAccCode("B");
    _status->setValidCorpId("C");

    _totalAttrs->addAccCode("A", 9);
    _totalAttrs->addAccCode("B", 9);
    _totalAttrs->addCorpId("C", 9);

    GroupId validGroup = 9;

    GroupsFilter filter(*_totalAttrs);
    bool rc= filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(true, rc);

    validGroup = 10;
    rc= filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(false, rc);
   }

  void testFilterOutNew()
  {
    _status->setStatus<NO_MIN_MAX_STAY>(FAIL);
    _status->setValidCorpId("A");
    _status->setValidCorpId("C");

    _totalAttrs->addGroup<NO_PENALTIES>(11);
    _totalAttrs->addGroup<NO_PENALTIES>(12);
    _totalAttrs->addGroup<NO_MIN_MAX_STAY>(11);

    _totalAttrs->addCorpId("A", 11);
    _totalAttrs->addCorpId("C", 12);

    bool rc = false;

    GroupId validGroup = 11;
    GroupsFilter filter(*_totalAttrs);
    rc = filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(false, rc);

    validGroup = 12;
    rc = filter.filterOutInvalidGroups(_status, validGroup);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(GroupsFilterTest);
}
}
