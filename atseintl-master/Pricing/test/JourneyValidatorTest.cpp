//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/ClassOfService.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/JourneyValidator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Pricing/test/PricingMockDataBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using ::testing::Return;
class JourneyValidatorTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _diag = _memHandle.create<DiagCollector>();
    _trx = PricingMockDataBuilder::getPricingTrx();
    _journeyValidator = _memHandle.create<JourneyValidator>(*_trx, *_diag);
    _pricingOptions = _memHandle.create<PricingOptions>();
    _farePath = _memHandle.create<FarePath>();
    _fu11 = _memHandle.create<FareUsage>();
    _fu12 = _memHandle.create<FareUsage>();
    _fu21 = _memHandle.create<FareUsage>();
    _fu22 = _memHandle.create<FareUsage>();
    PricingUnit* _pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* _pu2 = _memHandle.create<PricingUnit>();

    PaxTypeFare* ptf11 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf12 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf21 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf22 = _memHandle.create<PaxTypeFare>();

    _fu11->paxTypeFare() = ptf11;
    _fu12->paxTypeFare() = ptf12;
    _fu21->paxTypeFare() = ptf21;
    _fu22->paxTypeFare() = ptf22;
    _pu1->fareUsage().push_back(_fu11);
    _pu1->fareUsage().push_back(_fu12);
    _pu2->fareUsage().push_back(_fu21);
    _pu2->fareUsage().push_back(_fu22);
    _farePath->pricingUnit().push_back(_pu1);
    _farePath->pricingUnit().push_back(_pu2);

    Itin* itin = addItin();
    _trx->itin().push_back(itin);
    _farePath->itin() = itin;
  }

  void TearDown() override
  {
    _memHandle.clear();
    delete _trx;
    _trx = nullptr;
  }

  Itin* addItin()
  {
    Itin* itin = PricingMockDataBuilder::addItin(*_trx);

    // DFW-AA-NYC-BA-LON-BA-NYC-AA-DFW
    Loc* loc1 = PricingMockDataBuilder::getLoc(*_trx, "DFW");
    Loc* loc2 = PricingMockDataBuilder::getLoc(*_trx, "NYC");
    Loc* loc3 = PricingMockDataBuilder::getLoc(*_trx, "LON");

    TravelSeg* tSeg1 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc1, loc2, 1);
    TravelSeg* tSeg2 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc2, loc3, 2);

    TravelSeg* tSeg3 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc3, loc2, 3);
    TravelSeg* tSeg4 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc2, loc1, 4);

    FareMarket* fm1 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc1, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);

    FareMarket* fm2 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc2, loc3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);

    FareMarket* fm3 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc3, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);

    FareMarket* fm4 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc2, loc1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg4, *fm4);

    return itin;
  }
  void setJourneyValidatorWithNewTrx(PricingTrx& trx)
  {
    _journeyValidator = _memHandle.create<JourneyValidator>(trx, *_diag);
  }
  void prepareTrx()
  {
    PricingOptions* options = _memHandle.create<PricingOptions>();
    options->applyJourneyLogic() = true;
    options->journeyActivatedForPricing() = true;
    options->journeyActivatedForShopping() = true;
    _trx->setOptions(options);
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->lowFareRequested() = YES;
    _trx->setRequest(req);
    Billing* billing = _memHandle.create<Billing>();
    billing->actionCode() = "WPNC";
    _trx->billing() = billing;
  }
  void putTravelSegsInFareUsages()
  {
    Itin* itin = _trx->itin()[0];
    itin->travelSeg()[0]->setBookingCode("Y");
    itin->travelSeg()[1]->setBookingCode("Y");
    itin->travelSeg()[2]->setBookingCode("Y");
    itin->travelSeg()[3]->setBookingCode("Y");

    _fu11->travelSeg().push_back(itin->travelSeg()[0]);
    _fu12->travelSeg().push_back(itin->travelSeg()[1]);
    _fu21->travelSeg().push_back(itin->travelSeg()[2]);
    _fu22->travelSeg().push_back(itin->travelSeg()[3]);

    _fu11->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu12->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu21->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu22->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
    _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu12->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu21->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
    _fu22->bookingCodeStatus() = PaxTypeFare::BKS_PASS;

    PaxTypeFare* ptf11 = _memHandle.create<PaxTypeFare>();
    ptf11->fareMarket() = _trx->itin()[0]->fareMarket()[0];
    PaxTypeFare* ptf12 = _memHandle.create<PaxTypeFare>();
    ptf12->fareMarket() = _trx->itin()[0]->fareMarket()[1];
    PaxTypeFare* ptf21 = _memHandle.create<PaxTypeFare>();
    ptf21->fareMarket() = _trx->itin()[0]->fareMarket()[2];
    PaxTypeFare* ptf22 = _memHandle.create<PaxTypeFare>();
    ptf22->fareMarket() = _trx->itin()[0]->fareMarket()[3];
    _fu11->paxTypeFare() = ptf11;
    _fu12->paxTypeFare() = ptf12;
    _fu21->paxTypeFare() = ptf21;
    _fu22->paxTypeFare() = ptf22;

    _fu11->segmentStatus().resize(1);
    _fu12->segmentStatus().resize(1);
    _fu21->segmentStatus().resize(1);
    _fu22->segmentStatus().resize(1);
    _fu11->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu11->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu12->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu12->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu21->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu21->segmentStatus()[0]._bkgCodeReBook = "Q";
    _fu22->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu22->segmentStatus()[0]._bkgCodeReBook = "Q";
  }
  void putDifferentialDataInFu(FareUsage* fu)
  {
    DifferentialData* diffData = _memHandle.create<DifferentialData>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->segmentStatus().resize(1);
    ptf->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    ptf->segmentStatus()[0]._bkgCodeReBook = "D";
    diffData->fareHigh() = ptf;
    diffData->travelSeg() = fu->travelSeg();
    diffData->status() = DifferentialData::SC_PASSED;
    fu->differentialPlusUp().push_back(diffData);
  }
  void preparePaxType()
  {
    PaxType* actualPType = _memHandle.create<PaxType>();
    actualPType->totalPaxNumber() = 1;
    actualPType->number() = 1;
    PaxTypeInfo* pInfo = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo::dummyData(*pInfo);
    actualPType->paxTypeInfo() = pInfo;
    _trx->paxType().clear();
    _trx->paxType().push_back(actualPType);
  }
  void fillInClassOfService(std::vector<ClassOfService*>& vec, ClassOfService& cos)
  {
    cos.bookingCode() = "Q";
    cos.numSeats() = 2;
    vec.push_back(&cos);
  }

  void checkStartFlow(FarePath& fp, TravelSeg* tvlSeg, size_t expectedFlowLen, bool shouldPass)
  {
    bool failJourney = false;
    ASSERT_EQ(expectedFlowLen, _journeyValidator->startFlow(fp, tvlSeg, failJourney));
    ASSERT_TRUE(failJourney == shouldPass);
  }

  void checkLocalAvailStatus(FareUsage* fu, TravelSeg* tvlSeg, uint16_t tvlSegIndex, bool expected)
  {
    ASSERT_EQ(_journeyValidator->checkLocalAvailStatus(fu, tvlSeg, tvlSegIndex), expected);
  }
  void checkRealAvail(FareUsage* fu,
                      uint16_t iTravelFu,
                      FareMarket& fm,
                      uint16_t iTravelFm,
                      bool checkBookedCosAlso,
                      bool useLocal,
                      bool expected)
  {
    ASSERT_EQ(
        _journeyValidator->realAvail(_fu11, iTravelFu, fm, iTravelFm, checkBookedCosAlso, useLocal),
        expected);
  }
  void checkCat5Rebook(const FareUsage* fu,
                       const TravelSeg* tvlSeg,
                       const PaxTypeFare::SegmentStatus& segStat,
                       bool expected)
  {
    ASSERT_EQ(_journeyValidator->hasCat5Rebook(fu, tvlSeg, segStat), expected);
  }
  void checkBookingCode(const FareUsage* fu,
                        const TravelSeg* tvlSeg,
                        const PaxTypeFare::SegmentStatus& fuSegStat,
                        BookingCode expectedBookingCode)
  {
    ASSERT_EQ(expectedBookingCode, _journeyValidator->bookingCode(fu, tvlSeg, fuSegStat));
  }

  bool flowAvailFailed() { return _journeyValidator->flowAvailFailed(*_fu11); }
  bool foundFlow(FarePath& fpath, TravelSeg* startTvlseg, size_t& flowLen)
  {
    return _journeyValidator->foundFlow(fpath, startTvlseg, flowLen);
  }

  AirSeg* makeAirSeg(const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     const CarrierCode& carrier,
                     LocCode origAirport = "",
                     LocCode destAirport = "")
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc(boardMultiCity, DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc(offMultiCity, DateTime::localTime());

    if ("" == origAirport)
      origAirport = boardMultiCity;
    if ("" == destAirport)
      destAirport = offMultiCity;

    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = boardMultiCity;
    as->offMultiCity() = offMultiCity;
    as->origAirport() = origAirport;
    as->destAirport() = destAirport;
    as->carrier() = carrier;
    return as;
  }

  FareMarket*
  makeFareMarket(AirSeg* as1, AirSeg* as2 = NULL, AirSeg* as3 = NULL, AirSeg* as4 = NULL)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(as1);
    if (NULL != as2)
      fm->travelSeg().push_back(as2);
    if (NULL != as3)
      fm->travelSeg().push_back(as3);
    if (NULL != as4)
      fm->travelSeg().push_back(as4);

    return fm;
  }

  Itin*
  generateItinTravelSeg(AirSeg* as1, AirSeg* as2 = NULL, AirSeg* as3 = NULL, AirSeg* as4 = NULL)
  {
    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(as1);
    if (NULL != as2)
      itin->travelSeg().push_back(as2);
    if (NULL != as3)
      itin->travelSeg().push_back(as3);
    if (NULL != as4)
      itin->travelSeg().push_back(as4);

    return itin;
  }

