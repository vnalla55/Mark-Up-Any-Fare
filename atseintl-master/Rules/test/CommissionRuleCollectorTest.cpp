//-------------------------------------------------------------------
//
//  File:        CommissionRuleCollector.cpp
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
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/CollectedNegFareData.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/CommissionRuleCollector.h"
#include "Common/CommissionKeys.h"

using namespace std;
namespace tse
{

namespace amc
{

class CommissionRuleCollectorTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
    public:
      const std::vector<CommissionProgramInfo*>&
        getCommissionProgram(const VendorCode& vendor, ContractId contractId)
      {
        std::vector<CommissionProgramInfo*>* ret =
            _memHandle.create<std::vector<CommissionProgramInfo*> >();
        return *ret;
      }

      const std::vector<CommissionRuleInfo*>&
        getCommissionRule(const VendorCode& vendor, const uint64_t programId)
      {
        std::vector<CommissionRuleInfo*>* ret =
            _memHandle.create<std::vector<CommissionRuleInfo*> >();
        return *ret;
      }

    private:
      TestMemHandle _memHandle;
  };

  CPPUNIT_TEST_SUITE(CommissionRuleCollectorTest);

  CPPUNIT_TEST(testFmfcMathForDiag_DiagNotActive);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_NO_FM_FC_PrintCommissionFCHeader);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_FCMatch_DD_Not_Active);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_FCMatch_DD_Active);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_FC_NotMatch);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_FMMatch_DD_Active);
  CPPUNIT_TEST(testFmfcMathForDiag_DiagActive_FM_NotMatch);
  CPPUNIT_TEST(testDisplayContractIdInfoShort);
  CPPUNIT_TEST(testDisplayContractIdInfoFull);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    initTrx();
    _diag867 = _memHandle.create<Diag867Collector>();
    _diag867->initTrx(*_trx);
    _commissionRC = _memHandle.create<CommissionRuleCollector>(*_trx, _diag867, *_farePath, _vCProgs, _vCRules);
  }

  void tearDown() { _memHandle.clear(); }

  void testFmfcMathForDiag_DiagNotActive()
  {
    FareUsage fu;
    CPPUNIT_ASSERT(!_commissionRC->_diag867->isDiagFCMatchFail(fu));
  }

  void testFmfcMathForDiag_DiagActive()
  {
    createActivateDiagnostic();
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;
    CPPUNIT_ASSERT(!_commissionRC->_diag867->isDiagFCMatchFail(fu));
  }

  void testFmfcMathForDiag_DiagActive_NO_FM_FC_PrintCommissionFCHeader()
  {
    std::string expectedResult(" DEN - BA - LON  Y\n"
        "***************************************************************\n");

    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "AMC"));
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;

    CPPUNIT_ASSERT_EQUAL(bool(false), _commissionRC->_diag867->isDiagFCMatchFail(fu));

    _commissionRC->_diag867->printCommissionFC(fu);
    CPPUNIT_ASSERT_EQUAL(expectedResult, _commissionRC->_diag867->str());
  }

  void testFmfcMathForDiag_DiagActive_FCMatch_DD_Not_Active()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "Y"));
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;
    CPPUNIT_ASSERT(_commissionRC->_diag867->isDiagFareClassMatch(fu));
  }

  void testFmfcMathForDiag_DiagActive_FCMatch_DD_Active()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "Y"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "AMC"));
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;
    CPPUNIT_ASSERT(_commissionRC->_diag867->isDiagFareClassMatch(fu));
  }

  void testFmfcMathForDiag_DiagActive_FC_NotMatch()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "Y26"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "AMC"));
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;
    CPPUNIT_ASSERT(!_commissionRC->_diag867->isDiagFCMatchFail(fu));
  }

  void testFmfcMathForDiag_DiagActive_FMMatch_DD_Active()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DENLON"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    MockPaxTypeFare paxTfare;
    CPPUNIT_ASSERT(_commissionRC->_diag867->isDiagFareMarketMatch(*paxTfare.fareMarket()));
  }

  void testFmfcMathForDiag_DiagActive_FM_NotMatch()
  {
    createActivateDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DENDFW"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    FareUsage fu;
    MockPaxTypeFare paxTfare;
    fu.paxTypeFare() = &paxTfare;
    CPPUNIT_ASSERT(!_commissionRC->_diag867->isDiagFCMatchFail(fu));
  }

  void testDisplayContractIdInfoShort()
  {
    std::stringstream expectedDiag(
        "*-------------------------------------------------------------*\n"
        "CONTRACT CARRIER CODE: AA \n"
        "CONTRACT ID          : 88\n"
        "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
        " \n");

    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "AMC"));
    createActivateDiagnostic();
    createCommissionContractInfo();
    _commissionRC->_diag867->printDetailedCommissionContractInfo(*_cci);
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _commissionRC->_diag867->str());
  }

  void testDisplayContractIdInfoFull()
  {
    createActivateDiagnostic();
    createCommissionContractInfo();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));

    std::stringstream expectedDiag("*-------------------------------------------------------------*\n"
                                   "CONTRACT CARRIER CODE: AA \n"
                                   "CONTRACT ID          : 88\n"
                                   "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
                                   "CONTRACT VENDOR      : COS\n"
                                   "EFFECTIVE DATE TIME  : 11-NOV-2015  15.30.44\n"
                                   "EXPIRED DATE TIME    : 11-NOV-2016  23.45.12\n");

    _commissionRC->_diag867->printCommissionContractInfo(*_cci);
    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _commissionRC->_diag867->str());
  }

  void createCommissionContractInfo()
  {
    _memHandle.get(_cci);
    _cci->vendor() = "COS";
    _cci->effDateTime() = DateTime(2015, 11, 11, 15, 30, 44);
    _cci->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    _cci->description() = "COMMISSION Contract TESTING";
    _cci->contractId() = 88;
    _cci->carrier() = "AA ";
  }


