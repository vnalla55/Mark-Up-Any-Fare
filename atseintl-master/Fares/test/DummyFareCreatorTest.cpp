#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Fares/DummyFareCreator.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"
#include <gtest/gtest.h>

namespace tse
{
class DummyFareCreatorTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _req = _memHandle.create<PricingRequest>();
    _pro = _memHandle.create<PricingOptions>();
    TrxBuilder trxBld(_memHandle);
    _trx = trxBld.withPaxTypes({"ADT", "CNN"}).build();
    _trx->setRequest(_req);
    _trx->setOptions(_pro);
    _fm = _memHandle.create<FareMarket>();
    AirSegBuilder asBld(_memHandle);
    _fm->travelSeg() = { asBld.withLocs("DFW", "JFK").withCxr("AA").withCabin('8').build(),
                         asBld.withLocs("JFK", "LON").withCxr("LH").withCabin('5').build() };
    _fm->primarySector() = _fm->travelSeg().back();
    _fm->boardMultiCity() = "DFW";
    _fm->offMultiCity() = "LON";
    _fm->governingCarrier() = "LH";
    _fm->setGlobalDirection(GlobalDirection::AT);
    _fm->geoTravelType() = GeoTravelType::International;
    _fm->fareCalcFareAmt() = "0.00";
    _fm->fareBasisCode() = "Y";
    _fm->paxTypeCortege().resize(_trx->paxType().size());
    for (size_t paxNo = 0; paxNo < _trx->paxType().size(); ++paxNo)
      _fm->paxTypeCortege()[paxNo].requestedPaxType() = _trx->paxType()[paxNo];
  }

  void TearDown() { _memHandle.clear(); }

protected:

  void assertPointers(const PaxTypeFare* ptf)
  {
    ASSERT_NE(nullptr, ptf);
    ASSERT_NE(nullptr, ptf->fare());
    ASSERT_NE(nullptr, ptf->fare()->fareInfo());
    ASSERT_NE(nullptr, ptf->fare()->tariffCrossRefInfo());
    ASSERT_NE(nullptr, ptf->fareClassAppInfo());
    ASSERT_NE(nullptr, ptf->fareClassAppSegInfo());
    ASSERT_NE(nullptr, ptf->rec2Cat10());
  }

  void expectFareInCortege(PaxTypeFare* ptf, const PaxTypeBucket& ptc)
  {
    EXPECT_EQ(ptc.requestedPaxType()->paxType(), ptf->fareClassAppSegInfo()->_paxType);
    const auto& ptcFares = ptc.paxTypeFare();
    EXPECT_TRUE(std::find(ptcFares.begin(), ptcFares.end(), ptf) != ptcFares.end());
  }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _req;
  PricingOptions* _pro;
  FareMarket* _fm;
};

TEST_F(DummyFareCreatorTest, testCreatePtf)
{
  DummyFareCreator::FareSettings sett;
  sett._pax = "ADT";
  sett._currency = "USD";
  sett._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

  PaxTypeFare* const ptf = DummyFareCreator::createPtf(*_trx, Itin(), *_fm, sett);
  assertPointers(ptf);

  EXPECT_EQ(_fm, ptf->fareMarket());
  EXPECT_EQ(ATPCO_VENDOR_CODE, ptf->vendor());
  EXPECT_TRUE(ptf->ruleNumber().empty());
  EXPECT_EQ(0, ptf->fareTariff());
  EXPECT_EQ(CarrierCode("LH"), ptf->carrier());
  EXPECT_EQ(LocCode("DFW"), ptf->market1());
  EXPECT_EQ(LocCode("LON"), ptf->market2());
  EXPECT_EQ(GlobalDirection::AT, ptf->globalDirection());
  EXPECT_EQ(FareClassCode("Y"), ptf->fareClass());
  EXPECT_EQ(ROUND_TRIP_MAYNOT_BE_HALVED, ptf->owrt());
  EXPECT_TRUE(ptf->isValid());
  EXPECT_TRUE(ptf->isRoutingProcessed());
  EXPECT_TRUE(ptf->cabin().isBusinessClass());

  EXPECT_TRUE(ptf->nucFareAmount() < EPSILON);
  EXPECT_EQ(2, ptf->fare()->fareInfo()->noDec());
  EXPECT_EQ(CurrencyCode("USD"), ptf->currency());
}

