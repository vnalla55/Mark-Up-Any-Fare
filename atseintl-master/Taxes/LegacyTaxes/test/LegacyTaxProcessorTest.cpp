//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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
#include "test/include/GtestHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include "Taxes/LegacyTaxes/LegacyTaxProcessor.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

static constexpr bool MultiSPTrue = true;
static constexpr bool MultiSPFalse = false;

class MyDataHandle : public DataHandleMock
{
public:
  TestMemHandle _memHandle;
  std::vector<TaxReissue*> _taxReissue;

  const std::vector<TaxReissue*>& getTaxReissue(const TaxCode& taxCode, const DateTime& date)
  {
    _taxReissue.clear();
    TaxReissue* reis = _memHandle.create<TaxReissue>();
    reis->taxType() = TaxReissueSelector::LEGACY_TAXES_TAX_TYPE;

    if (taxCode == "US1")
    {
      _taxReissue.push_back(reis);
    }
    else if (taxCode == "DU1")
    {
      reis->refundInd() = 'N';
      _taxReissue.push_back(reis);
    }
    else if (taxCode == "C33")
    {
      reis->cat33onlyInd() = 'Y';
      reis->refundInd() = 'N';
      _taxReissue.push_back(reis);
    }

    return _taxReissue;
  }
};

class MockLegacyTaxProcessor : public LegacyTaxProcessor
{
public:
  MockLegacyTaxProcessor(const TseThreadingConst::TaskId taskId, Logger& logger) :
    LegacyTaxProcessor(taskId, logger) {}

  void handleAdjustedSellingLevelFarePaths(PricingTrx& trx)
  {
    LegacyTaxProcessor::handleAdjustedSellingLevelFarePaths(trx);
  }

  void processAdjustedSellingLevelFarePath(PricingTrx& trx, FarePath& fp) const
  {
    fp.processed() = true;
    fp.adjustedSellingFarePath()->processed() = true;
  }
};

struct LegacyTaxProcessorTest : public Test
{
  typedef std::map<CarrierCode, MoneyAmount> CxrPriceMap;
  typedef std::map< MoneyAmount, std::vector<CarrierCode>, compareMoneyAmount> PriceCxrsMap;

  TestMemHandle _memHandle;
  MyDataHandle* _mdh;
  Itin* _itin;

  PaxType* _paxType1;
  PaxType* _paxType2;
  PaxType* _paxType3;
  PaxType* _paxType4;

  FarePath* _farePath1;
  FarePath* _farePath2;
  FarePath* _farePath3;
  FarePath* _farePath4;

  TaxResponse* _taxResp1;
  TaxResponse* _taxResp2;
  TaxResponse* _taxResp3;
  TaxResponse* _taxResp4;

  TaxRecord* _taxRec1;
  TaxRecord* _taxRec2;
  TaxRecord* _taxRec3;
  TaxRecord* _taxRec4;

  PricingTrx* _trx;
  LegacyTaxProcessor* _ltp;
  const Loc* _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

