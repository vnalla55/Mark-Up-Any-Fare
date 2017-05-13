#include <time.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/CabinType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxType.h"
#include "DBAccess/DataHandle.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag411Collector.h"
#include "Diagnostic/Diag400Collector.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "Common/ClassOfService.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Itin.h"
#include "Rules/RuleConst.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

//--- for Fare By Rule creation ---
#include "DBAccess/FareByRuleItemInfo.h"

#include "DBAccess/FareByRuleApp.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"

#include "Common/Config/ConfigMan.h"

using namespace std;
namespace tse
{

class FareBookingCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareBookingCodeValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testSetBkgSegmStatus1);
  CPPUNIT_TEST(testSetBkgSegmStatus2);
  CPPUNIT_TEST(domesticPrimeRBD_and_internationalPrimeRBD_and_LocalMarket);
  CPPUNIT_TEST(testValidateOneFare);
  CPPUNIT_TEST(testfindCxr);
  CPPUNIT_TEST(testCheckBookedClassAvail);
  CPPUNIT_TEST(testValidateWP);
  CPPUNIT_TEST(testProcessFareReturnTrueForIndustryFare);
  CPPUNIT_TEST(testProcessFareReturnTrueForPublishedFare);
  CPPUNIT_TEST(testProcessFareReturnTrueForConstructedFare);
  CPPUNIT_TEST(testProcessFareReturnTrueForDiscountedFare);
  CPPUNIT_TEST(testProcessFareReturnTrueForFareByRuleFare);
  CPPUNIT_TEST(testProcessFareReturnFalseIfFareNotValid);
  CPPUNIT_TEST(testCheckBkgCodeSegmentStatus_Fail_WhenFailOnTagN);
  CPPUNIT_TEST(testSetIgnoreCat31KeepFareFailDoNotSetWhenNotFail);
  CPPUNIT_TEST(testSetIgnoreCat31KeepFareFailSetWhenKeepFareFail);
  CPPUNIT_TEST(testSetPTFBkgStatusRTW);
  CPPUNIT_TEST(testSetFinalBkgStatusRTW);
  CPPUNIT_TEST(testValidateRBDRecord1PriceByCabin);
  CPPUNIT_TEST(testSetFinalStatusForPriceByCabin);
  CPPUNIT_TEST(testPrintDiag400InvalidFM);
  CPPUNIT_TEST(testSetJumpedDownCabinAllowedTrue);
  CPPUNIT_TEST(testSetJumpedDownCabinAllowedFalse);
  CPPUNIT_TEST_SUITE_END();

