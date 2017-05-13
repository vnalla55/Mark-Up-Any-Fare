// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class ConvertCodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ConvertCodeTest);
  CPPUNIT_TEST(toTseVendor);
  CPPUNIT_TEST_SUITE_END();

public:
  void toTseVendor()
  {
    tax::type::Vendor taxCode("ABCD");
    tse::VendorCode ans = toTseVendorCode(taxCode);
    CPPUNIT_ASSERT_EQUAL(ans, tse::VendorCode("ABCD"));

    taxCode = "X";
    ans = toTseVendorCode(taxCode);
    CPPUNIT_ASSERT_EQUAL(ans, tse::VendorCode("X"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvertCodeTest);

}