protected:
  TestMemHandle _memHandle;
  JourneyValidator* _journeyValidator;
  DiagCollector* _diag;
  PricingOptions* _pricingOptions;
  PricingTrx* _trx;
  FarePath* _farePath;
  FareUsage* _fu11;
  FareUsage* _fu12;
  FareUsage* _fu21;
  FareUsage* _fu22;
};

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnFalseWhenRexPricingNotNewItinPhase)
{
  RexPricingTrx rexTrx;
  rexTrx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
  setJourneyValidatorWithNewTrx(rexTrx);
  ASSERT_TRUE(!_journeyValidator->checkMarriedConnection());
}
TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnFalseWhenNotWpnc)
{
  prepareTrx();
  _trx->getRequest()->lowFareRequested() = NO;
  ASSERT_TRUE(!_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnFalseWhenMIPandNotCurrentItin)
{
  prepareTrx();
  _trx->setTrxType(PricingTrx::MIP_TRX);
  ASSERT_TRUE(!_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnFalseWhenNotMip)
{
  prepareTrx();
  _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
  ASSERT_TRUE(!_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnTrueWhenMIPCurrentItin)
{
  prepareTrx();
  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->billing()->actionCode() = "WPNI.C";
  ASSERT_TRUE(_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnTrueWhenWpncPricing)
{
  prepareTrx();
  ASSERT_TRUE(_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testCheckMarriedConnectionReturnTrueWhenRexPricingNewItinPhase)
{
  prepareTrx();
  RexPricingTrx rexTrx;
  rexTrx.setOptions(_trx->getOptions());
  rexTrx.setRequest(_trx->getRequest());
  rexTrx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
  setJourneyValidatorWithNewTrx(rexTrx);
  ASSERT_TRUE(_journeyValidator->checkMarriedConnection());
}

TEST_F(JourneyValidatorTest, testStartFlowReturnZeroWhenStartTvlSegNotFound)
{
  putTravelSegsInFareUsages();
  AirSeg airSeg;
  checkStartFlow(*_farePath, &airSeg, 0, false);
}
TEST_F(JourneyValidatorTest, testStartFlowReturnOneWhenStartTvlSegFailLocalAvail)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL,
                                 false); // means start tvl Seg passed local avail

  size_t expectedFlowLen = 1;
  checkStartFlow(*_farePath, _trx->itin()[0]->travelSeg()[0], expectedFlowLen, true);
}

TEST_F(JourneyValidatorTest, testStartFlowReturnOneWhenInterline)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  size_t expectedFlowLen = 1;
  checkStartFlow(*_farePath, _trx->itin()[0]->travelSeg()[0], expectedFlowLen, false);
}
TEST_F(JourneyValidatorTest, testStartFlowReturnOneWhenSecondSegmentNotMultiAirportArunk)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  ArunkSeg arunkSeg;
  arunkSeg.boardMultiCity() = LOC_KUL;
  arunkSeg.offMultiCity() = LOC_NYC;
  _trx->itin()[0]->travelSeg()[1] = &arunkSeg;
  size_t expectedFlowLen = 1;
  checkStartFlow(*_farePath, _trx->itin()[0]->travelSeg()[0], expectedFlowLen, false);
}

TEST_F(JourneyValidatorTest, testStartFlowReturnTwoWhenSecondSegmentMultiAirportArunk)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  ArunkSeg arunkSeg;
  arunkSeg.boardMultiCity() = LOC_EWR;
  arunkSeg.offMultiCity() = LOC_NYC;
  _trx->itin()[0]->travelSeg()[1] = &arunkSeg;

  size_t expectedFlowLen = 2;
  checkStartFlow(*_farePath, _trx->itin()[0]->travelSeg()[0], expectedFlowLen, false);
}
TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnFalseWhenFuZero)
{
  FareUsage* actualFareUsage = 0;
  checkLocalAvailStatus(actualFareUsage, _trx->itin()[0]->travelSeg()[0], 0, false);
}
TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnFalseWhenFareFailedByMixClass)
{
  putTravelSegsInFareUsages();
  _fu11->mixClassStatus() = PaxTypeFare::MX_DIFF;
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_FAIL;
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, false);
}
TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnTrueWhenFarePassedByMixClass)
{
  putTravelSegsInFareUsages();
  _fu11->mixClassStatus() = PaxTypeFare::MX_DIFF;
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, true);
}

TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnFalseWhenFareFailed)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_FAIL;
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, false);
}

TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnFalseWhenFareFailedLocalAvail)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, false);
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, false);
}

TEST_F(JourneyValidatorTest, testCheckLocalAvailStatusReturnTrueWhenFarePassedLocalAvail)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, true);
}

TEST_F(JourneyValidatorTest,
       testCheckLocalAvailStatusReturnFalseWhenFareHasCat5RebookButBookedClassNotAvail)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  _trx->itin()[0]->travelSeg()[0]->setBookingCode(
      _fu11->segmentStatus()[0]._bkgCodeReBook); // means cat5 rebook
  std::vector<ClassOfService*> vec;
  ClassOfService cos;
  fillInClassOfService(vec, cos);
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec().push_back(&vec);
  cos.numSeats() = 0; // means class not Avail
  preparePaxType();
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, false);
}

TEST_F(JourneyValidatorTest,
       testCheckLocalAvailStatusReturnTrueWhenFareHasCat5RebookAndBookedClassAvail)
{
  putTravelSegsInFareUsages();
  _fu11->bookingCodeStatus().set(PaxTypeFare::BKS_PASS_LOCAL_AVAIL, true);
  _trx->itin()[0]->travelSeg()[0]->setBookingCode(
      _fu11->segmentStatus()[0]._bkgCodeReBook); // means cat5 rebook
  std::vector<ClassOfService*> vec;
  ClassOfService cos;
  fillInClassOfService(vec, cos);
  preparePaxType();
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec().push_back(&vec);
  _trx->itin()[0]->fareMarket()[0]->travelSeg()[0]->classOfService() = vec;
  checkLocalAvailStatus(_fu11, _trx->itin()[0]->travelSeg()[0], 0, true);
}
TEST_F(JourneyValidatorTest, testRealAvailReturnFalseIfTravelFmIndexGreaterThanNumberOfTvlSegsInFm)
{
  putTravelSegsInFareUsages();
  uint16_t iTravelFm = _trx->itin()[0]->fareMarket()[0]->travelSeg().size() + 1;
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), iTravelFm, true, false, false);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnFalseIfTravelFuIndexGreaterThanNumberOfTvlSegsInFu)
{
  putTravelSegsInFareUsages();
  uint16_t iTravelFu = _fu11->travelSeg().size() + 1;
  checkRealAvail(_fu11, iTravelFu, *(_trx->itin()[0]->fareMarket()[0]), 0, true, false, false);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnTrueIfAsBookedAndCheckBookedCosIsFalse)
{
  putTravelSegsInFareUsages();
  _fu11->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), 0, false, false, true);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnFalseIfFmCosVecEmpty)
{
  putTravelSegsInFareUsages();
  preparePaxType();
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), 0, true, false, false);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnFalseIfCosVecZero)
{
  putTravelSegsInFareUsages();
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec().resize(1);
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0] = 0;
  preparePaxType();
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), 0, true, false, false);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnTrueWhenBookingCodeAvailable)
{
  putTravelSegsInFareUsages();
  std::vector<ClassOfService*> vec;
  ClassOfService cos;
  fillInClassOfService(vec, cos);
  preparePaxType();
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec().push_back(&vec);
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), 0, true, false, true);
}

