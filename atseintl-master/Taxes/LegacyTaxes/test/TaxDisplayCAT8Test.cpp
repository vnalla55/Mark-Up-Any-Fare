#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "Taxes/LegacyTaxes/Category8.h"

#include "test/include/CppUnitHelperMacros.h"

#include <memory>

using namespace tx_test;

namespace tse
{
class TaxDisplayCAT8Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT8Test);
  CPPUNIT_TEST(catInstanceTest);
  CPPUNIT_TEST(taxDisplayTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void catInstanceTest()
  {
    std::unique_ptr<Category8> ptr(new Category8);
    CPPUNIT_ASSERT(ptr);
  }

  void taxDisplayTest()
  {
    IOContainer io;
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat8_01.xml",
                               "* ON CARRIER EQUIPMENT AT7, BEH, CVR, DH3, SWM.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat8_02.xml",
                               "* EXCEPT ON CARRIER EQUIPMENT AT7, BE9, BEB, BEC, BH2.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat8_03.xml",
                               "* ON CARRIER EQUIPMENT AT7, BEH, CVR, DH3, SWM.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat8_04.xml",
                               "     NO EQUIPMENT RESTRICTIONS APPLY.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat8_05.xml", "* ON CARRIER EQUIPMENT AT7.\n"));

    TaxDisplayTestBuilder<Category8> test;
    test.execute(io);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT8Test);
}
