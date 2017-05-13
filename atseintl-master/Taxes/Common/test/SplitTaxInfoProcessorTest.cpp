
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcTaxInfo.h"
#include "Taxes/Common/SplitTaxInfoProcessor.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <memory>

namespace tse
{

class SplitTaxInfoProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SplitTaxInfoProcessorTest);
  CPPUNIT_TEST(testGetFlatTaxItem_roundTo1DecimalDown);
  CPPUNIT_TEST(testGetFlatTaxItem_roundTo1DecimalUp);
  CPPUNIT_TEST(testGetFlatTaxItem_roundTo2Decimals);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fcTaxInfo.reset(new FareCalc::FcTaxInfo());
    _calcTotals.reset(new CalcTotals());
    _trx.reset(new PricingTrx());
    _taxResponse.reset(new TaxResponse());
    _fcConfig.reset(new FareCalcConfig());
    _taxCurrencyCode.reset(new CurrencyCode());

    _farePath.reset(new FarePath());
    _calcTotals->farePath = _farePath.get();

    _itin.reset(new Itin());
    _farePath->itin() = _itin.get();

    _splitTaxInfoProcessor.reset(
        new SplitTaxInfoProcessor(*_fcTaxInfo, *_calcTotals, *_trx, _taxResponse.get(), _fcConfig.get(), *_taxCurrencyCode));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testGetFlatTaxItem_roundTo1DecimalDown()
  {
    _splitTaxInfoProcessor->_fareComponentCount = 3;

    TaxItem item;
    item.taxAmount() = 10;
    item.taxNodec() = 1;

    TaxItem* newItem = 0;
    newItem = _splitTaxInfoProcessor->getFlatTaxItem(&item);

    CPPUNIT_ASSERT_EQUAL(3.3, newItem->taxAmount());
  }

  void testGetFlatTaxItem_roundTo1DecimalUp()
  {
    _splitTaxInfoProcessor->_fareComponentCount = 3;

    TaxItem item;
    item.taxAmount() = 20;
    item.taxNodec() = 1;

    TaxItem* newItem = 0;
    newItem = _splitTaxInfoProcessor->getFlatTaxItem(&item);

    CPPUNIT_ASSERT_EQUAL(6.7, newItem->taxAmount());
  }

  void testGetFlatTaxItem_roundTo2Decimals()
  {
    _splitTaxInfoProcessor->_fareComponentCount = 3;

    TaxItem item;
    item.taxAmount() = 10;
    item.taxNodec() = 2;

    TaxItem* newItem = 0;
    newItem = _splitTaxInfoProcessor->getFlatTaxItem(&item);

    CPPUNIT_ASSERT_EQUAL(3.33, newItem->taxAmount());
  }

private:
  TestMemHandle _memHandle;
  std::unique_ptr<FareCalc::FcTaxInfo> _fcTaxInfo;
  std::unique_ptr<CalcTotals> _calcTotals;
  std::unique_ptr<PricingTrx> _trx;
  std::unique_ptr<TaxResponse> _taxResponse;
  std::unique_ptr<FareCalcConfig> _fcConfig;
  std::unique_ptr<CurrencyCode> _taxCurrencyCode;
  std::unique_ptr<FarePath> _farePath;
  std::unique_ptr<Itin> _itin;
  std::unique_ptr<SplitTaxInfoProcessor> _splitTaxInfoProcessor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SplitTaxInfoProcessorTest);
};
