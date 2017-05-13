#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/SurchargesRule.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

using namespace boost::assign;

class SurchargesRuleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SurchargesRuleTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testIsDataCorrect);
  CPPUNIT_TEST(testCheckDateRange);
  CPPUNIT_TEST(testCheckDOWandTime);
  CPPUNIT_TEST(testAddSurcharge);
  CPPUNIT_TEST(testIsSideTrip);
  CPPUNIT_TEST(testIsSideTripBtwAnd);

  CPPUNIT_TEST(testAddSurchargeShouldSetCurrencyUSDAmountFiftyForOWFareMayBeDoubled);
  CPPUNIT_TEST(testAddSurchargeShouldSetCurrencyUSDAmountHundredForRoundTripMayNotBeHalved);
  CPPUNIT_TEST(testAddSurchargeShouldSetCurrencyUSDAmountFiftyForOWFareMayNotBeDoubled);

  CPPUNIT_TEST(testAddSurchargeShouldSetSurchargeAmountWhenIsNotPercent);
  CPPUNIT_TEST(testAddSurchargeForDiscountedFareWhenIsNotPercentAndIsNotCalculated);
  CPPUNIT_TEST(testAddSurchargeForDiscountedFareWhenIsNotPercentAndIsCalculated);
  CPPUNIT_TEST(testAddSurchargeForFareByRuleFareWhenIsNotPercentAndIsCalculated);
  CPPUNIT_TEST(testAddSurchargeForDiscountedFareByRuleFareWhenIsNotPercentAndIsCalculated);

  CPPUNIT_TEST(testMatchSegmentGoverningCarrier_ArunkSegment);
  CPPUNIT_TEST(testMatchSegmentGoverningCarrier_SameCarrier);
  CPPUNIT_TEST(testMatchSegmentGoverningCarrier_DifferentCarrier);

  CPPUNIT_TEST(testHongKongException_NotFromHKG);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurrsNotHKD);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurr1HKD);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurr2HKD);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeo);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeoAnd);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeoBtw);
  CPPUNIT_TEST(testHongKongException_FromHKG_SurchargeCurrsNotEmptyEquip);

  CPPUNIT_TEST(testShouldMatchDOW_Blank_first);
  CPPUNIT_TEST(testShouldMatchDOW_Blank_second);
  CPPUNIT_TEST(testShouldMatchDOW_OneWay_first);
  CPPUNIT_TEST(testShouldMatchDOW_OneWay_second);
  CPPUNIT_TEST(testShouldMatchDOW_RoundTrip_first);
  CPPUNIT_TEST(testShouldMatchDOW_RoundTrip_second);
  CPPUNIT_TEST(testShouldMatchDOW_PerTicket_first);
  CPPUNIT_TEST(testShouldMatchDOW_PerTicket_second);
  CPPUNIT_TEST(testShouldMatchDOW_PerDirection_first);
  CPPUNIT_TEST(testShouldMatchDOW_PerDirection_second);
  CPPUNIT_TEST(testShouldMatchDOW_PerTransfer_first);
  CPPUNIT_TEST(testShouldMatchDOW_PerTransfer_second);
  CPPUNIT_TEST(testShouldMatchDOW_PerCoupon_first);
  CPPUNIT_TEST(testShouldMatchDOW_PerCoupon_second);

  CPPUNIT_TEST(testFindGoverningCarrierSegment_AABBAA_GovAA);
  CPPUNIT_TEST(testFindGoverningCarrierSegment_AABBAA_GovBB);
  CPPUNIT_TEST(testFindGoverningCarrierSegment_AABBAA_GovCC);

  CPPUNIT_TEST(testProcessSectionPortion_Sector_NoBtw_Skip);
  CPPUNIT_TEST(testProcessSectionPortion_Sector_NoAnd_Skip);
  CPPUNIT_TEST(testProcessSectionPortion_Sector_MatchAAABBB);
  CPPUNIT_TEST(testProcessSectionPortion_Sector_MatchBBBAAA);
  CPPUNIT_TEST(testProcessSectionPortion_Sector_NoMatchBBBDDD);
  CPPUNIT_TEST(testProcessSectionPortion_Portion_NoBtw_Skip);
  CPPUNIT_TEST(testProcessSectionPortion_Portion_NoAnd_Skip);
  CPPUNIT_TEST(testProcessSectionPortion_Portion_MatchAAABBB);
  CPPUNIT_TEST(testProcessSectionPortion_Portion_MatchBBBDDD);
  CPPUNIT_TEST(testProcessSectionPortion_Portion_NoMatchFFFAAA);

  CPPUNIT_TEST(testMatchT986_CxrFlight_Pass);
  CPPUNIT_TEST(testMatchT986_NoT986_Fail);
  CPPUNIT_TEST(testMatchT986_NoT986Segs_Fail);
  CPPUNIT_TEST(testMatchT986_NoMatchMark);
  CPPUNIT_TEST(testMatchT986_NoMatchOper);
  CPPUNIT_TEST(testMatchT986_NoMatchFlt1);
  CPPUNIT_TEST(testMatchT986_NoMatchFltRange);
  CPPUNIT_TEST(testMatchT986_Pass_Flt1);
  CPPUNIT_TEST(testMatchT986_Pass_FltRange);
  CPPUNIT_TEST(testMatchT986_Pass_AnyFlt);
  CPPUNIT_TEST(testMatchT986_Pass_RTW);
  CPPUNIT_TEST(testMatchT986_Fail_RTW);
  CPPUNIT_TEST(testMatchT986_Pass_No_Marketing_RTW);
  CPPUNIT_TEST(testMatchT986_Fail_Operating_Only_RTW);
  CPPUNIT_TEST(testMatchT986_Pass_Exact_Match_Marketing_RTW);
  CPPUNIT_TEST(testMatchT986_Pass_Exact_Match_Marketing_And_Operating_RTW);

  CPPUNIT_TEST(testMatchRBD_NoRbd_Pass);
  CPPUNIT_TEST(testMatchRBD_NoBookingCode_Fail);
  CPPUNIT_TEST(testMatchRBD_MatchBookingCode_Pass);

  CPPUNIT_TEST(testNegativeSurcharge_alpha);
  CPPUNIT_TEST(testNegativeSurcharge_digit);
  CPPUNIT_TEST(testNegativeSurcharge_blank);

  CPPUNIT_TEST(testCommonTravelSeg0match);
  CPPUNIT_TEST(testCommonTravelSeg0matchSecondEmpty);
  CPPUNIT_TEST(testCommonTravelSeg1match);
  CPPUNIT_TEST(testCommonTravelSeg1match1no);
  CPPUNIT_TEST(testCommonTravelSeg2match);

  CPPUNIT_TEST(testValidatePerTransferAllFc1);
  CPPUNIT_TEST(testValidatePerTransferAllFc2);
  CPPUNIT_TEST(testValidatePerTransferAllFc3);
  CPPUNIT_TEST(testValidatePerTransferSkipFirst);
  CPPUNIT_TEST(testValidatePerTransferSkipLast);
  CPPUNIT_TEST(testValidatePerTransferSkipIfOnlyOne);

  CPPUNIT_TEST(testAddSurchargeForPerTransferOrigin);
  CPPUNIT_TEST(testAddSurchargeForPerTransferDest);
  CPPUNIT_TEST(testAddSurchargeForPerTransferFc);
  CPPUNIT_TEST(testAddSurchargeShouldSetCurrencyUSDAmountTruncatedAPO40780);

  CPPUNIT_TEST_SUITE_END();

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    NUCInfo* getNuc(double nucFactor,
                    double roundingFactor,
                    int nucFactorNodec,
                    int roundingFactorNodec,
                    RoundingRule rule)
    {
      NUCInfo* ret = _memHandle.create<NUCInfo>();
      ret->_nucFactor = nucFactor;
      ret->_roundingFactor = roundingFactor;
      ret->_nucFactorNodec = nucFactorNodec;
      ret->_roundingFactorNodec = roundingFactorNodec;
      ret->_roundingRule = rule;
      return ret;
    }
    GeoRuleItem* getGeo(int item, LocCode loc)
    {
      GeoRuleItem* ret = _memHandle.create<GeoRuleItem>();
      ret->loc1().loc() = loc;
      ret->loc1().locType() = LOCTYPE_AIRPORT;
      return ret;
    }
    CarrierFlightSeg* T986seg(CarrierCode ma, CarrierCode op, int flt1, int flt2)
    {
      CarrierFlightSeg* s = new CarrierFlightSeg; // will be deleted by parent destructor
      s->marketingCarrier() = ma;
      s->operatingCarrier() = op;
      s->flt1() = flt1;
      s->flt2() = flt2;
      return s;
    }

  public:
    const CarrierFlight* getCarrierFlight(const VendorCode& vendor, int itemNo)
    {
      CarrierFlight* ret = _memHandle.create<CarrierFlight>();
      ret->segCnt() = 1;

      if (itemNo == 1000)
        return 0;
      else if (itemNo == 1001)
        return ret;
      else if (itemNo == 1)
        ret->segs() += T986seg("XX", "", 0, 0);
      else if (itemNo == 2)
        ret->segs() += T986seg("", "XX", 0, 0);
      else if (itemNo == 3)
        ret->segs() += T986seg("XX", "", 5, 0);
      else if (itemNo == 4)
        ret->segs() += T986seg("XX", "", 500, 1000);
      else if (itemNo == 5)
        ret->segs() += T986seg("BB", "AA", 200, 0);
      else if (itemNo == 6)
        ret->segs() += T986seg("BB", "AA", 190, 200);
      else if (itemNo == 7)
        ret->segs() += T986seg("BB", "AA", -1, 0);
      else if (itemNo == 8)
        ret->segs() += T986seg("*A", "*A", -1, 0);
      else if (itemNo == 9)
        ret->segs() += T986seg("*B", "*B", -1, 0);
      else if (itemNo == 10)
        ret->segs() += T986seg("*A", "", -1, 0);
      else if (itemNo == 11)
        ret->segs() += T986seg("*B", "", -1, 0);
      else if (itemNo == 12)
      {
        ret->segs() += T986seg("*B", "", -1, 0);
        ret->segs() += T986seg("AA", "", -1, 0);
      }
      else if (itemNo == 13)
      {
        ret->segs() += T986seg("*B", "*B", -1, 0);
        ret->segs() += T986seg("AA", "BB", -1, 0);
      }
      else
        return DataHandleMock::getCarrierFlight(vendor, itemNo);
      return ret;
    }
    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      if (currency == "USD")
        return getNuc(1, 1, 8, 0, NEAREST);
      else if (currency == "GBP")
        return getNuc(0.68871800, 1, 8, 0, NEAREST);

      return DataHandleMock::getNUCFirst(currency, carrier, date);
    }
    const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber)
    {
      std::vector<GeoRuleItem*>* ret = _memHandle.create<std::vector<GeoRuleItem*> >();
      if (itemNumber == 1)
        *ret += getGeo(1, "AAA");
      else if (itemNumber == 2)
        *ret += getGeo(2, "BBB");
      else if (itemNumber == 3)
        *ret += getGeo(3, "CCC");
      else if (itemNumber == 4)
        *ret += getGeo(4, "DDD");
      else if (itemNumber == 5)
        *ret += getGeo(5, "EEE");
      else if (itemNumber == 6)
        *ret += getGeo(6, "FFF");
      else
        return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
      return *ret;
    }
    const std::vector<AirlineAllianceCarrierInfo*>&
    getAirlineAllianceCarrier(const CarrierCode& carrierCode)
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();

      if (carrierCode == "AA" || carrierCode == "BB")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*A";
        ret->push_back(cxrInfo);
        return *ret;
      }

      return *ret;
    }
  };

  class SurchargesRuleMock : public SurchargesRule
  {
    friend class SurchargesRuleTest;

  public:
    typedef std::pair<LocCode, LocCode> CityPair;
    typedef std::vector<CityPair> SurchargeStore;

    SurchargeStore _surchargeStore;

    SurchargesRuleMock() {}

  protected:
    virtual SurchargeData* addSurcharge(PricingTrx& trx,
                                        const FarePath* farePath,
                                        const PaxTypeFare& fare,
                                        FareUsage* fareUsage,
                                        const SurchargesInfo& surchInfo,
                                        SurchargeData* surchargeData,
                                        TravelSeg& tSeg,
                                        const LocCode& fcBrdCity,
                                        const LocCode& fcOffCity,
                                        bool singleSector = true)

    {
      _surchargeStore.push_back(std::make_pair(fcBrdCity, fcOffCity));
      return _memHandle.create<SurchargeData>();
    }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FVO_SURCHARGES_ENABLED", "N", "RULES_OPTIONS", true);

    _trx = _memHandle.create<PricingTrx>();
    _req = _memHandle.create<PricingRequest>();
    _fPath = _memHandle.create<FarePath>();
    _fM = _memHandle.create<FareMarket>();
    _fUsage = _memHandle.create<FareUsage>();

    _fInfo = _memHandle.create<FareInfo>();
    _fInfo->_carrier = "BA";
    _fInfo->_currency = "GBP";
    _fInfo->_fareAmount = 1000.00;
    _fInfo->_originalFareAmount = 1000.00;
    _fInfo->_noDec = 2;
    _fInfo->_currency = "USD";
    _fInfo->_vendor = "ATP";

    _fare = _memHandle.create<Fare>();
    _fare->initialize(Fare::FS_Domestic, _fInfo, *_fM);
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->fareMarket() = _fM;
    _ptf->setFare(_fare);
    _fUsage->paxTypeFare() = _ptf;
    _trx->setRequest(_req);
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    _trx->setOptions(pricingOptions);

    // to test active Qsurcharge path
    DateTime defaultDate(2026, 4, 1);

    _req->ticketingDT() = defaultDate;

    _itin = _memHandle.create<Itin>();
    _fPath->itin() = _itin;
    _fPath->itin()->calculationCurrency() = "USD";

    _seg = _memHandle.create<AirSeg>();
    _seg->origAirport() = "DFW";
    _seg->destAirport() = "ATL";

    _sInfo = _memHandle.create<SurchargesInfo>();
    _sInfo->surchargeCur1() = "";
    _sInfo->surchargeCur2() = "";
    _sInfo->surchargePercent() = 10;
    _sInfo->tvlPortion() = RuleConst::ROUNDTRIP;

    _srr = _memHandle.create<SurchargesRule>();
    _srr->setRuleDataAccess(_memHandle(new FareMarketDataAccess(*_trx, _itin, *_ptf)));

    _pu = _memHandle.create<PricingUnit>();
    _applTravelSegment = _memHandle.create<RuleUtil::TravelSegWrapperVector>();
  }

  void tearDown() { _memHandle.clear(); }

  void testValidate()
  {
    SurchargesInfo sr;
    sr.overrideDateTblItemNo() = 0;

    PricingTrx trx;

    Itin itin;
    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 0;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = dfw;
    seg->destination() = lon;
    seg->stopOver() = false;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg);

    request.formOfPaymentCash() = 'N';

    FarePath farePath;
    PricingUnit pricingUnit;
    FareUsage* fareUsage = _memHandle.create<FareUsage>();

    PaxType* pt = _memHandle.create<PaxType>();

    pt->paxType() = "ADT";
    pt->vendorCode() = "ATP"; // Fare.h was changed!?

    farePath.paxType() = pt;

    paxTypeFare->actualPaxType() = pt;

    fareUsage->paxTypeFare() = paxTypeFare;

    sr.unavailTag() = 'X';
    Record3ReturnTypes ret = _srr->validate(trx, farePath, pricingUnit, *fareUsage, &sr);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);

    // 1.
    sr.unavailTag() = 'Y';

    ret = _srr->validate(trx, farePath, pricingUnit, *fareUsage, &sr);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);

    // 2.
    fareInfo->_fareAmount = 100;
    sr.unavailTag() = ' ';
    sr.surchargeType() = RuleConst::BLANK; //  BLANK

    sr.surchargeAppl() = '1'; //  child surcharge

    ret = _srr->validate(trx, farePath, pricingUnit, *fareUsage, &sr);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);

    sr.surchargeType() = RuleConst::EQUIPMENT;
    sr.equipType().clear();

    ret = _srr->validate(trx, farePath, pricingUnit, *fareUsage, &sr);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testIsDataCorrect()
  {
    PricingTrx trx;
    Itin itin;
    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 0;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    request.formOfPaymentCash() = 'N';

    FarePath farePath;
    PricingUnit pricingUnit;
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare;

    fareInfo->_fareAmount = 100;

    SurchargesInfo sr;
    sr.surchargeType() = RuleConst::SIDETRIP; // 'I'

    bool ret = _srr->isDataCorrect(sr);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.startYear() = 3;

    ret = _srr->isDataCorrect(sr);

    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testCheckDateRange()
  {
    PricingTrx trx;
    Itin itin;
    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 0;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    request.formOfPaymentCash() = 'N';

    FarePath farePath;
    PricingUnit pricingUnit;
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare;

    fareInfo->_fareAmount = 100;

    request.ticketingDT() = DateTime::localTime();
    FareMarket* fm = _memHandle.create<FareMarket>();
    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    fm->origin() = dfw;
    fm->destination() = lon;

    paxTypeFare->fareMarket() = fm;

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = dfw;
    seg->destination() = lon;
    seg->stopOver() = false;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();

    request.formOfPaymentCash() = 'Y';
    trx.setRequest(&request);

    SurchargesInfo sr;
    sr.surchargeType() = RuleConst::FUEL;

    bool origin = true;

    bool ret = _srr->checkDateRange(trx, sr, *seg, origin);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.unavailTag() = ' ';
    sr.equipType().clear();
    sr.dow().clear();
    sr.todAppl() = ' ';
    sr.startYear() = 2;
    sr.startMonth() = 1;
    sr.startDay() = 30;
    sr.stopYear() = 113;
    sr.stopMonth() = 10;
    sr.stopDay() = 20;
    sr.surchargeType() = RuleConst::FUEL; //   != 'S'
    ret = _srr->checkDateRange(trx, sr, *seg, origin);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.startYear() = 112; // This test will fail starting in 2112

    ret = _srr->checkDateRange(trx, sr, *seg, origin);

    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testCheckDOWandTime()
  {
    PricingTrx trx;

    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 0;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    request.formOfPaymentCash() = 'N';

    FarePath farePath;
    PricingUnit pricingUnit;
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare;

    fareInfo->_fareAmount = 100;

    request.ticketingDT() = DateTime::localTime();
    FareMarket* fm = _memHandle.create<FareMarket>();
    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    fm->origin() = dfw;
    fm->destination() = lon;

    paxTypeFare->fareMarket() = fm;

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = dfw;
    seg->destination() = lon;
    seg->stopOver() = false;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();

    request.formOfPaymentCash() = 'Y';
    trx.setRequest(&request);

    SurchargesInfo sr;

    sr.surchargeType() = RuleConst::FUEL;

    sr.startTime() = -1;
    sr.stopTime() = -1;

    bool origin = true;

    bool ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.todAppl() = 'D';
    DateTime myDate(2004, 8, 25, 9, 40, 10); // dow =3 WED, time =9.40.10
    seg->departureDT() = myDate; //
    sr.dow() = "71"; // Sun,Mon
    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(false, ret);

    sr.dow() = "7123";
    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.dow() = "7123";
    sr.startTime() = 1;
    sr.stopTime() = 2;
    sr.todAppl() = 'D';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(false, ret);

    sr.dow() = "7123";
    sr.startTime() = 1;
    sr.stopTime() = 2259;
    sr.todAppl() = 'D';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.dow() = "3";
    sr.startTime() = 1;
    sr.stopTime() = 2400;
    sr.todAppl() = 'D';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.dow() = "3";
    sr.startTime() = 600;
    sr.stopTime() = 800;
    sr.todAppl() = 'R';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(false, ret);

    sr.dow() = "23";
    sr.startTime() = 600;
    sr.stopTime() = 500;
    sr.todAppl() = 'R';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(false, ret);

    sr.dow() = "23";
    sr.startTime() = 100;
    sr.stopTime() = 1000;
    sr.todAppl() = 'R';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    sr.dow() = "34";
    sr.startTime() = 2000;
    sr.stopTime() = 2010;
    sr.todAppl() = 'R';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(false, ret);

    sr.dow() = "34";
    sr.startTime() = 100;
    sr.stopTime() = 800;
    sr.todAppl() = 'R';

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testAddSurcharge()
  {
    MyDataHandle mdh;
    PricingTrx trx;

    FareMarket* fm = _memHandle.create<FareMarket>();

    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 0;
    fareInfo->_carrier = "BA";
    fareInfo->_currency = "GBP";
    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(Fare::FS_Domestic, fareInfo, *fm);

    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    request.formOfPaymentCash() = 'N';

    FarePath farePath;
    Itin itin;
    farePath.itin() = &itin;
    farePath.itin()->calculationCurrency() = "USD";

    PricingUnit pricingUnit;
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare;

    fareInfo->_fareAmount = 100;

    request.ticketingDT() = DateTime::localTime();

    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    fm->origin() = dfw;
    fm->destination() = lon;

    paxTypeFare->fareMarket() = fm;

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = dfw;
    seg->destination() = lon;
    seg->stopOver() = false;
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();

    request.formOfPaymentCash() = 'Y';
    trx.setRequest(&request);

    SurchargesInfo sr;

    sr.surchargeType() = RuleConst::FUEL;

    sr.startTime() = -1;
    sr.stopTime() = -1;

    bool origin = true;

    bool ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    ret = _srr->checkDOWandTime(trx, sr, *seg, origin, true);

    CPPUNIT_ASSERT_EQUAL(true, ret);

    SurchargeData* srd = 0;

    LocKey lk1;
    LocKey lk2;
    LocKey lk3;
    LocKey lk4;
    LocKey lk5;
    LocKey lk6;
    ret = false;

    trx.dataHandle().get(srd);

    sr.surchargeCur1() = "GBP";
    sr.surchargeAmt1() = 5000;

    if (srd != 0)
    {
      _srr->addSurcharge(trx, &farePath, *_ptf, fareUsage, sr, srd, *seg, "AAA", "BBB", true);

      if (srd->currSelected() == "GBP" && srd->amountSelected() == 5000 && srd->carrier() == "BA")
      {
        ret = true;
      }
    }

    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testAddSurchargeShouldSetCurrencyUSDAmountFiftyForOWFareMayBeDoubled()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    _fInfo->_owrt = ONE_WAY_MAY_BE_DOUBLED;

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(50.00, srd1->amountSelected());
  }

  void testAddSurchargeShouldSetCurrencyUSDAmountHundredForRoundTripMayNotBeHalved()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    _fInfo->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(50.00, srd1->amountSelected());
  }

  void testAddSurchargeShouldSetCurrencyUSDAmountFiftyForOWFareMayNotBeDoubled()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    _fInfo->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(50.00, srd1->amountSelected());
  }

  void testAddSurchargeShouldSetSurchargeAmountWhenIsNotPercent()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    setUpSurchargeAmountData();

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(125.00, srd1->amountSelected());
  }

  void testAddSurchargeForDiscountedFareWhenIsNotPercentAndIsNotCalculated()
  {
    setUpDiscountedFare(RuleConst::SPECIFIED, 0.0, *_fUsage->paxTypeFare());
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    setUpSurchargeAmountData();

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(125.00, srd1->amountSelected());
  }

  void testAddSurchargeForDiscountedFareWhenIsNotPercentAndIsCalculated()
  {
    setUpDiscountedFare(RuleConst::CALCULATED, 10.0, *_fUsage->paxTypeFare());
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    setUpSurchargeAmountData();

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(12.50, srd1->amountSelected());
  }

  void testAddSurchargeForFareByRuleFareWhenIsNotPercentAndIsCalculated()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    setUpSurchargeAmountData();

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(125.00, srd1->amountSelected());
  }

  void testAddSurchargeForDiscountedFareByRuleFareWhenIsNotPercentAndIsCalculated()
  {
    setUpDiscountedFareByRuleFare();
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    setUpSurchargeAmountData();

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(125.00, srd1->amountSelected());
  }

  void setUpDiscountedFare(Indicator farecalcInd, MoneyAmount discPercent, PaxTypeFare& ptFare)
  {
    PaxTypeFareRuleData* ptfRuleData = _memHandle.create<PaxTypeFareRuleData>();
    DiscountInfo* di = _memHandle.create<DiscountInfo>();
    di->farecalcInd() = farecalcInd;
    di->discPercent() = discPercent;
    ptfRuleData->ruleItemInfo() = di;
    ptFare.setRuleData(19, _trx->dataHandle(), ptfRuleData);
    ptFare.status().set(PaxTypeFare::PTF_Discounted);

    PaxType* paxType = _memHandle.create<PaxType>();
    _fPath->paxType() = paxType;
    _fPath->paxType()->paxType() = "CNN";
  }

  void setUpDiscountedFareByRuleFare()
  {
    PaxTypeFare* fbrPaxTypeFare = _memHandle.create<PaxTypeFare>();
    FareInfo* fbrFareInfo = _memHandle.create<FareInfo>();
    fbrFareInfo->_currency = "USD";
    fbrFareInfo->_vendor = "ATP";

    Fare* fbrFare = _memHandle.create<Fare>();
    fbrFare->initialize(Fare::FS_Domestic, fbrFareInfo, *_fM);
    fbrPaxTypeFare->setFare(fbrFare);

    setUpDiscountedFare(RuleConst::CALCULATED, 10.0, *fbrPaxTypeFare);
  }

  void setUpSurchargeAmountData()
  {
    _sInfo->surchargeAmt1() = 250.00;
    _sInfo->surchargeCur1() = "USD";
    _sInfo->surchargeAppl() = RuleConst::ADT_CHILD_DISC_INFANT_DISC_SURCHARGE;
  }

  void testIsSideTrip()
  {
    PricingTrx trx;

    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    const Loc* lgw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGW.xml");
    const Loc* rix = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRIX.xml");
    const Loc* prg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPRG.xml");
    const Loc* rom = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocROM.xml");
    const Loc* par = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    const Loc* mil = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIL.xml");

    // dfw-lhr
    AirSeg* seg1 = _memHandle.create<AirSeg>();
    seg1->pnrSegment() = 1;
    seg1->segmentOrder() = 1;
    seg1->origin() = dfw;
    seg1->destination() = lhr;
    seg1->stopOver() = false;
    seg1->carrier() = "AA";
    seg1->departureDT() = DateTime::localTime();
    seg1->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg1);
    // lgw-rix
    AirSeg* seg2 = _memHandle.create<AirSeg>();
    seg2->pnrSegment() = 2;
    seg2->segmentOrder() = 1;
    seg2->origin() = lgw;
    seg2->destination() = rix;
    seg2->stopOver() = false;
    seg2->carrier() = "AA";
    seg2->departureDT() = DateTime::localTime();
    seg2->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg2);
    // rix-prg
    AirSeg* seg3 = _memHandle.create<AirSeg>();
    seg3->pnrSegment() = 3;
    seg3->segmentOrder() = 1;
    seg3->origin() = rix;
    seg3->destination() = prg;
    seg3->stopOver() = false;
    seg3->carrier() = "AA";
    seg3->departureDT() = DateTime::localTime();
    seg3->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg3);
    // prg-rom
    AirSeg* seg4 = _memHandle.create<AirSeg>();
    seg4->pnrSegment() = 4;
    seg4->segmentOrder() = 1;
    seg4->origin() = prg;
    seg4->destination() = rom;
    seg4->stopOver() = false;
    seg4->carrier() = "AA";
    seg4->departureDT() = DateTime::localTime();
    seg4->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg4);
    // rom-par
    AirSeg* seg5 = _memHandle.create<AirSeg>();
    seg5->pnrSegment() = 5;
    seg5->segmentOrder() = 1;
    seg5->origin() = rom;
    seg5->destination() = par;
    seg5->stopOver() = false;
    seg5->carrier() = "AA";
    seg5->departureDT() = DateTime::localTime();
    seg5->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg5);
    // par-mil
    AirSeg* seg6 = _memHandle.create<AirSeg>();
    seg6->pnrSegment() = 6;
    seg6->segmentOrder() = 1;
    seg6->origin() = par;
    seg6->destination() = mil;
    seg6->stopOver() = false;
    seg6->carrier() = "AA";
    seg6->departureDT() = DateTime::localTime();
    seg6->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg6);
    // mil-dfw
    AirSeg* seg7 = _memHandle.create<AirSeg>();
    seg7->pnrSegment() = 7;
    seg7->segmentOrder() = 1;
    seg7->origin() = mil;
    seg7->destination() = dfw;
    seg7->stopOver() = false;
    seg7->carrier() = "AA";
    seg7->departureDT() = DateTime::localTime();
    seg7->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg7);
    // rix-lhr
    AirSeg* seg8 = _memHandle.create<AirSeg>();
    seg8->pnrSegment() = 8;
    seg8->segmentOrder() = 1;
    seg8->origin() = rix;
    seg8->destination() = lhr;
    seg8->stopOver() = false;
    seg8->carrier() = "AA";
    seg8->departureDT() = DateTime::localTime();
    seg8->arrivalDT() = DateTime::localTime();

    // lhr-rix
    AirSeg* seg9 = _memHandle.create<AirSeg>();
    seg9->pnrSegment() = 9;
    seg9->segmentOrder() = 1;
    seg9->origin() = lhr;
    seg9->destination() = rix;
    seg9->stopOver() = false;
    seg9->carrier() = "AA";
    seg9->departureDT() = DateTime::localTime();
    seg9->arrivalDT() = DateTime::localTime();
    // lhr-par
    AirSeg* segA = _memHandle.create<AirSeg>();
    segA->pnrSegment() = 10;
    segA->segmentOrder() = 1;
    segA->origin() = lhr;
    segA->destination() = par;
    segA->stopOver() = false;
    segA->carrier() = "AA";
    segA->departureDT() = DateTime::localTime();
    segA->arrivalDT() = DateTime::localTime();

    //.................................................
    bool ret = false;
    //  int count = 0;
    bool orig = true;
    std::vector<TravelSeg*>::const_iterator itRet;

    std::vector<TravelSeg*>::const_iterator itB = trx.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator itE = trx.travelSeg().end();
    std::vector<TravelSeg*>::const_iterator itL = itE - 1;
    std::vector<TravelSeg*>::const_iterator itBegin = itB;

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTrip(itB, itE, itBegin, itL, orig, itRet))
      {
        ret = true;
      }
    }
    CPPUNIT_ASSERT_EQUAL(false, ret);
    // dfw-lhr-rix-lhr
    ret = false;

    orig = true;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTrip(itB, itE, itBegin, itL, orig, itRet))
      {
        ret = true;
        break;
      }
    }
    CPPUNIT_ASSERT_EQUAL(false, ret);

    // dfw-lhr-rix-lhr-par
    ret = false;
    //  count = 0;
    orig = true;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);
    trx.travelSeg().push_back(segA);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTrip(itB, itE, itBegin, itL, orig, itRet))
      {
        ret = true;
        break;
      }
    }
    CPPUNIT_ASSERT_EQUAL(true, ret);

    // dfw-lhr-rix-lhr-par
    ret = false;
    //  count = 0;
    orig = false;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);
    trx.travelSeg().push_back(segA);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTrip(itB, itE, itBegin, itL, orig, itRet))
      {
        //
        ret = true;
        break;
        //
      }
    }
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testIsSideTripBtwAnd()
  {
    PricingTrx trx;

    const Loc* dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    const Loc* lgw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGW.xml");
    const Loc* rix = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRIX.xml");
    const Loc* prg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPRG.xml");
    const Loc* rom = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocROM.xml");
    const Loc* par = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    const Loc* mil = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIL.xml");
    // dfw-lhr
    AirSeg* seg1 = _memHandle.create<AirSeg>();
    seg1->pnrSegment() = 1;
    seg1->segmentOrder() = 1;
    seg1->origin() = dfw;
    seg1->destination() = lhr;
    seg1->stopOver() = false;
    seg1->carrier() = "AA";
    seg1->departureDT() = DateTime::localTime();
    seg1->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg1);
    // lgw-rix
    AirSeg* seg2 = _memHandle.create<AirSeg>();
    seg2->pnrSegment() = 2;
    seg2->segmentOrder() = 1;
    seg2->origin() = lgw;
    seg2->destination() = rix;
    seg2->stopOver() = false;
    seg2->carrier() = "AA";
    seg2->departureDT() = DateTime::localTime();
    seg2->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg2);
    // rix-prg
    AirSeg* seg3 = _memHandle.create<AirSeg>();
    seg3->pnrSegment() = 3;
    seg3->segmentOrder() = 1;
    seg3->origin() = rix;
    seg3->destination() = prg;
    seg3->stopOver() = false;
    seg3->carrier() = "AA";
    seg3->departureDT() = DateTime::localTime();
    seg3->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg3);
    // prg-rom
    AirSeg* seg4 = _memHandle.create<AirSeg>();
    seg4->pnrSegment() = 4;
    seg4->segmentOrder() = 1;
    seg4->origin() = prg;
    seg4->destination() = rom;
    seg4->stopOver() = false;
    seg4->carrier() = "AA";
    seg4->departureDT() = DateTime::localTime();
    seg4->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg4);
    // rom-par
    AirSeg* seg5 = _memHandle.create<AirSeg>();
    seg5->pnrSegment() = 5;
    seg5->segmentOrder() = 1;
    seg5->origin() = rom;
    seg5->destination() = par;
    seg5->stopOver() = false;
    seg5->carrier() = "AA";
    seg5->departureDT() = DateTime::localTime();
    seg5->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg5);
    // par-mil
    AirSeg* seg6 = _memHandle.create<AirSeg>();
    seg6->pnrSegment() = 6;
    seg6->segmentOrder() = 1;
    seg6->origin() = par;
    seg6->destination() = mil;
    seg6->stopOver() = false;
    seg6->carrier() = "AA";
    seg6->departureDT() = DateTime::localTime();
    seg6->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg6);
    // mil-dfw
    AirSeg* seg7 = _memHandle.create<AirSeg>();
    seg7->pnrSegment() = 7;
    seg7->segmentOrder() = 1;
    seg7->origin() = mil;
    seg7->destination() = dfw;
    seg7->stopOver() = false;
    seg7->carrier() = "AA";
    seg7->departureDT() = DateTime::localTime();
    seg7->arrivalDT() = DateTime::localTime();
    trx.travelSeg().push_back(seg7);
    // rix-lhr
    AirSeg* seg8 = _memHandle.create<AirSeg>();
    seg8->pnrSegment() = 8;
    seg8->segmentOrder() = 1;
    seg8->origin() = rix;
    seg8->destination() = lhr;
    seg8->stopOver() = false;
    seg8->carrier() = "AA";
    seg8->departureDT() = DateTime::localTime();
    seg8->arrivalDT() = DateTime::localTime();

    // lhr-rix
    AirSeg* seg9 = _memHandle.create<AirSeg>();
    seg9->pnrSegment() = 9;
    seg9->segmentOrder() = 1;
    seg9->origin() = lhr;
    seg9->destination() = rix;
    seg9->stopOver() = false;
    seg9->carrier() = "AA";
    seg9->departureDT() = DateTime::localTime();
    seg9->arrivalDT() = DateTime::localTime();
    // lhr-par
    AirSeg* segA = _memHandle.create<AirSeg>();
    segA->pnrSegment() = 10;
    segA->segmentOrder() = 1;
    segA->origin() = lhr;
    segA->destination() = par;
    segA->stopOver() = false;
    segA->carrier() = "AA";
    segA->departureDT() = DateTime::localTime();
    segA->arrivalDT() = DateTime::localTime();

    //.................................................
    bool ret = false;
    //  int count = 0;
    bool orig = true;

    std::vector<TravelSeg*>::const_iterator itB = trx.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator itE = trx.travelSeg().end();
    std::vector<TravelSeg*>::const_iterator itL = itE - 1;
    std::vector<TravelSeg*>::const_iterator itBegin = itB;
    std::vector<TravelSeg*>::const_iterator itRet;
    //.................................................
    RuleUtil::TravelSegWrapperVector vector;
    //.................................................

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTripBtwAnd(itB, itE, itBegin, itL, vector, orig, itRet))
      {
        ret = true;
      }
    }
    CPPUNIT_ASSERT_EQUAL(false, ret);
    // dfw-lhr-rix-lhr
    ret = false;
    //  count = 0;
    orig = true;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................
    RuleUtil::TravelSegWrapper* tsw1 = 0;
    trx.dataHandle().get(tsw1);
    tsw1->travelSeg() = seg8;
    tsw1->origMatch() = true;
    tsw1->destMatch() = false;
    vector.push_back(tsw1);
    //...................................................
    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTripBtwAnd(itB, itE, itBegin, itL, vector, orig, itRet))
      {
        ret = true;
      }
    }
    CPPUNIT_ASSERT_EQUAL(false, ret);

    // dfw-lhr-rix-lhr-par
    ret = false;
    //  count = 0;
    orig = true;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);
    trx.travelSeg().push_back(segA);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTripBtwAnd(itB, itE, itBegin, itL, vector, orig, itRet))
      {
        ret = true;
        break;
      }
    }
    CPPUNIT_ASSERT_EQUAL(true, ret);

    // dfw-lhr-rix-lhr-par
    ret = false;
    //  count = 0;
    orig = true;

    trx.travelSeg().clear();
    trx.travelSeg().push_back(seg1);
    trx.travelSeg().push_back(seg9);
    trx.travelSeg().push_back(seg8);
    trx.travelSeg().push_back(segA);

    itB = trx.travelSeg().begin();
    itE = trx.travelSeg().end();
    itL = itE - 1;
    itBegin = itB;
    //...................................................
    vector.clear();
    RuleUtil::TravelSegWrapper* tsw2 = 0;
    trx.dataHandle().get(tsw2);
    tsw2->travelSeg() = seg8;
    tsw2->origMatch() = true;
    tsw2->destMatch() = false;
    vector.push_back(tsw2);

    for (; itB != itE; ++itB)
    {
      if (_srr->isSideTripBtwAnd(itB, itE, itBegin, itL, vector, orig, itRet))
      {
        ret = true;
        break;
      }
    }
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testMatchSegmentGoverningCarrier_ArunkSegment()
  {
    ArunkSeg travelSeg;

    CPPUNIT_ASSERT_EQUAL(true, _srr->matchSegmentGoverningCarrier("AA", &travelSeg));
  }

  void testMatchSegmentGoverningCarrier_SameCarrier()
  {
    CarrierCode carrier("AA");

    _seg->carrier() = carrier;

    CPPUNIT_ASSERT_EQUAL(true, _srr->matchSegmentGoverningCarrier(carrier, _seg));
  }

  void testMatchSegmentGoverningCarrier_DifferentCarrier()
  {
    _seg->carrier() = "AA";

    CPPUNIT_ASSERT_EQUAL(false, _srr->matchSegmentGoverningCarrier("LO", _seg));
  }

  void testHongKongException_NotFromHKG()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "KRK";

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurrsNotHKD()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "HKG";

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurr1HKD()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);

    _sInfo->surchargeCur1() = "HKD";
    _seg->origAirport() = "HKG";

    CPPUNIT_ASSERT_EQUAL(true, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurr2HKD()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);

    _sInfo->surchargeCur2() = "HKD";
    _seg->origAirport() = "HKG";

    CPPUNIT_ASSERT_EQUAL(true, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeo()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "HKG";
    _sInfo->surchargeCur1() = "HKD";
    _sInfo->surchargeCur2() = "HKD";
    _sInfo->geoTblItemNo() = 1;

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeoAnd()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "HKG";
    _sInfo->surchargeCur1() = "HKD";
    _sInfo->surchargeCur2() = "HKD";
    _sInfo->geoTblItemNo() = 0;
    _sInfo->geoTblItemNoAnd() = 1;

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurrsNotEmptyGeoBtw()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "HKG";
    _sInfo->surchargeCur1() = "HKD";
    _sInfo->surchargeCur2() = "HKD";
    _sInfo->geoTblItemNo() = 0;
    _sInfo->geoTblItemNoAnd() = 0;
    _sInfo->geoTblItemNoBtw() = 1;

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testHongKongException_FromHKG_SurchargeCurrsNotEmptyEquip()
  {
    std::vector<TravelSeg*> segs;

    segs.push_back(_seg);
    _seg->origAirport() = "HKG";
    _sInfo->surchargeCur1() = "HKD";
    _sInfo->surchargeCur2() = "HKD";
    _sInfo->geoTblItemNo() = 0;
    _sInfo->geoTblItemNoAnd() = 0;
    _sInfo->geoTblItemNoBtw() = 0;
    _sInfo->equipType() = "A";

    CPPUNIT_ASSERT_EQUAL(false, _srr->hongKongException(*_trx, segs, *_sInfo));
  }

  void testShouldMatchDOW_Blank_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW(' ', true));
  }
  void testShouldMatchDOW_Blank_second()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW(' ', false));
  }
  void testShouldMatchDOW_OneWay_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('1', true));
  }
  void testShouldMatchDOW_OneWay_second()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW('1', false));
  }
  void testShouldMatchDOW_RoundTrip_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('2', true));
  }
  void testShouldMatchDOW_RoundTrip_second()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW('2', false));
  }
  void testShouldMatchDOW_PerTicket_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('4', true));
  }
  void testShouldMatchDOW_PerTicket_second()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW('4', false));
  }
  void testShouldMatchDOW_PerDirection_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('6', true));
  }
  void testShouldMatchDOW_PerDirection_second()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW('6', false));
  }
  void testShouldMatchDOW_PerTransfer_first()
  {
    CPPUNIT_ASSERT_EQUAL(false, _srr->shouldMatchDOW('3', true));
  }
  void testShouldMatchDOW_PerTransfer_second()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('3', false));
  }
  void testShouldMatchDOW_PerCoupon_first()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('5', true));
  }
  void testShouldMatchDOW_PerCoupon_second()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->shouldMatchDOW('5', false));
  }
  void testFindGoverningCarrierSegment_AABBAA_GovAA()
  {
    std::vector<TravelSeg*>& v = createTravelSegVector();
    CPPUNIT_ASSERT_EQUAL(ptrdiff_t(2),
                         _srr->findGoverningCarrierSegment(v, v.begin() + 1, "AA") - v.begin());
  }
  void testFindGoverningCarrierSegment_AABBAA_GovBB()
  {
    std::vector<TravelSeg*>& v = createTravelSegVector();
    CPPUNIT_ASSERT_EQUAL(ptrdiff_t(1),
                         _srr->findGoverningCarrierSegment(v, v.begin() + 1, "BB") - v.begin());
  }
  void testFindGoverningCarrierSegment_AABBAA_GovCC()
  {
    std::vector<TravelSeg*>& v = createTravelSegVector();
    CPPUNIT_ASSERT_EQUAL(ptrdiff_t(1),
                         _srr->findGoverningCarrierSegment(v, v.begin() + 1, "CC") - v.begin());
  }
  std::vector<TravelSeg*>& createTravelSegVector()
  {
    std::vector<TravelSeg*>* ret = _memHandle.create<std::vector<TravelSeg*> >();
    AirSeg* a1 = _memHandle.create<AirSeg>(), *a2 = _memHandle.create<AirSeg>(),
            *a3 = _memHandle.create<AirSeg>();
    a1->carrier() = a3->carrier() = "AA";
    a2->carrier() = "BB";
    *ret += a1, a2, a3;
    return *ret;
  }

  void testProcessSectionPortion_Sector_NoBtw_Skip()
  {
    initSecPorSegs('S', 0, 2);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
  }
  void testProcessSectionPortion_Sector_NoAnd_Skip()
  {
    initSecPorSegs('S', 1, 0);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
  }
  void testProcessSectionPortion_Sector_MatchAAABBB()
  {
    initSecPorSegs('S', 1, 2);
    CPPUNIT_ASSERT(_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _applTravelSegment->size());
  }
  void testProcessSectionPortion_Sector_MatchBBBAAA()
  {
    initSecPorSegs('S', 2, 1);
    CPPUNIT_ASSERT(_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _applTravelSegment->size());
  }
  void testProcessSectionPortion_Sector_NoMatchBBBDDD()
  {
    initSecPorSegs('S', 2, 4);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(0), _applTravelSegment->size());
  }
  void testProcessSectionPortion_Portion_NoBtw_Skip()
  {
    initSecPorSegs('P', 0, 2);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
  }
  void testProcessSectionPortion_Portion_NoAnd_Skip()
  {
    initSecPorSegs('P', 1, 0);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
  }
  void testProcessSectionPortion_Portion_MatchAAABBB()
  {
    initSecPorSegs('P', 1, 2);
    CPPUNIT_ASSERT(_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _applTravelSegment->size()); // AAA-BBB-CCC-DDD-AAA-CCC-AAA-BBB
  }
  void testProcessSectionPortion_Portion_MatchBBBDDD()
  {
    initSecPorSegs('P', 2, 4);
    CPPUNIT_ASSERT(_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _applTravelSegment->size());
  }
  void testProcessSectionPortion_Portion_NoMatchFFFAAA()
  {
    initSecPorSegs('P', 6, 1);
    CPPUNIT_ASSERT(!_srr->processSectionPortion(
                             *_trx, _fPath, _pu, _fUsage, *_ptf, _sInfo, *_applTravelSegment, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(0), _applTravelSegment->size());
  }
  void testMatchT986_CxrFlight_Pass()
  {
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoT986_Fail()
  {
    initT986Data(1000);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoT986Segs_Fail()
  {
    initT986Data(1001);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoMatchMark()
  {
    initT986Data(1);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoMatchOper()
  {
    initT986Data(2);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoMatchFlt1()
  {
    initT986Data(3);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_NoMatchFltRange()
  {
    initT986Data(4);
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_Flt1()
  {
    initT986Data(5);
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_FltRange()
  {
    initT986Data(6);
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_AnyFlt()
  {
    initT986Data(7);
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_RTW()
  {
    initT986Data(8);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Fail_RTW()
  {
    initT986Data(9);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(!_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_No_Marketing_RTW()
  {
    initT986Data(10);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Fail_Operating_Only_RTW()
  {
    initT986Data(11);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(!_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_Exact_Match_Marketing_RTW()
  {
    initT986Data(12);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchT986_Pass_Exact_Match_Marketing_And_Operating_RTW()
  {
    initT986Data(13);
    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(_srr->matchT986(*_trx, _sInfo, "ATP", *_applTravelSegment, 0));
  }
  void testMatchRBD_NoRbd_Pass()
  {
    initRbd("", "A", "B");
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchRBD(_sInfo, *_applTravelSegment, 0, *_trx, _fUsage));
  }
  void testMatchRBD_NoBookingCode_Fail()
  {
    initRbd("X", "A", "B");
    CPPUNIT_ASSERT_EQUAL(false, _srr->matchRBD(_sInfo, *_applTravelSegment, 0, *_trx, _fUsage));
  }
  void testMatchRBD_MatchBookingCode_Pass()
  {
    initRbd("B", "A", "B");
    CPPUNIT_ASSERT_EQUAL(true, _srr->matchRBD(_sInfo, *_applTravelSegment, 0, *_trx, _fUsage));
  }

  void testNegativeSurcharge_alpha()
  {
    SurchargesInfo surcharge;
    DiagManager diagMgr(*_trx);
    surcharge.surchargeAppl() = 'A';

    CPPUNIT_ASSERT(_srr->isNegativeSurcharge(&surcharge, diagMgr, false));
  }

  void testNegativeSurcharge_digit()
  {
    SurchargesInfo surcharge;
    DiagManager diagMgr(*_trx);
    surcharge.surchargeAppl() = '1';

    CPPUNIT_ASSERT(!_srr->isNegativeSurcharge(&surcharge, diagMgr, false));
  }

  void testNegativeSurcharge_blank()
  {
    SurchargesInfo surcharge;
    DiagManager diagMgr(*_trx);
    surcharge.surchargeAppl() = RuleConst::BLANK;

    CPPUNIT_ASSERT(!_srr->isNegativeSurcharge(&surcharge, diagMgr, false));
  }

  void testCommonTravelSeg0match()
  {
    RuleUtil::TravelSegWrapperVector v1, v2;
    v1.push_back(makeTsw("MEL", "SYD"));
    v1.push_back(makeTsw("SYD", "SIN"));
    v1.push_back(makeTsw("SIN", "FRA"));

    v2.push_back(makeTsw("FRA", "SIN"));
    v2.push_back(makeTsw("SIN", "NYC"));

    _srr->commonTravelSeg(v1, v2);

    CPPUNIT_ASSERT_EQUAL(0, (int)v1.size());
  }

  void testCommonTravelSeg0matchSecondEmpty()
  {
    RuleUtil::TravelSegWrapperVector v1, v2;
    v1.push_back(makeTsw("MEL", "SYD"));
    v1.push_back(makeTsw("SYD", "SIN"));
    v1.push_back(makeTsw("SIN", "FRA"));

    _srr->commonTravelSeg(v1, v2);

    CPPUNIT_ASSERT_EQUAL(0, (int)v1.size());
  }

  void testCommonTravelSeg1match()
  {
    RuleUtil::TravelSegWrapperVector v1, v2;
    v1.push_back(makeTsw("MEL", "SYD"));
    v1.push_back(makeTsw("SYD", "SIN"));
    v1.push_back(makeTsw("SIN", "FRA"));
    v2.push_back(makeTsw("SYD", "SIN"));

    _srr->commonTravelSeg(v1, v2);

    CPPUNIT_ASSERT_EQUAL(1, (int)v1.size());
    CPPUNIT_ASSERT_EQUAL(LocCode("SYD"), v1.front()->travelSeg()->boardMultiCity());
    CPPUNIT_ASSERT_EQUAL(LocCode("SIN"), v1.front()->travelSeg()->offMultiCity());
  }

  void testCommonTravelSeg1match1no()
  {
    RuleUtil::TravelSegWrapperVector v1, v2;
    v1.push_back(makeTsw("MEL", "SYD"));
    v1.push_back(makeTsw("SYD", "SIN"));
    v1.push_back(makeTsw("SIN", "FRA"));

    v2.push_back(makeTsw("SYD", "SIN"));
    v2.push_back(makeTsw("SIN", "NYC"));

    _srr->commonTravelSeg(v1, v2);

    CPPUNIT_ASSERT_EQUAL(1, (int)v1.size());
    CPPUNIT_ASSERT_EQUAL(LocCode("SYD"), v1.front()->travelSeg()->boardMultiCity());
    CPPUNIT_ASSERT_EQUAL(LocCode("SIN"), v1.front()->travelSeg()->offMultiCity());
  }

  void testCommonTravelSeg2match()
  {
    RuleUtil::TravelSegWrapperVector v1, v2;

    v2.push_back(makeTsw("SYD", "SIN"));
    v2.push_back(makeTsw("SIN", "FRA"));
    v2.push_back(makeTsw("FRA", "NYC"));

    v1.push_back(makeTsw("MEL", "SYD"));
    v1.push_back(makeTsw("SYD", "SIN"));
    v1.push_back(makeTsw("SIN", "FRA"));

    _srr->commonTravelSeg(v1, v2);
    CPPUNIT_ASSERT_EQUAL(2, (int)v1.size());
  }

  void testValidatePerTransferAllFc1()
  {
    bool originMatch = true;
    bool destMatch = true;
    RuleUtil::TravelSegWrapperVector segmetns = getTswVector();
    RuleUtil::TravelSegWrapperVectorCI current = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(validationResult);
    CPPUNIT_ASSERT(!originMatch);
    CPPUNIT_ASSERT(destMatch);
  }

  void testValidatePerTransferAllFc2()
  {
    bool originMatch = true;
    bool destMatch = true;
    RuleUtil::TravelSegWrapperVector segmetns = getTswVector();
    RuleUtil::TravelSegWrapperVectorCI current = (segmetns.begin() + 1);
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(validationResult);
    CPPUNIT_ASSERT(originMatch);
    CPPUNIT_ASSERT(destMatch);
  }

  void testValidatePerTransferAllFc3()
  {
    bool originMatch = true;
    bool destMatch = true;
    RuleUtil::TravelSegWrapperVector segmetns = getTswVector();
    RuleUtil::TravelSegWrapperVectorCI current = (segmetns.end() - 1);
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(validationResult);
    CPPUNIT_ASSERT(originMatch);
    CPPUNIT_ASSERT(!destMatch);
  }

  void testValidatePerTransferSkipFirst()
  {
    bool originMatch = true;
    bool destMatch = false;
    RuleUtil::TravelSegWrapperVector segmetns = getTswVector();
    RuleUtil::TravelSegWrapperVectorCI current = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(!validationResult);
  }

  void testValidatePerTransferSkipLast()
  {
    bool originMatch = false;
    bool destMatch = true;
    RuleUtil::TravelSegWrapperVector segmetns = getTswVector();
    RuleUtil::TravelSegWrapperVectorCI current = (segmetns.end() - 1);
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(!validationResult);
  }

  void testValidatePerTransferSkipIfOnlyOne()
  {
    bool originMatch = true;
    bool destMatch = true;
    RuleUtil::TravelSegWrapperVector segmetns;
    segmetns.push_back(makeTsw("SYD", "SIN"));
    RuleUtil::TravelSegWrapperVectorCI current = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI begin = segmetns.begin();
    RuleUtil::TravelSegWrapperVectorCI last = (segmetns.end() - 1);

    bool validationResult =
        _srr->validatePerTransfer(current, begin, last, true, originMatch, destMatch, *_trx);
    CPPUNIT_ASSERT(!validationResult);
  }

  void testAddSurchargeForPerTransferOrigin()
  {
    bool originMatch = true;
    bool destMatch = false;
    SurchargesRule::travelSegVector segmetns = getTravelVector();
    SurchargesRuleMock* srm = _memHandle.create<SurchargesRuleMock>();
    // addSurchargeForPerTransfer from SurchargesRuleMock do not use Diag312Collector and because of
    // that it can be NULL
    Diag312Collector* diagPt = NULL;

    // second segment (SIN - FRA), SIN is transfer point
    SurchargesRule::travelSegConstIt current = (segmetns.begin() + 1);
    srm->addSurchargeForPerTransfer(
        *_trx, _fPath, *_ptf, _fUsage, *_sInfo, current, false, originMatch, destMatch, diagPt);
    CPPUNIT_ASSERT_EQUAL(1, (int)srm->_surchargeStore.size());

    // expected surcharge SYD-FRA
    CPPUNIT_ASSERT_EQUAL(LocCode("SYD"), srm->_surchargeStore.begin()->first);
    CPPUNIT_ASSERT_EQUAL(LocCode("FRA"), srm->_surchargeStore.begin()->second);
  }

  void testAddSurchargeForPerTransferDest()
  {
    bool originMatch = false;
    bool destMatch = true;
    SurchargesRule::travelSegVector segmetns = getTravelVector();
    SurchargesRuleMock* srm = _memHandle.create<SurchargesRuleMock>();
    // addSurchargeForPerTransfer from SurchargesRuleMock do not use Diag312Collector and because of
    // that it can be NULL
    Diag312Collector* diagPt = NULL;

    // second segment (SIN - FRA), FRA is transfer point
    SurchargesRule::travelSegConstIt current = (segmetns.begin() + 1);
    srm->addSurchargeForPerTransfer(
        *_trx, _fPath, *_ptf, _fUsage, *_sInfo, current, false, originMatch, destMatch, diagPt);
    CPPUNIT_ASSERT_EQUAL(1, (int)srm->_surchargeStore.size());

    // expected surcharge SIN-NYC
    CPPUNIT_ASSERT_EQUAL(LocCode("SIN"), srm->_surchargeStore.begin()->first);
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), srm->_surchargeStore.begin()->second);
  }

  void testAddSurchargeForPerTransferFc()
  {
    bool originMatch = false;
    bool destMatch = true;
    SurchargesRule::travelSegVector segmetns = getTravelVector();
    SurchargesRuleMock* srm = _memHandle.create<SurchargesRuleMock>();
    // addSurchargeForPerTransfer from SurchargesRuleMock do not use Diag312Collector and because of
    // that it can be NULL
    Diag312Collector* diagPt = NULL;

    // first segment, originMatch=false destMatch=true
    SurchargesRule::travelSegConstIt current = segmetns.begin();
    srm->addSurchargeForPerTransfer(
        *_trx, _fPath, *_ptf, _fUsage, *_sInfo, current, false, originMatch, destMatch, diagPt);

    // second segment, originMatch=true destMatch=true;
    current++;
    originMatch = true;
    srm->addSurchargeForPerTransfer(
        *_trx, _fPath, *_ptf, _fUsage, *_sInfo, current, false, originMatch, destMatch, diagPt);

    // third segment, originMatch=true destMatch=false
    current++;
    originMatch = true;
    destMatch = false;
    srm->addSurchargeForPerTransfer(
        *_trx, _fPath, *_ptf, _fUsage, *_sInfo, current, false, originMatch, destMatch, diagPt);

    // should be 2 surcharges
    CPPUNIT_ASSERT_EQUAL(2, (int)srm->_surchargeStore.size());

    // first surcharge SYD-FRA
    CPPUNIT_ASSERT_EQUAL(LocCode("SYD"), srm->_surchargeStore.begin()->first);
    CPPUNIT_ASSERT_EQUAL(LocCode("FRA"), srm->_surchargeStore.begin()->second);

    // second surcharge SIN-NYC
    CPPUNIT_ASSERT_EQUAL(LocCode("SIN"), (srm->_surchargeStore.begin() + 1)->first);
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), (srm->_surchargeStore.begin() + 1)->second);
  }

  void testAddSurchargeShouldSetCurrencyUSDAmountTruncatedAPO40780()
  {
    SurchargeData* srd1 = 0;
    _trx->dataHandle().get(srd1);
    _fInfo->_fareAmount = 68.84 ;
    _fInfo->_originalFareAmount = 68.84 ;
    _fInfo->_noDec = 2;
    _fInfo->_currency = "USD";
    _sInfo->surchargePercent() = 15;
    _fInfo->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    _sInfo->tvlPortion() = RuleConst::ONEWAY;

    if (srd1 != 0)
    {
      _srr->addSurcharge(*_trx, _fPath, *_ptf, _fUsage, *_sInfo, srd1, *_seg, "AAA", "BBB", true);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("USD"), std::string(srd1->currSelected()));
    CPPUNIT_ASSERT_EQUAL(10.32, srd1->amountSelected());
  }

  RuleUtil::TravelSegWrapperVector getTswVector()
  {
    RuleUtil::TravelSegWrapperVector segmetns;

    segmetns.push_back(makeTsw("SYD", "SIN"));
    segmetns.push_back(makeTsw("SIN", "FRA"));
    segmetns.push_back(makeTsw("FRA", "NYC"));

    return segmetns;
  }

  SurchargesRule::travelSegVector getTravelVector()
  {
    SurchargesRule::travelSegVector segmetns;
    segmetns.push_back(createAS("SYD", "SIN", 1));
    segmetns.push_back(createAS("SIN", "FRA", 2));
    segmetns.push_back(createAS("FRA", "NYC", 3));

    return segmetns;
  }

  RuleUtil::TravelSegWrapper* makeTsw(LocCode boardMultiCity, LocCode offMultiCity)
  {
    RuleUtil::TravelSegWrapper* tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
    tsw->travelSeg() = _memHandle.create<AirSeg>();
    tsw->travelSeg()->boardMultiCity() = boardMultiCity;
    tsw->travelSeg()->offMultiCity() = offMultiCity;
    return tsw;
  }

  void initRbd(BookingCode bc, BookingCode b1, BookingCode b2)
  {
    _sInfo->bookingCode() = bc;
    _sInfo->surchargeType() = RuleConst::RBD;
    RuleUtil::TravelSegWrapper* tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
    AirSeg* s = createAS("AAA", "BBB", 1);
    s->setBookingCode(b1);
    tsw->travelSeg() = s;
    _applTravelSegment->push_back(tsw);
    tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
    s = createAS("BBB", "CCC", 2);
    s->setBookingCode(b2);
    tsw->travelSeg() = s;
    _applTravelSegment->push_back(tsw);
  }

  void initT986Data(int t986)
  {
    _sInfo->carrierFltTblItemNo() = t986;
    RuleUtil::TravelSegWrapper* tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
    tsw->travelSeg() = createAS("AAA", "BBB", 1, "AA", "BB");
    _applTravelSegment->push_back(tsw);
    tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
    tsw->travelSeg() = createAS("BBB", "CCC", 2, "BB", "AA");
    _applTravelSegment->push_back(tsw);
    _memHandle.create<MyDataHandle>();
  }

  AirSeg* createAS(LocCode loc1, LocCode loc2, uint16_t so, CarrierCode ma, CarrierCode op)
  {
    AirSeg* s = createAS(loc1, loc2, so);
    s->setMarketingCarrierCode(ma);
    s->setOperatingCarrierCode(op);
    s->flightNumber() = so * 100;
    return s;
  }

  AirSeg* createAS(LocCode loc1, LocCode loc2, uint16_t so)
  {
    AirSeg* s = _memHandle.create<AirSeg>();
    Loc* l1 = _memHandle.create<Loc>();
    l1->loc() = loc1;
    Loc* l2 = _memHandle.create<Loc>();
    l2->loc() = loc2;
    s->origin() = l1;
    s->destination() = l2;
    s->origAirport() = loc1;
    s->destAirport() = loc2;
    s->segmentOrder() = so;

    s->boardMultiCity() = loc1;
    s->offMultiCity() = loc2;
    return s;
  }

  void initSecPorSegs(char sp, int btw, int aand)
  {
    _sInfo->sectorPortion() = sp;
    _sInfo->geoTblItemNoBtw() = btw;
    _sInfo->geoTblItemNoAnd() = aand;
    _fM->travelSeg() += createAS("AAA", "BBB", 1), createAS("BBB", "CCC", 2),
        createAS("CCC", "DDD", 3), createAS("DDD", "EEE", 4);
    //, createAS("AAA", "CCC", 5), createAS("CCC", "AAA", 6),
    // createAS("AAA", "BBB", 7), createAS("EEE", "CCC", 8), createAS("CCC", "EEE", 9);
    _itin->travelSeg() = _fM->travelSeg();
    _memHandle.create<MyDataHandle>();
  }

private:
  SurchargesInfo* _sInfo;
  PricingTrx* _trx;
  FarePath* _fPath;
  PricingRequest* _req;
  Itin* _itin;
  FareInfo* _fInfo;
  Fare* _fare;
  FareUsage* _fUsage;
  PaxTypeFare* _ptf;
  FareMarket* _fM;
  AirSeg* _seg;
  TestMemHandle _memHandle;
  SurchargesRule* _srr;
  PricingUnit* _pu;
  RuleUtil::TravelSegWrapperVector* _applTravelSegment;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SurchargesRuleTest);
}