private:
  CabinType* _firstClassCabin;
  CabinType* _businessClassCabin;
  CabinType* _economyClassCabin;

  PricingTrx* _trx;
  Itin* _itin;
  FareMarket* _fareMarket;
  PaxTypeFare* _paxTypeFare;
  Fare* _fare;
  PaxType* _paxType;
  TestMemHandle _memHandle;
  FareBookingCodeValidator* _fbcValidator;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _itin = _memHandle.create<Itin>();
    _fareMarket = _memHandle.create<FareMarket>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _paxType = _memHandle.create<PaxType>();
    _firstClassCabin = _memHandle.create<CabinType>();
    _businessClassCabin = _memHandle.create<CabinType>();
    _economyClassCabin = _memHandle.create<CabinType>();
    _memHandle.create<TestConfigInitializer>();
    _firstClassCabin->setFirstClass();
    _businessClassCabin->setBusinessClass();
    _economyClassCabin->setEconomyClass();
    _fbcValidator = _memHandle.insert<FareBookingCodeValidator>(
        new FareBookingCodeValidator(*_trx, *_fareMarket, _itin));
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      // FareBookingCodeValidoator fareBookingCodeValidator;
      CPPUNIT_ASSERT(true);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testSetBkgSegmStatus1()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    AirSeg AirSeg;

    _fareMarket->travelSeg().push_back(&AirSeg);
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    TravelSegType segType = Air;
    AirSeg.segmentType() = segType;

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);
  }
  ///////////////////////////////////////////////////////////////////////
  void testSetBkgSegmStatus2()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    AirSeg AirSeg;

    _fareMarket->travelSeg().push_back(&AirSeg);
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    TravelSegType segType = Air;
    AirSeg.segmentType() = segType;

    char num = '1';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void Set_and_CheckStatus_and_SectorPrimeRBD_and_T999_and_Convention()
  {
    AirSeg AirSeg;
    Fare mFare;
    FareInfo mFareInfo;
    Itin* itn1 = new Itin();
    _trx->itin().push_back(itn1);
    PricingOptions opt;

    PricingRequest req;
    _trx->setRequest(&req);

    opt.journeyActivatedForPricing() = false;
    _trx->setOptions(&opt);

    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_Domestic;

    mFare.initialize(Fare::FS_Domestic, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    FareClassAppInfo mockFareClassAppInfo;
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;

    FareClassAppSegInfo mockFareClassAppSegInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;

    mockFareClassAppSegInfo._bookingCodeTblItemNo = 342535;

    _fareMarket->travelSeg().push_back(&AirSeg);
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    TravelSegType segType = Air;
    AirSeg.segmentType() = segType;
    AirSeg.setBookingCode("Y ");

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    CPPUNIT_ASSERT(fBkgVal.CheckBkgCodeSegmentStatus(*_paxTypeFare));

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    mockFareClassAppSegInfo._bookingCode[0] = "Y ";

    std::vector<BookingCode> bkgCodes;

    _paxTypeFare->getPrimeBookingCode(bkgCodes);
//    typedef std::vector<std::vector<ClassOfService*>*> COSPtrVec;
//    typedef std::vector<std::vector<ClassOfService*>*>::iterator COSPtrVecI;
//    typedef std::vector<std::vector<ClassOfService*>*>::const_iterator COSPtrVecIC;
//    typedef std::vector<ClassOfService*>::const_iterator COSInnerPtrVecIC;

    std::vector<ClassOfService*>* cosVec = 0;
    _paxTypeFare->fareMarket()->classOfServiceVec().push_back(cosVec);

    CPPUNIT_ASSERT(
        fBkgVal.validateSectorPrimeRBDRec1(AirSeg,
                                           bkgCodes,
                                           _paxTypeFare->fareMarket()->classOfServiceVec()[0],
                                           *_paxTypeFare,
                                           _paxTypeFare->segmentStatus()[0]));

    CarrierCode Carrier = "AA "; // The BookingCode objects carrier.

    AirSeg.carrier() = Carrier;

    uint16_t number = 1;

    AirSeg.pnrSegment() = number;

//    char convNum = '1';

    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;

    DateTime ltime;
    AirSeg.departureDT() = ltime;

    DataHandle dataHandle;
    fBkgVal._statusType = FareBookingCodeValidator::STATUS_RULE1;

    CPPUNIT_ASSERT(fBkgVal.validateConvention1(*_paxTypeFare, _paxTypeFare->vendor(), &AirSeg));

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;

    mFareInfo._fareTariff = tariff;

    mFareInfo._ruleNumber = rule;

    mFareInfo._vendor = "ATP";

//    convNum = '2';

    bool isRuleZero = false;
    CPPUNIT_ASSERT(fBkgVal.validateConvention2(*_paxTypeFare, *_paxTypeFare, &AirSeg, isRuleZero));
    _trx->setRequest(new PricingRequest());

    PricingRequest pR;

    pR.lowFareNoAvailability() = ' ';

    CPPUNIT_ASSERT(fBkgVal.validateT999(*_paxTypeFare));

    mockFareClassAppSegInfo._bookingCode[0] = "B";

    pR.lowFareNoAvailability() = 'T'; // Low fare No Availability

    CPPUNIT_ASSERT(
        fBkgVal.validateSectorPrimeRBDRec1(AirSeg,
                                           bkgCodes,
                                           _paxTypeFare->fareMarket()->classOfServiceVec()[0],
                                           *_paxTypeFare,
                                           _paxTypeFare->segmentStatus()[0]));

    pR.lowFareNoAvailability() = ' '; // Low fare No Availability
    pR.lowFareRequested() = 'T'; // Low fare No Availability

    CPPUNIT_ASSERT(
        fBkgVal.validateSectorPrimeRBDRec1(AirSeg,
                                           bkgCodes,
                                           _paxTypeFare->fareMarket()->classOfServiceVec()[0],
                                           *_paxTypeFare,
                                           _paxTypeFare->segmentStatus()[0]));
  }
  ////////////////////////////////////////////////////////////////////////////

  void domesticPrimeRBD_and_internationalPrimeRBD_and_LocalMarket()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingOptions opt;
    PricingRequest req;
    PaxTypeInfo mockPaxTypeInfo;

    opt.journeyActivatedForPricing() = false;
    _trx->setOptions(&opt);

    _trx->setRequest(&req);

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    // Travel / AirSeg
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg.setBookingCode("Y ");

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode Carrier = "AA "; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 1;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime ltime;
    airSeg.departureDT() = ltime;

    _fareMarket->travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;
    _fareMarket->governingCarrier() = Carrier;

    // PaxTypeFare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";

    _trx->paxType().push_back(&paxType);
    paxType.paxType() = paxT;
    paxType.number() = 1;

    paxType.paxTypeInfo() = &mockPaxTypeInfo;
    mockPaxTypeInfo.numberSeatsReq() = 1;

    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._bookingCode[0] = "Y ";
    mockFareClassAppInfo._fareType = fareType;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    //  Fare
    // GeoTravelType        geoTravelType = Domestic;

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;
    mFareInfo._fareTariff = tariff;
    mFareInfo._ruleNumber = rule;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;
    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);
    _paxTypeFare->setFare(&mFare);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);
    _trx->setRequest(new PricingRequest());

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    std::vector<TravelSeg*>::iterator iterTvl = _fareMarket->travelSeg().begin();
//    AirSeg& airseg = dynamic_cast<AirSeg&>(**iterTvl);
    fBkgVal._statusType = FareBookingCodeValidator::STATUS_RULE1;

    vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
    _paxTypeFare->getPrimeBookingCode(bkgCodes);

    CPPUNIT_ASSERT(!fBkgVal.validatePrimeRBDRecord1Domestic(*_paxTypeFare, bkgCodes));
//    typedef std::vector<std::vector<ClassOfService*>*>::const_iterator COSPtrVecIC;
    std::vector<ClassOfService*>* cosVec = 0;
    _paxTypeFare->fareMarket()->classOfServiceVec().push_back(cosVec);

