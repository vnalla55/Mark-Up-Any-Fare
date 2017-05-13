
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CircleTripProvision.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/FareMarketMerger.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/test/PricingMockDataBuilder.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include <iostream>

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (globalDir == GlobalDirection::XX)
      return 0;

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
  const CircleTripProvision*
  getCircleTripProvision(const LocCode& market1, const LocCode& market2, const DateTime& date)
  {
    CircleTripProvision* ret = _memHandle.create<CircleTripProvision>();
    ret->market1() = market1;
    ret->market2() = market2;
    if (market1 == "BSL" && market2 == "MLH")
      return ret;
    else if (market1 == "MLH" && market2 == "BSL")
      return ret;
    return DataHandleMock::getCircleTripProvision(market1, market2, date);
  }
};
}

class PUPathMatrixTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PUPathMatrixTest);
  CPPUNIT_TEST(testBuildAllPUPath);
  CPPUNIT_TEST(testIsInboundToCountry);
  CPPUNIT_TEST(testIsOutBoundFromCountry);
  CPPUNIT_TEST(testIsValidFCforCT_true_When_OrigDestSamePoint_ItinNotIntl);
  CPPUNIT_TEST(testIsValidFCforCT_true_When_OrigDestSamePoint_ItinIntl);
  CPPUNIT_TEST(testIsValidFCforCT_false_When_DestNotSamePoint_ItinNotIntl);
  CPPUNIT_TEST(testIsValidFCforCT_true_NotPassClosed_When_OrigNotSamePoint_ItinNotIntl);
  CPPUNIT_TEST(testIsValidFCforCT_trueByProvision_When_DestNotSamePoint_ItinIntl);
  CPPUNIT_TEST(testIsValidFCforCT_trueByProvision_When_OrigNotSamePoint_ItinIntl);
  CPPUNIT_TEST(testIsValidCT_true_When_NotPassedCTProvision_OnNoIntlPU);
  CPPUNIT_TEST(testIsValidCT_false_When_PassedCTProvision_OnNoIntlPU);
  CPPUNIT_TEST(testIsSamePointInFcc_false_When_NoFccSeg);
  CPPUNIT_TEST(testIsSamePointInFcc_false_When_NotOrigMatchAndNoDisplayLocs);
  CPPUNIT_TEST(testIsSamePointInFcc_false_When_NotDestMatchAndNotSamePointInFcc);
  CPPUNIT_TEST(testIsSamePointInFcc_true_When_NotDestMatchAndSamePointInFcc);
  CPPUNIT_TEST(testSetOWPUDirectionality_USCA_KeepAsOB);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2AreasPerLeg_false_When_Default_CarrierPref);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2AreasPerLeg_true_When_ShorterY_OJ_Shorter_ShorterTPM);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2AreasPerLeg_false_When_ShorterY_OJ_Longer_ShorterTPM);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2AreasPerLeg_true_When_LongerY_OJ_Shorter_LongerTPM);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2AreasPerLeg_false_When_LongerY_OJ_Longer_LongerTPM);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2Areas_true_When_BothLegsAreValid);
  CPPUNIT_TEST(testCheckMileageForTOJAcross2Areas_false_When_OneLegIsInvalid);
  CPPUNIT_TEST(testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_Orig);
  CPPUNIT_TEST(testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_Double);
  CPPUNIT_TEST(testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_OrigAllAreas);

  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildAllPUPath();
  void testIsInboundToCountry();
  void testIsOutBoundFromCountry();
  void testIsValidCT_true_When_NotPassedCTProvision_OnNoIntlPU();
  void testIsValidCT_false_When_PassedCTProvision_OnNoIntlPU();
  void testIsValidFCforCT_true_When_OrigDestSamePoint_ItinNotIntl();
  void testIsValidFCforCT_true_When_OrigDestSamePoint_ItinIntl();
  void testIsValidFCforCT_false_When_DestNotSamePoint_ItinNotIntl();
  void testIsValidFCforCT_true_NotPassClosed_When_OrigNotSamePoint_ItinNotIntl();
  void testIsValidFCforCT_trueByProvision_When_DestNotSamePoint_ItinIntl();
  void testIsValidFCforCT_trueByProvision_When_OrigNotSamePoint_ItinIntl();
  void testIsSamePointInFcc_false_When_NoFccSeg()
  {
    MergedFareMarket obOrigFM, ibDestFM, obDestFM, ibOrigFM;
    bool origMatch = true;
    bool destMatch = false;
    CPPUNIT_ASSERT(
        !_puMatrix.isSamePointInFcc(origMatch, obOrigFM, ibDestFM, destMatch, obDestFM, ibOrigFM));
  }

  void testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_Orig()
  {
    bool failOnMileage = false;
    bool invalidOJ = false;
    bool openJawBtwTwoAreas = false;
    PricingUnit::PUSubType ojType = PricingUnit::ORIG_OPENJAW;

    Loc* locCGK = PricingMockDataBuilder::getLoc(_trx, "CGK", "ID"); // obOrig
    Loc* locBKK = PricingMockDataBuilder::getLoc(_trx, "BKK", "TH"); // obDest ibOrig
    Loc* locDEL = PricingMockDataBuilder::getLoc(_trx, "DEL", "IN"); // ibOrig
    locCGK->subarea() = IATA_SUB_ARE_32(); // South East Asia
    locBKK->subarea() = IATA_SUB_ARE_32();
    locDEL->subarea() = IATA_SUB_ARE_33(); // Japan/Korea
    GlobalDirection obGD = GlobalDirection::EH;
    GlobalDirection ibGD = GlobalDirection::EH;

    CPPUNIT_ASSERT(_puMatrix.checkOJIataArea(*locCGK,
                                             *locBKK,
                                             obGD,
                                             *locBKK,
                                             *locDEL,
                                             ibGD,
                                             ojType,
                                             invalidOJ,
                                             openJawBtwTwoAreas,
                                             failOnMileage));

    CPPUNIT_ASSERT(failOnMileage);
  }

  void testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_Double()
  {
    bool failOnMileage = false;
    bool invalidOJ = false;
    bool openJawBtwTwoAreas = false;
    PricingUnit::PUSubType ojType = PricingUnit::ORIG_OPENJAW;

    Loc* locCGK = PricingMockDataBuilder::getLoc(_trx, "CGK", "ID"); // obOrig
    Loc* locBKK = PricingMockDataBuilder::getLoc(_trx, "BKK", "TH"); // obDest ibOrig
    Loc* locDEL = PricingMockDataBuilder::getLoc(_trx, "DEL", "IN"); // ibOrig
    locCGK->subarea() = IATA_SUB_ARE_32(); // South East Asia
    locBKK->subarea() = IATA_SUB_ARE_32();
    locDEL->subarea() = IATA_SUB_ARE_33(); // Japan/Korea
    GlobalDirection obGD = GlobalDirection::EH;
    GlobalDirection ibGD = GlobalDirection::EH;

    CPPUNIT_ASSERT(_puMatrix.checkOJIataArea(*locCGK,
                                             *locBKK,
                                             obGD,
                                             *locBKK,
                                             *locDEL,
                                             ibGD,
                                             ojType,
                                             invalidOJ,
                                             openJawBtwTwoAreas,
                                             failOnMileage));

    CPPUNIT_ASSERT(failOnMileage);
  }

  void testcheckOJIataAreaWhen_Fare_Components_Within_Subarea_OrigAllAreas()
  {
    bool failOnMileage = false;
    bool invalidOJ = false;
    bool openJawBtwTwoAreas = false;
    PricingUnit::PUSubType ojType = PricingUnit::ORIG_OPENJAW;

    Loc* locFRA = PricingMockDataBuilder::getLoc(_trx, "FRA", "GE"); // obOrig
    Loc* locDEL = PricingMockDataBuilder::getLoc(_trx, "DEL", "IN"); // obDest ibOrig
    Loc* locJFK = PricingMockDataBuilder::getLoc(_trx, "JFK", "US"); // ibOrig
    locFRA->subarea() = IATA_SUB_AREA_21();
    locDEL->subarea() = IATA_SUB_ARE_32();
    locJFK->subarea() = IATA_SUB_AREA_11();
    GlobalDirection obGD = GlobalDirection::EH;
    GlobalDirection ibGD = GlobalDirection::EH;

    _puMatrix._itinBoundary = PUPathMatrix::ALL_IATA;

    CPPUNIT_ASSERT(_puMatrix.checkOJIataArea(*locFRA,
                                             *locDEL,
                                             obGD,
                                             *locDEL,
                                             *locJFK,
                                             ibGD,
                                             ojType,
                                             invalidOJ,
                                             openJawBtwTwoAreas,
                                             failOnMileage));

    CPPUNIT_ASSERT(!failOnMileage);
  }

  void testIsSamePointInFcc_false_When_NotOrigMatchAndNoDisplayLocs()
  {
    addFareCalcConfigSeg(*_trx.fareCalcConfig());
    MergedFareMarket obOrigFM, ibDestFM, obDestFM, ibOrigFM;
    AirSeg* airSeg1 = _memHandle.create<AirSeg>();
    airSeg1->origAirport() = "SFO";
    obOrigFM.travelSeg().push_back(airSeg1);
    obOrigFM.boardMultiCity() = "SFO";
    AirSeg* airSeg2 = _memHandle.create<AirSeg>();
    airSeg2->destAirport() = "NRT";
    ibDestFM.travelSeg().push_back(airSeg2);
    ibDestFM.offMultiCity() = "NRT";
    bool origMatch = false;
    bool destMatch = true;
    CPPUNIT_ASSERT(
        !_puMatrix.isSamePointInFcc(origMatch, obOrigFM, ibDestFM, destMatch, obDestFM, ibOrigFM));
  }

  void testIsSamePointInFcc_false_When_NotDestMatchAndNotSamePointInFcc()
  {
    addFareCalcConfigSeg(*_trx.fareCalcConfig());
    MergedFareMarket obOrigFM, ibDestFM, obDestFM, ibOrigFM;
    AirSeg* airSeg1 = _memHandle.create<AirSeg>();
    airSeg1->destAirport() = "SFO";
    obDestFM.travelSeg().push_back(airSeg1);
    AirSeg* airSeg2 = _memHandle.create<AirSeg>();
    airSeg2->origAirport() = "JFK";
    ibOrigFM.travelSeg().push_back(airSeg2);
    bool origMatch = true;
    bool destMatch = false;
    CPPUNIT_ASSERT(
        !_puMatrix.isSamePointInFcc(origMatch, obOrigFM, ibDestFM, destMatch, obDestFM, ibOrigFM));
  }

  void testIsSamePointInFcc_true_When_NotDestMatchAndSamePointInFcc()
  {
    addFareCalcConfigSeg(*_trx.fareCalcConfig());
    MergedFareMarket obOrigFM, ibDestFM, obDestFM, ibOrigFM;
    AirSeg* airSeg1 = _memHandle.create<AirSeg>();
    airSeg1->destAirport() = "EWR";
    obDestFM.travelSeg().push_back(airSeg1);
    AirSeg* airSeg2 = _memHandle.create<AirSeg>();
    airSeg2->origAirport() = "JFK";
    ibOrigFM.travelSeg().push_back(airSeg2);
    bool origMatch = true;
    bool destMatch = false;
    CPPUNIT_ASSERT(
        _puMatrix.isSamePointInFcc(origMatch, obOrigFM, ibDestFM, destMatch, obDestFM, ibOrigFM));
    CPPUNIT_ASSERT(destMatch);
  }

  void addFareCalcConfigSeg(FareCalcConfig& fareCalcConfig)
  {
    FareCalcConfigSeg* fccSeg1 = new FareCalcConfigSeg;
    fccSeg1->marketLoc() = "EWR";
    fccSeg1->displayLoc() = "NYC";
    fareCalcConfig.segs().push_back(fccSeg1);
    FareCalcConfigSeg* fccSeg2 = new FareCalcConfigSeg;
    fccSeg2->marketLoc() = "JFK";
    fccSeg2->displayLoc() = "NYC";
    fareCalcConfig.segs().push_back(fccSeg2);
  }

  void testSetOWPUDirectionality_USCA_KeepAsOB()
  {
    _itin.geoTravelType() = GeoTravelType::Transborder;
    _pu1.puType() = PricingUnit::Type::ONEWAY;
    _pu1.fareDirectionality().push_back(FROM);

    _fm1.origin() = PricingMockDataBuilder::getLoc(_trx, "YVR", "CA");
    _fm1.destination() = PricingMockDataBuilder::getLoc(_trx, "SEA", "US");
    _pu1.fareMarket().push_back(&_fm1);

    _pu2.puType() = PricingUnit::Type::ONEWAY;
    _pu2.fareDirectionality().push_back(FROM);

    _fm2.origin() = PricingMockDataBuilder::getLoc(_trx, "SEA", "US");
    _fm2.destination() = PricingMockDataBuilder::getLoc(_trx, "YVR", "CA");
    _pu2.fareMarket().push_back(&_fm2);

    _puPath.puPath().push_back(&_pu1);
    _puPath.puPath().push_back(&_pu2);

    _puMatrix.setOWPUDirectionality(_puPath);
    CPPUNIT_ASSERT_EQUAL(FROM, _pu1.fareDirectionality().front());
    CPPUNIT_ASSERT_EQUAL(FROM, _pu2.fareDirectionality().front());
  }

  void testCheckMileageForTOJAcross2AreasPerLeg_false_When_Default_CarrierPref()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);
    _carrierPref.applySingleTOJBetwAreasShorterFC() = 'N';
    _carrierPref.applySingleTOJBetwAreasLongerFC() = 'N';

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 50;

    std::vector<CarrierCode> invalidCxrForTOJ;

    CPPUNIT_ASSERT(!_puMatrix.checkMileageForTOJAcross2AreasPerLeg(
        _fmktVect, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));

    CPPUNIT_ASSERT(!invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2AreasPerLeg_true_When_ShorterY_OJ_Shorter_ShorterTPM()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);
    _carrierPref.applySingleTOJBetwAreasShorterFC() = 'Y';

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 50;

    std::vector<CarrierCode> invalidCxrForTOJ;

    CPPUNIT_ASSERT(_puMatrix.checkMileageForTOJAcross2AreasPerLeg(
        _fmktVect, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));

    CPPUNIT_ASSERT(invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2AreasPerLeg_false_When_ShorterY_OJ_Longer_ShorterTPM()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);
    _carrierPref.applySingleTOJBetwAreasShorterFC() = 'Y';

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 150;

    std::vector<CarrierCode> invalidCxrForTOJ;

    CPPUNIT_ASSERT(!_puMatrix.checkMileageForTOJAcross2AreasPerLeg(
        _fmktVect, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));

    CPPUNIT_ASSERT(!invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2AreasPerLeg_true_When_LongerY_OJ_Shorter_LongerTPM()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);
    _carrierPref.applySingleTOJBetwAreasLongerFC() = 'Y';

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 150;

    std::vector<CarrierCode> invalidCxrForTOJ;

    CPPUNIT_ASSERT(_puMatrix.checkMileageForTOJAcross2AreasPerLeg(
        _fmktVect, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));

    CPPUNIT_ASSERT(invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2AreasPerLeg_false_When_LongerY_OJ_Longer_LongerTPM()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);
    _carrierPref.applySingleTOJBetwAreasLongerFC() = 'Y';

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 250;

    std::vector<CarrierCode> invalidCxrForTOJ;

    CPPUNIT_ASSERT(!_puMatrix.checkMileageForTOJAcross2AreasPerLeg(
        _fmktVect, smaller_mileage, larger_mileage, surfaceTOJ_mileage, invalidCxrForTOJ));

    CPPUNIT_ASSERT(!invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2Areas_true_When_BothLegsAreValid()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);

    _fareMarketInb.governingCarrier() = "BA";
    _fm2.mergedFareMarket().push_back(&_fareMarketInb);

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 50;

    std::vector<CarrierCode> invalidCxrForTOJ;
    _carrierPref.applySingleTOJBetwAreasShorterFC() = 'Y';
    _carrierPrefInb.applySingleTOJBetwAreasLongerFC() = 'Y';

    CPPUNIT_ASSERT(_puMatrix.checkMileageForTOJAcross2Areas(_fmktVect,
                                                            _fmktVectInb,
                                                            smaller_mileage,
                                                            larger_mileage,
                                                            surfaceTOJ_mileage,
                                                            invalidCxrForTOJ));

    CPPUNIT_ASSERT(invalidCxrForTOJ.empty());
  }

  void testCheckMileageForTOJAcross2Areas_false_When_OneLegIsInvalid()
  {
    _fareMarket1.governingCarrier() = "AA";
    _fm1.mergedFareMarket().push_back(&_fareMarket1);

    _fareMarketInb.governingCarrier() = "BA";
    _fm2.mergedFareMarket().push_back(&_fareMarketInb);

    const uint32_t smaller_mileage = 100;
    const uint32_t larger_mileage = 200;
    const uint32_t surfaceTOJ_mileage = 150;

    std::vector<CarrierCode> invalidCxrForTOJ;
    _carrierPref.applySingleTOJBetwAreasShorterFC() = 'Y';
    _carrierPrefInb.applySingleTOJBetwAreasLongerFC() = 'Y';

    CPPUNIT_ASSERT(!_puMatrix.checkMileageForTOJAcross2Areas(_fmktVect,
                                                             _fmktVectInb,
                                                             smaller_mileage,
                                                             larger_mileage,
                                                             surfaceTOJ_mileage,
                                                             invalidCxrForTOJ));

    CPPUNIT_ASSERT(!invalidCxrForTOJ.empty());
  }

  void setUp();
  void tearDown();

