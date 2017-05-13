#include "Taxes/LegacyTaxes/Category6.h"
#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <string>

namespace tx_test
{
template <>
std::string
TaxDisplayTestBuilder<tse::Category6>::expectedDisplay(IORecord& ior) const
{
  if (std::get<1>(ior) != "")
    return (std::get<1>(ior) + ".\n");

  return "";
}
} // tx_test

namespace tse
{
class TaxDisplayCAT6Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT6Test);
  CPPUNIT_TEST(testBuildCAT6);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testBuildCAT6()
  {
    using namespace tx_test;
    tx_test::IOContainer io;

    // XML file with data, group test,
    // group 2
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat02_01.xml",
                               "* EXCEPT ON TICKETS SOLD IN CURRENCY ANG"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat02_02.xml",
                               "* ON TICKETS SOLD IN CURRENCY ANG"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat02_03.xml",
                               "* EXCEPT ON TICKETS SOLD IN CURRENCY USD"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat02_04.xml",
                               "* ON TICKETS SOLD IN CURRENCY USD"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat6_cat02_05.xml","NO CURRENCY RESTRICTIONS
    // APPLY"));

    tx_test::TaxDisplayTestBuilder<Category6> test;
    test.execute(io, &Category6::subCat2);
    io.clear();

    // group 1
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat01_01.xml",
                               "* TAX APPLIES ON EQUIVALENT AMOUNT PAID"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat01_02.xml", ""));
    //  TaxDisplayTestBuilder<Category6> test1;
    test.execute(io, &Category6::subCat1);
    io.clear();

    // form of payment

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat03_01.xml",
                               "* ON TICKETS PAID IN CASH"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat03_02.xml",
                               "* ON TICKETS PAID BY CHECK"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat6_cat03_03.xml",
                               "* ON TICKETS PAID BY CREDIT CARD"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat6_cat03_04.xml",""));

    test.execute(io, &Category6::subCat3);
    io.clear();

    tx_test::TaxDisplayTestBuilder<Category6> test4;

    TaxCodeReg* taxCodeReg1 = _memHandle.create<TaxCodeReg>();
    taxCodeReg1->taxcdRoundRule() = UP;
    taxCodeReg1->taxcdRoundUnitInt().nodec() = 4;
    taxCodeReg1->taxcdRoundUnitInt().amt() = 10;

    TaxCodeReg* taxCodeReg2 = _memHandle.create<TaxCodeReg>();
    taxCodeReg2->taxcdRoundRule() = DOWN;
    taxCodeReg2->taxcdRoundUnitInt().nodec() = 0;
    taxCodeReg2->taxcdRoundUnitInt().amt() = 100;

    TaxCodeReg* taxCodeReg3 = _memHandle.create<TaxCodeReg>();
    taxCodeReg3->taxcdRoundRule() = UP;
    taxCodeReg3->taxcdRoundUnitInt().nodec() = 4;
    taxCodeReg3->taxcdRoundUnitInt().amt() = 100000;

    TaxCodeReg* taxCodeReg4 = _memHandle.create<TaxCodeReg>();

    io.push_back(buildIORecord(taxCodeReg1,
                               "* TAX CODE ROUNDING OVERRIDE APPLIES AS ROUND UP TO NEXT 0.001 "));
    io.push_back(buildIORecord(taxCodeReg2,
                               "* TAX CODE ROUNDING OVERRIDE APPLIES AS ROUND DOWN TO 100 "));
    io.push_back(buildIORecord(taxCodeReg3,
                               "* TAX CODE ROUNDING OVERRIDE APPLIES AS ROUND UP TO NEXT 10 "));
    io.push_back(buildIORecord(taxCodeReg4, ""));
    test4.execute(io, &Category6::subCat4);
    io.clear();
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT6Test);
}
