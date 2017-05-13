//-------------------------------------------------------------------
//
//  File:        AgencyCommissionsTest.cpp
//  Created:     2015
//  Authors:
//
//  Description:
//
//  Updates:
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
//-------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/GtestHelperMacros.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <cppunit/TestFixture.h>

#include "Common/CommissionKeys.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/AgencyCommissions.h"
#include "Rules/test/AgencyCommissionsTestUtil.h"


using namespace std;
namespace tse
{

namespace amc
{
  static const ContractId B4T0_AA_CONT_ID = 88;
  static const ContractId B4T0_BA_CONT_ID1 = 89;
  static const ContractId B4T0_BA_CONT_ID2 = 90;

class AgencyCommissionsTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
    public:
      void createCommissionContractInfo(
          CommissionContractInfo& info, const CarrierCode& carrier, ContractId contId)
      {
        info.vendor() = "COS";
        info.effDateTime() = DateTime(2015, 11, 11, 15, 30, 44);
        info.expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
        info.description() = "COMMISSION Contract TESTING";
        info.contractId() = contId;
        info.carrier() = carrier;
      }

      const std::vector<CommissionContractInfo*>&
        getCommissionContract(const VendorCode& vendor,
            const CarrierCode& carrier,
            const PseudoCityCode& pcc)
        {
          std::vector<CommissionContractInfo*>* ret =
            _memHandle.create<std::vector<CommissionContractInfo*> >();

          if (pcc == "B4T0" && carrier=="AA")
          {
            CommissionContractInfo* info = _memHandle.create<CommissionContractInfo>();
            info->sourcePCC() = "B4T0";
            createCommissionContractInfo(*info, "AA", B4T0_AA_CONT_ID);
            ret->push_back(info);
          }
          else if (pcc == "B4T0" && carrier=="BA")
          {
            CommissionContractInfo* info1 = _memHandle.create<CommissionContractInfo>();
            info1->sourcePCC() = "B4T0";
            createCommissionContractInfo(*info1, "BA", B4T0_BA_CONT_ID1);

            CommissionContractInfo* info2 = _memHandle.create<CommissionContractInfo>();
            info2->sourcePCC() = "B4T0";
            createCommissionContractInfo(*info2, "BA", B4T0_BA_CONT_ID2);

            ret->push_back(info1);
            ret->push_back(info2);
          }
          return *ret;
        }

      const std::vector<CustomerSecurityHandshakeInfo*>&
        getCustomerSecurityHandshake(
            const PseudoCityCode& pcc,
            const Code<8>& productCD,
            const DateTime& dateTime)
        {
          init();
          return _cshsInfoCol;
        }

    private:
      TestMemHandle _memHandle;
      std::vector<CustomerSecurityHandshakeInfo*> _cshsInfoCol;

      void init()
      {
        if (_cshsInfoCol.empty())
        {
          CustomerSecurityHandshakeInfo* c1 =
            _memHandle.create<CustomerSecurityHandshakeInfo>();
          c1->securitySourcePCC()="B4T0"; // Agent - branch, under HOMEPCC (customer)
          c1->securityTargetPCC()="B4T0"; // Rule creator, above homePCC (AMEX)
          _cshsInfoCol.push_back(c1);
        }
      }
  };

  CPPUNIT_TEST_SUITE(AgencyCommissionsTest);
  CPPUNIT_TEST(testGetContractIdForAgency_NotFound);
  CPPUNIT_TEST(testGetContractIdForAgency_NotFound_DiagMsg);
  CPPUNIT_TEST(testGetContractIdForAgency_FoundOneContract);
  CPPUNIT_TEST(testGetContractIdForAgency_FoundManyContract);
  CPPUNIT_TEST(test_getMaxCommAmountWithoutSurcharge_emptyTest);
  CPPUNIT_TEST(test_getMaxCommAmountWithoutSurcharge);
  CPPUNIT_TEST(test_getMaxCommAmountWithSurcharge_emptyTest);
  CPPUNIT_TEST(test_getMaxCommAmountWithSurcharge);
  CPPUNIT_TEST(test_getMaxCommAmount_emptyTest);
  CPPUNIT_TEST(test_getMaxCommAmount_withoutSurcharge);
  CPPUNIT_TEST(test_getMaxCommAmount_withSurcharge);
  //CPPUNIT_TEST(test_getNetTotalAmount);

  /** Begin: Using Single Contract ID **/
  CPPUNIT_TEST(test_calculateMaxCommForSingleFc_emptyTest);
  CPPUNIT_TEST(test_calculateMaxCommForSingleFc);