  std::map<CarrierCode, TaxResponse*> _cxrTaxResp;
  void SetUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());

    _memHandle.create<TestConfigInitializer>();

    _mdh = _memHandle.create<MyDataHandle>();

    _itin = _memHandle.create<Itin>();

    _farePath1 = _memHandle.create<FarePath>();
    _farePath2 = _memHandle.create<FarePath>();
    _farePath3 = _memHandle.create<FarePath>();
    _farePath4 = _memHandle.create<FarePath>();

    _farePath1->processed()=true;
    _farePath2->processed()=true;
    _farePath3->processed()=true;
    _farePath4->processed()=true;

    _farePath1->itin()=_itin;
    _farePath2->itin()=_itin;
    _farePath3->itin()=_itin;
    _farePath4->itin()=_itin;

    _paxType1 = _memHandle.create<PaxType>();
    _paxType1->paxType() = "ADT";
    _farePath1->paxType()=_paxType1;

    _paxType2 = _memHandle.create<PaxType>();
    _paxType2->paxType() = "ADT";
    _farePath2->paxType()=_paxType2;

    _paxType3 = _memHandle.create<PaxType>();
    _paxType3->paxType() = "ADT";
    _farePath3->paxType()=_paxType3;

    _paxType4 = _memHandle.create<PaxType>();
    _paxType4->paxType() = "ADT";
    _farePath4->paxType()=_paxType4;

    _taxResp1 = _memHandle.create<TaxResponse>();
    _taxResp2 = _memHandle.create<TaxResponse>();
    _taxResp3 = _memHandle.create<TaxResponse>();
    _taxResp4 = _memHandle.create<TaxResponse>();

    _taxRec1 = _memHandle.create<TaxRecord>();
    _taxRec2 = _memHandle.create<TaxRecord>();
    _taxRec3 = _memHandle.create<TaxRecord>();
    _taxRec4 = _memHandle.create<TaxRecord>();

    _taxRec1->setTaxCode("LL");
    _taxRec2->setTaxCode("LL");
    _taxRec3->setTaxCode("LL");
    _taxRec4->setTaxCode("LL");

    _taxRec1->setTaxAmount(0.00);
    _taxRec2->setTaxAmount(0.00);
    _taxRec3->setTaxAmount(0.00);
    _taxRec4->setTaxAmount(0.00);

    _taxResp1->taxRecordVector().push_back(_taxRec1);
    _taxResp2->taxRecordVector().push_back(_taxRec2);
    _taxResp3->taxRecordVector().push_back(_taxRec3);
    _taxResp4->taxRecordVector().push_back(_taxRec4);

    _itin->farePath().push_back(_farePath1);
    _itin->farePath().push_back(_farePath2);
    _itin->farePath().push_back(_farePath3);
    _itin->farePath().push_back(_farePath4);

    std::string name="LegacyTaxProcesstorTest";
    Logger logger("atseintl.Taxes.LegacyTaxes.LegacyTaxProcessorTest");
    _ltp = _memHandle.create<LegacyTaxProcessor>(TseThreadingConst::SYNCHRONOUS_TASK, logger);
  }

  void TearDown()
  {
  }

  TaxResponse* getTaxResponse(MoneyAmount taxAmt=0.00,
      std::initializer_list<std::string> list={})
  {
    TaxResponse* tresp = _memHandle.create<TaxResponse>();
    TaxRecord* trecord = _memHandle.create<TaxRecord>();
    trecord->setTaxCode("LL");
    trecord->setTaxAmount(taxAmt);
    tresp->taxRecordVector().push_back(trecord);

    for (const SettlementPlanType& sp : list)
      tresp->settlementPlans().push_back(sp);
    return tresp;
  }

  void setFarePath(FarePath& farePath,
      const PaxTypeCode& paxType,
      const CarrierCode& defValCxr,
      MoneyAmount totalNUCAmt=100.0,
      MoneyAmount taxAmt=10.0,
      bool isMultiSp = false,
      std::initializer_list<std::string> valCxrList={})
  {
    if (farePath.paxType())
      farePath.paxType()->paxType() = paxType;

    farePath.defaultValidatingCarrier() = defValCxr;
    farePath.itin()->validatingCarrier() = defValCxr;
    farePath.setTotalNUCAmount(totalNUCAmt + taxAmt);

    for (const CarrierCode& cxr : valCxrList)
      if (defValCxr != cxr) // alternates
        farePath.validatingCarriers().push_back(cxr);
  }

  void setTaxResponse(FarePath& farePath,
      TaxResponse& taxResp,
      const PaxTypeCode& paxType,
      const CarrierCode& cxr,
      MoneyAmount taxAmt,
      std::initializer_list<std::string> spList={},
      bool isMultiSp = false)
  {
    taxResp.validatingCarrier() = cxr;
    taxResp.taxRecordVector()[0]->setTaxAmount(taxAmt);
    taxResp.paxTypeCode() = paxType;
    taxResp.farePath() = &farePath;

    for (const std::string& sp : spList)
      if (!sp.empty())
        taxResp.settlementPlans().push_back(sp);

    if (isMultiSp)
      farePath.setValCxrTaxResponses(cxr, &taxResp);
    else
      farePath.setValCxrTaxResponse(cxr, &taxResp);

    farePath.itin()->mutableTaxResponses().push_back(&taxResp);
    farePath.itin()->valCxrTaxResponses().push_back(&taxResp);
  }

  void parseTaxComponent(const std::string& s, MoneyAmount& m, TaxCode& tc)
  {
    size_t ind = s.find(":");
    assert(ind != std::string::npos);

    tc = s.substr(0, ind);
    m = atof(s.substr(ind+1).c_str());
  }

  void setTaxComponents(TaxResponse& taxResp,
      const std::initializer_list<std::string> sList)
  {
    taxResp.taxRecordVector().clear();
    for (const std::string& s : sList)
    {
      MoneyAmount m;
      TaxCode tc;
      parseTaxComponent(s, m, tc);
      TaxRecord* taxRec = _memHandle.create<TaxRecord>();
      taxRec->setTaxCode(tc);
      taxRec->setTaxAmount(m);
      taxResp.taxRecordVector().push_back(taxRec);
    }
  }

  void setSpValCxrs(SettlementPlanValCxrsMap& spValCxrs,
      const SettlementPlanType& sp,
      std::initializer_list<std::string> cxrList)
  {
    for(const CarrierCode& cxr : cxrList)
      spValCxrs[sp].push_back(cxr);
  }

  void call_findValidatingCxrWithLowestTotal2(
      PricingTrx& trx,
      FarePath& farePath,
      std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
      std::set<FarePath*>& fpSet,
      MoneyAmount& currentLowTotal)
  {
    _ltp->findValCxrWithLowestTotal(
        trx, farePath, valCxrWithLowestTotalAmt, fpSet, currentLowTotal);
  }

  void call_getValidValCxr(PricingTrx& trx, FarePath& fp, std::vector<CarrierCode>& vcVect)
  {
    _ltp->getValidValCxr(trx, fp, vcVect);
  }

  bool call_suppressSettlementPlanGTC(
    PricingTrx& trx,
    const SettlementPlanType& sp,
    size_t numOfSpInFinalSolution) const
  {
    return _ltp->suppressSettlementPlanGTC(trx, sp, numOfSpInFinalSolution);
  }

  bool call_isFinalSettlementPlanInTaxResponse(
      TaxResponse& taxResp,
      const SettlementPlanValCxrsMap& spValCxrs) const
  {
    return _ltp->isFinalSettlementPlanInTaxResponse(taxResp, spValCxrs);
  }

  void call_modifyItinTaxResponseForFinalFarePath(FarePath& finalFp) const
  {
    _ltp->modifyItinTaxResponseForFinalFarePath(finalFp);
  }

  void call_setSpValCxrsMap(
    const CarrierCode& cxr,
    const std::vector<SettlementPlanTaxAmount>& spTaxAmountCol,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
  {
    _ltp->setSpValCxrsMap(cxr, spTaxAmountCol, spValCxrsWithLowestTotalAmt);
  }

  void call_clearSettlementPlanData(
    std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const SettlementPlanType& higherSp) const
  {
    _ltp->clearSettlementPlanData(currentSpTaxAmountCol, higherSp);
  }

  bool call_checkSettlementPlanHierarchy(
    const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol,
    SettlementPlanType& higherSp) const
  {
    return _ltp->checkSettlementPlanHierarchy(
        currentSpTaxAmountCol,
        nextSpTaxAmountCol,
        higherSp);
  }

  bool call_checkSpIndex(
    const TaxResponse& taxResp,
    const SettlementPlanType& sp,
    size_t& ind) const
  {
    return _ltp->checkSpIndex(taxResp, sp, ind);
  }

  bool call_canMergeTaxResponses(
    const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const
  {
    return _ltp->canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol);
  }

  void call_checkTaxComponentsAndMerge(
      std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
      const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const
  {
    _ltp->checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  }

  MoneyAmount call_getSpTotalAmount(const std::vector<SettlementPlanTaxAmount>& spTaxAmtCol) const
  {
    return _ltp->getSpTotalAmount(spTaxAmtCol);
  }

  void call_findSpTotalAmount(
    const FarePath& farePath,
    std::vector<TaxResponse*>& taxResponses,
    SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const
  {
    _ltp->findSpTotalAmount(farePath, taxResponses, spTaxAmtGroup);
  }

  void call_processSettlementPlanTaxData(
    MoneyAmount& currentLowestTotal,
    std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
    const SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const
  {
    _ltp->processSettlementPlanTaxData(currentLowestTotal,
        currentSpTaxAmountCol,
        spTaxAmtGroup);
  }

  void call_processValidatingCxrTotalAmount(
    const std::map<CarrierCode, ValidatingCxrTotalAmount>& valCxrTotalAmtPerCxr,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const
  {
    _ltp->processValidatingCxrTotalAmount(valCxrTotalAmtPerCxr, spValCxrsWithLowestTotalAmt);
  }

  void call_addTaxInfoResponsesToTrx(PricingTrx& trx)
  {
    _ltp->addTaxInfoResponsesToTrx(trx);
  }

  void call_handleValidatingCarrierError(PricingTrx& trx)
  {
    _ltp->handleValidatingCarrierError(trx);
  }

  void call_processAdjustedSellingLevelFarePath(PricingTrx& trx, FarePath* fp)
  {
    _ltp->processAdjustedSellingLevelFarePath(trx, *fp);
  }

  Agent* createAgent(bool isMultiSp = false)
  {
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = _locDFW;
    agent->agentCity() = "DFW";
    agent->tvlAgencyPCC() = "HDQ";
    agent->mainTvlAgencyPCC() = "HDQ";
    agent->tvlAgencyIATA() = "XYZ";
    agent->homeAgencyIATA() = "XYZ";
    agent->agentFunctions() = "XYZ";
    agent->agentDuty() = "XYZ";
    agent->airlineDept() = "XYZ";
    agent->cxrCode() = "AA";
    agent->currencyCodeAgent() = "USD";
    agent->coHostID() = 9;
    agent->agentCommissionType() = "PERCENT";
    agent->agentCommissionAmount() = 10;

    Customer* customer = _memHandle.create<Customer>();
    agent->agentTJR() = customer;

    return agent;
  }
};

TEST_F(LegacyTaxProcessorTest, FindValCxrWith2ADTLowestTotalAmount) {
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 0);
  setFarePath(*_farePath1, "ADT", "9W", 100, 0, MultiSPFalse, {"9W", "LH"}); // lower

  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "LH", 10);
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPFalse, {"9W", "LH"});

  std::set<FarePath*> fpSet;
  MoneyAmount currentLowTotal = std::numeric_limits<MoneyAmount>::max();
  std::vector<CarrierCode> valCxrWithLowestTotalAmt;

  call_findValidatingCxrWithLowestTotal2(*_trx,
      *_farePath1,
      valCxrWithLowestTotalAmt,
      fpSet,
      currentLowTotal);

  EXPECT_EQ("9W", valCxrWithLowestTotalAmt.front());
}

TEST_F(LegacyTaxProcessorTest, FindValCxrWith2ADTWithSameTotalAmount) {

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "LH", 10);
  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "9W", 10);
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPFalse, {"9W", "LH"}); // lower

  std::set<FarePath*> fpSet;
  MoneyAmount currentLowTotal = std::numeric_limits<MoneyAmount>::max();
  std::vector<CarrierCode> valCxrWithLowestTotalAmt;

  call_findValidatingCxrWithLowestTotal2(*_trx,
      *_farePath1,
      valCxrWithLowestTotalAmt,
      fpSet,
      currentLowTotal);

  ASSERT_TRUE(valCxrWithLowestTotalAmt.size()==2);
}