protected:
  Loc* getLoc(std::string code);
  FareMarket* getFareMarket(Loc* orig, Loc* dest);
  AirSeg* getAirSeg(Loc* orig, Loc* dest);

  void initTestForSPR120316(PricingTrx& trx,
                            Itin& itin,
                            Loc*& puOrig,
                            MergedFareMarket*& mfm1,
                            MergedFareMarket*& mfm2,
                            MergedFareMarket*& mfm3);

  void setTvlDFW_EWR_JFK_DFW();
  void setTvlFRA_EWR_JFK_FRA();
  void setTvlDFW_EWR_HOU_DEN();
  void setTvlDFW_EWR_EWR_DEN();
  void setTvl_Intl_DestOJ_ArunkPassCTProvision();
  void setTvl_Intl_OrigOJ_ArunkPassCTProvision();

private:
  static Logger _logger;

  PU _ctPU;
  std::vector<MergedFareMarket*> _fmktVect;
  MergedFareMarket _fm1, _fm2;

  PUPathMatrix _puMatrix;
  Itin _itin;
  PricingTrx _trx;
  PricingRequest _req;
  Directionality _dir;

  bool _passedCTProvision;
  bool _closed;

  ArunkSeg _arunk1;
  const FareCalcConfig* _fcConfig;
  PUPath _puPath;
  PU _pu1, _pu2;

  CarrierPreference _carrierPref;
  FareMarket _fareMarket1;

  std::vector<MergedFareMarket*> _fmktVectInb;
  CarrierPreference _carrierPrefInb;
  FareMarket _fareMarketInb;

  TestMemHandle _memHandle;
};

