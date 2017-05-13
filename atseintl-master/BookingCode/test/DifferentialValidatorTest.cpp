#include "DataModel/PaxTypeFare.h"
#include "BookingCode/DifferentialValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Differentials.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestIndustryPricingApplFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestPricingRequestFactory.h"
#include "test/testdata/TestPricingUnitFactory.h"

using namespace std;

namespace tse
{
class HIPMinimumFare;

class MockDifferentialValidator : public DifferentialValidator
{
  vector<const IndustryPricingAppl*> _lipa;
  mutable IndustryPricingAppl _ipa;
  bool _returnVal;
  MoneyAmount _hipMoneyAmount;

  RepricingTrx* getRepricingTrx(const std::vector<TravelSeg*>& tvlSeg,
                                FMDirection fmDirectionOverride) const
  {
    return 0;
  }

public:
  MockDifferentialValidator()
    : _returnVal(false),
      _hipMoneyAmount(0.0),
      _applyPremEconCabinDiffCalc(false),
      _applyPremBusCabinDiffCalc(false),
      _allowPremiumCabinSlide(false) {};
  ~MockDifferentialValidator() {};
  bool
  checkDifferentialFare(const PaxTypeFare& tf, const PaxTypeFare& df, const DifferentialData& diffD)
      const
  {
    return true;
  };

  bool
  validatePuRules(const PaxTypeFare& paxTF,
                  std::vector<uint16_t> puCategories,
                  bool catPU) const
  {
    return true;
  }

  const vector<const IndustryPricingAppl*>& getIndustryPricingAppl(const CarrierCode& carrier,
                                                                   const GlobalDirection& globalDir,
                                                                   const DateTime& date) const
  {
    (const_cast<vector<const IndustryPricingAppl*>&>(_lipa)).clear();
    if (carrier == "SQ")
    {
      IndustryPricingAppl* i1 =
          TestIndustryPricingApplFactory::create("testdata/PC1/IndustryPricingAppl1.xml");
      IndustryPricingAppl* i2 =
          TestIndustryPricingApplFactory::create("testdata/PC1/IndustryPricingAppl2.xml");
      IndustryPricingAppl* i3 =
          TestIndustryPricingApplFactory::create("testdata/PC1/IndustryPricingAppl3.xml");
      (const_cast<vector<const IndustryPricingAppl*>&>(_lipa)).push_back(i1);
      (const_cast<vector<const IndustryPricingAppl*>&>(_lipa)).push_back(i2);
      (const_cast<vector<const IndustryPricingAppl*>&>(_lipa)).push_back(i3);
      return _lipa;
    }
    static Indicator ind('L');

    _ipa.primePricingAppl() = ind;
    (const_cast<vector<const IndustryPricingAppl*>&>(_lipa)).push_back(&_ipa);
    return _lipa;
  }

  bool
  diffSectorValidation(DifferentialData* diffData, const std::vector<Differentials*>& differList)
  {
    PaxTypeFare* fA = new PaxTypeFare();
    Fare* fareA = new Fare();
    PaxType paxTypeA;
    FareInfo finf_A;
    fareA->setFareInfo(&finf_A);
    FareMarket fareMarketA;
    FareUsage fu;

    fA->initialize(fareA, &paxTypeA, &fareMarketA);
    const_cast<CarrierCode&>(fA->carrier()) = "UA";
    const_cast<MoneyAmount&>(fA->nucFareAmount()) = 123.F;
    ((FareInfo*)fA->fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fA->fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fA->fare()->fareInfo())->_routingNumber = "059";
    fA->cabin().setFirstClass();
    fA->bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;

    fareMarketA.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    finf_A._ruleNumber = "0";
    finf_A._fareAmount = 0.0;
    finf_A._currency = "USD";
    finf_A._fareClass = "Y";
    finf_A._carrier = "AA";
    finf_A._vendor = Vendor::ATPCO;
    finf_A._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    finf_A._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    fareA->initialize(Fare::FS_Domestic, &finf_A, fareMarketA, &tcri);

    fareA->mileageSurchargePctg() = 0;

    PaxTypeFare fBA;
    Fare fareBA;
    PaxType paxTypeBA;
    FareInfo finf_BA;
    fareBA.setFareInfo(&finf_BA);
    FareMarket fareMarketBA;

    fareBA.mileageSurchargePctg() = 0;

    fBA.initialize(&fareBA, &paxTypeBA, &fareMarketBA);
    const_cast<CarrierCode&>(fBA.carrier()) = "UA";
    const_cast<MoneyAmount&>(fBA.nucFareAmount()) = 234.F;
    ((FareInfo*)fBA.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fBA.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fBA.fare()->fareInfo())->_routingNumber = "059";
    fBA.cabin().setBusinessClass();
    fBA.setFare(&fareBA);

    diffData->fareHigh() = fA;

    diffData->fareLow() = &fBA;
    diffData->fareHigh()->cabin().setBusinessClass();
    diffData->fareHigh()->mileageSurchargePctg() = 0;
    diffData->amount() = 0.0f;
    diffData->amountFareClassHigh() = 75.0f;
    diffData->amountFareClassLow() = 10.0f;

    return (MockDifferentialValidator::returnVal());
  }

  bool getHip(const Itin& itin,
              DifferentialData& diffItem,
              PaxTypeFare& paxTfare,
              const PaxType& reqPaxType)
  {
    return true;
  }

  MoneyAmount hipProcess(HIPMinimumFare& hip,
                         PaxTypeFare& paxTypeFare,
                         const Itin& itin,
                         const PaxType& reqPaxType,
                         DiagCollector& diag)
  {
    return _hipMoneyAmount;
  }
  const bool returnVal(void)
  {
    return _returnVal;
  };
  void setReturnVal(const bool rv)
  {
    _returnVal = rv;
  };

  const MoneyAmount hipMoneyAmount(void)
  {
    return _hipMoneyAmount;
  };
  void setHipMoneyAmount(const MoneyAmount hipMoneyAmount)
  {
    _hipMoneyAmount = hipMoneyAmount;
  };

  bool _applyPremEconCabinDiffCalc;
  bool _applyPremBusCabinDiffCalc;
  bool _allowPremiumCabinSlide;
  bool applyPremEconCabinDiffCalc(const CarrierCode& cxr) const
  {
    return _applyPremEconCabinDiffCalc;
  }
  bool applyPremBusCabinDiffCalc(const CarrierCode& cxr) const
  {
    return _applyPremBusCabinDiffCalc;
  }
  bool allowPremiumCabinSlide(const CarrierCode& cxr) const { return _allowPremiumCabinSlide; }
};
struct PremiumCabin1TestData
{
  FareMarket* _fm;
  FareMarket* _fmth;
  PaxTypeFare* _through;
  PaxTypeFare* _ptf1;
  PaxTypeFare* _ptf2;
  PaxTypeFare* _ptf4;
  PaxTypeFare* _ptf5;
  PaxTypeFare* _ptf7;
  PaxTypeFare* _ptf8;
  PaxType* _reqPT;
  FareUsage* _fu;
  PricingUnit* _pu;
  PricingTrx _trx;
  PricingOptions _opt;
  PricingRequest* _req;
  DifferentialData _dd;
  MockDifferentialValidator _diffV;
  std::vector<PaxTypeFare*> _ptfVec;
};

namespace
{
class MyDataHandle : public DataHandleMock
{
  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    return DataHandleMock::getFareCalcConfig('C', "", "");
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "CDG")
      return "PAR";
    else if (locCode == "SIN")
      return "SIN";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
};
}

class DifferentialValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DifferentialValidatorTest);
  //-->SR
  CPPUNIT_TEST(testValidateFareTypeDesignators);
  CPPUNIT_TEST(testSetDiffLowHigh);
  CPPUNIT_TEST(testFareSelection);
  CPPUNIT_TEST(testValidateMiscFareTag);
  CPPUNIT_TEST(testCheckDifferentialFare);
  CPPUNIT_TEST(testValidateConsecutiveSectorsForThroughFareType);
  CPPUNIT_TEST(testCompareCabinsPassesForBusinessClass);
  CPPUNIT_TEST(testCompareCabinsPassesForPremiumBusinessClass);
  CPPUNIT_TEST(testCompareCabinsPassesForEconomyClass);
  CPPUNIT_TEST(testCompareCabinsPassesForPremiumEconomyClass);
  CPPUNIT_TEST(testCompareCabinsPassesForFirstClass);
  CPPUNIT_TEST(testCompareCabinsPassesForPremiumFirstClass);
  CPPUNIT_TEST(testCompareCabinsFailsWithNoClassesOfService);
  CPPUNIT_TEST(testCompareCabinsFailsWithCabinMismatch);
  CPPUNIT_TEST(testCompareCabinsFailsWithBookingCodeMismatch);
  CPPUNIT_TEST(testCompareFareMarkets_1);
  CPPUNIT_TEST(testCompareFareMarkets_2);

  CPPUNIT_TEST(testValidateConsecutiveSectors);
  CPPUNIT_TEST(testCheckFareType);
  CPPUNIT_SKIP_TEST(testConsolidateTwoDifferentialSectors);
  CPPUNIT_TEST(testConsecutiveSectorsFoundInSequence);
  CPPUNIT_TEST(testCheckTagFareType);
  CPPUNIT_TEST(testFindCombinedFareType);
  CPPUNIT_TEST(testFindEndElementInNextSequence);
  //<--SR
  CPPUNIT_TEST(testCheckBkgCodeSegmentStatus);
  CPPUNIT_TEST(testAnalyzeFailBkgCode);
  CPPUNIT_TEST(testCurrentSectorLocalFareRBDValidation);
  CPPUNIT_TEST(testMatchIntermTypeCxrBkg);
  CPPUNIT_TEST(testMatchLocation1);

  CPPUNIT_TEST(testMatchGeo1);
  CPPUNIT_TEST(testMatchGeo2);

  CPPUNIT_TEST(testMatchThroughBkgCode);
  CPPUNIT_SKIP_TEST(testMatchLoc1A2A1B2B);
  CPPUNIT_SKIP_TEST(testValidateFlightAppL);
  CPPUNIT_TEST(testGetBookingCode);
  CPPUNIT_TEST(testDefineLowFareCabin);
  CPPUNIT_TEST(testDefineHighFareCabin);

  CPPUNIT_TEST(testGetHip);
  CPPUNIT_TEST(testcheckDifferentialVector);
  CPPUNIT_TEST(testcopyDifferentialData);
  CPPUNIT_TEST(testAdjacentSectorDetermination);
  CPPUNIT_TEST(testhighAndLowFound);
  CPPUNIT_TEST(testcountStopsCarriers);
  CPPUNIT_TEST(testConsolidateDifferentials);
  CPPUNIT_TEST(testConsolidate);
  CPPUNIT_TEST(testAdjustRexPricingDates_retrieval);
  CPPUNIT_TEST(testAdjustRexPricingDates_travel);
  CPPUNIT_TEST(testRestorePricingDates);

  CPPUNIT_TEST(testCabinTypeFromFTD);
  CPPUNIT_TEST(testCabinTypeToFTD);
  CPPUNIT_TEST(testCabinTypeFromHip);
  CPPUNIT_TEST(testCabinTypeToHip);

  CPPUNIT_TEST(testGetLowerCabinPremium);

  CPPUNIT_TEST(testProcessNormalFareSelectionFirstHigh);
  CPPUNIT_TEST(testProcessNormalFareSelectionPremiumBusinessHigh);
  CPPUNIT_TEST(testProcessNormalFareSelectionBusinessHigh);
  CPPUNIT_TEST(testProcessNormalFareSelectionPremiumEconomyHigh);
  CPPUNIT_TEST(testProcessNormalFareSelectionEconomyHigh);
  CPPUNIT_TEST(testProcessWPNCFareSelectionFirst);
  CPPUNIT_TEST(testProcessWPNCFareSelectionPremiumBusiness);
  CPPUNIT_TEST(testProcessWPNCFareSelectionBusiness);
  CPPUNIT_TEST(testProcessWPNCFareSelectionPremiumEconomy);
  CPPUNIT_TEST(testProcessWPNCFareSelectionEconomy);
  CPPUNIT_TEST(testProcessNormalFareSelectionFirstLow);
  CPPUNIT_TEST(testProcessNormalFareSelectionPremiumBusinessLow);
  CPPUNIT_TEST(testProcessNormalFareSelectionBusinessLow);
  CPPUNIT_TEST(testProcessNormalFareSelectionPremiumEconomyLow);

  CPPUNIT_TEST(testGetIndustryPrimePricingPrecedence);

  CPPUNIT_TEST(testValidateFareTypeDesignatorsPC);
  CPPUNIT_TEST(testValidatePuRulesNoCat14);
  CPPUNIT_TEST(testIsFareValidForFareSelection_falseForTagNPTF);

  CPPUNIT_TEST(testIsPremiumEconomyCabin_True);
  CPPUNIT_TEST(testIsPremiumEconomyCabin_False);

  CPPUNIT_TEST(testValidateHighFareForPremiumEconomyCabin_True);
  CPPUNIT_TEST(testValidateHighFareForPremiumEconomyCabin_False);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testCheckBkgCodeSegmentStatus()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    FareUsage fu;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
      fu.segmentStatus().push_back(segStat);
    }
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    fu.paxTypeFare() = &paxTfare;

    FareClassCode fareClass = "Y";
    diffVal.throughFare() = &paxTfare;
    diffVal._mkt = &fareMarket;
    diffVal.fareUsage() = &fu;

    CPPUNIT_ASSERT(diffVal.CheckBkgCodeSegmentStatus());
  }

  void testAnalyzeFailBkgCode()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    FareUsage fu;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    fu.paxTypeFare() = &paxTfare;

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    //++++++++++++++
    AirSeg airSeg1;

    Loc origin1;
    Loc dest1;
    origin1.loc() = "DEN";
    dest1.loc() = "LON";
    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("B");
    fareMarket.travelSeg().push_back(&airSeg1);

    //++++++++++++++++++

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest1;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_International, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";
    fcasi._bookingCode[2] = "B";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
      fu.segmentStatus().push_back(segStat);
    }
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    FareClassCode fareClass = "Y";

    trx.fareMarket().push_back(&fareMarket);

    req.fareClassCode() = fareClass.c_str();
    trx.setRequest(&req);

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin().setEconomyClass();

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin().setFirstClass();

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 0;
    cos2.cabin().setBusinessClass();

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

    fareMarket.classOfServiceVec().push_back(&_cos);
    fareMarket.classOfServiceVec().push_back(&_cos1);

    int number = 1;
    diffVal.throughFare() = &paxTfare;
    diffVal._mkt = &fareMarket;
    diffVal.fareUsage() = &fu;
    DifferentialData::FareTypeDesignators ftDes = DifferentialData::ECONOMY_FTD;
    CPPUNIT_ASSERT(diffVal.analyzeFailBkgCode(number, ftDes));
  }

  void testCurrentSectorLocalFareRBDValidation()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTypeFare;
    MockDifferentialValidator diffVal;
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    PricingTrx trx;
    PricingRequest req;
    PricingOptions po;

    po.iataFares() = 'N';

    trx.setOptions(&po);
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.diffTrx()->setOptions(&po);

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

    CarrierCode Carrier = "AA "; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 0;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime ltime;
    airSeg.departureDT() = ltime;

    fareMarket.travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;
    fareMarket.governingCarrier() = Carrier;
    fareMarket.boardMultiCity() = "MIA";
    fareMarket.offMultiCity() = "DFW";

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_International, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";
    fcasi._bookingCode[2] = "B";

    fcai._pricingCatType = 'N';

    paxTypeFare.cabin().setEconomyClass();

    paxTypeFare.fareTypeApplication() = 'N';
    paxTypeFare.fareTypeDesignator().setFTDUnknown();

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";
    fcasi._paxType = paxT;
    fcasi._bookingCode[0] = "Y";
    fcai._fareType = fareType;

    paxTypeFare.fareClassAppInfo() = &fcai;

    paxTypeFare.fareClassAppSegInfo() = &fcasi;

    paxTypeFare.initialize(&mFare, &paxType, &fareMarket);

    paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);

    PaxTypeBucket ptc;
    ptc.requestedPaxType() = &paxType;
    ptc.paxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv;
    ptcv.push_back(ptc);
    fareMarket.paxTypeCortege() = ptcv;

    //  Fare

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;
    mFareInfo._fareTariff = tariff;
    mFareInfo._ruleNumber = rule;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;

    mFareInfo._vendor = Vendor::ATPCO;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, fareMarket, &tariffCrossRefInfo);
    // TRX
    FareClassCode fareClass = "Y";

    trx.fareMarket().push_back(&fareMarket);

    req.fareClassCode() = fareClass.c_str();
    trx.setRequest(&req);

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();

    diffVal.throughFare() = &paxTypeFare;
    diffVal._mkt = &fareMarket;
    diffVal.requestedPaxType() = &paxType;
    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //+++++++++++++++++
    paxTypeFare.fareTypeApplication() = 'S';

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //+++++++++++++++++++

    paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //+++++++++++++++++++

    airSeg.carrier() = "BA";

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //==================================================
    paxTypeFare.fareTypeApplication() = 'N';

    airSeg.setBookingCode("B");

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //+++++++++++++++++
    paxTypeFare.fareTypeApplication() = 'S';

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //==================================================
    paxTypeFare.fareTypeApplication() = 'N';

    airSeg.setBookingCode("F");

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));

    //+++++++++++++++++
    paxTypeFare.fareTypeApplication() = 'S';

    CPPUNIT_ASSERT(!diffVal.currentSectorLocalFareRBDValidation(number));
  }

  //===================================================================================

  void testMatchIntermTypeCxrBkg()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;

    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;

    DifferentialData diffSeg;
    Differentials diffTable;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';
    FareType fareType = "ER";
    fcai._fareType = fareType;
    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
    }
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    FareClassCode fareClass = "Y";

    //++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CarrierCode cr0("");
    vector<CarrierCode> crl0;
    crl0.push_back(cr0);
    diffSeg.carrier() = crl0;
    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*E";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*B";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(!diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//    paxTfare.cabin().setPremiumEconomyClass();
    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*W";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::PREMIUM_ECONOMY_FTD;
    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

//    paxTfare.cabin().setPremiumEconomyClass();
    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*Z";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::PREMIUM_ECONOMY_FTD;
    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "ZEX";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::PREMIUM_ECONOMY_FTD;
    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "WU";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::PREMIUM_ECONOMY_FTD;
    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*Z";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(!diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "*";
    diffTable.intermedBookingCode() = "";
    diffSeg.cabin() = DifferentialData::PREMIUM_ECONOMY_FTD;
    CPPUNIT_ASSERT(!diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

//

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "Y";
    diffSeg.bookingCode() = "Y";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "J";
    diffSeg.bookingCode() = "J";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "J";
    diffSeg.bookingCode() = "B";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(!diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl0, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "AA";
    CarrierCode cr("AA");
    vector<CarrierCode> crl;
    crl.push_back(cr);
    diffSeg.carrier() = crl;
    diffSeg.bookingCode() = "J";
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffTable.intermedCarrier() = "AA";
    crl.clear();
    cr = "BA";
    crl.push_back(cr);
    diffSeg.carrier() = crl;
    diffSeg.cabin() = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT(!diffVal.matchIntermTypeCxrBkg(
        diffSeg, crl, diffSeg.bookingCode(), diffSeg.cabin(), diffTable));
  }

  void testMatchGeo1()
  {
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);

    DateTime ltime;
    req.ticketingDT() = ltime;

    DifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;

    LocKey loc1k;
    LocKey loc2k;
    Loc origin;
    Loc destination;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    destination.cityInd() = true;

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchGeo(loc1k, loc1k.loc(), origin));

    //  CPPUNIT_ASSERT( ! diffVal.matchGeo( loc2k, loc2k.loc(), origin ));

    CPPUNIT_ASSERT(diffVal.matchGeo(loc2k, loc2k.loc(), destination));
  }
  //////////////////////////////////////////////////////////////////////////////////////////////
  void testMatchGeo2()
  {
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);

    DateTime ltime;
    req.ticketingDT() = ltime;

    DifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;

    LocKey loc1k;
    LocKey loc2k;
    Loc origin;
    Loc destination;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    destination.cityInd() = true;

    //++++++++++++++++++++++++++++++++++

    Indicator ind = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    //+++++++++++++++++++++++++++++++++++++

    ind = 'O';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    //+++++++++++++++++++++++++++++++++++

    ind = 'B';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    //++++++++++++++++++++++++++++++++++

    ind = 'W';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    destination.loc() = "DEN";
    destination.area() = "1";
    destination.nation() = "US";
    destination.cityInd() = true;

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(
        diffVal.matchGeo(loc1k, loc1k.loc(), loc2k, loc2k.loc(), origin, destination, ind));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  void testMatchLocation1()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;

    Fare mFare;
    PaxType mPaxType;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);

    DateTime ltime;
    req.ticketingDT() = ltime;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    DifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;

    DifferentialData diffSeg;

    LocKey loc1k;
    LocKey loc2k;
    Loc origin;
    Loc destination;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    destination.cityInd() = true;

    //++++++++++++++++++++++++++++++++++

    diffSeg.origin() = &origin;
    diffSeg.destination() = &destination;

    //++++++++++++++++++++++++++++++++++

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = FROM;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';
    FareType fareType = "ER";
    fcai._fareType = fareType;
    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    Indicator ind = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    diffVal.throughFare() = &paxTfare;
    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    //+++++++++++++++++++++++++++++++++++++

    ind = 'O';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    //+++++++++++++++++++++++++++++++++++

    ind = 'B';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "LON";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    //++++++++++++++++++++++++++++++++++

    ind = 'W';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    destination.loc() = "DEN";
    destination.area() = "1";
    destination.nation() = "US";
    destination.cityInd() = true;

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, ind));

    //++++++++++++++++++++++++++++++++

    AirSeg airSeg;
    AirSeg airSeg1;

    airSeg.origin() = &origin;
    airSeg.destination() = &destination;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    diffVal._mkt = &fareMarket;
    CPPUNIT_ASSERT(!diffVal.matchLocation(loc1k, loc1k.loc()));

    airSeg1.origin() = &origin;
    airSeg1.destination() = &destination;
    fareMarket.travelSeg().push_back(&airSeg1);

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc()));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    ind = 'F';

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), fareMarket, ind));

    ind = 'B';

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), fareMarket, ind));

    ind = 'W';

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), fareMarket, ind));

    ind = 'O';

    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), fareMarket, ind));

    //+++++++++++++++++++++++  ITIN  ++++++++++++++++

    Itin itin;
    itin.travelSeg().push_back(&airSeg);
    itin.travelSeg().push_back(&airSeg1);

    trx.itin().push_back(&itin);

    ind = 'F';
    CPPUNIT_ASSERT(diffVal.matchLocation(loc1k, loc1k.loc(), loc2k, loc2k.loc(), fareMarket, ind));
  }

  //////////////////////////////////////////////////////////////////////////////////

  void testMatchThroughBkgCode()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    AirSeg airSeg1;
    FareUsage fu;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    fu.paxTypeFare() = &paxTfare;

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    Loc origin, origin1;
    Loc dest, dest1;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
      fu.segmentStatus().push_back(segStat);
    }
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;

    FareClassCode fareClass = "Y";

    diffVal.throughFare() = &paxTfare;
    diffVal._mkt = &fareMarket;
    diffVal.fareUsage() = &fu;
    CPPUNIT_ASSERT(!diffVal.matchThroughBkgCode(airSeg.getBookingCode()));

    //+++++++++++++++++++++++++++++

    origin1.loc() = "DEN";
    dest1.loc() = "MIA";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("Y");
    airSeg1.segmentType() = segType;

    fareMarket.travelSeg().push_back(&airSeg1);
    fareMarket.destination() = &dest1;

    iterTvl = fareMarket.travelSeg().begin();
    iterTvlEnd = fareMarket.travelSeg().end();

    paxTfare.segmentStatus().clear();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
      fu.segmentStatus().push_back(segStat);
    }

    CPPUNIT_ASSERT(diffVal.matchThroughBkgCode(airSeg.getBookingCode()));

    airSeg1.setBookingCode(BookingCode('B'));

    CPPUNIT_ASSERT(diffVal.matchThroughBkgCode(airSeg1.getBookingCode()));
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //=========================================================================

  void testMatchLoc1A2A1B2B()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    DifferentialData diffSeg;

    Differentials diffTable;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    AirSeg airSeg1;

    TravelSegType segType = Air;
    FareClassCode fareClass = "Y";

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    LocKey loc1k;
    LocKey loc2k;
    Loc origin, origin1;
    Loc destination, destination1;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    origin1.loc() = "MIA";
    origin1.area() = "1";
    origin1.nation() = "US";
    origin1.state() = "FL";
    origin1.cityInd() = true;

    destination.loc() = "DEN";
    destination.area() = "1";
    destination.nation() = "US";
    destination.state() = "CO";
    destination.cityInd() = true;

    destination1.loc() = "LON";
    destination1.area() = "2";
    destination1.nation() = "GB";
    destination1.cityInd() = true;

    //++++++++++++++++++++++++++++++++++

    uint16_t number = 1, num1 = 2;

    airSeg.origin() = &origin;
    airSeg.destination() = &destination;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");
    airSeg.segmentType() = segType;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    fareMarket.travelSeg().push_back(&airSeg);

    airSeg1.origin() = &destination;
    airSeg1.destination() = &origin1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("Y");
    airSeg1.segmentType() = segType;
    airSeg1.segmentOrder() = num1;
    airSeg1.pnrSegment() = num1;

    //+++++++ Diff Segment
    diffSeg.origin() = &origin;
    diffSeg.destination() = &destination;

    diffSeg.travelSeg().push_back(&airSeg);

    //+++++

    fareMarket.travelSeg().push_back(&airSeg1);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &origin1;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    FareType fareType = "ER";
    fcai._fareType = fareType;

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;
    paxTfare.segmentStatus().clear();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
    }

    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "";

    diffVal.throughFare() = &paxTfare;
    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'O';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'B';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_NATION;
    loc2k.loc() = "US";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'W';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(
        diffVal.matchLoc1A2A1B2B(loc1k, loc1k.loc(), loc2k, loc2k.loc(), diffSeg, diffTable));
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //=========================================================================

  void testValidateFlightAppL()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    DifferentialData diffSeg;

    Differentials diffTable;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    AirSeg airSeg1;

    TravelSegType segType = Air;
    FareClassCode fareClass = "Y";

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    LocKey loc1k;
    LocKey loc2k;
    Loc origin, origin1;
    Loc destination, destination1;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    origin1.loc() = "MIA";
    origin1.area() = "1";
    origin1.nation() = "US";
    origin1.state() = "FL";
    origin1.cityInd() = true;

    destination.loc() = "DEN";
    destination.area() = "1";
    destination.nation() = "US";
    destination.state() = "CO";
    destination.cityInd() = true;

    destination1.loc() = "LON";
    destination1.area() = "2";
    destination1.nation() = "GB";
    destination1.cityInd() = true;

    //++++++++++++++++++++++++++++++++++

    uint16_t number = 1, num1 = 2;

    airSeg.origin() = &origin;
    airSeg.destination() = &destination;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");
    airSeg.segmentType() = segType;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    fareMarket.travelSeg().push_back(&airSeg);

    airSeg1.origin() = &destination;
    airSeg1.destination() = &origin1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("Y");
    airSeg1.segmentType() = segType;
    airSeg1.segmentOrder() = num1;
    airSeg1.pnrSegment() = num1;

    //+++++++ Diff Segment
    diffSeg.origin() = &origin;
    diffSeg.destination() = &destination;

    diffSeg.travelSeg().push_back(&airSeg);

    //+++++

    fareMarket.travelSeg().push_back(&airSeg1);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &origin1;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    FareType fareType = "ER";
    fcai._fareType = fareType;

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;
    paxTfare.segmentStatus().clear();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
    }

    //=====================================================================

    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    diffVal.throughFare() = &paxTfare;
    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'O';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(!diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(!diffVal.validateFlightAppL(diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'B';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_NATION;
    loc2k.loc() = "US";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'W';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    //++++++++++++++++++++++++++++++++

    diffTable.flightAppl() = 'L';
    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'O';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(!diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "MIA";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(!diffVal.validateFlightAppL(diffSeg, diffTable));

    //+++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'B';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_NATION;
    loc2k.loc() = "US";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'W';

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_AREA;
    loc2k.loc() = "1";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));

    loc1k.locType() = LOCTYPE_NATION;
    loc1k.loc() = "US";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    CPPUNIT_ASSERT(diffVal.validateFlightAppL(diffSeg, diffTable));
    //++++++++++++++++++++++++++++++++
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////
  void testFareSelection()
  {
    PricingTrx trx;
    MockDifferentialValidator diffVal;
    PricingUnit pu;
    PricingRequest req;
    FarePath fp;
    diffVal.farePath() = &fp;

    pu.puType() = PricingUnit::Type::ROUNDTRIP;
    diffVal.pricingUnit() = &pu;

    trx.setRequest(&req);
    PricingOptions opt;
    opt.iataFares() = 'N';
    trx.setOptions(&opt);
    diffVal.diffTrx() = &trx;
    diffVal.diffTrx()->setOptions(&opt);

    TariffCrossRefInfo tariffCrossRefInfo;
    tariffCrossRefInfo.tariffCat() = 0;

    PaxType reqPaxType;
    diffVal.requestedPaxType() = &reqPaxType;
    reqPaxType.paxType() = "ADT";

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";

    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "LON";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg);
    fareMarket.travelSeg() = tsv_1;

    // 1 ... Low fare
    PaxTypeFare lf;
    Fare farel;
    PaxType paxTypel;
    FareInfo finf_l;
    farel.initialize(Fare::FS_International, &finf_l, fareMarket, &tariffCrossRefInfo);

    lf.initialize(&farel, &paxTypel, &fareMarket);
    const_cast<CarrierCode&>(lf.carrier()) = "AA";
    const_cast<MoneyAmount&>(lf.nucFareAmount()) = 123.F;
    ((FareInfo*)lf.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)lf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)lf.fare()->fareInfo())->_routingNumber = "059";
    lf.cabin().setEconomyClass();
    DifferentialData diffDPl;
    vector<FareMarket*> fareMarkets;
    fareMarkets.push_back(&fareMarket);
    diffDPl.fareMarket() = fareMarkets;
    diffDPl.calculationIndicator() = 'S';

    AirSeg airs_l;
    vector<TravelSeg*> vts_l;
    vts_l.push_back(&airs_l);
    diffDPl.travelSeg() = vts_l;

    vector<CarrierCode> carriers1;
    carriers1.push_back(bcCarrier);
    diffDPl.carrier() = carriers1;

    // 2 ... High fare
    PaxTypeFare hf;
    Fare fareh;
    PaxType paxTypeh;
    FareInfo finf_h;
    FareUsage fu;
    fu.differentialPlusUp().clear();

    fareh.initialize(Fare::FS_International, &finf_h, fareMarket, &tariffCrossRefInfo);

    hf.initialize(&fareh, &paxTypeh, &fareMarket);
    const_cast<CarrierCode&>(hf.carrier()) = "AA";
    const_cast<MoneyAmount&>(hf.nucFareAmount()) = 234.F;
    ((FareInfo*)hf.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)hf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)hf.fare()->fareInfo())->_routingNumber = "059";
    hf.cabin().setBusinessClass();
    hf.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDPh;
    diffDPh.fareMarket() = fareMarkets;
    diffDPh.calculationIndicator() = 'S';

    AirSeg airs_h;
    vector<TravelSeg*> vts_h;
    vts_h.push_back(&airs_h);
    diffDPh.travelSeg() = vts_h;
    vector<CarrierCode> carriers2;
    carriers2.push_back(bcCarrier);
    diffDPh.carrier() = carriers2;

    // 3 ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.initialize(Fare::FS_Domestic, &finf_t, fareMarket, &tariffCrossRefInfo);
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_2;
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "AA";
    tf.cabin().setEconomyClass();
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    vector<PaxTypeFare*> v_f;
    v_f.push_back(&lf);
    v_f.push_back(&hf);

    PaxTypeBucket ptc;
    ptc.requestedPaxType() = &paxTypet;
    ptc.paxTypeFare().push_back(&lf); // add paxtypefare to the faremarket
    ptc.paxTypeFare().push_back(&hf); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv;
    ptcv.push_back(ptc);
    fareMarket.paxTypeCortege() = ptcv;
    Loc destination;
    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;

    trx.fareMarket().push_back(&fareMarket);
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDPl);
    vecDiffData.push_back(&diffDPh);

    paxTypet.paxType() = "ADT";

    fu.differentialPlusUp() = vecDiffData;
    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    diffVal.throughFare() = &tf;

    vector<Differentials*> differList;

    bool rv;
    rv = diffVal.fareSelection(diffDPh);
    CPPUNIT_ASSERT(!rv);
    vector<DifferentialData*>::iterator ie = fu.differentialPlusUp().end();
    fu.differentialPlusUp().erase(--ie);

    vector<PaxTypeFare*>::iterator ief = fareMarket.paxTypeCortege()[0].paxTypeFare().end();
    fareMarket.paxTypeCortege()[0].paxTypeFare().erase(--ief);
    {
      DifferentialDataPtrVecI itDiffItem = fu.differentialPlusUp().begin();

      for (; itDiffItem != fu.differentialPlusUp().end(); itDiffItem++)
      {
        (*itDiffItem)->amountFareClassLow() = 0.f;
        (*itDiffItem)->amountFareClassHigh() = 0.f;
        (*itDiffItem)->fareLow() = NULL;
        (*itDiffItem)->fareHigh() = NULL;
      }
    }
    rv = diffVal.fareSelection(diffDPh);
    CPPUNIT_ASSERT(!rv);
  }

  void testValidateMiscFareTag()
  {
    PaxTypeFare paxTfare;
    MiscFareTag mft;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    DateTime ltime = DateTime(2010, 3, 9, 8, 15, 0);
    req.ticketingDT() = ltime;
    mft.diffcalcInd() = 'N';
    paxTfare.miscFareTag() = &mft;

    DifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;
    bool rv = diffVal.validateMiscFareTag(paxTfare);
    CPPUNIT_ASSERT(!rv);
    mft.diffcalcInd() = 'Y';
    rv = diffVal.validateMiscFareTag(paxTfare);
    CPPUNIT_ASSERT(rv);
  }

  void testValidateFareTypeDesignators()
  {
    PricingTrx trx;
    MockDifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;
    PricingRequest req;
    trx.setRequest(&req);
    PricingOptions opt;
    opt.iataFares() = 'N';
    trx.setOptions(&opt);
    diffVal.diffTrx()->setOptions(&opt);

    TariffCrossRefInfo tariffCrossRefInfo;
    tariffCrossRefInfo.tariffCat() = 0;

    vector<Differentials*> differList;

    Indicator diffCalculation = 'S';

    PaxType reqPaxType;
    diffVal.requestedPaxType() = &reqPaxType;
    reqPaxType.paxType() = "ADT";

    PaxTypeBucket ptc;
    vector<PaxTypeBucket> ptc_v;
    ptc_v.push_back(ptc);

    // Differential segment...
    FareMarket fareMarket; // H&L are in the same market
    fareMarket.paxTypeCortege() = ptc_v;
    fareMarket.governingCarrier() = "AA";
    trx.fareMarket().push_back(&fareMarket);
    // 1 ... Low fare
    PaxTypeFare lf;
    Fare farel;
    PaxType paxTypel;
    FareInfo finf_l;
    farel.initialize(Fare::FS_International, &finf_l, fareMarket, &tariffCrossRefInfo);

    lf.initialize(&farel, &paxTypel, &fareMarket);
    const_cast<MoneyAmount&>(lf.nucFareAmount()) = 123.F;
    ((FareInfo*)lf.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)lf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)lf.fare()->fareInfo())->_routingNumber = "059";
    lf.cabin().setEconomyClass();
    // 2 ... High fare
    PaxTypeFare hf;
    Fare fareh;
    PaxType paxTypeh;
    FareInfo finf_h;
    fareh.initialize(Fare::FS_International, &finf_h, fareMarket, &tariffCrossRefInfo);

    hf.initialize(&fareh, &paxTypeh, &fareMarket);
    const_cast<MoneyAmount&>(hf.nucFareAmount()) = 234.F;
    ((FareInfo*)hf.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)hf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)hf.fare()->fareInfo())->_routingNumber = "059";
    hf.cabin().setBusinessClass();
    hf.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    // 3 ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    fareMarkett.paxTypeCortege() = ptc_v;
    FareInfo finf_t;
    faret.initialize(Fare::FS_International, &finf_t, fareMarket, &tariffCrossRefInfo);

    paxTypet.paxType() = "ADT";

    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "AA";
    tf.cabin().setEconomyClass();
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";

    DifferentialData diffDP;
    vector<FareMarket*> fareMarkets;
    Loc origin;
    Loc destination;
    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;
    fareMarkets.push_back(&fareMarket);

    diffDP.fareMarket() = fareMarkets;
    diffDP.calculationIndicator() = diffCalculation;
    CarrierCode carr = "AA";
    AirSeg airs;
    airs.carrier() = carr; //"AA";
    TravelSeg* trev = dynamic_cast<TravelSeg*>(&airs);
    vector<TravelSeg*> vts;
    vts.push_back(trev);
    diffDP.travelSeg() = vts;

    vector<CarrierCode> carriers;
    carriers.push_back(carr);
    diffDP.carrier() = carriers;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxTypet;
    ptc1.paxTypeFare().push_back(&lf); // add paxtypefare to the faremarket
    ptc1.paxTypeFare().push_back(&hf); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv;
    ptcv.push_back(ptc1);
    fareMarket.paxTypeCortege() = ptcv;

    diffVal.throughFare() = &tf;
    bool rv = diffVal.validateFareTypeDesignators(diffDP, diffDP.fareMarket(), diffDP.carrier());
    CPPUNIT_ASSERT(!rv);
    ////////////
    diffCalculation = 'L';
    diffDP.amountFareClassLow() = 0.F;
    diffDP.amountFareClassHigh() = 0.F;
    diffDP.calculationIndicator() = diffCalculation;
    rv = diffVal.validateFareTypeDesignators(diffDP, diffDP.fareMarket(), diffDP.carrier());
    CPPUNIT_ASSERT(!rv);
    diffCalculation = 'H';
    diffDP.amountFareClassLow() = 0.F;
    diffDP.amountFareClassHigh() = 0.F;
    fareMarket.setGlobalDirection(GlobalDirection::ZZ);
    diffDP.calculationIndicator() = diffCalculation;
    rv = diffVal.validateFareTypeDesignators(diffDP, diffDP.fareMarket(), diffDP.carrier());
    CPPUNIT_ASSERT(!rv);
    diffCalculation = 'C';
    diffDP.amountFareClassLow() = 0.F;
    diffDP.amountFareClassHigh() = 0.F;
    diffDP.calculationIndicator() = diffCalculation;
    rv = diffVal.validateFareTypeDesignators(diffDP, diffDP.fareMarket(), diffDP.carrier());
    CPPUNIT_ASSERT(!rv);
  }
  //////////////////////////////////////////////////////////////////////////////////////////

  void testSetDiffLowHigh()
  {
    DifferentialValidator diffVal;

    PricingUnit pu;
    pu.puType() = PricingUnit::Type::ROUNDTRIP;

    diffVal.pricingUnit() = &pu;

    FareInfo fi;
    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y26";
    fi._carrier = "BA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fi._directionality = FROM;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    DifferentialValidator::FareSelectionData diffDP;
    PaxTypeFare df;
    Fare fare;
    PaxType paxType;
    FareMarket fareMarket;

    fare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);
    df.initialize(&fare, &paxType, &fareMarket);

    const_cast<MoneyAmount&>(df.nucFareAmount()) = 123.F;
    const_cast<FareClassCode&>(df.fareClass()) = "B";
    diffVal.setDiffLowHigh(diffDP, df, DifferentialValidator::LOW);

    const_cast<MoneyAmount&>(df.nucFareAmount()) = 234.F;
    const_cast<FareClassCode&>(df.fareClass()) = "F";

    diffVal.setDiffLowHigh(diffDP, df, DifferentialValidator::HIGH);

    CPPUNIT_ASSERT(true);

    fcasi._tktCode = "*AAA";
    diffVal.setDiffLowHigh(diffDP, df, DifferentialValidator::HIGH);

    CPPUNIT_ASSERT(true);

    fcasi._tktCode = "AAA";
    diffVal.setDiffLowHigh(diffDP, df, DifferentialValidator::HIGH);

    CPPUNIT_ASSERT(true);
  }

  void testCheckDifferentialFare()
  {
    PricingUnit pu;
    pu.puType() = PricingUnit::Type::ONEWAY;
    DifferentialValidator diffVal;
    DifferentialData diffDP;
    // 1
    PaxTypeFare df;
    Fare fare1;
    PaxType paxType1;
    FareMarket fareMarket1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    df.initialize(&fare1, &paxType1, &fareMarket1);
    df.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    // 2
    PaxTypeFare tf;
    Fare fare2;
    PaxType paxType2;
    FareMarket fareMarket2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    tf.initialize(&fare2, &paxType2, &fareMarket2);
    tf.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);

    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    diffDP.tripType() = 'O';
    diffVal.pricingUnit() = &pu;

    vector<Differentials*> differList;

    diffVal.throughFare() = &tf;
    bool rv = diffVal.checkDifferentialFare(df, diffDP);
    CPPUNIT_ASSERT(!rv);

    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    rv = diffVal.checkDifferentialFare(df, diffDP);
    CPPUNIT_ASSERT(!rv);
  }

  void testValidateConsecutiveSectorsForThroughFareType()
  {
    PricingTrx trx;
    PricingOptions opt;
    FareUsage fu;
    PricingRequest req;
    trx.setRequest(&req);
    trx.setOptions(&opt);
    FarePath fp;

    MockDifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;
    diffVal.farePath() = &fp;

    opt.iataFares() = 'N';
    diffVal.diffTrx()->setOptions(&opt);

    TariffCrossRefInfo tariffCrossRefInfo;
    tariffCrossRefInfo.tariffCat() = 0;

    PaxType reqPaxType;
    diffVal.requestedPaxType() = &reqPaxType;
    reqPaxType.paxType() = "ADT";

    bool rv;
    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;
    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";

    PaxTypeBucket ptcf;
    ptcf.requestedPaxType() = &paxType1;
    ptcf.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptcf.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptcf.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptcf.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvf;
    ptcvf.push_back(ptcf);
    fareMarkett.paxTypeCortege() = ptcvf;

    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;
    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;
    ///
    diffVal.throughFare() = &tf;
    diffVal._mkt = &fareMarkett;
    diffVal.requestedPaxType() = &paxType1;
    diffVal.fareUsage() = &fu;

    rv = diffVal.validateConsecutiveSectorsForThroughFareType(vecDiffData);
    CPPUNIT_ASSERT(rv);
  }

  void testCompareCabinsFailsWithNoClassesOfService()
  {
    FareMarket fareMarket;

    AirSeg airSeg;
    fareMarket.primarySector() = &airSeg;

    vector<ClassOfService*> classesOfService;

    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&classesOfService);
    fareMarket.classOfServiceVec() = vss;

    DifferentialValidator::TypeOfCabin businessCabin(DifferentialValidator::CABIN_FAMILY_BUSINESS);
    CPPUNIT_ASSERT(!diffVal.compareCabins(fareMarket, businessCabin));
  }

  void testCompareCabinsPassesForBusinessClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setBusinessClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_BUSINESS);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_BUSINESS;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void testCompareCabinsPassesForPremiumBusinessClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setPremiumBusinessClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_BUSINESS);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_PREMIUM_BUSINESS;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void testCompareCabinsPassesForEconomyClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setEconomyClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_ECONOMY);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_ECONOMY;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void testCompareCabinsPassesForPremiumEconomyClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setPremiumEconomyClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_ECONOMY);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_PREMIUM_ECONOMY;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void testCompareCabinsPassesForFirstClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setFirstClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_FIRST);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_FIRST;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void testCompareCabinsPassesForPremiumFirstClass()
  {
    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setPremiumFirstClass();

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_FIRST);
    verifyCompareCabinsPasses(classOfService, cabin);

    cabin = DifferentialValidator::CABIN_PREMIUM_FIRST;
    verifyCompareCabinsPasses(classOfService, cabin);
  }

  void verifyCompareCabinsPasses(ClassOfService& classOfService,
                                 DifferentialValidator::TypeOfCabin& cabin)
  {
    FareMarket fareMarket;
    AirSeg airSeg = createAirSegWithBookingCode(classOfService.bookingCode());
    fareMarket.primarySector() = &airSeg;

    vector<ClassOfService*> classesOfService;
    addClassOfService(fareMarket, classesOfService, classOfService);

    CPPUNIT_ASSERT(diffVal.compareCabins(fareMarket, cabin));
  }

  void testCompareCabinsFailsWithBookingCodeMismatch()
  {
    FareMarket fareMarket;
    AirSeg airSeg = createAirSegWithBookingCode("AB");
    fareMarket.primarySector() = &airSeg;

    ClassOfService classOfService = createClassOfServiceWithBookingCode("Y");

    vector<ClassOfService*> classesOfService;
    addClassOfService(fareMarket, classesOfService, classOfService);

    DifferentialValidator::TypeOfCabin cabin(DifferentialValidator::CABIN_FAMILY_BUSINESS);
    CPPUNIT_ASSERT(!diffVal.compareCabins(fareMarket, cabin));
  }

  void testCompareCabinsFailsWithCabinMismatch()
  {
    FareMarket fareMarket;
    AirSeg airSeg = createAirSegWithBookingCode("AB");
    fareMarket.primarySector() = &airSeg;

    ClassOfService classOfService = createClassOfServiceWithBookingCode("AB");
    classOfService.cabin().setBusinessClass();

    vector<ClassOfService*> classesOfService;
    addClassOfService(fareMarket, classesOfService, classOfService);

    DifferentialValidator::TypeOfCabin cabin = DifferentialValidator::CABIN_FAMILY_ECONOMY;
    CPPUNIT_ASSERT(!diffVal.compareCabins(fareMarket, cabin));
  }

  AirSeg createAirSegWithBookingCode(const string& bookingCode)
  {
    AirSeg airSeg;
    airSeg.setBookingCode(bookingCode);
    return airSeg;
  }

  ClassOfService createClassOfServiceWithBookingCode(const string& bookingCode)
  {
    ClassOfService classOfService;
    classOfService.bookingCode() = bookingCode;
    return classOfService;
  }

  void addClassOfService(FareMarket& fareMarket,
                         vector<ClassOfService*>& classesOfService,
                         ClassOfService& classOfService)
  {
    classesOfService.push_back(&classOfService);

    vector<vector<ClassOfService*>*> classOfServiceVec;
    classOfServiceVec.push_back(&classesOfService);

    fareMarket.classOfServiceVec() = classOfServiceVec;
  }

  void testCompareFareMarkets_1()
  {
    DifferentialValidator diffVal;
    DifferentialData* rv = NULL, diffData;

    PaxTypeFare df;
    Fare fare;
    PaxType paxType;
    FareMarket fm;
    FareUsage fu;

    vector<FareMarket*> fms;
    fms.push_back(&fm);
    diffData.fareMarket() = fms;
    vector<DifferentialData*> dv;
    dv.push_back(&diffData);
    df.initialize(&fare, &paxType, &fm);

    diffVal.throughFare() = &df;
    diffVal.requestedPaxType() = &paxType;
    diffVal.fareUsage() = &fu;
    try { rv = diffVal.compareFareMarkets(fm, dv, 0); }
    catch (...)
    {
      // "differential" method throws exception, caught here
    }
    fu.differentialPlusUp() = dv;
    rv = diffVal.compareFareMarkets(fm, dv, 0);
    CPPUNIT_ASSERT(rv == 0);
  }

  void testCompareFareMarkets_2()
  {
    DifferentialValidator diffVal;
    DifferentialData* rv = NULL, diffData;

    FareMarket fm;

    vector<FareMarket*> fms;
    fms.push_back(&fm);

    diffData.fareMarket() = fms;
    vector<DifferentialData*> dv;

    rv = diffVal.compareFareMarkets(fm, dv, 0);
    CPPUNIT_ASSERT(rv == NULL);

    dv.push_back(&diffData);

    rv = diffVal.compareFareMarkets(fm, dv, 0);
    CPPUNIT_ASSERT(rv == 0);
  }
  //========================================================================================
  void testValidateConsecutiveSectors()
  {
    PricingTrx trx;
    PricingOptions opt;
    FareUsage fu;
    FarePath fp;

    PricingRequest req;
    trx.setRequest(&req);
    trx.setOptions(&opt);
    MockDifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;
    diffVal.farePath() = &fp;

    opt.iataFares() = 'N';
    diffVal.diffTrx()->setOptions(&opt);

    TariffCrossRefInfo tariffCrossRefInfo;
    tariffCrossRefInfo.tariffCat() = 0;

    PaxType reqPaxType;
    diffVal.requestedPaxType() = &reqPaxType;
    reqPaxType.paxType() = "ADT";

    bool rv;
    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;

    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;

    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";

    PaxTypeBucket ptctt;
    ptctt.requestedPaxType() = &paxType1;
    ptctt.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptctt.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptctt.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptctt.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvtt;
    ptcvtt.push_back(ptctt);
    fareMarkett.paxTypeCortege() = ptcvtt;

    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);
    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    fu.differentialPlusUp() = vecDiffData;

    diffVal.throughFare() = &tf;
    diffVal.requestedPaxType() = &paxType1;
    diffVal.fareUsage() = &fu;

    rv = diffVal.validateConsecutiveSectors();
    CPPUNIT_ASSERT(rv);
  }
  //==================================================================================================
  void testCheckFareType()
  {
    DifferentialValidator diffVal;
    PaxTypeFare diffFare;
    FareType throughFT;

    CPPUNIT_ASSERT(diffVal.checkFareType(throughFT, diffFare, false));
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void testConsolidateTwoDifferentialSectors()
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    PricingUnit pu;
    FarePath fp;
    PricingOptions po;

    po.iataFares() = 'N';

    pu.puType() = PricingUnit::Type::ROUNDTRIP;

    trx.setRequest(&req);
    trx.setOptions(&po);
    diffVal.diffTrx() = &trx;
    diffVal.fareUsage() = &fu;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;
    diffVal.diffTrx()->setOptions(&po);

    DifferentialValidator::ReturnValues rv;
    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out

    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    //       fareMarket1.allPaxTypeFare() = v_f_1;
    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //      fareMarket2.allPaxTypeFare() = v_f_2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";
    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    ///
    fareMarket12.classOfServiceVec() = vss2;

    //      vector<PaxTypeFare*> v_f_2;
    //      v_f_2.push_back (&f2);

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;
    ///:
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    //      vector<PaxTypeFare*> v_f_3;
    //      v_f_3.push_back (&f3);

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    //      fareMarket3.allPaxTypeFare() = v_f_3;
    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    //      fareMarketB1.travelSeg().push_back(&airSegB1);
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;

    //      vector<PaxTypeFare*> v_fB1;
    //      v_fB1.push_back (&fB1);

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    //      fareMarketB1.allPaxTypeFare() = v_fB1;
    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;

    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    diffVal.setReturnVal(true);

    unsigned int jmLeftItem = 0;
    unsigned int jmRightItem = 0;

    vector<Differentials*> differList;
    //      vector<DifferentialData *>::iterator jmLeft  = listDiffData.begin ();
    //      vector<DifferentialData *>::iterator jmRight = listDiffData.begin ();
    DifferentialData* diffItem;

    jmRightItem++;

    // 1...
    FareMarket* mktS = tf.fareMarket();
    tf.fareMarket() = NULL;

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;

    diffVal._mkt = NULL;
    diffVal.throughFare() = &tf;

    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::SUCCEEDED);
    tf.fareMarket() = mktS;
    diffVal._mkt = mktS;
    // 2...
    DifferentialData* dSave = listDiffData[jmLeftItem];
    listDiffData[jmLeftItem] = NULL;
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::FATAL_ERROR);

    listDiffData[jmLeftItem] = dSave;
    // 3...
    jmRightItem = listDiffData.size();

    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::FATAL_ERROR);
    jmRightItem = 0;
    jmRightItem++;
    // 4...
    listDiffData[jmLeftItem]->tag() = string("1ABF");
    listDiffData[jmRightItem]->tag() = string("1BF");
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::END_OF_SEQUENCE);
    listDiffData[jmLeftItem]->tag() = string("1AB");
    listDiffData[jmRightItem]->tag() = string("2AF");
    // 5...
    FareMarket* fmSave = *(listDiffData[jmLeftItem]->fareMarket().begin());
    *(listDiffData[jmLeftItem]->fareMarket().begin()) = NULL;
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::FATAL_ERROR);
    *(listDiffData[jmLeftItem]->fareMarket().begin()) = fmSave;
    // 6...
    listDiffData[jmLeftItem]->tag() = string("abracadabra");
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::FATAL_ERROR);
    listDiffData[jmLeftItem]->tag() = string("1AB");
    // 7...
    listDiffData[jmLeftItem]->tag() = string("1AX");
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::FATAL_ERROR);
    listDiffData[jmLeftItem]->tag() = string("1AB");
    // 8...
    rv = diffVal.consolidateTwoDifferentialSectors(
        listDiffData, differList, jmLeftItem, jmRightItem, diffItem, true, true);
    CPPUNIT_ASSERT(rv == DifferentialValidator::SUCCEEDED);
  }
  //+++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++

  void testConsecutiveSectorsFoundInSequence()
  {
    DifferentialValidator diffVal;
    bool rv;
    bool consecFound = false;
    vector<DifferentialData*> dv;
    //    vector<DifferentialData *>::iterator diffSec;

    unsigned int diffStart = 0;
    unsigned int diffSec = 0;

    // 1st sequence
    DifferentialData diffDP1;
    diffDP1.tag() = string("1AF");
    dv.push_back(&diffDP1);
    DifferentialData diffDP2;
    diffDP2.tag() = string("2AB");
    dv.push_back(&diffDP2);
    DifferentialData diffDP3;
    diffDP3.tag() = string("abracadabra");
    dv.push_back(&diffDP3);

    rv = diffVal.consecutiveSectorsFoundInSequence(dv, diffStart, consecFound);
    CPPUNIT_ASSERT(!rv);

    diffDP3.tag() = string("3AB");
    diffSec = dv.size() - 1;

    // 2nd sequence
    DifferentialData diffDP4;
    diffDP4.tag() = string("1BB");
    dv.push_back(&diffDP4);

    rv = diffVal.consecutiveSectorsFoundInSequence(dv, diffSec, consecFound = false);

    CPPUNIT_ASSERT(rv && !consecFound && (diffSec == 2));

    diffSec = dv.size() - 1;
    DifferentialData diffDP5;
    diffDP5.tag() = string("2BF");
    dv.push_back(&diffDP5);
    DifferentialData diffDP6;
    diffDP6.tag() = string("3BFB");
    dv.push_back(&diffDP6);
    DifferentialData diffDP7;
    diffDP7.tag() = string("4BF");
    dv.push_back(&diffDP7);
    // rv = diffVal.consecutiveSectorsFoundInSequence (dv, diffSec, consecFound);
    // CPPUNIT_ASSERT (rv && consecFound && (*diffSec)->tag() == diffDP4.tag());
  }

  void testCheckTagFareType()
  {
    DifferentialValidator diffVal;

    CPPUNIT_ASSERT(diffVal.checkTagFareType(Indicator(DifferentialData::PREMIUM_FIRST_FTD)) &&
                   diffVal.checkTagFareType(Indicator(DifferentialData::FIRST_FTD)) &&
                   diffVal.checkTagFareType(Indicator(DifferentialData::PREMIUM_BUSINESS_FTD_NEW)) &&
                   diffVal.checkTagFareType(Indicator(DifferentialData::BUSINESS_FTD)) &&
                   diffVal.checkTagFareType(Indicator(DifferentialData::PREMIUM_ECONOMY_FTD)) &&
                   diffVal.checkTagFareType(Indicator(DifferentialData::ECONOMY_FTD)));
    CPPUNIT_ASSERT(!diffVal.checkTagFareType(Indicator('A')));
  }

  void testFindCombinedFareType()
  {
    DifferentialValidator diffVal;
    // new
    DifferentialData::FareTypeDesignators firstP = DifferentialData::PREMIUM_FIRST_FTD;
    DifferentialData::FareTypeDesignators first = DifferentialData::FIRST_FTD;
    DifferentialData::FareTypeDesignators BusP = DifferentialData::PREMIUM_BUSINESS_FTD_NEW;
    DifferentialData::FareTypeDesignators Bus = DifferentialData::BUSINESS_FTD;
    DifferentialData::FareTypeDesignators EcoP = DifferentialData::PREMIUM_ECONOMY_FTD;
    DifferentialData::FareTypeDesignators Eco = DifferentialData::ECONOMY_FTD;

    CPPUNIT_ASSERT_EQUAL(firstP, diffVal.findCombinedFareType(firstP, first, false));
    CPPUNIT_ASSERT_EQUAL(first, diffVal.findCombinedFareType(firstP, first, true));
    CPPUNIT_ASSERT_EQUAL(firstP, diffVal.findCombinedFareType(firstP, BusP, false));
    CPPUNIT_ASSERT_EQUAL(BusP, diffVal.findCombinedFareType(firstP, BusP, true));
    CPPUNIT_ASSERT_EQUAL(firstP, diffVal.findCombinedFareType(firstP, EcoP, false));
    CPPUNIT_ASSERT_EQUAL(EcoP, diffVal.findCombinedFareType(firstP, EcoP, true));
    CPPUNIT_ASSERT_EQUAL(Bus, diffVal.findCombinedFareType(Bus, Eco, false));
    CPPUNIT_ASSERT_EQUAL(Eco, diffVal.findCombinedFareType(Bus, Eco, true));
  }

  void testFindEndElementInNextSequence()
  {
    DifferentialValidator diffVal;
    vector<DifferentialData*> dv;
    // 1st sequence
    DifferentialData diffDP1;
    diffDP1.tag() = string("1AF");
    dv.push_back(&diffDP1);
    DifferentialData diffDP2;
    diffDP2.tag() = string("abracadabra");
    dv.push_back(&diffDP2);
    DifferentialData diffDP3;
    diffDP3.tag() = string("3AB");
    dv.push_back(&diffDP3);

    diffDP2.tag() = string("2AF");
    //    vector<DifferentialData *>::iterator rv = dv.begin();

    unsigned int rv = 0;
    rv = diffVal.findEndElementInNextSequence(rv, dv);
    CPPUNIT_ASSERT(rv == dv.size());

    // 2nd sequence
    DifferentialData diffDP4;
    diffDP4.tag() = string("1BB");
    dv.push_back(&diffDP4);
    DifferentialData diffDP5;
    diffDP5.tag() = string("2BF");
    dv.push_back(&diffDP5);

    rv = 0;
    rv = diffVal.findEndElementInNextSequence(rv, dv);
    CPPUNIT_ASSERT(dv[rv]->tag() == diffDP5.tag());

    // 3rd sequence
    DifferentialData diffDP6;
    diffDP6.tag() = string("1CF");
    dv.push_back(&diffDP6);
    DifferentialData diffDP7;
    diffDP7.tag() = string("2CB");
    dv.push_back(&diffDP7);

    rv = 0;
    rv = diffVal.findEndElementInNextSequence(rv, dv);
    CPPUNIT_ASSERT(dv[rv]->tag() == diffDP5.tag());
  }
  //?????????????????????????????????????????????????????????????
  //??????????????

  void testGetBookingCode(void)
  {
    PricingTrx trx;
    FareUsage fu;
    PricingRequest req;
    req.lowFareRequested() = 'Y';
    trx.setRequest(&req);
    MockDifferentialValidator diffVal;
    diffVal.diffTrx() = &trx;
    bool rv;
    AirSeg airSeg;
    airSeg.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg);
    FareMarket fareMarket1;
    fareMarket1.travelSeg() = tsv_1;
    PaxTypeFare paxTfare;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);
    PaxTypeFare::SegmentStatus segStat;
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    segStat._bkgCodeReBook = "X";
    vector<PaxTypeFare::SegmentStatus> segStatV;
    segStatV.push_back(segStat);
    paxTfare.segmentStatus() = segStatV;
    fu.paxTypeFare() = &paxTfare;
    fu.segmentStatus() = segStatV;
    paxTfare.initialize(&fare1, &paxType1, &fareMarket1);

    BookingCode bc = "Y";
    TravelSeg* ts = &airSeg;

    diffVal.throughFare() = &paxTfare;
    diffVal._mkt = &fareMarket1;
    diffVal.fareUsage() = &fu;

    rv = diffVal.getBookingCode(ts, airSeg, bc);

    CPPUNIT_ASSERT(bc == "X");

    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);

    segStatV.clear();
    segStatV.push_back(segStat);

    rv = diffVal.getBookingCode(ts, airSeg, bc);

    CPPUNIT_ASSERT(rv);

    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
    segStatV.clear();
    segStatV.push_back(segStat);
    AirSeg airSeg_1;
    ts = &airSeg_1;

    rv = diffVal.getBookingCode(ts, airSeg, bc);

    CPPUNIT_ASSERT(!rv);

    paxTfare.initialize(&fare1, &paxType1, 0);

    rv = diffVal.getBookingCode(ts, airSeg, bc);

    CPPUNIT_ASSERT(!rv);
  }

  void testDefineLowFareCabin(void)
  {
    MockDifferentialValidator diffVal;
    DifferentialData diffItem;
    PaxTypeFare paxTfare;
    diffItem.fareLow() = &paxTfare;

    diffItem.fareLow()->cabin().setFirstClass();
    diffVal.defineLowFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinLow() == 'F');

    diffItem.fareLow()->cabin().setBusinessClass();
    diffVal.defineLowFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinLow() == 'C');

    diffItem.fareLow()->cabin().setEconomyClass();
    diffVal.defineLowFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinLow() == 'Y');
  }

  void testDefineHighFareCabin(void)
  {
    MockDifferentialValidator diffVal;
    DifferentialData diffItem;
    PaxTypeFare paxTfare;
    diffItem.fareHigh() = &paxTfare;

    diffItem.fareHigh()->cabin().setFirstClass();
    diffVal.defineHighFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinHigh() == 'F');

    diffItem.fareHigh()->cabin().setBusinessClass();
    diffVal.defineHighFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinHigh() == 'C');

    diffItem.fareHigh()->cabin().setEconomyClass();
    diffVal.defineHighFareCabin(diffItem);
    CPPUNIT_ASSERT(diffItem.hipCabinHigh() == 'Y');
  }

  void testGetHip(void)
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.fareUsage() = &fu;

    Itin itin;
    DifferentialData diffItem;
    diffItem.amountFareClassHigh() = 550.0;
    diffItem.amountFareClassLow() = 250.0;
    PaxType reqPaxType;
    PaxTypeFare f;
    Fare fare;
    PaxType paxType;
    FareInfo finf;
    FareMarket fareMarket;
    fare.setFareInfo(&finf);
    f.initialize(&fare, &paxType, &fareMarket);

    PaxTypeFare fh;
    Fare fareh;
    PaxType paxTypeh;
    FareInfo finfh;
    FareMarket fareMarketh;
    fareh.setFareInfo(&finfh);
    fareh.mileageSurchargePctg() = 25;

    // Sergey: The following is the new code for minFarePlusUp
    //   /*
    MinFarePlusUpItem hipInfoh;
    hipInfoh.baseAmount = 321.00;
    hipInfoh.boardPoint = "DFW";
    hipInfoh.offPoint = "MOS";

    fh.initialize(&fareh, &paxTypeh, &fareMarketh);

    PaxTypeFare fl;
    Fare farel;
    PaxType paxTypel;
    FareInfo finfl;
    FareMarket fareMarketl;
    farel.setFareInfo(&finfl);
    farel.mileageSurchargePctg() = 10;

    // Sergey: The following is the new code for minFarePlusUp
    //   /*
    MinFarePlusUpItem hipInfol;
    hipInfol.baseAmount = 123.00;
    hipInfol.boardPoint = "LEN";
    hipInfol.offPoint = "KIE";

    fl.initialize(&farel, &paxTypel, &fareMarketl);

    diffItem.fareHigh() = &fh;
    diffItem.fareLow() = &fl;

    diffVal.setHipMoneyAmount(333.0);
    fu.paxTypeFare() = &f;
    diffVal.itinerary() = &itin;
    diffVal.throughFare() = &f;
    diffVal.requestedPaxType() = &paxType;
    fu.hipExemptInd() = 'N';

    ///*
    CPPUNIT_ASSERT(diffVal.DifferentialValidator::getHip(diffItem));
    fu.hipExemptInd() = 'N';
    CPPUNIT_ASSERT(diffVal.DifferentialValidator::getHip(diffItem));
    CPPUNIT_ASSERT(diffVal.DifferentialValidator::getHip(diffItem));
    CPPUNIT_ASSERT(diffVal.DifferentialValidator::getHip(diffItem));
    CPPUNIT_ASSERT(diffVal.DifferentialValidator::getHip(diffItem));
    //*/
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void testcheckDifferentialVector()
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    PricingUnit pu;
    FarePath fp;

    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.fareUsage() = &fu;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;

    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out
    TravelSegType segType = Air;
    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.segmentType() = segType;
    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;
    diffDP1.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP1.status() = DifferentialData::SC_PASSED;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.segmentType() = segType;
    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    diffDP2.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP2.status() = DifferentialData::SC_PASSED;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";

    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    diffDP12.tripType() = DifferentialValidator::ROUND_TRIP;
    diffDP12.status() = DifferentialData::SC_PASSED;
    ///
    fareMarket12.classOfServiceVec() = vss2;

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.segmentType() = segType;
    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;
    diffDP3.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP3.status() = DifferentialData::SC_PASSED;
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.segmentType() = segType;
    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;
    diffDP4.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP4.status() = DifferentialData::SC_PASSED;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.segmentType() = segType;
    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;
    diffDPB1.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDPB1.status() = DifferentialData::SC_PASSED;

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.segmentType() = segType;
    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    //      vector<PaxTypeFare*> v_f;
    //      v_f.push_back (&f1);
    //      v_f.push_back (&f2);
    //      v_f.push_back (&f3);
    //      v_f.push_back (&f4);

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;

    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    /*      diffItem->fareHigh() = &f1;
            diffItem->fareLow() = &fB1;
            diffItem->fareHigh()->cabin().setBusinessClass()
            diffItem->fareHigh()->mileageSurchargePctg() = 0;
            diffItem->amount() =  0.0f;
            diffItem->amountFareClassHigh() = 75.0f;
            diffItem->amountFareClassLow() = 10.0f;
    */

    // 1...
    FareMarket* mktS = tf.fareMarket();
    //      tf.fareMarket() = NULL;

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;

    diffVal._mkt = mktS;
    diffVal.throughFare() = &tf;

    CPPUNIT_ASSERT(!diffVal.checkDifferentialVector());
    TravelSegType segTypes = Arunk;
    TravelSegType segType1 = Bus;
    TravelSegType segType2 = Train;
    TravelSegType segType3 = Surface;
    airSeg.segmentType() = segTypes;
    airSeg1.segmentType() = segType1;
    airSeg2.segmentType() = segType2;
    airSeg3.segmentType() = segType3;
    diffDP3.status() = DifferentialData::SC_FAILED;

    CPPUNIT_ASSERT(diffVal.checkDifferentialVector());
  }
  //+++++++++++++++++++++++++++++

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void testcopyDifferentialData()
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    PricingUnit pu;
    FarePath fp;

    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.fareUsage() = &fu;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;

    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out

    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    fareMarket1.direction() = FMDirection::OUTBOUND;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;
    diffDP1.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP1.status() = DifferentialData::SC_FAILED;
    diffDP1.throughFare() = &f1;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;
    fareMarket2.direction() = FMDirection::OUTBOUND;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    diffDP2.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP2.status() = DifferentialData::SC_PASSED;
    diffDP2.throughFare() = &f2;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";

    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    fareMarket12.direction() = FMDirection::INBOUND;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    diffDP12.tripType() = DifferentialValidator::ROUND_TRIP;
    diffDP12.status() = DifferentialData::SC_PASSED;
    diffDP12.throughFare() = &f12;
    ///
    fareMarket12.classOfServiceVec() = vss2;

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    fareMarket3.direction() = FMDirection::INBOUND;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;
    diffDP3.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP3.status() = DifferentialData::SC_FAILED;
    diffDP3.throughFare() = &f3;
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    fareMarket4.direction() = FMDirection::UNKNOWN;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;
    diffDP4.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDP4.status() = DifferentialData::SC_PASSED;
    diffDP4.throughFare() = &f4;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    fareMarketB1.direction() = FMDirection::INBOUND;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;
    diffDPB1.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    diffDPB1.status() = DifferentialData::SC_FAILED;
    diffDPB1.throughFare() = &fB1;

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    fareMarkett.direction() = FMDirection::INBOUND;
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    //      vector<PaxTypeFare*> v_f;
    //      v_f.push_back (&f1);
    //      v_f.push_back (&f2);
    //      v_f.push_back (&f3);
    //      v_f.push_back (&f4);

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;

    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    // 1...
    FareMarket* mktS = tf.fareMarket();

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;

    diffVal._mkt = mktS;
    diffVal.throughFare() = &tf;

    pu.puType() = PricingUnit::Type::UNKNOWN;

    diffVal.copyDifferentialData();

    pu.puType() = PricingUnit::Type::ROUNDTRIP;

    diffVal.copyDifferentialData();

    pu.puType() = PricingUnit::Type::ONEWAY;

    diffVal.copyDifferentialData();

    pu.puType() = PricingUnit::Type::OPENJAW;

    diffVal.copyDifferentialData();
  }
  //+++++++++++++++++++++++++++++

  void testAdjacentSectorDetermination()
  {
    FareMarket fareMarket, fareMarket1, fareMarket2, fareMarket3, fareMarket4, fareMarket5;
    PaxTypeFare paxTypeFare, paxTypeFare1;
    DifferentialValidator diffVal;
    AirSeg airSeg;
    AirSeg airSeg1;
    AirSeg airSeg2;
    Itin itin;
    Fare mFare, mFare1;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    PricingTrx trx;
    DataHandle dataHandle;
    FareUsage fu;
    PricingUnit pu;
    PricingRequest req;
    PricingOptions opt;

    opt.iataFares() = 'N';
    trx.setOptions(&opt);
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;

    Indicator diffNumber = '1'; // start number of differential Tag
    Indicator diffLetter = 'A'; // start letter of differential Tag
    uint16_t thruNumber = 1;
    // Indicator  prevdiffLetter = ' ';        // store prev letter of differential Tag
    // Indicator  previousCabin  = 'Z';
    pu.puType() = PricingUnit::Type::ONEWAY;
    fu.differentialPlusUp().clear();

    int travelSegNumber = 1;

    DifferentialData* diffItem = 0;
    trx.dataHandle().get(diffItem);
    DifferentialData& diffData = *diffItem;

    CarrierCode aCarrier = "AA"; // The BookingCode objects carrier.
    CarrierCode bCarrier = "AA"; // The BookingCode objects carrier.

    TravelSegType segType = Air;
    uint16_t number = 1;
    DateTime ltime;

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "ORD";

    Loc origin1;
    Loc dest1;

    origin1.loc() = "ORD";
    dest1.loc() = "MIA";

    Loc origin2;
    Loc dest2;

    origin2.loc() = "MIA";
    dest2.loc() = "DFW";

    // Travel / AirSeg

    airSeg.segmentType() = segType;
    airSeg.setBookingCode("C");

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.boardMultiCity() = "DFW";
    airSeg.offMultiCity() = "ORD";
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "ORD";

    airSeg.carrier() = aCarrier;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    airSeg.departureDT() = ltime;
    //+++++++++++++++++++++++++++++++++++
    airSeg1.segmentType() = segType;
    airSeg1.setBookingCode("B");

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.boardMultiCity() = "ORD";
    airSeg1.offMultiCity() = "MIA";
    airSeg1.origAirport() = "ORD";
    airSeg1.destAirport() = "MIA";

    airSeg1.carrier() = bCarrier;

    number++;

    airSeg1.segmentOrder() = number;
    airSeg1.pnrSegment() = number;

    airSeg1.departureDT() = ltime + Hours(6);

    //+++++++++++++++++++++++++++++++++++
    airSeg2.segmentType() = segType;
    airSeg2.setBookingCode("F");

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.boardMultiCity() = "MIA";
    airSeg2.offMultiCity() = "DFW";
    airSeg2.origAirport() = "MIA";
    airSeg2.destAirport() = "DFW";

    airSeg2.carrier() = bCarrier;

    number++;

    airSeg2.segmentOrder() = number;
    airSeg2.pnrSegment() = number;

    airSeg2.departureDT() = ltime + Hours(24);

    itin.travelSeg().push_back(&airSeg);
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);

    fareMarket.travelSeg().push_back(&airSeg);
    fareMarket.travelSeg().push_back(&airSeg1);
    fareMarket.travelSeg().push_back(&airSeg2);

    fareMarket1.travelSeg().push_back(&airSeg);

    fareMarket2.travelSeg().push_back(&airSeg);
    fareMarket2.travelSeg().push_back(&airSeg1);

    fareMarket3.travelSeg().push_back(&airSeg1);
    fareMarket3.travelSeg().push_back(&airSeg2);

    fareMarket4.travelSeg().push_back(&airSeg1);

    fareMarket5.travelSeg().push_back(&airSeg2);

    // Intialize the faremarket object with any data you need

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest2;
    fareMarket.governingCarrier() = bCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    fareMarket1.origin() = &origin;
    fareMarket1.destination() = &dest;
    fareMarket1.governingCarrier() = bCarrier;

    fareMarket1.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    fareMarket2.origin() = &origin;
    fareMarket2.destination() = &dest1;
    fareMarket2.governingCarrier() = bCarrier;

    fareMarket2.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    fareMarket3.origin() = &origin1;
    fareMarket3.destination() = &dest2;
    fareMarket3.governingCarrier() = bCarrier;

    fareMarket3.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    fareMarket4.origin() = &origin1;
    fareMarket4.destination() = &dest1;
    fareMarket4.governingCarrier() = bCarrier;

    fareMarket4.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    fareMarket5.origin() = &origin2;
    fareMarket5.destination() = &dest2;
    fareMarket5.governingCarrier() = bCarrier;

    fareMarket5.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y26";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fi._directionality = FROM;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";
    fcasi._bookingCode[2] = "B";

    fcai._pricingCatType = 'N';

    // PaxTypeFare

    paxTypeFare.cabin().setEconomyClass();

    paxTypeFare.fareTypeApplication() = 'N';
    paxTypeFare.fareTypeDesignator().setFTDEconomy();

    FareType fareType = "ER";
    PaxTypeCode paxT = "ADT";
    paxType.paxType() = paxT;
    fcasi._paxType = paxT;

    fcai._fareType = fareType;

    paxTypeFare.fareClassAppInfo() = &fcai;

    paxTypeFare.fareClassAppSegInfo() = &fcasi;
    paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_MIXED);

    paxTypeFare.initialize(&mFare, &paxType, &fareMarket);

    paxTypeFare1.fareTypeApplication() = 'N';
    paxTypeFare1.cabin().setBusinessClass();
    paxTypeFare1.fareTypeDesignator().setFTDBusiness();
    paxTypeFare1.bookingCodeStatus().set(PaxTypeFare::BKS_MIXED);

    paxTypeFare1.initialize(&mFare1, &paxType, &fareMarket);

    fareMarket.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket1.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket2.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket3.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket4.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket5.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    fareMarket.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    fareMarket1.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    fareMarket2.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    fareMarket3.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    fareMarket4.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    fareMarket5.allPaxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    bool a = true;
    PaxTypeFare::SegmentStatus segStat;
    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        if (a)
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
          a = false;
        }
        else
        {
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
          a = true;
        }
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTypeFare.segmentStatus().push_back(segStat);
      paxTypeFare1.segmentStatus().push_back(segStat);
      fu.segmentStatus().push_back(segStat);
    }

    //  Fare
    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = aCarrier;
    mFareInfo._fareTariff = tariff;
    mFareInfo._ruleNumber = rule;
    mFareInfo._fareClass = "Y25";

    mFare1.setFareInfo(&mFareInfo);
    mFare1.status() = Fare::FS_PublishedFare;
    VendorCode atpco = "ATP";

    mFareInfo._vendor = atpco;

    mFare1.initialize(Fare::FS_PublishedFare, &mFareInfo, fareMarket, &tariffCrossRefInfo);

    PaxTypeBucket ptcf;
    ptcf.requestedPaxType() = &paxType;
    ptcf.paxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket
    ptcf.paxTypeFare().push_back(&paxTypeFare1); // add paxtypefare to the faremarket
    ptcf.requestedPaxType() = &paxType;
    vector<PaxTypeBucket> ptcvf;
    ptcvf.push_back(ptcf);

    fareMarket.paxTypeCortege() = ptcvf;
    fareMarket1.paxTypeCortege() = ptcvf;
    fareMarket2.paxTypeCortege() = ptcvf;
    fareMarket3.paxTypeCortege() = ptcvf;
    fareMarket4.paxTypeCortege() = ptcvf;
    fareMarket5.paxTypeCortege() = ptcvf;

    // TRX
    FareClassCode fareClass = "Y";

    trx.fareMarket().push_back(&fareMarket);
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarket5);

    req.fareClassCode() = fareClass.c_str();
    trx.setRequest(&req);

    ClassOfService cos;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin().setEconomyClass();

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin().setFirstClass();

    ClassOfService cos2;

    cos2.bookingCode() = "B";
    cos2.numSeats() = 0;
    cos2.cabin().setBusinessClass();

    ClassOfService cos3;

    cos3.bookingCode() = "M";
    cos3.numSeats() = 10;
    cos3.cabin().setEconomyClass();

    vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    vector<ClassOfService*> _cos1;
    _cos1.push_back(&cos);
    _cos1.push_back(&cos1);
    _cos1.push_back(&cos2);

    vector<ClassOfService*> _cos2;
    _cos2.push_back(&cos);
    _cos2.push_back(&cos1);
    _cos2.push_back(&cos2);

    vector<ClassOfService*> _cos3;
    _cos3.push_back(&cos);
    _cos3.push_back(&cos1);
    _cos3.push_back(&cos3);

    fareMarket.classOfServiceVec().push_back(&_cos);
    fareMarket.classOfServiceVec().push_back(&_cos1);
    fareMarket.classOfServiceVec().push_back(&_cos2);
    fareMarket.classOfServiceVec().push_back(&_cos3);

    fareMarket1.classOfServiceVec().push_back(&_cos);
    fareMarket1.classOfServiceVec().push_back(&_cos1);
    fareMarket1.classOfServiceVec().push_back(&_cos2);
    fareMarket1.classOfServiceVec().push_back(&_cos3);

    fareMarket2.classOfServiceVec().push_back(&_cos);
    fareMarket2.classOfServiceVec().push_back(&_cos1);
    fareMarket2.classOfServiceVec().push_back(&_cos2);
    fareMarket2.classOfServiceVec().push_back(&_cos3);

    fareMarket3.classOfServiceVec().push_back(&_cos);
    fareMarket3.classOfServiceVec().push_back(&_cos1);
    fareMarket3.classOfServiceVec().push_back(&_cos2);
    fareMarket3.classOfServiceVec().push_back(&_cos3);

    fareMarket4.classOfServiceVec().push_back(&_cos);
    fareMarket4.classOfServiceVec().push_back(&_cos1);
    fareMarket4.classOfServiceVec().push_back(&_cos2);
    fareMarket4.classOfServiceVec().push_back(&_cos3);

    fareMarket5.classOfServiceVec().push_back(&_cos);
    fareMarket5.classOfServiceVec().push_back(&_cos1);
    fareMarket5.classOfServiceVec().push_back(&_cos2);
    fareMarket5.classOfServiceVec().push_back(&_cos3);

    diffVal.itinerary() = &itin;

    diffVal._mkt = &fareMarket;
    diffVal.throughFare() = &paxTypeFare;
    diffVal.requestedPaxType() = &paxType;
    diffVal.pricingUnit() = &pu;

    vector<DifferentialData*> vecDiffData;

    fu.differentialPlusUp() = vecDiffData;

    diffVal.fareUsage() = &fu;

    TestConfigInitializer::setValue("USE_BOOKINGCODEEXCEPTION_INDEX", "N", "FARESV_SVC");

    FareBookingCodeValidator fcbv(trx, fareMarket, &itin);

    diffVal.fBCVal() = &fcbv;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, --travelSegNumber, diffNumber, diffLetter, thruNumber));
    travelSegNumber++;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, --travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, ++travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, ++travelSegNumber, diffNumber, diffLetter, thruNumber));
    //======================================================================================

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    origin1.loc() = "DEN";
    dest1.loc() = "LAX";

    origin2.loc() = "LAX";
    dest2.loc() = "DFW";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;

    fareMarket.destination() = &dest2;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    airSeg.setBookingCode("F");

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    diffVal.throughFare() = &paxTypeFare1;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, --travelSegNumber, diffNumber, diffLetter, thruNumber));
    travelSegNumber++;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, --travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, ++travelSegNumber, diffNumber, diffLetter, thruNumber));

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, ++travelSegNumber, diffNumber, diffLetter, thruNumber));
    //======================================================================================

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    origin1.loc() = "DEN";
    dest1.loc() = "LAX";

    origin2.loc() = "LAX";
    dest2.loc() = "DFW";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;

    fareMarket.destination() = &dest2;

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    airSeg.setBookingCode("Y");

    CPPUNIT_ASSERT(!diffVal.adjacentSectorDetermination(
        diffData, travelSegNumber, diffNumber, diffLetter, thruNumber));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void testhighAndLowFound()
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    PricingUnit pu;
    FarePath fp;

    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;

    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out

    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    FareUsage fu1;
    fu1.paxTypeFare() = const_cast<PaxTypeFare*>(&f1);
    fu1.travelSeg().insert(
        fu1.travelSeg().end(), fareMarket1.travelSeg().begin(), fareMarket1.travelSeg().end());

    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;

    vector<DifferentialData*> vecDiffData1;
    vecDiffData1.push_back(&diffDP1);

    fu1.differentialPlusUp() = vecDiffData1;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    FareUsage fu2;
    fu2.paxTypeFare() = const_cast<PaxTypeFare*>(&f2);
    fu2.travelSeg().insert(
        fu2.travelSeg().end(), fareMarket2.travelSeg().begin(), fareMarket2.travelSeg().end());
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    vector<DifferentialData*> vecDiffData2;
    vecDiffData2.push_back(&diffDP2);

    fu2.differentialPlusUp() = vecDiffData2;

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";
    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    FareUsage fu12;
    fu12.paxTypeFare() = const_cast<PaxTypeFare*>(&f12);
    fu12.travelSeg().insert(
        fu12.travelSeg().end(), fareMarket12.travelSeg().begin(), fareMarket12.travelSeg().end());

    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    ///
    diffDP12.status() = DifferentialData::SC_CONSOLIDATED_FAIL;
    fareMarket12.classOfServiceVec() = vss2;

    vector<DifferentialData*> vecDiffData12;
    vecDiffData12.push_back(&diffDP12);

    fu12.differentialPlusUp() = vecDiffData12;

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    FareUsage fu3;
    fu3.paxTypeFare() = const_cast<PaxTypeFare*>(&f3);
    fu3.travelSeg().insert(
        fu3.travelSeg().end(), fareMarket3.travelSeg().begin(), fareMarket3.travelSeg().end());

    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;

    vector<DifferentialData*> vecDiffData3;
    vecDiffData3.push_back(&diffDP3);

    fu3.differentialPlusUp() = vecDiffData3;
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    FareUsage fu4;
    fu4.paxTypeFare() = const_cast<PaxTypeFare*>(&f4);
    fu4.travelSeg().insert(
        fu4.travelSeg().end(), fareMarket4.travelSeg().begin(), fareMarket4.travelSeg().end());

    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;

    vector<DifferentialData*> vecDiffData4;
    vecDiffData4.push_back(&diffDP4);

    fu4.differentialPlusUp() = vecDiffData4;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    FareUsage fufB1;
    fufB1.paxTypeFare() = const_cast<PaxTypeFare*>(&fB1);
    fufB1.travelSeg().insert(
        fufB1.travelSeg().end(), fareMarketB1.travelSeg().begin(), fareMarketB1.travelSeg().end());

    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;

    vector<DifferentialData*> vecDiffDataB1;
    vecDiffDataB1.push_back(&diffDPB1);

    fufB1.differentialPlusUp() = vecDiffDataB1;

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    //      fareMarketB1.allPaxTypeFare() = v_fB1;
    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();

    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    FareUsage fuf;
    fuf.paxTypeFare() = const_cast<PaxTypeFare*>(&tf);
    fuf.travelSeg().insert(
        fuf.travelSeg().end(), fareMarkett.travelSeg().begin(), fareMarkett.travelSeg().end());

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fuf.differentialPlusUp() = vecDiffData;

    //      PaxTypeFare::SegmentStatus segStat;
    //      tf.segmentStatus().push_back(segStat);
    //      fuf.segmentStatus().push_back(segStat);

    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    // 1...
    FareMarket* mktS = tf.fareMarket();
    //      tf.fareMarket() = NULL;

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;
    diffVal.fareUsage() = &fuf;

    diffVal._mkt = mktS;
    diffVal.throughFare() = &tf;

    PaxTypeFare::SegmentStatus segStat;

    std::vector<TravelSeg*>::iterator iterTvl = fareMarkett.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarkett.travelSeg().end();
    int a = 0;
    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();
      a++;
      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }
      tf.segmentStatus().push_back(segStat);
      fuf.segmentStatus().push_back(segStat);
    }

    CPPUNIT_ASSERT(!diffVal.highAndLowFound());
  }

  //=========================================================================

  void testcountStopsCarriers()
  {
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    DifferentialValidator diffVal;
    DifferentialData diffSeg;

    Differentials diffTable;
    Itin itin;
    Fare mFare;
    PricingTrx trx;
    PricingRequest req;
    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    PaxType mPaxType;
    AirSeg airSeg;
    AirSeg airSeg1;

    TravelSegType segType = Air;
    FareClassCode fareClass = "Y";

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';

    LocKey loc1k;
    LocKey loc2k;
    Loc origin, origin1;
    Loc destination, destination1;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    origin1.loc() = "MIA";
    origin1.area() = "1";
    origin1.nation() = "US";
    origin1.state() = "FL";
    origin1.cityInd() = true;

    destination.loc() = "DEN";
    destination.area() = "1";
    destination.nation() = "US";
    destination.state() = "CO";
    destination.cityInd() = true;

    destination1.loc() = "LON";
    destination1.area() = "2";
    destination1.nation() = "GB";
    destination1.cityInd() = true;

    //++++++++++++++++++++++++++++++++++

    uint16_t number = 1, num1 = 2;

    airSeg.origin() = &origin;
    airSeg.destination() = &destination;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");
    airSeg.segmentType() = segType;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    fareMarket.travelSeg().push_back(&airSeg);

    airSeg1.origin() = &destination;
    airSeg1.destination() = &origin1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("Y");
    airSeg1.segmentType() = segType;
    airSeg1.segmentOrder() = num1;
    airSeg1.pnrSegment() = num1;

    //+++++++ Diff Segment
    diffSeg.origin() = &origin;
    diffSeg.destination() = &destination;

    diffSeg.travelSeg().push_back(&airSeg);

    //   diffSeg.travelSeg().push_back(&airSeg1);

    //+++++

    fareMarket.travelSeg().push_back(&airSeg1);

    fareMarket.origin() = &origin;
    fareMarket.destination() = &origin1;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;
    // MoneyAmount nucAmount = 0;

    mFare.initialize(Fare::FS_Domestic,
                     &fi,
                     fareMarket,
                     // nucAmount ,
                     &tcri);
    // mFare.initialize(Fare::FS_Domestic, UnknownGeoTravelType, &fi, &tcri );

    FareType fareType = "ER";
    fcai._fareType = fareType;

    fcasi._bookingCode[0] = "F";
    fcasi._bookingCode[1] = "Y";

    fcai._pricingCatType = 'N';

    paxTfare.fareClassAppInfo() = &fcai;

    paxTfare.fareClassAppSegInfo() = &fcasi;

    paxTfare.cabin().setEconomyClass();

    std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;
    paxTfare.segmentStatus().clear();

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      paxTfare.segmentStatus().push_back(segStat);
    }

    //=====================================================================

    //  diffTable.flightAppl() == 'L';
    //++++++++++++++++++++++++++++++++++
    diffTable.directionality() = 'F';

    loc1k.locType() = LOCTYPE_CITY;
    loc1k.loc() = "DFW";
    loc2k.locType() = LOCTYPE_CITY;
    loc2k.loc() = "DEN";

    diffTable.intermedCarrier() = "";
    diffTable.intermedFareType() = "";
    diffTable.intermedBookingCode() = "";

    diffTable.intermedLoc1a() = loc1k; // LocKey
    diffTable.intermedLoc1a().loc() = loc1k.loc(); // LocKey

    diffTable.intermedLoc2a() = loc2k;
    diffTable.intermedLoc2a().loc() = loc2k.loc();

    diffVal.throughFare() = &paxTfare;
    diffVal.itinerary() = &itin;

    itin.travelSeg().push_back(&airSeg);
    itin.travelSeg().push_back(&airSeg1);

    trx.itin().push_back(&itin);

    CPPUNIT_ASSERT(diffVal.countStopsCarriers(diffSeg, diffTable));
  }

  ////////////////////////////////////////////////////////////////////

  void testConsolidateDifferentials()
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;
    PricingUnit pu;
    FarePath fp;
    PricingOptions po;

    po.iataFares() = 'N';
    pu.puType() = PricingUnit::Type::ROUNDTRIP;

    trx.setRequest(&req);
    trx.setOptions(&po);
    diffVal.diffTrx() = &trx;
    diffVal.fareUsage() = &fu;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;
    diffVal.diffTrx()->setOptions(&po);

    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out

    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    //       vector<PaxTypeFare*> v_f_1;
    //       v_f_1.push_back (&f1);

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    //       fareMarket1.allPaxTypeFare() = v_f_1;
    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    //      vector<PaxTypeFare*> v_f_2;
    //      v_f_2.push_back (&f2);

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //      fareMarket2.allPaxTypeFare() = v_f_2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";
    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    ///
    fareMarket12.classOfServiceVec() = vss2;

    //      vector<PaxTypeFare*> v_f_2;
    //      v_f_2.push_back (&f2);

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    //      vector<PaxTypeFare*> v_f_3;
    //      v_f_3.push_back (&f3);

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    //      fareMarket3.allPaxTypeFare() = v_f_3;
    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    //      fareMarketB1.travelSeg().push_back(&airSegB1);
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;

    //      vector<PaxTypeFare*> v_fB1;
    //      v_fB1.push_back (&fB1);

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    //      fareMarketB1.allPaxTypeFare() = v_fB1;
    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    //      vector<PaxTypeFare*> v_f;
    //      v_f.push_back (&f1);
    //      v_f.push_back (&f2);
    //      v_f.push_back (&f3);
    //      v_f.push_back (&f4);

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;

    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    diffVal.setReturnVal(true);
    vector<Differentials*> differList;
    vector<DifferentialData*>::iterator jmLeft = listDiffData.begin();
    vector<DifferentialData*>::iterator jmRight = listDiffData.begin();

    /*      diffItem->fareHigh() = &f1;
            diffItem->fareLow() = &fB1;
            diffItem->fareHigh()->cabin().setBusinessClass()
            diffItem->fareHigh()->mileageSurchargePctg() = 0;
            diffItem->amount() =  0.0f;
            diffItem->amountFareClassHigh() = 75.0f;
            diffItem->amountFareClassLow() = 10.0f;
    */
    jmRight++;

    // 1...
    FareMarket* mktS = tf.fareMarket();

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;
    diffVal.throughFare() = &tf;
    diffVal._mkt = mktS;

    jmRight = listDiffData.end();
    CPPUNIT_ASSERT(!diffVal.consolidateDifferentials(listDiffData, differList));
    jmRight = listDiffData.begin();
    jmRight++;
    // 4...
    (*jmLeft)->tag() = string("1ABF");
    (*jmRight)->tag() = string("1BF");
    CPPUNIT_ASSERT(diffVal.consolidateDifferentials(listDiffData, differList));
  }
  //+++++++++++++++++++++++++++++

  void testConsolidate(void)
  {
    MockDifferentialValidator diffVal;
    PricingTrx trx;
    PricingRequest req;
    FareUsage fu;

    fu.differentialPlusUp().clear();

    PricingUnit pu;
    FarePath fp;

    trx.setRequest(&req);
    diffVal.diffTrx() = &trx;
    diffVal.pricingUnit() = &pu;
    diffVal.farePath() = &fp;

    Itin itin; // no need to fill it with a real data
    //  ?????  Looks like we need it fiil out

    // 1 ...
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "UA";
    AirSeg airSeg1;
    Loc origin1;
    Loc dest1;

    origin1.loc() = "DAY";
    origin1.area() = IATA_AREA1;
    dest1.loc() = "ORD";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;
    airSeg1.pnrSegment() = airSeg1.segmentOrder() = 1;

    CarrierCode bcCarrier1 = "UA";
    airSeg1.carrier() = bcCarrier1;
    airSeg1.setBookingCode("Y");
    vector<TravelSeg*> tsv_1;
    tsv_1.push_back(&airSeg1);
    fareMarket1.travelSeg() = tsv_1;
    fareMarket1.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f1;
    Fare fare1;
    PaxType paxType1;
    FareInfo finf_1;
    fare1.setFareInfo(&finf_1);

    f1.initialize(&fare1, &paxType1, &fareMarket1);
    const_cast<CarrierCode&>(f1.carrier()) = "UA";
    const_cast<MoneyAmount&>(f1.nucFareAmount()) = 123.F;
    ((FareInfo*)f1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f1.fare()->fareInfo())->_routingNumber = "059";
    f1.cabin().setFirstClass();
    f1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP1;
    vector<FareMarket*> fareMarkets1;
    fareMarkets1.push_back(&fareMarket1);
    diffDP1.fareMarket() = fareMarkets1;
    diffDP1.tag() = "1AF";
    diffDP1.fareHigh() = &f1;
    diffDP1.fareLow() = &f1;
    diffDP1.fareHigh()->cabin().setFirstClass();
    diffDP1.fareHigh()->mileageSurchargePctg() = 0;
    diffDP1.amount() = 0.0f;
    diffDP1.amountFareClassHigh() = 25.0f;
    diffDP1.amountFareClassLow() = 10.0f;
    diffDP1.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    ///
    ClassOfService cs1;
    vector<ClassOfService*> vs1;
    cs1.bookingCode() = airSeg1.getBookingCode();
    cs1.cabin().setFirstClass();
    vs1.push_back(&cs1);
    vector<vector<ClassOfService*>*> vss1;
    vss1.push_back(&vs1);
    fareMarket1.classOfServiceVec() = vss1;

    //       vector<PaxTypeFare*> v_f_1;
    //       v_f_1.push_back (&f1);

    PaxTypeBucket ptc1;
    ptc1.requestedPaxType() = &paxType1;
    ptc1.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv1;
    ptcv1.push_back(ptc1);
    fareMarket1.paxTypeCortege() = ptcv1;

    //       fareMarket1.allPaxTypeFare() = v_f_1;
    ///
    // 2 ...
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "UA";
    AirSeg airSeg2;
    Loc origin2;
    Loc dest2;

    origin2.loc() = "ORD";
    origin2.area() = IATA_AREA1;
    dest2.loc() = "SFO";

    airSeg2.origin() = &origin2;
    airSeg2.destination() = &dest2;
    airSeg2.pnrSegment() = airSeg2.segmentOrder() = 2;

    CarrierCode bcCarrier2 = "UA";
    airSeg2.carrier() = bcCarrier2;
    airSeg2.setBookingCode("Y");
    vector<TravelSeg*> tsv_2;
    tsv_2.push_back(&airSeg2);
    fareMarket2.travelSeg() = tsv_2;

    fareMarket2.primarySector() = &airSeg2;
    //  ...
    PaxTypeFare f2;
    Fare fare2;
    PaxType paxType2;
    FareInfo finf_2;
    fare2.setFareInfo(&finf_2);

    f2.initialize(&fare2, &paxType2, &fareMarket2);
    const_cast<CarrierCode&>(f2.carrier()) = "UA";
    const_cast<MoneyAmount&>(f2.nucFareAmount()) = 234.F;
    ((FareInfo*)f2.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f2.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f2.fare()->fareInfo())->_routingNumber = "059";
    f2.cabin().setFirstClass();
    f2.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP2;
    vector<FareMarket*> fareMarkets2;
    fareMarkets2.push_back(&fareMarket2);
    diffDP2.fareMarket() = fareMarkets2;
    diffDP2.tag() = "2AF";
    diffDP2.fareHigh() = &f1;
    diffDP2.fareLow() = &f2;
    diffDP2.fareHigh()->cabin().setFirstClass();
    diffDP2.fareHigh()->mileageSurchargePctg() = 0;
    diffDP2.amount() = 0.0f;
    diffDP2.amountFareClassHigh() = 35.0f;
    diffDP2.amountFareClassLow() = 10.0f;
    diffDP2.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    ///
    ClassOfService cs2;
    vector<ClassOfService*> vs2;
    cs2.bookingCode() = airSeg2.getBookingCode();
    cs2.cabin().setFirstClass();
    vs2.push_back(&cs2);
    vector<vector<ClassOfService*>*> vss2;
    vss2.push_back(&vs2);
    fareMarket2.classOfServiceVec() = vss2;

    //      vector<PaxTypeFare*> v_f_2;
    //      v_f_2.push_back (&f2);

    PaxTypeBucket ptc2;
    ptc2.requestedPaxType() = &paxType1;
    ptc2.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv2;
    ptcv2.push_back(ptc2);
    fareMarket2.paxTypeCortege() = ptcv2;

    //      fareMarket2.allPaxTypeFare() = v_f_2;

    //
    // Fare market combined 1-2

    // 1-2 ...
    FareMarket fareMarket12;
    fareMarket12.governingCarrier() = "UA";
    vector<TravelSeg*> tsv_12;
    tsv_12.push_back(&airSeg1);
    tsv_12.push_back(&airSeg2);
    fareMarket12.travelSeg() = tsv_12;

    fareMarket12.primarySector() = &airSeg1;
    //  ...
    PaxTypeFare f12;
    Fare fare12;
    PaxType paxType12;
    FareInfo finf_12;
    fare12.setFareInfo(&finf_12);

    f12.initialize(&fare12, &paxType12, &fareMarket12);
    const_cast<CarrierCode&>(f12.carrier()) = "UA";
    const_cast<MoneyAmount&>(f12.nucFareAmount()) = 334.F;
    ((FareInfo*)f12.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f12.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f12.fare()->fareInfo())->_routingNumber = "059";
    f12.cabin().setFirstClass();
    f12.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    DifferentialData diffDP12;
    vector<FareMarket*> fareMarkets12;
    fareMarkets12.push_back(&fareMarket12);
    diffDP12.fareMarket() = fareMarkets12;
    diffDP12.tag() = "2AF";
    diffDP12.fareHigh() = &f1;
    diffDP12.fareLow() = &f2;
    diffDP12.fareHigh()->cabin().setFirstClass();
    diffDP12.fareHigh()->mileageSurchargePctg() = 0;
    diffDP12.amount() = 0.0f;
    diffDP12.amountFareClassHigh() = 45.0f;
    diffDP12.amountFareClassLow() = 10.0f;
    diffDP12.tripType() = DifferentialValidator::ROUND_TRIP;
    ///
    fareMarket12.classOfServiceVec() = vss2;

    //      vector<PaxTypeFare*> v_f_2;
    //      v_f_2.push_back (&f2);

    PaxTypeBucket ptc12;
    ptc12.requestedPaxType() = &paxType1;
    ptc12.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv12;
    ptcv12.push_back(ptc12);
    fareMarket12.paxTypeCortege() = ptcv12;

    ///
    // 3 ...
    FareMarket fareMarket3;
    fareMarket3.governingCarrier() = "UA";
    AirSeg airSeg3;
    Loc origin3;
    Loc dest3;

    origin3.loc() = "SFO";
    origin3.area() = IATA_AREA1;
    dest3.loc() = "NRT";

    airSeg3.origin() = &origin3;
    airSeg3.destination() = &dest3;
    airSeg3.pnrSegment() = airSeg3.segmentOrder() = 3;

    CarrierCode bcCarrier3 = "UA";
    airSeg3.carrier() = bcCarrier3;
    airSeg3.setBookingCode("Y");
    vector<TravelSeg*> tsv_3;
    tsv_3.push_back(&airSeg3);
    fareMarket3.travelSeg() = tsv_3;

    fareMarket3.primarySector() = &airSeg3;
    //  ...
    PaxTypeFare f3;
    Fare fare3;
    PaxType paxType3;
    FareInfo finf_3;
    fare3.setFareInfo(&finf_3);

    f3.initialize(&fare3, &paxType3, &fareMarket3);
    const_cast<CarrierCode&>(f3.carrier()) = "UA";
    const_cast<MoneyAmount&>(f3.nucFareAmount()) = 234.F;
    ((FareInfo*)f3.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f3.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f3.fare()->fareInfo())->_routingNumber = "059";
    f3.cabin().setBusinessClass();
    f3.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP3;
    vector<FareMarket*> fareMarkets3;
    fareMarkets3.push_back(&fareMarket3);
    diffDP3.fareMarket() = fareMarkets3;
    diffDP3.tag() = "3AB";
    diffDP3.fareHigh() = &f1;
    diffDP3.fareLow() = &f3;
    diffDP3.fareHigh()->cabin().setBusinessClass();
    diffDP3.fareHigh()->mileageSurchargePctg() = 0;
    diffDP3.amount() = 0.0f;
    diffDP3.amountFareClassHigh() = 55.0f;
    diffDP3.amountFareClassLow() = 10.0f;
    diffDP3.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    ///
    ClassOfService cs3;
    vector<ClassOfService*> vs3;
    cs3.bookingCode() = airSeg3.getBookingCode();
    cs3.cabin().setBusinessClass();
    vs3.push_back(&cs3);
    vector<vector<ClassOfService*>*> vss3;
    vss3.push_back(&vs3);
    fareMarket3.classOfServiceVec() = vss3;

    //      vector<PaxTypeFare*> v_f_3;
    //      v_f_3.push_back (&f3);

    PaxTypeBucket ptc3;
    ptc3.requestedPaxType() = &paxType1;
    ptc3.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv3;
    ptcv3.push_back(ptc3);
    fareMarket3.paxTypeCortege() = ptcv3;

    //      fareMarket3.allPaxTypeFare() = v_f_3;
    ///
    // 4 ...
    FareMarket fareMarket4;
    fareMarket4.governingCarrier() = "UA";
    AirSeg airSeg4;
    Loc origin4;
    Loc dest4;

    origin4.loc() = "NRT";
    origin4.area() = IATA_AREA2;
    dest4.loc() = "SIN";

    airSeg4.origin() = &origin4;
    airSeg4.destination() = &dest4;
    airSeg4.pnrSegment() = airSeg4.segmentOrder() = 4;

    CarrierCode bcCarrier4 = "SQ";
    airSeg4.carrier() = bcCarrier4;
    airSeg4.setBookingCode("Y");
    vector<TravelSeg*> tsv_4;
    tsv_4.push_back(&airSeg4);
    fareMarket4.travelSeg() = tsv_4;
    fareMarket4.primarySector() = &airSeg4;
    //  ...
    PaxTypeFare f4;
    Fare fare4;
    PaxType paxType4;
    FareInfo finf_4;
    fare4.setFareInfo(&finf_4);

    f4.initialize(&fare4, &paxType4, &fareMarket4);
    const_cast<CarrierCode&>(f4.carrier()) = "SQ";
    const_cast<MoneyAmount&>(f4.nucFareAmount()) = 234.F;
    ((FareInfo*)f4.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)f4.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)f4.fare()->fareInfo())->_routingNumber = "059";
    f4.cabin().setEconomyClass();
    f4.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDP4;
    vector<FareMarket*> fareMarkets4;
    fareMarkets4.push_back(&fareMarket4);
    diffDP4.fareMarket() = fareMarkets4;
    diffDP4.fareHigh() = &f1;
    diffDP4.fareLow() = &f3;
    diffDP4.fareHigh()->cabin().setBusinessClass();
    diffDP4.fareHigh()->mileageSurchargePctg() = 0;
    diffDP4.amount() = 0.0f;
    diffDP4.amountFareClassHigh() = 65.0f;
    diffDP4.amountFareClassLow() = 10.0f;
    diffDP4.tripType() = DifferentialValidator::ONE_WAY_TRIP;
    ///
    ClassOfService cs4;
    vector<ClassOfService*> vs4;
    cs4.bookingCode() = airSeg4.getBookingCode();
    cs4.cabin().setEconomyClass();
    vs4.push_back(&cs4);
    vector<vector<ClassOfService*>*> vss4;
    vss4.push_back(&vs4);
    fareMarket4.classOfServiceVec() = vss4;
    ///

    // For Business fare market ...
    FareMarket fareMarketB1;
    fareMarketB1.governingCarrier() = "UA";
    AirSeg airSegB1;
    Loc originB1;
    Loc destB1;

    originB1.loc() = "DAY";
    originB1.area() = IATA_AREA1;
    destB1.loc() = "NRT";

    airSegB1.origin() = &originB1;
    airSegB1.destination() = &destB1;
    airSegB1.pnrSegment() = airSegB1.segmentOrder() = 6;

    CarrierCode bcCarrierB1 = "UA";
    airSegB1.carrier() = bcCarrierB1;
    airSegB1.setBookingCode("Y");
    //      fareMarketB1.travelSeg().push_back(&airSegB1);
    ///
    fareMarketB1.governingCarrier() = "UA";
    ClassOfService csB1;
    vector<ClassOfService*> vsB1;
    csB1.bookingCode() = airSegB1.getBookingCode();
    csB1.cabin().setBusinessClass();
    vsB1.push_back(&csB1);
    vector<vector<ClassOfService*>*> vssB1;
    vssB1.push_back(&vsB1);
    fareMarketB1.classOfServiceVec() = vssB1;
    vector<TravelSeg*> tsv_a;
    tsv_a.push_back(&airSeg1);
    tsv_a.push_back(&airSeg2);
    tsv_a.push_back(&airSeg3);
    fareMarketB1.travelSeg() = tsv_a;
    ///
    //  ...
    PaxTypeFare fB1;
    Fare fareB1;
    PaxType paxTypeB1;
    FareInfo finf_B1;
    fareB1.setFareInfo(&finf_B1);

    fB1.initialize(&fareB1, &paxTypeB1, &fareMarketB1);
    const_cast<CarrierCode&>(fB1.carrier()) = "UA";
    const_cast<MoneyAmount&>(fB1.nucFareAmount()) = 234.F;
    ((FareInfo*)fB1.fare()->fareInfo())->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    ((FareInfo*)fB1.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)fB1.fare()->fareInfo())->_routingNumber = "059";
    fB1.cabin().setBusinessClass();
    fB1.bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    fu.hipExemptInd() = DifferentialData::NO_HIP;
    DifferentialData diffDPB1;
    vector<FareMarket*> fareMarketsB1;
    fareMarketsB1.push_back(&fareMarketB1);
    diffDPB1.fareMarket() = fareMarketsB1;
    diffDPB1.fareHigh() = &f1;
    diffDPB1.fareLow() = &fB1;
    diffDPB1.fareHigh()->cabin().setBusinessClass();
    diffDPB1.fareHigh()->mileageSurchargePctg() = 0;
    diffDPB1.amount() = 0.0f;
    diffDPB1.amountFareClassHigh() = 75.0f;
    diffDPB1.amountFareClassLow() = 10.0f;
    diffDPB1.tripType() = DifferentialValidator::ONE_WAY_TRIP;

    //      vector<PaxTypeFare*> v_fB1;
    //      v_fB1.push_back (&fB1);

    PaxTypeBucket ptcB1;
    ptcB1.requestedPaxType() = &paxType1;
    ptcB1.paxTypeFare().push_back(&fB1); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcvB1;
    ptcvB1.push_back(ptcB1);
    fareMarketB1.paxTypeCortege() = ptcvB1;

    //      fareMarketB1.allPaxTypeFare() = v_fB1;
    ///

    //  ... Through fare
    PaxTypeFare tf;
    Fare faret;
    PaxType paxTypet;
    FareMarket fareMarkett;
    FareInfo finf_t;
    faret.setFareInfo(&finf_t);
    ///
    AirSeg airSeg;
    Loc origin;
    Loc dest;

    origin.loc() = "DAY";
    dest.loc() = "SIN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;
    airSeg.pnrSegment() = airSeg.segmentOrder() = 5;

    CarrierCode bcCarrier = "UA";
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarkett.primarySector() = &airSeg3;
    vector<TravelSeg*> tsv_b;
    tsv_b.push_back(&airSeg1);
    tsv_b.push_back(&airSeg2);
    tsv_b.push_back(&airSeg3);
    tsv_b.push_back(&airSeg4);
    tsv_b.push_back(&airSeg);
    fareMarkett.travelSeg() = tsv_b;
    ///
    fareMarkett.governingCarrier() = "UA";
    tf.initialize(&faret, &paxTypet, &fareMarkett);
    const_cast<CarrierCode&>(tf.carrier()) = "UA";
    tf.cabin().setEconomyClass();
    fu.hipExemptInd() = DifferentialData::BUSINESS_HIP;
    ((FareInfo*)tf.fare()->fareInfo())->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    ((FareInfo*)tf.fare()->fareInfo())->_globalDirection = GlobalDirection::ZZ;
    ((FareInfo*)tf.fare()->fareInfo())->_routingNumber = "059";
    //      vector<PaxTypeFare*> v_f;
    //      v_f.push_back (&f1);
    //      v_f.push_back (&f2);
    //      v_f.push_back (&f3);
    //      v_f.push_back (&f4);

    PaxTypeBucket ptc_f;
    ptc_f.requestedPaxType() = &paxType1;
    ptc_f.paxTypeFare().push_back(&f1); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f2); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f3); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f4); // add paxtypefare to the faremarket
    ptc_f.paxTypeFare().push_back(&f12); // add paxtypefare to the faremarket
    vector<PaxTypeBucket> ptcv_f;
    ptcv_f.push_back(ptc_f);
    fareMarkett.paxTypeCortege() = ptcv_f;

    //      fareMarkett.allPaxTypeFare() = v_f;
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket12);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);
    trx.fareMarket().push_back(&fareMarketB1);
    //      trx.fareMarket().push_back( &fareMarkett );
    vector<DifferentialData*> vecDiffData;
    vecDiffData.push_back(&diffDP1);
    vecDiffData.push_back(&diffDP2);
    vecDiffData.push_back(&diffDP12);
    vecDiffData.push_back(&diffDP3);
    vecDiffData.push_back(&diffDP4);

    vector<DifferentialData*> listDiffData;
    listDiffData.push_back(&diffDP1);
    listDiffData.push_back(&diffDP2);
    listDiffData.push_back(&diffDP12);
    listDiffData.push_back(&diffDP3);
    listDiffData.push_back(&diffDP4);

    fu.differentialPlusUp() = vecDiffData;

    PaxTypeFare::SegmentStatus segStat;
    tf.segmentStatus().push_back(segStat);
    fareMarkett.primarySector() = &airSeg3;
    //  ...
    ClassOfService cs;
    vector<ClassOfService*> vs;
    cs.bookingCode() = airSeg.getBookingCode();
    cs.cabin().setEconomyClass();
    vs.push_back(&cs);
    vector<vector<ClassOfService*>*> vss;
    vss.push_back(&vs);
    fareMarkett.classOfServiceVec() = vss;

    diffVal.setReturnVal(true);
    vector<Differentials*> differList;
    //      vector<DifferentialData *>::iterator jmLeft  = listDiffData.begin ();
    vector<DifferentialData*>::iterator jmRight = listDiffData.begin();
    //      DifferentialData* diffItem;

    /*      diffItem->fareHigh() = &f1;
            diffItem->fareLow() = &fB1;
            diffItem->fareHigh()->cabin().setBusinessClass()
            diffItem->fareHigh()->mileageSurchargePctg() = 0;
            diffItem->amount() =  0.0f;
            diffItem->amountFareClassHigh() = 75.0f;
            diffItem->amountFareClassLow() = 10.0f;
    */
    jmRight++;

    // 1...
    FareMarket* mktS = tf.fareMarket();

    vector<TravelSeg*> tsv_itin;
    tsv_itin.push_back(&airSeg1);
    tsv_itin.push_back(&airSeg2);
    tsv_itin.push_back(&airSeg3);
    tsv_itin.push_back(&airSeg4);
    tsv_itin.push_back(&airSeg);
    itin.travelSeg() = tsv_itin;

    diffVal.itinerary() = &itin;
    diffVal.throughFare() = &tf;
    diffVal._mkt = mktS;

    diffVal.fareUsage() = &fu;
    CPPUNIT_ASSERT(!diffVal.consolidate(differList));
  }

  void testAdjustRexPricingDates_retrieval()
  {
    DateTime retrievalDate(2008, 01, 02);
    DateTime ticketingDate(2008, 01, 03);
    DateTime travelDate(2008, 01, 04);

    Itin itin;
    itin.setTravelDate(travelDate);

    PaxTypeFare ptf;
    FareMarket::RetrievalInfo ri;
    ri._date = retrievalDate;
    ptf.retrievalInfo() = &ri;

    RexPricingTrx trx;
    trx.dataHandle().setTicketDate(ticketingDate);

    DifferentialValidator dv;
    dv._diffTrx = &trx;
    dv._itin = &itin;
    dv._paxTfare = &ptf;

    dv.adjustRexPricingDates();

    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == retrievalDate);
  }

  void testAdjustRexPricingDates_travel()
  {
    DateTime retrievalDate(2008, 01, 02);
    DateTime ticketingDate(2008, 01, 03);
    DateTime travelDate(2008, 01, 01);

    Itin itin;
    itin.setTravelDate(travelDate);

    PaxTypeFare ptf;
    FareMarket::RetrievalInfo ri;
    ri._date = retrievalDate;
    ptf.retrievalInfo() = &ri;

    RexPricingTrx trx;
    trx.dataHandle().setTicketDate(ticketingDate);

    DifferentialValidator dv;
    dv._diffTrx = &trx;
    dv._itin = &itin;
    dv._paxTfare = &ptf;

    dv.adjustRexPricingDates();

    CPPUNIT_ASSERT(dv._travelDate == retrievalDate);
  }

  void testRestorePricingDates()
  {
    DateTime ticketingDate(2008, 01, 01);
    DateTime orginalTicketingDate(2008, 01, 03);

    RexPricingTrx trx;
    trx.ticketingDate() = orginalTicketingDate;
    trx.dataHandle().setTicketDate(ticketingDate);

    DifferentialValidator dv;
    dv._diffTrx = &trx;

    dv.restorePricingDates();

    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == orginalTicketingDate);
  }
  void testCabinTypeFromFTD()
  {
    CabinType c;
    diffVal.cabinTypeFromFTD(DifferentialData::ECONOMY_FTD, c);
    CPPUNIT_ASSERT(c.isEconomyClass());
    diffVal.cabinTypeFromFTD(DifferentialData::BUSINESS_FTD, c);
    CPPUNIT_ASSERT(c.isBusinessClass());
    diffVal.cabinTypeFromFTD(DifferentialData::FIRST_FTD, c);
    CPPUNIT_ASSERT(c.isFirstClass());
    diffVal.cabinTypeFromFTD(DifferentialData::BLANK_FTD, c);
    CPPUNIT_ASSERT(c.isEconomyClass());

    diffVal.cabinTypeFromFTD(DifferentialData::PREMIUM_ECONOMY_FTD, c);
    CPPUNIT_ASSERT(c.isPremiumEconomyClass());

    diffVal.cabinTypeFromFTD(DifferentialData::PREMIUM_BUSINESS_FTD_NEW, c);
    CPPUNIT_ASSERT(c.isPremiumBusinessClass());

    diffVal.cabinTypeFromFTD(DifferentialData::PREMIUM_FIRST_FTD, c);
    CPPUNIT_ASSERT(c.isPremiumFirstClass());
  }
  void testCabinTypeToFTD()
  {
    CabinType c;
    CPPUNIT_ASSERT_EQUAL(DifferentialData::BLANK_FTD, diffVal.cabinTypeToFTD(c));
    c.setEconomyClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::BLANK_FTD, diffVal.cabinTypeToFTD(c));
    c.setBusinessClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::BUSINESS_FTD, diffVal.cabinTypeToFTD(c));
    c.setFirstClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::FIRST_FTD, diffVal.cabinTypeToFTD(c));
    c.setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_ECONOMY_FTD, diffVal.cabinTypeToFTD(c));
    c.setPremiumBusinessClass();

    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_BUSINESS_FTD_NEW, diffVal.cabinTypeToFTD(c));

    c.setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_FIRST_FTD, diffVal.cabinTypeToFTD(c));
  }
  void testCabinTypeFromHip()
  {
    CabinType c;
    diffVal.cabinTypeFromHip(DifferentialData::ECONOMY_HIP, c);
    CPPUNIT_ASSERT(c.isEconomyClass());
    diffVal.cabinTypeFromHip(DifferentialData::BUSINESS_HIP, c);
    CPPUNIT_ASSERT(c.isBusinessClass());
    diffVal.cabinTypeFromHip(DifferentialData::FIRST_HIP, c);
    CPPUNIT_ASSERT(c.isFirstClass());
    diffVal.cabinTypeFromHip(DifferentialData::NO_HIP, c);
    CPPUNIT_ASSERT(c.isInvalidClass());
    diffVal.cabinTypeFromHip(DifferentialData::PREMIUM_ECONOMY_HIP, c);
    CPPUNIT_ASSERT(c.isPremiumEconomyClass());
    diffVal.cabinTypeFromHip(DifferentialData::PREMIUM_ECONOMY_HIP_ANSWER, c);
    CPPUNIT_ASSERT(c.isPremiumEconomyClass());
    diffVal.cabinTypeFromHip(DifferentialData::PREMIUM_BUSINESS_HIP, c);
    CPPUNIT_ASSERT(c.isPremiumBusinessClass());
    diffVal.cabinTypeFromHip(DifferentialData::PREMIUM_FIRST_HIP, c);
    CPPUNIT_ASSERT(c.isPremiumFirstClass());
    diffVal.cabinTypeFromHip(DifferentialData::PREMIUM_FIRST_HIP_ANSWER, c);
    CPPUNIT_ASSERT(c.isPremiumFirstClass());
  }
  void testCabinTypeToHip()
  {
    MyDataHandle mdh;
    DateTime ticketingDate(2008, 01, 03);
    PricingTrx trx;
    trx.dataHandle().setTicketDate(ticketingDate);
    PricingRequest req;
    trx.setRequest(&req);

    DifferentialValidator dv;
    diffVal._diffTrx = &trx;

    CabinType c;
    CPPUNIT_ASSERT_EQUAL(DifferentialData::NO_HIP, diffVal.cabinTypeToHip(c));
    c.setEconomyClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::ECONOMY_HIP, diffVal.cabinTypeToHip(c));
    c.setBusinessClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::BUSINESS_HIP, diffVal.cabinTypeToHip(c));
    c.setFirstClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::FIRST_HIP, diffVal.cabinTypeToHip(c));
    c.setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_ECONOMY_HIP, diffVal.cabinTypeToHip(c));
    c.setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_BUSINESS_HIP, diffVal.cabinTypeToHip(c));
    c.setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(DifferentialData::PREMIUM_FIRST_HIP, diffVal.cabinTypeToHip(c));
  }
  void testGetLowerCabinPremium()
  {
    CabinType c;
    c.setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(true, diffVal.getLowerCabin(c));
    CPPUNIT_ASSERT_EQUAL(true, c.isFirstClass());
    CPPUNIT_ASSERT_EQUAL(true, diffVal.getLowerCabin(c));
    CPPUNIT_ASSERT_EQUAL(true, c.isPremiumBusinessClass());
    CPPUNIT_ASSERT_EQUAL(true, diffVal.getLowerCabin(c));
    CPPUNIT_ASSERT_EQUAL(true, c.isBusinessClass());
    CPPUNIT_ASSERT_EQUAL(true, diffVal.getLowerCabin(c));
    CPPUNIT_ASSERT_EQUAL(true, c.isPremiumEconomyClass());
    CPPUNIT_ASSERT_EQUAL(true, diffVal.getLowerCabin(c));
    CPPUNIT_ASSERT_EQUAL(true, c.isEconomyClass());
    CPPUNIT_ASSERT_EQUAL(false, diffVal.getLowerCabin(c));
  }
  void initPC1data(PremiumCabin1TestData& tcd)
  {

    tcd._fm = TestFareMarketFactory::create("testdata/PC1/FMarket_SINDXB-SQ.xml");
    tcd._fmth = TestFareMarketFactory::create("testdata/PC1/FMarket_SINCDG-SQ.xml");
    tcd._through = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare_th.xml");
    tcd._ptf1 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare1.xml");
    tcd._ptf2 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare2.xml");
    tcd._ptf4 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare4.xml");
    tcd._ptf5 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare5.xml");
    tcd._ptf7 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare7.xml");
    tcd._ptf8 = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare8.xml");
    tcd._reqPT = TestPaxTypeFactory::create("testdata/PC1/PaxType.xml");
    tcd._fu = TestFareUsageFactory::create("testdata/PC1/FareUsage.xml");
    tcd._pu = TestPricingUnitFactory::create("testdata/PC1/PricingUnit.xml");
    tcd._req = TestPricingRequestFactory::create("testdata/PC1/PricingRequest.xml");
    tcd._trx.setOptions(&tcd._opt);
    tcd._trx.setRequest(tcd._req);
    tcd._diffV.throughFare() = tcd._through;
    tcd._diffV.diffTrx() = &tcd._trx;
    tcd._diffV._mkt = tcd._fmth;
    tcd._diffV.pricingUnit() = tcd._pu;
    tcd._diffV.fareUsage() = tcd._fu;
    tcd._diffV.requestedPaxType() = tcd._reqPT;

    tcd._dd.cabin() = DifferentialData::FIRST_FTD;
    tcd._dd.calculationIndicator() = DifferentialValidator::SAME_TYPE;

    tcd._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // AOW8
    tcd._ptf2->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // BOW8
    tcd._ptf4->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // JOW8
    tcd._ptf5->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // KOW8
    tcd._ptf7->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // ZOW8
    tcd._ptf8->bookingCodeStatus().set(PaxTypeFare::BKS_PASS); // YOW8

    tcd._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // AOW8
    tcd._ptf2->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // BOW8
    tcd._ptf4->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // JOW8
    tcd._ptf5->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // KOW8
    tcd._ptf7->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // ZOW8
    tcd._ptf8->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false); // YOW8

    tcd._ptfVec.push_back(tcd._ptf1);
    tcd._ptfVec.push_back(tcd._ptf2);
    tcd._ptfVec.push_back(tcd._ptf4);
    tcd._ptfVec.push_back(tcd._ptf5);
    tcd._ptfVec.push_back(tcd._ptf7);
    tcd._ptfVec.push_back(tcd._ptf8);
  };
  void initFareSelectionDataHelper(PremiumCabin1TestData& tcd,
                                   DifferentialValidator::FareSelectionDataHelper& fsh)
  {
    fsh.slideAllowed() = true;
    fsh.globalDirection() = tcd._fm->getGlobalDirection();
    fsh.prPricePr() = DifferentialValidator::LOWEST_FARE;
    fsh.ilow() = false;
    fsh.govCXR() = "SQ";
    fsh.paxTypeFareVec() = tcd._ptfVec;
  }
  void resetMap(DifferentialValidator::FareSelectionDataHelper& fdst)
  {
    std::map<size_t, DifferentialValidator::FareSelectionData*>::iterator iB =
        fdst.fareSelectionMap().begin();
    std::map<size_t, DifferentialValidator::FareSelectionData*>::iterator iE =
        fdst.fareSelectionMap().end();
    for (; iB != iE; iB++)
    {
      DifferentialValidator::FareSelectionData* fsd = iB->second;
      fsd->amountFareClassHigh() = 0;
      fsd->amountFareClassLow() = 0;
      fsd->fareHigh() = 0;
      fsd->fareLow() = 0;
      fsd->amountFareClassHighOW() = 0;
      fsd->amountFareClassLowOW() = 0;
      fsd->fareHighOW() = 0;
      fsd->fareLowOW() = 0;
      fsd->foundHigh() = false;
      fsd->foundLow() = false;
      fsd->fareClassLow() = "";
      fsd->fareClassHigh() = "";
    }
  }
  void checkHighLowResults(DifferentialValidator::FareSelectionDataHelper& fdsm,
                           const FareClassCode& fcHigh,
                           FareClassCode fcLow)
  {
    DifferentialValidator::FareSelectionData fsd;
    fdsm.getValue(fsd);
    if (fcLow.length())
      CPPUNIT_ASSERT_EQUAL(fcLow, fsd.fareClassLow());
    else
      CPPUNIT_ASSERT_EQUAL(false, fsd.foundLow());
    if (fcHigh.length())
      CPPUNIT_ASSERT_EQUAL(fcHigh, fsd.fareClassHigh());
    else
      CPPUNIT_ASSERT_EQUAL(false, fsd.foundHigh());
  }

  void testProcessNormalFareSelectionFirstHigh()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setPremiumFirstClass();
    fdsm.throughFTD().setFirstClass();
    // should find high and low. High in premium first, low in first
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "AOW8", "BOW8");

    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, false);
    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    resetMap(fdsm);
    // should find low in first
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "BOW8");

    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL, false);
    td._ptf2->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, false);
    td._ptf2->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    resetMap(fdsm);

    // high in premium first
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "AOW8", "BOW8");
  }
  void testProcessNormalFareSelectionPremiumBusinessHigh()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setFirstClass();
    fdsm.throughFTD().setPremiumBusinessClass();

    // should find high and low. High in first, low in premium business
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "BOW8", "JOW8");
  }
  void testProcessNormalFareSelectionBusinessHigh()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setFirstClass();
    fdsm.throughFTD().setBusinessClass();

    // should find high and low. High in first, low in business
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "BOW8", "KOW8");
  }
  void testProcessNormalFareSelectionPremiumEconomyHigh()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setPremiumBusinessClass();
    fdsm.throughFTD().setPremiumEconomyClass();

    // should find high and low. High in premium business, low in premium economy
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "JOW8", "ZOW8");
  }
  void testProcessNormalFareSelectionEconomyHigh()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setBusinessClass();
    fdsm.throughFTD().setEconomyClass();

    // should find high and low. High in business, low in economy
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "KOW8", "YOW8");
  }

  void testProcessNormalFareSelectionFirstLow()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);
    fdsm.ilow() = true;

    fdsm.throughRealFTD().setFirstClass();
    fdsm.throughFTD().setFirstClass();
    // before calling processNormalFareSelection for low fares,
    // validateFareTypeDesignators call getLowerCabin, so let's simulate that
    td._diffV.getLowerCabin(fdsm.throughFTD());

    // should find low in premium business
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "JOW8");
  }
  void testProcessNormalFareSelectionPremiumBusinessLow()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);
    fdsm.ilow() = true;

    fdsm.throughRealFTD().setPremiumBusinessClass();
    fdsm.throughFTD().setPremiumBusinessClass();
    // before calling processNormalFareSelection for low fares,
    // validateFareTypeDesignators call getLowerCabin, so let's simulate that
    td._diffV.getLowerCabin(fdsm.throughFTD());

    // should find low in business
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "KOW8");

    fdsm.slideAllowed() = false;
    resetMap(fdsm);

    // should find low in premium economy
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "ZOW8");
  }
  void testProcessNormalFareSelectionBusinessLow()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);
    fdsm.ilow() = true;

    fdsm.throughRealFTD().setBusinessClass();
    fdsm.throughFTD().setBusinessClass();
    // before calling processNormalFareSelection for low fares,
    // validateFareTypeDesignators call getLowerCabin, so let's simulate that
    td._diffV.getLowerCabin(fdsm.throughFTD());

    // should find low in premium economy
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "ZOW8");
  }
  void testProcessNormalFareSelectionPremiumEconomyLow()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);
    fdsm.ilow() = true;

    fdsm.throughRealFTD().setPremiumEconomyClass();
    fdsm.throughFTD().setPremiumEconomyClass();
    // before calling processNormalFareSelection for low fares,
    // validateFareTypeDesignators call getLowerCabin, so let's simulate that
    td._diffV.getLowerCabin(fdsm.throughFTD());

    // should find low in economy
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "YOW8");

    fdsm.slideAllowed() = false;
    resetMap(fdsm);

    // nothing to find?
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processNormalFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "");
  }

  void testProcessWPNCFareSelectionFirst()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setPremiumFirstClass();
    fdsm.throughFTD().setFirstClass();

    // should find high premium first
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "AOW8", "");

    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, false);
    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    resetMap(fdsm);
    // should find nothing
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "");
  }
  void testProcessWPNCFareSelectionPremiumBusiness()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setPremiumFirstClass();
    fdsm.throughFTD().setPremiumBusinessClass();

    // should find high premium first
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "AOW8", "");

    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_PASS, false);
    td._ptf1->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    resetMap(fdsm);
    // should find nothing
    CPPUNIT_ASSERT_EQUAL(false, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "", "");
  }
  void testProcessWPNCFareSelectionBusiness()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setPremiumBusinessClass();
    fdsm.throughFTD().setBusinessClass();

    // should find high premium business
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "JOW8", "");
  }
  void testProcessWPNCFareSelectionPremiumEconomy()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setBusinessClass();
    fdsm.throughFTD().setPremiumEconomyClass();

    // should find high premium business
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "KOW8", "");
  }
  void testProcessWPNCFareSelectionEconomy()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    DifferentialValidator::FareSelectionDataHelper fdsm(td._trx, td._dd);
    initFareSelectionDataHelper(td, fdsm);

    fdsm.failedFareCabin().setBusinessClass();
    fdsm.throughFTD().setEconomyClass();

    // should find high premium business
    CPPUNIT_ASSERT_EQUAL(true, td._diffV.processWPNCFareSelection(fdsm));
    checkHighLowResults(fdsm, "KOW8", "");
  }
  void testGetIndustryPrimePricingPrecedence()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);

    std::vector<FareMarket*> fmv;
    fmv.push_back(td._fm);
    fmv.push_back(td._fmth);
    CarrierCode cxr("SQ");
    FareMarket* fmidif = td._fmth;
    td._dd.travelSeg() = fmidif->travelSeg();

    Indicator ret;
    CPPUNIT_ASSERT_EQUAL(
        true,
        td._diffV.getIndustryPrimePricingPrecedence(td._dd, cxr, false, fmv.begin(), fmidif, ret));
    CPPUNIT_ASSERT_EQUAL('C', ret);
    IndustryPricingAppl* i2 =
        TestIndustryPricingApplFactory::create("testdata/PC1/IndustryPricingAppl2.xml");
    i2->directionality() = TO;
    CPPUNIT_ASSERT_EQUAL(
        true,
        td._diffV.getIndustryPrimePricingPrecedence(td._dd, cxr, false, fmv.begin(), fmidif, ret));
    CPPUNIT_ASSERT_EQUAL('L', ret);
  }
  void testValidateFareTypeDesignatorsPC()
  {
    MyDataHandle mdh;
    PremiumCabin1TestData td;
    initPC1data(td);
    FarePath fp;
    td._diffV.farePath() = &fp;

    std::vector<FareMarket*> fmv;
    fmv.push_back(td._fm);
    std::vector<CarrierCode> cxv;
    cxv.push_back("SQ");

    // through fare is in business, failed sector in firts, should find high in first, low in
    // business

    CPPUNIT_ASSERT_EQUAL(true, td._diffV.validateFareTypeDesignators(td._dd, fmv, cxv, false));
    CPPUNIT_ASSERT_EQUAL(FareClassCode("BOW8"), td._dd.fareClassHigh());
    CPPUNIT_ASSERT_EQUAL(FareClassCode("KOW8"), td._dd.fareClassLow());
  }
  void testValidatePuRulesNoCat14()
  {
    MyDataHandle mdh;
    DateTime ticketingDate(2008, 01, 03);
    PricingTrx trx;
    trx.dataHandle().setTicketDate(ticketingDate);

    DifferentialValidator dv;
    dv._diffTrx = &trx;

    PaxTypeFare* pFare = TestPaxTypeFareFactory::create("testdata/PC1/PaxTypeFare_th.xml");
    PricingUnit* pu = TestPricingUnitFactory::create("testdata/PC1/PricingUnit.xml");
    dv._pricingUnit = pu;
    vector<uint16_t> puCategories;
    puCategories.clear(); // push_back(RuleConst::PENALTIES_RULE);
    FarePath fp;
    dv._farePath = &fp;

    CPPUNIT_ASSERT_EQUAL(true, dv.validatePuRules(*pFare, puCategories));
  }

  void testIsFareValidForFareSelection_falseForTagNPTF()
  {
    PremiumCabin1TestData td;
    initPC1data(td);

    Indicator prPricePr = ' ';
    CarrierCode govCXR = "AA";

    std::vector<PaxTypeFare*>::const_iterator ptfVecIC = td._ptfVec.begin();
    td._ptfVec.front()->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_TAG_N);
    td._ptfVec.front()->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);
    DifferentialValidator dv;
    CPPUNIT_ASSERT(!dv.isFareValidForFareSelection(ptfVecIC, td._dd, prPricePr, govCXR));
  }

  void testIsPremiumEconomyCabin_True()
  {
    DifferentialValidator dv;
    FareType ft = "*W";
    CPPUNIT_ASSERT(dv.isPremiumEconomyCabin(ft));

    ft = "ZEX";
    CPPUNIT_ASSERT(dv.isPremiumEconomyCabin(ft));

    ft = "*W";
    CPPUNIT_ASSERT(dv.isPremiumEconomyCabin(ft));

    ft = "WU";
    CPPUNIT_ASSERT(dv.isPremiumEconomyCabin(ft));
  }

  void testIsPremiumEconomyCabin_False()
  {
    DifferentialValidator dv;
    FareType ft = "*";
    CPPUNIT_ASSERT(!dv.isPremiumEconomyCabin(ft));

    ft = "W";
    CPPUNIT_ASSERT(!dv.isPremiumEconomyCabin(ft));

    ft = "BU";
    CPPUNIT_ASSERT(!dv.isPremiumEconomyCabin(ft));

    ft = "*B";
    CPPUNIT_ASSERT(!dv.isPremiumEconomyCabin(ft));
  }

  void testValidateHighFareForPremiumEconomyCabin_True()
  {
    DifferentialValidator dv;
    DifferentialData diffDP;
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    Fare mFare;
    PaxType mPaxType;

    FareInfo fi;
    TariffCrossRefInfo tcri;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);
    FareType fareType = "ER";
    fcai._fareType = fareType;
    paxTfare.fareClassAppInfo() = &fcai;
    paxTfare.fareClassAppSegInfo() = &fcasi;
    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);
    paxTfare.setFare(&mFare);

    diffDP.intermFareType() = "*B";
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*Z";
    diffDP.calcIntermIndicator() = 'N';
    fareType = "WU";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "WU";
    diffDP.calcIntermIndicator() = 'N';
    fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "ZEX";
    diffDP.calcIntermIndicator() = 'A';
    fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*Z";
    diffDP.calcIntermIndicator() = 'A';
    fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*W";
    diffDP.calcIntermIndicator() = 'A';
    fareType = "WU";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

  }

  void testValidateHighFareForPremiumEconomyCabin_False()
  {
    DifferentialValidator dv;
    DifferentialData diffDP;
    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    Fare mFare;
    PaxType mPaxType;

    FareInfo fi;
    TariffCrossRefInfo tcri;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);
    paxTfare.fareClassAppInfo() = &fcai;
    paxTfare.fareClassAppSegInfo() = &fcasi;
    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);
    paxTfare.setFare(&mFare);

    diffDP.intermFareType() = "ZEX";
    diffDP.calcIntermIndicator() = 'N';
    FareType  fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*Z";
    diffDP.calcIntermIndicator() = 'N';
    fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*W";
    diffDP.calcIntermIndicator() = 'N';
    fareType = "WR";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "ZEX";
    diffDP.calcIntermIndicator() = 'H';
    fareType = "WU";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*Z";
    diffDP.calcIntermIndicator() = 'H';
    fareType = "WR";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));

    diffDP.intermFareType() = "*W";
    diffDP.calcIntermIndicator() = 'H';
    fareType = "ZEX";
    fcai._fareType = fareType;
    CPPUNIT_ASSERT(!dv.validateHighFareForPremiumEconomyCabin(diffDP, paxTfare));
  }

  DifferentialValidator diffVal;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DifferentialValidatorTest);
}
