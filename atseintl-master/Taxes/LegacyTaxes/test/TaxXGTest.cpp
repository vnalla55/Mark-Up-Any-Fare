#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxXG.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TaxXGMock : public TaxXG
{
public:
  void setZeroesBaseFare(bool value)
  {
    _zeroesBaseFare = value;
  }
};

class TaxXGTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxXGTest);
  CPPUNIT_TEST(taxOnTaxTest);
  CPPUNIT_TEST_SUITE_END();

private:
  TaxItem* createTaxItem(const TaxCode& code, MoneyAmount money, bool serviceFee = false)
  {
    TaxItem* taxItem = _memHandle.create<TaxItem>();
    taxItem->taxCode() = code;
    taxItem->taxAmount() = money;
    taxItem->serviceFee() = serviceFee;
    return taxItem;
  }

private:
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void taxOnTaxTest()
  {
    const std::string XG("XG");
    const std::string CA1("CA1");
    const std::string SQ("SQ");
    const std::string SQ1("SQ1");
    const std::string SQ3("SQ3");
    const std::string YQR("YQR");
    const std::string YQI("YQI");

    PricingTrx trx;
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = XG;
    taxCodeReg.taxOnTaxCode().push_back(CA1);
    taxCodeReg.taxOnTaxCode().push_back(SQ);
    taxCodeReg.taxOnTaxCode().push_back(SQ1);
    taxCodeReg.taxOnTaxCode().push_back(SQ3);
    taxCodeReg.taxOnTaxCode().push_back(YQR);
    taxCodeReg.taxOnTaxCode().push_back(YQI);

    const NationCode nation("CA");
    Itin itin;
    FarePath farePath;
    farePath.getMutableExternalTaxes().push_back(createTaxItem(YQI, 5.0, true));
    farePath.getMutableExternalTaxes().push_back(createTaxItem(YQR, 15.0, true));
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.taxItemVector().push_back(createTaxItem("AR", 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(CA1, 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(SQ, 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(SQ1, 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(YQI, 2.0, true /*serviceFee*/));
    taxResponse.taxItemVector().push_back(createTaxItem(YQR, 3.0, true /*serviceFee*/));

    {// without external taxes
      TaxXGMock taxXg;
      taxXg.setTaxableFare(1.0);
      taxXg.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(9.0, taxXg.taxableFare());
      CPPUNIT_ASSERT_EQUAL(3.0, taxXg.calculationDetails().taxableTaxSumAmount);
    }

    {// without external taxes, zeroesBaseFare = true
      TaxXGMock taxXg;
      taxXg.setTaxableFare(1.0);
      taxXg.setZeroesBaseFare(true);
      taxXg.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(2.0, taxXg.taxableFare());
      CPPUNIT_ASSERT_EQUAL(2.0, taxXg.calculationDetails().taxableTaxSumAmount);
      CPPUNIT_ASSERT(!taxXg.applyFeeOnTax());
    }

    {// external taxes, not shopping tax request
      TaxXGMock taxXg;
      taxXg.setTaxableFare(1.0);
      taxResponse.farePath() = &farePath;
      taxXg.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(9.0, taxXg.taxableFare());
      CPPUNIT_ASSERT_EQUAL(3.0, taxXg.calculationDetails().taxableTaxSumAmount);
    }

    {// external taxes, shopping tax request
      TaxTrx taxTrx;
      TaxXGMock taxXg;
      taxXg.setTaxableFare(1.0);
      taxTrx.setShoppingPath(true);
      taxResponse.farePath() = &farePath;
      taxXg.applyTaxOnTax(taxTrx, taxResponse, taxCodeReg);
      CPPUNIT_ASSERT_EQUAL(29.0, taxXg.taxableFare());
      CPPUNIT_ASSERT_EQUAL(3.0, taxXg.calculationDetails().taxableTaxSumAmount);
    }

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxXGTest);
};