void
PUPathMatrixTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  AirSeg* _airSeg1 = _memHandle.create<AirSeg>();
  _fm1.travelSeg().push_back(_airSeg1);
  AirSeg* _airSeg2 = _memHandle.create<AirSeg>();
  _fm2.travelSeg().push_back(_airSeg2);

  _fmktVect.push_back(&_fm1);
  _puMatrix.itin() = &_itin;
  _trx.setRequest(&_req);
  _puMatrix.trx() = &_trx;
  _passedCTProvision = false;
  _closed = false;
  FareCalcConfig* fareCalcConfig = _memHandle.create<FareCalcConfig>();
  _trx.fareCalcConfig() = fareCalcConfig;
  _puMatrix._fcConfig = fareCalcConfig;
  _fareMarket1.governingCarrierPref() = &_carrierPref;

  _fmktVectInb.push_back(&_fm2);
  _fareMarketInb.governingCarrierPref() = &_carrierPrefInb;
  _memHandle.create<MyDataHandle>();
}
void
PUPathMatrixTest::tearDown()
{
  _fmktVect.clear();
  _fmktVectInb.clear();
  _memHandle.clear();
}
void
PUPathMatrixTest::testBuildAllPUPath()
{
  PricingTrx* trx = PricingMockDataBuilder::getPricingTrx();
  Itin* itin = PricingMockDataBuilder::addItin(*trx);

  // FareCalcConfig fareCalcConfig;
  // trx->fareCalcConfig() = &fareCalcConfig;
  trx->diagnostic().diagnosticType() = Diagnostic600;
  trx->diagnostic().activate();

  FareCalcConfig* fareCalcConfig = _memHandle.create<FareCalcConfig>();
  trx->fareCalcConfig() = fareCalcConfig;

  //---- Create Loc Objects  -------------
  Loc* locLAX = PricingMockDataBuilder::getLoc(*trx, "LAX");
  Loc* locDFW = PricingMockDataBuilder::getLoc(*trx, "DFW");
  Loc* locTUL = PricingMockDataBuilder::getLoc(*trx, "TUL");
  Loc* locLON = PricingMockDataBuilder::getLoc(*trx, "LON");
  Loc* locNYC = PricingMockDataBuilder::getLoc(*trx, "NYC");
  Loc* locORD = PricingMockDataBuilder::getLoc(*trx, "ORD");
  Loc* locAUS = PricingMockDataBuilder::getLoc(*trx, "AUS");
  Loc* locHOU = PricingMockDataBuilder::getLoc(*trx, "HOU");

  //---- Create Travel Seg  -------------
  TravelSeg* tvlsegLAXDFW =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locLAX, locDFW, 1);

  TravelSeg* tvlsegDFWTUL =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locDFW, locTUL, 2);
  TravelSeg* tvlsegTULDFW =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locTUL, locDFW, 3);

  TravelSeg* tvlsegDFWORD =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locDFW, locORD, 4);

  TravelSeg* tvlsegORDNYC =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locORD, locNYC, 5);
  TravelSeg* tvlsegNYCLON =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locNYC, locLON, 6);
  TravelSeg* tvlsegLONNYC =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locLON, locNYC, 7);
  TravelSeg* tvlsegNYCORD =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locNYC, locORD, 8);

  TravelSeg* tvlsegORDAUS =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locORD, locAUS, 9);
  TravelSeg* tvlsegAUSHOU =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locAUS, locHOU, 10);
  TravelSeg* tvlsegHOULAX =
      PricingMockDataBuilder::addTravelSeg(*trx, *itin, "AA", locHOU, locLAX, 11);

  std::vector<TravelSeg*> sideTrip;
  sideTrip.push_back(tvlsegDFWTUL);
  sideTrip.push_back(tvlsegTULDFW);

  //---  Create Fare Markets ------
  FareMarket* mktLAXDFW = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locLAX, locDFW);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegLAXDFW, *mktLAXDFW);

  FareMarket* mktLAXTUL = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locLAX, locDFW);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegLAXDFW, *mktLAXTUL);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWTUL, *mktLAXTUL);

  FareMarket* mktLAXORD = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locLAX, locORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegLAXDFW, *mktLAXORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWTUL, *mktLAXORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegTULDFW, *mktLAXORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktLAXORD);

  FareMarket* mktLAXORD_ST =
      PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locLAX, locORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegLAXDFW, *mktLAXORD_ST);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktLAXORD_ST);
  mktLAXORD_ST->sideTripTravelSeg().push_back(sideTrip);

  FareMarket* mktDFWTUL = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locDFW, locTUL);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWTUL, *mktDFWTUL);

  FareMarket* mktDFWORD = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locDFW, locORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktDFWORD);

  FareMarket* mktDFWNYC = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locDFW, locNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktDFWNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegORDNYC, *mktDFWNYC);

  FareMarket* mktDFWLON = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locDFW, locLON);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktDFWLON);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegORDNYC, *mktDFWLON);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegNYCLON, *mktDFWLON);

  FareMarket* mktTULDFW = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locTUL, locDFW);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegTULDFW, *mktTULDFW);

  FareMarket* mktTULNYC = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locTUL, locNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegTULDFW, *mktTULNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegDFWORD, *mktTULNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegORDNYC, *mktTULNYC);

  FareMarket* mktNYCLON = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locNYC, locLON);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegNYCLON, *mktNYCLON);

  FareMarket* mktLONNYC = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locLON, locNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegLONNYC, *mktLONNYC);

  FareMarket* mktNYCORD = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locNYC, locORD);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegNYCORD, *mktNYCORD);

  FareMarket* mktORDNYC = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locORD, locNYC);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegORDNYC, *mktORDNYC);

  FareMarket* mktORDAUS = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locORD, locAUS);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegORDAUS, *mktORDAUS);

  FareMarket* mktAUSHOU = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locAUS, locHOU);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegAUSHOU, *mktAUSHOU);

  FareMarket* mktHOULAX = PricingMockDataBuilder::addFareMarket(*trx, *itin, "AA", locHOU, locLAX);
  PricingMockDataBuilder::addTraveSegToFareMarket(tvlsegHOULAX, *mktHOULAX);

  std::vector<PricingUnitFactoryBucket*> puFactoryBucketVect;
  std::vector<PaxType*>::iterator paxIter = trx->paxType().begin();
  for (; paxIter != trx->paxType().end(); ++paxIter)
  {
    PricingUnitFactoryBucket* puFactoryBucket = 0;
    trx->dataHandle().get(puFactoryBucket);
    puFactoryBucket->paxType() = *paxIter;
    puFactoryBucketVect.push_back(puFactoryBucket);
  }

  std::vector<MergedFareMarket*> mergedFareMarketVect;
  CarrierCode carrier = "AA";
  FareMarketMerger fmMerger(*trx, *itin, true, &carrier);
  fmMerger.buildAllMergedFareMarket(mergedFareMarketVect);

  //------- Build FareMarket Path Matrix ----------
  // std::cout << std::endl << "Build FareMarketPathMatrix" << std::endl;
  FareMarketPathMatrix fmpMatrix(*trx, *itin, mergedFareMarketVect);
  bool ret = fmpMatrix.buildAllFareMarketPath();

  //------- Build PUPath Matrix ----------

  // PUPathMatrix* puMatrix = 0;
  // trx.dataHandle().get(puMatrix);
  _puMatrix.trx() = trx;
  _puMatrix.itin() = itin;
  _puMatrix._taskId = TseThreadingConst::SYNCHRONOUS_TASK;

  ret = _puMatrix.buildAllPUPath(fmpMatrix, puFactoryBucketVect);

  //------- Print Diagnostic Messages -----------
  // std::cout << std::endl << trx->diagnostic().toString() << std::endl;
  delete trx;
  CPPUNIT_ASSERT_EQUAL(ret, true);
}