TEST_F(JourneyValidatorTest, testRealAvailReturnFalseWhenBookingCodeNotAvailable)
{
  putTravelSegsInFareUsages();
  std::vector<ClassOfService*> vec;
  ClassOfService cos;
  fillInClassOfService(vec, cos);
  preparePaxType();
  cos.numSeats() = 0;
  _trx->itin()[0]->fareMarket()[0]->classOfServiceVec().push_back(&vec);
  checkRealAvail(_fu11, 0, *(_trx->itin()[0]->fareMarket()[0]), 0, true, false, false);
}
TEST_F(JourneyValidatorTest, testHasCat5RebookReturnFalseWhenRebookClassNotSameAsBooked)
{
  putTravelSegsInFareUsages();
  checkCat5Rebook(_fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], false);
}

TEST_F(JourneyValidatorTest, testHasCat5RebookReturnTrueWhenRebookClassSameAsBooked)
{
  putTravelSegsInFareUsages();
  _trx->itin()[0]->travelSeg()[0]->setBookingCode(_fu11->segmentStatus()[0]._bkgCodeReBook);
  checkCat5Rebook(_fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], true);
}
TEST_F(JourneyValidatorTest, testHasCat5RebookReturnFalseWhenDiffFareNotRebooked)
{
  putTravelSegsInFareUsages();
  putDifferentialDataInFu(_fu11);
  _fu11->differentialPlusUp()[0]->fareHigh()->segmentStatus()[0]._bkgCodeSegStatus.set(
      PaxTypeFare::BKSS_REBOOKED, false);
  checkCat5Rebook(_fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], false);
}

