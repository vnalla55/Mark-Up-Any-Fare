#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/ElementField.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ElementFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ElementFilterTest);

  CPPUNIT_TEST(testNetFareIndicatorWhenAdjustedSellingFare);
  CPPUNIT_TEST(testNetFareIndicatorWhenAdjustedSellingFareButXRS);
  CPPUNIT_TEST(testNetFareIndicatorWhenOriginalSellingFare);
  CPPUNIT_TEST(testNetFareIndicatorWhenNetFare);
  CPPUNIT_TEST(testNetFareIndicatorWhenRedistributedFare);
  CPPUNIT_TEST(testNetFareNONCOCWhenAdjustedSellingFare);
  CPPUNIT_TEST(testNetFareNONCOCWhenAdjustedSellingFareButXRS);
  CPPUNIT_TEST(testNetFareNONCOCWhenOriginalSellingFare);
  CPPUNIT_TEST(testNetFareNONCOCWhenNetFare);
  CPPUNIT_TEST(testNetFareNONCOCWhenRedistributedFare);
  CPPUNIT_TEST(testNetFareNONCOCWhenAsterisk);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _options = _memHandle.create<FareDisplayOptions>();
    _request = _memHandle.create<FareDisplayRequest>();
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* fcri = _memHandle.create<TariffCrossRefInfo>();
    fare->setFareInfo(_fareInfo);
    fare->setTariffCrossRefInfo(fcri);
    _ptf->setFare(fare);
    _field = _memHandle.create<ElementField>();
    _displayCurrency = _memHandle.create<CurrencyCode>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testNetFareIndicatorWhenAdjustedSellingFare()
  {
    AdjustedSellingCalcData asd;
    _ptf->setAdjustedSellingCalcData(&asd);
    _options->setXRSForFRRule(false);

    ElementFilter::netFareIndicator(*_field, *_ptf, *_trx);

    CPPUNIT_ASSERT_EQUAL(std::string(SELLING_QUALIFIER), _field->strValue());
  }

  void testNetFareIndicatorWhenAdjustedSellingFareButXRS()
  {
    AdjustedSellingCalcData asd;
    _ptf->setAdjustedSellingCalcData(&asd);
    _options->setXRSForFRRule(true);
    _request->inclusionCode() = FD_NET;
    _ptf->fareDisplayCat35Type() = RuleConst::NET_FARE;

    ElementFilter::netFareIndicator(*_field, *_ptf, *_trx);

    CPPUNIT_ASSERT_EQUAL(std::string(NET_QUALIFIER), _field->strValue());
  }

  void testNetFareIndicatorWhenOriginalSellingFare()
  {
    _options->setPDOForFRRule(true);
    _ptf->setAdjustedSellingBaseFare();

    ElementFilter::netFareIndicator(*_field, *_ptf, *_trx);

    CPPUNIT_ASSERT_EQUAL(std::string(ORIGINAL_SELLING_QUALIFIER), _field->strValue());
  }

  void testNetFareIndicatorWhenNetFare()
  {
    _request->inclusionCode() = FD_NET;
    _ptf->fareDisplayCat35Type() = RuleConst::NET_FARE;

    ElementFilter::netFareIndicator(*_field, *_ptf, *_trx);

    CPPUNIT_ASSERT_EQUAL(std::string(NET_QUALIFIER), _field->strValue());
  }

  void testNetFareIndicatorWhenRedistributedFare()
  {
    _request->inclusionCode() = FD_NET;
    _ptf->fareDisplayCat35Type() = RuleConst::REDISTRIBUTED_FARE;

    ElementFilter::netFareIndicator(*_field, *_ptf, *_trx);

    CPPUNIT_ASSERT_EQUAL(std::string(REDISTRIBUTE_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenAdjustedSellingFare()
  {
    AdjustedSellingCalcData asd;
    _ptf->setAdjustedSellingCalcData(&asd);
    _options->setXRSForFRRule(false);

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(SELLING_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenAdjustedSellingFareButXRS()
  {
    AdjustedSellingCalcData asd;
    _ptf->setAdjustedSellingCalcData(&asd);
    _options->setXRSForFRRule(true);
    _ptf->fareDisplayCat35Type() = RuleConst::NET_FARE;

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(NET_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenOriginalSellingFare()
  {
    _options->setPDOForFRRule(true);
    _ptf->setAdjustedSellingBaseFare();

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(ORIGINAL_SELLING_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenNetFare()
  {
    _ptf->fareDisplayCat35Type() = RuleConst::NET_FARE;

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(NET_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenRedistributedFare()
  {
    _ptf->fareDisplayCat35Type() = RuleConst::REDISTRIBUTED_FARE;

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(REDISTRIBUTE_QUALIFIER), _field->strValue());
  }

  void testNetFareNONCOCWhenAsterisk()
  {
    *_displayCurrency = "USD";
    _fareInfo->currency() = "GBP";

    ElementFilter::netFareNONCOC(*_field, *_ptf, *_trx, *_displayCurrency);

    CPPUNIT_ASSERT_EQUAL(std::string(ASTERISK), _field->strValue());
  }

private:
  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  PaxTypeFare* _ptf;
  ElementField* _field;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  CurrencyCode* _displayCurrency;
  FareInfo* _fareInfo;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ElementFilterTest);
}
