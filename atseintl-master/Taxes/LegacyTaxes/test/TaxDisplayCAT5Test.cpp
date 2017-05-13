// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/Category5.h"
#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "test/include/CppUnitHelperMacros.h"


#include <memory>

namespace tx_test
{
template <>
std::string
TaxDisplayTestBuilder<tse::Category5>::expectedDisplay(IORecord& ior) const
{
  return ("*" + std::get<1>(ior) + " TICKETS VALIDATED ON CARRIER/S " + std::get<2>(ior) + ".\n");
}
} // tx_test

namespace tse
{
class TaxDisplayCAT5Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT5Test);
  CPPUNIT_TEST(catInstanceTest);
  CPPUNIT_TEST(taxDisplayTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void catInstanceTest()
  {
    std::unique_ptr<Category5> ptr(new Category5);
    CPPUNIT_ASSERT(ptr);
  }

  void taxDisplayTest()
  {
    using namespace tx_test;
    IOContainer io;

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat5_01.xml", " EXCEPT", "AA, DL, UA"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat5_02.xml", "", "AA, DL, UA"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat5_03.xml", "", "AA"));

    tx_test::TaxDisplayTestBuilder<Category5> test;
    test.execute(io);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT5Test);
}