TEST_F(JourneyValidatorTest, testHasCat5RebookReturnFalseWhenDiffFareRebookClassNotSameAsBooked)
{
  putTravelSegsInFareUsages();
  putDifferentialDataInFu(_fu11);
  checkCat5Rebook(_fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], false);
}

TEST_F(JourneyValidatorTest, testHasCat5RebookReturnTrueWhenDiffFareRebookClassSameAsBooked)
{
  putTravelSegsInFareUsages();
  putDifferentialDataInFu(_fu11);
  _trx->itin()[0]->travelSeg()[0]->setBookingCode(
      _fu11->differentialPlusUp()[0]->fareHigh()->segmentStatus()[0]._bkgCodeReBook);
  checkCat5Rebook(_fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], true);
}
TEST_F(JourneyValidatorTest, testBookingCodeReturnRebookedClassWhenSegmentRebooked)
{
  putTravelSegsInFareUsages();
  BookingCode expectedBookingCode = _fu11->segmentStatus()[0]._bkgCodeReBook;

  checkBookingCode(
      _fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], expectedBookingCode);
}

TEST_F(JourneyValidatorTest, testBookingCodeReturnAsbookedClassWhenSegmentNotRebooked)
{
  putTravelSegsInFareUsages();
  BookingCode expectedBookingCode = _trx->itin()[0]->travelSeg()[0]->getBookingCode();
  _fu11->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
  checkBookingCode(
      _fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], expectedBookingCode);
}

TEST_F(JourneyValidatorTest, testBookingCodeReturnRebookClassFromDiffDataWHenDiffFareRebooked)
{
  putTravelSegsInFareUsages();
  putDifferentialDataInFu(_fu11);
  BookingCode expectedBookingCode =
      _fu11->differentialPlusUp()[0]->fareHigh()->segmentStatus()[0]._bkgCodeReBook;

  checkBookingCode(
      _fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], expectedBookingCode);
}