  CPPUNIT_TEST(test_calculateMaxCommForCT9_2FC_EmptySetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT9_2FC_CommonSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT9_2FC_DiffSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT9_2FC_DiffSetOfRulesWithSurcharge);
  CPPUNIT_TEST(test_calculateMaxCommForCT9_2FC_ShouldNotCombineWithNonCT9);

  CPPUNIT_TEST(test_calculateMaxCommForCT10_2FC_EmptySetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT10_2FC_CommonSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT10_2FC_DiffSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT10_2FC_DiffSetOfRulesWithSurcharge);
  CPPUNIT_TEST(test_calculateMaxCommForCT10_2FC_ShouldNotCombineWithNonCT10);

  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_EmptySetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_CommonSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_DiffSetOfRules);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_DiffSetOfRulesWithCT10);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_DiffSetOfRulesWithSurcharge);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_NonCommissionableFC);
  CPPUNIT_TEST(test_calculateMaxCommForCT11_2FC_NonCommissionableFC_nullptrCheck);

  CPPUNIT_TEST(test_calculateCommission_EmptyTest);
  CPPUNIT_TEST(test_calculateCommission_2FC_MaxCommission_CT9);
  CPPUNIT_TEST(test_calculateCommission_2FC_MaxCommission_CT10);
  CPPUNIT_TEST(test_calculateCommission_2FC_MaxCommission_CT11);
  CPPUNIT_TEST(test_calculateCommission_2FC_MaxCommission_Combining_CT10_CT11);

  CPPUNIT_TEST(test_calculateCommission_1FC_ValCxrWithMaxCommission);
  CPPUNIT_TEST(test_calculateCommission_2FC_ValCxrWithMaxCommission);
  CPPUNIT_TEST(test_calculateCommission_2FC_ValCxrWithMaxCommission_CT11_COMBINES_CT10);
  CPPUNIT_TEST(test_calculateCommission_2FC_ValCxrWithMaxCommission_CT11_DiffRuleSet);
  /** End: Using Single Contract ID **/

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _fp;
  FcCommissionInfoPairVec _fcAgcyCommDataCol;
  FareUsage* _fu1;
  FareUsage* _fu2;
  PricingUnit* _pu;
  AirSeg* _airSeg1;
  AirSeg* _airSeg2;
  Fare* _fare1;
  Fare* _fare2;
  PaxTypeFare* _paxTypeFare1;
  PaxTypeFare* _paxTypeFare2;
  FareMarket* _fm1;
  FareMarket* _fm2;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tariffRefInfo;
  PaxType* _paxType;
  PaxTypeInfo* _pti;

  AgencyCommissions* _agencyCommission;
  Diag867Collector* _diag867;
  MyDataHandle* _mdh;