// LocalMarket no longer testable as it relies on repricing
//    CPPUNIT_ASSERT(fBkgVal.validateLocalMarket(*_paxTypeFare,
//                                               airseg,
//                                               *iterTvl,
//                                               _paxTypeFare->fareMarket()->classOfServiceVec()[0],
//                                               _paxTypeFare->segmentStatus()[0]));

    CPPUNIT_ASSERT(fBkgVal.validateRBDInternational(*_paxTypeFare));
  }

  ///////////////////////////////////////////////////////////////////////////////////
  void validate_and_validateCxrSpecifiedFareBkgCode()
  {
    DataHandle dataHandle;
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;
    PaxTypeInfo mockPaxTypeInfo;
    PricingOptions po;

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    // Travel / AirSeg
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg.setBookingCode("Y");
    airSeg.bookedCabin() = *_businessClassCabin;
    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    FareClassCode fareClass = "Y26S";
    CarrierCode Carrier = "AA"; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 1;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime ltime;
    airSeg.departureDT() = ltime;

    _fareMarket->travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "BKK";
    dest.loc() = "NYC";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;
    _fareMarket->governingCarrier() = Carrier;
    _fareMarket->direction() = FMDirection::OUTBOUND;
    _fareMarket->governingCarrier() = Carrier;

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinSameCountryExceptUSCA; // International

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";

    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    //  Fare

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";
    Directionality from = FROM;

    mFareInfo._directionality = from;

    mFareInfo._carrier = Carrier;

    mFareInfo._fareTariff = tariff;

    mFareInfo._ruleNumber = rule;

    mFareInfo._fareClass = fareClass;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;
    mFare.status() = Fare::FS_ScopeIsDefined;

    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    _paxTypeFare->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED);
    for (uint16_t a = 1; a <= 35; a++)
    {
      _paxTypeFare->setCategoryProcessed(a);
    }
    _paxTypeFare->setRoutingValid(true);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);

    _trx->paxType().push_back(&paxType);
    paxType.paxType() = paxT;
    paxType.number() = 1;

    paxType.paxTypeInfo() = &mockPaxTypeInfo;
    mockPaxTypeInfo.numberSeatsReq() = 1;

    po.journeyActivatedForPricing() = false;
    _trx->setOptions(&po);

    _trx->itin().push_back(_itin);

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    CPPUNIT_ASSERT(!fBkgVal.validate());

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(*_paxTypeFare));

    // For FBR

    // set another thru fare
    Fare fare11;
    fare11.nucFareAmount() = 2000.20;

    FareInfo fareInfo11;
    fareInfo11._carrier = "BA";
    fareInfo11._market1 = "BKK";
    fareInfo11._market2 = "NYC";
    fareInfo11._fareClass = "Y11";
    fareInfo11._fareAmount = 600.00;
    fareInfo11._currency = "GBP";
    fareInfo11._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo11._ruleNumber = "2000";
    fareInfo11._routingNumber = "XXXX";
    fareInfo11._directionality = FROM;
    fareInfo11._globalDirection = GlobalDirection::AT;
    fareInfo11._vendor = "FMS";

    TariffCrossRefInfo tariffRefInfo11;
    tariffRefInfo11._fareTariffCode = "TAFPBA";
    tariffRefInfo11._tariffCat = 0;
    tariffRefInfo11._ruleTariff = 304;
    tariffRefInfo11._routingTariff1 = 99;
    tariffRefInfo11._routingTariff2 = 96;

    fare11.initialize(Fare::FS_International, &fareInfo11, *_fareMarket, &tariffRefInfo11);

    PaxType paxType11;
    PaxTypeCode paxTypeCode11 = "ADT";
    paxType11.paxType() = paxTypeCode11;

    PaxTypeFare pxTypeFare11;
    pxTypeFare11.initialize(&fare11, &paxType11, _fareMarket);
    pxTypeFare11.status().set(PaxTypeFare::PTF_FareByRule);

    FareClassAppInfo appInfo11;
    appInfo11._fareType = "XEX";
    appInfo11._pricingCatType = 'S';
    pxTypeFare11.fareClassAppInfo() = &appInfo11;

    FareClassAppSegInfo fareClassAppSegInfo11;
    fareClassAppSegInfo11._paxType = "ADT";
    fareClassAppSegInfo11._carrierApplTblItemNo = 0;
    fareClassAppSegInfo11._bookingCodeTblItemNo = 123456;
    fareClassAppSegInfo11._bookingCode[0] = "Y";
    pxTypeFare11.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    pxTypeFare11.cabin() = *_economyClassCabin;

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(_paxTypeFare);
    paxTypeCortege1.paxTypeFare().push_back(&pxTypeFare11);
    _fareMarket->paxTypeCortege().push_back(paxTypeCortege1);

    FareByRuleApp fbrApp;
    fbrApp.vendor() = "ATP";
    fbrApp.carrier() = "AA";
    fbrApp.segCnt() = 1;
    fbrApp.primePaxType() = "ADT";

    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.fareInd() = RuleConst::ADD_SPECIFIED_TO_CALCULATED;
    fbrItemInfo.percent() = 100;
    fbrItemInfo.specifiedFareAmt1() = 23.00;
    fbrItemInfo.specifiedCur1() = "GBP";

    CategoryRuleInfo cRI;
    cRI.carrierCode() = "AA";

    FBRPaxTypeFareRuleData fbrPaxTypeFareRuleData;
    fbrPaxTypeFareRuleData.ruleItemInfo() = &fbrItemInfo;
    fbrPaxTypeFareRuleData.fbrApp() = &fbrApp;
    fbrPaxTypeFareRuleData.categoryRuleInfo() = &cRI;

    PaxTypeFare basePaxTypeFare;
    fbrPaxTypeFareRuleData.baseFare() = &basePaxTypeFare;
    TariffCrossRefInfo baseTariffRefInfo;
    baseTariffRefInfo._tariffCat = 0; // public tariff
    Fare baseFare;
    FareInfo baseFareInfo;
    baseFare.initialize(Fare::FS_International, &baseFareInfo, *_fareMarket, &baseTariffRefInfo);
    basePaxTypeFare.initialize(&baseFare, &paxType, _fareMarket);

    pxTypeFare11.setRuleData(RuleConst::HIP_RULE, dataHandle, &fbrPaxTypeFareRuleData);

    std::vector<TravelSeg*>::iterator iterTvl = _fareMarket->travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = _fareMarket->travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      pxTypeFare11.segmentStatus().push_back(segStat);
    }

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = *_economyClassCabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = *_firstClassCabin;

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 2;
    cos2.cabin() = *_businessClassCabin;

    //    FIRST_CLASS,
    //    BUSINESS_CLASS,
    //    ECONOMY_CLASS,

    vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    vector<ClassOfService*> _cos1;
    _cos1.push_back(&cos);
    _cos1.push_back(&cos1);
    _cos1.push_back(&cos2);

    _fareMarket->classOfServiceVec().clear();
    _fareMarket->classOfServiceVec().push_back(&_cos);
    _fareMarket->classOfServiceVec().push_back(&_cos1);

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare11));

    ///////////////////////////////////////////////////////////////////////////////

    pxTypeFare11.fareMarket()->fareBasisCode() = pxTypeFare11.fareClass().c_str();

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare11));

    ///////////////////////////////////////////////////////////////////////////////

    pxTypeFare11.fareMarket()->fareBasisCode() = _paxTypeFare->fareClass().c_str();

    // FBR pointer is != 0

    pxTypeFare11.setRuleData(RuleConst::FARE_BY_RULE, dataHandle, &fbrPaxTypeFareRuleData);

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare11));
    //                                              FareBookingCodeValidator::STATUS_RULE1 ) );

    ///////////////////////////////////////////////////////////////////////////////

    fareClassAppSegInfo11._bookingCodeTblItemNo = 123456;
    fareClassAppSegInfo11._bookingCode[0] = "";
    //    fbrItemInfo.fareInd() = FareByRuleItemInfo::SPECIFIED;             // FBR specified

    PaxTypeFare pxTypeFare13;
    pxTypeFare13.initialize(&fare11, &paxType11, _fareMarket);
    pxTypeFare13.status().set(PaxTypeFare::PTF_FareByRule);
    pxTypeFare13.fareClassAppInfo() = &appInfo11;
    pxTypeFare13.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    pxTypeFare13.cabin() = *_economyClassCabin;

    paxTypeCortege1.paxTypeFare().push_back(&pxTypeFare13);
    //    _fareMarket->paxTypeCortege().push_back(paxTypeCortege1);

    pxTypeFare13.setRuleData(RuleConst::FARE_BY_RULE, dataHandle, &fbrPaxTypeFareRuleData);

    iterTvl = _fareMarket->travelSeg().begin();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      pxTypeFare13.segmentStatus().push_back(segStat);
    }

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare13));
    //                                              FareBookingCodeValidator::STATUS_RULE1 ) );

    ///////////////////////////////////////////////////////////////////////////////

    fareClassAppSegInfo11._bookingCodeTblItemNo = 12345;
    fareClassAppSegInfo11._bookingCode[0].clear();
    fbrItemInfo.fareInd() = FareByRuleItemInfo::SPECIFIED; // FBR specified

    PaxTypeFare pxTypeFare14;
    pxTypeFare14.initialize(&fare11, &paxType11, _fareMarket);
    pxTypeFare14.status().set(PaxTypeFare::PTF_FareByRule);
    pxTypeFare14.status().set(PaxTypeFare::PTF_Discounted);
    pxTypeFare14.fareClassAppInfo() = &appInfo11;
    pxTypeFare14.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    pxTypeFare14.cabin() = *_economyClassCabin;

    paxTypeCortege1.paxTypeFare().push_back(&pxTypeFare14);
    //    _fareMarket->paxTypeCortege().push_back(paxTypeCortege1);

    pxTypeFare14.setRuleData(RuleConst::FARE_BY_RULE, dataHandle, &fbrPaxTypeFareRuleData);
    pxTypeFare14.setRuleData(
        RuleConst::CHILDREN_DISCOUNT_RULE, dataHandle, &fbrPaxTypeFareRuleData);

    DiscountInfo discInfo;
    discInfo.bookingCode() = "Y";

    iterTvl = _fareMarket->travelSeg().begin();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      pxTypeFare14.segmentStatus().push_back(segStat);
    }

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare14));

    ///////////////////////////////////////////////////////////////////////////////

    PaxTypeFare pxTypeFare12;
    pxTypeFare12.initialize(&fare11, &paxType11, _fareMarket);
    pxTypeFare12.status().set(PaxTypeFare::PTF_FareByRule);
    pxTypeFare12.fareClassAppInfo() = &appInfo11;
    pxTypeFare12.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    pxTypeFare12.cabin() = *_economyClassCabin;

    paxTypeCortege1.paxTypeFare().push_back(&pxTypeFare12);

    pxTypeFare12.setRuleData(RuleConst::FARE_BY_RULE, dataHandle, &fbrPaxTypeFareRuleData);

    cRI.carrierCode() = "BA";

    iterTvl = _fareMarket->travelSeg().begin();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      pxTypeFare12.segmentStatus().push_back(segStat);
    }

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare12));

    ///////////////////////////////////////////////////////////////////////////////

    pxTypeFare11.fareMarket()->fareBasisCode() = pxTypeFare11.fareClass().c_str();

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare11));

    ///////////////////////////////////////////////////////////////////////////////

    fareClassAppSegInfo11._bookingCodeTblItemNo = 0;

    pxTypeFare11.fareMarket()->fareBasisCode() = _paxTypeFare->fareClass().c_str();

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(pxTypeFare11));

    ///////////////////////////////////////////////////////////////////////////////

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // Domestic
    mockFareClassAppSegInfo._bookingCode[0] = "B";

    req.lowFareNoAvailability() = 'T'; // Low fare No Availability

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(*_paxTypeFare));

    ///////////////////////////////////////////////////////////////////////////////
    req.lowFareNoAvailability() = ' '; // Low fare No Availability
    req.lowFareRequested() = 'T'; // Low fare No Availability

    CPPUNIT_ASSERT(fBkgVal.validateFareBkgCode(*_paxTypeFare));
  }

  //////////////////////////////////////////////////////////////////////////////////////

  void testValidateIndustryFareBkgCode()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;

    Loc origin;
    Loc dest;

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    CarrierCode bcCarrier = "YY"; // The BookingCode objects carrier.
    FareClassCode fareClass = "Y26S";
    TariffNumber tariff = 10101;
    MoneyAmount fareAmount = 999.99;
    CurrencyNoDec noDec = 2;
    CurrencyCode currency = "USD";
    PaxTypeCode paxT = "ADT";
    RuleNumber rule = "2020";
    Directionality from = FROM;
    VendorCode atpco = "ATP";
    FareType fareType = "ER";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;

    _fareMarket->governingCarrier() = bcCarrier;

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinSameCountryExceptUSCA; // International

    //            Create the fareInfo -- so the Fare can stuff things into it.
    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_IndustryFare;
    mFareInfo._carrier = bcCarrier;
    mFareInfo._fareClass = fareClass;
    mFareInfo._globalDirection = GlobalDirection::US;
    mFareInfo._fareTariff = tariff;
    mFareInfo._currency = currency;
    mFareInfo._noDec = noDec;
    mFareInfo._fareAmount = fareAmount;
    mFareInfo._ruleNumber = rule;
    mFareInfo._directionality = from;
    mFareInfo._vendor = atpco;
    mFareInfo._owrt = ONE_WAY_MAY_BE_DOUBLED;

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._carrierApplTblItemNo = 0;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    // Setters for the Travel Segments

    //  TravelSeg tvlSeg;
    _fareMarket->travelSeg().push_back(&airSeg);

    airSeg.origin() = &origin;

    airSeg.destination() = &dest;

    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    airSeg.carrier() = bcCarrier;

    std::vector<TravelSeg*>::iterator iterTvl = _fareMarket->travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = _fareMarket->travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      _paxTypeFare->segmentStatus().push_back(segStat);
    }

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = *_economyClassCabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = *_firstClassCabin;

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 0;
    cos2.cabin() = *_businessClassCabin;

    //    FIRST_CLASS,
    //    BUSINESS_CLASS,
    //    ECONOMY_CLASS,

    vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    vector<ClassOfService*> _cos1;
    _cos1.push_back(&cos);
    _cos1.push_back(&cos1);
    _cos1.push_back(&cos2);

    _fareMarket->classOfServiceVec().push_back(&_cos);
    _fareMarket->classOfServiceVec().push_back(&_cos1);

    // Set PaxTypeFare

    _paxTypeFare->setFare(&mFare);

    // Set FareMarket with PaxTypeFare

    std::vector<PaxTypeFare*> vecPaxType;

    vecPaxType.push_back(_paxTypeFare);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);

    DataHandle dataHandle;

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    fBkgVal._statusType = FareBookingCodeValidator::STATUS_RULE1;

    CPPUNIT_ASSERT(fBkgVal.validateIndustryFareBkgCode(*_paxTypeFare));
  }
  //////////////////////////////////////////////////////////////////////////////////////
  void testValidateBookingCodeTblItemNo()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;

    Loc origin;
    Loc dest;

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    CarrierCode bcCarrier = "YY"; // The BookingCode objects carrier.
    FareClassCode fareClass = "Y26S";
    TariffNumber tariff = 10101;
    MoneyAmount fareAmount = 999.99;
    CurrencyNoDec noDec = 2;
    CurrencyCode currency = "USD";
    PaxTypeCode paxT = "ADT";
    RuleNumber rule = "2020";
    Directionality from = FROM;
    VendorCode atpco = "ATP";
    FareType fareType = "ER";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;

    _fareMarket->governingCarrier() = bcCarrier;

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinSameCountryExceptUSCA; // International

    //  Create the fareInfo -- so the Fare can stuff things into it.
    mFare.setFareInfo(&mFareInfo);
    mFareInfo._carrier = bcCarrier;
    mFareInfo._fareClass = fareClass;
    mFareInfo._globalDirection = GlobalDirection::US;
    mFareInfo._fareTariff = tariff;
    mFareInfo._currency = currency;
    mFareInfo._noDec = noDec;
    mFareInfo._fareAmount = fareAmount;
    mFareInfo._ruleNumber = rule;
    mFareInfo._directionality = from;
    mFareInfo._vendor = atpco;
    mFareInfo._owrt = ONE_WAY_MAY_BE_DOUBLED;

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._carrierApplTblItemNo = 0;
    mockFareClassAppSegInfo._bookingCodeTblItemNo = 123456;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    // Setters for the Travel Segments

    //  TravelSeg tvlSeg;
    _fareMarket->travelSeg().push_back(&airSeg);

    airSeg.origin() = &origin;

    airSeg.destination() = &dest;

    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    airSeg.carrier() = bcCarrier;

    std::vector<TravelSeg*>::iterator iterTvl = _fareMarket->travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = _fareMarket->travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      _paxTypeFare->segmentStatus().push_back(segStat);
    }

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = *_economyClassCabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = *_firstClassCabin;

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 0;
    cos2.cabin() = *_businessClassCabin;

    //    FIRST_CLASS,
    //    BUSINESS_CLASS,
    //    ECONOMY_CLASS,

    vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    vector<ClassOfService*> _cos1;
    _cos1.push_back(&cos);
    _cos1.push_back(&cos1);
    _cos1.push_back(&cos2);

    _fareMarket->classOfServiceVec().push_back(&_cos);
    _fareMarket->classOfServiceVec().push_back(&_cos1);
    // Set PaxTypeFare

    _paxTypeFare->setFare(&mFare);

    // Set FareMarket with PaxTypeFare

    std::vector<PaxTypeFare*> vecPaxType;

    vecPaxType.push_back(_paxTypeFare);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);

    DataHandle dataHandle;

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    fBkgVal._statusType = FareBookingCodeValidator::STATUS_RULE1;

    CPPUNIT_ASSERT(fBkgVal.validateBookingCodeTblItemNo(*_paxTypeFare));

    // CPPUNIT_ASSERT( fBkgVal.validateBookingCodeTblItemNo(_paxTypeFare, true, 1 ));

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // Domestic

    CPPUNIT_ASSERT(fBkgVal.validateBookingCodeTblItemNo(*_paxTypeFare));

    _fareMarket->fareBasisCode() = fareClass.c_str();

    CPPUNIT_ASSERT(fBkgVal.validateBookingCodeTblItemNo(*_paxTypeFare));
  }

  void testValidateDomesticMixedClass()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;

    Loc origin;
    Loc dest;

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    CarrierCode bcCarrier = "YY"; // The BookingCode objects carrier.
    FareClassCode fareClass = "Y26S";
    TariffNumber tariff = 10101;
    MoneyAmount fareAmount = 999.99;
    CurrencyNoDec noDec = 2;
    CurrencyCode currency = "USD";
    PaxTypeCode paxT = "ADT";
    RuleNumber rule = "2020";
    Directionality from = FROM;
    VendorCode atpco = "ATP";
    FareType fareType = "ER";
    BookingCode bkg = "Y";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;

    _fareMarket->governingCarrier() = bcCarrier;

    _fareMarket->travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // Domestic

    //  Create the fareInfo -- so the Fare can stuff things into it.
    mFare.setFareInfo(&mFareInfo);
    mFareInfo._carrier = bcCarrier;
    mFareInfo._fareClass = fareClass;
    mFareInfo._globalDirection = GlobalDirection::US;
    mFareInfo._fareTariff = tariff;
    mFareInfo._currency = currency;
    mFareInfo._noDec = noDec;
    mFareInfo._fareAmount = fareAmount;
    mFareInfo._ruleNumber = rule;
    mFareInfo._directionality = from;
    mFareInfo._vendor = atpco;
    mFareInfo._owrt = ONE_WAY_MAY_BE_DOUBLED;

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._carrierApplTblItemNo = 0;
    mockFareClassAppSegInfo._bookingCodeTblItemNo = 123456;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    // Setters for the Travel Segments

    //  TravelSeg tvlSeg;
    _fareMarket->travelSeg().push_back(&airSeg);

    airSeg.origin() = &origin;

    airSeg.destination() = &dest;

    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    airSeg.carrier() = bcCarrier;

    std::vector<TravelSeg*>::iterator iterTvl = _fareMarket->travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = _fareMarket->travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      _paxTypeFare->segmentStatus().push_back(segStat);
    }

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = *_economyClassCabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = *_firstClassCabin;

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 0;
    cos2.cabin() = *_businessClassCabin;

    //    FIRST_CLASS,
    //    BUSINESS_CLASS,
    //    ECONOMY_CLASS,

    vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    vector<ClassOfService*> _cos1;
    _cos1.push_back(&cos);
    _cos1.push_back(&cos1);
    _cos1.push_back(&cos2);

    _fareMarket->classOfServiceVec().push_back(&_cos);
    _fareMarket->classOfServiceVec().push_back(&_cos1);

    //    uint16_t num =1;
    //    airSeg.set_number(num);

    // Set PaxTypeFare

    _paxTypeFare->setFare(&mFare);

    // Set FareMarket with PaxTypeFare

    std::vector<PaxTypeFare*> vecPaxType;

    vecPaxType.push_back(_paxTypeFare);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);

    DataHandle dataHandle;

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    CPPUNIT_ASSERT(!fBkgVal.validateDomesticMixedClass(*_paxTypeFare, bkg));
  }

  void testValidateOneFare()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    PaxTypeInfo paxTypeInfo;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;
    PricingOptions opt;

    opt.journeyActivatedForPricing() = false;
    _trx->setOptions(&opt);

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    // Travel / AirSeg
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg.setBookingCode("Y");

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    FareClassCode fareClass = "Y26S";
    CarrierCode Carrier = "AA"; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 1;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime ltime;
    airSeg.departureDT() = ltime;

    _fareMarket->travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;
    _fareMarket->governingCarrier() = Carrier;

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    //  Fare

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;

    mFareInfo._fareTariff = tariff;

    mFareInfo._ruleNumber = rule;

    mFareInfo._fareClass = fareClass;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;
    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;
    // MoneyAmount nucAmount = 0;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);
    _trx->itin().push_back(_itin);

    _trx->paxType().push_back(&paxType);
    paxType.paxType() = paxT;
    paxType.number() = 1;

    paxType.paxTypeInfo() = &paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    // CPPUNIT_ASSERT( fBkgVal.validate( *_paxTypeFare) );

    mFare.status() = Fare::FS_IndustryFare;
    // CPPUNIT_ASSERT( fBkgVal.validate( *_paxTypeFare) );
  }

  void testfindCxr()
  {
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingRequest req;

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    // Travel / AirSeg
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg.setBookingCode("Y");

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    FareClassCode fareClass = "Y26S";
    CarrierCode Carrier = "AA"; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 1;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime ltime;
    airSeg.departureDT() = ltime;

    _fareMarket->travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    _fareMarket->origin() = &origin;
    _fareMarket->destination() = &dest;
    _fareMarket->governingCarrier() = Carrier;

    // Fare
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;
    mockFareClassAppInfo._fareClass = fareClass;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare); // add paxtypefare to the faremarket

    //  Fare

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;

    mFareInfo._fareTariff = tariff;

    mFareInfo._ruleNumber = rule;

    mFareInfo._fareClass = fareClass;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;
    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;
    // MoneyAmount nucAmount = 0;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    // TRX
    _trx->fareMarket().push_back(_fareMarket);

    req.fareClassCode() = fareClass.c_str();
    _trx->setRequest(&req);

    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    CarrierApplicationInfo cai;
    cai.applInd() = '$';
    cai.carrier() = "BA";

    CarrierApplicationInfo cai1;
    cai1.applInd() = '$';
    cai1.carrier() = "BA";

    CarrierApplicationInfo cai2;
    cai2.applInd() = '$';
    cai2.carrier() = "AA";

    std::vector<CarrierApplicationInfo*> Cai;

    Cai.push_back(&cai);
    Cai.push_back(&cai1);
    Cai.push_back(&cai2);

    CPPUNIT_ASSERT(fBkgVal.findCXR(Carrier, Cai));

    cai.applInd() = 'X';
    cai.carrier() = "BA";

    cai1.applInd() = 'X';
    cai1.carrier() = "BA";

    cai2.applInd() = '$';
    cai2.carrier() = "BA";

    CPPUNIT_ASSERT(fBkgVal.findCXR(Carrier, Cai));

    cai.applInd() = 'X';
    cai.carrier() = "BA";

    cai1.applInd() = 'X';
    cai1.carrier() = "BA";

    cai2.applInd() = 'X';
    cai2.carrier() = "AA";

    CPPUNIT_ASSERT(!fBkgVal.findCXR(Carrier, Cai));

    cai.applInd() = 'X';
    cai.carrier() = "BA";

    cai1.applInd() = 'X';
    cai1.carrier() = "BA";

    cai2.applInd() = 'X';
    cai2.carrier() = "BA";

    CPPUNIT_ASSERT(!fBkgVal.findCXR(Carrier, Cai));
  }

  void testCheckBookedClassAvail()
  {
    AirSeg as;
    as.realResStatus() = QF_RES_STATUS;
    PricingRequest prcReq;
    prcReq.lowFareRequested() = 'Y';
    _trx->setRequest(&prcReq);
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    fbcv._rexLocalMarketWPvalidation = false;

    CPPUNIT_ASSERT(fbcv.checkBookedClassAvail(as));
  }

  void testValidateWP()
  {
    AirSeg as;
    as.setBookingCode("C");
    PricingRequest prcReq;
    prcReq.lowFareRequested() = 'N';
    _trx->setRequest(&prcReq);

    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);

    std::vector<BookingCode> bc;
    bc.push_back("A");
    bc.push_back("C");
    bc.push_back("B");

    CPPUNIT_ASSERT(fbcv.validateWP(as, bc));
  }

  void testProcessFareReturnTrueForIndustryFare()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _fare->status().set(Fare::FS_IndustryFare, true);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    CPPUNIT_ASSERT(fbcv.processFare(*_paxTypeFare));
  }

  void testProcessFareReturnTrueForPublishedFare()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _fare->status().set(Fare::FS_PublishedFare, true);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    CPPUNIT_ASSERT(fbcv.processFare(*_paxTypeFare));
  }

  void testProcessFareReturnTrueForConstructedFare()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _fare->status().set(Fare::FS_ConstructedFare, true);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    CPPUNIT_ASSERT(fbcv.processFare(*_paxTypeFare));
  }

  void testProcessFareReturnTrueForDiscountedFare()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _paxTypeFare->status().set(PaxTypeFare::PTF_Discounted);
    CPPUNIT_ASSERT(fbcv.processFare(*_paxTypeFare));
  }

  void testProcessFareReturnTrueForFareByRuleFare()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule);
    CPPUNIT_ASSERT(fbcv.processFare(*_paxTypeFare));
  }

  void testProcessFareReturnFalseIfFareNotValid()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _fare->status() = Fare::FS_InvalidGlobalDirection;
    _paxTypeFare->status() = PaxTypeFare::PTF_Invalid_Fare_Currency;
    //_paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule);
    CPPUNIT_ASSERT(!fbcv.processFare(*_paxTypeFare));
  }

  void testCheckBkgCodeSegmentStatus_Fail_WhenFailOnTagN()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);

    _paxTypeFare->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_TAG_N);
    CPPUNIT_ASSERT_EQUAL(FareBookingCodeValidator::FAIL,
                         fbcv.CheckBkgCodeSegmentStatus(*_paxTypeFare));

    _paxTypeFare->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_TAG_N, false);
    CPPUNIT_ASSERT(FareBookingCodeValidator::PASS != fbcv.CheckBkgCodeSegmentStatus(*_paxTypeFare));
  }

  void testSetIgnoreCat31KeepFareFailDoNotSetWhenNotFail()
  {
    FareBookingCodeValidator fbcv(*_trx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _paxTypeFare->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, false);
    fbcv.setIgnoreCat31KeepFareFail(_paxTypeFare->bookingCodeStatus(), *_paxTypeFare);
    CPPUNIT_ASSERT(!_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE));
  }

  void testSetIgnoreCat31KeepFareFailSetWhenKeepFareFail()
  {
    RexPricingTrx rexTrx;
    FareBookingCodeValidator fbcv(rexTrx, *_fareMarket, _itin);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _paxTypeFare->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, true);
    FareMarket::RetrievalInfo retInfo;
    retInfo._flag = FareMarket::RetrievKeep;
    _paxTypeFare->retrievalInfo() = &retInfo;
    fbcv.setIgnoreCat31KeepFareFail(_paxTypeFare->bookingCodeStatus(), *_paxTypeFare);
    CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE));
  }

  void testSetPTFBkgStatusRTW()
  {
    _trx->getOptions()->setRtw(true);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _fareMarket->fbcUsage() = FILTER_FBC;
    _fbcValidator->setPTFBkgStatus(*_paxTypeFare, FareBookingCodeValidator::MIXED);
    CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED));
    CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT(!_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS));
  }

  void testSetFinalBkgStatusRTW()
  {
    _trx->getOptions()->setRtw(true);
    _paxTypeFare->initialize(_fare, _paxType, _fareMarket);
    _fareMarket->fbcUsage() = FILTER_FBC;
    _fbcValidator->setFinalBkgStatus(*_paxTypeFare);
    CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED));
    CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL));
    CPPUNIT_ASSERT(!_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS));
  }

  void testValidateRBDRecord1PriceByCabin()
  {
    AirSeg AirSeg;
    Fare mFare;
    FareInfo mFareInfo;
    Itin* itn1 = new Itin();
    _trx->itin().push_back(itn1);
    PricingOptions opt;

    PricingRequest req;
    req.lowFareNoAvailability() = 'Y';
    req.setjumpUpCabinAllowed();
    _trx->setRequest(&req);

    opt.journeyActivatedForPricing() = false;
    opt.cabin() = *_firstClassCabin;
    _trx->setOptions(&opt);

    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_Domestic;

    mFare.initialize(Fare::FS_Domestic, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    FareClassAppInfo mockFareClassAppInfo;
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;

    FareClassAppSegInfo mockFareClassAppSegInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;

    mockFareClassAppSegInfo._bookingCodeTblItemNo = 342535;

    _fareMarket->travelSeg().push_back(&AirSeg);
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    TravelSegType segType = Air;
    AirSeg.segmentType() = segType;
    AirSeg.setBookingCode("J ");
    AirSeg.rbdReplaced() = true;

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    mockFareClassAppSegInfo._bookingCode[0] = "Y ";

    std::vector<BookingCode> bkgCodes;

    _paxTypeFare->getPrimeBookingCode(bkgCodes);

    std::vector<ClassOfService*>* cosVec = 0;
    _paxTypeFare->fareMarket()->classOfServiceVec().push_back(cosVec);

    fBkgVal.validateSectorPrimeRBDRec1(AirSeg,
                                       bkgCodes,
                                       _paxTypeFare->fareMarket()->classOfServiceVec()[0],
                                       *_paxTypeFare,
                                       _paxTypeFare->segmentStatus()[0]);
    CPPUNIT_ASSERT(_paxTypeFare->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL));
  }
 // *********************
  void testSetFinalStatusForPriceByCabin()
  {
    AirSeg AirSeg;
    Fare mFare;
    FareInfo mFareInfo;
    Itin* itn1 = new Itin();
    _trx->itin().push_back(itn1);
    PricingOptions opt;

    PricingRequest req;
    req.lowFareNoAvailability() = 'Y';
    _trx->setRequest(&req);

    opt.journeyActivatedForPricing() = false;
    opt.cabin() = *_firstClassCabin;
    _trx->setOptions(&opt);

    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;

    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_Domestic;

    mFare.initialize(Fare::FS_Domestic, &mFareInfo, *_fareMarket, &tariffCrossRefInfo);

    FareClassAppInfo mockFareClassAppInfo;
    _paxTypeFare->fareClassAppInfo() = &mockFareClassAppInfo;

    FareClassAppSegInfo mockFareClassAppSegInfo;
    _paxTypeFare->fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;

    mockFareClassAppSegInfo._bookingCodeTblItemNo = 342535;

    _fareMarket->travelSeg().push_back(&AirSeg);
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);

    TravelSegType segType = Air;
    AirSeg.segmentType() = segType;
    AirSeg.setBookingCode("J ");
    AirSeg.rbdReplaced() = true;

    char num = '0';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    num = '2';

    fBkgVal.SetBkgSegmStatus(*_paxTypeFare, num);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    mockFareClassAppSegInfo._bookingCode[0] = "Y ";

    std::vector<BookingCode> bkgCodes;

    _paxTypeFare->getPrimeBookingCode(bkgCodes);

    std::vector<ClassOfService*>* cosVec = 0;
    _paxTypeFare->fareMarket()->classOfServiceVec().push_back(cosVec);

    fBkgVal.setFinalStatusForPriceByCabin(*_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
  }

  void testPrintDiag400InvalidFM()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic400;
    _trx->diagnostic().activate();
    _fbcValidator->_diag = _memHandle.insert(new Diag400Collector(_trx->diagnostic()));
    _fbcValidator->_diag->activate();
    _fbcValidator->printDiag400InvalidFM();

    std::stringstream expectedDiag;
    expectedDiag
    << "FARE MARKET NOT VALID FOR PRICE BY CABIN - SEE DIAGNOSTIC 185\n";
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _fbcValidator->_diag->str());
  }

  void prepareFareMarket()
  {
    AirSeg* as[2];
    TravelSeg* ts[2];
    for(int i = 0; i < 2; i++)
    {
      as[i] = _memHandle.create<AirSeg>();
      as[i]->carrier() = "AR";
      as[i]->marriageStatus() = 'S';
      //By default all flights are domestic
      as[i]->origin() = _memHandle.create<Loc>();
      const_cast<NationCode&>(as[i]->origin()->nation()) = UNITED_STATES;
      as[i]->destination() = _memHandle.create<Loc>();
      const_cast<NationCode&>(as[i]->destination()->nation()) = UNITED_STATES;
      ts[i] = (TravelSeg*)as[i];
    }

    ts[0]->origAirport() = "BWI";
    ts[0]->destAirport() = "LAX";
    ts[1]->origAirport() = "LAX";
    ts[1]->destAirport() = "SYD";

    ts[0]->legId()=0;
    ts[1]->legId()=1;

    ClassOfService* cos[6];
    for(int i = 0; i < 6; i++)
    {
      cos[i] = _memHandle.create<ClassOfService>();
      cos[i]->numSeats() = 9;
    }
    cos[0]->bookingCode() = 'F'; cos[0]->cabin().setFirstClass();
    cos[1]->bookingCode() = 'Y'; cos[1]->cabin().setEconomyClass();
    cos[2]->bookingCode() = 'B'; cos[2]->cabin().setEconomyClass();
    cos[3]->bookingCode() = 'F'; cos[3]->cabin().setFirstClass();
    cos[4]->bookingCode() = 'J'; cos[4]->cabin().setBusinessClass();
    cos[5]->bookingCode() = 'Y'; cos[5]->cabin().setEconomyClass();
    ts[0]->classOfService().push_back(cos[0]);
    ts[0]->classOfService().push_back(cos[1]);
    ts[0]->classOfService().push_back(cos[2]);
    ts[1]->classOfService().push_back(cos[3]);
    ts[1]->classOfService().push_back(cos[4]);
    ts[1]->classOfService().push_back(cos[5]);

    _fareMarket->travelSeg().push_back(ts[0]);
    _fareMarket->travelSeg().push_back(ts[1]);
  }

  void testSetJumpedDownCabinAllowedTrue()
  {
    prepareFareMarket();
    Fare mFare;
    PaxType paxType;
    CabinType* _leg0Cabin = _memHandle.create<CabinType>();
    _leg0Cabin->setPremiumBusinessClass();
    CabinType* _leg1Cabin = _memHandle.create<CabinType>();
    _leg1Cabin->setPremiumBusinessClass();

    _trx->legPreferredCabinClass().push_back(*_leg0Cabin);
    _trx->legPreferredCabinClass().push_back(*_leg1Cabin);
    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);
    _paxTypeFare->fareMarket() = _fareMarket;
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);
    fBkgVal.setJumpedDownCabinAllowedStatus(*_paxTypeFare);
    CPPUNIT_ASSERT(_paxTypeFare->jumpedDownCabinAllowed());
  }

  void testSetJumpedDownCabinAllowedFalse()
  {
    prepareFareMarket();
    Fare mFare;
    PaxType paxType;
    CabinType* _leg0Cabin = _memHandle.create<CabinType>();
    _leg0Cabin->setEconomyClass();
    CabinType* _leg1Cabin = _memHandle.create<CabinType>();
    _leg1Cabin->setEconomyClass();

    _trx->legPreferredCabinClass().push_back(*_leg0Cabin);
    _trx->legPreferredCabinClass().push_back(*_leg1Cabin);
    _paxTypeFare->initialize(&mFare, &paxType, _fareMarket);
    _paxTypeFare->fareMarket() = _fareMarket;
    FareBookingCodeValidator fBkgVal(*_trx, *_fareMarket, _itin);
    fBkgVal.setJumpedDownCabinAllowedStatus(*_paxTypeFare);
    CPPUNIT_ASSERT(!_paxTypeFare->jumpedDownCabinAllowed());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareBookingCodeValidatorTest);
}