TEST_F(JourneyValidatorTest, testBookingCodeReturnAsbookedClassWhenDiffFareNotRebooked)
{
  putTravelSegsInFareUsages();
  putDifferentialDataInFu(_fu11);
  BookingCode expectedBookingCode = _trx->itin()[0]->travelSeg()[0]->getBookingCode();
  _fu11->differentialPlusUp()[0]->fareHigh()->segmentStatus()[0]._bkgCodeSegStatus.set(
      PaxTypeFare::BKSS_REBOOKED, false);
  checkBookingCode(
      _fu11, _trx->itin()[0]->travelSeg()[0], _fu11->segmentStatus()[0], expectedBookingCode);
}

TEST_F(JourneyValidatorTest, testFlowAvailFailed_ReturnFalseWhen_Status_PassFlowAvail)
{
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS_FLOW_AVAIL;
  ASSERT_TRUE(!flowAvailFailed());
}

TEST_F(JourneyValidatorTest, testFlowAvailFailed_ReturnFalseWhen_Status_MixedFlowAvail)
{
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_MIXED_FLOW_AVAIL;
  ASSERT_TRUE(!flowAvailFailed());
}

TEST_F(JourneyValidatorTest, testFlowAvailFailed_ReturnTrueWhen_Status_FareFail)
{
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_FAIL;
  ASSERT_TRUE(flowAvailFailed());
}

TEST_F(JourneyValidatorTest,
       testFlowAvailFailed_ReturnTrueWhen_Status_Not_PassFlowAvail_MixedFlowAvail)
{
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_PASS;
  ASSERT_TRUE(flowAvailFailed());
}

TEST_F(JourneyValidatorTest,
       testFlowAvailFailed_ReturnTrueWhen_Status_MixedFlowAvailAndFailFlowAvail)
{
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_MIXED_FLOW_AVAIL;
  _fu11->bookingCodeStatus() = PaxTypeFare::BKS_FAIL_FLOW_AVAIL;
  ASSERT_TRUE(flowAvailFailed());
}

TEST_F(JourneyValidatorTest, testFoundFlow)
{
  AirSeg* airSeg = makeAirSeg("BBB", "KRK", "AA");
  AirSeg* airSegcc = makeAirSeg("CCC", "KRK", "AA");
  size_t flowLen = 2;

  Itin* itin = generateItinTravelSeg(airSeg, airSegcc);

  // add fare market
  itin->fareMarket().push_back(makeFareMarket(airSeg, airSegcc));

  FarePath* fpath = _memHandle.create<FarePath>();
  fpath->itin() = itin;

  ASSERT_TRUE(foundFlow(*fpath, airSeg, flowLen));
}

TEST_F(JourneyValidatorTest, testFoundFlowReverse)
{
  AirSeg* airSeg = makeAirSeg("BBB", "KRK", "AA");
  AirSeg* airSegcc = makeAirSeg("CCC", "KRK", "AA");
  size_t flowLen = 2;

  Itin* itin = generateItinTravelSeg(airSeg, airSegcc);

  // add fare market
  itin->fareMarket().push_back(makeFareMarket(airSegcc, airSeg));

  FarePath* fpath = _memHandle.create<FarePath>();
  fpath->itin() = itin;

  ASSERT_TRUE(!foundFlow(*fpath, airSeg, flowLen));
}

TEST_F(JourneyValidatorTest, testFoundFlowCoupleMarket)
{
  AirSeg* airSeg = makeAirSeg("BBB", "KRK", "AA");
  AirSeg* airSegcc = makeAirSeg("CCC", "KRK", "AA");
  AirSeg* airSeglon = makeAirSeg("LON", "KRK", "AA");
  size_t flowLen = 2;

  Itin* itin = generateItinTravelSeg(airSeg, airSegcc);

  // add fare market
  itin->fareMarket().push_back(makeFareMarket(airSeglon, airSegcc));
  itin->fareMarket().push_back(makeFareMarket(airSeg, airSegcc));
  itin->fareMarket().push_back(makeFareMarket(airSegcc, airSeglon));
  itin->fareMarket().push_back(makeFareMarket(airSegcc, airSeglon, airSeg));

  FarePath* fpath = _memHandle.create<FarePath>();
  fpath->itin() = itin;

  ASSERT_TRUE(foundFlow(*fpath, airSeg, flowLen));
}
}
