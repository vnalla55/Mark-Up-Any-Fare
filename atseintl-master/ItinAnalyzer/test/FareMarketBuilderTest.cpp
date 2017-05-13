#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "test/include/MockTseServer.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/ItinIndex.h"
#include "Common/ShoppingUtil.h"
#include "Common/Global.h"
#include "Common/Hasher.h"
#include "DBAccess/DataHandle.h"
#include "ItinAnalyzer/FareMarketBuilder.h"
#include "DataModel/ExchangePricingTrx.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <vector>

namespace tse
{
class FareMarketBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareMarketBuilderTest);
  CPPUNIT_TEST(testNoCxrOverride);
  CPPUNIT_TEST(testGovCxrMatchCxrOverride);
  CPPUNIT_TEST(testGovCxrNotMatchCxrOverride);
  CPPUNIT_TEST(testMultipleCxrOverrides);
  CPPUNIT_TEST(testGovCxrMatchYYOverride);
  CPPUNIT_TEST(testGovCxrNoMatchYYOverride);
  CPPUNIT_TEST(testCxrOverrideNotParticipatingCxr);
  CPPUNIT_TEST(testSetFareCalcAmtForDummyFareFM);
  CPPUNIT_TEST(testSetBreakIndicatorForPbbReqNoBrands);
  CPPUNIT_TEST(testSetBreakIndicatorForPbbReqBrandsAndNoBrandsMixed);
  CPPUNIT_TEST(testSetBreakIndicatorForPbbReqAllSegWithSameBrand);
  CPPUNIT_TEST(testSetBreakIndicatorForPbbReqMixedBrands);
  CPPUNIT_TEST_SUITE_END();