void
PUPathMatrixTest::testIsInboundToCountry()
{
  PricingTrx* trx = PricingMockDataBuilder::getPricingTrx();
  Itin* itin = PricingMockDataBuilder::addItin(*trx);

  Loc* locYVR = 0;
  MergedFareMarket* mfmYVR_SEA = 0;
  MergedFareMarket* mfmSEA_CUN = 0;
  MergedFareMarket* mfmCUN_YVR = 0;

  initTestForSPR120316(*trx, *itin, locYVR, mfmYVR_SEA, mfmSEA_CUN, mfmCUN_YVR);

  _puMatrix.trx() = trx;
  _puMatrix.itin() = itin;

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isInboundToCountry(*locYVR, *mfmYVR_SEA), false);

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isInboundToCountry(*locYVR, *mfmSEA_CUN), false);

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isInboundToCountry(*locYVR, *mfmCUN_YVR), true);

  delete trx;
}

void
PUPathMatrixTest::testIsOutBoundFromCountry()
{
  PricingTrx* trx = PricingMockDataBuilder::getPricingTrx();
  Itin* itin = PricingMockDataBuilder::addItin(*trx);

  Loc* locYVR = 0;
  MergedFareMarket* mfmYVR_SEA = 0;
  MergedFareMarket* mfmSEA_CUN = 0;
  MergedFareMarket* mfmCUN_YVR = 0;

  initTestForSPR120316(*trx, *itin, locYVR, mfmYVR_SEA, mfmSEA_CUN, mfmCUN_YVR);

  _puMatrix.trx() = trx;
  _puMatrix.itin() = itin;

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isOutBoundFromCountry(*locYVR, *mfmYVR_SEA), false);

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isOutBoundFromCountry(*locYVR, *mfmSEA_CUN), true);

  CPPUNIT_ASSERT_EQUAL(_puMatrix.isOutBoundFromCountry(*locYVR, *mfmCUN_YVR), false);

  delete trx;
}