TEST_F(DummyFareCreatorTest, testCreatePtf_PrivateOwFare)
{
  DummyFareCreator::FareSettings sett;
  sett._pax = "CNN";
  sett._currency = "PLN";
  sett._owrt = ONE_WAY_MAY_BE_DOUBLED;
  sett._privateTariff = true;

  PaxTypeFare* const ptf = DummyFareCreator::createPtf(*_trx, Itin(), *_fm, sett);
  assertPointers(ptf);

  EXPECT_EQ(ONE_WAY_MAY_BE_DOUBLED, ptf->owrt());
  EXPECT_EQ(CurrencyCode("PLN"), ptf->currency());
  EXPECT_EQ(1, ptf->tcrTariffCat());
}

TEST_F(DummyFareCreatorTest, testCreateFaresForExchange)
{
  Itin itin;
  itin.calcCurrencyOverride() = "GBP";
  itin.calculationCurrency() = "GBP";
  DummyFareCreator::createFaresForExchange(*_trx, itin, *_fm);

  ASSERT_EQ(size_t(1), _fm->allPaxTypeFare().size());
  PaxTypeFare* ptf = _fm->allPaxTypeFare().front();
  assertPointers(ptf);

  EXPECT_EQ(PaxTypeCode("ADT"), ptf->fareClassAppSegInfo()->_paxType);
  EXPECT_EQ(CurrencyCode("GBP"), ptf->currency());
  EXPECT_EQ(ROUND_TRIP_MAYNOT_BE_HALVED, ptf->owrt());

  for (const auto& ptc : _fm->paxTypeCortege())
  {
    ASSERT_EQ(size_t(1), ptc.paxTypeFare().size());
    ASSERT_EQ(ptf, ptc.paxTypeFare().front());
  }
}

TEST_F(DummyFareCreatorTest, createFaresForOriginBasedRT)
{
  _trx->calendarCurrencyCode() = "USD";
  Itin itin;
  DummyFareCreator::createFaresForOriginBasedRT(*_trx, itin, *_fm);

  ASSERT_EQ(size_t(4), _fm->allPaxTypeFare().size()); // (OW, RT) x (ADT, CNN)
  ASSERT_EQ(size_t(2), _fm->paxTypeCortege().front().paxTypeFare().size());
  ASSERT_EQ(size_t(2), _fm->paxTypeCortege().back().paxTypeFare().size());

  for (PaxTypeFare* ptf : _fm->allPaxTypeFare())
  {
    assertPointers(ptf);
    EXPECT_EQ(CurrencyCode("USD"), ptf->currency());
  }

  PaxTypeFare* ptfAdtRt = _fm->allPaxTypeFare()[0];
  PaxTypeFare* ptfAdtOw = _fm->allPaxTypeFare()[1];
  PaxTypeFare* ptfCnnRt = _fm->allPaxTypeFare()[2];
  PaxTypeFare* ptfCnnOw = _fm->allPaxTypeFare()[3];

  expectFareInCortege(ptfAdtRt, _fm->paxTypeCortege().front());
  expectFareInCortege(ptfAdtOw, _fm->paxTypeCortege().front());
  expectFareInCortege(ptfCnnRt, _fm->paxTypeCortege().back());
  expectFareInCortege(ptfCnnOw, _fm->paxTypeCortege().back());

  EXPECT_EQ(ROUND_TRIP_MAYNOT_BE_HALVED, ptfAdtRt->owrt());
  EXPECT_EQ(ROUND_TRIP_MAYNOT_BE_HALVED, ptfCnnRt->owrt());
  EXPECT_EQ(ONE_WAY_MAY_BE_DOUBLED, ptfAdtOw->owrt());
  EXPECT_EQ(ONE_WAY_MAY_BE_DOUBLED, ptfCnnOw->owrt());
}

}
