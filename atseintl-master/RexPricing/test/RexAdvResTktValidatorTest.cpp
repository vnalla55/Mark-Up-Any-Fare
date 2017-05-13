//-------------------------------------------------------------------
//
//  File:        RexAdvResTktValidatorTest.cpp
//  Created:     February 07, 2008
//  Authors:     Artur Krezel
//
//  Updates:
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
#include "RexPricing/RexAdvResTktValidator.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag689Collector.h"
#include "DataModel/ProcessTagInfo.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/ReissueSequence.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
using namespace boost::assign;

namespace tse
{

const LocCode LOC_DFW = "DFW";
const LocCode LOC_CHI = "CHI";
const LocCode LOC_PAR = "PAR";
const LocCode LOC_LON = "LON";
const LocCode LOC_MIA = "MIA";
const CarrierCode CARRIER_AA = "AA";
const CarrierCode CARRIER_UA = "UA";

class RexAdvResTktValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexAdvResTktValidatorTest);

  CPPUNIT_TEST(testIsOutboundChanged);
  CPPUNIT_TEST(testWhollyUnchangedPU);
  CPPUNIT_TEST(testPartiallyChangedPU);
  CPPUNIT_TEST(testTotallyChangedPU);
  CPPUNIT_TEST(testUnchangedFCExistInCache);
  CPPUNIT_TEST(testUnchangedFCNotExistInCache);
  CPPUNIT_TEST(testFCHasTheSameFareBreaksAndCarrier);
  CPPUNIT_TEST(testFCHasDifferentFareBreaksOrCarrier);
  CPPUNIT_TEST(testAtLeastOneFCHasChangeToFareBreaksOrCarrier);
  CPPUNIT_TEST(testTicketResvIndIsTheSameOnJourney);
  CPPUNIT_TEST(testTicketResvIndIsNotTheSameOnJourney);
  CPPUNIT_TEST(testSimultaneousCheckNeeded);
  CPPUNIT_TEST(testSimultaneousCheckNotNeeded);
  CPPUNIT_TEST(testSimultaneousCheckForDomesticItinAndChangedSegmentsPass);
  CPPUNIT_TEST(testSimultaneousCheckForDomesticItinAndChangedSegmentsFail);
  CPPUNIT_TEST(testSimultaneousCheckForInternationalItinAndChangedSegmentsPass);
  CPPUNIT_TEST(testSimultaneousCheckForInternationalItinAndChangedSegmentsFail);
  CPPUNIT_TEST(testPriorOfDepartureCheckForChangedSegmentsPass);
  CPPUNIT_TEST(testPriorOfDepartureCheckForChangedSegmentsFail);

  CPPUNIT_TEST(testIsUnflownTrueWhenJourney);
  CPPUNIT_TEST(testIsUnflownFalseWhenJourneyAndPartiallyFlown);
  CPPUNIT_TEST(testIsUnflownFalseWhenJourneyAndFullyFlown);
  CPPUNIT_TEST(testIsUnflownTrueWhenPricingUnit);
  CPPUNIT_TEST(testIsUnflownFalseWhenPricingUnitAndPartiallyFlown);
  CPPUNIT_TEST(testIsUnflownFalseWhenPricingUnitAndFullyFlown);
  CPPUNIT_TEST(testIsUnflownTrueWhenFareComponent);
  CPPUNIT_TEST(testIsUnflownFalseWhenFareComponentAndPartiallyFlown);
  CPPUNIT_TEST(testIsUnflownFalseWhenFareComponentAndFullyFlown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateFalseWhenAdvResOutboundAndFareUsageIsInbound);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateFalseWhenAdvResOutboundAndFareUsageIsOutboundAndFlown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateTrueWhenAdvResOutboundAndFareUsageIsOutboundAndUnflown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateFalseWhenAdvResTypeOfFareAndFareNotCurrent);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateTrueWhenAdvResTypeOfFareAndFareCurrentAndUnflown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateFalseWhenAdvResTypeOfFareAndFareCurrentAndFlown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateFalseWhenAdvResNewTktDateAndFlown);
  CPPUNIT_TEST(testIsMostRestrictiveFromDateTrueWhenAdvResNewTktDateAndUnflown);

  CPPUNIT_TEST(testInitializeEmptyBytes93to106);
  CPPUNIT_TEST(testInitializeFalseEmptyBytes93to106);

  CPPUNIT_TEST(testFindMostRestrictiveFromDateForPUFalse);
  CPPUNIT_TEST(testFindMostRestrictiveFromDateForPUFalseNoFU);

  CPPUNIT_TEST(testIsJourneyScopeTicketResvIndTheSameOnJourney);
  CPPUNIT_TEST(testIsJourneyScopeAllFCsFBAndCarrierUnchanged);
  CPPUNIT_TEST(testIsJourneyScopeFalse);
  CPPUNIT_TEST(testIsJourneyScopeFalseEmptyBytes93to106);

  CPPUNIT_TEST(testEmptyBytes93to106);
  CPPUNIT_TEST(testEmptyBytes93to106NoOrig);
  CPPUNIT_TEST(testEmptyBytes93to106FalseReissuePeriod);
  CPPUNIT_TEST(testEmptyBytes93to106FalseReissueUnit);
  CPPUNIT_TEST(testEmptyBytes93to106FalseDeparture);
  CPPUNIT_TEST(testEmptyBytes93to106FalseDepartureUnit);
  CPPUNIT_TEST(testToDateNeverEmpty);

  CPPUNIT_TEST_SUITE_END();