void
PUPathMatrixTest::initTestForSPR120316(PricingTrx& trx,
                                       Itin& itin,
                                       Loc*& puOrig,
                                       MergedFareMarket*& mfmYVR_SEA,
                                       MergedFareMarket*& mfmSEA_CUN,
                                       MergedFareMarket*& mfmCUN_YVR)
{
  itin.geoTravelType() = GeoTravelType::International;

  Loc* locYVR = PricingMockDataBuilder::getLoc(trx, "YVR", "CA");
  Loc* locSEA = PricingMockDataBuilder::getLoc(trx, "SEA", "US");
  Loc* locHOU = PricingMockDataBuilder::getLoc(trx, "HOU", "US");
  Loc* locCUN = PricingMockDataBuilder::getLoc(trx, "CUN", "MX");

  puOrig = locYVR;

  TravelSeg* tSeg1 = PricingMockDataBuilder::addTravelSeg(trx, itin, "AS", locYVR, locSEA, 1);
  TravelSeg* tSeg2 = PricingMockDataBuilder::addTravelSeg(trx, itin, "AS", locSEA, locCUN, 2);
  TravelSeg* tSeg3 = PricingMockDataBuilder::addTravelSeg(trx, itin, "CO", locCUN, locHOU, 3);
  TravelSeg* tSeg4 = PricingMockDataBuilder::addTravelSeg(trx, itin, "CO", locHOU, locCUN, 4);

  FareMarket* fmYVR_SEA = PricingMockDataBuilder::addFareMarket(trx, itin, "AS", locYVR, locSEA);
  PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fmYVR_SEA);
  mfmYVR_SEA = PricingMockDataBuilder::getMergedFareMarket(trx, fmYVR_SEA);

  FareMarket* fmSEA_CUN = PricingMockDataBuilder::addFareMarket(trx, itin, "AS", locSEA, locCUN);
  PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fmSEA_CUN);
  mfmSEA_CUN = PricingMockDataBuilder::getMergedFareMarket(trx, fmSEA_CUN);

  FareMarket* fmCUN_YVR = PricingMockDataBuilder::addFareMarket(trx, itin, "CO", locCUN, locYVR);
  PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fmCUN_YVR);
  PricingMockDataBuilder::addTraveSegToFareMarket(tSeg4, *fmCUN_YVR);
  mfmCUN_YVR = PricingMockDataBuilder::getMergedFareMarket(trx, fmCUN_YVR);
}