TEST_F(LegacyTaxProcessorTest, GetValidValCxrTest) {
  // Empty Test
  std::vector<CarrierCode> vcVect;
  call_getValidValCxr(*_trx, *_farePath1, vcVect);
  EXPECT_EQ(0, vcVect.size());

  // Non empty test
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);

  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"BSP"}, MultiSPTrue);
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);

  setTaxResponse(*_farePath3, *_taxResp3, "ADT", "BA", 10, {"BSP"}, MultiSPTrue);
  setFarePath(*_farePath3, "ADT", "9W", 100, 10, MultiSPTrue);

  vcVect.clear();
  call_getValidValCxr(*_trx, *_farePath1, vcVect);
  EXPECT_EQ(1, vcVect.size());

  vcVect.clear();
  call_getValidValCxr(*_trx, *_farePath2, vcVect);
  EXPECT_EQ(1, vcVect.size());

  // Gsa cloned (Tagged Fp) test
  _farePath1->validatingCarriers().push_back("AA");
  _farePath1->validatingCarriers().push_back("BA");
  _farePath1->gsaClonedFarePaths().push_back(_farePath2);
  _farePath1->gsaClonedFarePaths().push_back(_farePath3);

  ASSERT_TRUE(_farePath1->gsaClonedFarePaths().size()==2);

  vcVect.clear();
  call_getValidValCxr(*_trx, *_farePath1, vcVect);
  EXPECT_EQ(3, vcVect.size());
}

TEST_F(LegacyTaxProcessorTest, TestSuppressSettlementPlanGTC) {
  _trx->getRequest()->ticketingAgent() = createAgent();
  _trx->getRequest()->ticketingAgent()->agentTJR()->settlementPlans()="";
  ASSERT_FALSE(call_suppressSettlementPlanGTC(*_trx, "GTC", 1));

  _trx->getRequest()->ticketingAgent()->agentTJR()->settlementPlans()="GTC";
  ASSERT_FALSE(call_suppressSettlementPlanGTC(*_trx, "GTC", 1));

  _trx->getRequest()->ticketingAgent()->agentTJR()->settlementPlans()="BSPGEN";
  ASSERT_FALSE(call_suppressSettlementPlanGTC(*_trx, "BSP", 2));

  // still false
  _trx->getRequest()->ticketingAgent()->agentTJR()->settlementPlans()="BSPGTC";
  ASSERT_FALSE(call_suppressSettlementPlanGTC(*_trx, "GTC", 2));

  // only true
  _trx->getRequest()->ticketingAgent()->agentTJR()->settlementPlans()="";
  ASSERT_TRUE(call_suppressSettlementPlanGTC(*_trx, "GTC", 2));
}

TEST_F(LegacyTaxProcessorTest, TestIsFinalSettlementPlanInTaxResponse) {
  SettlementPlanValCxrsMap spValCxrs;
  TaxResponse* taxResp = getTaxResponse(100.00, {"BSP"});
  ASSERT_FALSE(call_isFinalSettlementPlanInTaxResponse(*taxResp, spValCxrs));

  setSpValCxrs(spValCxrs, "BSP", {"AA", "BA"});
  setSpValCxrs(spValCxrs, "TCH", {"BA"});
  ASSERT_TRUE(call_isFinalSettlementPlanInTaxResponse(*taxResp, spValCxrs));
}

TEST_F(LegacyTaxProcessorTest, TestModifyItinTaxResponseForFinalFarePath) {
  // AA => BSP, GTC and TAX: AA => TCH
  setSpValCxrs(_farePath1->settlementPlanValidatingCxrs(), "BSP", {"AA"});
  setSpValCxrs(_farePath1->settlementPlanValidatingCxrs(), "GTC", {"AA"});
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"TCH"}, MultiSPTrue);
  _farePath1->itin()->mutableTaxResponses().push_back(_taxResp1);
  call_modifyItinTaxResponseForFinalFarePath(*_farePath1);
  ASSERT_EQ(size_t(0), _farePath1->itin()->mutableTaxResponses().size());

  // AA => {BSP, TCH} and TAX: AA => {AAA, BBB}
  setSpValCxrs(_farePath2->settlementPlanValidatingCxrs(), "BSP", {"AA"});
  setSpValCxrs(_farePath2->settlementPlanValidatingCxrs(), "TCH", {"AA"});
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"AAA"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp3, "ADT", "BA", 20, {"BBB"}, MultiSPTrue);
  ASSERT_TRUE(2==_farePath2->itin()->mutableTaxResponses().size());
  call_modifyItinTaxResponseForFinalFarePath(*_farePath2);
  ASSERT_EQ(size_t(0), _farePath2->itin()->mutableTaxResponses().size());

  // AA => BSP, GTC and TAX: AA=> BSP, GTC
  setSpValCxrs(_farePath3->settlementPlanValidatingCxrs(), "BSP", {"AA"});
  setSpValCxrs(_farePath3->settlementPlanValidatingCxrs(), "GTC", {"AA"});
  setTaxResponse(*_farePath3, *_taxResp3, "ADT", "AA", 10, {"BSP", "GTC"}, MultiSPTrue);
  call_modifyItinTaxResponseForFinalFarePath(*_farePath3);
  ASSERT_EQ(size_t(1), _farePath3->itin()->mutableTaxResponses().size());
}

TEST_F(LegacyTaxProcessorTest, TestSetSpValCxrsMap_NullAndEmptyTest) {
  // empty test
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;
  call_setSpValCxrsMap("", spTaxAmountCol, spValCxrsWithLowestTotalAmt);
  ASSERT_TRUE(spValCxrsWithLowestTotalAmt.empty());

  // null test
  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0),
                          spTaxAmount2(nullptr, 0.0),
                          spTaxAmount3(nullptr, 0.0),
                          spTaxAmount4(nullptr, 0.0);
  spTaxAmountCol.push_back(spTaxAmount1);
  spTaxAmountCol.push_back(spTaxAmount2);
  spValCxrsWithLowestTotalAmt.clear();
  call_setSpValCxrsMap("AA", spTaxAmountCol, spValCxrsWithLowestTotalAmt);
  ASSERT_TRUE(spValCxrsWithLowestTotalAmt.empty());
}

TEST_F(LegacyTaxProcessorTest, TestSetSpValCxrsMap_SameSPSingleValCxrTest) {
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0);
  spTaxAmount1.taxResponse = _taxResp1;
  spTaxAmount1.totalAmount = 20;

  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;
  ValidatingCxrTotalAmount valCxrTotalAmt;
  valCxrTotalAmt.spTaxAmountCol.push_back(spTaxAmount1);
  valCxrTotalAmtPerCxr["AA"] = valCxrTotalAmt;

  for (const auto& it : valCxrTotalAmtPerCxr)
    call_setSpValCxrsMap(it.first, it.second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);

  ASSERT_EQ(size_t(1), spValCxrsWithLowestTotalAmt.size());
}

