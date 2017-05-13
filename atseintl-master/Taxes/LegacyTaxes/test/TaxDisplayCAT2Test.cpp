#include "Taxes/LegacyTaxes/Category2.h"
#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tx_test
{
template <>
std::string
TaxDisplayTestBuilder<tse::Category2>::expectedDisplay(IORecord& ior) const
{
  return ("*" + std::get<1>(ior) + " TAX" + std::get<2>(ior) + std::get<3>(ior) + "\n");
}
} // tx_test

namespace tse
{

class TaxDisplayCAT2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT2Test);
  CPPUNIT_TEST(testBuildCAT2);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildCAT2()
  {
    using namespace tx_test;

    IOContainer io;
    TaxDisplayTestBuilder<Category2> test;

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_01.xml", " MINIMUM", " USD", " 10.00"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_04.xml", " MINIMUM", " USD", " 12.37"));
    test.execute(io, &Category2::subCat1);
    io.clear();

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_01.xml", " MAXIMUM", " USD", " 11.00"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_02.xml", " MAXIMUM", " USD", " 44.55"));
    test.execute(io, &Category2::subCat2);
    io.clear();

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_02.xml", " PLUS UP", " USD", " 31.25"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat2_03.xml", " PLUS UP", " USD", " 1.50"));
    test.execute(io, &Category2::subCat3);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT2Test);
}
