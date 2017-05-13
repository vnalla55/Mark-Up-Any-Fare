#include <iostream>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "Common/GoverningCarrier.h"
#include "Common/TravelSegAnalysis.h"

namespace tse
{

struct AirSegDesc
{
  IATAAreaCode orgArea;
  IATASubAreaCode orgSubarea;
  NationCode orgNation;
  IATAAreaCode dstArea;
  IATASubAreaCode dstSubarea;
  NationCode dstNation;
  CarrierCode cxr;
};

static const AirSegDesc RtwTestSegments[] = {
  { IATA_AREA2, "S21", "N211", IATA_AREA2, "S21", "N211", "C1" },
  { IATA_AREA2, "S21", "N211", IATA_AREA2, "S21", "N212", "C2" }, // different nation
  { IATA_AREA2, "S21", "N212", IATA_AREA2, "S22", "N221", "C3" }, // different subarea
  { IATA_AREA2, "S22", "N221", IATA_AREA3, "S31", "N311", "C4" }, // different area
  { IATA_AREA3, "S31", "N311", IATA_AREA1, "S11", "N111", "C5" } // IATA1
};

class MockGoverningCarrier : public GoverningCarrier
{
public:
  uint32_t getTPM(const AirSeg& airSeg)
  {
    GlobalDirection gd = GlobalDirection::XX;

    const Loc& loc1 = *(airSeg.origin());
    const Loc& loc2 = *(airSeg.destination());

    DataHandle dataHandle;

    return LocUtil::getTPM(loc1, loc2, gd, airSeg.departureDT(), dataHandle);
  }
};

class GoverningCarrierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GoverningCarrierTest);
  CPPUNIT_TEST(testNegativeOpenArunkInbound);
  CPPUNIT_TEST(testNegativeOpenArunkOutbound);
  CPPUNIT_TEST(testProcessRtw_OpenArunk);
  CPPUNIT_TEST(testProcessRtw_SameSubarea);
  CPPUNIT_TEST(testProcessRtw_SameArea);
  CPPUNIT_TEST(testProcessRtw_TwoAreas);
  CPPUNIT_TEST(testProcessRtw_Transatlantic);
  CPPUNIT_TEST(testGetGoverningCarrierRtw_OpenArunk);
  CPPUNIT_TEST(testGetGoverningCarrierRtw_SameSubarea);
  CPPUNIT_TEST(testGetGoverningCarrierRtw_SameArea);
  CPPUNIT_TEST(testGetGoverningCarrierRtw_TwoAreas);
  CPPUNIT_TEST(testGetGoverningCarrierRtw_Transatlantic);
  CPPUNIT_TEST(testGetHighestTPMCxr_BA);
  CPPUNIT_TEST(testGetHighestTPMCxr_Domectic_Plus_International_CC);
  CPPUNIT_TEST(testGetHighestTPMCxr_Domectic_Arunk_International_CA);
  CPPUNIT_TEST(testGetHighestTPMCxr_Domectic_Arunk_International_CC);
  CPPUNIT_TEST(testGetHighestTPMCxr_Domectic_Arunk_International_MX);
  CPPUNIT_TEST(testGetHighestTPMCxr_Inbound_Domectic_Plus_International_SQ);
  CPPUNIT_TEST(testSetFirstInternationalCrossing_OutBound);
  CPPUNIT_TEST(testSetFirstInternationalCrossing_InBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_SubArea21_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_SubArea21_InBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_USCA_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_USCA_InBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_ONE_IATA_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_ONE_IATA_InBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_EXCEPT_USCA_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_AREA_11_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_AREA_11_InBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_TWO_IATA_OutBound);
  CPPUNIT_TEST(testSelectFirstCrossingGovCxr_TWO_IATA_InBound);
  CPPUNIT_TEST(testGetForeignDomHighestTPMCarrier);
  CPPUNIT_TEST(testGetForeignDomHighestTPMByCarrier);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  GoverningCarrier* _gc;
  GoverningCarrier* _gcRtw;
  MockGoverningCarrier* _ogc;

  Itin* _itin;
  FareMarket* _fareMarket;
  std::vector<TravelSeg*>* _tvlSegs;
  std::set<CarrierCode>* _govCxrSet;
  TravelSeg* _primarySector;
  FMDirection* _direction;
  ArunkSeg* _arunk;
  AirSeg* _air;
  PricingTrx* _trx;