Loc*
PUPathMatrixTest::getLoc(std::string code)
{
  Loc* loc = _memHandle.create<Loc>();
  LocCode locCode = code;
  loc->loc() = (locCode);
  return loc;
}

AirSeg*
PUPathMatrixTest::getAirSeg(Loc* orig, Loc* dest)
{
  AirSeg* tvlseg = _memHandle.create<AirSeg>();
  tvlseg->origin() = orig;
  tvlseg->destination() = dest;
  tvlseg->origAirport() = orig->loc();
  tvlseg->destAirport() = dest->loc();
  tvlseg->boardMultiCity() = orig->loc();
  tvlseg->offMultiCity() = dest->loc();
  tvlseg->departureDT() = DateTime::localTime();
  tvlseg->segmentOrder() = 0;
  return tvlseg;
}

FareMarket*
PUPathMatrixTest::getFareMarket(Loc* orig, Loc* dest)
{
  FareMarket* fareMarket = _memHandle.create<FareMarket>();

  fareMarket->origin() = (orig);
  fareMarket->destination() = (dest);

  fareMarket->boardMultiCity() = orig->loc();
  fareMarket->offMultiCity() = dest->loc();

  GlobalDirection globleDirection = GlobalDirection::AT;
  fareMarket->setGlobalDirection(globleDirection);
  CarrierCode cxrCode = "BA";
  fareMarket->governingCarrier() = cxrCode;

  // LON-BA-NYC    /CXR-BA/ #GI-XX#  .OUT.
  // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   1845.26
  Fare* fare1 = _memHandle.create<Fare>();
  fare1->nucFareAmount() = 1845.26;

  FareInfo* fareInfo1 = _memHandle.create<FareInfo>();
  fareInfo1->_carrier = "BA";
  fareInfo1->_market1 = orig->loc();
  fareInfo1->_market2 = dest->loc();
  fareInfo1->_fareClass = "WMLUQOW";
  fareInfo1->_fareAmount = 999.00;
  fareInfo1->_currency = "GBP";
  fareInfo1->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
  fareInfo1->_ruleNumber = "5135";
  fareInfo1->_routingNumber = "XXXX";
  fareInfo1->_directionality = FROM;
  fareInfo1->_globalDirection = GlobalDirection::AT;
  fareInfo1->_vendor = Vendor::ATPCO;

  TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
  tariffRefInfo->_fareTariffCode = "TAFPBA";

  fare1->initialize(Fare::FS_International, fareInfo1, *fareMarket, tariffRefInfo);

  PaxType* paxType = _memHandle.create<PaxType>();
  PaxTypeCode paxTypeCode = "ADT";
  paxType->paxType() = paxTypeCode;

  PaxTypeFare* paxTypeFare1 = _memHandle.create<PaxTypeFare>();
  paxTypeFare1->initialize(fare1, paxType, fareMarket);

  FareClassAppInfo* appInfo1 = _memHandle.create<FareClassAppInfo>();
  appInfo1->_fareType = "EU";
  paxTypeFare1->fareClassAppInfo() = appInfo1;

  FareClassAppSegInfo* fareClassAppSegInfo1 = _memHandle.create<FareClassAppSegInfo>();
  fareClassAppSegInfo1->_paxType = "ADT";
  paxTypeFare1->fareClassAppSegInfo() = fareClassAppSegInfo1;

  PaxTypeBucket paxTypeCortege;
  paxTypeCortege.requestedPaxType() = paxType;
  paxTypeCortege.paxTypeFare().push_back(paxTypeFare1);
  fareMarket->paxTypeCortege().push_back(paxTypeCortege);
  return fareMarket;
}

