#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareUsage.h"
#include "ServiceFees/PseudoFarePathBuilder.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class MockPseudoFarePathBuilder : public PseudoFarePathBuilder
{
public:
  MockPseudoFarePathBuilder(PricingTrx& trx, bool setRTJourney)
    : PseudoFarePathBuilder(trx), _setRTJourney(setRTJourney)
  {
  }

protected:
  virtual void setJourneyType() { _isRTJourney = _setRTJourney; }

private:
  bool _setRTJourney;
};

class PseudoFarePathBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PseudoFarePathBuilderTest);
  CPPUNIT_TEST(testBuild_OW_1Seg_noKnownFC);
  CPPUNIT_TEST(testBuild_OW_TwoSeg_noKnownFC);
  CPPUNIT_TEST(testBuild_RT_noKnownFC);
  CPPUNIT_TEST(testBuild_OW_1Seg_KnownFC);
  CPPUNIT_TEST(testBuild_OW_TwoSeg_KnownFC);
  CPPUNIT_TEST(testBuild_RT_KnownFC);
  CPPUNIT_SKIP_TEST(testBuild_RT_SideTrip_KnownFC);
  CPPUNIT_TEST(testBuildPUWithExistingFareMarket);
  CPPUNIT_TEST(testInsertFareUsageTvlSegToItsPU);
  CPPUNIT_TEST(testSideTripPUType_OW_One_FlightSegment_in_SideTrip);
  CPPUNIT_SKIP_TEST(testSideTripPUType_RT_One_SideTrip);
  CPPUNIT_TEST(testSideTripPUType_RT_One_SideTrip_and_OW_2_SideTrip);
  CPPUNIT_TEST(testInsertTourCode);
  CPPUNIT_TEST(testBuild_RT_Mix_KnownFC_and_UnknownFBI);

  CPPUNIT_TEST(testDiag881_Not_Active);
  CPPUNIT_TEST(testDiag881_Is_Active_No_PUs);
  CPPUNIT_TEST(testDiag881_Is_Active_With_PUs_No_FU);
  CPPUNIT_TEST(testDiag881_Is_Active_With_PUs_With_FU);

  CPPUNIT_TEST(testInitAccountCodes_tktDesignators);
  CPPUNIT_TEST(testInitAccountCodes_corpId);
  CPPUNIT_TEST(testInitAccountCodes_invalidCorpId);
  CPPUNIT_TEST(testInitAccountCodes_accountCode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    buildOCFeeTrx();
    TestConfigInitializer::setValue("M70_IGNORE_FBI_CXRS", "VA", "SERVICE_FEES_SVC");
  }

  void tearDown() { _memHandle.clear(); }

