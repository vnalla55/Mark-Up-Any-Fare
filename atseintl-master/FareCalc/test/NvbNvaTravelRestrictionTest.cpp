//----------------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "DBAccess/TravelRestriction.h"
#include "Rules/TravelRestrictions.h"
#include "Rules/UpdaterObserver.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"

#include "FareCalc/CalcTotals.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestAirSegFactory.h"
#include "DataModel/PricingUnit.h"
#include "test/testdata/TestLocFactory.h"

#include "FareCalc/NvbNvaOrchestrator.h"

#include <vector>

namespace
{
typedef std::map<int16_t, tse::DateTime> DatesMap;
inline std::ostream& operator<<(std::ostream& os, const DatesMap& m)
{
  for (DatesMap::const_iterator it = m.begin(), itEnd = m.end(); it != itEnd; ++it)
    os << it->first << " " << it->second << "\n";

  return os;
}
}

namespace tse
{
class NvbNvaTravelRestrictionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NvbNvaTravelRestrictionTest);
  CPPUNIT_TEST(testNVA1_fu0);
  CPPUNIT_TEST(testNVA1_fu1);
  CPPUNIT_TEST(testNVA2);
  CPPUNIT_TEST(testNVA3);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCompletedBy);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCompletedByEqualToLatestTvlStartDate);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCompletedByGreaterThanLatestTvlStartDate);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCommencedBy);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCommencedByEqualToLatestTvlStartDate);
  CPPUNIT_TEST(testNVAStopDateWithTvlMustBeCommencedByGreaterThanLatestTvlStartDate);
  CPPUNIT_TEST_SUITE_END();