public:
  void setSurchargeAmount(FareUsage& fu, MoneyAmount m)
  {
    MoneyAmount newAmount = fu.paxTypeFare()->fare()->nucFareAmount();
    fu.surchargeAmt() = m;
    newAmount += m;
    fu.paxTypeFare()->fare()->nucFareAmount() = newAmount;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    initTrx();
    _diag867 = _memHandle.create<Diag867Collector>();
    _agencyCommission = _memHandle.create<AgencyCommissions>(*_trx, _diag867);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void initTrx()
  {
    _trx = _memHandle.create<PricingTrx>();
    _fp = _memHandle.create<FarePath>();

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());

    _trx->billing() = _memHandle.create<Billing>();

    Customer* customer = _memHandle.create<Customer>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _trx->getRequest()->ticketingAgent() = agent;
    _trx->billing()->partitionID() = "F9";
    _trx->billing()->aaaCity() = "DFW0";

    _trx->diagnostic().diagnosticType() = Diagnostic867;
    _trx->diagnostic().activate();

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    itin->farePath().push_back(_fp);
    _fp->itin() = itin;
    _trx->itin().push_back(itin);

    _pti = _memHandle.create<PaxTypeInfo>();

    _paxType = _memHandle.create<PaxType>();
    _paxType->paxType() = std::string("ADT");
    _paxType->paxTypeInfo() = _pti;


    _fp->paxType() = _paxType;
    _fp->collectedNegFareData() = _memHandle.create<CollectedNegFareData>();
    _fp->collectedNegFareData()->comAmount() = 0;
    _fp->collectedNegFareData()->comPercent() = RuleConst::PERCENT_NO_APPL;

    // Fare Component
    _airSeg1 = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJFK_DFW.xml");
    _airSeg2 = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml");

    _paxTypeFare1 = _memHandle.create<PaxTypeFare>();
    _paxTypeFare1->paxType() = _paxType;

    _paxTypeFare2 = _memHandle.create<PaxTypeFare>();
    _paxTypeFare2->paxType() = _paxType;

    _tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    _tariffRefInfo->_fareTariff = 0;

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->_fareAmount = 1000.00;
    _fareInfo->_currency = "USD";
    _fareInfo->_fareTariff = 0;
    _fareInfo->_fareClass = "AA";

    _fm1 = _memHandle.create<FareMarket>();
    _fm1->travelSeg().push_back(_airSeg1);

    _fm2 = _memHandle.create<FareMarket>();
    _fm2->travelSeg().push_back(_airSeg2);

    _fare1 = _memHandle.create<Fare>();
    _fare1->nucFareAmount() = 2000.00; // This is what PaxTypeFare returns
    _fare1->initialize(Fare::FS_International, _fareInfo, *_fm1, _tariffRefInfo);

    _fare2 = _memHandle.create<Fare>();
    _fare2->nucFareAmount() = 1000.00; // This is what PaxTypeFare returns
    _fare2->initialize(Fare::FS_International, _fareInfo, *_fm2, _tariffRefInfo);

    _fu1 = _memHandle.create<FareUsage>();
    _fu1->paxTypeFare() = _paxTypeFare1;
    _fu1->paxTypeFare()->setFare(_fare1);
    _fu1->travelSeg().push_back(_airSeg1);
    _fu1->surchargeAmt() = 1000; // surcharge (cat12) amount

    _fu2 = _memHandle.create<FareUsage>();
    _fu2->paxTypeFare() = _paxTypeFare2;
    _fu2->paxTypeFare()->setFare(_fare2);
    _fu2->travelSeg().push_back(_airSeg2);

    _pu = _memHandle.create<PricingUnit>();
    _pu->fareUsage().push_back(_fu1);
    _pu->fareUsage().push_back(_fu2);

    _fp->pricingUnit().push_back(_pu);
  }

  void call_collectContractIdForAgency(
      const CarrierCode& valCxr,
      const PseudoCityCode& pcc,
      ContractIdCol& contractIdCol) const
  {
    std::vector<CustomerSecurityHandshakeInfo*> csHsInfoCol;
    CustomerSecurityHandshakeInfo c1;
    c1.securitySourcePCC() = "B4T0";
    c1.securityTargetPCC() = "B4T0";
    csHsInfoCol.push_back(&c1);
    _agencyCommission->collectContractIdForAgency(valCxr, pcc, csHsInfoCol, contractIdCol);
  }

  bool calculateMaxCommForSingleFc(
      const VCFMPTFCommissionRules& vcfmptfCommRules,
      MoneyAmount& maxAmt)
  {
    return _agencyCommission->calculateMaxCommForSingleFc(
        *_fp,
        vcfmptfCommRules,
        _fcAgcyCommDataCol,
        maxAmt);
  }

  bool calculateCommissionForCT9(const VCFMPTFCommissionRules& vcfmptfCommRules, MoneyAmount& maxAmt)
  {
    return _agencyCommission->calculateCommissionForCT9(
        *_fp,
        vcfmptfCommRules,
        _fcAgcyCommDataCol,
        maxAmt);
  }

  bool calculateCommissionForCT10(const VCFMPTFCommissionRules& vcfmptfCommRules, MoneyAmount& maxAmt)
  {
    return _agencyCommission->calculateCommissionForCT10(
        *_fp,
        vcfmptfCommRules,
        _fcAgcyCommDataCol,
        maxAmt);
  }

  bool calculateCommissionForCT11(const VCFMPTFCommissionRules& vcfmptfCommRules, MoneyAmount& maxAmt)
  {
    return _agencyCommission->calculateCommissionForCT11(
        *_fp,
        vcfmptfCommRules,
        _fcAgcyCommDataCol,
        maxAmt);
  }

  void testGetContractIdForAgency_NotFound()
  {
    ContractIdCol contractIdCol;
    call_collectContractIdForAgency("XX", "B4T0", contractIdCol);
    CPPUNIT_ASSERT(contractIdCol.empty());
  }

  void testGetContractIdForAgency_NotFound_DiagMsg()
  {
    CarrierCode valCxr = "XX";
    PseudoCityCode pcc = "B4T0";

    std::stringstream expectedDiag;
    expectedDiag << "AMC IS NOT APPLICABLE FOR CARRIER CODE "
      << valCxr << " / PCC " << pcc << "\n"
      << "REASON: NO CONTRACT FOUND\n"
      << " \n"
      << "***************************************************************\n";

    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    Diag867Collector diag867(_trx->diagnostic());
    diag867.initTrx(*_trx);
    diag867.activate();

    ContractIdCol contractIdCol;
    std::vector<CustomerSecurityHandshakeInfo*> csHsInfoCol;
    AgencyCommissions agencyComm(*_trx, &diag867);
    agencyComm.collectContractIdForAgency(valCxr, pcc, csHsInfoCol, contractIdCol);
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), agencyComm._diag867->str());
  }

  void testGetContractIdForAgency_FoundOneContract()
  {
    CarrierCode valCxr = "AA";
    PseudoCityCode pcc = "B4T0";

    ContractIdCol contractIdCol;
    call_collectContractIdForAgency(valCxr, pcc, contractIdCol);

    CPPUNIT_ASSERT(
        std::find_if(contractIdCol.begin(), contractIdCol.end(),
          [] (const CommissionContractInfo* ccInfo)->bool {
          return B4T0_AA_CONT_ID == ccInfo->contractId(); }) != contractIdCol.end());
  }

  void testGetContractIdForAgency_FoundManyContract()
  {
    CarrierCode valCxr = "BA";
    PseudoCityCode pcc = "B4T0";

    ContractIdCol contractIdCol;
    call_collectContractIdForAgency(valCxr, pcc, contractIdCol);

    CPPUNIT_ASSERT(contractIdCol.size()==2);
    CPPUNIT_ASSERT(
        std::find_if(contractIdCol.begin(), contractIdCol.end(),
          [] (const CommissionContractInfo* ccInfo)->bool {
          return B4T0_BA_CONT_ID1 == ccInfo->contractId(); }) != contractIdCol.end());

    CPPUNIT_ASSERT(
        std::find_if(contractIdCol.begin(), contractIdCol.end(),
          [] (const CommissionContractInfo* ccInfo)->bool {
          return B4T0_BA_CONT_ID2 == ccInfo->contractId(); }) != contractIdCol.end());
  }

  void test_getMaxCommAmountWithoutSurcharge_emptyTest()
  {
    CommissionRuleData selectedCrd;
    SortedCommRuleDataVec crdCol;
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(0),
        _agencyCommission->getMaxCommAmountWithoutSurcharge(*_fp, *_fu1, crdCol, selectedCrd));
  }

  void test_getMaxCommAmountWithoutSurcharge()
  {
    CommissionRuleData selectedCrd;
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, "9:3,20,30 - 10:10,14,20 - 11:10,6,3", ctCrdCol);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600),
        _agencyCommission->getMaxCommAmountWithoutSurcharge(*_fp, *_fu1, ctCrdCol[9], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(400),
        _agencyCommission->getMaxCommAmountWithoutSurcharge(*_fp, *_fu1, ctCrdCol[10], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200),
        _agencyCommission->getMaxCommAmountWithoutSurcharge(*_fp, *_fu1, ctCrdCol[11], selectedCrd));
  }

  void test_getMaxCommAmountWithSurcharge_emptyTest()
  {
    CommissionRuleData selectedCrd;
    SortedCommRuleDataVec crdCol;
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(0),
        _agencyCommission->getMaxCommAmountWithSurcharge(*_fp, *_fu1, crdCol, selectedCrd));
  }

  void test_getMaxCommAmountWithSurcharge()
  {
    CommissionRuleData selectedCrd;
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, "9:3,(20),30 - 10:10,14,20 - 11:10,(6),3", ctCrdCol);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600),
        _agencyCommission->getMaxCommAmountWithSurcharge(*_fp, *_fu1, ctCrdCol[9], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(400),
        _agencyCommission->getMaxCommAmountWithSurcharge(*_fp, *_fu1, ctCrdCol[10], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200),
        _agencyCommission->getMaxCommAmountWithSurcharge(*_fp, *_fu1, ctCrdCol[11], selectedCrd));
  }

  void test_getMaxCommAmount_emptyTest()
  {
    CommissionRuleData selectedCrd;
    SortedCommRuleDataVec crdCol;
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(0), _agencyCommission->getMaxCommAmount(*_fp, *_fu1, crdCol, selectedCrd));
  }

  void test_getMaxCommAmount_withoutSurcharge()
  {
    CommissionRuleData selectedCrd;
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, "9:3,20,30 - 10:10,14,20 - 11:10,6,3", ctCrdCol);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[9], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(400),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[10], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[11], selectedCrd));
  }

  void test_getMaxCommAmount_withSurcharge()
  {
    CommissionRuleData selectedCrd;
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, "9:3,(20),30 - 10:10,14,20 - 11:10,(6),3", ctCrdCol);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[9], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(400),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[10], selectedCrd));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200),
        _agencyCommission->getMaxCommAmount(*_fp, *_fu1, ctCrdCol[11], selectedCrd));
  }

  /*
   * void test_getNetTotalAmount()
  {
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1000),
        _agencyCommission->getNetTotalAmount(*_fp, *_fu1, false));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(2000),
        _agencyCommission->getNetTotalAmount(*_fp, *_fu1, true));
  }
  */

  void test_calculateMaxCommForSingleFc_emptyTest()
  {
    MoneyAmount maxAmt = 0;
    CommissionRuleData crd;
    VCFMPTFCommissionRules vcfmptfCommRules;
    CPPUNIT_ASSERT(!calculateMaxCommForSingleFc(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForSingleFc()
  {
    MoneyAmount maxAmt = 0;
    CommissionRuleData crd;
    VCFMPTFCommissionRules vcfmptfCommRules;

    CarrierCode valCxr("AA");
    CommissionRuleKey crKey(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, "9:3,(20),30", ctCrdCol);

    FcCommissionRuleData* fcRuleData = _memHandle.create<FcCommissionRuleData>();
    fcRuleData->commRuleDataColPerCommType() = ctCrdCol;

    ContractFcCommRuleDataMap contFcCommRuleCol;
    contFcCommRuleCol.insert(ContractFcCommRuleDataPair(B4T0_AA_CONT_ID, fcRuleData));
    vcfmptfCommRules.insert(VCFMPTFCommRulePair(crKey, contFcCommRuleCol));

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey));
    CPPUNIT_ASSERT(calculateMaxCommForSingleFc(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600), maxAmt);
  }

  /* Test: Two FCs. Both have same set of comm-rules
   * Results:
   *  1. Pick max comm-rules from each FC. Select least accross.
   *  2. Selected rule will be applied across. The Q-surcharge will be applied
   *     based on the rule selet in FC and not the least.
   */

  void setEmptyVCFMPTFCommissionRule(
      const CommissionRuleKey& crKey,
      VCFMPTFCommissionRules& vcfmptfCommRules)
  {
    ContractFcCommRuleDataMap contFcCommRuleCol;
    vcfmptfCommRules.insert(VCFMPTFCommRulePair(crKey, contFcCommRuleCol));
  }

  void setVCFMPTFCommissionRule(
      const CommissionRuleKey& crKey,
      const std::string& ruleSet,
      VCFMPTFCommissionRules& vcfmptfCommRules)
  {
    CommRuleDataColPerCT ctCrdCol;
    amcTestUtil::getRuleData(_memHandle, ruleSet, ctCrdCol);
    FcCommissionRuleData* fcRuleData = _memHandle.create<FcCommissionRuleData>();
    fcRuleData->commRuleDataColPerCommType() = ctCrdCol;

    ContractFcCommRuleDataMap contFcCommRuleCol;
    contFcCommRuleCol.insert(ContractFcCommRuleDataPair(B4T0_AA_CONT_ID, fcRuleData));
    vcfmptfCommRules.insert(VCFMPTFCommRulePair(crKey, contFcCommRuleCol));
  }

  void setVCFMPTFCommissionRules(
      const CommissionRuleKey& crKey,
      const std::string& ruleSet,
      VCFMPTFCommissionRules& vcfmptfCommRules)
  {
    ruleSet.empty() ?
      setEmptyVCFMPTFCommissionRule(crKey, vcfmptfCommRules):
      setVCFMPTFCommissionRule(crKey, ruleSet, vcfmptfCommRules);
  }

  void test_calculateMaxCommForCT9_2FC_EmptySetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CommissionRuleData crd;
    VCFMPTFCommissionRules vcfmptfCommRules;
    CPPUNIT_ASSERT(!calculateCommissionForCT9(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForCT9_2FC_CommonSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet("9:2, 3, 4");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(2 == vcfmptfCommRules.size());
    CPPUNIT_ASSERT(calculateCommissionForCT9(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(120), maxAmt);
  }

  void test_calculateMaxCommForCT9_2FC_DiffSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:2, 3, 4");
    const std::string ruleSet2("9:5, 7, 9");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT9(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(120), maxAmt);
  }

  void test_calculateMaxCommForCT9_2FC_DiffSetOfRulesWithSurcharge()
  {
    setSurchargeAmount(*_fu2, 1000); // 1000 + 1000 (q-surhcarge)

    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:2, 3, 4");
    const std::string ruleSet2("9:5, 7, (9)");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT9(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(160), maxAmt);
  }

  void test_calculateMaxCommForCT9_2FC_ShouldNotCombineWithNonCT9()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:2, 3, 4");
    const std::string ruleSet2("10:5, 7, (9)");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(!calculateCommissionForCT9(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForCT10_2FC_EmptySetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CommissionRuleData crd;
    VCFMPTFCommissionRules vcfmptfCommRules;
    CPPUNIT_ASSERT(!calculateCommissionForCT10(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForCT10_2FC_CommonSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet("10:2, 3, 4");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT10(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(120), maxAmt);
  }

  void test_calculateMaxCommForCT10_2FC_DiffSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("10:2, 3, 4");
    const std::string ruleSet2("10:4, 5, 6");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT10(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(140), maxAmt);
  }

  void test_calculateMaxCommForCT10_2FC_DiffSetOfRulesWithSurcharge()
  {
    setSurchargeAmount(*_fu2, 1000); // 1000 + 1000 (q-surhcarge)

    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("10:2, 3, (4)");
    const std::string ruleSet2("10:4, 5, (6)");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT10(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200), maxAmt);
  }

  void test_calculateMaxCommForCT10_2FC_ShouldNotCombineWithNonCT10()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("10:2, 3, (4)");
    const std::string ruleSet2("9:4, 5, (6)");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(!calculateCommissionForCT10(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForCT11_2FC_EmptySetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CommissionRuleData crd;
    VCFMPTFCommissionRules vcfmptfCommRules;
    CPPUNIT_ASSERT(!calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
  }

  void test_calculateMaxCommForCT11_2FC_CommonSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet("11:2, 3, 4");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(120), maxAmt);
  }

  void test_calculateMaxCommForCT11_2FC_DiffSetOfRules()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("11:2, 3, 4");
    const std::string ruleSet2("11:4, 5, 6");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(140), maxAmt);
  }

  void test_calculateMaxCommForCT11_2FC_DiffSetOfRulesWithCT10()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("11:2, 3, 4");
    const std::string ruleSet2("10:4, 5, 6");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(140), maxAmt);
  }

  void test_calculateMaxCommForCT11_2FC_DiffSetOfRulesWithSurcharge()
  {
    setSurchargeAmount(*_fu2, 1000); // 1000 + 1000 (q-surhcarge)

    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("11:2, 3, (4)");
    const std::string ruleSet2("11:4, 5, (6)");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200), maxAmt);
  }

  void test_calculateMaxCommForCT11_2FC_NonCommissionableFC()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    const std::string ruleSet1("11:2, 3, 4");
    const std::string ruleSet2;
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(80), maxAmt);
  }

  void test_calculateMaxCommForCT11_2FC_NonCommissionableFC_nullptrCheck()
  {
    MoneyAmount maxAmt = 0;
    CarrierCode valCxr("AA");
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    VCFMPTFCommissionRules vcfmptfCommRules;
    ContractFcCommRuleDataMap contFcCommRuleCol2;
    contFcCommRuleCol2.insert(ContractFcCommRuleDataPair(B4T0_AA_CONT_ID, nullptr));
    vcfmptfCommRules.insert(VCFMPTFCommRulePair(crKey2, contFcCommRuleCol2));

    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));
    CPPUNIT_ASSERT(!calculateCommissionForCT11(vcfmptfCommRules, maxAmt));
  }

  void test_calculateCommission_EmptyTest()
  {
    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    CPPUNIT_ASSERT(!_agencyCommission->calculateCommission(*_fp, commRulesPerKey));

    const CarrierCode valCxr("AA");
    VCFMPTFCommissionRules vcfmptfCommRules;
    commRulesPerKey[valCxr] = vcfmptfCommRules;
    CPPUNIT_ASSERT(!_agencyCommission->calculateCommission(*_fp, commRulesPerKey));
  }

  void test_calculateCommission_2FC_MaxCommission_CT9()
  {
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:30,20,3 - 10:20,14,10 - 11:10,6,3");
    const std::string ruleSet2("9:40,20,3 - 10:20,14,10 - 11:10,6,3");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));

    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    commRulesPerKey[valCxr] = vcfmptfCommRules;

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));
    auto it = _fp->valCxrCommissionAmount().find(valCxr);
    CPPUNIT_ASSERT(it != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(900), it->second);
  }

  void test_calculateCommission_2FC_MaxCommission_CT10()
  {
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:30,20,3 - 10:20,14,30 - 11:10,6,3");
    const std::string ruleSet2("9:40,20,3 - 10:20,14,40 - 11:10,6,3");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));

    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    commRulesPerKey[valCxr] = vcfmptfCommRules;

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));
    auto it = _fp->valCxrCommissionAmount().find(valCxr);
    CPPUNIT_ASSERT(it != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1000), it->second);
  }

  void test_calculateCommission_2FC_MaxCommission_CT11()
  {
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:30,20,3 - 10:20,14,10 - 11:10,6,30");
    const std::string ruleSet2("9:40,20,3 - 10:20,14,10 - 11:10,6,40");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));

    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    commRulesPerKey[valCxr] = vcfmptfCommRules;

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));
    auto it = _fp->valCxrCommissionAmount().find(valCxr);
    CPPUNIT_ASSERT(it != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1000), it->second);
  }

  void test_calculateCommission_2FC_MaxCommission_Combining_CT10_CT11()
  {
    CarrierCode valCxr("AA");
    const std::string ruleSet1("9:30,20,3 - 10:20,14,10 - 11:10,6,30");
    const std::string ruleSet2("9:40,20,3 - 10:20,50,10 - 11:10,6,40");
    CommissionRuleKey crKey1(valCxr, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    CommissionRuleKey crKey2(valCxr, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());

    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey1, ruleSet1, vcfmptfCommRules); //FC1
    setVCFMPTFCommissionRules(crKey2, ruleSet2, vcfmptfCommRules); //FC2

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, crKey2));

    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    commRulesPerKey[valCxr] = vcfmptfCommRules;

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));
    auto it = _fp->valCxrCommissionAmount().find(valCxr);
    CPPUNIT_ASSERT(it != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1100), it->second);
  }

  void setVCFMPTFCommRulesPerValCxr(
      const CarrierCode& valCxr,
      const CommissionRuleKey& crKey,
      const std::string& ruleSet,
      VCFMPTFCommRulesPerValCxr& commRulesPerKey)
  {
    VCFMPTFCommissionRules vcfmptfCommRules;
    setVCFMPTFCommissionRules(crKey, ruleSet, vcfmptfCommRules);
    auto it = commRulesPerKey.emplace(valCxr, vcfmptfCommRules);
    if (!it.second)
    {
      VCFMPTFCommissionRules& refRulesCol = it.first->second;
      refRulesCol.emplace(crKey, vcfmptfCommRules.begin()->second);
    }
  }

  void test_calculateCommission_1FC_ValCxrWithMaxCommission()
  {
    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    CarrierCode AA("AA"), BA("BA");

    const std::string ruleSet1("9:30,20,3 - 10:20,14,10 - 11:10,6,30");
    CommissionRuleKey crKey1(AA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, crKey1, ruleSet1, commRulesPerKey);

    const std::string ruleSet2("9:40,20,3 - 10:20,50,10 - 11:10,6,40");
    CommissionRuleKey crKey2(BA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, crKey2, ruleSet2, commRulesPerKey);

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey1));
    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, crKey2));

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));

    auto itAA = _fp->valCxrCommissionAmount().find(AA);
    CPPUNIT_ASSERT(itAA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(600), itAA->second);

    auto itBA = _fp->valCxrCommissionAmount().find(BA);
    CPPUNIT_ASSERT(itBA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1000), itBA->second);

    CPPUNIT_ASSERT(itBA->second > itAA->second);
  }

  // Both VaidatingCarrier sharing same contracts/programs/rules
  void test_calculateCommission_2FC_ValCxrWithMaxCommission()
  {
    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    CarrierCode AA("AA"), BA("BA");

    /** ValCxr: AA **/
    const std::string AA_ruleSet("9:30,20,3 - 10:20,14,10 - 11:40,10,6");
    // FC1
    CommissionRuleKey AA_FC1_crKey(AA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC1_crKey, AA_ruleSet, commRulesPerKey);

    // FC2
    CommissionRuleKey AA_FC2_crKey(AA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC2_crKey, AA_ruleSet, commRulesPerKey);

    /** ValCxr: BA **/
    const std::string BA_ruleSet("9:40,20,3 - 10:50,20,10 - 11:40,10,6");
    // FC1
    CommissionRuleKey BA_FC1_crKey(BA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC1_crKey, BA_ruleSet, commRulesPerKey);

    // FC2
    CommissionRuleKey BA_FC2_crKey(BA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC2_crKey, BA_ruleSet, commRulesPerKey);

    /*****/

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, AA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, AA_FC2_crKey));

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, BA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, BA_FC2_crKey));

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));

    auto itAA = _fp->valCxrCommissionAmount().find(AA);
    CPPUNIT_ASSERT(itAA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1200), itAA->second);

    auto itBA = _fp->valCxrCommissionAmount().find(BA);
    CPPUNIT_ASSERT(itBA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1500), itBA->second);

    CPPUNIT_ASSERT(itBA->second > itAA->second);
  }

  /**
   * ValidatingCarrier uses different contract/program/rules
   * CT11 combines with CT10 to get maximum commission
   */
  void test_calculateCommission_2FC_ValCxrWithMaxCommission_CT11_COMBINES_CT10()
  {
    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    CarrierCode AA("AA"), BA("BA");

    /** ValCxr: AA **/
    const std::string AA_ruleSet1("9:30,20,3 - 10:20,14,10 - 11:40,10,6");
    const std::string AA_ruleSet2("9:40,20,3 - 10:50,14,10 - 11:40,10,6");
    // FC1
    CommissionRuleKey AA_FC1_crKey(AA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC1_crKey, AA_ruleSet1, commRulesPerKey);

    // FC2
    CommissionRuleKey AA_FC2_crKey(AA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC2_crKey, AA_ruleSet2, commRulesPerKey);
    /*****/

    /** ValCxr: BA **/
    const std::string BA_ruleSet1("9:30,20,3 - 10:20,14,10 - 11:40,10,6");
    const std::string BA_ruleSet2("9:40,20,3 - 10:50,20,10 - 11:40,10,6");
    // FC1
    CommissionRuleKey BA_FC1_crKey(BA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC1_crKey, BA_ruleSet1, commRulesPerKey);

    // FC2
    CommissionRuleKey BA_FC2_crKey(BA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC2_crKey, BA_ruleSet2, commRulesPerKey);
    /*****/

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, AA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, AA_FC2_crKey));

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, BA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, BA_FC2_crKey));

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));

    auto itAA = _fp->valCxrCommissionAmount().find(AA);
    CPPUNIT_ASSERT(itAA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1300), itAA->second);

    auto itBA = _fp->valCxrCommissionAmount().find(BA);
    CPPUNIT_ASSERT(itBA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1300), itBA->second);

    CPPUNIT_ASSERT(itBA->second == itAA->second);
  }

  /**
   * CT11 combines with non commisionable FC giving maximum commissions
   */
  void test_calculateCommission_2FC_ValCxrWithMaxCommission_CT11_DiffRuleSet()
  {
    setSurchargeAmount(*_fu2, 1000); // 1000 + 1000 (q-surhcarge)

    VCFMPTFCommRulesPerValCxr commRulesPerKey;
    CarrierCode AA("AA"), BA("BA");

    /** ValCxr: AA **/
    const std::string AA_ruleSet1("9:20,10,3 - 10:20,14,10 - 11:60,10,6");
    const std::string AA_ruleSet2("9:30,10,3 - 10:30,14,10 - 11:30,10,6");
    // FC1
    CommissionRuleKey AA_FC1_crKey(AA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC1_crKey, AA_ruleSet1, commRulesPerKey);

    // FC2
    CommissionRuleKey AA_FC2_crKey(AA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(AA, AA_FC2_crKey, AA_ruleSet2, commRulesPerKey);
    /*****/

    /** ValCxr: BA **/
    const std::string BA_ruleSet1("9:20,10,3 - 10:20,14,10 - 11:(50),10,6");
    const std::string BA_ruleSet2("9:30,10,3 - 10:30,14,10 - 11:30,10,6");
    // FC1
    CommissionRuleKey BA_FC1_crKey(BA, _fu1->paxTypeFare()->fareMarket(), _fu1->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC1_crKey, BA_ruleSet1, commRulesPerKey);

    // FC2
    CommissionRuleKey BA_FC2_crKey(BA, _fu2->paxTypeFare()->fareMarket(), _fu2->paxTypeFare());
    setVCFMPTFCommRulesPerValCxr(BA, BA_FC2_crKey, BA_ruleSet2, commRulesPerKey);
    /*****/

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, AA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, AA_FC2_crKey));

    CPPUNIT_ASSERT(_fu1 == _agencyCommission->findFareUsage(*_fp, BA_FC1_crKey));
    CPPUNIT_ASSERT(_fu2 == _agencyCommission->findFareUsage(*_fp, BA_FC2_crKey));

    CPPUNIT_ASSERT(_agencyCommission->calculateCommission(*_fp, commRulesPerKey));

    auto itAA = _fp->valCxrCommissionAmount().find(AA);
    CPPUNIT_ASSERT(itAA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1800), itAA->second);

    auto itBA = _fp->valCxrCommissionAmount().find(BA);
    CPPUNIT_ASSERT(itBA != _fp->valCxrCommissionAmount().end());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1600), itBA->second);

    CPPUNIT_ASSERT(itAA->second > itBA->second);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AgencyCommissionsTest);
}
}
