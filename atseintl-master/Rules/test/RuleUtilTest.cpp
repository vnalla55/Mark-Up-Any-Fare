#include "test/include/CppUnitHelperMacros.h"
#include <set>
#include <vector>

#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Tours.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "DBAccess/FareFocusDaytimeApplInfo.h"
#include "DBAccess/FareFocusDaytimeApplDetailInfo.h"

using namespace tse;

class MyDataHandle : public DataHandleMock
{
  ZoneInfo _z1;
  Loc _loc;
  FareFocusBookingCodeInfo _ffbci;
  std::vector<FareFocusBookingCodeInfo*> _ffbciV;

public:

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    _loc.loc() = locCode;
    _loc.nation() = "US";
    return &_loc;
  }

  const ZoneInfo* getZoneFareFocusGroup(const VendorCode& vendor,
                                        const Zone& zone,
                                        Indicator zoneType,
                                        const DateTime& date,
                                        bool fareFocusGroup)
  {
    if (zone == "CA")
      return 0;

    _z1.zone() = "US";
    _z1.zoneType() = 'C';

    ZoneInfo::ZoneSeg zoneSeg;
    zoneSeg.loc() = "US";
    zoneSeg.locType() = 'C';
    zoneSeg.inclExclInd() = 'I';

    std::vector<ZoneInfo::ZoneSeg> v;
    v.push_back(zoneSeg);

    _z1.sets().push_back(v);

    return &_z1;
  }

  const std::vector<FareFocusBookingCodeInfo*>& getFareFocusBookingCode(uint64_t bookingCodeItemNo,
                                                                        DateTime adjustedTicketDate)
  {
    _ffbciV.clear();

    if (bookingCodeItemNo == 999)
      return _ffbciV;

    _ffbci.bookingCode().push_back("C");
    _ffbciV.push_back(&_ffbci);

    return _ffbciV;
  }
};

class RuleUtilTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(RuleUtilTest);

  CPPUNIT_TEST(testCheckMatchCriteria1);
  CPPUNIT_TEST(testCheckMatchCriteria2);
  CPPUNIT_TEST(testCheckMatchCriteria3);
  CPPUNIT_TEST(testCheckMatchCriteria4);
  CPPUNIT_TEST(testCheckMatchCriteria5);
  CPPUNIT_TEST(testCheckMatchCriteria6);
  CPPUNIT_TEST(testCheckMatchCriteria7);
  CPPUNIT_TEST(testCheckMatchCriteria8);
  CPPUNIT_TEST(testCheckMatchCriteria9);
  CPPUNIT_TEST(testCheckMatchCriteria11);
  CPPUNIT_TEST(testCheckMatchCriteria12);
  CPPUNIT_TEST(testCheckMatchCriteria13);
  CPPUNIT_TEST(testCheckMatchCriteria14);
  CPPUNIT_TEST(testCheckMatchCriteria16);
  CPPUNIT_TEST(testCheckMatchCriteria17);
  CPPUNIT_TEST(testCheckMatchCriteria18);
  CPPUNIT_TEST(testCheckMatchCriteria19);
  CPPUNIT_TEST(testCheckMatchCriteria20);
  CPPUNIT_TEST(testCheckMatchCriteria21);
  CPPUNIT_TEST(testCheckMatchCriteria22);

  CPPUNIT_TEST(testMatchFareFootnote1);
  CPPUNIT_TEST(testMatchFareFootnote2);
  CPPUNIT_TEST(testMatchFareFootnote3);
  CPPUNIT_TEST(testMatchFareFootnote4);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch1);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch2);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch3);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch4);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch5);
  CPPUNIT_TEST(testFareValidForRetailerCodeQualiferMatch6);
  CPPUNIT_TEST(testMatchFareClass);

  CPPUNIT_TEST(testMatchLocation_Fail);
  CPPUNIT_TEST(testMatchLocation_Pass_AnyLoc);
  CPPUNIT_TEST(testMatchLocation_Pass_NotConstructed);

  CPPUNIT_TEST(testMatchOneWayRoundTrip);
  CPPUNIT_TEST(testMatchFareType1);
  CPPUNIT_TEST(testMatchFareType2);
  CPPUNIT_TEST(testMatchFareType3);
  CPPUNIT_TEST(testMatchGenericFareType1);
  CPPUNIT_TEST(testMatchGenericFareType2);
  CPPUNIT_TEST(testMatchGenericFareType3);
  CPPUNIT_TEST(testMatchGenericFareType4);
  CPPUNIT_TEST(testMatchGenericFareType5);
  CPPUNIT_TEST(testMatchGenericFareType6);
  CPPUNIT_TEST(testMatchGenericFareType7);
  CPPUNIT_TEST(testMatchSeasons1);
  CPPUNIT_TEST(testMatchSeasons2);
  CPPUNIT_TEST(testMatchSeasons3);
  CPPUNIT_TEST(testMatchDayOfWeek1);
  CPPUNIT_TEST(testMatchDayOfWeek2);
  CPPUNIT_TEST(testMatchDayOfWeek3);
  CPPUNIT_TEST(testMatchFareClass2);

  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_S_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_S_Fail);
  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_R_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_R_Fail);
  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_M_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_Processed_M_Fail);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_S_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_S_Fail);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_R_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_R_Fail);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_M_Pass);
  CPPUNIT_TEST(testMatchFareRouteNumber_NotProcessed_M_Fail);

  CPPUNIT_TEST(testMatchTravelRangeX5);
  CPPUNIT_TEST(testAddPeriodToDate_Minutes);
  CPPUNIT_TEST(testAddPeriodToDate_Hours);
  CPPUNIT_TEST(testAddPeriodToDate_Days);
  CPPUNIT_TEST(testAddPeriodToDate_MonthsAddDaysExcpt);
  CPPUNIT_TEST(testAddPeriodToDate_MonthsSubtractDaysExcpt);

  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnFalseWhenNoDutyCodeInAgent);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnTrueWhenDutyCodeMatchAndDBFunctionCodeLengthOne);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnFalseWhenNoMatchAndAgentDutyCode0To9);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnFalseWhenDBDutyAAndAgentDutyNotDollar);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnTrueWhenDBDutyAAndAgentDutyDollar);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnFalseAgentFunctionEmpty);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnFalseAgentFunctionDoesNotMatch);
  CPPUNIT_TEST(testValidateDutyFunctionCodeReturnTrueWhenAgentFunctionMatch);

  CPPUNIT_TEST(testGetCat27TourCodeFareNull);
  CPPUNIT_TEST(testGetCat27TourCodeNoTourCode);
  CPPUNIT_TEST(testGetCat27TourCodeTourCode);
  CPPUNIT_TEST(testGetCat27TourCodeTourCodeEmpty);

  CPPUNIT_TEST(testGetCat27TourCodeNoFBR);
  CPPUNIT_TEST(testGetCat27TourCodeFBRResultingFare);
  CPPUNIT_TEST(testGetCat27TourCodeFBRBaseFare);
  CPPUNIT_TEST(testGetCat27TourCodeFBROtherInd);

  CPPUNIT_TEST(testGetFootnotes_ConstructedATP_omittedFootnotes);
  CPPUNIT_TEST(testGetFootnotes_ConstructedATP_collectedFootnotes);
  CPPUNIT_TEST(testGetFootnotes_ConstructedOther);

  CPPUNIT_TEST(testValidateMatchExpression);
  CPPUNIT_TEST(testIsInFFUserGroup);
  CPPUNIT_TEST(testMatchFareClassExpression);
  CPPUNIT_TEST(testIsInFFUserGroup);
  CPPUNIT_TEST(testIsInLoc);
  CPPUNIT_TEST(testMatchPrimeRBD_True);
  CPPUNIT_TEST(testMatchPrimeRBD_False);
  CPPUNIT_TEST(testGetPrimeRBDrec1);
  CPPUNIT_TEST(testMatchBookingCode);
  CPPUNIT_TEST(testMatchOWRT_True_Rule_Blank);
  CPPUNIT_TEST(testMatchOWRT_True);
  CPPUNIT_TEST(testMatchOWRT_False);

  CPPUNIT_TEST(testMatchCat35Type);
  CPPUNIT_TEST(testGetPrimeRBD_Booking);
  CPPUNIT_TEST(testGetPrimeRBD_Booking_Empty);
  CPPUNIT_TEST(testGetPrimeRBDrec1_Booking);
  CPPUNIT_TEST(testGetPrimeRBDrec1_FBR_Specified_Booking_Empty);
  CPPUNIT_TEST(testGetPrimeRBDrec1_FBR_Specified_Booking);
  CPPUNIT_TEST(testGetPrimeRBDrec1_FBR_Calculated_Base_Booking);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_request);
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _fbrItemInfo = _memHandle.create<FareByRuleItemInfo>();
    _fbrRuleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    _ffdtda      = new FareFocusDaytimeApplDetailInfo();
    _ffdta       = _memHandle.create<FareFocusDaytimeApplInfo>();
    std::vector<FareFocusDaytimeApplDetailInfo*> vec;
    vec.push_back(_ffdtda);
    _ffdta->details()= vec;
    _frri        = new FareRetailerRuleInfo();

    _negRuleData = new NegPaxTypeFareRuleData();
    _categRuleItemInfo = new CategoryRuleItemInfo();
  
    sfo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    dal = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDAL.xml");
    sjc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJC.xml");
    jfk = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    nyc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    bos = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    lax = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    iah = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocIAH.xml");
    mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    yvr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    hnl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHNL.xml");
    stt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSTT.xml");
    anc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocANC.xml");
    sju = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJU.xml");
    cdg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");
    mex = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    tul = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
    man = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMAN.xml");
    pap = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAP.xml");
    yvi = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVI.xml");
    hav = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHAV.xml");
  }

  void tearDown() { _memHandle.clear(); }

  PaxTypeFare& createPTF(
      const Loc* orig,
      const Loc* dest,
      const LocCode& gateway1,
      const LocCode& gateway2,
      bool isConstructed = true,
      ConstructedFareInfo::ConstructionType constructionType = ConstructedFareInfo::DOUBLE_ENDED,
      Directionality directionality = BOTH,
      const char* origAddonFootNote1 = "",
      const char* origAddonFootNote2 = "a1",
      const char* destAddonFootNote1 = "a2",
      const char* destAddonFootNote2 = "",
      const char* vendor = "ATP")
  {
    PaxTypeFare* ptf = _memHandle.insert(new PaxTypeFare);
    Fare* fare = _memHandle.insert(new Fare);
    ConstructedFareInfo* fareInfo = _memHandle.insert(new ConstructedFareInfo);
    FareMarket* fareMarket = _memHandle.insert(new FareMarket);
    _adjustedSellingCalcData = _memHandle.insert(new AdjustedSellingCalcData()); 

    fareMarket->origin() = orig;
    fareMarket->destination() = dest;

    // Setup fare info
    fareInfo->constructionType() = constructionType;
    fareInfo->fareInfo().directionality() = directionality;
    fareInfo->fareInfo().market1() = orig->loc();
    fareInfo->fareInfo().market2() = dest->loc();
    fareInfo->gateway1() = gateway1;
    fareInfo->gateway2() = gateway2;
    fareInfo->fareInfo().vendor() = "ATP";
    fareInfo->origAddonTariff() = 20;
    fareInfo->fareInfo().fareTariff() = 30;
    fareInfo->destAddonTariff() = 40;
    fareInfo->fareInfo().footNote1() = "s1";
    fareInfo->fareInfo().footNote2() = "";
    fareInfo->origAddonFootNote1() = origAddonFootNote1;
    fareInfo->origAddonFootNote2() = origAddonFootNote2;
    fareInfo->destAddonFootNote1() = destAddonFootNote1;
    fareInfo->destAddonFootNote2() = destAddonFootNote2;
    fareInfo->fareInfo().vendor() = vendor;

    fare->initialize(isConstructed ? Fare::FS_ConstructedFare : Fare::FS_PublishedFare,
                     &(fareInfo->fareInfo()),
                     *fareMarket,
                     0,
                     fareInfo);
    ptf->initialize(fare, 0, fareMarket);

    return *ptf;
  }

  void setTravelDateRange(DateTime dateRangeStart,DateTime dateRangeStop)
  {
    _ffdtda->startDate()=dateRangeStart;
    _ffdtda->stopDate()=dateRangeStop;
  }

  void setFBR(PaxTypeFare& fare, bool specified)
  {
    if (specified)
      _fbrItemInfo->fareInd() = 'S'; //SPECIFIED
    else
      _fbrItemInfo->fareInd() = 'C'; //calc
    _fbrRuleData->ruleItemInfo() = _fbrItemInfo;

    PaxTypeFare::PaxTypeFareAllRuleData* data =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    data->fareRuleData = _fbrRuleData;
    (*fare.paxTypeFareRuleDataMap())[25] = data;
  }

  PaxTypeFare* createPaxTypeFare(const std::string& carrier,
                                 const std::string& fareClass,
                                 const std::string& rule,
                                 const TariffNumber ruleTariff,
                                 const TariffCategory tCat,
                                 const std::string& fareType,
                                 const std::string& vendor,
                                 const Indicator  cat35,
                                 const std::string& bookingCode)
  {

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_carrier = carrier;
    fi->_fareClass = fareClass;
    fi->_ruleNumber = "0100";
    fi->fareAmount() = 12.34;
    fi->_currency = "USD";
    fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    fi->ruleNumber() = rule;
    fi->vendor() = vendor;
    Fare* fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = 12.34;
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();

    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    fcasi->_bookingCode[0] = bookingCode;
    ptf->fareClassAppSegInfo() = fcasi;

    FareClassAppInfo* fca = _memHandle.create<FareClassAppInfo>();
    fca->_fareType = fareType;
    fca->_displayCatType = cat35;
    ptf->fareClassAppInfo() = fca;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();
    tcr->ruleTariff() = ruleTariff;
    tcr->tariffCat() = tCat;
    fare->setTariffCrossRefInfo(tcr);

    ptf->setFare(fare);
    return ptf;
  }

  LocKey& createLocKey(LocTypeCode type, const LocCode& loc)
  {
    LocKey* locKey = _memHandle.insert(new LocKey);
    locKey->locType() = type;
    locKey->loc() = loc;

    return *locKey;
  };

  /// Test StopOver - Journey scope

  void testCheckMatchCriteria1()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = true;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;
    syd_hkg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_hkg);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;
    tsm_mel_syd.isStopover() = true;

    RuleUtil::TSITravelSegMarkup tsm_syd_hkg;
    tsm_syd_hkg.travelSeg() = &syd_hkg;
    tsm_syd_hkg.direction() = RuleConst::OUTBOUND;
    tsm_syd_hkg.fareBreakAtDestination() = true;
    tsm_syd_hkg.isTurnAroundPoint() = true;
    tsm_syd_hkg.destIsTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::STOP_OVER));

    std::string reason;

    //
    // MEL-SYD is a stop-over
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 1A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-HKG is not a stop-over but HKG is the turn around point so
    //  it is automatically considered a stopover match
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 1B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_hkg, mc, reason, *_trx));
  }

  /// Test StopOver - SubJourney scope

  void testCheckMatchCriteria2()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = true;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;
    syd_hkg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_hkg);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;
    tsm_mel_syd.isStopover() = true;

    RuleUtil::TSITravelSegMarkup tsm_syd_hkg;
    tsm_syd_hkg.travelSeg() = &syd_hkg;
    tsm_syd_hkg.direction() = RuleConst::OUTBOUND;
    tsm_syd_hkg.fareBreakAtDestination() = true;
    tsm_syd_hkg.isTurnAroundPoint() = true;
    tsm_syd_hkg.destIsTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::STOP_OVER));

    std::string reason;

    //
    // MEL-SYD is a stop-over
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 2A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-HKG is a not stop-over but HKG is the turn around point so
    //  it is automatically considered a stopover match
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 2B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_hkg, mc, reason, *_trx));
  }

  /// Test StopOver - FareComponent scope

  void testCheckMatchCriteria3()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;
    syd_hkg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_hkg);

    // Create a fare market
    FareMarket fm;

    // Attach the travel segments to the fare market
    fm.travelSeg().push_back(&mel_syd);
    fm.travelSeg().push_back(&syd_hkg);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_hkg;
    tsm_syd_hkg.travelSeg() = &syd_hkg;
    tsm_syd_hkg.direction() = RuleConst::OUTBOUND;
    tsm_syd_hkg.fareBreakAtDestination() = true;
    tsm_syd_hkg.isTurnAroundPoint() = true;
    tsm_syd_hkg.destIsTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, 0, 0, 0, &fm);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::STOP_OVER));

    std::string reason;

    //
    // MEL-SYD is a stop-over, but it does not matter because
    //  stop-over will never match for fare component scope
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 3A",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 3B",
                           std::string::npos != reason.find(RuleConst::MATCH_STOP_OVER_DESC));

    //
    // SYD-HKG is a not stop-over but HKG is the turn around point so
    //  it is automatically considered a stopover match
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 3C",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_hkg, mc, reason, *_trx));
  }

  /// Test Inbound - Journey scope

  void testCheckMatchCriteria4()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INBOUND));

    std::string reason;

    //
    // SYD-MEL is an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 4A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_mel, mc, reason, *_trx));

    //
    // MEL-SYD is not an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 4B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 4C",
                           std::string::npos != reason.find(RuleConst::MATCH_INBOUND_DESC));
  }

  /// Test Inbound - SubJourney scope

  void testCheckMatchCriteria5()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INBOUND));

    std::string reason;

    //
    // SYD-MEL is an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 5A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_mel, mc, reason, *_trx));

    //
    // MEL-SYD is not an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 5B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 5C",
                           std::string::npos != reason.find(RuleConst::MATCH_INBOUND_DESC));
  }

  /// Test Inbound - FareComponent scope

  void testCheckMatchCriteria6()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    // Create the fare markets
    FareMarket fm1;
    FareMarket fm2;

    fm1.direction() = FMDirection::OUTBOUND;
    fm2.direction() = FMDirection::INBOUND;

    // Attach the travel segments to the fare markets
    fm1.travelSeg().push_back(&mel_syd);
    fm2.travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData1(tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, 0, 0, 0, &fm1);

    RuleUtil::TSIData tsiData2(tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, 0, 0, 0, &fm2);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INBOUND));

    std::string reason;

    //
    // SYD-MEL is an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 6A",
                           RuleUtil::checkMatchCriteria(tsiData2, tsm_syd_mel, mc, reason, *_trx));

    //
    // MEL-SYD is not an inbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 6B",
                           !RuleUtil::checkMatchCriteria(tsiData1, tsm_mel_syd, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 6C",
                           std::string::npos != reason.find(RuleConst::MATCH_INBOUND_DESC));
  }

  /// Test Outbound - Journey scope

  void testCheckMatchCriteria7()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::OUTBOUND));

    std::string reason;

    //
    // MEL-SYD is an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 7A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-MEL is not an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 7B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_syd_mel, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 7C",
                           std::string::npos != reason.find(RuleConst::MATCH_OUTBOUND_DESC));
  }

  /// Test Outbound - SubJourney scope

  void testCheckMatchCriteria8()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::OUTBOUND));

    std::string reason;

    //
    // MEL-SYD is an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 8A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-MEL is not an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 8B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_syd_mel, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 8C",
                           std::string::npos != reason.find(RuleConst::MATCH_OUTBOUND_DESC));
  }

  /// Test Outbound - FareComponent scope

  void testCheckMatchCriteria9()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = false;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 2;
    syd_mel.segmentOrder() = 2;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_mel);

    // Create the fare markets
    FareMarket fm1;
    FareMarket fm2;

    fm1.direction() = FMDirection::OUTBOUND;
    fm2.direction() = FMDirection::INBOUND;

    // Attach the travel segments to the fare markets
    fm1.travelSeg().push_back(&mel_syd);
    fm2.travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData1(tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, 0, 0, 0, &fm1);

    RuleUtil::TSIData tsiData2(tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, 0, 0, 0, &fm2);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::OUTBOUND));

    std::string reason;

    //
    // MEL-SYD is an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 9A",
                           RuleUtil::checkMatchCriteria(tsiData1, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-MEL is not an outbound segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 9B",
                           !RuleUtil::checkMatchCriteria(tsiData2, tsm_syd_mel, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 9C",
                           std::string::npos != reason.find(RuleConst::MATCH_OUTBOUND_DESC));
  }

  /// Test Domestic - This match is not sensitive to scope

  void testCheckMatchCriteria11()
  {
    // Create the travel segments
    //
    AirSeg yyz_dfw;
    yyz_dfw.pnrSegment() = 1;
    yyz_dfw.segmentOrder() = 1;
    yyz_dfw.origin() = yyz;
    yyz_dfw.destination() = dfw;
    yyz_dfw.stopOver() = false;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;
    dfw_jfk.stopOver() = false;

    AirSeg jfk_lhr;
    jfk_lhr.pnrSegment() = 3;
    jfk_lhr.segmentOrder() = 3;
    jfk_lhr.origin() = jfk;
    jfk_lhr.destination() = lhr;
    jfk_lhr.stopOver() = false;

    AirSeg lhr_jfk;
    lhr_jfk.pnrSegment() = 4;
    lhr_jfk.segmentOrder() = 4;
    lhr_jfk.origin() = lhr;
    lhr_jfk.destination() = jfk;
    lhr_jfk.stopOver() = false;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;
    jfk_dfw.stopOver() = false;

    AirSeg dfw_yyz;
    dfw_yyz.pnrSegment() = 6;
    dfw_yyz.segmentOrder() = 6;
    dfw_yyz.origin() = dfw;
    dfw_yyz.destination() = yyz;
    dfw_yyz.stopOver() = false;

    // Miami to St Thomas (Virgin Islands)
    AirSeg mia_stt;
    mia_stt.pnrSegment() = 1;
    mia_stt.segmentOrder() = 1;
    mia_stt.origin() = mia;
    mia_stt.destination() = stt;
    mia_stt.stopOver() = false;

    // Miami to San Juan (Puerto Rico)
    AirSeg mia_sju;
    mia_sju.pnrSegment() = 1;
    mia_sju.segmentOrder() = 1;
    mia_sju.origin() = mia;
    mia_sju.destination() = sju;
    mia_sju.stopOver() = false;

    // Los Angeles to Honalulu
    AirSeg lax_hnl;
    lax_hnl.pnrSegment() = 1;
    lax_hnl.segmentOrder() = 1;
    lax_hnl.origin() = lax;
    lax_hnl.destination() = hnl;
    lax_hnl.stopOver() = false;

    // San Francisco to Anchorage
    AirSeg sfo_anc;
    sfo_anc.pnrSegment() = 1;
    sfo_anc.segmentOrder() = 1;
    sfo_anc.origin() = sfo;
    sfo_anc.destination() = anc;
    sfo_anc.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&yyz_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_lhr);
    _trx->travelSeg().push_back(&lhr_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_yyz);
    _trx->travelSeg().push_back(&lax_hnl);
    _trx->travelSeg().push_back(&sfo_anc);

    RuleUtil::TSITravelSegMarkup tsm_yyz_dfw;
    tsm_yyz_dfw.travelSeg() = &yyz_dfw;
    tsm_yyz_dfw.direction() = RuleConst::OUTBOUND;
    tsm_yyz_dfw.fareBreakAtDestination() = true;
    tsm_yyz_dfw.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_dfw_jfk;
    tsm_dfw_jfk.travelSeg() = &dfw_jfk;
    tsm_dfw_jfk.direction() = RuleConst::OUTBOUND;
    tsm_dfw_jfk.fareBreakAtDestination() = true;
    tsm_dfw_jfk.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_jfk_lhr;
    tsm_jfk_lhr.travelSeg() = &jfk_lhr;
    tsm_jfk_lhr.direction() = RuleConst::OUTBOUND;
    tsm_jfk_lhr.fareBreakAtDestination() = true;
    tsm_jfk_lhr.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_lhr_jfk;
    tsm_lhr_jfk.travelSeg() = &lhr_jfk;
    tsm_lhr_jfk.direction() = RuleConst::INBOUND;
    tsm_lhr_jfk.fareBreakAtDestination() = true;
    tsm_lhr_jfk.isTurnAroundPoint() = true;

    RuleUtil::TSITravelSegMarkup tsm_jfk_dfw;
    tsm_jfk_dfw.travelSeg() = &jfk_dfw;
    tsm_jfk_dfw.direction() = RuleConst::INBOUND;
    tsm_jfk_dfw.fareBreakAtDestination() = true;
    tsm_jfk_dfw.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_dfw_yyz;
    tsm_dfw_yyz.travelSeg() = &dfw_yyz;
    tsm_dfw_yyz.direction() = RuleConst::INBOUND;
    tsm_dfw_yyz.fareBreakAtDestination() = true;
    tsm_dfw_yyz.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_mia_stt;
    tsm_mia_stt.travelSeg() = &mia_stt;
    tsm_mia_stt.direction() = RuleConst::OUTBOUND;
    tsm_mia_stt.fareBreakAtDestination() = true;
    tsm_mia_stt.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_mia_sju;
    tsm_mia_sju.travelSeg() = &mia_sju;
    tsm_mia_sju.direction() = RuleConst::OUTBOUND;
    tsm_mia_sju.fareBreakAtDestination() = true;
    tsm_mia_sju.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_lax_hnl;
    tsm_lax_hnl.travelSeg() = &lax_hnl;
    tsm_lax_hnl.direction() = RuleConst::OUTBOUND;
    tsm_lax_hnl.fareBreakAtDestination() = true;
    tsm_lax_hnl.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_sfo_anc;
    tsm_sfo_anc.travelSeg() = &sfo_anc;
    tsm_sfo_anc.direction() = RuleConst::OUTBOUND;
    tsm_sfo_anc.fareBreakAtDestination() = true;
    tsm_sfo_anc.isTurnAroundPoint() = false;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::DOMESTIC));

    std::string reason;

    //
    // DFW-JFK is domestic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_jfk, mc, reason, *_trx));

    //
    // MIA-STT is domestic (Continental US <-> Virgin Islands)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mia_stt, mc, reason, *_trx));

    //
    // MIA-SJU is domestic (Continental US <-> Puerto Rico)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11C",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mia_sju, mc, reason, *_trx));

    //
    // LAX-HNL is domestic (Continental US <-> Hawaii)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11D",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_lax_hnl, mc, reason, *_trx));

    //
    // SFO-ANC is domestic (Continental US <-> Alaska)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11E",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_sfo_anc, mc, reason, *_trx));

    //
    // JFK-LHR is not domestic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11F",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_jfk_lhr, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11G",
                           std::string::npos != reason.find(RuleConst::MATCH_DOMESTIC_DESC));

    //
    // YYZ-DFW is not domestic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11H",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_yyz_dfw, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 11I",
                           std::string::npos != reason.find(RuleConst::MATCH_DOMESTIC_DESC));
  }

  /// Test OneCountry - This match is not sensitive to scope

  void testCheckMatchCriteria12()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = true;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;
    syd_hkg.stopOver() = false;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;
    hkg_syd.stopOver() = true;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_hkg);
    _trx->travelSeg().push_back(&hkg_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_hkg;
    tsm_syd_hkg.travelSeg() = &syd_hkg;
    tsm_syd_hkg.direction() = RuleConst::OUTBOUND;
    tsm_syd_hkg.fareBreakAtDestination() = true;
    tsm_syd_hkg.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_hkg_syd;
    tsm_hkg_syd.travelSeg() = &hkg_syd;
    tsm_hkg_syd.direction() = RuleConst::INBOUND;
    tsm_hkg_syd.fareBreakAtDestination() = true;
    tsm_hkg_syd.isTurnAroundPoint() = true;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = false;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::ONE_COUNTRY));

    std::string reason;

    //
    // MEL-SYD is one country
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 12A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    //
    // SYD-HKG is not one country
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 12B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_syd_hkg, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 12C",
                           std::string::npos != reason.find(RuleConst::MATCH_ONE_COUNTRY_DESC));
  }

  /// Test International - This match is not sensitive to scope

  void testCheckMatchCriteria13()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;
    mel_syd.stopOver() = true;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;
    syd_hkg.stopOver() = false;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;
    hkg_syd.stopOver() = true;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;
    syd_mel.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mel_syd);
    _trx->travelSeg().push_back(&syd_hkg);
    _trx->travelSeg().push_back(&hkg_syd);
    _trx->travelSeg().push_back(&syd_mel);

    RuleUtil::TSITravelSegMarkup tsm_mel_syd;
    tsm_mel_syd.travelSeg() = &mel_syd;
    tsm_mel_syd.direction() = RuleConst::OUTBOUND;
    tsm_mel_syd.fareBreakAtDestination() = true;
    tsm_mel_syd.isTurnAroundPoint() = false;
    tsm_mel_syd.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_syd_hkg;
    tsm_syd_hkg.travelSeg() = &syd_hkg;
    tsm_syd_hkg.direction() = RuleConst::OUTBOUND;
    tsm_syd_hkg.fareBreakAtDestination() = true;
    tsm_syd_hkg.isTurnAroundPoint() = false;
    tsm_syd_hkg.isInternational() = true;

    RuleUtil::TSITravelSegMarkup tsm_hkg_syd;
    tsm_hkg_syd.travelSeg() = &hkg_syd;
    tsm_hkg_syd.direction() = RuleConst::INBOUND;
    tsm_hkg_syd.fareBreakAtDestination() = true;
    tsm_hkg_syd.isTurnAroundPoint() = true;
    tsm_hkg_syd.isInternational() = true;

    RuleUtil::TSITravelSegMarkup tsm_syd_mel;
    tsm_syd_mel.travelSeg() = &syd_mel;
    tsm_syd_mel.direction() = RuleConst::INBOUND;
    tsm_syd_mel.fareBreakAtDestination() = true;
    tsm_syd_mel.isTurnAroundPoint() = false;
    tsm_syd_mel.isInternational() = false;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INTERNATIONAL));

    std::string reason;

    //
    // SYD-HKG is international
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 13A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_syd_hkg, mc, reason, *_trx));

    //
    // MEL-SYD is not international
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 13B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mel_syd, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 13C",
                           std::string::npos != reason.find(RuleConst::MATCH_INTERNATIONAL_DESC));
  }

  /// Test International - Continental US to Canada/Puerto Rico/Virgin Islands

  void testCheckMatchCriteria14()
  {
    // Create the travel segments
    //
    AirSeg stt_mia;
    stt_mia.pnrSegment() = 1;
    stt_mia.segmentOrder() = 1;
    stt_mia.origin() = stt;
    stt_mia.destination() = mia;
    stt_mia.stopOver() = false;

    AirSeg mia_sju;
    mia_sju.pnrSegment() = 2;
    mia_sju.segmentOrder() = 2;
    mia_sju.origin() = mia;
    mia_sju.destination() = sju;
    mia_sju.stopOver() = false;

    AirSeg sju_dfw;
    sju_dfw.pnrSegment() = 3;
    sju_dfw.segmentOrder() = 3;
    sju_dfw.origin() = sju;
    sju_dfw.destination() = dfw;
    sju_dfw.stopOver() = false;

    AirSeg dfw_yyz;
    dfw_yyz.pnrSegment() = 4;
    dfw_yyz.segmentOrder() = 4;
    dfw_yyz.origin() = dfw;
    dfw_yyz.destination() = yyz;
    dfw_yyz.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&stt_mia);
    _trx->travelSeg().push_back(&mia_sju);
    _trx->travelSeg().push_back(&sju_dfw);
    _trx->travelSeg().push_back(&dfw_yyz);

    RuleUtil::TSITravelSegMarkup tsm_stt_mia;
    tsm_stt_mia.travelSeg() = &stt_mia;
    tsm_stt_mia.direction() = RuleConst::OUTBOUND;
    tsm_stt_mia.fareBreakAtDestination() = true;
    tsm_stt_mia.isTurnAroundPoint() = false;
    tsm_stt_mia.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_mia_sju;
    tsm_mia_sju.travelSeg() = &mia_sju;
    tsm_mia_sju.direction() = RuleConst::OUTBOUND;
    tsm_mia_sju.fareBreakAtDestination() = true;
    tsm_mia_sju.isTurnAroundPoint() = false;
    tsm_mia_sju.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_sju_dfw;
    tsm_sju_dfw.travelSeg() = &sju_dfw;
    tsm_sju_dfw.direction() = RuleConst::OUTBOUND;
    tsm_sju_dfw.fareBreakAtDestination() = true;
    tsm_sju_dfw.isTurnAroundPoint() = false;
    tsm_sju_dfw.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_dfw_yyz;
    tsm_dfw_yyz.travelSeg() = &dfw_yyz;
    tsm_dfw_yyz.direction() = RuleConst::OUTBOUND;
    tsm_dfw_yyz.fareBreakAtDestination() = true;
    tsm_dfw_yyz.isTurnAroundPoint() = false;
    tsm_dfw_yyz.isInternational() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INTERNATIONAL));

    std::string reason;

    //
    // MIA-SJU is not considered international (Cont US <-> Puerto Rico)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 14A",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mia_sju, mc, reason, *_trx));

    //
    // STT-MIA is not considered international (Cont US <-> Virgin Islands)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 14B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_stt_mia, mc, reason, *_trx));

    //
    // MIA-YYZ is considered international (US <-> Canada)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 14C",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_yyz, mc, reason, *_trx));
  }

  /// Test Gateway - This match is not sensitive to scope

  void testCheckMatchCriteria16()
  {
    // Create the travel segments
    //
    AirSeg mex_dfw;
    mex_dfw.pnrSegment() = 1;
    mex_dfw.segmentOrder() = 1;
    mex_dfw.origin() = mex;
    mex_dfw.destination() = dfw;
    mex_dfw.stopOver() = false;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;
    dfw_jfk.stopOver() = false;

    AirSeg jfk_lhr;
    jfk_lhr.pnrSegment() = 3;
    jfk_lhr.segmentOrder() = 3;
    jfk_lhr.origin() = jfk;
    jfk_lhr.destination() = lhr;
    jfk_lhr.stopOver() = false;

    AirSeg lhr_cdg;
    lhr_cdg.pnrSegment() = 4;
    lhr_cdg.segmentOrder() = 4;
    lhr_cdg.origin() = lhr;
    lhr_cdg.destination() = cdg;
    lhr_cdg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mex_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_lhr);
    _trx->travelSeg().push_back(&lhr_cdg);

    RuleUtil::TSITravelSegMarkup tsm_mex_dfw;
    tsm_mex_dfw.travelSeg() = &mex_dfw;
    tsm_mex_dfw.direction() = RuleConst::OUTBOUND;
    tsm_mex_dfw.fareBreakAtDestination() = true;
    tsm_mex_dfw.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_dfw_jfk;
    tsm_dfw_jfk.travelSeg() = &dfw_jfk;
    tsm_dfw_jfk.direction() = RuleConst::OUTBOUND;
    tsm_dfw_jfk.fareBreakAtDestination() = true;
    tsm_dfw_jfk.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_jfk_lhr;
    tsm_jfk_lhr.travelSeg() = &jfk_lhr;
    tsm_jfk_lhr.direction() = RuleConst::OUTBOUND;
    tsm_jfk_lhr.fareBreakAtDestination() = true;
    tsm_jfk_lhr.isTurnAroundPoint() = false;
    tsm_jfk_lhr.departsOrigGateway() = true;
    tsm_jfk_lhr.arrivesDestGateway() = true;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    tsiInfo.type() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::GATEWAY));

    std::string reason;

    //
    // JFK-LHR contains the origin and destination gateways
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 16A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_jfk_lhr, mc, reason, *_trx));

    //
    // MEX-DFW is not a gateway
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 16B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mex_dfw, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 16C",
                           std::string::npos != reason.find(RuleConst::MATCH_GATEWAY_DESC));

    //
    // DFW-JFK is not a gateway
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 16D",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_jfk, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 16E",
                           std::string::npos != reason.find(RuleConst::MATCH_GATEWAY_DESC));
  }

  /// Test Trans Atlantic - This match is not sensitive to scope

  void testCheckMatchCriteria17()
  {
    // Create the travel segments
    //
    AirSeg mex_dfw;
    mex_dfw.pnrSegment() = 1;
    mex_dfw.segmentOrder() = 1;
    mex_dfw.origin() = mex;
    mex_dfw.destination() = dfw;
    mex_dfw.stopOver() = false;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;
    dfw_jfk.stopOver() = false;

    AirSeg jfk_lhr;
    jfk_lhr.pnrSegment() = 3;
    jfk_lhr.segmentOrder() = 3;
    jfk_lhr.origin() = jfk;
    jfk_lhr.destination() = lhr;
    jfk_lhr.stopOver() = false;

    AirSeg lhr_cdg;
    lhr_cdg.pnrSegment() = 4;
    lhr_cdg.segmentOrder() = 4;
    lhr_cdg.origin() = lhr;
    lhr_cdg.destination() = cdg;
    lhr_cdg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mex_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_lhr);
    _trx->travelSeg().push_back(&lhr_cdg);

    RuleUtil::TSITravelSegMarkup tsm_mex_dfw;
    tsm_mex_dfw.travelSeg() = &mex_dfw;
    tsm_mex_dfw.direction() = RuleConst::OUTBOUND;
    tsm_mex_dfw.fareBreakAtDestination() = true;
    tsm_mex_dfw.isTurnAroundPoint() = false;
    tsm_mex_dfw.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_dfw_jfk;
    tsm_dfw_jfk.travelSeg() = &dfw_jfk;
    tsm_dfw_jfk.direction() = RuleConst::OUTBOUND;
    tsm_dfw_jfk.fareBreakAtDestination() = true;
    tsm_dfw_jfk.isTurnAroundPoint() = false;
    tsm_dfw_jfk.globalDirection() = GlobalDirection::US;

    RuleUtil::TSITravelSegMarkup tsm_jfk_lhr;
    tsm_jfk_lhr.travelSeg() = &jfk_lhr;
    tsm_jfk_lhr.direction() = RuleConst::OUTBOUND;
    tsm_jfk_lhr.fareBreakAtDestination() = true;
    tsm_jfk_lhr.isTurnAroundPoint() = false;
    tsm_jfk_lhr.globalDirection() = GlobalDirection::AT;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::TRANS_ATLANTIC));

    std::string reason;

    //
    // JFK-LHR is the trans-atlantic segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 17A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_jfk_lhr, mc, reason, *_trx));

    //
    // MEX-DFW is not trans-atlantic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 17B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mex_dfw, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 17C",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_ATLANTIC_DESC));

    //
    // DFW-JFK is not trans-atlantic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 17D",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_jfk, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 17E",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_ATLANTIC_DESC));
  }

  /// Test Trans Pacific - This match is not sensitive to scope

  void testCheckMatchCriteria18()
  {
    // Create the travel segments
    //
    AirSeg mex_sfo;
    mex_sfo.pnrSegment() = 1;
    mex_sfo.segmentOrder() = 1;
    mex_sfo.origin() = mex;
    mex_sfo.destination() = sfo;
    mex_sfo.stopOver() = false;

    AirSeg sfo_nrt;
    sfo_nrt.pnrSegment() = 2;
    sfo_nrt.segmentOrder() = 2;
    sfo_nrt.origin() = sfo;
    sfo_nrt.destination() = nrt;
    sfo_nrt.stopOver() = false;

    AirSeg nrt_hkg;
    nrt_hkg.pnrSegment() = 3;
    nrt_hkg.segmentOrder() = 3;
    nrt_hkg.origin() = nrt;
    nrt_hkg.destination() = hkg;
    nrt_hkg.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mex_sfo);
    _trx->travelSeg().push_back(&sfo_nrt);
    _trx->travelSeg().push_back(&nrt_hkg);

    RuleUtil::TSITravelSegMarkup tsm_mex_sfo;
    tsm_mex_sfo.travelSeg() = &mex_sfo;
    tsm_mex_sfo.direction() = RuleConst::OUTBOUND;
    tsm_mex_sfo.fareBreakAtDestination() = true;
    tsm_mex_sfo.isTurnAroundPoint() = false;
    tsm_mex_sfo.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_sfo_nrt;
    tsm_sfo_nrt.travelSeg() = &sfo_nrt;
    tsm_sfo_nrt.direction() = RuleConst::OUTBOUND;
    tsm_sfo_nrt.fareBreakAtDestination() = true;
    tsm_sfo_nrt.isTurnAroundPoint() = false;
    tsm_sfo_nrt.globalDirection() = GlobalDirection::NP;

    RuleUtil::TSITravelSegMarkup tsm_nrt_hkg;
    tsm_nrt_hkg.travelSeg() = &nrt_hkg;
    tsm_nrt_hkg.direction() = RuleConst::OUTBOUND;
    tsm_nrt_hkg.fareBreakAtDestination() = true;
    tsm_nrt_hkg.isTurnAroundPoint() = false;
    tsm_nrt_hkg.globalDirection() = GlobalDirection::ZZ;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::TRANS_PACIFIC));

    std::string reason;

    //
    // SFO-NRT is the trans-pacific segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 18A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_sfo_nrt, mc, reason, *_trx));

    //
    // NRT-HKG is not trans-pacific
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 18B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_nrt_hkg, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 18C",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_PACIFIC_DESC));

    //
    // MEX-SFO is not trans-pacific
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 18D",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mex_sfo, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 18E",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_PACIFIC_DESC));
  }

  /// Test Trans Oceanic - This match is not sensitive to scope

  void testCheckMatchCriteria19()
  {
    // Create the travel segments
    //
    AirSeg mex_dfw;
    mex_dfw.pnrSegment() = 1;
    mex_dfw.segmentOrder() = 1;
    mex_dfw.origin() = mex;
    mex_dfw.destination() = dfw;
    mex_dfw.stopOver() = false;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;
    dfw_jfk.stopOver() = false;

    AirSeg jfk_lhr;
    jfk_lhr.pnrSegment() = 3;
    jfk_lhr.segmentOrder() = 3;
    jfk_lhr.origin() = jfk;
    jfk_lhr.destination() = lhr;
    jfk_lhr.stopOver() = false;

    AirSeg lhr_cdg;
    lhr_cdg.pnrSegment() = 4;
    lhr_cdg.segmentOrder() = 4;
    lhr_cdg.origin() = lhr;
    lhr_cdg.destination() = cdg;
    lhr_cdg.stopOver() = false;

    AirSeg cdg_hkg;
    cdg_hkg.pnrSegment() = 5;
    cdg_hkg.segmentOrder() = 5;
    cdg_hkg.origin() = cdg;
    cdg_hkg.destination() = hkg;
    cdg_hkg.stopOver() = false;

    AirSeg hkg_nrt;
    hkg_nrt.pnrSegment() = 6;
    hkg_nrt.segmentOrder() = 6;
    hkg_nrt.origin() = hkg;
    hkg_nrt.destination() = nrt;
    hkg_nrt.stopOver() = false;

    AirSeg nrt_sfo;
    nrt_sfo.pnrSegment() = 7;
    nrt_sfo.segmentOrder() = 7;
    nrt_sfo.origin() = nrt;
    nrt_sfo.destination() = sfo;
    nrt_sfo.stopOver() = false;

    AirSeg sfo_mex;
    sfo_mex.pnrSegment() = 8;
    sfo_mex.segmentOrder() = 8;
    sfo_mex.origin() = sfo;
    sfo_mex.destination() = mex;
    sfo_mex.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mex_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_lhr);
    _trx->travelSeg().push_back(&lhr_cdg);
    _trx->travelSeg().push_back(&cdg_hkg);
    _trx->travelSeg().push_back(&hkg_nrt);
    _trx->travelSeg().push_back(&nrt_sfo);
    _trx->travelSeg().push_back(&sfo_mex);

    RuleUtil::TSITravelSegMarkup tsm_mex_dfw;
    tsm_mex_dfw.travelSeg() = &mex_dfw;
    tsm_mex_dfw.direction() = RuleConst::OUTBOUND;
    tsm_mex_dfw.fareBreakAtDestination() = true;
    tsm_mex_dfw.isTurnAroundPoint() = false;
    tsm_mex_dfw.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_dfw_jfk;
    tsm_dfw_jfk.travelSeg() = &dfw_jfk;
    tsm_dfw_jfk.direction() = RuleConst::OUTBOUND;
    tsm_dfw_jfk.fareBreakAtDestination() = true;
    tsm_dfw_jfk.isTurnAroundPoint() = false;
    tsm_dfw_jfk.globalDirection() = GlobalDirection::US;

    RuleUtil::TSITravelSegMarkup tsm_jfk_lhr;
    tsm_jfk_lhr.travelSeg() = &jfk_lhr;
    tsm_jfk_lhr.direction() = RuleConst::OUTBOUND;
    tsm_jfk_lhr.fareBreakAtDestination() = true;
    tsm_jfk_lhr.isTurnAroundPoint() = false;
    tsm_jfk_lhr.globalDirection() = GlobalDirection::AT;

    RuleUtil::TSITravelSegMarkup tsm_lhr_cdg;
    tsm_lhr_cdg.travelSeg() = &lhr_cdg;
    tsm_lhr_cdg.direction() = RuleConst::OUTBOUND;
    tsm_lhr_cdg.fareBreakAtDestination() = true;
    tsm_lhr_cdg.isTurnAroundPoint() = false;
    tsm_lhr_cdg.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_cdg_hkg;
    tsm_cdg_hkg.travelSeg() = &cdg_hkg;
    tsm_cdg_hkg.direction() = RuleConst::OUTBOUND;
    tsm_cdg_hkg.fareBreakAtDestination() = true;
    tsm_cdg_hkg.isTurnAroundPoint() = false;
    tsm_cdg_hkg.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_hkg_nrt;
    tsm_hkg_nrt.travelSeg() = &hkg_nrt;
    tsm_hkg_nrt.direction() = RuleConst::INBOUND;
    tsm_hkg_nrt.fareBreakAtDestination() = true;
    tsm_hkg_nrt.isTurnAroundPoint() = true;
    tsm_hkg_nrt.globalDirection() = GlobalDirection::ZZ;

    RuleUtil::TSITravelSegMarkup tsm_nrt_sfo;
    tsm_nrt_sfo.travelSeg() = &nrt_sfo;
    tsm_nrt_sfo.direction() = RuleConst::INBOUND;
    tsm_nrt_sfo.fareBreakAtDestination() = true;
    tsm_nrt_sfo.isTurnAroundPoint() = false;
    tsm_nrt_sfo.globalDirection() = GlobalDirection::NP;

    RuleUtil::TSITravelSegMarkup tsm_sfo_mex;
    tsm_sfo_mex.travelSeg() = &sfo_mex;
    tsm_sfo_mex.direction() = RuleConst::INBOUND;
    tsm_sfo_mex.fareBreakAtDestination() = true;
    tsm_sfo_mex.isTurnAroundPoint() = false;
    tsm_sfo_mex.globalDirection() = GlobalDirection::ZZ;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::TRANS_OCEANIC));

    std::string reason;

    //
    // JFK-LHR is the trans-atlantic segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_jfk_lhr, mc, reason, *_trx));

    //
    // NRT-SFO is the trans-pacific segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_nrt_sfo, mc, reason, *_trx));

    //
    // MEX-DFW is not trans-oceanic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19C",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mex_dfw, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19D",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_OCEANIC_DESC));

    //
    // DFW-JFK is not trans-oceanic
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19E",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_jfk, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 19F",
                           std::string::npos != reason.find(RuleConst::MATCH_TRANS_OCEANIC_DESC));
  }

  /// Test Intercontinental - This match is not sensitive to scope

  void testCheckMatchCriteria20()
  {
    // Create the travel segments
    //
    AirSeg mex_dfw;
    mex_dfw.pnrSegment() = 1;
    mex_dfw.segmentOrder() = 1;
    mex_dfw.origin() = mex;
    mex_dfw.destination() = dfw;
    mex_dfw.stopOver() = false;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;
    dfw_jfk.stopOver() = false;

    AirSeg jfk_lhr;
    jfk_lhr.pnrSegment() = 3;
    jfk_lhr.segmentOrder() = 3;
    jfk_lhr.origin() = jfk;
    jfk_lhr.destination() = lhr;
    jfk_lhr.stopOver() = false;

    AirSeg lhr_cdg;
    lhr_cdg.pnrSegment() = 4;
    lhr_cdg.segmentOrder() = 4;
    lhr_cdg.origin() = lhr;
    lhr_cdg.destination() = cdg;
    lhr_cdg.stopOver() = false;

    AirSeg cdg_hkg;
    cdg_hkg.pnrSegment() = 5;
    cdg_hkg.segmentOrder() = 5;
    cdg_hkg.origin() = cdg;
    cdg_hkg.destination() = hkg;
    cdg_hkg.stopOver() = false;

    AirSeg hkg_nrt;
    hkg_nrt.pnrSegment() = 6;
    hkg_nrt.segmentOrder() = 6;
    hkg_nrt.origin() = hkg;
    hkg_nrt.destination() = nrt;
    hkg_nrt.stopOver() = false;

    AirSeg nrt_sfo;
    nrt_sfo.pnrSegment() = 7;
    nrt_sfo.segmentOrder() = 7;
    nrt_sfo.origin() = nrt;
    nrt_sfo.destination() = sfo;
    nrt_sfo.stopOver() = false;

    AirSeg sfo_mex;
    sfo_mex.pnrSegment() = 8;
    sfo_mex.segmentOrder() = 8;
    sfo_mex.origin() = sfo;
    sfo_mex.destination() = mex;
    sfo_mex.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&mex_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_lhr);
    _trx->travelSeg().push_back(&lhr_cdg);
    _trx->travelSeg().push_back(&cdg_hkg);
    _trx->travelSeg().push_back(&hkg_nrt);
    _trx->travelSeg().push_back(&nrt_sfo);
    _trx->travelSeg().push_back(&sfo_mex);

    RuleUtil::TSITravelSegMarkup tsm_mex_dfw;
    tsm_mex_dfw.travelSeg() = &mex_dfw;
    tsm_mex_dfw.direction() = RuleConst::OUTBOUND;
    tsm_mex_dfw.fareBreakAtDestination() = true;
    tsm_mex_dfw.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_dfw_jfk;
    tsm_dfw_jfk.travelSeg() = &dfw_jfk;
    tsm_dfw_jfk.direction() = RuleConst::OUTBOUND;
    tsm_dfw_jfk.fareBreakAtDestination() = true;
    tsm_dfw_jfk.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_jfk_lhr;
    tsm_jfk_lhr.travelSeg() = &jfk_lhr;
    tsm_jfk_lhr.direction() = RuleConst::OUTBOUND;
    tsm_jfk_lhr.fareBreakAtDestination() = true;
    tsm_jfk_lhr.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_lhr_cdg;
    tsm_lhr_cdg.travelSeg() = &lhr_cdg;
    tsm_lhr_cdg.direction() = RuleConst::OUTBOUND;
    tsm_lhr_cdg.fareBreakAtDestination() = true;
    tsm_lhr_cdg.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_cdg_hkg;
    tsm_cdg_hkg.travelSeg() = &cdg_hkg;
    tsm_cdg_hkg.direction() = RuleConst::OUTBOUND;
    tsm_cdg_hkg.fareBreakAtDestination() = true;
    tsm_cdg_hkg.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_hkg_nrt;
    tsm_hkg_nrt.travelSeg() = &hkg_nrt;
    tsm_hkg_nrt.direction() = RuleConst::INBOUND;
    tsm_hkg_nrt.fareBreakAtDestination() = true;
    tsm_hkg_nrt.isTurnAroundPoint() = true;

    RuleUtil::TSITravelSegMarkup tsm_nrt_sfo;
    tsm_nrt_sfo.travelSeg() = &nrt_sfo;
    tsm_nrt_sfo.direction() = RuleConst::INBOUND;
    tsm_nrt_sfo.fareBreakAtDestination() = true;
    tsm_nrt_sfo.isTurnAroundPoint() = false;

    RuleUtil::TSITravelSegMarkup tsm_sfo_mex;
    tsm_sfo_mex.travelSeg() = &sfo_mex;
    tsm_sfo_mex.direction() = RuleConst::INBOUND;
    tsm_sfo_mex.fareBreakAtDestination() = true;
    tsm_sfo_mex.isTurnAroundPoint() = false;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INTERCONTINENTAL));

    std::string reason;

    //
    // JFK-LHR is an intercontinental segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_jfk_lhr, mc, reason, *_trx));

    //
    // CDG-HKG is an intercontinental segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_cdg_hkg, mc, reason, *_trx));

    //
    // NRT-SFO is an intercontinental segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20C",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_nrt_sfo, mc, reason, *_trx));

    //
    // MEX-DFW is not intercontinental
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20D",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mex_dfw, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20E",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTERCONTINENTAL_DESC));

    //
    // DFW-JFK is not intercontinental
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20F",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_dfw_jfk, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20G",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTERCONTINENTAL_DESC));

    //
    // LHR-CDG is not intercontinental
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20H",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_lhr_cdg, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20I",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTERCONTINENTAL_DESC));

    //
    // HKG-NRT is intercontinental
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 20J",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_hkg_nrt, mc, reason, *_trx));
  }

  /// Test Over Water - This match is not sensitive to scope

  void testCheckMatchCriteria21()
  {
    // Create the travel segments
    //
    AirSeg gig_lax;
    gig_lax.pnrSegment() = 1;
    gig_lax.segmentOrder() = 1;
    gig_lax.origin() = gig;
    gig_lax.destination() = lax;
    gig_lax.stopOver() = false;

    AirSeg lax_hnl;
    lax_hnl.pnrSegment() = 2;
    lax_hnl.segmentOrder() = 2;
    lax_hnl.origin() = lax;
    lax_hnl.destination() = hnl;
    lax_hnl.stopOver() = false;

    AirSeg hnl_lax;
    hnl_lax.pnrSegment() = 3;
    hnl_lax.segmentOrder() = 3;
    hnl_lax.origin() = hnl;
    hnl_lax.destination() = lax;
    hnl_lax.stopOver() = false;

    AirSeg lax_mia;
    lax_mia.pnrSegment() = 4;
    lax_mia.segmentOrder() = 4;
    lax_mia.origin() = lax;
    lax_mia.destination() = mia;
    lax_mia.stopOver() = false;

    AirSeg mia_gig;
    mia_gig.pnrSegment() = 5;
    mia_gig.segmentOrder() = 5;
    mia_gig.origin() = mia;
    mia_gig.destination() = gig;
    mia_gig.stopOver() = false;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&gig_lax);
    _trx->travelSeg().push_back(&lax_hnl);
    _trx->travelSeg().push_back(&hnl_lax);
    _trx->travelSeg().push_back(&lax_mia);
    _trx->travelSeg().push_back(&mia_gig);

    RuleUtil::TSITravelSegMarkup tsm_gig_lax;
    tsm_gig_lax.travelSeg() = &gig_lax;
    tsm_gig_lax.direction() = RuleConst::OUTBOUND;
    tsm_gig_lax.fareBreakAtDestination() = true;
    tsm_gig_lax.isTurnAroundPoint() = false;
    tsm_gig_lax.isOverWater() = false;

    RuleUtil::TSITravelSegMarkup tsm_lax_hnl;
    tsm_lax_hnl.travelSeg() = &lax_hnl;
    tsm_lax_hnl.direction() = RuleConst::OUTBOUND;
    tsm_lax_hnl.fareBreakAtDestination() = true;
    tsm_lax_hnl.isTurnAroundPoint() = false;
    tsm_lax_hnl.isOverWater() = true;

    RuleUtil::TSITravelSegMarkup tsm_hnl_lax;
    tsm_hnl_lax.travelSeg() = &hnl_lax;
    tsm_hnl_lax.direction() = RuleConst::OUTBOUND;
    tsm_hnl_lax.fareBreakAtDestination() = true;
    tsm_hnl_lax.isTurnAroundPoint() = false;
    tsm_hnl_lax.isOverWater() = true;

    RuleUtil::TSITravelSegMarkup tsm_lax_mia;
    tsm_lax_mia.travelSeg() = &lax_mia;
    tsm_lax_mia.direction() = RuleConst::OUTBOUND;
    tsm_lax_mia.fareBreakAtDestination() = true;
    tsm_lax_mia.isTurnAroundPoint() = false;
    tsm_lax_mia.isOverWater() = false;

    RuleUtil::TSITravelSegMarkup tsm_mia_gig;
    tsm_mia_gig.travelSeg() = &mia_gig;
    tsm_mia_gig.direction() = RuleConst::INBOUND;
    tsm_mia_gig.fareBreakAtDestination() = true;
    tsm_mia_gig.isTurnAroundPoint() = true;
    tsm_mia_gig.isOverWater() = false;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria> mc;
    mc.push_back(TSIInfo::TSIMatchCriteria(TSIInfo::OVER_WATER));

    std::string reason;

    //
    // LAX-HNL is an over water segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_lax_hnl, mc, reason, *_trx));

    //
    // HNL-LAX is an over water segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21B",
                           RuleUtil::checkMatchCriteria(tsiData, tsm_hnl_lax, mc, reason, *_trx));

    //
    // MIA-GIG is not over water
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21C",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_mia_gig, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21D",
                           std::string::npos != reason.find(RuleConst::MATCH_OVER_WATER_DESC));

    //
    // GIG-LAX is not over water
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21E",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_gig_lax, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21F",
                           std::string::npos != reason.find(RuleConst::MATCH_OVER_WATER_DESC));

    //
    // LAX-MIA is not over water
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21G",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm_lax_mia, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21H",
                           std::string::npos != reason.find(RuleConst::MATCH_OVER_WATER_DESC));
  }

  /// Test International Domestic Transfer - This match is not sensitive to scope

  void testCheckMatchCriteria22()
  {
    // Create the travel segments
    //
    AirSeg gig_lax;
    gig_lax.pnrSegment() = 1;
    gig_lax.segmentOrder() = 1;
    gig_lax.origin() = gig;
    gig_lax.destination() = lax;
    gig_lax.stopOver() = false;
    gig_lax.carrier() = "AA";
    gig_lax.flightNumber() = 1234;

    AirSeg lax_hnl;
    lax_hnl.pnrSegment() = 2;
    lax_hnl.segmentOrder() = 2;
    lax_hnl.origin() = lax;
    lax_hnl.destination() = hnl;
    lax_hnl.stopOver() = false;
    lax_hnl.carrier() = "AA";
    lax_hnl.flightNumber() = 2345;

    AirSeg hnl_lax;
    hnl_lax.pnrSegment() = 3;
    hnl_lax.segmentOrder() = 3;
    hnl_lax.origin() = hnl;
    hnl_lax.destination() = lax;
    hnl_lax.stopOver() = false;
    hnl_lax.carrier() = "AA";
    hnl_lax.flightNumber() = 3456;

    AirSeg lax_mia;
    lax_mia.pnrSegment() = 4;
    lax_mia.segmentOrder() = 4;
    lax_mia.origin() = lax;
    lax_mia.destination() = mia;
    lax_mia.stopOver() = false;
    lax_mia.carrier() = "AA";
    lax_mia.flightNumber() = 4567;

    AirSeg mia_gig;
    mia_gig.pnrSegment() = 5;
    mia_gig.segmentOrder() = 5;
    mia_gig.origin() = mia;
    mia_gig.destination() = gig;
    mia_gig.stopOver() = false;
    mia_gig.carrier() = "AA";
    mia_gig.flightNumber() = 5678;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&gig_lax);
    _trx->travelSeg().push_back(&lax_hnl);
    _trx->travelSeg().push_back(&hnl_lax);
    _trx->travelSeg().push_back(&lax_mia);
    _trx->travelSeg().push_back(&mia_gig);

    RuleUtil::TSITravelSegMarkup tsm_gig_lax;
    tsm_gig_lax.travelSeg() = &gig_lax;
    tsm_gig_lax.nextTravelSeg() = &lax_hnl;
    tsm_gig_lax.direction() = RuleConst::OUTBOUND;
    tsm_gig_lax.fareBreakAtDestination() = false;
    tsm_gig_lax.isTurnAroundPoint() = false;
    tsm_gig_lax.isOverWater() = false;
    tsm_gig_lax.isInternational() = true;

    RuleUtil::TSITravelSegMarkup tsm_lax_hnl;
    tsm_lax_hnl.travelSeg() = &lax_hnl;
    tsm_lax_hnl.nextTravelSeg() = &hnl_lax;
    tsm_lax_hnl.direction() = RuleConst::OUTBOUND;
    tsm_lax_hnl.fareBreakAtDestination() = false;
    tsm_lax_hnl.isTurnAroundPoint() = false;
    tsm_lax_hnl.isOverWater() = true;
    tsm_lax_hnl.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_hnl_lax;
    tsm_hnl_lax.travelSeg() = &hnl_lax;
    tsm_hnl_lax.nextTravelSeg() = &lax_mia;
    tsm_hnl_lax.direction() = RuleConst::OUTBOUND;
    tsm_hnl_lax.fareBreakAtDestination() = false;
    tsm_hnl_lax.isTurnAroundPoint() = false;
    tsm_hnl_lax.isOverWater() = true;
    tsm_hnl_lax.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_lax_mia;
    tsm_lax_mia.travelSeg() = &lax_mia;
    tsm_lax_mia.nextTravelSeg() = &mia_gig;
    tsm_lax_mia.direction() = RuleConst::OUTBOUND;
    tsm_lax_mia.fareBreakAtDestination() = true;
    tsm_lax_mia.isTurnAroundPoint() = false;
    tsm_lax_mia.isOverWater() = false;
    tsm_lax_mia.isInternational() = false;

    RuleUtil::TSITravelSegMarkup tsm_mia_gig;
    tsm_mia_gig.travelSeg() = &mia_gig;
    tsm_mia_gig.direction() = RuleConst::INBOUND;
    tsm_mia_gig.fareBreakAtDestination() = true;
    tsm_mia_gig.isTurnAroundPoint() = true;
    tsm_mia_gig.isOverWater() = false;
    tsm_mia_gig.isInternational() = true;

    RuleUtil::TSITravelSegMarkupContainer tsMarkup;

    tsMarkup.push_back(tsm_gig_lax);
    tsMarkup.push_back(tsm_lax_hnl);
    tsMarkup.push_back(tsm_hnl_lax);
    tsMarkup.push_back(tsm_lax_mia);
    tsMarkup.push_back(tsm_mia_gig);

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;
    tsiInfo.matchCriteria().push_back(TSIInfo::TSIMatchCriteria(TSIInfo::INTL_DOM_TRANSFER));

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiInfo.matchCriteria();

    std::string reason;

    RuleUtil::identifyIntlDomTransfers(tsiData, tsMarkup);

    RuleUtil::TSITravelSegMarkupI iter = tsMarkup.begin();
    RuleUtil::TSITravelSegMarkup& tsm = *iter;

    //
    // GIG-LAX is an international domestic transfer point
    //
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 21A",
                           RuleUtil::checkMatchCriteria(tsiData, tsm, mc, reason, *_trx));

    //
    // LAX-HNL is not an international domestic transfer point
    //
    ++iter;
    tsm = *iter;
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22B",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22C",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTL_DOM_TRANSFER_DESC));

    //
    // HNL-LAX is not an international domestic transfer point
    //
    ++iter;
    tsm = *iter;
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22D",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22E",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTL_DOM_TRANSFER_DESC));

    //
    // LAX-MIA is not an international domestic transfer point because it
    //  is a fare-break point.
    //
    ++iter;
    tsm = *iter;
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22F",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22G",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTL_DOM_TRANSFER_DESC));

    //
    // MIA-GIG is not an international domestic transfer point
    //
    ++iter;
    tsm = *iter;
    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22H",
                           !RuleUtil::checkMatchCriteria(tsiData, tsm, mc, reason, *_trx));

    CPPUNIT_ASSERT_MESSAGE("Error in checkMatchCriteria: Test 22I",
                           std::string::npos !=
                               reason.find(RuleConst::MATCH_INTL_DOM_TRANSFER_DESC));
  }

  //***************************************************************************************
  void testMatchFareFootnote1()
  {

    Footnote ft1FromRule;
    ft1FromRule.clear();
    Footnote ft2FromRule = "r1";
    Footnote ft1FromFare = "f1";
    Footnote ft2FromFare = "f2";

    try
    {

      bool ret = RuleUtil::matchFareFootnote(ft1FromRule, ft2FromRule, ft1FromFare, ft2FromFare);
      CPPUNIT_ASSERT_EQUAL(true, ret);
    }
    catch (...) { CPPUNIT_ASSERT_MESSAGE("Exception generated", false); }
  }
  void testMatchFareFootnote2()
  {

    Footnote ft1FromRule = "r1";
    Footnote ft2FromRule;
    ft2FromRule.clear();
    Footnote ft1FromFare = "f1";
    Footnote ft2FromFare = "f2";

    try
    {
      bool ret = RuleUtil::matchFareFootnote(ft1FromRule, ft2FromRule, ft1FromFare, ft2FromFare);
      CPPUNIT_ASSERT_EQUAL(false, ret);
    }
    catch (...) { CPPUNIT_ASSERT_MESSAGE("Exception generated", false); }
  }
  void testMatchFareFootnote3()
  {

    Footnote ft1FromRule = "t1";
    Footnote ft2FromRule = "r1";
    Footnote ft1FromFare = "f1";
    Footnote ft2FromFare = "t1";

    try
    {
      bool ret = RuleUtil::matchFareFootnote(ft1FromRule, ft2FromRule, ft1FromFare, ft2FromFare);
      CPPUNIT_ASSERT_EQUAL(false, ret);
    }
    catch (...) { CPPUNIT_ASSERT_MESSAGE("Exception generated", false); }
  }
  void testMatchFareFootnote4()
  {

    Footnote ft1FromRule = "t1";
    Footnote ft2FromRule = "r1";
    Footnote ft1FromFare = "f1";
    Footnote ft2FromFare = "f2";

    try
    {
      bool ret = RuleUtil::matchFareFootnote(ft1FromRule, ft2FromRule, ft1FromFare, ft2FromFare);
      CPPUNIT_ASSERT_EQUAL(false, ret);
    }
    catch (...) { CPPUNIT_ASSERT_MESSAGE("Exception generated", false); }
  }

  void testMatchFareClass()
  {
    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 1",
                           RuleUtil::matchFareClass("H-E70", "HLE70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 2",
                           !RuleUtil::matchFareClass("H-E70", "BE70HNR"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 3",
                           RuleUtil::matchFareClass("B-E70", "BHE70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 4",
                           RuleUtil::matchFareClass("B-E70", "BE70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 5",
                           RuleUtil::matchFareClass("B-E70", "BHE70NR"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 6",
                           !RuleUtil::matchFareClass("B-E70", "BXE170"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 7",
                           RuleUtil::matchFareClass("-E70", "BE70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 8",
                           RuleUtil::matchFareClass("-E70", "BHE70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 9",
                           !RuleUtil::matchFareClass("-E70", "E70"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 10",
                           RuleUtil::matchFareClass("-M", "YM"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 11",
                           !RuleUtil::matchFareClass("-M", "M"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 12", RuleUtil::matchFareClass("M-", "M"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 13",
                           RuleUtil::matchFareClass("GL-3", "GL3BRTH"));

    CPPUNIT_ASSERT_MESSAGE("Error in matchFareClass: Test 14",
                           !RuleUtil::matchFareClass("S0MS-BR", "QW0ACA"));
  }

  void testMatchLocation_Fail()
  {
    bool isLocSwapped = false;
    Footnote footnote;
    footnote = "a1";

    CPPUNIT_ASSERT(!RuleUtil::matchLocation(*_trx,
                                            createLocKey(LOCTYPE_CITY, lon->loc()),
                                            createLocKey(LOCTYPE_CITY, man->loc()),
                                            createPTF(tul, man, dfw->loc(), lon->loc()),
                                            isLocSwapped,
                                            footnote,
                                            20));
  }

  void testMatchLocation_Pass_AnyLoc()
  {
    bool isLocSwapped = false;

    CPPUNIT_ASSERT(RuleUtil::matchLocation(*_trx,
                                           createLocKey(RuleConst::ANY_LOCATION_TYPE, ""),
                                           createLocKey(RuleConst::ANY_LOCATION_TYPE, ""),
                                           createPTF(tul, man, dfw->loc(), lon->loc()),
                                           isLocSwapped,
                                           "",
                                           0));
  }

  void testMatchLocation_Pass_NotConstructed()
  {
    bool isLocSwapped = false;

    CPPUNIT_ASSERT(RuleUtil::matchLocation(*_trx,
                                           createLocKey(LOCTYPE_CITY, tul->loc()),
                                           createLocKey(LOCTYPE_CITY, man->loc()),
                                           createPTF(tul, man, dfw->loc(), lon->loc(), false),
                                           isLocSwapped,
                                           "",
                                           0));
  }

  void testMatchOneWayRoundTrip()
  {
    Indicator owrtFromRule;
    Indicator owrtFromFare;

    owrtFromRule = ' ';
    owrtFromFare = '1';

    bool ret = RuleUtil::matchOneWayRoundTrip(owrtFromRule, owrtFromFare);
    CPPUNIT_ASSERT_EQUAL(true, ret);

    owrtFromRule = '1';
    owrtFromFare = '3';

    ret = RuleUtil::matchOneWayRoundTrip(owrtFromRule, owrtFromFare);
    CPPUNIT_ASSERT_EQUAL(true, ret);

    owrtFromRule = '1';
    owrtFromFare = '1';

    ret = RuleUtil::matchOneWayRoundTrip(owrtFromRule, owrtFromFare);
    CPPUNIT_ASSERT_EQUAL(true, ret);

    owrtFromRule = '1';
    owrtFromFare = '2';

    ret = RuleUtil::matchOneWayRoundTrip(owrtFromRule, owrtFromFare);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testMatchOWRT_True_Rule_Blank()
  {
    Indicator owrtFromFareFocusRule = ' ';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));
  }

  void testMatchOWRT_True()
  {
    Indicator owrtFromFareFocusRule = '1';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

    owrtFromFareFocusRule = '4';
    owrtFromFare = '1';
    CPPUNIT_ASSERT(RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

    owrtFromFareFocusRule = '4';
    owrtFromFare = '3';
    CPPUNIT_ASSERT(RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

  }

  void testMatchOWRT_False()
  {
    Indicator owrtFromFareFocusRule = '2';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(!RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

    owrtFromFareFocusRule = '4';
    owrtFromFare = '2';
    CPPUNIT_ASSERT(!RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

    owrtFromFareFocusRule = '3';
    owrtFromFare = '2';
    CPPUNIT_ASSERT(!RuleUtil::matchOWRT(owrtFromFareFocusRule,owrtFromFare));

  }


  PaxTypeFare* createFareWithRouting(bool rtgProcessed, RoutingNumber rtg)
  {
    PaxTypeFare* ret = _memHandle.create<PaxTypeFare>();
    ret->setFare(_memHandle.create<Fare>());
    FareInfo* fi = _memHandle.create<FareInfo>();
    ret->fare()->setFareInfo(fi);
    ret->fare()->setRoutingProcessed(rtgProcessed);
    fi->routingNumber() = rtg;
    if (rtgProcessed && rtg != "0000")
      ret->fare()->setIsRouting();
    return ret;
  }

  void testMatchFareRouteNumber_Processed_S_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "1234");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('S', "1234", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_Processed_S_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "9999");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('S', "1234", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_Processed_R_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "1234");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('R', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_Processed_R_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "0000");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('R', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_Processed_M_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "0000");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('M', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_Processed_M_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(true, "1234");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('M', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_S_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "1234");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('S', "1234", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_S_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "9999");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('S', "1234", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_R_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "1234");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('R', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_R_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "0000");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('R', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_M_Pass()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "0000");
    CPPUNIT_ASSERT(RuleUtil::matchFareRouteNumber('M', "", ptf->routingNumber()));
  }

  void testMatchFareRouteNumber_NotProcessed_M_Fail()
  {
    PaxTypeFare* ptf = createFareWithRouting(false, "1234");
    CPPUNIT_ASSERT(!RuleUtil::matchFareRouteNumber('M', "", ptf->routingNumber()));
  }

  void testMatchFareType1()
  {
    FareType ftFromRule = "AB";
    FareType ftFromFareC = "AB";

    bool ret = RuleUtil::matchFareType(ftFromRule, ftFromFareC);
    CPPUNIT_ASSERT(ret);
  }

  void testMatchFareType2()
  {
    FareType ftFromRule;
    ftFromRule.clear();
    FareType ftFromFareC = "AB";

    bool ret = RuleUtil::matchFareType(ftFromRule, ftFromFareC);
    CPPUNIT_ASSERT(ret);
  }

  void testMatchFareType3()
  {
    FareType ftFromRule = "AB";
    FareType ftFromFareC = "XY";

    bool ret = RuleUtil::matchFareType(ftFromRule, ftFromFareC);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testMatchGenericFareType1()
  {
    FareType ftFromRule =  "*X";
    FareType ftFromFareC = "XEX";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchGenericFareType2()
  {
    FareType ftFromRule;
    ftFromRule.clear();
    FareType ftFromFareC = "FR";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchGenericFareType3()
  {
    FareType ftFromRule = "*FR";
    FareType ftFromFareC = "BU";

    CPPUNIT_ASSERT_EQUAL(false, RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchGenericFareType4()
  {
    FareType ftFromRule =  "*W";
    FareType ftFromFareC = "WR";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchGenericFareType5()
  {
    FareType ftFromRule =  "*EW";
    FareType ftFromFareC = "ZIP";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchGenericFareType6()
  {
    FareType ftFromRule =  "*BJ";
    FareType ftFromFareC = "JR";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

    void testMatchGenericFareType7()
  {
    FareType ftFromRule =  "*EW";
    FareType ftFromFareC = "WR";

    CPPUNIT_ASSERT(RuleUtil::matchGenericFareType(ftFromRule, ftFromFareC));
  }

  void testMatchSeasons1()
  {
    Indicator sFromRule = 'A';
    Indicator sFromFareC = 'B';

    bool ret = RuleUtil::matchSeasons(sFromRule, sFromFareC);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testMatchSeasons2()
  {
    Indicator sFromRule = ' ';
    Indicator sFromFareC = 'B';

    bool ret = RuleUtil::matchSeasons(sFromRule, sFromFareC);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testMatchSeasons3()
  {
    Indicator sFromRule = 'A';
    Indicator sFromFareC = 'A';

    bool ret = RuleUtil::matchSeasons(sFromRule, sFromFareC);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testMatchDayOfWeek1()
  {
    Indicator dowFromRule = ' ';
    Indicator dowFromFareC = 'd';

    bool ret = RuleUtil::matchDayOfWeek(dowFromRule, dowFromFareC);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testMatchDayOfWeek2()
  {
    Indicator dowFromRule = 'x';
    Indicator dowFromFareC = 'y';

    bool ret = RuleUtil::matchDayOfWeek(dowFromRule, dowFromFareC);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testMatchDayOfWeek3()
  {
    Indicator dowFromRule = 'm';
    Indicator dowFromFareC = 'm';

    bool ret = RuleUtil::matchDayOfWeek(dowFromRule, dowFromFareC);
    CPPUNIT_ASSERT_EQUAL(true, ret);
  }

  void testMatchFareClass2()
  {
    FareClassCode fcFromRule;
    FareClassCode fcFromFare;

    fcFromRule.clear();
    fcFromFare = "Y26";
    bool ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "Y26";
    fcFromFare = "Y26";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "BE-70";
    fcFromFare = "BEX70";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "BE-70";
    fcFromFare = "BE1X70";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "BE-70";
    fcFromFare = "BEX170";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(false, ret);

    fcFromRule = "-E70";
    fcFromFare = "BE70NR";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "B-E70";
    fcFromFare = "BE701";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(false, ret);

    fcFromRule = "B-E70";
    fcFromFare = "BE710";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(false, ret);

    fcFromRule = "B-E70";
    fcFromFare = "BE170";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(false, ret);

    fcFromRule = "-E70";
    fcFromFare = "BE70";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "-E70";
    fcFromFare = "YNWE70";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "-E70";
    fcFromFare = "Q123E70R";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(true, ret);

    fcFromRule = "-HE70";
    fcFromFare = "HE70NR";
    ret = RuleUtil::matchFareClass(fcFromRule.c_str(), fcFromFare.c_str());
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testAddPeriodToDate_Minutes()
  {
    PeriodOfStay pos("3", "N");
    DateTime tvlDt = DateTime(2008, 02, 29, 23, 59, 6);

    DateTime product = RuleUtil::addPeriodToDate(tvlDt, pos);

    DateTime expected = DateTime(2008, 03, 1, 0, 2, 6);
    CPPUNIT_ASSERT_EQUAL(expected, product);
  }

  void testAddPeriodToDate_Hours()
  {
    PeriodOfStay pos("3", "H");
    DateTime tvlDt = DateTime(2008, 02, 29, 23, 59, 6);

    DateTime product = RuleUtil::addPeriodToDate(tvlDt, pos);

    DateTime expected = DateTime(2008, 03, 1, 2, 59, 6);
    CPPUNIT_ASSERT_EQUAL(expected, product);
  }

  void testAddPeriodToDate_Days()
  {
    PeriodOfStay pos("3", "D");
    DateTime tvlDt = DateTime(2008, 02, 29, 23, 59, 6);

    DateTime product = RuleUtil::addPeriodToDate(tvlDt, pos);

    DateTime expected = DateTime(2008, 03, 3, 23, 59, 6);
    CPPUNIT_ASSERT_EQUAL(expected, product);
  }

  void testAddPeriodToDate_MonthsAddDaysExcpt()
  {
    PeriodOfStay pos("3", "M");
    DateTime tvlDt = DateTime(2008, 02, 29, 23, 59, 6);

    DateTime product = RuleUtil::addPeriodToDate(tvlDt, pos);

    DateTime expected = DateTime(2008, 05, 31, 23, 59, 6);
    CPPUNIT_ASSERT_EQUAL(expected, product);
  }

  void testAddPeriodToDate_MonthsSubtractDaysExcpt()
  {
    PeriodOfStay pos("12", "M");
    DateTime tvlDt = DateTime(2008, 02, 29, 23, 59, 6);

    DateTime product = RuleUtil::addPeriodToDate(tvlDt, pos);

    DateTime expected = DateTime(2009, 02, 28, 23, 59, 6);
    CPPUNIT_ASSERT_EQUAL(expected, product);
  }

  void testValidateDutyFunctionCodeReturnFalseWhenNoDutyCodeInAgent()
  {
    Agent agent;
    agent.agentDuty().clear();
    AgencyDutyCode dbDutyFunctionCode = "*";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnTrueWhenDutyCodeMatchAndDBFunctionCodeLengthOne()
  {
    Agent agent;
    agent.agentDuty() = "*";
    AgencyDutyCode dbDutyFunctionCode = "*";
    CPPUNIT_ASSERT(RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnFalseWhenNoMatchAndAgentDutyCode0To9()
  {
    Agent agent;
    agent.agentDuty() = "*";
    AgencyDutyCode dbDutyFunctionCode = "0";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
    dbDutyFunctionCode = "5";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
    dbDutyFunctionCode = "9";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnFalseWhenDBDutyAAndAgentDutyNotDollar()
  {
    Agent agent;
    agent.agentDuty() = "*";
    AgencyDutyCode dbDutyFunctionCode = "A";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnTrueWhenDBDutyAAndAgentDutyDollar()
  {
    Agent agent;
    agent.agentDuty() = "$";
    AgencyDutyCode dbDutyFunctionCode = "A";
    CPPUNIT_ASSERT(RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnFalseAgentFunctionEmpty()
  {
    Agent agent;
    agent.agentDuty() = "$";
    agent.agentFunctions().clear();
    AgencyDutyCode dbDutyFunctionCode = "AV";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnFalseAgentFunctionDoesNotMatch()
  {
    Agent agent;
    agent.agentDuty() = "$";
    agent.agentFunctions() = "O";
    AgencyDutyCode dbDutyFunctionCode = "AV";
    CPPUNIT_ASSERT(!RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testValidateDutyFunctionCodeReturnTrueWhenAgentFunctionMatch()
  {
    Agent agent;
    agent.agentDuty() = "$";
    agent.agentFunctions() = "V";
    AgencyDutyCode dbDutyFunctionCode = "AV";
    CPPUNIT_ASSERT(RuleUtil::validateDutyFunctionCode(&agent, dbDutyFunctionCode));
  }

  void testGetCat27TourCodeFareNull()
  {
    std::string tourCode = "";

    RuleUtil::getCat27TourCode(NULL, tourCode);
    CPPUNIT_ASSERT_EQUAL(std::string(""), tourCode);
  }

  void testGetCat27TourCodeNoTourCode()
  {
    PaxTypeFare empty;
    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&empty, tourCode);

    CPPUNIT_ASSERT_EQUAL(std::string(""), tourCode);
  }

  void testGetCat27TourCodeTourCode()
  {
    const std::string TOURCODE = "TEST";

    PaxTypeFare fareWithTourCode;
    addTourCode(fareWithTourCode, TOURCODE);

    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&fareWithTourCode, tourCode);

    CPPUNIT_ASSERT_EQUAL(TOURCODE, tourCode);
  }

  void testGetCat27TourCodeTourCodeEmpty()
  {
    PaxTypeFare fareWithTourCode;
    addTourCode(fareWithTourCode, "");

    std::string tourCode = "not empty";
    RuleUtil::getCat27TourCode(&fareWithTourCode, tourCode);

    CPPUNIT_ASSERT_EQUAL(std::string(""), tourCode);
  }

  void addTourCode(PaxTypeFare& fare, const std::string& tourCode)
  {
    Tours* tours = _memHandle.create<Tours>();
    tours->tourNo() = tourCode;

    PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
    ruleData->ruleItemInfo() = tours;

    PaxTypeFare::PaxTypeFareAllRuleData* data =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    data->fareRuleData = ruleData;

    (*fare.paxTypeFareRuleDataMap())[RuleConst::TOURS_RULE] = data;
  }

  void testGetCat27TourCodeNoFBR()
  {
    const std::string TOURCODE = "NOFBR";

    PaxTypeFare fare;
    addTourCode(fare, TOURCODE);

    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&fare, tourCode);

    CPPUNIT_ASSERT_EQUAL(TOURCODE, tourCode);
  }

  void testGetCat27TourCodeFBRResultingFare()
  {
    const std::string RESULTING_TOURCODE = "RESULTING";
    const std::string BASE_TOURCODE = "BASE";

    PaxTypeFare fare, baseFare;
    addTourCode(fare, RESULTING_TOURCODE);
    addTourCode(baseFare, BASE_TOURCODE);
    addCat27FBRData(fare, 'R', &baseFare);

    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&fare, tourCode);

    CPPUNIT_ASSERT_EQUAL(RESULTING_TOURCODE, tourCode);
  }

  void testGetCat27TourCodeFBRBaseFare()
  {
    const std::string RESULTING_TOURCODE = "RESULTING";
    const std::string BASE_TOURCODE = "BASE";

    PaxTypeFare fare, baseFare;
    addTourCode(fare, RESULTING_TOURCODE);
    addTourCode(baseFare, BASE_TOURCODE);
    addCat27FBRData(fare, 'B', &baseFare);

    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&fare, tourCode);

    CPPUNIT_ASSERT_EQUAL(BASE_TOURCODE, tourCode);
  }

  void testGetCat27TourCodeFBROtherInd()
  {
    const std::string RESULTING_TOURCODE = "RESULTING";
    const std::string BASE_TOURCODE = "BASE";

    PaxTypeFare fare, baseFare;
    addTourCode(fare, RESULTING_TOURCODE);
    addTourCode(baseFare, BASE_TOURCODE);
    addCat27FBRData(fare, 'X', &baseFare);

    std::string tourCode = "";
    RuleUtil::getCat27TourCode(&fare, tourCode);

    CPPUNIT_ASSERT_EQUAL(RESULTING_TOURCODE, tourCode);
  }

  void addCat27FBRData(PaxTypeFare& fare, Indicator ovrdcat27, PaxTypeFare* baseFare)
  {
    FareByRuleItemInfo* fbrItemInfo = _memHandle.create<FareByRuleItemInfo>();
    fbrItemInfo->fareInd() = ' ';
    fbrItemInfo->ovrdcat27() = ovrdcat27;

    FBRPaxTypeFareRuleData* ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    ruleData->ruleItemInfo() = fbrItemInfo;
    ruleData->baseFare() = baseFare;

    PaxTypeFare::PaxTypeFareAllRuleData* data =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    data->fareRuleData = ruleData;

    (*fare.paxTypeFareRuleDataMap())[RuleConst::FARE_BY_RULE] = data;

    fare.status().set(PaxTypeFare::PTF_FareByRule);
  }

  void testGetFootnotes_ConstructedATP_omittedFootnotes()
  {
    PaxTypeFare& paxTypeFare = createPTF(tul,
                                         man,
                                         dfw->loc(),
                                         lon->loc(),
                                         true,
                                         ConstructedFareInfo::DOUBLE_ENDED,
                                         BOTH,
                                         "T",
                                         "A",
                                         "",
                                         "F",
                                         "ATP");

    std::vector<Footnote> footnotes;
    std::vector<TariffNumber> fareTariffs;

    RuleUtil::getFootnotes(paxTypeFare, footnotes, fareTariffs);
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "T") == footnotes.end());
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "F") == footnotes.end());
  }

  void testGetFootnotes_ConstructedATP_collectedFootnotes()
  {
    PaxTypeFare& paxTypeFare = createPTF(tul,
                                         man,
                                         dfw->loc(),
                                         lon->loc(),
                                         true,
                                         ConstructedFareInfo::DOUBLE_ENDED,
                                         BOTH,
                                         "T",
                                         "A",
                                         "",
                                         "F",
                                         "ATP");

    std::vector<Footnote> footnotes;
    std::vector<TariffNumber> fareTariffs;

    RuleUtil::getFootnotes(paxTypeFare, footnotes, fareTariffs);
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "A") != footnotes.end());
  }

  void testGetFootnotes_ConstructedOther()
  {
    PaxTypeFare& paxTypeFare = createPTF(tul,
                                         man,
                                         dfw->loc(),
                                         lon->loc(),
                                         true,
                                         ConstructedFareInfo::DOUBLE_ENDED,
                                         BOTH,
                                         "T",
                                         "A",
                                         "",
                                         "F",
                                         "OTH");

    std::vector<Footnote> footnotes;
    std::vector<TariffNumber> fareTariffs;

    RuleUtil::getFootnotes(paxTypeFare, footnotes, fareTariffs);
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "T") != footnotes.end());
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "F") != footnotes.end());
    CPPUNIT_ASSERT(std::find(footnotes.begin(), footnotes.end(), "A") != footnotes.end());
  }

  void testValidateMatchExpression()
  {
    FareClassCodeC f = "-";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "?";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "-?";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "?-";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "--";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "Y--";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "--T";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "-";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "Y?-";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "Y-?";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "-?T";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "?-T";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "?";
    CPPUNIT_ASSERT(!RuleUtil::validateMatchExpression(f));

    f = "YRT";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-T";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y-T";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-T-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y-T-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y???????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "???????T";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y??????T";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??T?????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "???T????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "????T???";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "?????T??";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??????T?";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y?";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y?";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y??";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y???";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y?????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "Y??????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "???????Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??????Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "?????Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "????Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "???Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "?Y";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "?L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "???L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "????L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "?????L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "??????L-";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L?";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L??";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L???";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L?????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));

    f = "-L??????";
    CPPUNIT_ASSERT(RuleUtil::validateMatchExpression(f));
  }

  void testMatchFareClassExpression()
  {
    std::string s = "YRT";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "YRT"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "YR"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "YRT2"));

    s = "Y-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2345678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "AY"));

    s = "-T";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "123456T8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T"));

    s = "Y-T";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y1234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "YT"));

    s = "-T-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1T345678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12T45678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123T5678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234T678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345T78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123456T8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "1234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "Y-T-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2T45678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y23T5678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y234T678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2345T78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y23456T8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "YT345678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "YT34567T"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "YT3456T8"));

    s = "Y???????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2345678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "???????T";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "Y??????T";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y234567T"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "??T?????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12T45678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "???T????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123T5678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "????T???";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234T678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "?????T??";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345T78"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "??????T?";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123456T8"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "T234T6T8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y234678"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345678"));

    s = "Y?";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2"));

    s = "Y?";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2"));

    s = "Y??";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y23"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y223"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23"));

    s = "Y???";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y234"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T234"));

    s = "Y????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y2345"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T2345"));

    s = "Y?????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y23456"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "Y??????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "Y234567"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T234567"));

    s = "???????Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234567Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "??????Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123456Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "?????Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "????Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "???Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "??Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "?Y";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1Y"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "Y22"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "T23456"));

    s = "?L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L345678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L34567"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L3456"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L345"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L34"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L3"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "LL3"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "LTL"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "1T345678"));

    s = "??L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L45678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L4567"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L456"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L45"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L4"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "12L"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "1L4"));

    s = "???L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123L5678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123L567"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123L56"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123L5"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "123L"));

    s = "????L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234L678"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234L67"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "1234444L5"));

    s = "?????L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345L78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345L7"));

    s = "??????L-";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123456L8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "123456L"));

    s = "-L?";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123456L8"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "23456L8"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "3456L8"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "456L8"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "6L8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "L8"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "123456L"));

    s = "-L??";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12345L78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "2345L78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "345L78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "45L78"));
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "5L78"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "L78"));
    CPPUNIT_ASSERT(!RuleUtil::matchFareClassExpression(s.c_str(), "AL7"));

    s = "-L???";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1234L678"));

    s = "-L????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "123L5678"));

    s = "-L?????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "12L45678"));

    s = "-L??????";
    CPPUNIT_ASSERT(RuleUtil::matchFareClassExpression(s.c_str(), "1L345678"));
  }

  void testIsInFFUserGroup()
  {
    LocTypeCode ruleLocType = 'C';
    PaxTypeFare ptf;
    GeoTravelType geoType = GeoTravelType::International;

    LocCode ruleLoc = "CA";
    LocCode vLoc = "CA";
    // zoneInfo will be zero, returning false
    CPPUNIT_ASSERT(!RuleUtil::isInFFUserGroup(*_trx, vLoc, ruleLocType, ruleLoc, ptf, geoType));

    ruleLoc = "US";
    vLoc = "US";
    CPPUNIT_ASSERT(RuleUtil::isInFFUserGroup(*_trx, vLoc, ruleLocType, ruleLoc, ptf, geoType));

    ruleLoc = "ZZ";
    vLoc = "ZZ";
    CPPUNIT_ASSERT(!RuleUtil::isInFFUserGroup(*_trx, vLoc, ruleLocType, ruleLoc, ptf, geoType));
  }

  void testIsInLoc()
  {
    LocTypeCode ruleLocType = GROUP_LOCATION;
    PaxTypeFare ptf;

    LocCode ruleLoc = "CA";
    LocCode vLoc = "CA";
    // if loc type is group, will act same as isInFFUserGroup
    CPPUNIT_ASSERT(!RuleUtil::isInLoc(*_trx, vLoc, ruleLocType, ruleLoc, ptf));

    ruleLoc = "US";
    vLoc = "US";
    CPPUNIT_ASSERT(RuleUtil::isInLoc(*_trx, vLoc, ruleLocType, ruleLoc, ptf));

    ruleLoc = "ZZ";
    vLoc = "ZZ";
    CPPUNIT_ASSERT(!RuleUtil::isInLoc(*_trx, vLoc, ruleLocType, ruleLoc, ptf));

    // ruleLocType is different. It will return LocUtil::isInLoc();
    ruleLocType = LOCTYPE_NATION;
    vLoc = "NYC";
    ruleLoc = "US";
    CPPUNIT_ASSERT(RuleUtil::isInLoc(*_trx, vLoc, ruleLocType, ruleLoc, ptf));

    ruleLoc = "CA";
    CPPUNIT_ASSERT(!RuleUtil::isInLoc(*_trx, vLoc, ruleLocType, ruleLoc, ptf));
  }

  void testGetPrimeRBD_Booking()
  {
    std::string bcTest = "NA";
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBD(*ptf, bkgCodes);
    CPPUNIT_ASSERT(!bkgCodes.empty());
    std::string bcRBD = bkgCodes[0];
    CPPUNIT_ASSERT_EQUAL(bcRBD, bcTest);
  }

  void testGetPrimeRBD_Booking_Empty()
  {
    std::string bcTest ;
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBD(*ptf, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.empty());
  }

  void testGetPrimeRBDrec1_Booking()
  {
    std::string bcTest = "NA";
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBDrec1(*ptf, bkgCodes);
    CPPUNIT_ASSERT(!bkgCodes.empty());
    std::string bcRBD = bkgCodes[0];
    CPPUNIT_ASSERT_EQUAL(bcRBD, bcTest);
  }

  void testGetPrimeRBDrec1_FBR_Specified_Booking_Empty()
  {
    std::string bcTest ;
    PaxTypeFare* ptf25 = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    ptf25->status().set(PaxTypeFare::PTF_FareByRule);
    setFBR(*ptf25, true);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBD(*ptf25, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.empty());
  }

  void testGetPrimeRBDrec1_FBR_Specified_Booking()
  {
    std::string bcTest = "K";
    PaxTypeFare* ptf25 = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    ptf25->status().set(PaxTypeFare::PTF_FareByRule);
    setFBR(*ptf25, true);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBD(*ptf25, bkgCodes);
    CPPUNIT_ASSERT(!bkgCodes.empty());
    std::string bcRBD = bkgCodes[0];
    CPPUNIT_ASSERT_EQUAL(bcRBD, bcTest);
  }

  void testGetPrimeRBDrec1_FBR_Calculated_Base_Booking()
  {
    std::string bcTest ;
    PaxTypeFare* ptf25 = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', bcTest);
    ptf25->status().set(PaxTypeFare::PTF_FareByRule);
    setFBR(*ptf25, false);

    std::vector<BookingCode> bkgCodesBase;
    bkgCodesBase.push_back("K");
    _fbrRuleData->setBaseFarePrimeBookingCode(bkgCodesBase);
    std::vector<BookingCode> bkgCodes;
    RuleUtil::getPrimeRBD(*ptf25, bkgCodes);
    CPPUNIT_ASSERT(!bkgCodes.empty());
  }

  void testMatchCat35Type()
  {
    Indicator displayTypeRule = ' ';
    Indicator displayTypeFare = 'S';
    CPPUNIT_ASSERT(RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'L';
    displayTypeFare = 'L';
    CPPUNIT_ASSERT(RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'L';
    CPPUNIT_ASSERT(RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'T';
    CPPUNIT_ASSERT(RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'C';
    displayTypeFare = 'C';
    CPPUNIT_ASSERT(!RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'C';
    CPPUNIT_ASSERT(!RuleUtil::matchCat35Type(displayTypeRule,displayTypeFare));
  }
  // PRM is set to false
  void testFareValidForRetailerCodeQualiferMatch1()
  {
    PaxTypeFare ptf;

    CPPUNIT_ASSERT(RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }

   // PRM is set to True and all ASL values are populated
  void testFareValidForRetailerCodeQualiferMatch2()
  {
    _request->setPRM(true);
    _trx->setRequest(_request);
    PaxTypeFare ptf;
    
    _adjustedSellingCalcData = new AdjustedSellingCalcData(); 
    ptf.setAdjustedSellingCalcData(_adjustedSellingCalcData);
    _adjustedSellingCalcData->setSourcePcc("ABC1");
    _frri->fareRetailerCode() = "12345";
    _adjustedSellingCalcData->setFareRetailerRuleInfo(_frri);

    ptf.fareDisplayCat35Type() = ' ';

   CPPUNIT_ASSERT(RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }
  
  // PRM is se to true and FareRetailerRuleInfo is not set.
  void testFareValidForRetailerCodeQualiferMatch3()
  {
    _request->setPRM(true);
    _trx->setRequest(_request);
    PaxTypeFare ptf;

    _adjustedSellingCalcData = new AdjustedSellingCalcData();
    ptf.setAdjustedSellingCalcData(_adjustedSellingCalcData);
    _adjustedSellingCalcData->setSourcePcc("ABC1");
    _frri->fareRetailerCode() = "12345";
    _adjustedSellingCalcData->setFareRetailerRuleInfo(0);

    ptf.fareDisplayCat35Type() = ' ';

    CPPUNIT_ASSERT(!RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }
  // More Test cases for Cat35 will be added later to Validate Retailer Code Qualifier Match
  
  void testFareValidForRetailerCodeQualiferMatch4()
  {
    _request->setPRM(true);
    _trx->setRequest(_request);
    PaxTypeFare ptf;

    _adjustedSellingCalcData = new AdjustedSellingCalcData();
    ptf.setAdjustedSellingCalcData(_adjustedSellingCalcData);
    _adjustedSellingCalcData->setFareRetailerRuleInfo(0);

    ptf.fareDisplayCat35Type() = ' ';

    CPPUNIT_ASSERT(!RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }

  void testFareValidForRetailerCodeQualiferMatch5()
  {
    _request->setPRM(true);
    _trx->setRequest(_request);
    PaxTypeFare ptf;

    ptf.fareDisplayCat35Type() = 'C';
    ptf.setRuleData(RuleConst::NEGOTIATED_RULE, _trx->dataHandle(), _negRuleData);
    _negRuleData->fareRetailerCode() = "";
    _negRuleData->sourcePseudoCity() = "ABC2";
    _categRuleItemInfo->setItemNo(11111);
    _negRuleData->categoryRuleItemInfo() = _categRuleItemInfo;

    CPPUNIT_ASSERT(!RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }

  void testFareValidForRetailerCodeQualiferMatch6()
  {
    _request->setPRM(true);
    _trx->setRequest(_request);
    PaxTypeFare ptf;

    ptf.fareDisplayCat35Type() = 'C';
    ptf.setRuleData(RuleConst::NEGOTIATED_RULE, _trx->dataHandle(), _negRuleData);
    _negRuleData->fareRetailerCode() = "12345";
    _negRuleData->sourcePseudoCity() = "ABC2";
    _categRuleItemInfo->setItemNo(11111);
    _negRuleData->categoryRuleItemInfo() = _categRuleItemInfo;

    CPPUNIT_ASSERT(RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, ptf));
  }

  void testMatchPrimeRBD_True()
  {
    std::vector<BookingCode> bcV;
    bcV.push_back("K");
    bcV.push_back("L");
    FareFocusBookingCodeInfo ffbci;
    ffbci.bookingCode() = bcV;

    std::vector<BookingCode> bcRBD;
    bcRBD.push_back("L");
    CPPUNIT_ASSERT(RuleUtil::matchPrimeRBD(&ffbci, bcRBD));
  }

  void testMatchPrimeRBD_False()
  {
    std::vector<BookingCode> bcV;
    bcV.push_back("K");
    bcV.push_back("L");
    FareFocusBookingCodeInfo ffbci;
    ffbci.bookingCode() = bcV;

    std::vector<BookingCode> bcRBD;
    bcRBD.push_back("M");
    CPPUNIT_ASSERT(!RuleUtil::matchPrimeRBD(&ffbci, bcRBD));
  }

  void testGetPrimeRBDrec1()
  {
    PaxTypeFare ptf;
    std::vector<BookingCode> bkgCodes;

    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.empty());

    FareClassAppSegInfo fcasi;
    ptf.fareClassAppSegInfo() = &fcasi;

    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.empty());

    fcasi._bookingCode[0] = "";
    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.empty());

    fcasi._bookingCode[0] = "A";
    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);
    CPPUNIT_ASSERT(bkgCodes.size() == 1);
    CPPUNIT_ASSERT(bkgCodes[0] == "A");

    bkgCodes.clear();
    fcasi._bookingCode[0] = "A";
    fcasi._bookingCode[1] = "B";
    fcasi._bookingCode[2] = "C";
    fcasi._bookingCode[3] = "D";
    fcasi._bookingCode[4] = "E";
    fcasi._bookingCode[5] = "F";
    fcasi._bookingCode[6] = "G";
    fcasi._bookingCode[7] = "H";

    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);

    CPPUNIT_ASSERT(bkgCodes.size() == 8);
    CPPUNIT_ASSERT(bkgCodes[0] == "A");
    CPPUNIT_ASSERT(bkgCodes[1] == "B");
    CPPUNIT_ASSERT(bkgCodes[2] == "C");
    CPPUNIT_ASSERT(bkgCodes[3] == "D");
    CPPUNIT_ASSERT(bkgCodes[4] == "E");
    CPPUNIT_ASSERT(bkgCodes[5] == "F");
    CPPUNIT_ASSERT(bkgCodes[6] == "G");
    CPPUNIT_ASSERT(bkgCodes[7] == "H");

    bkgCodes.clear();
    fcasi._bookingCode[3] = "";
    RuleUtil::getPrimeRBDrec1(ptf, bkgCodes);

    CPPUNIT_ASSERT(bkgCodes.size() == 3);
    CPPUNIT_ASSERT(bkgCodes[0] == "A");
    CPPUNIT_ASSERT(bkgCodes[1] == "B");
    CPPUNIT_ASSERT(bkgCodes[2] == "C");
  }

  void testMatchBookingCode()
  {
    PaxTypeFare ptf;
    DateTime adjustedTicketDate = DateTime::localTime();

    // item no = 0
    CPPUNIT_ASSERT(RuleUtil::matchBookingCode(*_trx, 0, ptf, adjustedTicketDate));

    // empty vector
    CPPUNIT_ASSERT(!RuleUtil::matchBookingCode(*_trx, 999, ptf, adjustedTicketDate));

    FareClassAppSegInfo fcasi;
    fcasi._bookingCode[0] = "A";
    ptf.fareClassAppSegInfo() = &fcasi;

    // non empty vector but no match
    CPPUNIT_ASSERT(!RuleUtil::matchBookingCode(*_trx, 1, ptf, adjustedTicketDate));

    fcasi._bookingCode[0] = "C";
    CPPUNIT_ASSERT(RuleUtil::matchBookingCode(*_trx, 1, ptf, adjustedTicketDate));
  }

  void testMatchTravelRangeX5()
  {
    PaxTypeFare ptf;
    DateTime travelDate(2016, tse::Jun, 3);
    FareMarket* fm = new FareMarket();
    Fare *fare = new Fare();
    PaxType *paxType = new PaxType();
    ptf.initialize(fare, paxType, fm);
    fm->travelDate() = travelDate;

    DateTime startDate(2016, tse::Jun, 12);
    DateTime stopDate(2016, tse::Aug, 12);
    DateTime adjustedTicketDate = DateTime::localTime();

    //  Travel Date is not between Date Range
    setTravelDateRange(startDate, stopDate);
    CPPUNIT_ASSERT(!RuleUtil::matchTravelRangeX5(*_trx, ptf, adjustedTicketDate, _ffdta));

    //  Travel Date is in  between Date Range
    DateTime startDate1(2016, tse::May, 12);
    DateTime stopDate1(2016, tse::Aug, 12);

    setTravelDateRange(startDate1, stopDate1);
    CPPUNIT_ASSERT(RuleUtil::matchTravelRangeX5(*_trx, ptf, adjustedTicketDate, _ffdta));
  }

#ifdef PERFORMANCE_TEST
  void testMatchFareClassStringMultiple()
  {
    boost::chrono::high_resolution_clock::time_point start = boost::chrono::high_resolution_clock::now();

    for (int i = 0; i < PERFORMANCE_ITERATIONS; ++i)
    {
      testMatchFareClassString();
    }

    boost::chrono::milliseconds ms = boost::chrono::duration_cast<boost::chrono::milliseconds>
      (boost::chrono::high_resolution_clock::now() - start);
    std::cout << std::endl << "Timer = " << ms.count() << "ms for ";
    std::cout << PERFORMANCE_ITERATIONS << " iterations."<< std::endl;
  }
#endif

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  TestMemHandle _memHandle;
  FareByRuleItemInfo* _fbrItemInfo;
  FBRPaxTypeFareRuleData* _fbrRuleData;
  FareFocusDaytimeApplDetailInfo* _ffdtda;
  FareFocusDaytimeApplInfo* _ffdta;
  AdjustedSellingCalcData* _adjustedSellingCalcData;
  FareRetailerRuleInfo* _frri; 
  NegPaxTypeFareRuleData* _negRuleData;
  CategoryRuleItemInfo* _categRuleItemInfo;
 // Locations
  //
  const Loc* sfo;
  const Loc* dfw;
  const Loc* dal; // Love Field
  const Loc* sjc; // San Jose
  const Loc* jfk;
  const Loc* nyc;
  const Loc* bos;
  const Loc* lga;
  const Loc* lax;
  const Loc* iah; // Houston
  const Loc* mel;
  const Loc* syd;
  const Loc* hkg; // Hong Kong
  const Loc* nrt; // Tokyo, Narita
  const Loc* mia;
  const Loc* yyz; // Toronto, Canada
  const Loc* yvr; // Vancouver, Canada
  const Loc* lhr; // London
  const Loc* gig; // Brazil
  const Loc* hnl; // Honalulu (Hawaii)
  const Loc* stt; // St Thomas (Virgin Islands)
  const Loc* anc; // Anchorage, Alaska
  const Loc* sju; // San Juan (Puerto Rico)
  const Loc* cdg; // Paris
  const Loc* mex;
  const Loc* tul; // Tulsa
  const Loc* lon; // London
  const Loc* man; // Manchester
  const Loc* pap; // Port Au Prince, Haiti
  const Loc* yvi; // San Jose,Costa Rica
  const Loc* hav; // Havana, Cuba
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTest);
