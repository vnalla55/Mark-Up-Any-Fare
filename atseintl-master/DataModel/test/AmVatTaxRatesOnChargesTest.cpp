#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AmVatTaxRatesOnCharges.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class AmVatTaxRatesOnChargesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AmVatTaxRatesOnChargesTest);
  CPPUNIT_TEST(loadAcmsDataTest);
  CPPUNIT_TEST(getCorrectRateTest);
  CPPUNIT_TEST(getIncorrectRateTest);
  CPPUNIT_TEST(getNotExistingRateTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    std::string amChargesTaxesData = "MX/IVA/16|BO/BOA/13|PY/PYA/10|PE/PEA/1Z8|CO/YSA/16|GU/XBA/12|HN/HNA/12|NI/NIA/15|VE/YNA/12";
    _amVatTaxRatesOnCharges =  new AmVatTaxRatesOnCharges(amChargesTaxesData);
  }

  void tearDown()
  {
    delete _amVatTaxRatesOnCharges;
  }

  void loadAcmsDataTest()
  {
    CPPUNIT_ASSERT(_amVatTaxRatesOnCharges->getData().size() == 8);
  }

  void getCorrectRateTest()
  {
    const AmVatTaxRatesOnCharges::AmVatTaxRate* rate = _amVatTaxRatesOnCharges->getAmVatTaxRate("GU");

    CPPUNIT_ASSERT(rate != 0);
    CPPUNIT_ASSERT(rate->getTaxCode() == "XBA");
    CPPUNIT_ASSERT(rate->getTaxRate() == 12);
  }

  void getIncorrectRateTest()
  {
    CPPUNIT_ASSERT(_amVatTaxRatesOnCharges->getAmVatTaxRate("PE") == 0);
  }

  void getNotExistingRateTest()
  {
    CPPUNIT_ASSERT(_amVatTaxRatesOnCharges->getAmVatTaxRate("PL") == 0);
  }

private:
  TestMemHandle _memHandle;
  AmVatTaxRatesOnCharges* _amVatTaxRatesOnCharges;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AmVatTaxRatesOnChargesTest);
}