void
PUPathMatrixTest::testIsValidCT_true_When_NotPassedCTProvision_OnNoIntlPU()
{
  PU ctPU;
  ctPU.fareMarket().resize(
      3); // so to pass isValidCT if not fail PU geoTravelType/passedCTProvision
  ctPU.geoTravelType() = GeoTravelType::Domestic;
  const bool passedCTProvision = false;

  CPPUNIT_ASSERT(_puMatrix.isValidCT(ctPU, passedCTProvision));

  ctPU.geoTravelType() = GeoTravelType::ForeignDomestic;
  CPPUNIT_ASSERT(_puMatrix.isValidCT(ctPU, passedCTProvision));
}

void
PUPathMatrixTest::testIsValidCT_false_When_PassedCTProvision_OnNoIntlPU()
{
  PU ctPU;
  ctPU.fareMarket().resize(
      3); // so to pass isValidCT if not fail PU geoTravelType/passedCTProvision
  ctPU.geoTravelType() = GeoTravelType::Domestic;
  const bool passedCTProvision = true;

  CPPUNIT_ASSERT(!_puMatrix.isValidCT(ctPU, passedCTProvision));

  ctPU.geoTravelType() = GeoTravelType::ForeignDomestic;
  CPPUNIT_ASSERT(!_puMatrix.isValidCT(ctPU, passedCTProvision));
}

void
PUPathMatrixTest::setTvlDFW_EWR_JFK_DFW()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  _fm1.boardMultiCity() = "DFW";
  _fm1.offMultiCity() = "NYC";
  _fm2.boardMultiCity() = "NYC";
  _fm2.offMultiCity() = "DFW";

  _itin.geoTravelType() = GeoTravelType::Domestic;
}

void
PUPathMatrixTest::testIsValidFCforCT_true_When_OrigDestSamePoint_ItinNotIntl()
{
  setTvlDFW_EWR_JFK_DFW();

  CPPUNIT_ASSERT(
      _puMatrix.isValidFCforCT(_ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed));

  CPPUNIT_ASSERT(!_passedCTProvision);
  CPPUNIT_ASSERT(_closed);
}

