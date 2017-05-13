// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Rules/TaxLimiter.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tax
{
class TaxLimiterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxLimiterTest);

  CPPUNIT_TEST(testBlank);
  CPPUNIT_TEST(testOnceForItin);
  CPPUNIT_TEST(testOnceForItinAndBlankHighestNotBlank);
  CPPUNIT_TEST(testOnceForItinAndBlankHighestBlank);

  CPPUNIT_TEST(testOverlapItinerary);

  CPPUNIT_TEST_SUITE_END();
public:
  void setUp()
  {
  }

  void tearDown()
  {
    _limits.clear();
    _amounts.clear();
  }

  void testBlank()
  {
    for (type::Index i = 0; i < 6; ++i)
    {
      _limits.push_back(type::TaxApplicationLimit::Unlimited);
      _amounts.push_back(type::MoneyAmount(70 - 3 * i, 1));
    }

    std::vector<bool> result = TaxLimiter::limit(_limits, _amounts);

    CPPUNIT_ASSERT_EQUAL(_limits.size(), result.size());
    CPPUNIT_ASSERT(result[0]);
    CPPUNIT_ASSERT(result[1]);
    CPPUNIT_ASSERT(result[2]);
    CPPUNIT_ASSERT(result[3]);
    CPPUNIT_ASSERT(result[4]);
    CPPUNIT_ASSERT(result[5]);
  }

  void testOnceForItin()
  {
    for (int i = 0; i < 6; ++i)
    {
      _limits.push_back(type::TaxApplicationLimit::OnceForItin);
      _amounts.push_back(type::MoneyAmount(22 - ((i - 2) * (i - 5)), 1)); // 12, 18, 22, 24, 24, 22
    }

    std::vector<bool> result = TaxLimiter::limit(_limits, _amounts);

    CPPUNIT_ASSERT_EQUAL(_limits.size(), result.size());
    CPPUNIT_ASSERT(!result[0]);
    CPPUNIT_ASSERT(!result[1]);
    CPPUNIT_ASSERT(!result[2]);
    CPPUNIT_ASSERT(result[3]); //first out of maximal amounts
    CPPUNIT_ASSERT(!result[4]);
    CPPUNIT_ASSERT(!result[5]);
  }

  void testOnceForItinAndBlankHighestNotBlank()
  {
    _limits.resize(6, type::TaxApplicationLimit::Unlimited);
    _amounts.resize(6);
    _limits[0] = type::TaxApplicationLimit::OnceForItin; _amounts[0] = 40;
    _limits[1] = type::TaxApplicationLimit::OnceForItin; _amounts[1] = 70;
    _limits[2] = type::TaxApplicationLimit::Unlimited; _amounts[2] = 30;
    _limits[3] = type::TaxApplicationLimit::OnceForItin; _amounts[3] = 50;
    _limits[4] = type::TaxApplicationLimit::Unlimited; _amounts[4] = 20;
    _limits[5] = type::TaxApplicationLimit::OnceForItin; _amounts[5] = 10;

    std::vector<bool> result = TaxLimiter::limit(_limits, _amounts);

    CPPUNIT_ASSERT_EQUAL(_limits.size(), result.size());
    CPPUNIT_ASSERT(!result[0]);
    CPPUNIT_ASSERT(result[1]); // maximal
    CPPUNIT_ASSERT(result[2]); // blank
    CPPUNIT_ASSERT(!result[3]);
    CPPUNIT_ASSERT(result[4]); // blank
    CPPUNIT_ASSERT(!result[5]);
  }

  void testOnceForItinAndBlankHighestBlank()
  {
    _limits.resize(6, type::TaxApplicationLimit::Unlimited);
    _amounts.resize(6);
    _limits[0] = type::TaxApplicationLimit::OnceForItin; _amounts[0] = 40;
    _limits[1] = type::TaxApplicationLimit::OnceForItin; _amounts[1] = 70;
    _limits[2] = type::TaxApplicationLimit::Unlimited; _amounts[2] = 30;
    _limits[3] = type::TaxApplicationLimit::OnceForItin; _amounts[3] = 50;
    _limits[4] = type::TaxApplicationLimit::Unlimited; _amounts[4] = 90;
    _limits[5] = type::TaxApplicationLimit::OnceForItin; _amounts[5] = 10;

    std::vector<bool> result = TaxLimiter::limit(_limits, _amounts);

    CPPUNIT_ASSERT_EQUAL(_limits.size(), result.size());
    CPPUNIT_ASSERT(!result[0]);
    CPPUNIT_ASSERT(!result[1]);
    CPPUNIT_ASSERT(result[2]); // blank
    CPPUNIT_ASSERT(!result[3]);
    CPPUNIT_ASSERT(result[4]); // maximal
    CPPUNIT_ASSERT(!result[5]);
  }

  void testOverlapItinerary()
  {
  }

private:
  std::vector<type::TaxApplicationLimit> _limits;
  std::vector<type::MoneyAmount> _amounts;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxLimiterTest);
}