private:
  tse::TestMemHandle _memHandle;
  tse::PricingTrx* _trx;
  tse::PricingRequest* _req;
  tse::PricingOptions* _options;
  tse::Agent* _agent;
  PricingUnit _pu;
  FarePath* _fp0;
  TravelRestriction _tvlRestrictionInfo;
  TravelRestriction _tvlRestrictionInfo2;
  TravelRestriction _tvlRestrictionInfo3;
  TravelRestrictionsObserverWrapper _tvlRestr;

  DatesMap& createDatesMap3(const DateTime& dt1, const DateTime& dt2, const DateTime& dt3)
  {
    DatesMap& m = *(_memHandle.create<DatesMap>());
    m[1] = dt1;
    m[2] = dt2;
    m[3] = dt3;
    return m;
  }
  DatesMap& createDatesMap4(const DateTime& dt1,
                            const DateTime& dt2,
                            const DateTime& dt3,
                            const DateTime& dt4)
  {
    DatesMap& m = *(_memHandle.create<DatesMap>());
    m[1] = dt1;
    m[2] = dt2;
    m[3] = dt3;
    m[4] = dt4;
    return m;
  }

  void createBaseTravelRestriction(tse::TravelRestriction& tvlRestrictionInfo)
  {
    tvlRestrictionInfo.vendor() = "ATP";
    tvlRestrictionInfo.unavailTag() = ' ';
    tvlRestrictionInfo.geoTblItemNo() = 0;
    tvlRestrictionInfo.earliestTvlStartDate() = DateTime::emptyDate();
    tvlRestrictionInfo.latestTvlStartDate() = DateTime::emptyDate();
    tvlRestrictionInfo.returnTvlInd() = ' ';
    tvlRestrictionInfo.stopDate() = DateTime::emptyDate();
    tvlRestrictionInfo.stopTime() = 0;
  }

  Itin* createItinRegressionSPR121133()
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    AirSeg* dfw_sfo = _memHandle.create<AirSeg>();
    dfw_sfo->pnrSegment() = 1;
    dfw_sfo->segmentOrder() = 1;
    dfw_sfo->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    dfw_sfo->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    dfw_sfo->stopOver() = false;
    dfw_sfo->carrier() = "AA";
    dfw_sfo->departureDT() = DateTime(2009, 4, 1, 8, 5, 0);
    dfw_sfo->arrivalDT() = DateTime(2009, 4, 1, 9, 55, 0);

    AirSeg* sfo_jfk = _memHandle.create<AirSeg>();
    sfo_jfk->pnrSegment() = 2;
    sfo_jfk->segmentOrder() = 2;
    sfo_jfk->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    sfo_jfk->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    sfo_jfk->stopOver() = false;
    sfo_jfk->carrier() = "AA";
    sfo_jfk->departureDT() = DateTime(2009, 4, 1, 11, 55, 0);
    sfo_jfk->arrivalDT() = DateTime(2009, 4, 1, 20, 30, 0);

    AirSeg* jfk_sfo = _memHandle.create<AirSeg>();
    jfk_sfo->pnrSegment() = 3;
    jfk_sfo->segmentOrder() = 3;
    jfk_sfo->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    jfk_sfo->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    jfk_sfo->stopOver() = false;
    jfk_sfo->carrier() = "AA";
    jfk_sfo->departureDT() = DateTime(2009, 7, 16, 15, 40, 0);
    jfk_sfo->arrivalDT() = DateTime(2009, 7, 17, 12, 15, 0);

    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 4;
    sfo_dfw->segmentOrder() = 4;
    sfo_dfw->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    sfo_dfw->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    sfo_dfw->stopOver() = false;
    sfo_dfw->carrier() = "AA";
    sfo_dfw->departureDT() = DateTime(2009, 7, 17, 17, 45, 0);
    sfo_dfw->arrivalDT() = DateTime(2009, 7, 17, 20, 55, 0);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(dfw_sfo);
    itin->travelSeg().push_back(sfo_jfk);
    itin->travelSeg().push_back(jfk_sfo);
    itin->travelSeg().push_back(sfo_dfw);

    _trx->travelSeg().push_back(dfw_sfo);
    _trx->travelSeg().push_back(sfo_jfk);
    _trx->travelSeg().push_back(jfk_sfo);
    _trx->travelSeg().push_back(sfo_dfw);

    itin->setTravelDate(dfw_sfo->departureDT());
    // Create the FareUsages
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu->travelSeg().push_back(dfw_sfo);
    pu->travelSeg().push_back(sfo_jfk);
    pu->travelSeg().push_back(jfk_sfo);
    pu->travelSeg().push_back(sfo_dfw);
    pu->turnAroundPoint() = jfk_sfo;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();

    fm1->direction() = FMDirection::OUTBOUND;
    fm1->origin() = dfw_sfo->origin();
    fm1->destination() = sfo_jfk->destination();
    fm1->setGlobalDirection(GlobalDirection::US);
    fm2->direction() = FMDirection::INBOUND;
    fm2->origin() = jfk_sfo->origin();
    fm2->destination() = sfo_dfw->destination();
    fm2->setGlobalDirection(GlobalDirection::US);

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();

    Fare* f1 = _memHandle.create<Fare>();
    Fare* f2 = _memHandle.create<Fare>();

    TariffCrossRefInfo* tcri1 = _memHandle.create<TariffCrossRefInfo>();
    TariffCrossRefInfo* tcri2 = _memHandle.create<TariffCrossRefInfo>();

    FareInfo* fi1 = _memHandle.create<FareInfo>();
    FareInfo* fi2 = _memHandle.create<FareInfo>();

    fi1->_globalDirection = GlobalDirection::ZZ;
    fi1->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fi2->_globalDirection = GlobalDirection::ZZ;
    fi2->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    fi1->_currency = "USD";
    fi2->_currency = "USD";

    f1->initialize(Fare::FS_Domestic, fi1, *fm1, tcri1);
    f2->initialize(Fare::FS_Domestic, fi2, *fm2, tcri2);

    PaxType* pt1 = _memHandle.create<PaxType>();
    PaxType* pt2 = _memHandle.create<PaxType>();

    pt1->paxType() = "ADT";
    pt2->paxType() = "ADT";

    pt1->vendorCode() = "ATP";
    pt2->vendorCode() = "ATP";

    PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti2 = _memHandle.create<PaxTypeInfo>();

    pti1->adultInd() = 'Y';
    pti2->adultInd() = 'Y';

    pt1->paxTypeInfo() = pti1;
    pt2->paxTypeInfo() = pti2;

    ptf1->initialize(f1, pt1, fm1);
    ptf2->initialize(f2, pt2, fm2);

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(dfw_sfo);
    fm1->travelSeg().push_back(sfo_jfk);
    fm2->travelSeg().push_back(jfk_sfo);
    fm2->travelSeg().push_back(sfo_dfw);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = pt1;

    // Attach the itin to the transaction
    //
    _trx->itin().push_back(itin);

    return itin;
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _agent = _memHandle.create<Agent>();
    _req = _memHandle.create<PricingRequest>();
    _req->ticketingDT() = DateTime::localTime();
    _req->ticketingAgent() = _agent;
    _trx->setRequest(_req);
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    // Clear Any cached data in Factories
    TestFactoryManager::instance()->destroyAll();


    AirSeg* airSeg0 = TestAirSegFactory::create("NvbNvaTravelRestrictionTestData/AirSegDFWJFK.xml");
    FareMarket* fm0 = _memHandle.create<FareMarket>();
    fm0->travelSeg().push_back(airSeg0);
    fm0->direction() = FMDirection::OUTBOUND;
    fm0->origin() = airSeg0->origin();
    fm0->destination() = airSeg0->destination();
    PaxTypeFare* paxTypeFare0 = _memHandle.create<PaxTypeFare>();
    paxTypeFare0->fareMarket() = fm0;
    FareUsage* fu0 = _memHandle.create<FareUsage>();
    fu0->paxTypeFare() = paxTypeFare0;
    fu0->inbound() = false;

    AirSeg* airSeg1 = TestAirSegFactory::create("NvbNvaTravelRestrictionTestData/AirSegJFKSFO.xml");
    AirSeg* airSeg2 = TestAirSegFactory::create("NvbNvaTravelRestrictionTestData/AirSegSFODFW.xml");
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg().push_back(airSeg1);
    fm1->travelSeg().push_back(airSeg2);
    fm1->direction() = FMDirection::INBOUND;
    fm1->origin() = airSeg1->origin();
    fm1->destination() = airSeg2->destination();
    PaxTypeFare* paxTypeFare1 = _memHandle.create<PaxTypeFare>();
    paxTypeFare1->fareMarket() = fm1;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->paxTypeFare() = paxTypeFare1;
    fu1->inbound() = true;

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu0);
    pu->fareUsage().push_back(fu1);
    pu->travelSeg().push_back(airSeg0);
    pu->travelSeg().push_back(airSeg1);
    pu->travelSeg().push_back(airSeg2);
    pu->turnAroundPoint() = airSeg1;

    _fp0 = _memHandle.create<FarePath>();
    _fp0->pricingUnit().push_back(pu);

    // Create the itin
    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(airSeg0);
    itin->travelSeg().push_back(airSeg1);
    itin->travelSeg().push_back(airSeg2);
    itin->setTravelDate(airSeg0->departureDT());
    _trx->itin().push_back(itin);
    _fp0->itin() = itin;

    _tvlRestrictionInfo.vendor() = "ATP";
    _tvlRestrictionInfo.unavailTag() = ' ';
    _tvlRestrictionInfo.geoTblItemNo() = 0;
    _tvlRestrictionInfo.earliestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo.latestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMPLETEDBY;
    _tvlRestrictionInfo.stopDate() = airSeg2->arrivalDT() + TimeDuration(240, 0, 0);
    _tvlRestrictionInfo.stopTime() = 0;

    _tvlRestrictionInfo2.vendor() = "ATP";
    _tvlRestrictionInfo2.unavailTag() = ' ';
    _tvlRestrictionInfo2.geoTblItemNo() = 0;
    _tvlRestrictionInfo2.earliestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo2.latestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo2.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMMENCEDBY;
    _tvlRestrictionInfo2.stopDate() = airSeg1->departureDT() + TimeDuration(120, 0, 0);
    _tvlRestrictionInfo2.stopTime() = 0;

    _tvlRestrictionInfo3.vendor() = "ATP";
    _tvlRestrictionInfo3.unavailTag() = ' ';
    _tvlRestrictionInfo3.geoTblItemNo() = 0;
    _tvlRestrictionInfo3.earliestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo3.latestTvlStartDate() = DateTime::emptyDate();
    _tvlRestrictionInfo3.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMMENCEDBY;
    _tvlRestrictionInfo3.stopDate() = airSeg0->departureDT() + TimeDuration(96, 0, 0);
    _tvlRestrictionInfo3.stopTime() = 0;
  }

  void tearDown() { _memHandle.clear(); }

  void testNVA1_fu0()
  {
    const PricingUnit& pu = *(_fp0->pricingUnit().front());
    FareUsage& fu0 = *(pu.fareUsage()[0]);

    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu0);

    //--------------------------------------------------------
    // Process NVA
    // fu0: airseg0 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *_fp0, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap3(_tvlRestrictionInfo.stopDate(),
                                         _trx->itin().front()->travelDate().addYears(1),
                                         _trx->itin().front()->travelDate().addYears(1)));
  }

  void testNVA1_fu1()
  {
    const PricingUnit& pu = *(_fp0->pricingUnit().front());
    FareUsage& fu0 = *(pu.fareUsage()[0]);
    FareUsage& fu1 = *(pu.fareUsage()[1]);

    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu0);
    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu1);

    //--------------------------------------------------------
    // Process NVA
    // fu0: airseg0 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    // fu1: airseg0 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    //              because all NVA of segment in front should apply the restriction of
    //              segment after.
    //      airseg1 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *_fp0, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap3(_tvlRestrictionInfo.stopDate(),
                                         _tvlRestrictionInfo.stopDate(),
                                         _tvlRestrictionInfo.stopDate()));
  }

  void testNVA2()
  {
    const PricingUnit& pu = *(_fp0->pricingUnit().front());
    FareUsage& fu0 = *(pu.fareUsage()[0]);
    FareUsage& fu1 = *(pu.fareUsage()[1]);
    AirSeg& airSeg1 = *(dynamic_cast<AirSeg*>(pu.travelSeg()[1]));
    AirSeg& airSeg2 = *(dynamic_cast<AirSeg*>(pu.travelSeg()[2]));

    airSeg1.departureDT() = DateTime::openDate();
    airSeg1.arrivalDT() = DateTime::openDate();
    airSeg2.departureDT() = DateTime::openDate();
    airSeg2.arrivalDT() = DateTime::openDate();

    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu0);
    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu1);
    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo2, *_fp0, pu, fu1);

    //--------------------------------------------------------
    // Process NVA
    // fu0: airseg0 MUST_BE_COMPLETED_BY _tvlRestrictionInfo2.stopDate()
    //              because all NVA of segment in front should apply the restriction of
    //              segment after. So fu0: airseg0 should not be NVA 08/28/2004 when
    //              fu1: airseg1 NVA 08/23/2004.
    // fu1: airseg0 MUST_BE_COMMENCED_BY _tvlRestrictionInfo2.stopDate()
    //      airseg1 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *_fp0, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap3(_tvlRestrictionInfo2.stopDate(),
                                         _tvlRestrictionInfo2.stopDate(),
                                         _tvlRestrictionInfo.stopDate()));
  }

  void testNVA3()
  {
    const PricingUnit& pu = *(_fp0->pricingUnit().front());
    FareUsage& fu0 = *(pu.fareUsage()[0]);
    FareUsage& fu1 = *(pu.fareUsage()[1]);
    PaxTypeFare& paxTypeFare0 = *((pu.fareUsage()[0])->paxTypeFare());
    const FareMarket& fm0 = *(paxTypeFare0.fareMarket());

    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu0);
    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo, *_fp0, pu, fu1);
    _tvlRestr.validate(*_trx, &_tvlRestrictionInfo2, *_fp0, pu, fu1);

    _tvlRestr.validate(*_trx, *(_trx->itin().front()), paxTypeFare0, &_tvlRestrictionInfo3, fm0);

    //---------------------------------------------------------
    // Process NVA
    // fu0: airseg0 MUST_BE_COMMENCED_BY _tvlRestrictionInfo3.stopDate()
    // fu1: airseg0 MUST_BE_COMMENCED_BY _tvlRestrictionInfo2.stopDate()
    //      airseg1 MUST_BE_COMPLETED_BY _tvlRestrictionInfo.stopDate()
    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *_fp0, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap3(_tvlRestrictionInfo3.stopDate(),
                                         _tvlRestrictionInfo2.stopDate(),
                                         _tvlRestrictionInfo.stopDate()));
  }

  void testNVAStopDateWithTvlMustBeCompletedBy()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMPLETEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap4(tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate()));
  }

  void testNVAStopDateWithTvlMustBeCompletedByEqualToLatestTvlStartDate()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.latestTvlStartDate() = DateTime(2009, 9, 30, 0, 0, 0);
    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMPLETEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap4(tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate()));
  }

  void testNVAStopDateWithTvlMustBeCompletedByGreaterThanLatestTvlStartDate()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.latestTvlStartDate() = DateTime(2009, 7, 30, 0, 0, 0);
    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMPLETEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap4(tvlRestrictionInfo.latestTvlStartDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate()));
  }

  void testNVAStopDateWithTvlMustBeCommencedBy()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMMENCEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap4(tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         itin->travelDate().addYears(1)));
  }

  void testNVAStopDateWithTvlMustBeCommencedByEqualToLatestTvlStartDate()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.latestTvlStartDate() = DateTime(2009, 9, 30, 0, 0, 0);
    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMMENCEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(totals.tvlSegNVA,
                         createDatesMap4(tvlRestrictionInfo.latestTvlStartDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         itin->travelDate().addYears(1)));
  }

  void testNVAStopDateWithTvlMustBeCommencedByGreaterThanLatestTvlStartDate()
  {
    Itin* itin = createItinRegressionSPR121133();

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TravelRestriction tvlRestrictionInfo;
    createBaseTravelRestriction(tvlRestrictionInfo);

    tvlRestrictionInfo.latestTvlStartDate() = DateTime(2009, 7, 30, 0, 0, 0);
    tvlRestrictionInfo.returnTvlInd() = TravelRestrictions::TVLMUSTBECOMMENCEDBY;
    tvlRestrictionInfo.stopDate() = DateTime(2009, 9, 30, 0, 0, 0);

    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu1);
    _tvlRestr.validate(*_trx, &tvlRestrictionInfo, *fp, *pu, *fu2);

    CalcTotals totals;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *fp, totals);

    CPPUNIT_ASSERT_EQUAL(createDatesMap4(tvlRestrictionInfo.latestTvlStartDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         tvlRestrictionInfo.stopDate(),
                                         itin->travelDate().addYears(1)),
                         totals.tvlSegNVA);
  }

}; // class TravelRestrictions

CPPUNIT_TEST_SUITE_REGISTRATION(NvbNvaTravelRestrictionTest);

} // namespace tse