public:
  GoverningCarrierTest() {  }

  void setUp()
  {
    PricingTrx* trx = _memHandle(new PricingTrx);
    PricingOptions* po = _memHandle(new PricingOptions);

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(po);
    _trx->setRequest(_memHandle.create<PricingRequest>());

    po->setRtw(true);
    trx->setOptions(po);

    _memHandle.insert(_ogc = new MockGoverningCarrier());
    _gc = _memHandle(new GoverningCarrier);
    _gcRtw = _memHandle(new GoverningCarrier(trx));
    _itin = _memHandle(new Itin);
    trx->itin().push_back(_itin);

    _fareMarket = _memHandle(new FareMarket);
    _tvlSegs = _memHandle(new std::vector<TravelSeg*>);
    _govCxrSet = _memHandle(new std::set<CarrierCode>);
    _primarySector = 0;
    _direction = _memHandle(new FMDirection(FMDirection::UNKNOWN));

    _arunk = _memHandle(new ArunkSeg);
    matchAllCriteria(*_arunk);

    _air = _memHandle(new AirSeg);
    matchAllCriteria(*_air);
  }

  void tearDown() { _memHandle.clear(); }

  void testExecutor(bool sucessStatus)
  {
    CPPUNIT_ASSERT_EQUAL(
        sucessStatus, _gc->selectWithinUSCA(*_tvlSegs, *_govCxrSet, *_direction, &_primarySector));
    CPPUNIT_ASSERT_EQUAL(
        sucessStatus,
        _gc->selectWithinSameIATA(*_tvlSegs, *_govCxrSet, *_direction, &_primarySector));
    CPPUNIT_ASSERT_EQUAL(
        sucessStatus,
        _gc->selectWithinMultiIATA(*_tvlSegs, *_govCxrSet, *_direction, &_primarySector));
    CPPUNIT_ASSERT_EQUAL(!sucessStatus, _govCxrSet->empty());
    CPPUNIT_ASSERT_EQUAL(sucessStatus,
                         static_cast<bool>(_gc->findTravelSegInAreaOne(*_tvlSegs, *_direction)));
  }

  void testNegativeOpenArunkInbound()
  {
    *_direction = FMDirection::INBOUND;
    _tvlSegs->push_back(_arunk);
    testExecutor(true);
  }

  void testNegativeOpenArunkOutbound()
  {
    *_direction = FMDirection::OUTBOUND;
    _tvlSegs->push_back(_arunk);
    testExecutor(true);
  }

  void testProcessRtw_OpenArunk()
  {
    _tvlSegs->push_back(_arunk);
    _itin->travelSeg() = *_tvlSegs;
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA);

    CPPUNIT_ASSERT(!_gcRtw->processRtw(*_fareMarket));
  }

  void testProcessRtw_SameSubarea()
  {
    initRtwSegments(RtwTestSegments, 2, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11);

    CPPUNIT_ASSERT(_gcRtw->processRtw(*_fareMarket));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C2"), _fareMarket->governingCarrier());
  }

  void testProcessRtw_SameArea()
  {
    initRtwSegments(RtwTestSegments, 3, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA);

    CPPUNIT_ASSERT(_gcRtw->processRtw(*_fareMarket));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C3"), _fareMarket->governingCarrier());
  }

  void testProcessRtw_TwoAreas()
  {
    initRtwSegments(RtwTestSegments, 4, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA);

    CPPUNIT_ASSERT(_gcRtw->processRtw(*_fareMarket));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C4"), _fareMarket->governingCarrier());
  }

  void testProcessRtw_Transatlantic()
  {
    initRtwSegments(RtwTestSegments, 5, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA);

    CPPUNIT_ASSERT(_gcRtw->processRtw(*_fareMarket));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C5"), _fareMarket->governingCarrier());
  }

  void testGetGoverningCarrierRtw_OpenArunk()
  {
    _tvlSegs->push_back(_arunk);
    _itin->travelSeg() = *_tvlSegs;
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA);

    CPPUNIT_ASSERT(
        !_gcRtw->getGoverningCarrierRtw(_fareMarket->travelSeg(), Boundary::ALL_IATA, *_govCxrSet));
  }

  void testGetGoverningCarrierRtw_SameSubarea()
  {
    initRtwSegments(RtwTestSegments, 2, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11);

    CPPUNIT_ASSERT(_gcRtw->getGoverningCarrierRtw(
        _fareMarket->travelSeg(), Boundary::OTHER_SUB_IATA, *_govCxrSet));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), _govCxrSet->size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C2"), *_govCxrSet->begin());
  }

  void testGetGoverningCarrierRtw_SameArea()
  {
    initRtwSegments(RtwTestSegments, 3, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA);

    CPPUNIT_ASSERT(
        _gcRtw->getGoverningCarrierRtw(_fareMarket->travelSeg(), Boundary::ONE_IATA, *_govCxrSet));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), _govCxrSet->size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C3"), *_govCxrSet->begin());
  }

  void testGetGoverningCarrierRtw_TwoAreas()
  {
    initRtwSegments(RtwTestSegments, 4, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA);

    CPPUNIT_ASSERT(
        _gcRtw->getGoverningCarrierRtw(_fareMarket->travelSeg(), Boundary::TWO_IATA, *_govCxrSet));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), _govCxrSet->size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C4"), *_govCxrSet->begin());
  }

  void testGetGoverningCarrierRtw_Transatlantic()
  {
    initRtwSegments(RtwTestSegments, 5, _fareMarket->travelSeg());
    _itin->travelSeg() = _fareMarket->travelSeg();
    _fareMarket->travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA);

    CPPUNIT_ASSERT(
        _gcRtw->getGoverningCarrierRtw(_fareMarket->travelSeg(), Boundary::ALL_IATA, *_govCxrSet));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), _govCxrSet->size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("C5"), *_govCxrSet->begin());
  }

  void testGetHighestTPMCxr_BA()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // 5360 TS
    _tvlSegs->push_back(createAirSeg("SEL", "FRA", "SQ"));  // 5360 TS
    _tvlSegs->push_back(createAirSeg("SEL", "TYO", "CA"));  // 758 EH

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), cxr);
    TravelSeg* ts = _tvlSegs->at(1);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testGetHighestTPMCxr_Domectic_Plus_International_CC()
  {
    _tvlSegs->push_back(createAirSeg("JFK", "SFO", "CC"));  // 2572 --
    _tvlSegs->push_back(createAirSeg("SFO", "SYD", "CC"));  // 7426 PA total on CC = 9998
    _tvlSegs->push_back(createAirSeg("SYD", "ATH", "CA"));  // 9521 PA

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), cxr);
    TravelSeg* ts = _tvlSegs->at(1);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testGetHighestTPMCxr_Domectic_Arunk_International_CA()
  {
    _tvlSegs->push_back(createAirSeg("JFK", "DEN", "CC"));  // 1629 --
    _tvlSegs->push_back(createArunkSeg("DEN", "SFO"));        // 954 arunk
    _tvlSegs->push_back(createAirSeg("SFO", "SYD", "CC"));  // 7426 PA total on CC = 9998
    _tvlSegs->push_back(createAirSeg("SYD", "ATH", "CA"));  // 9521 PA

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CA"), cxr);
    TravelSeg* ts = _tvlSegs->at(3);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testGetHighestTPMCxr_Domectic_Arunk_International_CC()
  {
    _tvlSegs->push_back(createAirSeg("JFK", "DEN", "CC"));  // 1629 --
    _tvlSegs->push_back(createArunkSeg("DEN", "SFO"));        // 954 arunk
    _tvlSegs->push_back(createAirSeg("SFO", "SYD", "CC"));  // 7426 PA total on CC = 9998
    _tvlSegs->push_back(createAirSeg("SYD", "DEL", "CA"));  // 6485 PA

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CC"), cxr);
    TravelSeg* ts = _tvlSegs->at(2);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testGetHighestTPMCxr_Domectic_Arunk_International_MX()
  {
    _tvlSegs->push_back(createAirSeg("JFK", "CHI", "AA"));  // 725 --
    _tvlSegs->push_back(createArunkSeg("CHI", "DEN"));      // arunk
    _tvlSegs->push_back(createAirSeg("DEN", "SAN", "AA"));  // 833
    _tvlSegs->push_back(createAirSeg("SAN", "DFW", "UA"));  // 1175
    _tvlSegs->push_back(createAirSeg("DFW", "MEX", "MX"));  // 944

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("MX"), cxr);
    TravelSeg* ts = _tvlSegs->at(4);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }


  void testGetHighestTPMCxr_Inbound_Domectic_Plus_International_SQ()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // 5360 TS
    _tvlSegs->push_back(createAirSeg("SEL", "FRA", "SQ"));  // 5360 TS
    _tvlSegs->push_back(createAirSeg("SEL", "TYO", "CA"));  // 758 EH

    CarrierCode cxr = _ogc->getHighestTPMCarrier(*_tvlSegs, FMDirection::INBOUND, _primarySector);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("SQ"), cxr);
    TravelSeg* ts = _tvlSegs->at(2);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testSetFirstInternationalCrossing_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // 5360 TS

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    CPPUNIT_ASSERT(_ogc->setFirstInternationalCrossing(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSetFirstInternationalCrossing_InBound()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // 5360 TS

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    CPPUNIT_ASSERT(_ogc->setFirstInternationalCrossing(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_SubArea21_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "WAW", "BA"));  // 5360 TS

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::AREA_21);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_SubArea21_InBound()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // 1264 EH
    _tvlSegs->push_back(createAirSeg("FRA", "WAW", "BA"));  // 5360 TS

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::AREA_21);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_USCA_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("DFW", "DEN", "AA"));
    _tvlSegs->push_back(createAirSeg("DEN", "BOS", "BA"));

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::USCA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_USCA_InBound()
  {
    _tvlSegs->push_back(createAirSeg("DFW", "DEN", "AA"));
    _tvlSegs->push_back(createAirSeg("DEN", "BOS", "BA"));

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::USCA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_ONE_IATA_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("DFW", "HAV", "AA"));
    _tvlSegs->push_back(createAirSeg("HAV", "SAO", "BA"));

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::ONE_IATA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_ONE_IATA_InBound()
  {
    _tvlSegs->push_back(createAirSeg("DFW", "HAV", "AA"));
    _tvlSegs->push_back(createAirSeg("HAV", "SAO", "BA"));

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::ONE_IATA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_EXCEPT_USCA_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("COR", "BUE", "AA"));
    _tvlSegs->push_back(createAirSeg("BUE", "EZE", "BA"));

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::EXCEPT_USCA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_AREA_11_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("MEX", "HDQ", "AA"));
    _tvlSegs->push_back(createAirSeg("HDQ", "YYT", "BA"));

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::AREA_11);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_AREA_11_InBound()
  {
    _tvlSegs->push_back(createAirSeg("MEX", "HDQ", "AA"));
    _tvlSegs->push_back(createAirSeg("HDQ", "YYT", "BA"));

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::AREA_11);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_TWO_IATA_OutBound()
  {
    _tvlSegs->push_back(createAirSeg("MEX", "MAD", "AA"));
    _tvlSegs->push_back(createAirSeg("MAD", "MOW", "BA"));

    std::set<CarrierCode> govCxrSet;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::TWO_IATA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, FMDirection::OUTBOUND, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testSelectFirstCrossingGovCxr_TWO_IATA_InBound()
  {
    _tvlSegs->push_back(createAirSeg("MEX", "MAD", "AA"));
    _tvlSegs->push_back(createAirSeg("MAD", "MOW", "BA"));

    std::set<CarrierCode> govCxrSet;
    *_direction = FMDirection::INBOUND;
    TravelSeg* dummy = 0;

    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(*_tvlSegs);

    CPPUNIT_ASSERT(boundary == Boundary::TWO_IATA);
    CPPUNIT_ASSERT(_ogc->selectFirstCrossingGovCxr(*_tvlSegs, govCxrSet, *_direction, dummy));
    CPPUNIT_ASSERT(!govCxrSet.empty());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), *(govCxrSet.begin()));
  }

  void testGetForeignDomHighestTPMCarrier()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // Mileage 1262
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // Mileage 5327
    _tvlSegs->push_back(createAirSeg("SEL", "TYO", "CA"));  // Mileage 749

    CarrierCode cxr = _ogc->getForeignDomHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), cxr);
    TravelSeg* ts = _tvlSegs->at(1);
    CPPUNIT_ASSERT_EQUAL( ts, _primarySector);
  }

  void testGetForeignDomHighestTPMByCarrier()
  {
    _tvlSegs->push_back(createAirSeg("MOW", "FRA", "AA"));  // Mileage 1262
    _tvlSegs->push_back(createAirSeg("FRA", "SEL", "BA"));  // Mileage 5327
    _tvlSegs->push_back(createAirSeg("SEL", "TYO", "CA"));  // Mileage 749
    CarrierCode highestGC = _ogc->getForeignDomHighestTPMCarrier(*_tvlSegs, FMDirection::OUTBOUND, _primarySector);
    uint32_t tpm = _ogc->getForeignDomHighestTPMByCarrier(CarrierCode("AA"), *_tvlSegs);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1262), tpm);
    uint32_t highestTPM = _ogc->getForeignDomHighestTPMByCarrier(highestGC, *_tvlSegs);
    CPPUNIT_ASSERT_EQUAL(uint32_t(5327), highestTPM);
  }

