#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Diagnostic/Diag864Collector.h"
#include "Diagnostic/DiagManager.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DBAccess/FareCalcConfig.h"

#include "DataModel/PaxTypeFare.h"

namespace tse
{
class Diag864CollectorTest : public ::testing::Test
{
protected:
  class StubFareUsage : public FareUsage
  {
  public:
    StubFareUsage(TestMemHandle& memHandle) : FareUsage()
    {
      _paxTypeFare = memHandle.create<PaxTypeFare>();
      _paxTypeFare->fareMarket() = memHandle.create<FareMarket>();
      _paxTypeFare->fareMarket()->boardMultiCity() = "NYC";
      _paxTypeFare->fareMarket()->governingCarrier() = "AA";
      _paxTypeFare->fareMarket()->offMultiCity() = "LAX";
      auto* fareInfo = memHandle.create<FareInfo>();
      fareInfo->fareAmount() = 100.0;
      _paxTypeFare->fare()->nucFareAmount() = 100.0;
      _paxTypeFare->fare()->setFareInfo(fareInfo);
      minFarePlusUpAmt() = 120.0;
      differentialAmt() = 130.0;
      surchargeAmt() = 140.0;
      transferAmt() = 100.0;
      stopOverAmt() = 110.0;
      setDiscAmount(50.0);
    }
  };

  class StubPricingUnit : public PricingUnit
  {
  public:
    StubPricingUnit(TestMemHandle& memHandle) : PricingUnit()
    {
      fareUsage().push_back(memHandle.create<StubFareUsage>(memHandle));
    }
  };

  class StubFarePath : public FarePath
  {
  public:
    StubFarePath(TestMemHandle& memHandle) : FarePath()
    {
      _paxType = memHandle.create<PaxType>();
      _paxType->paxType() = "ADT";
      _totalNUCAmount = 200.0;
      _rexChangeFee = 190.0;
      _yqyrNUCAmount = 20.0;
      _bagChargeNUCAmount = 20.0;
      _dynamicPriceDeviationAmount = 30.0;
      _pricingUnit.push_back(memHandle.create<StubPricingUnit>(memHandle));
    }
  };

  class StubItin : public Itin
  {
  public:
    StubItin(TestMemHandle& memHandle) : Itin()
    {
      farePath().push_back(memHandle.create<StubFarePath>(memHandle));
      itinNum() = 1;
      calculationCurrency() = "USD";
    }
  };

  class StubPricingTrx : public PricingTrx
  {
  public:
    StubPricingTrx(TestMemHandle& memHandle) : PricingTrx()
    {
      _fareCalcConfig = memHandle.create<FareCalcConfig>();
      _itin.push_back(memHandle.create<StubItin>(memHandle));
      setTrxType(TrxType::MIP_TRX);
    }
  };

  Diag864Collector* _collector;
  Diagnostic* _diagroot;
  StubPricingTrx* _trx;
  TestMemHandle _memHandle;

public:
  virtual void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<StubPricingTrx>(_memHandle);
    _trx->diagnostic().diagnosticType() = Diagnostic864;
  }

  void tearDown() { _memHandle.clear(); }
};

TEST_F(Diag864CollectorTest, testPrintPriceDeviationResults)
{
  DiagManager diag(*_trx, Diagnostic864);
  ASSERT_TRUE(diag.isActive());
  try
  {
    _collector = dynamic_cast<Diag864Collector*>(&diag.collector());
  }
  catch (...)
  {
    ASSERT_TRUE(false);
  }
  std::string equals = "***************************************************************\n"
                       "ITIN: 1 REQUESTED PAXTYPE: ADT\n"
                       "  TOTAL AMOUNT: 200USD\n"
                       "  TOTAL SCORE: 400USD\n"
                       "  TOTAL PRICE DEVIATION: 30USD\n"
                       "  PU FU FM         FAREBASIS\n"
                       "  1  1  NYC-AA-LAX          \n"
                       "    FARE AMOUNT: 100USD\n"
                       "    MIN FARE PLUS UP: 120USD\n"
                       "    DIFFERENTIAL: 130USD\n"
                       "    SURCHARGE: 140USD\n"
                       "    TRANSFER SURCHARGE: 100USD\n"
                       "    STOPOVER SURCHARGE: 110USD\n"
                       "    PRICE DEVIATION: -50USD\n"
                       "***************************************************************\n";
  _collector->printPriceDeviationResults(*_trx);
  EXPECT_EQ(_collector->str(), equals);
}
}
