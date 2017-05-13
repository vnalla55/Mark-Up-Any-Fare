
#include "test/include/CppUnitHelperMacros.h"



#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "Taxes/Common/TaxUtility.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
  class PaxTypeFareMock : public PaxTypeFare
  {
   public:
    void addRuleData(int cat, PaxTypeFareAllRuleData* allRuleData)
    {
       (*_paxTypeFareRuleDataMap)[cat] = allRuleData;
    }
  };

  class TaxUtility_USonYQYR : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(TaxUtility_USonYQYR);
    CPPUNIT_TEST( doUsTaxesApplyOnYQYR_pass );
    CPPUNIT_TEST( doUsTaxesApplyOnYQYR_onlyQ_pass );
    CPPUNIT_TEST( doUsTaxesApplyOnYQYR_wrongPartition );
    CPPUNIT_TEST( doUsTaxesApplyOnYQYR_notZeroFare );
    CPPUNIT_TEST( doUsTaxesApplyOnYQYR_emptyTD );
    CPPUNIT_TEST_SUITE_END();

    TestMemHandle _memHandle;
    PricingTrx* _trx;
    FarePath* _farePath;
    PricingUnit* _pu;
    FareUsage* _fu;
    PaxTypeFareMock* _ptf;
    FareByRuleApp* _fbra;
    FareByRuleItemInfo* _fbri;
    PricingRequest* _req;
    PaxTypeFare::PaxTypeFareAllRuleData* _ruleAllData;
    FBRPaxTypeFareRuleData* _ruleData;
    CategoryRuleItemInfo* _catRuleItemInfo;
    FareByRuleItemInfo* _ruleItemInfo;
    FareByRuleApp* _fbrApp;
    Agent* _agent;
    Fare* _fare;

  public:

    void
    setUp()
    {
      _memHandle.create<TestConfigInitializer>();
      _trx = _memHandle.create<PricingTrx>();
      _farePath = _memHandle.create<FarePath>();
      _pu = _memHandle.create<PricingUnit>();
      _fu = _memHandle.create<FareUsage>();
      _ptf = _memHandle.create<PaxTypeFareMock>();
      _req = _memHandle.create<PricingRequest>();
      _ruleAllData = _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
      _ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
      _catRuleItemInfo = _memHandle.create<CategoryRuleItemInfo>();
      _ruleItemInfo = _memHandle.create<FareByRuleItemInfo>();
      _agent = _memHandle.create<Agent>();
      _fbrApp = _memHandle.create<FareByRuleApp>();
      _fare = _memHandle.create<Fare>();

      _ptf->status().set(PaxTypeFare::PTF_FareByRule, true);
      _ruleAllData->fareRuleData = _ruleData;
      _ruleData->categoryRuleItemInfo() = _catRuleItemInfo;
      _ruleData->fbrApp() = _fbrApp;
      _ruleData->ruleItemInfo() = _ruleItemInfo;
      _fu->paxTypeFare() = _ptf;
      _trx->setRequest(_req);
      _req->ticketingAgent() = _agent;
      _ptf->addRuleData(25, _ruleAllData);
      _pu->fareUsage().push_back(_fu);
      _farePath->pricingUnit().push_back(_pu);
    }

    void
    tearDown()
    {
      _memHandle.clear();
    }

    void
    doUsTaxesApplyOnYQYR_pass()
    {
      Billing billing;
      billing.partitionID() = "AA";
      _trx->billing() = &billing;
      _fu->surchargeAmt() = 0;
      _ptf->fare()->nucFareAmount() = 0;
      _req->ticketingAgent()->tvlAgencyPCC() = "";
      _fbrApp->tktDesignator() = "X";
      _ruleItemInfo->tktDesignator()= "Y";

      CPPUNIT_ASSERT(taxUtil::doUsTaxesApplyOnYQYR(*_trx, *_farePath));
    }

    void
    doUsTaxesApplyOnYQYR_onlyQ_pass()
    {
      Billing billing;
      billing.partitionID() = "AA";
      _trx->billing() = &billing;
      _fu->surchargeAmt() = 108;
      _ptf->fare()->nucFareAmount() = 0;
      _req->ticketingAgent()->tvlAgencyPCC() = "";
      _fbrApp->tktDesignator() = "X";
      _ruleItemInfo->tktDesignator()= "Y";

      CPPUNIT_ASSERT(taxUtil::doUsTaxesApplyOnYQYR(*_trx, *_farePath));
    }

    void
    doUsTaxesApplyOnYQYR_wrongPartition()
    {
      Billing billing;
      billing.partitionID() = "CA";
      _trx->billing() = &billing;
      _fu->surchargeAmt() = 0;
      _ptf->fare()->nucFareAmount() = 0;
      _req->ticketingAgent()->tvlAgencyPCC() = "";
      _fbrApp->tktDesignator() = "X";
      _ruleItemInfo->tktDesignator()= "Y";
      CPPUNIT_ASSERT(!taxUtil::doUsTaxesApplyOnYQYR(*_trx, *_farePath));
    }

    void
    doUsTaxesApplyOnYQYR_notZeroFare()
    {
      Billing billing;
      billing.partitionID() = "AA";
      _trx->billing() = &billing;
      _fu->surchargeAmt() = 0;
      _ptf->fare()->nucFareAmount() = 108;
      _req->ticketingAgent()->tvlAgencyPCC() = "";
      _fbrApp->tktDesignator() = "X";
      _ruleItemInfo->tktDesignator()= "Y";
      CPPUNIT_ASSERT(!taxUtil::doUsTaxesApplyOnYQYR(*_trx, *_farePath));
    }

    void
    doUsTaxesApplyOnYQYR_emptyTD()
    {
      Billing billing;
      billing.partitionID() = "AA";
      _trx->billing() = &billing;
      _fu->surchargeAmt() = 0;
      _ptf->fare()->nucFareAmount() = 0;
      _req->ticketingAgent()->tvlAgencyPCC() = "";
      _fbrApp->tktDesignator() = "";
      _ruleItemInfo->tktDesignator()= "";
      CPPUNIT_ASSERT(!taxUtil::doUsTaxesApplyOnYQYR(*_trx, *_farePath));
    }

  };

  CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_USonYQYR);

}
;