private:
  RexPricingTrx* _trx;
  Itin* _newItin;
  FarePath* _excFarePath;
  FarePath* _newFarePath;
  Diag689Collector* _dc;
  RexAdvResTktValidator* _rexAdvResTktValidator;
  ProcessTagPermutation* _permutation;
  map<tse::AdvResOverride, bool, tse::AdvResOverride> _cache;
  TestMemHandle _memHandle;
  AirSeg* _airSeg1;
  AirSeg* _airSeg2;
  FareUsage* _fu;
  ProcessTagInfo* _pti;
  vector<const PaxTypeFare*>* _allRepricePTFs;
  GenericRexMapper* _genericRexMapper;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<RexPricingTrx>();
    _newItin = _memHandle.create<Itin>();
    _excFarePath = _memHandle.create<FarePath>();
    _trx->diagnostic().diagnosticType() = Diagnostic689;
    _trx->diagnostic().activate();
    _dc = static_cast<Diag689Collector*>(DCFactory::instance()->create(*_trx));
    if (_dc)
      _dc->enable(Diagnostic689);
    _newFarePath = _memHandle.create<FarePath>();
    _newItin->farePath().push_back(_newFarePath);
    _memHandle.get(_allRepricePTFs);

    ExcItin* excItin = 0;
    _memHandle.get(excItin);
    _trx->exchangeItin().push_back(excItin);

    _genericRexMapper = _memHandle.insert(new GenericRexMapper(*_trx, _allRepricePTFs));

    _rexAdvResTktValidator = _memHandle.insert(new RexAdvResTktValidator(
        *_trx, *_newItin, *_newFarePath, *_excFarePath, _cache, _dc, *_genericRexMapper));
    _permutation = _memHandle.create<ProcessTagPermutation>();
    _rexAdvResTktValidator->_permutation = _permutation;
  }

  void tearDown() { _memHandle.clear(); }