private:
  void matchAllCriteria(TravelSeg& seg)
  {
    seg.segmentType() = Open;

    Loc* org = _memHandle(new Loc);
    Loc* dest = _memHandle(new Loc);
    org->area() = IATA_AREA1;
    org->subarea() = "640509";
    dest->area() = IATA_AREA2;
    dest->subarea() = "040147";

    seg.origin() = org;
    seg.destination() = dest;
  }

  Loc* createLoc(const IATAAreaCode& area, const IATASubAreaCode& subarea, const NationCode& nation)
  {
    Loc* loc = _memHandle(new Loc);
    loc->area() = area;
    loc->subarea() = subarea;
    loc->nation() = nation;
    return loc;
  }

  void
  initRtwSegments(const AirSegDesc* segDesc, std::size_t segCnt, std::vector<TravelSeg*>& segments)
  {
    for (std::size_t i = 0; i < segCnt; ++i)
    {
      const AirSegDesc& desc = segDesc[i];
      AirSeg* seg = _memHandle(new AirSeg);
      seg->origin() = createLoc(desc.orgArea, desc.orgSubarea, desc.orgNation);
      seg->destination() = createLoc(desc.dstArea, desc.dstSubarea, desc.dstNation);
      seg->carrier() = desc.cxr;
      segments.push_back(seg);
    }
  }

  TravelSeg*
  createAirSeg(const LocCode& origAirport, const LocCode& destAirport, const CarrierCode& cxr)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    DataHandle dataHandle;

    seg->origin() = dataHandle.getLoc(origAirport, DateTime::localTime());
    seg->destination() = dataHandle.getLoc(destAirport, DateTime::localTime());
    seg->carrier() = cxr;
    seg->departureDT() = DateTime::localTime();

    return seg;
  }

  ArunkSeg*
  createArunkSeg(const LocCode& origAirport, const LocCode& destAirport)
  {
    ArunkSeg* seg = _memHandle.create<ArunkSeg>();
    DataHandle dataHandle;

    seg->origin() = dataHandle.getLoc(origAirport, DateTime::localTime());
    seg->destination() = dataHandle.getLoc(destAirport, DateTime::localTime());
    seg->segmentType() = tse::Arunk;

    return seg;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(GoverningCarrierTest);
}
