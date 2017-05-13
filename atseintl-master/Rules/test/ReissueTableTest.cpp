#include <boost/assign/std/vector.hpp>

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DST.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/ReissueSequence.h"
#include "Diagnostic/DiagCollector.h"
#include "ItinAnalyzer/ExcItinUtil.h"
#include "Rules/ReissueTable.h"
#include "Rules/RuleUtil.h"
#include "Rules/test/TestCommon.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"


namespace tse
{

using boost::assign::operator+=;

class ReissueTableTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissueTableTest);

  CPPUNIT_TEST(testFltNoFailCxr);
  CPPUNIT_TEST(testFltNoFailNo);
  CPPUNIT_TEST(testFltNoMatch);
  CPPUNIT_TEST(testFltNoBlank);
  CPPUNIT_TEST(testFltNoFail);

  CPPUNIT_TEST(testMatchFromToGeoAndPortionBlankNoRestrictions);
  CPPUNIT_TEST(testMatchWhenBothTSIGeoAreSpecified);
  CPPUNIT_TEST(testNoMatchWhenBothGeoTSIDoesNotSpecifyDepartureArrival);
  CPPUNIT_TEST(testMatchFromGeoFirstInternationalEuropeWhenPortionIsBlank);
  CPPUNIT_TEST(testNoMatchFromGeoFirstInternationalEuropeWhenPortionIsBlank);
  CPPUNIT_TEST(testMatchFromGeoFirstInternationalWhenPortionIsBlank);
  CPPUNIT_TEST(testNoMatchFromGeoFirstInternationalWhenPortionIsBlank);
  CPPUNIT_TEST(testMatchFromGeoInternationalWhenPortionIsBlank);
  CPPUNIT_TEST(testNoMatchFromGeoInternationalWhenPortionIsBlank);
  CPPUNIT_TEST(testMatchFromGeoInternationalEuropeWhenPortionIsBlank);
  CPPUNIT_TEST(testNoMatchFromGeoInternationalEuropeWhenPortionIsBlank);

  CPPUNIT_SKIP_TEST(testMatchFromToGeoBlankPortionFirstFlightCoupon);

  CPPUNIT_TEST(testNoMatchFromToGeoBlankPortionFirstFlightCoupon);
  CPPUNIT_TEST(testMatchFromToGeoBlankPortionFirstFlightComponent);
  CPPUNIT_TEST(testNoMatchFromToGeoBlankPortionFirstFlightComponent);
  CPPUNIT_TEST(testMatchFromGeoWhenPortionIsFirstFlightCoupon);
  CPPUNIT_TEST(testNoMatchFromGeoWhenPortionIsFirstFlightCoupon);
  CPPUNIT_TEST(testSecondNoMatchFromGeoWhenPortionIsFirstFlightCoupon);
  CPPUNIT_TEST(testMatchFromGeoWhenPortionIsFirstFlightComponent);
  CPPUNIT_TEST(testNoMatchFromGeoWhenPortionIsFirstFlightComponent);
  CPPUNIT_TEST(testMatchBothGeosAreTSIs);
  CPPUNIT_TEST(testNoMatchBothGeosAreTSIs);
  CPPUNIT_TEST(testMatchBothGeosAreLocales);
  CPPUNIT_TEST(testNoMatchBothGeosAreLocales);
  CPPUNIT_TEST(testMatchFromGeoTSIToGeoLocale);
  CPPUNIT_TEST(testNoMatchFromGeoTSIToGeoLocale);
  CPPUNIT_TEST(testMatchFromGeoLocaleToGeoTSIAndLocale);
  CPPUNIT_TEST(testNoMatchFromGeoLocaleToGeoTSIAndLocale);

  CPPUNIT_TEST(testMatchCancelAndStartOverTag1);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag2);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag3);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag4);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag5);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag6);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag7);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag8);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag9);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag10);
  CPPUNIT_TEST(testMatchCancelAndStartOverTag11);

  CPPUNIT_TEST(testMatchCarrierRestrictions_EmptyFarePath);
  CPPUNIT_TEST(testMatchCarrierRestrictions_CrrRestIndX);
  CPPUNIT_TEST(testMatchCarrierRestrictions_CrrRestIndY);

  CPPUNIT_TEST(testMatchTag7DefinitionTagIsNot7);
  CPPUNIT_TEST(testMatchTag7DefinitionFirstSegmentUnflownAndAllSegmentUnchangedOrInventoryChanged);
  CPPUNIT_TEST(testMatchTag7DefinitionFirstSegmentFlownAndAllSegmentUnchangedOrInventoryChanged);
  CPPUNIT_TEST(testMatchTag7DefinitionFirstSegmentUnflownAndSomeSegmentChanged);
  CPPUNIT_TEST(testMatchTag7DefinitionFirstSegmentUnflownAndSomeSegmentsConfirmed);

  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnBlank);
  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnTravelAgency);
  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnHomeTravelAgency);
  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnIATA);
  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnHomeIATA);
  CPPUNIT_TEST(testMatchAgencyRestrictionFailT);
  CPPUNIT_TEST(testMatchAgencyRestrictionFailU);
  CPPUNIT_TEST(testMatchAgencyRestrictionFailI);
  CPPUNIT_TEST(testMatchAgencyRestrictionFailH);
  CPPUNIT_TEST(testMatchAgencyRestrictionPassOnCarrier);

  CPPUNIT_TEST_SUITE_END();