private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  CommissionRuleCollector*   _commissionRC;
  Diag867Collector* _diag867;
  VCFMCommissionPrograms _vCProgs;
  VCFMPTFCommissionRules _vCRules;
  CommissionContractInfo* _cci;

  void initTrx()
  {
    _trx = _memHandle.create<PricingTrx>();
    _farePath = _memHandle.create<FarePath>();

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
    itin->farePath().push_back(_farePath);
    _farePath->itin() = itin;
    _trx->itin().push_back(itin);
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = "ADT";
    _farePath->paxType() = paxType;
    _farePath->collectedNegFareData() = _memHandle.create<CollectedNegFareData>();
    _farePath->collectedNegFareData()->comAmount() = 0;
    _farePath->collectedNegFareData()->comPercent() = RuleConst::PERCENT_NO_APPL;
  }

  class MockFareMarket : public FareMarket
  {
  public:
    MockFareMarket() : FareMarket()
    {
      _loc1.loc() = "DEN";
      _loc2.loc() = "LON";

      origin() = &_loc1;
      destination() = &_loc2;
    }

  protected:
    Loc _loc1;
    Loc _loc2;
  };

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare() : PaxTypeFare()
    {
      _tariffCrossRefInfo.ruleTariff() = 389;

      _fareInfo.vendor() = "ATP";
      _fareInfo.carrier() = "BA";
      _fareInfo.ruleNumber() = "JP01";
      _fareInfo.fareClass() = "Y";

      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);

      setFare(&_fare);
      fareMarket() = &_fareMarket;
    }

  protected:
    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    MockFareMarket _fareMarket;
  };

  void createActivateDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic867;
    _trx->diagnostic().activate();
    _commissionRC->_diag867 = _memHandle.insert(new Diag867Collector(_trx->diagnostic()));
    _commissionRC->_diag867->activate();
    _commissionRC->_diag867->initTrx(*_trx);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommissionRuleCollectorTest);
}
}