TEST_F(LegacyTaxProcessorTest, TestSetSpValCxrsMap_SameSPMultipleValCxr) {
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "BA", 20, {"BSP"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0);
  SettlementPlanTaxAmount spTaxAmount2(nullptr, 0.0);

  spTaxAmount1.taxResponse = _taxResp1;
  spTaxAmount2.taxResponse = _taxResp2;

  ValidatingCxrTotalAmount valCxrTotalAmt1, valCxrTotalAmt2;
  valCxrTotalAmt1.spTaxAmountCol.push_back(spTaxAmount1);
  valCxrTotalAmt2.spTaxAmountCol.push_back(spTaxAmount2);

  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;
  valCxrTotalAmtPerCxr["AA"] = valCxrTotalAmt1;
  valCxrTotalAmtPerCxr["BA"] = valCxrTotalAmt2;

  for (const auto& it : valCxrTotalAmtPerCxr)
    call_setSpValCxrsMap(it.first, it.second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);

  ASSERT_EQ(size_t(1), spValCxrsWithLowestTotalAmt.size());
  auto it = spValCxrsWithLowestTotalAmt.find("BSP");
  ASSERT_TRUE(it != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(it->second.size() == 2);
  ASSERT_TRUE(it->second[0] == "AA");
  ASSERT_TRUE(it->second[1] == "BA");
}

TEST_F(LegacyTaxProcessorTest, TestSetSpValCxrsMap_MultipleSPSameValCxr) {
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 20, {"GEN"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 0.0);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 0.0);

  ValidatingCxrTotalAmount valCxrTotalAmt1;
  valCxrTotalAmt1.spTaxAmountCol.push_back(spTaxAmount1);
  valCxrTotalAmt1.spTaxAmountCol.push_back(spTaxAmount2);

  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;
  valCxrTotalAmtPerCxr["AA"] = valCxrTotalAmt1;

  for (const auto& it : valCxrTotalAmtPerCxr)
    call_setSpValCxrsMap(it.first, it.second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);

  ASSERT_EQ(size_t(2), spValCxrsWithLowestTotalAmt.size());

  auto bspIt = spValCxrsWithLowestTotalAmt.find("BSP");
  ASSERT_TRUE(bspIt != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(bspIt->second.size() == 1);
  ASSERT_TRUE(bspIt->second[0] == "AA");

  auto genIt = spValCxrsWithLowestTotalAmt.find("GEN");
  ASSERT_TRUE(genIt != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(genIt->second.size() == 1);
  ASSERT_TRUE(genIt->second[0] == "AA");
}

TEST_F(LegacyTaxProcessorTest, TestSetSpValCxrsMap_MultipleSPMultipleValCxr) {
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "BA", 20, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath3, *_taxResp3, "ADT", "AA", 20, {"GEN"}, MultiSPTrue);
  setTaxResponse(*_farePath4, *_taxResp4, "ADT", "BA", 20, {"GEN"}, MultiSPTrue);

  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 0.0);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 0.0);
  SettlementPlanTaxAmount spTaxAmount3(_taxResp3, 0.0);
  SettlementPlanTaxAmount spTaxAmount4(_taxResp4, 0.0);

  ValidatingCxrTotalAmount valCxrTotalAmt1; //AA
  valCxrTotalAmt1.spTaxAmountCol.push_back(spTaxAmount1);
  valCxrTotalAmt1.spTaxAmountCol.push_back(spTaxAmount3);

  ValidatingCxrTotalAmount valCxrTotalAmt2; //BA
  valCxrTotalAmt2.spTaxAmountCol.push_back(spTaxAmount2);
  valCxrTotalAmt2.spTaxAmountCol.push_back(spTaxAmount4);

  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;
  valCxrTotalAmtPerCxr["AA"] = valCxrTotalAmt1;
  valCxrTotalAmtPerCxr["BA"] = valCxrTotalAmt2;

  for (const auto& it : valCxrTotalAmtPerCxr)
    call_setSpValCxrsMap(it.first, it.second.spTaxAmountCol, spValCxrsWithLowestTotalAmt);

  ASSERT_EQ(size_t(2), spValCxrsWithLowestTotalAmt.size());

  auto bspIt = spValCxrsWithLowestTotalAmt.find("BSP");
  ASSERT_TRUE(bspIt != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(bspIt->second.size() == 2);
  ASSERT_TRUE(bspIt->second[0] == "AA");
  ASSERT_TRUE(bspIt->second[1] == "BA");

  auto genIt = spValCxrsWithLowestTotalAmt.find("GEN");
  ASSERT_TRUE(genIt != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(genIt->second.size() == 2);
  ASSERT_TRUE(genIt->second[0] == "AA");
  ASSERT_TRUE(genIt->second[1] == "BA");
}

TEST_F(LegacyTaxProcessorTest, TestClearSettlementPlanData_EmptySP) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  call_clearSettlementPlanData(currentSpTaxAmountCol, "");
  ASSERT_TRUE(currentSpTaxAmountCol.empty());

  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);
  call_clearSettlementPlanData(currentSpTaxAmountCol, "");
  ASSERT_FALSE(currentSpTaxAmountCol.empty());

  currentSpTaxAmountCol.clear();
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {""}, MultiSPTrue);
  spTaxAmount1.taxResponse = _taxResp1;
  currentSpTaxAmountCol.push_back(spTaxAmount1);
  call_clearSettlementPlanData(currentSpTaxAmountCol, "BSP");
  ASSERT_TRUE(currentSpTaxAmountCol.empty());
}

TEST_F(LegacyTaxProcessorTest, TestClearSettlementPlanData_NoMatch_SingleSP) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;

  // Single SettlementPlanTaxAmount and TaxResponse. TaxResponse has only one SP
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 120.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  ASSERT_TRUE(!currentSpTaxAmountCol.empty());
  call_clearSettlementPlanData(currentSpTaxAmountCol, "ARC");
  ASSERT_TRUE(currentSpTaxAmountCol.empty());
}

TEST_F(LegacyTaxProcessorTest, TestClearSettlementPlanData_NoMatch_MultipleSP) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;

  // Single SettlementPlanTaxAmount and TaxResponse. TaxResponse has only one SP
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP", "GEN", "GTC"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 120.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  ASSERT_TRUE(!currentSpTaxAmountCol.empty());
  call_clearSettlementPlanData(currentSpTaxAmountCol, "ARC");
  ASSERT_TRUE(currentSpTaxAmountCol.empty());
}

TEST_F(LegacyTaxProcessorTest, TestClearSettlementPlanData_NoMatch_MultipleTaxResponses) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "BA", 20, {"BSP"}, MultiSPTrue);

  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 120.0);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 120.0);

  currentSpTaxAmountCol.push_back(spTaxAmount1);
  currentSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(2==currentSpTaxAmountCol.size());
  call_clearSettlementPlanData(currentSpTaxAmountCol, "ARC");
  ASSERT_TRUE(currentSpTaxAmountCol.empty());
}


TEST_F(LegacyTaxProcessorTest, TestCheckSpIndex) {
  // empty settlement-plans vector in _taxResp1
  SettlementPlanType sp = "BSP";
  size_t currInd = 0;
  ASSERT_FALSE(call_checkSpIndex(*_taxResp1, sp, currInd));
  ASSERT_FALSE(currInd < _taxResp1->settlementPlans().size());

  // empty sp test
  sp = "";
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP"}, MultiSPTrue);
  ASSERT_FALSE(call_checkSpIndex(*_taxResp1, sp, currInd));
  ASSERT_FALSE(currInd < _taxResp1->settlementPlans().size());

  // non match test
  sp = "ARC";
  ASSERT_FALSE(call_checkSpIndex(*_taxResp1, sp, currInd));
  ASSERT_FALSE(currInd < _taxResp1->settlementPlans().size());

  // match test
  sp = "BSP";
  ASSERT_TRUE(call_checkSpIndex(*_taxResp1, sp, currInd));
  ASSERT_TRUE(currInd < _taxResp1->settlementPlans().size());

  // not match in multiple settlements in _taxResp3
  sp="ARC";
  setTaxResponse(*_farePath3, *_taxResp2, "ADT", "AA", 20, {"BSP", "GEN"}, MultiSPTrue);
  ASSERT_FALSE(call_checkSpIndex(*_taxResp2, sp, currInd));
  ASSERT_FALSE(currInd < _taxResp2->settlementPlans().size());

  // match in multiple settlements in _taxResp3
  sp="GEN";
  ASSERT_TRUE(call_checkSpIndex(*_taxResp2, sp, currInd));
  ASSERT_TRUE(currInd < _taxResp2->settlementPlans().size());
}

TEST_F(LegacyTaxProcessorTest, TestCheckSettlementPlanHierarchy_EmptyTest) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  SettlementPlanType higherSp;
  ASSERT_FALSE(call_checkSettlementPlanHierarchy(currentSpTaxAmountCol,nextSpTaxAmountCol,higherSp));
  ASSERT_TRUE(higherSp.empty());
}

TEST_F(LegacyTaxProcessorTest, TestCheckSettlementPlanHierarchy_NullTest) {
  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0);
  SettlementPlanTaxAmount spTaxAmount2(nullptr, 0.0);

  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;

  currentSpTaxAmountCol.push_back(spTaxAmount1);
  nextSpTaxAmountCol.push_back(spTaxAmount2);
  SettlementPlanType higherSp;

  ASSERT_FALSE(call_checkSettlementPlanHierarchy(currentSpTaxAmountCol,nextSpTaxAmountCol,higherSp));
  ASSERT_TRUE(higherSp.empty());
}

TEST_F(LegacyTaxProcessorTest, TestCheckSettlementPlanHierarchy_SameListOfSP) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;

  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP", "GEN"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 0.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 20, {"BSP", "GEN"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 0.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  SettlementPlanType higherSp;

  ASSERT_TRUE(call_checkSettlementPlanHierarchy(currentSpTaxAmountCol,nextSpTaxAmountCol,higherSp));
  ASSERT_TRUE(higherSp == "BSP");
}