public:
  void testNoCxrOverride()
  {
    CarrierCode aa = "AA";
    CarrierCode af = "AF";

    std::vector<CarrierCode> govCxrOverrides;

    std::set<CarrierCode> participatingCarriers;
    participatingCarriers.insert(aa);
    participatingCarriers.insert(af);

    bool yyOverride = false;

    FareMarket fm;
    fm.governingCarrier() = af;
    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == false);
  }

  void testSetBreakIndicatorForPbbReqNoBrands()
  {
    setUpForPbb();
    FareMarketBuilder::setBreakIndicator(_fm, _itin, *_trx);
    CPPUNIT_ASSERT(_fm->breakIndicator() == false);
  }

  void testSetBreakIndicatorForPbbReqBrandsAndNoBrandsMixed()
  {
    setUpForPbb();
    _fm->travelSeg()[1]->setBrandCode("FL");
    FareMarketBuilder::setBreakIndicator(_fm, _itin, *_trx);
    CPPUNIT_ASSERT(_fm->breakIndicator() == true);
  }

  void testSetBreakIndicatorForPbbReqAllSegWithSameBrand()
  {
    setUpForPbb();
    std::vector<TravelSeg*>::iterator it = _fm->travelSeg().begin();

    for ( ; it != _fm->travelSeg().end(); ++it)
      (*it)->setBrandCode("FL");

    FareMarketBuilder::setBreakIndicator(_fm, _itin, *_trx);
    CPPUNIT_ASSERT(_fm->breakIndicator() == false);
  }

  void testSetBreakIndicatorForPbbReqMixedBrands()
  {
    setUpForPbb();
    std::vector<TravelSeg*>::iterator it = _fm->travelSeg().begin();
    (*it)->setBrandCode("BC");

    for (++it; it != _fm->travelSeg().end(); ++it)
      (*it)->setBrandCode("FL");

    FareMarketBuilder::setBreakIndicator(_fm, _itin, *_trx);
    CPPUNIT_ASSERT(_fm->breakIndicator() == true);
  }

  void testGovCxrMatchCxrOverride()
  {
    CarrierCode aa = "AA";
    CarrierCode af = "AF";

    std::vector<CarrierCode> govCxrOverrides;
    govCxrOverrides.push_back(af);

    std::set<CarrierCode> participatingCarriers;
    participatingCarriers.insert(aa);
    participatingCarriers.insert(af);

    bool yyOverride = false;

    FareMarket fm;

    fm.governingCarrier() = af;
    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == false);
  }

  void testGovCxrNotMatchCxrOverride()
  {
    CarrierCode aa = "AA";
    CarrierCode af = "AF";

    std::vector<CarrierCode> govCxrOverrides;
    govCxrOverrides.push_back(af);

    std::set<CarrierCode> participatingCarriers;
    participatingCarriers.insert(aa);
    participatingCarriers.insert(af);

    bool yyOverride = false;

    FareMarket fm;

    fm.governingCarrier() = aa;
    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == true);
  }

  void testMultipleCxrOverrides()
  {
    CarrierCode aa = "AA";
    CarrierCode af = "AF";

    std::vector<CarrierCode> govCxrOverrides;
    govCxrOverrides.push_back(aa);
    govCxrOverrides.push_back(af);

    std::set<CarrierCode> participatingCarriers;
    bool yyOverride = false;

    FareMarket fm;
    fm.governingCarrier() = aa;

    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == true);
  }

  void testGovCxrMatchYYOverride()
  {
    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    bool yyOverride = true;

    FareMarket fm;
    fm.governingCarrier() = INDUSTRY_CARRIER;

    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == false);
  }

  void testGovCxrNoMatchYYOverride()
  {
    CarrierCode aa = "AA";

    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    bool yyOverride = true;

    FareMarket fm;
    fm.governingCarrier() = aa;

    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == true);
  }

  void testCxrOverrideNotParticipatingCxr()
  {
    CarrierCode aa = "AA";
    CarrierCode af = "AF";
    CarrierCode jl = "JL";

    std::vector<CarrierCode> govCxrOverrides;
    govCxrOverrides.push_back(jl);

    std::set<CarrierCode> participatingCarriers;
    participatingCarriers.insert(aa);
    participatingCarriers.insert(af);

    bool yyOverride = false;

    FareMarket fm;

    fm.governingCarrier() = af;
    FareMarketBuilder::setBreakIndByCxrOverride(
        govCxrOverrides, participatingCarriers, fm, yyOverride);

    CPPUNIT_ASSERT(fm.breakIndicator() == false);
  }

  void testSetFareCalcAmtForDummyFareFM()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    FareMarket fm1, fm12, fm13, fm2, fm23, fm3;

    fm1.travelSeg().push_back(&tvlSeg1);
    fm12.travelSeg().push_back(&tvlSeg1);
    fm12.travelSeg().push_back(&tvlSeg2);
    fm13.travelSeg().push_back(&tvlSeg1);
    fm13.travelSeg().push_back(&tvlSeg2);
    fm13.travelSeg().push_back(&tvlSeg3);
    fm2.travelSeg().push_back(&tvlSeg2);
    fm23.travelSeg().push_back(&tvlSeg2);
    fm23.travelSeg().push_back(&tvlSeg3);
    fm3.travelSeg().push_back(&tvlSeg3);

    Itin itin;

    itin.fareMarket().push_back(&fm1);
    itin.fareMarket().push_back(&fm12);
    itin.fareMarket().push_back(&fm13);
    itin.fareMarket().push_back(&fm2);
    itin.fareMarket().push_back(&fm23);
    itin.fareMarket().push_back(&fm3);

    itin.travelSeg().push_back(&tvlSeg1);
    itin.travelSeg().push_back(&tvlSeg2);
    itin.travelSeg().push_back(&tvlSeg3);

    PricingTrx regularTrx;
    PricingOptions* options = _memHandle.create<PricingOptions>();
    regularTrx.setOptions(options);

    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(regularTrx, &itin, fm1));

    ExchangePricingTrx excTrx;
    excTrx.setOptions(options);

    excTrx.exchangeOverrides().dummyFCSegs().clear();
    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm1));

    uint16_t dummyFCNum = 1;
    excTrx.exchangeOverrides().dummyFCSegs()[&tvlSeg1] = dummyFCNum;
    excTrx.exchangeOverrides().dummyFCSegs()[&tvlSeg2] = dummyFCNum;
    tvlSeg2.fareBasisCode() = "DUMMY";
    tvlSeg2.fareCalcFareAmt() = "123.45";

    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm1));
    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm2));
    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm3));
    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm13));
    CPPUNIT_ASSERT_EQUAL(false,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm23));
    CPPUNIT_ASSERT_EQUAL(true,
                         FareMarketBuilder::setFareCalcAmtForDummyFareFM(excTrx, &itin, fm12));
    CPPUNIT_ASSERT_EQUAL(fm12.fareCalcFareAmt(), tvlSeg2.fareCalcFareAmt());
  }

  void setUpForPbb()
  {
    _trx = _memHandle(new PricingTrx);
    _fm = _memHandle(new FareMarket);
    _itin = _memHandle(new Itin);
    _trx->setOptions(_memHandle(new PricingOptions));
    _trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);

    for (int idx=0; idx<3; ++idx)
      _fm->travelSeg().push_back(_memHandle(new AirSeg));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

private:
  TestMemHandle _memHandle;
  FareMarket*   _fm;
  PricingTrx*   _trx;
  Itin*         _itin;

};
CPPUNIT_TEST_SUITE_REGISTRATION(FareMarketBuilderTest);
} // tse