protected:
  AncillaryPricingTrx* _trx;
  AncRequest* _request;
  TestMemHandle _memHandle;
  Itin* _itin;
  PricingOptions* _options;
  PaxType _paxType;
  AirSeg* _tvlSegLaxSfo;
  AirSeg* _tvlSegSfoLax;
  AirSeg* _tvlSegLaxDfw;
  AirSeg* _tvlSegDfwLax;
  ArunkSeg* _seg2Arunk;
  Billing* _billing;

  AncRequest::AncFareBreakInfo* fbi_1;
  AncRequest::AncFareBreakInfo* fbi_2;
  AncRequest::AncFareBreakAssociation* fba_1;
  AncRequest::AncFareBreakAssociation* fba_2;
  AncRequest::AncFareBreakAssociation* fba_3;
  AncRequest::AncFareBreakAssociation* fba_4;

  void buildOCFeeTrx()
  {
    _memHandle.get(_trx);
    _memHandle.get(_request);
    _memHandle.get(_itin);
    _memHandle.get(_options);
    _memHandle.get(_billing);
    _trx->billing() = _billing;

    _trx->setRequest(_request);
    _trx->itin().push_back(_itin);
    _trx->setOptions(_options);

    _tvlSegLaxSfo =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_SFO.xml");
    _tvlSegSfoLax =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSFO_LAX.xml");
    _tvlSegLaxDfw =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    _tvlSegDfwLax =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");

    _paxType.paxType() = "ADT";
    _trx->paxType().push_back(&_paxType);

    _memHandle.get(fbi_1);
    fbi_1->fareComponentID() = 1;
    _memHandle.get(fbi_2);
    fbi_2->fareComponentID() = 2;
    _memHandle.get(fba_1);
    fba_1->segmentID() = 1;
    _memHandle.get(fba_2);
    fba_2->segmentID() = 2;
    _memHandle.get(fba_3);
    fba_3->segmentID() = 3;
    _memHandle.get(fba_4);
    fba_4->segmentID() = 4;
  }

  void buildOWJouney_1Seg()
  {
    _itin->travelSeg().push_back(_tvlSegLaxSfo);
    _tvlSegLaxSfo->segmentOrder() = 1;
    FareMarket* fm = 0;
    _memHandle.get(fm);
    fm->travelSeg().push_back(_tvlSegLaxSfo);
    _itin->fareMarket().push_back(fm);
    _itin->geoTravelType() = GeoTravelType::Domestic;
    _itin->validatingCarrier() = _tvlSegLaxSfo->carrier();
  }

  void buildOWJouney_2Seg()
  {
    _itin->travelSeg().push_back(_tvlSegSfoLax);
    _tvlSegSfoLax->segmentOrder() = 1;
    _itin->travelSeg().push_back(_tvlSegLaxDfw);
    _tvlSegLaxDfw->segmentOrder() = 2;
    FareMarket* fm1 = 0;
    FareMarket* fm2 = 0;
    FareMarket* fm3 = 0;
    _memHandle.get(fm1);
    _memHandle.get(fm2);
    _memHandle.get(fm3);
    fm1->travelSeg().push_back(_tvlSegSfoLax);
    fm2->travelSeg().push_back(_tvlSegLaxDfw);
    fm3->travelSeg().push_back(_tvlSegSfoLax);
    fm3->travelSeg().push_back(_tvlSegLaxDfw);
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->geoTravelType() = GeoTravelType::Domestic;
    _itin->validatingCarrier() = _tvlSegLaxSfo->carrier();
  }

  void buildRTJouney_4Seg()
  {
    _itin->travelSeg().push_back(_tvlSegSfoLax);
    _tvlSegSfoLax->segmentOrder() = 1;
    _itin->travelSeg().push_back(_tvlSegLaxDfw);
    _tvlSegLaxDfw->segmentOrder() = 2;
    _itin->travelSeg().push_back(_tvlSegDfwLax);
    _tvlSegDfwLax->segmentOrder() = 3;
    _itin->travelSeg().push_back(_tvlSegLaxSfo);
    _tvlSegLaxSfo->segmentOrder() = 4;
    _itin->furthestPointSegmentOrder() = 2;
    FareMarket* fm1 = 0;
    FareMarket* fm2 = 0;
    FareMarket* fm3 = 0;
    FareMarket* fm4 = 0;
    _memHandle.get(fm1);
    _memHandle.get(fm2);
    _memHandle.get(fm3);
    _memHandle.get(fm4);
    fm1->travelSeg().push_back(_tvlSegSfoLax);
    _itin->fareMarket().push_back(fm1);
    fm2->travelSeg().push_back(_tvlSegSfoLax);
    fm2->travelSeg().push_back(_tvlSegLaxDfw);
    fm3->travelSeg().push_back(_tvlSegDfwLax);
    fm3->travelSeg().push_back(_tvlSegLaxSfo);
    fm4->travelSeg().push_back(_tvlSegLaxSfo);
    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);

    _itin->geoTravelType() = GeoTravelType::Domestic;
    _itin->validatingCarrier() = _tvlSegSfoLax->carrier();
  }

  void buildOW_FC1Seg()
  {
    buildOWJouney_1Seg();
    fba_1->fareComponentID() = 1;
    _request->fareBreakPerItin()[_itin].push_back(fbi_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_1);
  }

  void buildOW_FC2Seg()
  {
    buildOWJouney_2Seg();
    fba_1->fareComponentID() = 1;
    fba_2->fareComponentID() = 1;
    _request->fareBreakPerItin()[_itin].push_back(fbi_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_2);
  }

  void buildRT_2FC()
  {
    buildRTJouney_4Seg();
    fba_1->fareComponentID() = 1;
    fba_2->fareComponentID() = 1;
    fba_3->fareComponentID() = 2;
    fba_4->fareComponentID() = 2;
    _request->fareBreakPerItin()[_itin].push_back(fbi_1);
    _request->fareBreakPerItin()[_itin].push_back(fbi_2);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_2);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_3);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_4);
  }

  void buildRT_2FC_Mix_No_FBI_for_2nd_FC()
  {
    buildRTJouney_4Seg();
    fba_1->fareComponentID() = 1;
    fba_2->fareComponentID() = 1;
    fba_3->fareComponentID() = 2;
    fba_4->fareComponentID() = 2;
    _request->fareBreakPerItin()[_itin].push_back(fbi_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_2);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_3);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_4);
  }

  void addSideTrip()
  {
    AncRequest::AncFareBreakInfo* fbi_ST_1;
    AncRequest::AncFareBreakInfo* fbi_ST_2;
    AncRequest::AncFareBreakAssociation* fba_ST_1;
    AncRequest::AncFareBreakAssociation* fba_ST_2;
    _memHandle.get(fbi_ST_1);
    fbi_ST_1->fareComponentID() = 3;
    _memHandle.get(fbi_ST_2);
    fbi_ST_2->fareComponentID() = 4;

    _memHandle.get(fba_ST_1);
    fba_ST_1->fareComponentID() = 3;
    fba_ST_1->sideTripID() = 1;
    fba_ST_1->segmentID() = 5;
    _memHandle.get(fba_ST_2);
    fba_ST_2->fareComponentID() = 4;
    fba_ST_2->sideTripID() = 1;
    fba_ST_2->segmentID() = 6;

    _request->fareBreakPerItin()[_itin].push_back(fbi_ST_1);
    _request->fareBreakPerItin()[_itin].push_back(fbi_ST_2);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_ST_1);
    _request->fareBreakAssociationPerItin()[_itin].push_back(fba_ST_2);

    AirSeg* airSeg_ST_1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_GDL.xml");
    AirSeg* airSeg_ST_2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegGDL_LAX.xml");
    airSeg_ST_1->segmentOrder() = 5;
    airSeg_ST_2->segmentOrder() = 6;
    _itin->travelSeg().push_back(airSeg_ST_1);
    _itin->travelSeg().push_back(airSeg_ST_2);
    FareMarket* fmST_1, *fmST_2;
    _memHandle.get(fmST_1);
    _memHandle.get(fmST_2);
    fmST_1->travelSeg().push_back(airSeg_ST_1);
    fmST_2->travelSeg().push_back(airSeg_ST_2);
    _itin->fareMarket().push_back(fmST_1);
    _itin->fareMarket().push_back(fmST_2);
  }

  void buildFM_3SegItin_NoArunk()
  {
    FareMarket* fm1 = 0;
    FareMarket* fm2 = 0;
    FareMarket* fm3 = 0;
    FareMarket* fm4 = 0;
    AirSeg* seg1 = 0;
    AirSeg* seg2 = 0;
    AirSeg* seg3 = 0;

    _memHandle.get(_itin);
    _memHandle.get(fm1);
    _memHandle.get(fm2);
    _memHandle.get(fm3);
    _memHandle.get(fm4);
    _memHandle.get(seg1);
    _memHandle.get(seg2);
    _memHandle.get(seg3);

    fm1->travelSeg().push_back(seg1);

    fm2->travelSeg().push_back(seg1);
    fm2->travelSeg().push_back(seg2);

    fm3->travelSeg().push_back(seg2);
    fm4->travelSeg().push_back(seg3);

    _itin->travelSeg().push_back(seg1);
    _itin->travelSeg().push_back(seg2);
    _itin->travelSeg().push_back(seg3);

    _itin->fareMarket().push_back(fm1);
    _itin->fareMarket().push_back(fm2);
    _itin->fareMarket().push_back(fm3);
    _itin->fareMarket().push_back(fm4);
  }

  // TESTS
