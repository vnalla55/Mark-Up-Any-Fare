//----------------------------------------------------------------------------
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

#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Common/CommissionKeys.h"
#include "Rules/test/AgencyCommissionsTestUtil.h"

#include "test/include/GtestHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <gtest/gtest.h>

namespace tse
{
namespace amc
{
class FcCommissionRuleDataMock : public FcCommissionRuleData
{
  MoneyAmount convertCurrency(PricingTrx& trx,
                              const FarePath& fp,
                              MoneyAmount sourceAmount,
                              const CurrencyCode& sourceCurrency,
                              const CurrencyCode& targetCurrency,
                              bool doNonIataRounding) const
{
  return sourceAmount;
}
};

using namespace ::testing;

struct FcCommissionRuleDataTest : public Test
{
  //@todo remove most of following and move them to AgencyCommissions test
  PricingTrx* _trx;
  FarePath* _fp;
  FareUsage* _fareUsage;
  AirSeg* _airSeg;
  AirSeg* _airSeg2;
  Fare* _fare;
  PaxTypeFare* _paxTypeFare;
  FareMarket* _fareMarket;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tariffRefInfo;
  PaxType* _paxType;
  PaxTypeInfo* _pti;
  TestMemHandle _memHandle;
  FcCommissionRuleData _fccRd;
  FcCommissionRuleDataMock _fccRdM;

  void setSurchargeAmount(MoneyAmount m)
  {
    _fareUsage->surchargeAmt() = m;
  }

  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _fp = _memHandle.create<FarePath>();
    _fp->calculationCurrency() = "COS";
    _fp->baseFareCurrency() = "AMC";
    PricingRequest* request;
    _memHandle.get(request);
    PricingOptions* options;
    _memHandle.get(options);
    _trx->setOptions(options);
    Agent* agent;
    _memHandle.get(agent);
    request->ticketingAgent() = agent;
    agent->currencyCodeAgent() = "AMC";
    _trx->setRequest(request);

    // Travel Segment
    _airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJFK_DFW.xml");
    _airSeg2 = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml");

    // Fare Market
    _fareMarket = _memHandle.create<FareMarket>();
    _fareMarket->travelSeg().push_back(_airSeg);
    _fareMarket->travelSeg().push_back(_airSeg2);

    // Fare
    _pti = _memHandle.create<PaxTypeInfo>();

    _paxType = _memHandle.create<PaxType>();
    _paxType->paxType() = std::string("ADT");
    _paxType->paxTypeInfo() = _pti;

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->paxType() = _paxType;

    _tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    _tariffRefInfo->_fareTariff = 0;

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->_fareAmount = 1000.00;
    _fareInfo->_currency = "USD";
    _fareInfo->_fareTariff = 0;
    _fareInfo->_fareClass = "AA";

    _fare = _memHandle.create<Fare>();
    _fare->nucFareAmount() = 1000.00; // This is what PaxTypeFare returns
    _fare->initialize(Fare::FS_International, _fareInfo, *_fareMarket, _tariffRefInfo);

    // Fare Component
    _fareUsage = _memHandle.create<FareUsage>();
    _fareUsage->paxTypeFare() = _paxTypeFare;
    _fareUsage->paxTypeFare()->setFare(_fare);
    _fareUsage->travelSeg().push_back(_airSeg);
    _fareUsage->travelSeg().push_back(_airSeg2);
  }

  void TearDown() { _memHandle.clear(); }
};

TEST_F(FcCommissionRuleDataTest, ConstructorTest)
{
  FcCommissionRuleData fcCommRuleData;
  ASSERT_TRUE(fcCommRuleData.commRuleDataColPerCommType().empty());
}

TEST_F(FcCommissionRuleDataTest, TestInitRuleData)
{
  CommRuleDataColPerCT criColPerCt;

  FcCommissionRuleData fccRd;
  ASSERT_TRUE(fccRd.commRuleDataColPerCommType().empty());

  CommissionTypeId ct = 9;
  CommissionRuleInfo cri;

  CommissionProgramInfo cpi;
  CommissionContractInfo ccInfo;
  criColPerCt[ct].insert(CommissionRuleData(&cri, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_FALSE(fccRd.commRuleDataColPerCommType().empty());
}

TEST_F(FcCommissionRuleDataTest, TestIsFcHasHomogeneousCommissionType_Empty)
{
  CommRuleDataColPerCT criColPerCt;
  FcCommissionRuleData fccRd;
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_FALSE(fccRd.doesFcHaveCommissionType(CT9));
}

TEST_F(FcCommissionRuleDataTest, TestIsFcHasHomogeneousCommissionType_SameCT)
{
  CommRuleDataColPerCT criColPerCt;
  FcCommissionRuleData fccRd;

  // single obj
  CommissionRuleInfo cri1;
  CommissionProgramInfo cpi;
  CommissionContractInfo ccInfo;
  criColPerCt[CT9].insert(CommissionRuleData(&cri1, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_TRUE(fccRd.doesFcHaveCommissionType(CT9));

  // multiple obj of same CT
  CommissionRuleInfo cri2;
  criColPerCt[CT9].insert(CommissionRuleData(&cri2, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_TRUE(fccRd.doesFcHaveCommissionType(CT9));
}

TEST_F(FcCommissionRuleDataTest, TestIsFcHasHomogeneousCommissionType_MixedCT)
{
  CommRuleDataColPerCT criColPerCt;
  FcCommissionRuleData fccRd;
  ASSERT_TRUE(fccRd.commRuleDataColPerCommType().empty());

  // single obj
  CommissionRuleInfo cri1;
  CommissionProgramInfo cpi;
  CommissionContractInfo ccInfo;
  criColPerCt[CT9].insert(CommissionRuleData(&cri1, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_TRUE(fccRd.commRuleDataColPerCommType().size() == 1);
  ASSERT_TRUE(fccRd.doesFcHaveCommissionType(CT9));

  // multiple obj of same CT
  CommissionRuleInfo cri2;
  criColPerCt[CT10].insert(CommissionRuleData(&cri2, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_TRUE(fccRd.commRuleDataColPerCommType().size() == 2);
  ASSERT_TRUE(fccRd.doesFcHaveCommissionType(CT10));

  CommissionRuleInfo cri3;
  criColPerCt[CT11].insert(CommissionRuleData(&cri3, &cpi, &ccInfo));
  fccRd.commRuleDataColPerCommType() = criColPerCt;
  ASSERT_TRUE(fccRd.commRuleDataColPerCommType().size() == 3);
  ASSERT_TRUE(fccRd.doesFcHaveCommissionType(CT11));
}

} // amc
} //tse