TEST_F(LegacyTaxProcessorTest, TestCheckSettlementPlanHierarchy_DifferentListOfSP) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 20, {"BSP", "GEN"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 0.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 20, {"TCH"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 0.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  SettlementPlanType higherSp;

  ASSERT_TRUE(call_checkSettlementPlanHierarchy(currentSpTaxAmountCol, nextSpTaxAmountCol, higherSp));
  ASSERT_TRUE(higherSp == "BSP");
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_EmptyTest) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  ASSERT_FALSE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_NullTest) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  SettlementPlanTaxAmount spTaxAmount1(nullptr, 0.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  SettlementPlanTaxAmount spTaxAmount2(nullptr, 0.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_FALSE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_SingleTaxRecord_Merge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_SingleZeroTax_Merge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 0, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 0, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 100.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 0, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 0, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 100.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_UnEqualTaxComponents_NoMerge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_FALSE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_EqualTaxComponentsDiffTaxCodes_NoMerge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"GA:4.0", "GA:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_FALSE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCanMergeTaxResponse_SameTaxComponents_Merge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(call_canMergeTaxResponses(currentSpTaxAmountCol, nextSpTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestCheckAndProcessTaxComponents_SameTaxComponents_Merge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 2);
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
  ASSERT_EQ(std::string("GEN"),
      std::string(currentSpTaxAmountCol[1].taxResponse->settlementPlans()[0]));
}

TEST_F(LegacyTaxProcessorTest, TestCheckAndProcessTaxComponents_SameAmountDIffTaxComp_NoMerge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
}

TEST_F(LegacyTaxProcessorTest,
    TestCheckAndProcessTaxComponents_DIffTaxComponentsCurrentHigher_NoMerge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
}

TEST_F(LegacyTaxProcessorTest,
    TestCheckAndProcessTaxComponents_DIffTaxComponentsCurrentNotHigher_NoMerge) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"GEN"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"TCH"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);
  ASSERT_EQ(std::string("TCH"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
}

TEST_F(LegacyTaxProcessorTest,
    TestCheckAndProcessTaxComponents_DIffTaxComponentsMultiSPCurrentNotHigher_NoMerge1) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"RUT"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"TCH", "GEN"}, MultiSPTrue);
  _taxResp2->taxRecordVector().front()->setTaxCode("US");

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);
  ASSERT_EQ(std::string("TCH"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
}

// multiple items in the vector
TEST_F(LegacyTaxProcessorTest,
    TestCheckAndProcessTaxComponents_DIffTaxComponentsMultiSPCurrentNotHigher_NoMerge2) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"RUT"}, MultiSPTrue);
  _taxResp1->taxRecordVector().front()->setTaxCode("US");

  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp3, "ADT", "BA", 10, {"TCH", "GEN"}, MultiSPTrue);

  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  setTaxComponents(*_taxResp3, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  SettlementPlanTaxAmount spTaxAmount3(_taxResp3, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);
  nextSpTaxAmountCol.push_back(spTaxAmount3);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 1);
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
}

TEST_F(LegacyTaxProcessorTest,
    TestCheckAndProcessTaxComponents_DIffTaxComponentsMultiSPCurrentNotHigher_NoMerge3) {
  std::vector<SettlementPlanTaxAmount> currentSpTaxAmountCol;
  std::vector<SettlementPlanTaxAmount> nextSpTaxAmountCol;

  // CNN
  setFarePath(*_farePath1, "CNN", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "CNN", "9W", 10, {"RUT"}, MultiSPTrue);
  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  currentSpTaxAmountCol.push_back(spTaxAmount1);

  // ADT
  setFarePath(*_farePath2, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "ADT", "AA", 10, {"BSP"}, MultiSPTrue);
  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount2);

  setTaxResponse(*_farePath2, *_taxResp3, "ADT", "BA", 10, {"BSP", "TCH", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp3, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount3(_taxResp3, 110.0);
  nextSpTaxAmountCol.push_back(spTaxAmount3);

  call_checkTaxComponentsAndMerge(currentSpTaxAmountCol, nextSpTaxAmountCol);
  ASSERT_TRUE(currentSpTaxAmountCol.size() == 2);
  ASSERT_TRUE(currentSpTaxAmountCol[0].taxResponse->settlementPlans().size()==1);
  ASSERT_TRUE(currentSpTaxAmountCol[1].taxResponse->settlementPlans().size()==3);
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[0].taxResponse->settlementPlans()[0]));
  ASSERT_EQ(std::string("BSP"),
      std::string(currentSpTaxAmountCol[1].taxResponse->settlementPlans()[0]));
  ASSERT_EQ(std::string("TCH"),
      std::string(currentSpTaxAmountCol[1].taxResponse->settlementPlans()[1]));
  ASSERT_EQ(std::string("GEN"),
      std::string(currentSpTaxAmountCol[1].taxResponse->settlementPlans()[2]));
}

TEST_F(LegacyTaxProcessorTest, TestGetSpTotalAmount_positive) {
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;
  setFarePath(*_farePath1, "ADT", "AA", 100.0, 30.0, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10.0, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "BA", 20.0, {"TCH"}, MultiSPTrue);
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 120.0);
  spTaxAmountCol.push_back(spTaxAmount1);
  spTaxAmountCol.push_back(spTaxAmount2);
  ASSERT_EQ(230.0, call_getSpTotalAmount(spTaxAmountCol));
}