void
PUPathMatrixTest::setTvlFRA_EWR_JFK_FRA()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
  _fm1.boardMultiCity() = "FRA";
  _fm1.offMultiCity() = "NYC";
  _fm2.boardMultiCity() = "NYC";
  _fm2.offMultiCity() = "FRA";

  _itin.geoTravelType() = GeoTravelType::International;
}

void
PUPathMatrixTest::testIsValidFCforCT_true_When_OrigDestSamePoint_ItinIntl()
{
  setTvlFRA_EWR_JFK_FRA();

  CPPUNIT_ASSERT(
      _puMatrix.isValidFCforCT(_ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed));

  CPPUNIT_ASSERT(!_passedCTProvision);
  CPPUNIT_ASSERT(_closed);
}

void
PUPathMatrixTest::setTvlDFW_EWR_HOU_DEN()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHOU.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDEN.xml");
  _fm1.boardMultiCity() = "DFW";
  _fm1.offMultiCity() = "NYC";
  _fm2.boardMultiCity() = "HOU";
  _fm2.offMultiCity() = "DEN";

  _itin.geoTravelType() = GeoTravelType::Domestic;
}

void
PUPathMatrixTest::testIsValidFCforCT_false_When_DestNotSamePoint_ItinNotIntl()
{
  setTvlDFW_EWR_HOU_DEN();

  CPPUNIT_ASSERT(
      !_puMatrix.isValidFCforCT(_ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed));

  CPPUNIT_ASSERT(!_passedCTProvision);
  CPPUNIT_ASSERT(!_closed);
}

void
PUPathMatrixTest::setTvlDFW_EWR_EWR_DEN()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDEN.xml");
  _fm1.boardMultiCity() = "DFW";
  _fm1.offMultiCity() = "NYC";
  _fm2.boardMultiCity() = "NYC";
  _fm2.offMultiCity() = "DEN";

  _itin.geoTravelType() = GeoTravelType::Domestic;
}

void
PUPathMatrixTest::testIsValidFCforCT_true_NotPassClosed_When_OrigNotSamePoint_ItinNotIntl()
{
  setTvlDFW_EWR_EWR_DEN();

  CPPUNIT_ASSERT(_puMatrix.isValidFCforCT(
      _ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed)); // can be for more than 2 FC CT

  CPPUNIT_ASSERT(!_passedCTProvision);
  CPPUNIT_ASSERT(!_closed);
}

void
PUPathMatrixTest::setTvl_Intl_DestOJ_ArunkPassCTProvision()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMFM.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");

  _itin.geoTravelType() = GeoTravelType::International;
  _itin.travelSeg().insert(
      _itin.travelSeg().end(), _fm1.travelSeg().begin(), _fm1.travelSeg().end());
  // BSL MLH is a city pair that we can find in provision table
  _arunk1.boardMultiCity() = "BSL";
  _arunk1.offMultiCity() = "MLH";
  _arunk1.bookingDT() = DateTime::localTime();
  _itin.travelSeg().push_back(&_arunk1);
  _itin.travelSeg().insert(
      _itin.travelSeg().end(), _fm2.travelSeg().begin(), _fm2.travelSeg().end());
}

void
PUPathMatrixTest::testIsValidFCforCT_trueByProvision_When_DestNotSamePoint_ItinIntl()
{
  setTvl_Intl_DestOJ_ArunkPassCTProvision();

  CPPUNIT_ASSERT(
      _puMatrix.isValidFCforCT(_ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed));

  CPPUNIT_ASSERT(_passedCTProvision);
  CPPUNIT_ASSERT(_closed);
}

void
PUPathMatrixTest::setTvl_Intl_OrigOJ_ArunkPassCTProvision()
{
  _fm1.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
  _fm1.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
  _fm2.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
  _fm2.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMFM.xml");
  // BSL MLH is a city pair that we can find in provision table
  _fm1.boardMultiCity() = "BSL";
  _fm1.offMultiCity() = "SIN";
  _fm2.boardMultiCity() = "SIN";
  _fm2.travelSeg().back()->offMultiCity() = "MLH";
  _fm2.travelSeg().back()->bookingDT() = DateTime::localTime();

  _itin.geoTravelType() = GeoTravelType::International;
  _itin.travelSeg().insert(
      _itin.travelSeg().end(), _fm1.travelSeg().begin(), _fm1.travelSeg().end());
  _itin.travelSeg().insert(
      _itin.travelSeg().end(), _fm2.travelSeg().begin(), _fm2.travelSeg().end());
}

void
PUPathMatrixTest::testIsValidFCforCT_trueByProvision_When_OrigNotSamePoint_ItinIntl()
{
  setTvl_Intl_OrigOJ_ArunkPassCTProvision();

  CPPUNIT_ASSERT(
      _puMatrix.isValidFCforCT(_ctPU, _fmktVect, _fm2, _dir, _passedCTProvision, _closed));

  CPPUNIT_ASSERT(_passedCTProvision);
  CPPUNIT_ASSERT(_closed);
}

Logger
PUPathMatrixTest::_logger("atseintl.Pricing.test.PUPathMatrixTest");

CPPUNIT_TEST_SUITE_REGISTRATION(PUPathMatrixTest);
}
