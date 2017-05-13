#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include <iostream>
#include <vector>

#include "FareCalc/NvbNvaOrchestrator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/EndOnEnd.h"
#include "Rules/RuleConst.h"
#include "Common/VecMap.h"

namespace
{
typedef VecMap<int16_t, tse::DateTime> DatesVecMap;

inline std::ostream& operator<<(std::ostream& os, const DatesVecMap& m)
{
  for (DatesVecMap::const_iterator it = m.begin(), itEnd = m.end(); it != itEnd; ++it)
    os << it->first << " " << it->second << "\n";

  return os;
}
}

namespace tse
{

class NvbNvaRuleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NvbNvaRuleTest);
  CPPUNIT_TEST(testNvabApplyFUNoChangeWhenThereIsAnyNotConfirmed);
  CPPUNIT_TEST(testNvabApplyFUNoChangeWhenAllTravelSegAreConfirmed);
  CPPUNIT_TEST(testNvabApplyFUNoChangeWhenOnlyFirstTravelSegIsConfirmed);
  CPPUNIT_TEST(testProcessNVANVBDateWhenAllTravelSegHavePenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenOnlyOutboundHasPenalty);
  CPPUNIT_TEST(testNvabApplyFUNoChangeWhenNoPNRWithTravelDates);
  CPPUNIT_TEST(testNvabApplyFUNoChangeWhenNoPNRWithNoTravelDates);
  CPPUNIT_TEST(testProcessNVANVBDateWhenOnlyInboundHasPenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenNoPNRWithTravelDatesAndOutboundHasPenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenNoPNRWithNoTravelDatesAndOutboundHasPenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenNoPNRWithTravelDatesAndAllTravelSegHavePenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenNoPNRWithNoTravelDatesAndAllTravelSegHavePenalty);
  CPPUNIT_TEST(testProcessNVANVBDateWhenOutboundHasPenaltyAndThereIsCat6);
  CPPUNIT_TEST(testProcessNVANVBDateWhenOutboundHasPenaltyAndThereIsCat7);
  CPPUNIT_TEST(testProcessNVANVBDateWhenInboundHasPenaltyAndThereIsCat7);
  CPPUNIT_TEST(testProcessNVANVBDateWhenAllTravelSegHavePenaltyAndCat6Cat7);
  CPPUNIT_TEST(testProcessNVANVBDateWhenNoPNRWithNoDateAndAllTravelSegHavePenaltyAndCat6Cat7);
  CPPUNIT_TEST(testApplyNVANVBEndOnEndRestrWhenFareAlreadyHasPenalty);
  CPPUNIT_TEST(testApplyNVANVBEndOnEndRestrWhenFareHasNoEOERequired);
  CPPUNIT_TEST(testApplyNVANVBEndOnEndRestrWhenFareIsPrivate);
  CPPUNIT_TEST(testGetEndOnEndCombinationsFUReturnFalseWhenUnavailTagIsY);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _trx = _memHandle.create<PricingTrx>(); }

  void tearDown() { _memHandle.clear(); }

  Itin* createTestItin()
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
    // dfw_sfo->stopOver()     = false;
    // dfw_sfo->carrier()      = "AA";
    dfw_sfo->departureDT() = DateTime(2010, 4, 1, 8, 5, 0);
    dfw_sfo->arrivalDT() = DateTime(2010, 4, 1, 9, 55, 0);
    dfw_sfo->resStatus() = CONFIRM_RES_STATUS;

    AirSeg* sfo_jfk = _memHandle.create<AirSeg>();
    sfo_jfk->pnrSegment() = 2;
    sfo_jfk->segmentOrder() = 2;
    sfo_jfk->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    sfo_jfk->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    // sfo_jfk->stopOver()     = false;
    // sfo_jfk->carrier()      = "AA";
    sfo_jfk->departureDT() = DateTime(2010, 4, 1, 11, 55, 0);
    sfo_jfk->arrivalDT() = DateTime(2010, 4, 1, 20, 30, 0);
    sfo_jfk->resStatus() = CONFIRM_RES_STATUS;

    AirSeg* jfk_sfo = _memHandle.create<AirSeg>();
    jfk_sfo->pnrSegment() = 3;
    jfk_sfo->segmentOrder() = 3;
    jfk_sfo->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    jfk_sfo->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    // jfk_sfo->stopOver()     = false;
    // jfk_sfo->carrier()      = "AA";
    jfk_sfo->departureDT() = DateTime(2010, 7, 16, 15, 40, 0);
    jfk_sfo->arrivalDT() = DateTime(2010, 7, 17, 12, 15, 0);
    jfk_sfo->resStatus() = CONFIRM_RES_STATUS;

    AirSeg* sfo_dfw = _memHandle.create<AirSeg>();
    sfo_dfw->pnrSegment() = 4;
    sfo_dfw->segmentOrder() = 4;
    sfo_dfw->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    sfo_dfw->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    // sfo_dfw->stopOver()     = false;
    // sfo_dfw->carrier()      = "AA";
    sfo_dfw->departureDT() = DateTime(2010, 7, 17, 17, 45, 0);
    sfo_dfw->arrivalDT() = DateTime(2010, 7, 17, 20, 55, 0);
    sfo_dfw->resStatus() = CONFIRM_RES_STATUS;

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
    pu->tvlSegNVA()[1] = DateTime::openDate();
    pu->tvlSegNVB()[1] = DateTime::openDate();
    pu->tvlSegNVA()[2] = DateTime::openDate();
    pu->tvlSegNVB()[2] = DateTime::openDate();

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

    // Attach the travel segs to the FareUsages
    //
    fu1->travelSeg().push_back(dfw_sfo);
    fu1->travelSeg().push_back(sfo_jfk);
    fu2->travelSeg().push_back(jfk_sfo);
    fu2->travelSeg().push_back(sfo_dfw);

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

    // Attach the travel segs to the itin
    //
    itin->travelSeg().push_back(dfw_sfo);
    itin->travelSeg().push_back(sfo_jfk);
    itin->travelSeg().push_back(jfk_sfo);
    itin->travelSeg().push_back(sfo_dfw);
    itin->setTravelDate(dfw_sfo->departureDT());

    fp->paxType() = pt1;

    // Attach the itin to the transaction
    //
    _trx->itin().push_back(itin);

    return itin;
  }

  void setMinStayDate(PricingUnit* pu)
  {
    DateTime minStayDate1 = (pu->fareUsage()[0]->travelSeg()[0]->departureDT()).addDays(3);
    pu->fareUsage()[0]->minStayDate() = minStayDate1;
    pu->fareUsage()[0]->startNVBTravelSeg() = 3;
    DateTime minStayDate2 = (pu->fareUsage()[1]->travelSeg()[0]->departureDT()).addDays(3);
    pu->fareUsage()[1]->minStayDate() = minStayDate2;
    pu->fareUsage()[1]->startNVBTravelSeg() = 3;
  }

  void setMaxStayDate(PricingUnit* pu)
  {
    DateTime maxStayDate1 = (pu->fareUsage()[0]->travelSeg()[0]->departureDT()).addDays(30);
    pu->fareUsage()[0]->maxStayDate() = maxStayDate1;
    DateTime maxStayDate2 = (pu->fareUsage()[1]->travelSeg()[0]->departureDT()).addDays(30);
    pu->fareUsage()[1]->maxStayDate() = maxStayDate2;
  }

  void testNvabApplyFUNoChangeWhenThereIsAnyNotConfirmed()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    fu1->travelSeg()[0]->resStatus() = QF_RES_STATUS;
    bool isThereAnyNotConfirmed = false;
    NvbNvaOrchestrator::nvabApplyFUNoChange(*_trx, *pu, isThereAnyNotConfirmed, *itin, *fu1);
    CPPUNIT_ASSERT(isThereAnyNotConfirmed);
  }

  void testNvabApplyFUNoChangeWhenAllTravelSegAreConfirmed()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    bool isThereAnyNotConfirmed = false;
    NvbNvaOrchestrator::nvabApplyFUNoChange(*_trx, *pu, isThereAnyNotConfirmed, *itin, *fu1);

    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVA(),
        createDatesMap2(fu1->travelSeg()[0]->departureDT(), fu1->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVB(),
        createDatesMap2(fu1->travelSeg()[0]->departureDT(), fu1->travelSeg()[1]->departureDT()));
  }

  void testNvabApplyFUNoChangeWhenOnlyFirstTravelSegIsConfirmed()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    fu1->travelSeg()[1]->resStatus() = QF_RES_STATUS;
    bool isThereAnyNotConfirmed = false;
    NvbNvaOrchestrator::nvabApplyFUNoChange(*_trx, *pu, isThereAnyNotConfirmed, *itin, *fu1);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap2(fu1->travelSeg()[0]->departureDT(), DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap2(fu1->travelSeg()[0]->departureDT(), DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenAllTravelSegHavePenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    pu->fareUsage()[1]->changePenaltyApply() = true;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
  }

  void testProcessNVANVBDateWhenOnlyOutboundHasPenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
  }

  void testNvabApplyFUNoChangeWhenNoPNRWithTravelDates()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    fu1->travelSeg()[1]->resStatus() = QF_RES_STATUS;
    bool isThereAnyNotConfirmed = false;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::FullDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::nvabApplyFUNoChange(*noPNRTrx, *pu, isThereAnyNotConfirmed, *itin, *fu1);

    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVA(),
        createDatesMap2(fu1->travelSeg()[0]->departureDT(), fu1->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVB(),
        createDatesMap2(fu1->travelSeg()[0]->departureDT(), fu1->travelSeg()[1]->departureDT()));
  }

  void testNvabApplyFUNoChangeWhenNoPNRWithNoTravelDates()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    fu1->travelSeg()[1]->resStatus() = QF_RES_STATUS;
    bool isThereAnyNotConfirmed = false;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::NoDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::nvabApplyFUNoChange(*noPNRTrx, *pu, isThereAnyNotConfirmed, *itin, *fu1);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap2(fu1->travelSeg()[0]->departureDT(), DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap2(fu1->travelSeg()[0]->departureDT(), DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenOnlyInboundHasPenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[1]->changePenaltyApply() = true;
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(DateTime::openDate(),
                                         DateTime::openDate(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(DateTime::openDate(),
                                         DateTime::openDate(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
  }

  void testProcessNVANVBDateWhenNoPNRWithTravelDatesAndOutboundHasPenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::FullDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::processNVANVBDate(*noPNRTrx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenNoPNRWithNoTravelDatesAndOutboundHasPenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::NoDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::processNVANVBDate(*noPNRTrx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenNoPNRWithTravelDatesAndAllTravelSegHavePenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    pu->fareUsage()[1]->changePenaltyApply() = true;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::FullDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::processNVANVBDate(*noPNRTrx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
  }

  void testProcessNVANVBDateWhenNoPNRWithNoTravelDatesAndAllTravelSegHavePenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    pu->fareUsage()[1]->changePenaltyApply() = true;
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::NoDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::processNVANVBDate(*noPNRTrx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenOutboundHasPenaltyAndThereIsCat6()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    setMinStayDate(pu);
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVB(),
        createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                        pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                        pu->fareUsage()[1]->travelSeg()[0]->departureDT().addDays(3),
                        pu->fareUsage()[1]->travelSeg()[0]->departureDT().addDays(3)));
  }

  void testProcessNVANVBDateWhenOutboundHasPenaltyAndThereIsCat7()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    setMaxStayDate(pu);
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[0]->maxStayDate(),
                                         pu->fareUsage()[0]->maxStayDate()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         DateTime::openDate(),
                                         DateTime::openDate()));
  }

  void testProcessNVANVBDateWhenInboundHasPenaltyAndThereIsCat7()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[1]->changePenaltyApply() = true;
    setMaxStayDate(pu);
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->maxStayDate(),
                                         pu->fareUsage()[0]->maxStayDate(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(DateTime::openDate(),
                                         DateTime::openDate(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
  }

  void testProcessNVANVBDateWhenAllTravelSegHavePenaltyAndCat6Cat7()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    pu->fareUsage()[1]->changePenaltyApply() = true;
    setMinStayDate(pu);
    setMaxStayDate(pu);
    NvbNvaOrchestrator::processNVANVBDate(*_trx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVB(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->travelSeg()[1]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[1]->travelSeg()[1]->departureDT()));
  }

  void testProcessNVANVBDateWhenNoPNRWithNoDateAndAllTravelSegHavePenaltyAndCat6Cat7()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    pu->fareUsage()[0]->changePenaltyApply() = true;
    pu->fareUsage()[1]->changePenaltyApply() = true;
    setMinStayDate(pu);
    setMaxStayDate(pu);
    NoPNRPricingTrx* noPNRTrx = new NoPNRPricingTrx();
    itin->dateType() = Itin::NoDate;
    noPNRTrx->itin().push_back(itin);
    NvbNvaOrchestrator::processNVANVBDate(*noPNRTrx, *pu, *itin);

    CPPUNIT_ASSERT_EQUAL(pu->tvlSegNVA(),
                         createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                                         pu->fareUsage()[0]->maxStayDate(),
                                         pu->fareUsage()[0]->maxStayDate(),
                                         pu->fareUsage()[0]->maxStayDate()));
    CPPUNIT_ASSERT_EQUAL(
        pu->tvlSegNVB(),
        createDatesMap4(pu->fareUsage()[0]->travelSeg()[0]->departureDT(),
                        DateTime::openDate(),
                        pu->fareUsage()[1]->travelSeg()[0]->departureDT().addDays(3),
                        pu->fareUsage()[1]->travelSeg()[0]->departureDT().addDays(3)));
  }

  void testApplyNVANVBEndOnEndRestrWhenFareAlreadyHasPenalty()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    fu1->changePenaltyApply() = true;
    fu2->changePenaltyApply() = true;
    std::vector<FareUsage*> allFUVec;
    allFUVec.push_back(fu1);
    allFUVec.push_back(fu2);
    NvbNvaOrchestrator::applyNVANVBEndOnEndRestr(*_trx, *itin, allFUVec);
    CPPUNIT_ASSERT(fu1->changePenaltyApply());
    CPPUNIT_ASSERT(fu2->changePenaltyApply());
  }

  void testApplyNVANVBEndOnEndRestrWhenFareHasNoEOERequired()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    fu1->endOnEndRequired() = false;
    fu2->endOnEndRequired() = false;
    std::vector<FareUsage*> allFUVec;
    allFUVec.push_back(fu1);
    allFUVec.push_back(fu2);
    NvbNvaOrchestrator::applyNVANVBEndOnEndRestr(*_trx, *itin, allFUVec);
    CPPUNIT_ASSERT(!fu1->changePenaltyApply());
    CPPUNIT_ASSERT(!fu2->changePenaltyApply());
  }

  void testApplyNVANVBEndOnEndRestrWhenFareIsPrivate()
  {
    Itin* itin = createTestItin();
    PricingUnit* pu = itin->farePath()[0]->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    TariffCrossRefInfo privTcr;
    privTcr.tariffCat() = RuleConst::PRIVATE_TARIFF;
    fu1->paxTypeFare()->fare()->setTariffCrossRefInfo(&privTcr);
    fu1->changePenaltyApply() = false;
    fu2->changePenaltyApply() = true;
    std::vector<FareUsage*> allFUVec;
    allFUVec.push_back(fu1);
    allFUVec.push_back(fu2);
    NvbNvaOrchestrator::applyNVANVBEndOnEndRestr(*_trx, *itin, allFUVec);
    CPPUNIT_ASSERT(!fu1->changePenaltyApply());
    CPPUNIT_ASSERT(fu2->changePenaltyApply());
  }

  void testGetEndOnEndCombinationsFUReturnFalseWhenUnavailTagIsY()
  {
    EndOnEnd pEndOnEnd;
    NvbNvaOrchestrator::EOEAllSegmentIndicator allSegmentIndicator;
    std::vector<FareUsage*> allFUVec;
    std::vector<FareUsage*>::iterator fuIter = allFUVec.begin();
    std::vector<const FareUsage*> eoeFUVec;
    pEndOnEnd.unavailTag() = 'Y';
    CPPUNIT_ASSERT(!NvbNvaOrchestrator::getEndOnEndCombinationsFU(
                       pEndOnEnd, allSegmentIndicator, fuIter, allFUVec, eoeFUVec));
  }

protected:
  PricingTrx* _trx;
  TestMemHandle _memHandle;

  DatesVecMap& createDatesMap2(const DateTime& dt1, const DateTime& dt2)
  {
    DatesVecMap& m = *(_memHandle.create<DatesVecMap>());
    m[1] = dt1;
    m[2] = dt2;
    return m;
  }

  DatesVecMap& createDatesMap4(const DateTime& dt1,
                               const DateTime& dt2,
                               const DateTime& dt3,
                               const DateTime& dt4)
  {
    DatesVecMap& m = *(_memHandle.create<DatesVecMap>());
    m[1] = dt1;
    m[2] = dt2;
    m[3] = dt3;
    m[4] = dt4;
    return m;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(NvbNvaRuleTest);

} // namespace tse
