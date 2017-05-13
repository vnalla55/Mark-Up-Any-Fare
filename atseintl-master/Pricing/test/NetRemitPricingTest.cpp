//----------------------------------------------------------------------------
//  Copyright Sabre 2006
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

#include "Common/TSEException.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NegFareRest.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/NetRemitPricing.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class NetRemitPricingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NetRemitPricingTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcess_WithExceptionNoCat35);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentLocation() = _memHandle.create<Loc>();

    _farePath = _memHandle.create<FarePath>();
    _farePath->selectedNetRemitFareCombo() = true;
    _farePath->itin() = _memHandle.create<Itin>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu);
    FareUsage* fu = _memHandle.create<FareUsage>();
    pu->fareUsage().push_back(fu);
    _ptf = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare() = _ptf;
    FareMarket* fm = _memHandle.create<FareMarket>();
    _ptf->fareMarket() = fm;
    Fare* fare = _memHandle.create<Fare>();
    _ptf->setFare(fare);
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->setFareInfo(fareInfo);

    _nrPricing = _memHandle.insert(new NetRemitPricing(*_trx, *_farePath));
  }

  void tearDown() { _memHandle.clear(); }

  PaxTypeFare* createNegFare()
  {
    _ptf->status().set(PaxTypeFare::PTF_Negotiated);
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;

    ruleData->ruleItemInfo() = _memHandle.create<NegFareRest>();
    (*_ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

    return _ptf;
  }

  void testProcess()
  {
    createNegFare();

    CPPUNIT_ASSERT(_nrPricing->process());
  }

  void testProcess_WithExceptionNoCat35()
  {
    CPPUNIT_ASSERT_THROW(_nrPricing->process(), TSEException);
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  PaxTypeFare* _ptf;
  NetRemitPricing* _nrPricing;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetRemitPricingTest);
}
