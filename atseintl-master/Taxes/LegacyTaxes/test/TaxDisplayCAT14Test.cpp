#include "Taxes/LegacyTaxes/Category14.h"
#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"

#include "test/include/CppUnitHelperMacros.h"

#include <memory>

using namespace tx_test;

namespace tse
{
class TaxDisplayCAT14Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT14Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreation()
  {
    std::unique_ptr<Category14> ptr(new Category14);

    CPPUNIT_ASSERT(ptr);
  }

  void testDisplay()
  {
    IOContainer io;
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat14_01.xml",
                               "     NO DISCOUNT RESTRICTIONS APPLY.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat14_02.xml",
                               "* INDUSDTRY TRAVEL TICKETS ARE EXEMPT.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat14_03.xml", "* FREE TICKET IS EXEMPT.\n"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat14_04.xml",
                               "* FREE TICKET IS EXEMPT.\n"
                               "* INDUSDTRY TRAVEL TICKETS ARE EXEMPT.\n"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_05.xml","* TICKETS DISCOUNTED 90 PERCENT
    // OR MORE ARE EXEMPT.\n"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_06.xml","* TICKETS DISCOUNTED 90 PERCENT
    // OR MORE ARE EXEMPT.\n"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_07.xml","* TICKETS DISCOUNTED 90 PERCENT
    // OR MORE ARE EXEMPT.\n"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_08.xml","* TICKETS DISCOUNTED 90 PERCENT
    // OR MORE ARE EXEMPT.\n"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_09.xml","* TICKETS DISCOUNTED 90 PERCENT
    // OR MORE ARE EXEMPT.\n"));
    /*io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_10.xml","* TICKETS DISCOUNTED 75.55 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_11.xml","* TICKETS DISCOUNTED 75.55 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_12.xml","* TICKETS DISCOUNTED 75.55 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_13.xml","* TICKETS DISCOUNTED 75.552 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_14.xml","* TICKETS DISCOUNTED 75.6 PERCENT OR
    MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_15.xml","* TICKETS DISCOUNTED 75.5522 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_16.xml","* TICKETS DISCOUNTED 75.55 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_17.xml","* TICKETS DISCOUNTED 75.56 PERCENT
    OR MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_18.xml","* TICKETS DISCOUNTED 100 PERCENT OR
    MORE ARE EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_19.xml","* TICKETS DISCOUNTED 70 PERCENT OR
    MORE ARE EXEMPT.\n"
                                                               "* INDUSDTRY TRAVEL TICKETS ARE
    EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_20.xml","* TICKETS DISCOUNTED 75 PERCENT OR
    MORE ARE EXEMPT.\n"
                                                               "* FREE TICKET IS EXEMPT.\n"));
    io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat14_21.xml","* TICKETS DISCOUNTED 90.2 PERCENT OR
    MORE ARE EXEMPT.\n"
                                                               "* FREE TICKET IS EXEMPT.\n"
                                                               "* INDUSDTRY TRAVEL TICKETS ARE
    EXEMPT.\n")); */
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat14_22.xml",
                               "* FREE TICKET IS EXEMPT.\n"
                               "* INDUSDTRY TRAVEL TICKETS ARE EXEMPT.\n"));

    TaxDisplayTestBuilder<Category14> test;
    test.execute(io);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT14Test);
}
