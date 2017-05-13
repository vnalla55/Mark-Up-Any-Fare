#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRC.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TaxRCMock : public TaxRC
{
public:
  void setZeroesBaseFare(bool value)
  {
    _zeroesBaseFare = value;
  }
  void setSpecialTaxRCInd(char val)
  {
    _specialTaxRCInd = val;
  }
};

class TaxRCTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRCTest);
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
    const std::string RC("RC");
    const std::string CA1("CA1");
    const std::string SQ2("SQ2");
    const std::string YQR("YQR");
    const std::string YQI("YQI");

    PricingTrx trx;
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = RC;
    const NationCode nation("CA");
    Itin itin;
    FarePath farePath;
    farePath.getMutableExternalTaxes().push_back(createTaxItem(YQR, 5.0, true));
    farePath.getMutableExternalTaxes().push_back(createTaxItem(YQI, 15.0, true));
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.taxItemVector().push_back(createTaxItem("AR", 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(YQI, 2.0, true));
    taxResponse.taxItemVector().push_back(createTaxItem(YQR, 3.0, true));

    {// without CA1, SQ2
      TaxRCMock taxRC;
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(1.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(6.0, taxRC.calculationDetails().taxableTaxSumAmount);
      CPPUNIT_ASSERT(!taxRC.mixedTax());
    }

    taxResponse.taxItemVector().push_back(createTaxItem(CA1, 1.0));
    taxResponse.taxItemVector().push_back(createTaxItem(SQ2, 2.0));

    {// with CA1, SQ2, without external taxes
      TaxRCMock taxRC;
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(4.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(9.0, taxRC.calculationDetails().taxableTaxSumAmount);
      CPPUNIT_ASSERT(taxRC.mixedTax());
    }

    {// zeroesBaseFare = true
      TaxRCMock taxRC;
      taxRC.setZeroesBaseFare(true);
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      //exclude CA1
      CPPUNIT_ASSERT_EQUAL(2.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(8.0, taxRC.calculationDetails().taxableTaxSumAmount);
      CPPUNIT_ASSERT(taxRC.mixedTax());
    }

    {// specialTaxRCInd = true
      TaxRCMock taxRC;
      taxRC.setSpecialTaxRCInd(1);
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      //exclude CA1
      CPPUNIT_ASSERT_EQUAL(2.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(8.0, taxRC.calculationDetails().taxableTaxSumAmount);
      CPPUNIT_ASSERT(taxRC.mixedTax());
    }


    {// external taxes, not shopping tax request
      TaxRCMock taxRC;
      taxResponse.farePath() = &farePath;
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(trx, taxResponse, taxCodeReg);

      CPPUNIT_ASSERT_EQUAL(4.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(9.0, taxRC.calculationDetails().taxableTaxSumAmount);
    }

    {// external taxes, shopping tax request
      TaxTrx taxTrx;
      TaxRCMock taxRC;
      taxTrx.setShoppingPath(true);
      taxResponse.farePath() = &farePath;
      taxRC.setTaxableFare(1.0);
      taxRC.applyTaxOnTax(taxTrx, taxResponse, taxCodeReg);
      CPPUNIT_ASSERT_EQUAL(4.0, taxRC.taxableFare());
      CPPUNIT_ASSERT_EQUAL(29.0, taxRC.calculationDetails().taxableTaxSumAmount);
    }

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRCTest);
};
