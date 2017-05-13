
#include "Pricing/SimilarItin/Revalidator.h"

#include "DBAccess/FareInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

#include <vector>

namespace tse
{
namespace similaritin
{
class RoutingRevalidatorTest : public testing::Test
{
public:
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _pu = _memHandle.create<PricingUnit>();
    _fu = _memHandle.create<FareUsage>();
    _ptf = _memHandle.create<PaxTypeFare>();
    _fm = _memHandle.create<FareMarket>();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();

    _farePath->pricingUnit().push_back(_pu);
    _pu->fareUsage().push_back(_fu);
    _fu->paxTypeFare() = _ptf;
    _ptf->setFare(_fare);
    _ptf->fareMarket() = _fm;
    _fare->setFareInfo(_fareInfo);

    _farePathVec.push_back(_farePath);

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());

    _diag = _memHandle.create<DiagCollector>();
    _itin = _memHandle.create<Itin>();
    _context = _memHandle.create<Context>(*_trx, *_itin, *_diag);
    _noDiag = _memHandle.create<NoDiagnostic>();
    _revalidator = _memHandle.create<Revalidator<NoDiagnostic>>(*_context, *_noDiag);
  }

  void TearDown() override
  {
    _farePathVec.clear();
    _memHandle.clear();
  }

protected:
  TestMemHandle _memHandle;
  Context* _context;
  DiagCollector* _diag;
  Fare* _fare;
  FareMarket* _fm;
  FareInfo* _fareInfo;
  FarePath* _farePath;
  FareUsage* _fu;
  Itin* _itin;
  NoDiagnostic* _noDiag;
  PaxTypeFare* _ptf;
  PricingTrx* _trx;
  PricingUnit* _pu;
  std::vector<FarePath*> _farePathVec;

  Revalidator<NoDiagnostic>* _revalidator;

  bool callRouting() const
  {
    return _revalidator->validateRouting(_farePathVec);
  }
};

TEST_F(RoutingRevalidatorTest, specialRouting)
{
  _fareInfo->routingNumber() = CAT25_DOMESTIC;

  ASSERT_TRUE(callRouting());
}

TEST_F(RoutingRevalidatorTest, passed)
{
  ASSERT_TRUE(callRouting());
}

TEST_F(RoutingRevalidatorTest, restorePTFState)
{
  _ptf->setRoutingProcessed(false);
  _ptf->setRoutingValid(false);
  AirSeg ts1;
  AirSeg ts2;
  _fm->travelSeg().push_back(&ts1);
  _fu->travelSeg().push_back(&ts2);

  ASSERT_TRUE(callRouting());
  ASSERT_FALSE(_ptf->isRoutingProcessed());
  ASSERT_FALSE(_ptf->isRoutingValid());
  ASSERT_EQ(_fm->travelSeg().size(), 1u);
  ASSERT_EQ(_fm->travelSeg().front(), &ts1);
  ASSERT_EQ(_fu->travelSeg().size(), 1u);
  ASSERT_EQ(_fu->travelSeg().front(), &ts2);
}
}
}
