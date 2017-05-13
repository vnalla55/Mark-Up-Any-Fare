// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/Common/CommonEntries.h"
#include "test/include/CppUnitHelperMacros.h"


namespace tax
{
namespace display
{

class CommonEntriesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommonEntriesTest);
  CPPUNIT_TEST(testErrorNoData);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void testErrorNoData()
  {
    TaxDisplayRequest request;
    std::string ret;
    request.taxCode = "TC";
    ret = CommonEntries::getErrorNoData(request);
    CPPUNIT_ASSERT_EQUAL(std::string("NO TAX CODE DATA EXISTS"), ret);

    request.taxType = "001";
    ret = CommonEntries::getErrorNoData(request);
    CPPUNIT_ASSERT_EQUAL(std::string("NO TAX CODE/TAX TYPE DATA EXISTS"), ret);

    request.carrierCode1 = "CC";
    ret = CommonEntries::getErrorNoData(request);
    CPPUNIT_ASSERT_EQUAL(std::string("NO TAX CODE/TAX TYPE/CXR CARRIER CODE DATA EXISTS"), ret);

    request.taxType = type::TaxType();
    ret = CommonEntries::getErrorNoData(request);
    CPPUNIT_ASSERT_EQUAL(std::string("NO TAX CODE/CXR CARRIER CODE DATA EXISTS"), ret);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CommonEntriesTest);
} /* namespace display */
} /* namespace tax */