public:
  void testBuild_OW_1Seg_noKnownFC()
  {
    buildOWJouney_1Seg();
    MockPseudoFarePathBuilder builder(*_trx, false);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
  }

  void testBuild_OW_TwoSeg_noKnownFC()
  {
    buildOWJouney_2Seg();
    MockPseudoFarePathBuilder builder(*_trx, false);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
  }

  void testBuild_RT_noKnownFC()
  {
    buildRTJouney_4Seg();
    MockPseudoFarePathBuilder builder(*_trx, true);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().size() == 1 &&
                   _itin->farePath().front()->pricingUnit().front()->fareUsage().size() == 2);
  }

  void testBuild_OW_1Seg_KnownFC()
  {
    buildOW_FC1Seg();
    _trx->billing()->partitionID() = "VA";
    MockPseudoFarePathBuilder builder(*_trx, false);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
  }

  void testBuild_OW_TwoSeg_KnownFC()
  {
    buildOW_FC2Seg();
    MockPseudoFarePathBuilder builder(*_trx, false);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
  }

  void testBuild_RT_KnownFC()
  {
    buildRT_2FC();
    MockPseudoFarePathBuilder builder(*_trx, true);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().size() == 1 &&
                   _itin->farePath().front()->pricingUnit().front()->fareUsage().size() == 2);
  }

  void testBuild_RT_SideTrip_KnownFC()
  {
    buildRT_2FC();
    addSideTrip();
    MockPseudoFarePathBuilder builder(*_trx, true);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().size() == 2);
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().front()->isSideTripPU() !=
                   _itin->farePath().front()->pricingUnit().back()->isSideTripPU());
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().front()->fareUsage().size() == 2);
  }

  void testBuildPUWithExistingFareMarket()
  {
    buildFM_3SegItin_NoArunk();

    MockPseudoFarePathBuilder builder(*_trx, true);
    PricingUnit pu;
    builder._pricingUnit = &pu;
    builder._currentItin = _itin;

    bool failed = false;
    try { builder.buildPUWithExistingFareMarket(); }
    catch (...) { failed = true; }

    CPPUNIT_ASSERT(!failed);
    CPPUNIT_ASSERT(pu.fareUsage().size() == 3);
  }

  void testInsertFareUsageTvlSegToItsPU()
  {
    FareUsage fu1, fu2, fu3;
    AirSeg ts1, ts2, ts3, ts4;
    fu1.travelSeg().push_back(&ts1);
    fu1.travelSeg().push_back(&ts2);
    fu2.travelSeg().push_back(&ts3);
    fu3.travelSeg().push_back(&ts4);
    PricingUnit pu1, pu2;
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    MockPseudoFarePathBuilder builder(*_trx, true);
    FarePath* fp = 0;
    _memHandle.get(fp);
    fp->pricingUnit().push_back(&pu1);
    fp->pricingUnit().push_back(&pu2);
    builder._farePath = fp;
    builder.insertFareUsageTvlSegToItsPU();
    CPPUNIT_ASSERT(pu1.travelSeg().size() == 3);
    CPPUNIT_ASSERT(pu2.travelSeg().size() == 1);
  }

  void setSideTrip(PricingUnit& sideTripPU, FareUsage& fu, AirSeg* as1, AirSeg* as2)
  {
    sideTripPU.isSideTripPU() = true;
    sideTripPU.sideTripNumber() = 1;
    sideTripPU.travelSeg().push_back(as1);
    fu.travelSeg().push_back(as1);
    if (as2)
    {
      sideTripPU.travelSeg().push_back(as2);
      fu.travelSeg().push_back(as2);
    }
    sideTripPU.fareUsage().push_back(&fu);
    sideTripPU.puType() = PricingUnit::Type::ONEWAY;
  }

  void testSideTripPUType_OW_One_FlightSegment_in_SideTrip()
  {
    MockPseudoFarePathBuilder builder(*_trx, true);

    PricingUnit sideTripPU;
    FareUsage fu;
    setSideTrip(sideTripPU, fu, _tvlSegLaxDfw, 0);
    builder._sideTripPUs.push_back(&sideTripPU);
    builder.sideTripPUType();

    CPPUNIT_ASSERT(sideTripPU.puType() == PricingUnit::Type::ONEWAY);
  }

  void testSideTripPUType_RT_One_SideTrip()
  {
    MockPseudoFarePathBuilder builder(*_trx, true);

    PricingUnit sideTripPU;
    FareUsage fu;
    setSideTrip(sideTripPU, fu, _tvlSegLaxDfw, _tvlSegDfwLax);
    builder._sideTripPUs.push_back(&sideTripPU);

    builder.sideTripPUType();

    CPPUNIT_ASSERT(sideTripPU.puType() == PricingUnit::Type::ROUNDTRIP);
  }

  void testSideTripPUType_RT_One_SideTrip_and_OW_2_SideTrip()
  {
    MockPseudoFarePathBuilder builder(*_trx, true);

    PricingUnit sideTripPU1, sideTripPU2;
    FareUsage fu1, fu2;
    setSideTrip(sideTripPU1, fu1, _tvlSegLaxDfw, _tvlSegDfwLax);
    setSideTrip(sideTripPU2, fu2, _tvlSegLaxSfo, 0);
    builder._sideTripPUs.push_back(&sideTripPU1);
    builder._sideTripPUs.push_back(&sideTripPU2);

    builder.sideTripPUType();

    CPPUNIT_ASSERT(sideTripPU1.puType() == PricingUnit::Type::ROUNDTRIP);
    CPPUNIT_ASSERT(sideTripPU2.puType() == PricingUnit::Type::ONEWAY);
  }

  void testInsertTourCode()
  {
    MockPseudoFarePathBuilder builder(*_trx, true);
    builder._currentItin = _itin;
    _request->tourCodePerItin()[_itin] = "TRAVELOCITY";
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;

    builder.insertTourCode(_itin);

    CPPUNIT_ASSERT(builder._farePath->cat27TourCode() == "TRAVELOCITY");
  }

  void testBuild_RT_Mix_KnownFC_and_UnknownFBI()
  {
    buildRT_2FC_Mix_No_FBI_for_2nd_FC();
    MockPseudoFarePathBuilder builder(*_trx, true);
    CPPUNIT_ASSERT(builder.build(_itin, _itin, _paxType));
    CPPUNIT_ASSERT(_itin->farePath().front()->pricingUnit().size() == 1 &&
                   _itin->farePath().front()->pricingUnit().front()->fareUsage().size() == 2);
  }

  void createDiag(DiagnosticTypes diagType)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _trx->diagnostic().activate();
    }
  }

  void testDiag881_Not_Active()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    builder.diag881(_paxType);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ") == std::string::npos);
  }

  void testDiag881_Is_Active_No_PUs()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;

    builder.diag881(_paxType);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(
                       "**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****") !=
                   std::string::npos);
  }

  void testDiag881_Is_Active_With_PUs_No_FU()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;
    PricingUnit pu;
    fp->pricingUnit().push_back(&pu);
    builder.diag881(_paxType);
    CPPUNIT_ASSERT_EQUAL(std::string("**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****\n"
                                     " **  PU  ** FARE COMPONENT **  TRAVEL SEG  **\n"
                                     "     OW\n"),
                         _trx->diagnostic().toString());
  }

  void testDiag881_Is_Active_With_PUs_With_FU()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    createDiag(Diagnostic881);
    FarePath* fp = 0;
    _memHandle.get(fp);
    builder._farePath = fp;
    PricingUnit pu;
    fp->pricingUnit().push_back(&pu);

    FareUsage fu;
    PaxTypeFare ptf;
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, fm, &tCRInfo);
    ptf.setFare(&fare);
    fu.paxTypeFare() = &ptf;

    _seg2Arunk = _memHandle.create<ArunkSeg>();
    _seg2Arunk->segmentType() = Arunk;
    _seg2Arunk->origAirport() = "LAX";
    _seg2Arunk->destAirport() = "LAX";

    fu.travelSeg().push_back(_tvlSegSfoLax);
    fu.travelSeg().push_back(_seg2Arunk);
    fu.travelSeg().push_back(_tvlSegLaxDfw);

    _tvlSegSfoLax->segmentOrder() = 1;
    _seg2Arunk->segmentOrder() = 2;
    _tvlSegLaxDfw->segmentOrder() = 3;
    _itin->travelSeg().push_back(_tvlSegSfoLax);
    _itin->travelSeg().push_back(_seg2Arunk);
    _itin->travelSeg().push_back(_tvlSegLaxDfw);

    pu.fareUsage().push_back(&fu);

    FareUsage fu1;
    PaxTypeFare ptf1;
    FareInfo fareInfo1;
    Fare fare1;
    TariffCrossRefInfo tCRInfo1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm, &tCRInfo1);
    ptf1.setFare(&fare1);
    fu1.paxTypeFare() = &ptf1;

    fu1.travelSeg().push_back(_tvlSegDfwLax);
    fu1.travelSeg().push_back(_tvlSegLaxSfo);

    _tvlSegDfwLax->segmentOrder() = 4;
    _tvlSegLaxSfo->segmentOrder() = 5;
    _itin->travelSeg().push_back(_tvlSegDfwLax);
    _itin->travelSeg().push_back(_tvlSegLaxSfo);
    pu.fareUsage().push_back(&fu1);

    pu.puType() = PricingUnit::Type::ROUNDTRIP;
    builder._currentItin = _itin;

    builder.diag881(_paxType);
    CPPUNIT_ASSERT_EQUAL(std::string("**** PSEUDO FAREPATH DETAIL FOR PASSENGER = ADT ****\n"
                                     " **  PU  ** FARE COMPONENT **  TRAVEL SEG  **\n"
                                     "     RT\n"
                                     "            1          Y26   1 SFO-LAX\n"
                                     "                             2 LAX*LAX\n"
                                     "                             3 LAX-DFW\n"
                                     "            2                4 DFW-LAX\n"
                                     "                             5 LAX-SFO\n"),
                         _trx->diagnostic().toString());
  }
  void testInitAccountCodes_tktDesignators()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    dynamic_cast<AncRequest*>(_trx->getRequest())->tktDesignatorPerItin()[_itin][0] = "TKT1";
    dynamic_cast<AncRequest*>(_trx->getRequest())->tktDesignatorPerItin()[_itin][1] = "TKT2";
    _trx->getRequest()->tktDesignator()[2] = "TKT3";
    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->tktDesignator().size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("TKT1"), _trx->getRequest()->tktDesignator()[0]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("TKT2"), _trx->getRequest()->tktDesignator()[1]);
    CPPUNIT_ASSERT(!_trx->getRequest()->isMultiAccCorpId());
  }
  void testInitAccountCodes_corpId()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    dynamic_cast<AncRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("AC01");
    dynamic_cast<AncRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("AC02");
    _trx->getRequest()->corpIdVec().push_back("AC03");
    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->corpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("AC01"), _trx->getRequest()->corpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("AC02"), _trx->getRequest()->corpIdVec()[1]);
    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }
  void testInitAccountCodes_invalidCorpId()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    dynamic_cast<AncRequest*>(_trx->getRequest())->invalidCorpIdPerItin()[_itin].push_back("AC01");
    dynamic_cast<AncRequest*>(_trx->getRequest())->invalidCorpIdPerItin()[_itin].push_back("AC02");
    _trx->getRequest()->incorrectCorpIdVec().push_back("AC03");
    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->incorrectCorpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("AC01"), _trx->getRequest()->incorrectCorpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("AC02"), _trx->getRequest()->incorrectCorpIdVec()[1]);
    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }
  void testInitAccountCodes_accountCode()
  {
    MockPseudoFarePathBuilder builder(*_trx, false);
    dynamic_cast<AncRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("CORP1");
    dynamic_cast<AncRequest*>(_trx->getRequest())->corpIdPerItin()[_itin].push_back("CORP2");
    _trx->getRequest()->corpIdVec().push_back("CORP3");
    CPPUNIT_ASSERT_NO_THROW(builder.initAccountCodes(_itin));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _trx->getRequest()->corpIdVec().size());
    CPPUNIT_ASSERT_EQUAL(std::string("CORP1"), _trx->getRequest()->corpIdVec()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("CORP2"), _trx->getRequest()->corpIdVec()[1]);
    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PseudoFarePathBuilderTest);
}
