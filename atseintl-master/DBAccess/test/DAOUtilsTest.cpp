//----------------------------------------------------------------------------
//	File: DAOUtilsTest.cpp
//
//	Author:       Tony Lam
//  	Created:      09/18/2007
//  	Description:  This is a unit test class for DAOUtils.cpp
//
//  Copyright Sabre 2005
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
#include <algorithm>
#include <functional>
#include "test/include/CppUnitHelperMacros.h"
#include <log4cxx/propertyconfigurator.h>

#include "Common/Config/ConfigMan.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/DAOUtils.h"
#include "Common/DateTime.h"
#include "DBAccess/DeleteList.h"

namespace tse
{
class DAOUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DAOUtilsTest);
  CPPUNIT_TEST(getDateRangeTestDaily);
  CPPUNIT_TEST(getWeeklyDateRange);
  CPPUNIT_TEST(getMonthlyDateRange);
  CPPUNIT_TEST(getYearlyDateRangeTest);
  CPPUNIT_TEST_SUITE_END();

public:
  DateTime ticketDate;
  TestMemHandle _memHandle;

  void setUp()
  {
    ticketDate = DateTime(2007, 02, 27, 13, 12, 11);
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void getDateRangeTestDaily()
  {
    const DateTime expectedStartDate(2007, 02, 27);
    const DateTime expectedEndDate(2007, 02, 27, 23, 59, 59);
    verifyDateRange(ticketDate, DAOUtils::DAILY, expectedStartDate, expectedEndDate);
    return;
  }

  void getWeeklyDateRange()
  {
    const DateTime expectedStartDate(2007, 02, 25);
    const DateTime expectedEndDate(2007, 03, 03, 23, 59, 59);
    verifyDateRange(ticketDate, DAOUtils::WEEKLY, expectedStartDate, expectedEndDate);

    uint32_t tuesday = 2;
    uint32_t dow = ticketDate.date().day_of_week().as_number();
    CPPUNIT_ASSERT_EQUAL(tuesday, dow);
    return;
  }

  void getMonthlyDateRange()
  {
    const DateTime expectedStartDate(2007, 02, 1);
    const DateTime expectedEndDate(2007, 02, 28, 23, 59, 59);
    verifyDateRange(ticketDate, DAOUtils::MONTHLY, expectedStartDate, expectedEndDate);
    return;
  }

  void getYearlyDateRangeTest()
  {
    const DateTime expectedStartDate(2007, 01, 1);
    const DateTime expectedEndDate(2007, 12, 31, 23, 59, 59);
    verifyDateRange(ticketDate, DAOUtils::YEARLY, expectedStartDate, expectedEndDate);
    return;
  }

  void verifyDateRange(const DateTime& ticketDate,
                       const DAOUtils::CacheBy cacheBy,
                       const DateTime& expectedStartDate,
                       const DateTime& expectedEndDate)
  {
    DateTime startDate, endDate;
    DAOUtils::getDateRange(ticketDate, startDate, endDate, cacheBy);

    CPPUNIT_ASSERT_EQUAL(expectedStartDate, startDate);
    CPPUNIT_ASSERT_EQUAL(expectedEndDate, endDate);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DAOUtilsTest);
}