protected:
  RexPricingTrx* _trx;
  Itin* _newItin;
  ExcItin* _excItin;
  ReissueTableOverride* _t988;
  ReissueSequence _r3;
  FareMarket _fareMarket;
  std::vector<TravelSeg*>* _fCSegs;
  std::vector<TravelSeg*>* _newItinSegs;
  std::vector<TravelSeg*>* _excItinSegs;
  PricingUnit* _pu;
  std::vector<TravelSeg*>* _puFirstFCTvlSegs;
  TestMemHandle _memHandle;

public:
  static const int SEG_1 = 0x01;
  static const int SEG_2 = 0x02;
  static const int SEG_3 = 0x04;
  static const int SEG_4 = 0x08;
  static const int SEG_5 = 0x10;
  static const int SEG_6 = 0x20;
  static const int FARE_COMPONENT = 0x100;
  static const int SUB_JOURNEY = 0x200;
  static const int JOURNEY = 0x400;
  static const int TSI = 0x800;
  static const int CHECK_ORIG = 0x1000;
  static const int CHECK_DEST = 0x2000;
  static const int CHECK_ORIG_DEST = 0x4000;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _memHandle.create<RootLoggerGetOff>();
    _memHandle.create<ConsoleLogger>("atseintl.Assert");

    _trx = _memHandle.create<RexPricingTrx>();

    _trx->diagnostic().diagnosticType() = Diagnostic331;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "988"));

    _trx->setRequest(_memHandle.create<RexPricingRequest>());
    Loc* loc = _memHandle.create<Loc>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = loc;
    _trx->getRequest()->ticketingAgent() = agent;

    _trx->currentTicketingDT() = DateTime(time(0));

    _newItin = _memHandle.create<Itin>();
    _excItin = _memHandle.create<ExcItin>();
    _excItin->setItinIndex(0);
    _excItin->fareMarket().push_back(&_fareMarket);
    _trx->exchangeItin().push_back(_excItin);
    _trx->itin().push_back(_newItin);

    _pu = _memHandle.create<PricingUnit>();
    _pu->fareUsage().push_back(_memHandle.create<FareUsage>());
    _puFirstFCTvlSegs = &(_pu->fareUsage().front()->travelSeg());

    _t988 = _memHandle(new ReissueTableOverride(*_trx, _excItin, _pu));
    _r3.itemNo() = 7777;
    _r3.seqNo() = 1234;
    _fCSegs = &(_fareMarket.travelSeg());
    _newItinSegs = &(_newItin->travelSeg());
    _excItinSegs = &(_excItin->travelSeg());
  }

  void tearDown() { _memHandle.clear(); }

  std::string getDiagString() { return _t988->_dc->str(); }

  FareMarket* FM(TravelSeg* p1, TravelSeg* p2 = NULL) { return TestCommon::FM(p1, p2); }

  FareUsage* FU(TravelSeg* p1, TravelSeg* p2 = NULL) { return TestCommon::FU(p1, p2); }

  PricingUnit* PU(FareUsage* p1, FareUsage* p2 = NULL) { return TestCommon::PU(p1, p2); }

  FarePath* FP(PricingUnit* p1, PricingUnit* p2 = NULL) { return TestCommon::FP(p1, p2); }

  void addSegment(std::vector<TravelSeg*>* tvlSeg,
                  const LocCode& board,
                  const LocCode& off,
                  bool stopOver = false,
                  int flightNo = 123,
                  const CarrierCode& cxr = "AA",
                  const LocCode& boardMCity = "",
                  const LocCode& offMCity = "",
                  const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED,
                  bool unflown = true,
                  const std::string& depDateTime = "2007-07-07 07:07",
                  const TravelSegType segType = Air)
  {
    TestCommon::addSegment(tvlSeg,
                           board,
                           off,
                           stopOver,
                           flightNo,
                           cxr,
                           boardMCity,
                           offMCity,
                           changeStatus,
                           unflown,
                           depDateTime,
                           segType);
  }

  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  enum SegType
  {
    STOP = 0,
    CONX
  };

  TravelSeg* createSeg(const LocCode& board, const LocCode& off, SegType type)
  {
    AirSeg* seg = create<AirSeg>();
    seg->boardMultiCity() = board;
    seg->offMultiCity() = off;
    seg->stopOver() = type == STOP;
    return seg;
  }

  void addFareComp(Itin& itin, FareMarket& fm)
  {
    itin.fareMarket() += &fm;
    itin.travelSeg().insert(itin.travelSeg().end(), fm.travelSeg().begin(), fm.travelSeg().end());
  }

  void testFltNoFailCxr()
  {
    _r3.flightNoInd() = 'X';
    addSegment(_newItinSegs, "DFW", "LAX", false, 333, "AA");
    addSegment(_fCSegs, "DFW", "LAX", false, 333, "UA");

    CPPUNIT_ASSERT(!_t988->matchFlightNo(_fareMarket, _r3));
  }

  void testFltNoFailNo()
  {
    _r3.flightNoInd() = 'X';
    addSegment(_newItinSegs, "DFW", "LAX", false, 333, "AA");
    addSegment(_fCSegs, "DFW", "LAX", false, 555, "AA");

    CPPUNIT_ASSERT(!_t988->matchFlightNo(_fareMarket, _r3));
  }

  void testFltNoMatch()
  {
    _r3.flightNoInd() = 'X';
    addSegment(_newItinSegs, "WAW", "KTW", false, 100, "LO");
    addSegment(_newItinSegs, "KTW", "FRA", false, 111, "LH");
    addSegment(_newItinSegs, "FRA", "DFW", false, 222, "LH");
    addSegment(_newItinSegs, "DFW", "LAX", false, 333, "AA");

    addSegment(_fCSegs, "KTW", "FRA", false, 111, "LH");
    addSegment(_fCSegs, "FRA", "DFW", false, 222, "LH");
    CPPUNIT_ASSERT(_t988->matchFlightNo(_fareMarket, _r3));
  }

  void testFltNoBlank()
  {
    _r3.flightNoInd() = ReissueTable::NOT_APPLY;
    addSegment(_newItinSegs, "DFW", "LAX", false, 333, "AA");
    addSegment(_fCSegs, "DFW", "LAX", false, 555, "AA");

    CPPUNIT_ASSERT(_t988->matchFlightNo(_fareMarket, _r3));
  }

  void testFltNoFail()
  {
    _r3.flightNoInd() = 'X';
    addSegment(_newItinSegs, "WAW", "KTW", false, 100, "LO");
    addSegment(_newItinSegs, "KTW", "FRA", false, 111, "LH");
    addSegment(_newItinSegs, "FRA", "DFW", false, 222, "LH");
    addSegment(_newItinSegs, "DFW", "LAX", false, 333, "AA");

    addSegment(_fCSegs, "KTW", "FRA", false, 111, "LH");
    addSegment(_fCSegs, "FRA", "DFW", false, 223, "LH");
    CPPUNIT_ASSERT(!_t988->matchFlightNo(_fareMarket, _r3));
  }

  void testMatchFromToGeoAndPortionBlankNoRestrictions()
  {
    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() = 0;
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testMatchWhenBothTSIGeoAreSpecified()
  {
    int itemNo = 1;
    VendorCode vendor = "ATP";
    bool checkOrigFrom = true;
    bool checkDestFrom = false;
    bool checkOrigTo = false;
    bool checkDestTo = true;
    CPPUNIT_ASSERT(_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));
  }

  void testNoMatchWhenBothGeoTSIDoesNotSpecifyDepartureArrival()
  {
    int itemNo = 1;
    VendorCode vendor = "ATP";
    bool checkOrigFrom = false;
    bool checkDestFrom = true;
    bool checkOrigTo = true;
    bool checkDestTo = false;
    CPPUNIT_ASSERT(!_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));

    checkOrigFrom = true;
    checkDestFrom = false;
    checkOrigTo = true;
    checkDestTo = true;
    CPPUNIT_ASSERT(!_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));

    checkOrigFrom = true;
    checkDestFrom = false;
    checkOrigTo = true;
    checkDestTo = false;
    CPPUNIT_ASSERT(!_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));

    checkOrigFrom = true;
    checkDestFrom = true;
    checkOrigTo = false;
    checkDestTo = true;
    CPPUNIT_ASSERT(!_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));

    checkOrigFrom = false;
    checkDestFrom = true;
    checkOrigTo = false;
    checkDestTo = true;
    CPPUNIT_ASSERT(!_t988->checkPortionBadData(
        itemNo, vendor, checkOrigFrom, checkDestFrom, checkOrigTo, checkDestTo));
  }

  void testMatchFromGeoFirstInternationalEuropeWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG | SEG_3; // TSI 48 - (PU) Departure of First International
                                                // Sector PLUS(and) Z 210 (zone europe): LON-PAR
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoFirstInternationalEuropeWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG | SEG_3; // TSI 48 - (PU) Departure of First International
                                                // Sector PLUS(and) Z 210 (zone europe): LON-PAR
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoFirstInternationalWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG | SEG_2 |
        SEG_3; // TSI 48 - (PU) Departure of First International Sector: NYC-LON, LON-PAR
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoFirstInternationalWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG | SEG_2 |
        SEG_3; // TSI 48 - (PU) Departure of First International Sector: NYC-LON, LON-PAR
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoInternationalWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG_DEST | SEG_2 | SEG_3 | SEG_4 |
        SEG_5; // TSI 18 - (PU) All International Sectors: NYC-LON, LON-PAR, PAR-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoInternationalWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG_DEST | SEG_2 | SEG_3 | SEG_4 |
        SEG_5; // TSI 18 - (PU) All International Sectors: NYC-LON, LON-PAR, PAR-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoInternationalEuropeWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() = TSI | SUB_JOURNEY | CHECK_ORIG_DEST | SEG_3 |
                                SEG_4; // TSI 18 - (PU) All International Sectors PLUS(and) Z 210
                                       // (zone europe): LON-PAR, PAR-LON
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoInternationalEuropeWhenPortionIsBlank()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() = TSI | SUB_JOURNEY | CHECK_ORIG_DEST | SEG_3 |
                                SEG_4; // TSI 18 - (PU) All International Sectors PLUS(and) Z 210
                                       // (zone europe): LON-PAR, PAR-LON
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromToGeoBlankPortionFirstFlightCoupon()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COUPON;
    _r3.tvlGeoTblItemNoFrom() = 0;
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromToGeoBlankPortionFirstFlightCoupon()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COUPON;
    _r3.tvlGeoTblItemNoFrom() = 0;
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromToGeoBlankPortionFirstFlightComponent()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COMPONENT;
    _r3.tvlGeoTblItemNoFrom() = 0;
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromToGeoBlankPortionFirstFlightComponent()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COMPONENT;
    _r3.tvlGeoTblItemNoFrom() = 0;
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoWhenPortionIsFirstFlightCoupon()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0]), FU((*_excItinSegs)[3])),
                                      PU(FU((*_excItinSegs)[1]), FU((*_excItinSegs)[2]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COUPON;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG |
        SEG_1; // TSI 40 - Departure of Outbound Transatlantic Sector: DFW-LON
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoWhenPortionIsFirstFlightCoupon()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COUPON;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG |
        SEG_2; // TSI 40 - Departure of Outbound Transatlantic Sector: NYC-LON
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testSecondNoMatchFromGeoWhenPortionIsFirstFlightCoupon()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(PU(FU((*_excItinSegs)[0]), FU((*_excItinSegs)[3])),
                                      PU(FU((*_excItinSegs)[1]), FU((*_excItinSegs)[2]))));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COUPON;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG |
        SEG_1; // TSI 40 - (PU) Departure of Outbound Transatlantic Sector: DFW-LON
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoWhenPortionIsFirstFlightComponent()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));
    _excItin->fareMarket().clear();
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[0], (*_excItinSegs)[1]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[2]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[3]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[4], (*_excItinSegs)[5]));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COMPONENT;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | FARE_COMPONENT | CHECK_ORIG | SEG_2 |
        SEG_5; // TSI 42 - (FC) Departure of Each Transatlantic Sector: NYC-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoWhenPortionIsFirstFlightComponent()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));
    _excItin->fareMarket().clear();
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[0], (*_excItinSegs)[1]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[2]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[3]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[4], (*_excItinSegs)[5]));

    _r3.portionInd() = ReissueTable::FIRST_FLIGHT_COMPONENT;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | FARE_COMPONENT | CHECK_ORIG | SEG_2 |
        SEG_5; // TSI 42 - (FC) Departure of Each Transatlantic Sector: NYC-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() = 0;
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchBothGeosAreTSIs()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->fareMarket().clear();
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[0], (*_excItinSegs)[1]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[2]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[3]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[4], (*_excItinSegs)[5]));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | FARE_COMPONENT | CHECK_ORIG | SEG_2 |
        SEG_5; //(first) TSI 42 - (FC) Departure of Each Transatlantic Sector: NYC-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() =
        TSI | FARE_COMPONENT | CHECK_DEST | SEG_2 |
        SEG_5; //(last) TSI 44 - (FC) Arrivial of Each Transatlantic Sector: NYC-LON, LON-NYC
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchBothGeosAreTSIs()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->fareMarket().clear();
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[0], (*_excItinSegs)[1]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[2]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[3]));
    _excItin->fareMarket().push_back(FM((*_excItinSegs)[4], (*_excItinSegs)[5]));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | FARE_COMPONENT | CHECK_ORIG | SEG_2 |
        SEG_5; //(first) TSI 42 - (FC) Departure of Each Transatlantic Sector: NYC-LON, LON-NYC
    _r3.tvlGeoTblItemNoTo() =
        TSI | FARE_COMPONENT | CHECK_DEST | SEG_2 |
        SEG_5; //(last) TSI 44 - (FC) Arrivial of Each Transatlantic Sector: NYC-LON, LON-NYC
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchBothGeosAreLocales()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        JOURNEY | CHECK_ORIG | SEG_5; //(last) N GB - last out of Great Britain
    _r3.tvlGeoTblItemNoTo() =
        JOURNEY | CHECK_DEST | SEG_5; //(first) Z 00000 - first in North America
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchBothGeosAreLocales()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        JOURNEY | CHECK_ORIG | SEG_5; //(last) N GB - last out of Great Britain
    _r3.tvlGeoTblItemNoTo() =
        JOURNEY | CHECK_DEST | SEG_5; //(first) Z 00000 - first in North America
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoTSIToGeoLocale()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::CHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG |
        SEG_2; //(first) TSI 40 - (PU) Departure of Outbound Transatlantic Sector: NYC-LON
    _r3.tvlGeoTblItemNoTo() = SUB_JOURNEY | CHECK_DEST | SEG_5; //(first) Z 00000 - North America
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoTSIToGeoLocale()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() =
        TSI | SUB_JOURNEY | CHECK_ORIG |
        SEG_2; //(first) TSI 40 - (PU) Departure of Outbound Transatlantic Sector: NYC-LON
    _r3.tvlGeoTblItemNoTo() = SUB_JOURNEY | CHECK_DEST | SEG_5; //(first) Z 00000 - North America
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchFromGeoLocaleToGeoTSIAndLocale()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() = JOURNEY | CHECK_ORIG | SEG_2 | SEG_3; //(last) some nation
    _r3.tvlGeoTblItemNoTo() =
        TSI | JOURNEY | CHECK_DEST | SEG_5 | SEG_6; //(first) some TSI (Journey) + some zone
    CPPUNIT_ASSERT(_t988->matchPortion(_r3));
  }

  void testNoMatchFromGeoLocaleToGeoTSIAndLocale()
  {
    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::CHANGED);
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC", TravelSeg::UNCHANGED);
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW", TravelSeg::UNCHANGED);
    _excItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1]), FU((*_excItinSegs)[4], (*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[2]), FU((*_excItinSegs)[3]))));

    _r3.portionInd() = ReissueTable::NOT_APPLY;
    _r3.tvlGeoTblItemNoFrom() = JOURNEY | CHECK_ORIG | SEG_2 | SEG_3; //(last) some nation
    _r3.tvlGeoTblItemNoTo() =
        TSI | JOURNEY | CHECK_DEST | SEG_5 | SEG_6; //(first) some TSI (Journey) + some zone
    CPPUNIT_ASSERT(!_t988->matchPortion(_r3));
  }

  void testMatchCancelAndStartOverTag1()
  {
    _r3.processingInd() = KEEP_THE_FARES;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag2()
  {
    _r3.processingInd() = GUARANTEED_AIR_FARE;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag3()
  {
    _r3.processingInd() = KEEP_FARES_FOR_TRAVELED_FC;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag4()
  {
    _r3.processingInd() = KEEP_FARES_FOR_UNCHANGED_FC;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag5()
  {
    _r3.processingInd() = NO_GUARANTEED_FARES;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag6()
  {
    _r3.processingInd() = TRAVEL_COMENCEMENT_AIR_FARES;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag7()
  {
    _r3.processingInd() = REISSUE_DOWN_TO_LOWER_FARE;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCancelAndStartOverTag8()
  {
    _r3.processingInd() = CANCEL_AND_START_OVER;
    CPPUNIT_ASSERT(!_t988->matchCancelAndStartOver(_r3));
  }
  void testMatchCancelAndStartOverTag9()
  {
    _r3.processingInd() = HISTORICAL_FARES_FOR_TRAVELED_FC;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }
  void testMatchCancelAndStartOverTag10()
  {
    _r3.processingInd() = KEEP_FOR_UNCH_CURRENT_FOR_CHNG;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }
  void testMatchCancelAndStartOverTag11()
  {
    _r3.processingInd() = KEEP_UP_TO_FIRST_CHNG_THEN_HIST;
    CPPUNIT_ASSERT(_t988->matchCancelAndStartOver(_r3));
  }

  void testMatchCarrierRestrictions_EmptyFarePath()
  {
    _r3.carrierRestInd() = 'Y';
    std::vector<FarePath*> farePaths = _t988->_itin->farePath();
    farePaths.clear();

    RexPricingRequest* rexPricingRequest = dynamic_cast<RexPricingRequest*>(_trx->getRequest());
    rexPricingRequest->excValidatingCarrier() = "LO";
    rexPricingRequest->validatingCarrier() = "HA";

    _fareMarket.governingCarrier() = "AA";

    CPPUNIT_ASSERT(!_t988->matchCarrierRestrictions(_fareMarket, _r3));
  }

  void testMatchCarrierRestrictions_CrrRestIndX()
  {
    _r3.carrierRestInd() = 'X';

    RexPricingRequest* rexPricingRequest = dynamic_cast<RexPricingRequest*>(_trx->getRequest());
    rexPricingRequest->excValidatingCarrier() = "LO";
    rexPricingRequest->validatingCarrier() = "LH";

    //   addSegment(_newItinSegs, "CHI", "LAX");
    //   addSegment(_fCSegs, "CHI", "LAX");

    _fareMarket.governingCarrier() = "AA";

    CPPUNIT_ASSERT(!_t988->matchCarrierRestrictions(_fareMarket, _r3));
  }

  void testMatchCarrierRestrictions_CrrRestIndY()
  {
    _r3.carrierRestInd() = 'Y';

    _r3.risRestCxrTblItemNo() = 619;

    RexPricingRequest* rexPricingRequest = dynamic_cast<RexPricingRequest*>(_trx->getRequest());
    rexPricingRequest->excValidatingCarrier() = "LO";
    rexPricingRequest->validatingCarrier() = "LH";

    //   addSegment(_newItinSegs, "NYC", "CHI");

    //   addSegment(_fCSegs, "NYC", "CHI");

    _fareMarket.governingCarrier() = "AA";

    CPPUNIT_ASSERT(!_t988->matchCarrierRestrictions(_fareMarket, _r3));
  }

  void testMatchTag7DefinitionTagIsNot7()
  {
    _r3.processingInd() = KEEP_THE_FARES;
    CPPUNIT_ASSERT(_t988->matchTag7Definition(_r3));
  }

  void testMatchTag7DefinitionFirstSegmentUnflownAndAllSegmentUnchangedOrInventoryChanged()
  {
    _r3.processingInd() = REISSUE_DOWN_TO_LOWER_FARE;

    _excItin->someSegmentsChanged() = false;
    _excItin->someSegmentsConfirmed() = false;

    addSegment(
        _excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED, true);
    CPPUNIT_ASSERT(_t988->matchTag7Definition(_r3));
  }

  void testMatchTag7DefinitionFirstSegmentFlownAndAllSegmentUnchangedOrInventoryChanged()
  {
    _r3.processingInd() = REISSUE_DOWN_TO_LOWER_FARE;

    _excItin->someSegmentsChanged() = false;
    _excItin->someSegmentsConfirmed() = false;

    addSegment(
        _excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED, false);
    CPPUNIT_ASSERT(!_t988->matchTag7Definition(_r3));
  }

  void testMatchTag7DefinitionFirstSegmentUnflownAndSomeSegmentChanged()
  {
    _r3.processingInd() = REISSUE_DOWN_TO_LOWER_FARE;

    _excItin->someSegmentsChanged() = true;
    _excItin->someSegmentsConfirmed() = false;

    addSegment(
        _excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED, true);
    CPPUNIT_ASSERT(!_t988->matchTag7Definition(_r3));
  }

  void testMatchTag7DefinitionFirstSegmentUnflownAndSomeSegmentsConfirmed()
  {
    _r3.processingInd() = REISSUE_DOWN_TO_LOWER_FARE;

    _excItin->someSegmentsChanged() = false;
    _excItin->someSegmentsConfirmed() = true;

    addSegment(
        _excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON", TravelSeg::UNCHANGED, true);
    CPPUNIT_ASSERT(!_t988->matchTag7Definition(_r3));
  }

  void testMatchAgencyRestrictionPassOnBlank()
  {
    _r3.agencyLocRest() = ' ';
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }

  void setUpAgencyRestriction(Indicator r3AgencyLocRest, const std::string& iataAgencyNo)
  {
    _r3.agencyLocRest() = r3AgencyLocRest;
    _r3.iataAgencyNo() = iataAgencyNo;

    RexPricingRequest* rexPricingRequest = static_cast<RexPricingRequest*>(_trx->getRequest());
    rexPricingRequest->ticketingAgent() = _memHandle.create<Agent>();
    rexPricingRequest->ticketingAgent()->agentCity() = "W0H3";
    rexPricingRequest->ticketingAgent()->mainTvlAgencyPCC() = "W0H3";
    rexPricingRequest->ticketingAgent()->tvlAgencyPCC() = "W0H3";
    rexPricingRequest->ticketingAgent()->tvlAgencyIATA() = "W0H3";
    rexPricingRequest->ticketingAgent()->homeAgencyIATA() = "W0H3";
  }

  void testMatchAgencyRestrictionPassOnTravelAgency()
  {
    setUpAgencyRestriction('T', "W0H3");
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionPassOnHomeTravelAgency()
  {
    setUpAgencyRestriction('U', "W0H3");
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionPassOnIATA()
  {
    setUpAgencyRestriction('I', "W0H3");
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionPassOnHomeIATA()
  {
    setUpAgencyRestriction('H', "W0H3");
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionFailT()
  {
    setUpAgencyRestriction('T', "S990");
    CPPUNIT_ASSERT(!_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionFailU()
  {
    setUpAgencyRestriction('U', "S990");
    CPPUNIT_ASSERT(!_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionFailI()
  {
    setUpAgencyRestriction('I', "S990");
    CPPUNIT_ASSERT(!_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionFailH()
  {
    setUpAgencyRestriction('H', "S990");
    CPPUNIT_ASSERT(!_t988->matchAgencyRestrictions(_r3));
  }

  void testMatchAgencyRestrictionPassOnCarrier()
  {
    setUpAgencyRestriction('H', "S990");
    RexPricingRequest* rexPricingRequest = static_cast<RexPricingRequest*>(_trx->getRequest());
    rexPricingRequest->ticketingAgent()->tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(_t988->matchAgencyRestrictions(_r3));
  }
};

std::ostream& operator<<(std::ostream& os, const TravelSeg* seg)
{
  return os << seg->boardMultiCity() << "-" << seg->offMultiCity() << " ";
}

CPPUNIT_TEST_SUITE_REGISTRATION(ReissueTableTest);

} // tse