protected:
  // helper functions
  PaxTypeFare* PTF(FareMarket* fm)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->fareMarket() = fm;

    return ptf;
  }

  FareMarket* FM(TravelSeg* ts1, TravelSeg* ts2 = NULL, TravelSeg* ts3 = NULL)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    if (ts1)
      fm->travelSeg().push_back(ts1);
    if (ts2)
      fm->travelSeg().push_back(ts2);
    if (ts3)
      fm->travelSeg().push_back(ts3);
    fm->setFCChangeStatus(-1);

    return fm;
  }

  FareCompInfo* FC(FareMarket* fm, uint16_t fareCompNumber = 0)
  {
    FareCompInfo* fc = _memHandle.create<FareCompInfo>();
    fc->fareCompNumber() = fareCompNumber;
    fc->fareMarket() = fm;

    return fc;
  }

  ProcessTagInfo* PTI(FareCompInfo* fc)
  {
    ProcessTagInfo* _pti = _memHandle.create<ProcessTagInfo>();
    _pti->fareCompInfo() = fc;
    ReissueSequence* rs = _memHandle.create<ReissueSequence>();
    rs->fromAdvResInd() = RexAdvResTktValidator::FROM_ADVRES_TYPEOFFARE;
    rs->toAdvResInd() = RexAdvResTktValidator::TO_ADVRES_FARECOMPONENT;
    _pti->reissueSequence()->orig() = rs;

    return _pti;
  }
  FareUsage* FU(TravelSeg* ts1, TravelSeg* ts2 = NULL, TravelSeg* ts3 = NULL)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    if (ts1)
      fu->travelSeg().push_back(ts1);
    if (ts2)
      fu->travelSeg().push_back(ts2);
    if (ts3)
      fu->travelSeg().push_back(ts3);

    return fu;
  }

  FareUsage*
  FU(PaxTypeFare* ptf1, TravelSeg* ts2 = NULL, TravelSeg* ts3 = NULL, TravelSeg* ts4 = NULL)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = ptf1;
    if (ts2)
      fu->travelSeg().push_back(ts2);
    if (ts3)
      fu->travelSeg().push_back(ts3);
    if (ts4)
      fu->travelSeg().push_back(ts4);

    return fu;
  }

  PricingUnit* PU(FareUsage* fu1, FareUsage* fu2 = NULL)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    if (fu1)
      pu->fareUsage().push_back(fu1);
    if (fu2)
      pu->fareUsage().push_back(fu2);

    return pu;
  }

  void initializeNewFarePath(PricingUnit* pu1,
                             PricingUnit* pu2 = NULL,
                             PricingUnit* pu3 = NULL,
                             PricingUnit* pu4 = NULL)
  {
    if (pu1)
      _newFarePath->pricingUnit().push_back(pu1);
    if (pu2)
      _newFarePath->pricingUnit().push_back(pu2);
    if (pu3)
      _newFarePath->pricingUnit().push_back(pu3);
    if (pu4)
      _newFarePath->pricingUnit().push_back(pu4);
  }

  void createAirSegmentsWithStatusUnchanged()
  {
    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg2 = _memHandle.create<AirSeg>();
    _airSeg1->changeStatus() = TravelSeg::UNCHANGED;
    _airSeg2->changeStatus() = TravelSeg::UNCHANGED;
  }

  void createAirSegments(bool firstUnflown, bool secondUnflown = true)
  {
    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg2 = _memHandle.create<AirSeg>();
    _airSeg1->unflown() = firstUnflown;
    _airSeg2->unflown() = secondUnflown;
  }

  void createPtiWithReissueSequence(
      const Indicator fromAdvResInd,
      const Indicator toAdvResInd = RexAdvResTktValidator::TO_ADVRES_FARECOMPONENT)
  {
    _pti = _memHandle.create<ProcessTagInfo>();
    ReissueSequence* rs = _memHandle.create<ReissueSequence>();
    rs->fromAdvResInd() = fromAdvResInd;
    rs->toAdvResInd() = toAdvResInd;
    _pti->reissueSequence()->orig() = rs;
  }

  void createFareUsage(PricingUnit& pu)
  {
    createAirSegmentsWithStatusUnchanged();
    _fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += +_airSeg1, _airSeg2;
    ptf->fareMarket() = fm;
    _fu->paxTypeFare() = ptf;
    pu.fareUsage().push_back(_fu);
  }

  FareMarket* createFareMarket(const LocCode& boardMultiCity,
                               const LocCode& offMultiCity,
                               const CarrierCode& governingCarrier)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->boardMultiCity() = boardMultiCity;
    fm->offMultiCity() = offMultiCity;
    fm->governingCarrier() = governingCarrier;
    return fm;
  }

  void setFromAdvRes(const Indicator fromAdvRes)
  {
    const_cast<ReissueSequence*>(_pti->reissueSequence()->orig())->fromAdvResInd() =
        RexAdvResTktValidator::FROM_ADVRES_OUTBOUND;
  }

  void addAirSegmentsToItin(bool firstUnflown, bool secondUnflown = true)
  {
    createAirSegments(firstUnflown, secondUnflown);
    _newItin->travelSeg() += _airSeg1, _airSeg2;
    _trx->itin() += _newItin;
  }

  // tests
  void testIsOutboundChanged()
  {
    PricingUnit pu;
    createFareUsage(pu);
    _fu->inbound() = false;
    pu.fareUsage().push_back(_fu);

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isOutboundChanged(pu));

    _airSeg1->changeStatus() = TravelSeg::UNCHANGED;
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isOutboundChanged(pu));

    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->changeStatus() = TravelSeg::UNCHANGED;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isOutboundChanged(pu));
  }

  void testWhollyUnchangedPU()
  {
    createAirSegmentsWithStatusUnchanged();

    PricingUnit* pu = PU(FU(PTF(FM(_airSeg1, _airSeg2))));

    CPPUNIT_ASSERT(_rexAdvResTktValidator->isPUWhollyUnchanged(*pu));
  }

  void testPartiallyChangedPU()
  {
    createAirSegmentsWithStatusUnchanged();

    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->changeStatus() = TravelSeg::UNCHANGED;

    PricingUnit* pu = PU(FU(PTF(FM(_airSeg1, _airSeg2))));

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isPUWhollyUnchanged(*pu));
  }

  void testTotallyChangedPU()
  {
    createAirSegmentsWithStatusUnchanged();

    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->changeStatus() = TravelSeg::CHANGED;

    PricingUnit* pu = PU(FU(PTF(FM(_airSeg1, _airSeg2))));

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isPUWhollyUnchanged(*pu));
  }

  void testUnchangedFCExistInCache()
  {
    FareMarket repriceFM;
    vector<const ProcessTagInfo*> newPTIs;
    ProcessTagInfo pti1;
    ProcessTagInfo pti2;
    newPTIs.push_back(&pti1);
    newPTIs.push_back(&pti2);
    _rexAdvResTktValidator->_unchangedFareBreaksAndCarrierCache.insert(
        RexAdvResTktValidator::FareMarket2ProcessTagInfos::value_type(&repriceFM, newPTIs));

    const vector<const ProcessTagInfo*>& foundPTIs =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(&repriceFM);
    CPPUNIT_ASSERT_EQUAL(size_t(2), foundPTIs.size());
  }

  void testUnchangedFCNotExistInCache()
  {
    FareMarket repriceFM;
    vector<const ProcessTagInfo*> newPTIs;
    ProcessTagInfo pti1;
    ProcessTagInfo pti2;
    newPTIs.push_back(&pti1);
    newPTIs.push_back(&pti2);

    const vector<const ProcessTagInfo*>& foundPTIs =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(&repriceFM);
    CPPUNIT_ASSERT_EQUAL(size_t(0), foundPTIs.size());
  }

  void testFCHasTheSameFareBreaksAndCarrier()
  {
    FareMarket* repriceFM = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);

    ProcessTagInfo pti1;
    _rexAdvResTktValidator->_allPTIs.push_back(&pti1);
    FareCompInfo fc1;
    pti1.fareCompInfo() = &fc1;
    FareMarket* fm1 = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);
    fc1.fareMarket() = fm1;

    const vector<const ProcessTagInfo*>& foundPTI1s =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(repriceFM);
    CPPUNIT_ASSERT_EQUAL(size_t(1), foundPTI1s.size());

    _rexAdvResTktValidator->_unchangedFareBreaksAndCarrierCache.clear();
    ProcessTagInfo pti2;
    _rexAdvResTktValidator->_allPTIs.push_back(&pti2);
    FareCompInfo fc2;
    pti2.fareCompInfo() = &fc2;
    FareMarket* fm2 = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);
    fc2.fareMarket() = fm2;

    const vector<const ProcessTagInfo*>& foundPTI2s =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(repriceFM);
    CPPUNIT_ASSERT_EQUAL(size_t(2), foundPTI2s.size());
  }

  void testFCHasDifferentFareBreaksOrCarrier()
  {
    FareMarket* repriceFM = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);

    ProcessTagInfo pti1;
    _rexAdvResTktValidator->_allPTIs.push_back(&pti1);
    FareCompInfo fc1;
    pti1.fareCompInfo() = &fc1;
    FareMarket* fm1 = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_UA);
    fc1.fareMarket() = fm1;

    const vector<const ProcessTagInfo*>& foundPTI1s =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(repriceFM);
    CPPUNIT_ASSERT(foundPTI1s.empty());

    fm1->boardMultiCity() = LOC_MIA;
    fm1->governingCarrier() = CARRIER_AA;

    const vector<const ProcessTagInfo*>& foundPTI2s =
        _rexAdvResTktValidator->findFCsWithNoChangesToFBAndCarrier(repriceFM);
    CPPUNIT_ASSERT(foundPTI2s.empty());
  }

  void testAtLeastOneFCHasChangeToFareBreaksOrCarrier()
  {
    FareMarket* repriceFM_DFWCHI = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);
    FareMarket* repriceFM_CHILON = createFareMarket(LOC_CHI, LOC_DFW, CARRIER_UA);

    initializeNewFarePath(PU(FU(PTF(repriceFM_DFWCHI)), FU(PTF(repriceFM_CHILON))));

    ProcessTagInfo pti1;
    _rexAdvResTktValidator->_allPTIs.push_back(&pti1);
    FareCompInfo fc1;
    pti1.fareCompInfo() = &fc1;
    FareMarket* fm1 = createFareMarket(LOC_DFW, LOC_CHI, CARRIER_AA);
    fc1.fareMarket() = fm1;

    ProcessTagInfo pti2;
    _rexAdvResTktValidator->_allPTIs.push_back(&pti2);
    FareCompInfo fc2;
    pti2.fareCompInfo() = &fc2;
    FareMarket* fm2 = createFareMarket(LOC_CHI, LOC_PAR, CARRIER_UA);
    fc2.fareMarket() = fm2;

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->areAllFCsWithNoChangesToFBAndCarrier());
  }

  void testTicketResvIndIsTheSameOnJourney()
  {
    ProcessTagInfo pti1;
    ReissueSequence rs1;
    rs1.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti1.reissueSequence()->orig() = &rs1;
    _permutation->processTags().push_back(&pti1);
    ProcessTagInfo pti2;
    ReissueSequence rs2;
    rs2.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti2.reissueSequence()->orig() = &rs2;
    _permutation->processTags().push_back(&pti2);
    ProcessTagInfo pti3;
    ReissueSequence rs3;
    rs3.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti3.reissueSequence()->orig() = &rs3;
    _permutation->processTags().push_back(&pti3);

    _rexAdvResTktValidator->setTicketResvIndTheSameOnJourney();
    CPPUNIT_ASSERT(_rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney);
  }

  void testTicketResvIndIsNotTheSameOnJourney()
  {
    ProcessTagInfo pti1;
    ReissueSequence rs1;
    rs1.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti1.reissueSequence()->orig() = &rs1;
    _permutation->processTags().push_back(&pti1);
    ProcessTagInfo pti2;
    ReissueSequence rs2;
    rs2.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti2.reissueSequence()->orig() = &rs2;
    _permutation->processTags().push_back(&pti2);
    ProcessTagInfo pti3;
    ReissueSequence rs3;
    rs3.ticketResvInd() = RexAdvResTktValidator::IGNORE;
    pti3.reissueSequence()->orig() = &rs3;
    _permutation->processTags().push_back(&pti3);

    _rexAdvResTktValidator->setTicketResvIndTheSameOnJourney();
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney);
  }

  void testSimultaneousCheckNeeded()
  {
    bool ticketResvIndNeeded = false;

    ProcessTagInfo pti1;
    ReissueSequence rs1;
    rs1.ticketResvInd() = RexAdvResTktValidator::IGNORE;
    pti1.reissueSequence()->orig() = &rs1;
    _permutation->processTags().push_back(&pti1);
    ProcessTagInfo pti2;
    ReissueSequence rs2;
    rs2.ticketResvInd() = RexAdvResTktValidator::SIMULTANEOUS;
    pti2.reissueSequence()->orig() = &rs2;
    _permutation->processTags().push_back(&pti2);

    _rexAdvResTktValidator->setTicketResvInd(RexAdvResTktValidator::SIMULTANEOUS,
                                             ticketResvIndNeeded);

    CPPUNIT_ASSERT(ticketResvIndNeeded);
  }

  void testSimultaneousCheckNotNeeded()
  {
    bool ticketResvIndNeeded = false;

    ProcessTagInfo pti1;
    ReissueSequence rs1;
    rs1.ticketResvInd() = RexAdvResTktValidator::IGNORE;
    pti1.reissueSequence()->orig() = &rs1;
    _permutation->processTags().push_back(&pti1);
    ProcessTagInfo pti2;
    ReissueSequence rs2;
    rs2.ticketResvInd() = RexAdvResTktValidator::PRIOR_TO_DEPARTURE;
    pti2.reissueSequence()->orig() = &rs2;
    _permutation->processTags().push_back(&pti2);

    _rexAdvResTktValidator->setTicketResvInd(RexAdvResTktValidator::SIMULTANEOUS,
                                             ticketResvIndNeeded);

    CPPUNIT_ASSERT(!ticketResvIndNeeded);
  }

  void testSimultaneousCheckForDomesticItinAndChangedSegmentsPass()
  {
    DateTime bookingDate = DateTime(2008, 2, 13, 12, 0, 0);
    DateTime domesticLatestBookingDateForChangedSegments = bookingDate.addSeconds(1800);
    _trx->currentTicketingDT() = domesticLatestBookingDateForChangedSegments;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->bookingDT() = bookingDate.subtractSeconds(10);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->bookingDT() = bookingDate;
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    _newItin->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT(_rexAdvResTktValidator->checkSimultaneous());
  }

  void testSimultaneousCheckForDomesticItinAndChangedSegmentsFail()
  {
    DateTime bookingDate = DateTime(2008, 2, 13, 12, 0, 0);
    DateTime domesticLatestBookingDateForChangedSegments = bookingDate.addSeconds(1800);
    _trx->currentTicketingDT() = domesticLatestBookingDateForChangedSegments;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->bookingDT() = bookingDate.subtractSeconds(30);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->bookingDT() = bookingDate.addSeconds(10);
    _airSeg2->changeStatus() = TravelSeg::UNCHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    _newItin->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->checkSimultaneous());
  }

  void testSimultaneousCheckForInternationalItinAndChangedSegmentsPass()
  {
    DateTime bookingDate = DateTime(2008, 2, 13, 12, 0, 0);
    DateTime internationalLatestBookingDateForChangedSegments =
        DateTime(bookingDate.date(), 23, 59, 0);
    _trx->currentTicketingDT() = internationalLatestBookingDateForChangedSegments;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->bookingDT() = bookingDate.subtractDays(1);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->bookingDT() = bookingDate;
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    _newItin->geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(_rexAdvResTktValidator->checkSimultaneous());
  }

  void testSimultaneousCheckForInternationalItinAndChangedSegmentsFail()
  {
    DateTime bookingDate = DateTime(2008, 2, 13, 12, 0, 0);
    DateTime internationalLatestBookingDateForChangedSegments =
        DateTime(bookingDate.date(), 23, 59, 0);
    _trx->currentTicketingDT() = internationalLatestBookingDateForChangedSegments;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->bookingDT() = bookingDate.addDays(2);
    _airSeg1->changeStatus() = TravelSeg::UNCHANGED;
    _airSeg2->bookingDT() = bookingDate.subtractDays(1);
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    _newItin->geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->checkSimultaneous());
  }

  void testPriorOfDepartureCheckForChangedSegmentsPass()
  {
    DateTime departureDate = DateTime(2008, 2, 13, 12, 0, 0);
    _trx->currentTicketingDT() = departureDate;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->departureDT() = departureDate.subtractSeconds(10);
    _airSeg1->changeStatus() = TravelSeg::UNCHANGED;
    _airSeg2->departureDT() = departureDate.addSeconds(1);
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    CPPUNIT_ASSERT(_rexAdvResTktValidator->checkPriorOfDeparture());
  }

  void testPriorOfDepartureCheckForChangedSegmentsFail()
  {
    DateTime departureDate = DateTime(2008, 2, 13, 12, 0, 0);
    _trx->currentTicketingDT() = departureDate;

    createAirSegmentsWithStatusUnchanged();
    _airSeg1->departureDT() = departureDate.addSeconds(30);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _airSeg2->departureDT() = departureDate.subtractSeconds(10);
    _airSeg2->changeStatus() = TravelSeg::CHANGED;
    _newItin->travelSeg() += _airSeg1, _airSeg2;

    CPPUNIT_ASSERT(!_rexAdvResTktValidator->checkPriorOfDeparture());
  }

  void testIsUnflownTrueWhenJourney()
  {
    PricingUnit pu;
    FareUsage fu;
    addAirSegmentsToItin(true, true);
    CPPUNIT_ASSERT(
        _rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_JOURNEY, pu, fu));
  }

  void testIsUnflownFalseWhenJourneyAndPartiallyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    addAirSegmentsToItin(false, true);
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_JOURNEY, pu, fu));
  }

  void testIsUnflownFalseWhenJourneyAndFullyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    addAirSegmentsToItin(false, false);
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_JOURNEY, pu, fu));
  }

  void testIsUnflownTrueWhenPricingUnit()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(true, true);
    pu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        _rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_PRICINGUNIT, pu, fu));
  }

  void testIsUnflownFalseWhenPricingUnitAndPartiallyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(false, true);
    pu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_PRICINGUNIT, pu, fu));
  }

  void testIsUnflownFalseWhenPricingUnitAndFullyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(false, false);
    pu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_PRICINGUNIT, pu, fu));
  }

  void testIsUnflownTrueWhenFareComponent()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(true, true);
    fu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        _rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_FARECOMPONENT, pu, fu));
  }

  void testIsUnflownFalseWhenFareComponentAndPartiallyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(false, true);
    fu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_FARECOMPONENT, pu, fu));
  }

  void testIsUnflownFalseWhenFareComponentAndFullyFlown()
  {
    PricingUnit pu;
    FareUsage fu;
    createAirSegments(false, false);
    fu.travelSeg() += _airSeg1, _airSeg2;
    CPPUNIT_ASSERT(
        !_rexAdvResTktValidator->isUnflown(RexAdvResTktValidator::TO_ADVRES_FARECOMPONENT, pu, fu));
  }

  void testIsMostRestrictiveFromDateFalseWhenAdvResOutboundAndFareUsageIsInbound()
  {
    PricingUnit pu;
    createFareUsage(pu);
    _fu->inbound() = true;
    createPtiWithReissueSequence(RexAdvResTktValidator::FROM_ADVRES_OUTBOUND);
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, *_fu));
  }

  void testIsMostRestrictiveFromDateFalseWhenAdvResOutboundAndFareUsageIsOutboundAndFlown()
  {
    PricingUnit pu;
    createFareUsage(pu);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    FareUsage* _fu = FU(PTF(FM(_airSeg1, _airSeg2)));
    _airSeg1->unflown() = false;
    _fu->travelSeg() += _airSeg1;
    _fu->inbound() = false;
    createPtiWithReissueSequence(RexAdvResTktValidator::FROM_ADVRES_OUTBOUND);
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, *_fu));
  }

  void testIsMostRestrictiveFromDateTrueWhenAdvResOutboundAndFareUsageIsOutboundAndUnflown()
  {
    PricingUnit pu;
    createFareUsage(pu);
    _airSeg1->changeStatus() = TravelSeg::CHANGED;
    _fu->travelSeg() += _airSeg1;
    _fu->inbound() = false;
    createPtiWithReissueSequence(RexAdvResTktValidator::FROM_ADVRES_OUTBOUND);
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, *_fu));
  }

  void testIsMostRestrictiveFromDateFalseWhenAdvResTypeOfFareAndFareNotCurrent()
  {
    PricingUnit pu;
    FareUsage fu;
    ProcessTagInfo* _pti = PTI(FC(FM(NULL)));
    _permutation->setFareTypeSelection(UU, CURRENT);
    _permutation->setFareTypeSelection(UC, KEEP);
    _pti->fareCompInfo()->fareMarket()->changeStatus() = UC;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, fu));
  }

  void testIsMostRestrictiveFromDateTrueWhenAdvResTypeOfFareAndFareCurrentAndUnflown()
  {
    createAirSegments(true);
    PricingUnit pu;
    FareUsage fu;
    fu.travelSeg() += _airSeg1;
    ProcessTagInfo* _pti = PTI(FC(FM(NULL)));
    _permutation->setFareTypeSelection(UU, CURRENT);
    _permutation->setFareTypeSelection(UC, KEEP);
    _pti->fareCompInfo()->fareMarket()->changeStatus() = UU;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, fu));
  }

  void testIsMostRestrictiveFromDateFalseWhenAdvResTypeOfFareAndFareCurrentAndFlown()
  {
    createAirSegments(false);
    PricingUnit pu;
    FareUsage fu;
    fu.travelSeg() += _airSeg1;
    ProcessTagInfo* pti = PTI(FC(FM(NULL)));
    _permutation->setFareTypeSelection(UU, CURRENT);
    _permutation->setFareTypeSelection(UC, KEEP);
    pti->fareCompInfo()->fareMarket()->changeStatus() = UU;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isMostRestrictiveFromDate(*pti, pu, fu));
  }

  void testIsMostRestrictiveFromDateFalseWhenAdvResNewTktDateAndFlown()
  {
    createAirSegments(false);
    PricingUnit pu;
    FareUsage fu;
    fu.travelSeg() += _airSeg1;
    createPtiWithReissueSequence(RexAdvResTktValidator::FROM_ADVRES_NEWTKTDATE);
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, fu));
  }

  void testIsMostRestrictiveFromDateTrueWhenAdvResNewTktDateAndUnflown()
  {
    createAirSegments(true);
    PricingUnit pu;
    FareUsage fu;
    fu.travelSeg() += _airSeg1;
    createPtiWithReissueSequence(RexAdvResTktValidator::FROM_ADVRES_NEWTKTDATE);
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isMostRestrictiveFromDate(*_pti, pu, fu));
  }

  void testInitializeEmptyBytes93to106()
  {
    ProcessTagInfo* pti = PTI(0);
    _permutation->processTags().push_back(pti);
    _rexAdvResTktValidator->initialize();
    CPPUNIT_ASSERT(_rexAdvResTktValidator->_emptyBytes93to106);
  }

  void testInitializeFalseEmptyBytes93to106()
  {
    createAirSegments(false);
    ProcessTagInfo* pti = PTI(0);
    pti->paxTypeFare() = PTF(FM(_airSeg1, _airSeg2));
    const_cast<ReissueSequence*>(pti->reissueSequence()->orig())->reissuePeriod() = "abc";
    _permutation->processTags().push_back(pti);
    _rexAdvResTktValidator->initialize();
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->_emptyBytes93to106);
  }

  void testFindMostRestrictiveFromDateForPUFalse()
  {
    createAirSegments(false);
    PricingUnit pu;
    pu.travelSeg().push_back(_airSeg1);
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->findMostRestrictiveFromDateForPU(pu));
  }

  void testFindMostRestrictiveFromDateForPUFalseNoFU()
  {
    createAirSegments(true);
    PricingUnit pu;
    pu.travelSeg().push_back(_airSeg1);
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->findMostRestrictiveFromDateForPU(pu));
  }

  void testIsJourneyScopeTicketResvIndTheSameOnJourney()
  {
    _rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney = true;
    _rexAdvResTktValidator->_allFCsFBAndCarrierUnchanged = true;
    _rexAdvResTktValidator->_emptyBytes93to106 = false;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isJourneyScope());
  }

  void testIsJourneyScopeAllFCsFBAndCarrierUnchanged()
  {
    _rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney = false;
    _rexAdvResTktValidator->_allFCsFBAndCarrierUnchanged = false;
    _rexAdvResTktValidator->_emptyBytes93to106 = false;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->isJourneyScope());
  }

  void testIsJourneyScopeFalse()
  {
    _rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney = false;
    _rexAdvResTktValidator->_allFCsFBAndCarrierUnchanged = true;
    _rexAdvResTktValidator->_emptyBytes93to106 = false;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isJourneyScope());
  }

  void testIsJourneyScopeFalseEmptyBytes93to106()
  {
    _rexAdvResTktValidator->_isTicketResvIndTheSameOnJourney = true;
    _rexAdvResTktValidator->_allFCsFBAndCarrierUnchanged = false;
    _rexAdvResTktValidator->_emptyBytes93to106 = true;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->isJourneyScope());
  }

  void testEmptyBytes93to106()
  {
    ReissueSequence rs;
    ReissueSequenceW rsw;
    rsw.orig() = &rs;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testEmptyBytes93to106NoOrig()
  {
    ReissueSequenceW rsw;
    CPPUNIT_ASSERT(_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testEmptyBytes93to106FalseReissuePeriod()
  {
    ReissueSequence rs;
    rs.reissuePeriod() = "abc";
    ReissueSequenceW rsw;
    rsw.orig() = &rs;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testEmptyBytes93to106FalseReissueUnit()
  {
    ReissueSequence rs;
    rs.reissueUnit() = "abc";
    ReissueSequenceW rsw;
    rsw.orig() = &rs;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testEmptyBytes93to106FalseDeparture()
  {
    ReissueSequence rs;
    rs.departure() = 1;
    ReissueSequenceW rsw;
    rsw.orig() = &rs;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testEmptyBytes93to106FalseDepartureUnit()
  {
    ReissueSequence rs;
    rs.departureUnit() = 'A';
    ReissueSequenceW rsw;
    rsw.orig() = &rs;
    CPPUNIT_ASSERT(!_rexAdvResTktValidator->emptyBytes93to106(rsw));
  }

  void testToDateNeverEmpty()
  {
    FareUsage* fu = FU(PTF(FM(0)), _memHandle.create<AirSeg>());
    PricingUnit pu;
    pu.fareUsage().push_back(fu);

    DateTime refDate = DateTime(2008, 2, 13, 12, 0, 0);

    fu->travelSeg().front()->departureDT() = refDate;

    CPPUNIT_ASSERT_EQUAL(refDate, _rexAdvResTktValidator->findMostRestrictiveToDateForPU(pu, *fu));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(RexAdvResTktValidatorTest);
}