TEST_F(LegacyTaxProcessorTest, TestFindSpTotalAmount) {
  setFarePath(*_farePath1, "ADT", "AA", 100.0, 30.0, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10.0, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "BA", 10.0, {"GEN"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp3, "ADT", "BA", 10.0, {"RUT"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp4, "ADT", "BA", 20.0, {"TCH"}, MultiSPTrue);
  std::vector<TaxResponse*> taxResponses;
  taxResponses.push_back(_taxResp1);
  taxResponses.push_back(_taxResp2);
  taxResponses.push_back(_taxResp3);
  taxResponses.push_back(_taxResp4);

  SettlementPlanTaxAmountGroupMap spTaxAmtGroup;
  call_findSpTotalAmount(*_farePath1, taxResponses, spTaxAmtGroup);
  ASSERT_TRUE(spTaxAmtGroup.size()==2);

  auto itR = spTaxAmtGroup.find(SettlementPlanGroup::REG_SP);
  ASSERT_TRUE(itR != spTaxAmtGroup.end());
  ASSERT_TRUE(itR->second.size() == 3);
  ASSERT_EQ(std::string("BSP"),
      std::string(itR->second.front().taxResponse->settlementPlans().front()));

  auto itT = spTaxAmtGroup.find(SettlementPlanGroup::TCH_SP);
  ASSERT_TRUE(itT != spTaxAmtGroup.end());
  ASSERT_TRUE(itT->second.size() == 1);
  ASSERT_EQ(std::string("TCH"),
      std::string(itT->second.front().taxResponse->settlementPlans().front()));
}

TEST_F(LegacyTaxProcessorTest, TestFindSpTotalAmount_NoTCH) {
  setFarePath(*_farePath1, "ADT", "AA", 100.0, 30.0, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10.0, {"BSP"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "BA", 10.0, {"GEN"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp3, "ADT", "BA", 10.0, {"RUT"}, MultiSPTrue);
  std::vector<TaxResponse*> taxResponses;
  taxResponses.push_back(_taxResp1);
  taxResponses.push_back(_taxResp2);
  taxResponses.push_back(_taxResp3);

  SettlementPlanTaxAmountGroupMap spTaxAmtGroup;
  call_findSpTotalAmount(*_farePath1, taxResponses, spTaxAmtGroup);
  ASSERT_TRUE(spTaxAmtGroup.size()==1);

  auto itR = spTaxAmtGroup.find(SettlementPlanGroup::REG_SP);
  ASSERT_TRUE(itR != spTaxAmtGroup.end());
  ASSERT_TRUE(itR->second.size() == 3);
  ASSERT_EQ(std::string("BSP"),
      std::string(itR->second.front().taxResponse->settlementPlans().front()));

  auto itT = spTaxAmtGroup.find(SettlementPlanGroup::TCH_SP);
  ASSERT_TRUE(itT == spTaxAmtGroup.end());
}

TEST_F(LegacyTaxProcessorTest, TestFindSpTotalAmount_OnlyTCH) {
  setFarePath(*_farePath1, "ADT", "AA", 100.0, 30.0, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10.0, {"TCH"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "BA", 10.0, {"TCH"}, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp3, "ADT", "BA", 10.0, {"TCH"}, MultiSPTrue);
  std::vector<TaxResponse*> taxResponses;
  taxResponses.push_back(_taxResp1);
  taxResponses.push_back(_taxResp2);
  taxResponses.push_back(_taxResp3);

  SettlementPlanTaxAmountGroupMap spTaxAmtGroup;
  call_findSpTotalAmount(*_farePath1, taxResponses, spTaxAmtGroup);
  ASSERT_TRUE(spTaxAmtGroup.size()==1);

  auto itT = spTaxAmtGroup.find(SettlementPlanGroup::TCH_SP);
  ASSERT_TRUE(itT != spTaxAmtGroup.end());
  ASSERT_TRUE(itT->second.size() == 3);
  ASSERT_EQ(std::string("TCH"),
      std::string(itT->second.front().taxResponse->settlementPlans().front()));

  auto itR = spTaxAmtGroup.find(SettlementPlanGroup::REG_SP);
  ASSERT_TRUE(itR == spTaxAmtGroup.end());
}



// Two passengers with same carrier participating in {BSP, GEN} and {TCH} with
// different total amount. Higher amount SP will be rejected.
TEST_F(LegacyTaxProcessorTest, TestProcessSettlementPlanTaxData_NoMerge) {
  SettlementPlanTaxAmountGroupMap spTaxAmtGroup;

  // Passenger 1: ADT _taxResp1 and _taxResp2
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp1, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::REG_SP].push_back(spTaxAmount1);

  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "9W", 20, {"TCH"}, MultiSPTrue);
  setTaxComponents(*_taxResp2, {"US:6.0", "US:6.0", "US:8.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 120.0);
  spTaxAmtGroup[SettlementPlanGroup::TCH_SP].push_back(spTaxAmount2);

  // Passenger 2: CNN _taxResp3 and _taxResp4
  setFarePath(*_farePath2, "CNN", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp3, "ADT", "9W", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp3, {"US:4.0", "US:4.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount3(_taxResp3, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::REG_SP].push_back(spTaxAmount3);

  setTaxResponse(*_farePath2, *_taxResp4, "ADT", "9W", 20, {"TCH"}, MultiSPTrue);
  setTaxComponents(*_taxResp4, {"US:6.0", "US:6.0", "US:8.0"});
  SettlementPlanTaxAmount spTaxAmount4(_taxResp4, 120.0);
  spTaxAmtGroup[SettlementPlanGroup::TCH_SP].push_back(spTaxAmount4);


  ASSERT_TRUE(spTaxAmtGroup.size()==2);

  ValidatingCxrTotalAmount valCxrTotalAmt;
  call_processSettlementPlanTaxData(valCxrTotalAmt.totalAmount,
      valCxrTotalAmt.spTaxAmountCol,
      spTaxAmtGroup);

  ASSERT_EQ(220, valCxrTotalAmt.totalAmount);
  ASSERT_EQ(2, valCxrTotalAmt.spTaxAmountCol.size());
  ASSERT_EQ(2, valCxrTotalAmt.spTaxAmountCol.front().taxResponse->settlementPlans().size());
}

// Two passengers with same carrier participating in {BSP, GEN} and {TCH} with
// with same total amount and same tax components. We should see merge.
TEST_F(LegacyTaxProcessorTest, TestProcessSettlementPlanTaxData_Merge) {
  SettlementPlanTaxAmountGroupMap spTaxAmtGroup;

  // Passenger 1: ADT _taxResp1 and _taxResp2
  setFarePath(*_farePath1, "ADT", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "9W", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount1(_taxResp1, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::REG_SP].push_back(spTaxAmount1);

  setTaxResponse(*_farePath1, *_taxResp2, "ADT", "9W", 10, {"TCH"}, MultiSPTrue);
  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount2(_taxResp2, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::TCH_SP].push_back(spTaxAmount2);

  // Passenger 2: CNN _taxResp3 and _taxResp4
  setFarePath(*_farePath2, "CNN", "9W", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp3, "ADT", "9W", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp3, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount3(_taxResp3, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::REG_SP].push_back(spTaxAmount3);

  setTaxResponse(*_farePath2, *_taxResp4, "ADT", "9W", 10, {"TCH"}, MultiSPTrue);
  setTaxComponents(*_taxResp4, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount4(_taxResp4, 110.0);
  spTaxAmtGroup[SettlementPlanGroup::TCH_SP].push_back(spTaxAmount4);


  ASSERT_TRUE(spTaxAmtGroup.size()==2);

  ValidatingCxrTotalAmount valCxrTotalAmt;
  call_processSettlementPlanTaxData(valCxrTotalAmt.totalAmount,
      valCxrTotalAmt.spTaxAmountCol,
      spTaxAmtGroup);

  ASSERT_EQ(220, valCxrTotalAmt.totalAmount);
  ASSERT_EQ(4, valCxrTotalAmt.spTaxAmountCol.size());
}

// Carrier AA and BA participates in {BSP, GEN} for all passengers.
// Expected result: BSP=>AA, BA and GEN=>AA, BA
TEST_F(LegacyTaxProcessorTest, TestProcessValidatingCxrTotalAmount_Merge) {
  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;

  // Data for carrier AA and two passenger ADT and CNN
  setFarePath(*_farePath1, "ADT", "AA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_ADT_AA(_taxResp1, 110.0);

  setFarePath(*_farePath2, "CNN", "AA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "CNN", "AA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_CNN_AA(_taxResp1, 110.0);

  ValidatingCxrTotalAmount valCxrTotalAmt_AA;
  valCxrTotalAmt_AA.totalAmount=220.00;
  valCxrTotalAmt_AA.spTaxAmountCol.push_back(spTaxAmount_ADT_AA);
  valCxrTotalAmt_AA.spTaxAmountCol.push_back(spTaxAmount_CNN_AA);

  valCxrTotalAmtPerCxr["AA"]=valCxrTotalAmt_AA;

  // Data for carrier BA and two passenger ADT and CNN
  setFarePath(*_farePath3, "ADT", "BA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath3, *_taxResp3, "ADT", "BA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp3, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_ADT_BA(_taxResp3, 110.0);

  setFarePath(*_farePath4, "CNN", "BA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath4, *_taxResp4, "CNN", "BA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp4, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_CNN_BA(_taxResp4, 110.0);

  ValidatingCxrTotalAmount valCxrTotalAmt_BA;
  valCxrTotalAmt_BA.totalAmount=220.00;
  valCxrTotalAmt_BA.spTaxAmountCol.push_back(spTaxAmount_ADT_AA);
  valCxrTotalAmt_BA.spTaxAmountCol.push_back(spTaxAmount_CNN_AA);

  valCxrTotalAmtPerCxr["BA"]=valCxrTotalAmt_BA;

  // Test
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  call_processValidatingCxrTotalAmount(valCxrTotalAmtPerCxr, spValCxrsWithLowestTotalAmt);
  ASSERT_EQ(2, spValCxrsWithLowestTotalAmt.size());
  auto it = spValCxrsWithLowestTotalAmt.begin();

  ASSERT_TRUE(it->first == "BSP" || it->first == "GEN");
  ASSERT_EQ(2, it->second.size());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "AA") != it->second.end());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "BA") != it->second.end());

  ASSERT_TRUE(++it != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(it->first == "BSP" || it->first == "GEN");
  ASSERT_EQ(2, it->second.size());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "AA") != it->second.end());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "BA") != it->second.end());
}

// Carrier AA and BA participates in {BSP, GEN} for all passengers but BA is higher in total amount
// Expected result: BSP=>AA and GEN=>AA
TEST_F(LegacyTaxProcessorTest, TestProcessValidatingCxrTotalAmount_NoMerge) {
  std::map<CarrierCode, ValidatingCxrTotalAmount> valCxrTotalAmtPerCxr;

  // Data for carrier AA and two passenger ADT and CNN
  setFarePath(*_farePath1, "ADT", "AA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath1, *_taxResp1, "ADT", "AA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp1, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_ADT_AA(_taxResp1, 110.0);

  setFarePath(*_farePath2, "CNN", "AA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath2, *_taxResp2, "CNN", "AA", 10, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp2, {"US:4.0", "US:6.0"});
  SettlementPlanTaxAmount spTaxAmount_CNN_AA(_taxResp1, 110.0);

  ValidatingCxrTotalAmount valCxrTotalAmt_AA;
  valCxrTotalAmt_AA.totalAmount=220.00;
  valCxrTotalAmt_AA.spTaxAmountCol.push_back(spTaxAmount_ADT_AA);
  valCxrTotalAmt_AA.spTaxAmountCol.push_back(spTaxAmount_CNN_AA);

  valCxrTotalAmtPerCxr["AA"]=valCxrTotalAmt_AA;

  // Data for carrier BA and two passenger ADT and CNN with higher tax 
  setFarePath(*_farePath3, "ADT", "BA", 100, 12, MultiSPTrue);
  setTaxResponse(*_farePath3, *_taxResp3, "ADT", "BA", 12, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp3, {"US:4.0", "US:6.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount_ADT_BA(_taxResp3, 112.0);

  setFarePath(*_farePath4, "CNN", "BA", 100, 10, MultiSPTrue);
  setTaxResponse(*_farePath4, *_taxResp4, "CNN", "BA", 12, {"BSP", "GEN"}, MultiSPTrue);
  setTaxComponents(*_taxResp4, {"US:4.0", "US:6.0", "US:2.0"});
  SettlementPlanTaxAmount spTaxAmount_CNN_BA(_taxResp4, 112.0);

  ValidatingCxrTotalAmount valCxrTotalAmt_BA;
  valCxrTotalAmt_BA.totalAmount=224.00;
  valCxrTotalAmt_BA.spTaxAmountCol.push_back(spTaxAmount_ADT_AA);
  valCxrTotalAmt_BA.spTaxAmountCol.push_back(spTaxAmount_CNN_AA);

  valCxrTotalAmtPerCxr["BA"]=valCxrTotalAmt_BA;

  // Test
  SettlementPlanValCxrsMap spValCxrsWithLowestTotalAmt;
  call_processValidatingCxrTotalAmount(valCxrTotalAmtPerCxr, spValCxrsWithLowestTotalAmt);
  ASSERT_EQ(2, spValCxrsWithLowestTotalAmt.size());
  auto it = spValCxrsWithLowestTotalAmt.begin();

  ASSERT_TRUE(it->first == "BSP" || it->first == "GEN");
  ASSERT_EQ(1, it->second.size());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "AA") != it->second.end());

  ASSERT_TRUE(++it != spValCxrsWithLowestTotalAmt.end());
  ASSERT_TRUE(it->first == "BSP" || it->first == "GEN");
  ASSERT_EQ(1, it->second.size());
  ASSERT_TRUE(std::find(it->second.begin(), it->second.end(), "AA") != it->second.end());
}

TEST_F(LegacyTaxProcessorTest,
    TestHandleValidatingCarrierError_NetRemitNoDefaultValCxrError) {
  try {
    // Setting data for test
    PricingTrx trx;
    trx.setValidatingCxrGsaApplicable(true);
    trx.setRequest(_memHandle.create<PricingRequest>());
    trx.getRequest()->isMultiTicketRequest() = false;

    std::vector<CarrierCode> valCxrs;
    valCxrs.push_back("AA");
    valCxrs.push_back("BA");

    //setFarePath(*_farePath1, "ADT", "", valCxrs, 100, 10);
    setFarePath(*_farePath1, "ADT", "", 100, 10, MultiSPTrue, {"AA", "BB"});
    _farePath1->netRemitFarePath() = _memHandle.create<NetRemitFarePath>();
    _farePath1->processed() = false ;
    _itin->farePath().push_back(_farePath1);
    trx.itin().push_back(_itin);

    trx.setExcTrxType(PricingTrx::NOT_EXC_TRX);
    trx.setTrxType(PricingTrx::PRICING_TRX);

    // Test
    call_handleValidatingCarrierError(trx);
    ASSERT_TRUE(1==2); // We should NOT be here.
  }
  catch (ErrorResponseException& e) {
    ASSERT_TRUE(e.code() == ErrorResponseException::VALIDATING_CXR_ERROR);
    ASSERT_EQ(std::string("MULTIPLE VALIDATING CARRIER OPTIONS - SPECIFY #A OR #VM"), e.message());
  }
}

TEST_F(LegacyTaxProcessorTest, testHandleAdjustedSellingLevelFarePaths)
{
  Logger logger("atseintl.Taxes.LegacyTaxes.LegacyTaxProcessorTest");
  MockLegacyTaxProcessor m(TseThreadingConst::SYNCHRONOUS_TASK, logger);

  PricingTrx trx;
  // Make sure all itins and all farepaths with adjusted FP are called

  Itin itin1, itin2;
  FarePath fp1, fp2, fp3, fp4, adjFp2, adjFp4;

  fp1.processed() = false;
  fp2.processed() = false;
  fp3.processed() = false;
  fp4.processed() = false;
  adjFp2.processed() = false;
  adjFp4.processed() = false;

  fp2.adjustedSellingFarePath() = &adjFp2;
  fp4.adjustedSellingFarePath() = &adjFp4;

  itin1.farePath().push_back(&fp1);
  itin1.farePath().push_back(&fp2);
  itin2.farePath().push_back(&fp3);
  itin2.farePath().push_back(&fp4);

  trx.itin().push_back(&itin1);
  trx.itin().push_back(&itin2);

  m.handleAdjustedSellingLevelFarePaths(trx);
  ASSERT_TRUE(!fp1.processed());
  ASSERT_TRUE(fp2.processed());
  ASSERT_TRUE(!fp3.processed());
  ASSERT_TRUE(fp4.processed());
  ASSERT_TRUE(adjFp2.processed());
  ASSERT_TRUE(adjFp4.processed());
}

TEST_F(LegacyTaxProcessorTest, testProcessAdjustedSellingLevelFarePath)
{
  TaxRecord tr1, tr2, tr3, tr4, tr5;
  tr1.setTaxAmount(10);
  tr2.setTaxAmount(20);
  tr3.setTaxAmount(30);
  tr4.setTaxAmount(40);
  tr5.setTaxAmount(50);

  TaxResponse taxResponse1;
  taxResponse1.taxRecordVector().push_back(&tr1);

  TaxRecord adjTr1, adjTr2, adjTr3, adjTr4, adjTr5;
  adjTr1.setTaxAmount(199);
  adjTr2.setTaxAmount(299);
  adjTr3.setTaxAmount(399);
  adjTr4.setTaxAmount(499);
  adjTr5.setTaxAmount(599);

  TaxResponse taxResponse2;
  taxResponse2.taxRecordVector().push_back(&adjTr1);

  setFarePath(*_farePath1, "ADT", "", 100, 10, MultiSPTrue, {"AA", "BB"});
  setFarePath(*_farePath2, "ADT", "", 100, 10, MultiSPTrue, {"AA", "BB"});

  _farePath1->adjustedSellingFarePath() = _farePath2;
  _farePath1->itin()->mutableTaxResponses().push_back(&taxResponse1);
  _farePath2->itin()->mutableTaxResponses().push_back(&taxResponse2);

  taxResponse1.farePath() = _farePath1;
  taxResponse2.farePath() = _farePath2;

  PricingTrx trx;
  call_processAdjustedSellingLevelFarePath(trx, _farePath1);

  ASSERT_TRUE(1 == taxResponse2.taxRecordVector().size());
  ASSERT_TRUE(taxResponse2.taxRecordVector()[0]->getTaxAmount() == 10);

  taxResponse1.taxRecordVector().clear();
  taxResponse2.taxRecordVector().clear();

  taxResponse1.taxRecordVector().push_back(&tr1);
  taxResponse1.taxRecordVector().push_back(&tr2);
  taxResponse1.taxRecordVector().push_back(&tr3);
  taxResponse1.taxRecordVector().push_back(&tr4);
  taxResponse1.taxRecordVector().push_back(&tr5);

  taxResponse2.taxRecordVector().push_back(&adjTr1);
  taxResponse2.taxRecordVector().push_back(&adjTr2);
  taxResponse2.taxRecordVector().push_back(&adjTr3);
  taxResponse2.taxRecordVector().push_back(&adjTr4);
  taxResponse2.taxRecordVector().push_back(&adjTr5);

  call_processAdjustedSellingLevelFarePath(trx, _farePath1);

  ASSERT_TRUE(5 == taxResponse2.taxRecordVector().size());
  ASSERT_TRUE(taxResponse2.taxRecordVector()[0]->getTaxAmount() == 10);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[1]->getTaxAmount() == 20);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[2]->getTaxAmount() == 30);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[3]->getTaxAmount() == 40);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[4]->getTaxAmount() == 50);

  taxResponse1.taxRecordVector().clear();
  taxResponse2.taxRecordVector().clear();

  tr2._gstTaxInd = true;
  tr4._gstTaxInd = true;
  adjTr2._gstTaxInd = true;
  adjTr4._gstTaxInd = true;

  taxResponse1.taxRecordVector().push_back(&tr1);
  taxResponse1.taxRecordVector().push_back(&tr2);
  taxResponse1.taxRecordVector().push_back(&tr3);
  taxResponse1.taxRecordVector().push_back(&tr4);
  taxResponse1.taxRecordVector().push_back(&tr5);

  taxResponse2.taxRecordVector().push_back(&adjTr1);
  taxResponse2.taxRecordVector().push_back(&adjTr2);
  taxResponse2.taxRecordVector().push_back(&adjTr3);
  taxResponse2.taxRecordVector().push_back(&adjTr4);
  taxResponse2.taxRecordVector().push_back(&adjTr5);

  call_processAdjustedSellingLevelFarePath(trx, _farePath1);

  ASSERT_TRUE(5 == taxResponse2.taxRecordVector().size());
  ASSERT_TRUE(taxResponse2.taxRecordVector()[0]->getTaxAmount() == 10);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[1]->getTaxAmount() == 299);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[2]->getTaxAmount() == 30);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[3]->getTaxAmount() == 499);
  ASSERT_TRUE(taxResponse2.taxRecordVector()[4]->getTaxAmount() == 50);

  TaxItem ti1, ti2, ti3, ti4, ti5;
  ti1.taxAmt() = 11;
  ti1._isGstTax = false;
  ti2.taxAmt() = 12;
  ti2._isGstTax = false;
  ti3.taxAmt() = 13;
  ti3._isGstTax = false;
  ti4.taxAmt() = 14;
  ti4._isGstTax = false;
  ti5.taxAmt() = 15;
  ti5._isGstTax = false;

  TaxItem adjTi1, adjTi2, adjTi3, adjTi4, adjTi5;
  adjTi1.taxAmt() = 333;
  adjTi1._isGstTax = false;
  adjTi2.taxAmt() = 444;
  adjTi2._isGstTax = false;
  adjTi3.taxAmt() = 555;
  adjTi3._isGstTax = false;
  adjTi4.taxAmt() = 666;
  adjTi4._isGstTax = false;
  adjTi5.taxAmt() = 777;
  adjTi5._isGstTax = false;

  taxResponse1.taxItemVector().push_back(&ti1);
  taxResponse2.taxItemVector().push_back(&adjTi1);

  call_processAdjustedSellingLevelFarePath(trx, _farePath1);
  ASSERT_TRUE(1 == taxResponse2.taxItemVector().size());
  ASSERT_TRUE(taxResponse2.taxItemVector()[0]->taxAmt() == 11);

  taxResponse1.taxItemVector().clear();
  taxResponse2.taxItemVector().clear();

  taxResponse1.taxItemVector().push_back(&ti1);
  taxResponse1.taxItemVector().push_back(&ti2);
  taxResponse1.taxItemVector().push_back(&ti3);
  taxResponse1.taxItemVector().push_back(&ti4);
  taxResponse1.taxItemVector().push_back(&ti5);

  taxResponse2.taxItemVector().push_back(&adjTi1);
  taxResponse2.taxItemVector().push_back(&adjTi2);
  taxResponse2.taxItemVector().push_back(&adjTi3);
  taxResponse2.taxItemVector().push_back(&adjTi4);
  taxResponse2.taxItemVector().push_back(&adjTi5);

  call_processAdjustedSellingLevelFarePath(trx, _farePath1);
  ASSERT_TRUE(5 == taxResponse2.taxItemVector().size());
  ASSERT_TRUE(taxResponse2.taxItemVector()[0]->taxAmt() == 11);
  ASSERT_TRUE(taxResponse2.taxItemVector()[1]->taxAmt() == 12);
  ASSERT_TRUE(taxResponse2.taxItemVector()[2]->taxAmt() == 13);
  ASSERT_TRUE(taxResponse2.taxItemVector()[3]->taxAmt() == 14);
  ASSERT_TRUE(taxResponse2.taxItemVector()[4]->taxAmt() == 15);

  taxResponse1.taxItemVector().clear();
  taxResponse2.taxItemVector().clear();

  ti1._isGstTax = true;
  ti3._isGstTax = true;
  ti5._isGstTax = true;
  adjTi1._isGstTax = true;
  adjTi3._isGstTax = true;
  adjTi5._isGstTax = true;

  taxResponse1.taxItemVector().push_back(&ti1);
  taxResponse1.taxItemVector().push_back(&ti2);
  taxResponse1.taxItemVector().push_back(&ti3);
  taxResponse1.taxItemVector().push_back(&ti4);
  taxResponse1.taxItemVector().push_back(&ti5);

  taxResponse2.taxItemVector().push_back(&adjTi1);
  taxResponse2.taxItemVector().push_back(&adjTi2);
  taxResponse2.taxItemVector().push_back(&adjTi3);
  taxResponse2.taxItemVector().push_back(&adjTi4);
  taxResponse2.taxItemVector().push_back(&adjTi5);

  call_processAdjustedSellingLevelFarePath(trx, _farePath1);
  ASSERT_TRUE(5 == taxResponse2.taxItemVector().size());
  ASSERT_TRUE(taxResponse2.taxItemVector()[0]->taxAmt() == 333);
  ASSERT_TRUE(taxResponse2.taxItemVector()[1]->taxAmt() == 12);
  ASSERT_TRUE(taxResponse2.taxItemVector()[2]->taxAmt() == 555);
  ASSERT_TRUE(taxResponse2.taxItemVector()[3]->taxAmt() == 14);
  ASSERT_TRUE(taxResponse2.taxItemVector()[4]->taxAmt() == 777);
}

TEST_F(LegacyTaxProcessorTest, testAddTaxInfoResponsesToTrx)
{
  RexPricingTrx trx;
  trx.setExcTrxType(PricingTrx::AR_EXC_TRX);

  PricingRequest req;
  trx.setRequest(&req);

  std::vector<TaxCode> taxes;
  taxes.push_back("US1");
  taxes.push_back("DU1");
  taxes.push_back("AAA");
  taxes.push_back("C33");

  req.setTaxRequestedInfo(std::move(taxes));

  PricingTrx pricingTrx;
  call_addTaxInfoResponsesToTrx(pricingTrx);
  ASSERT_TRUE(trx.getTaxInfoResponse().size() == 0);

  call_addTaxInfoResponsesToTrx(trx);
  ASSERT_TRUE(trx.getTaxInfoResponse().size() == 0);

  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  call_addTaxInfoResponsesToTrx(trx);

  const std::vector<TaxResponse*>& res = trx.getTaxInfoResponse();
  ASSERT_TRUE(trx.getTaxInfoResponse().size() == 1);

  ASSERT_TRUE(res[0]->taxItemVector().size() == 4);

  ASSERT_TRUE(res[0]->taxItemVector()[0]->getRefundableTaxTag() == 'Y');//US1
  ASSERT_TRUE(res[0]->taxItemVector()[1]->getRefundableTaxTag() == 'N');//DU1
  ASSERT_TRUE(res[0]->taxItemVector()[2]->getRefundableTaxTag() == 'Y');//ABC
  ASSERT_TRUE(res[0]->taxItemVector()[3]->getRefundableTaxTag() == 'Y');//C33
}
}
