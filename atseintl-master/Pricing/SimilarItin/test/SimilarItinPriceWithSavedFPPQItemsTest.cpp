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
#include "Pricing/SimilarItin/PriceWithSavedFPPQItems.h"

#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/FarePathValidator.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "Rules/AccompaniedTravel.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using ::testing::Return;
using ::testing::_;
using similaritin::PriceWithSavedFPPQItems;
namespace
{
class MockFarePathValidator
{
public:
  bool finalValidation(FarePath&) { return finalValidation(); }
  MOCK_METHOD0(finalValidation, bool());
};
class StubRevalidator
{
public:
  StubRevalidator(similaritin::Context&, similaritin::DiagnosticWrapper&) {}
  bool validate(const std::vector<FarePath*>&, const std::vector<FPPQItem*>&, Itin&)
  {
    return true;
  }
};

class MockPaxFarePathFactory : public PaxFarePathFactoryBase
{
public:
  MockPaxFarePathFactory(const FactoriesConfig& factoriesConfig)
    : PaxFarePathFactoryBase(factoriesConfig)
  {
  }

  MOCK_METHOD1(initPaxFarePathFactory, bool(DiagCollector&));

  MOCK_METHOD2(getFPPQItem, FPPQItem*(uint32_t, DiagCollector&));

  MOCK_METHOD4(getFirstValidZeroAmountFPPQItem,
               FPPQItem*(uint32_t, const FPPQItem&, DiagCollector&, uint32_t*));

  MOCK_METHOD2(getSameFareBreakFPPQItem, FPPQItem*(const FPPQItem*, DiagCollector&));

  MOCK_METHOD1(getAlreadyBuiltFPPQItem, FPPQItem*(uint32_t));

  MOCK_METHOD4(removeFPF,
               void(DiagCollector*, const FareMarketPath*, const DatePair*, bool nonThruPricing));

  MOCK_METHOD3(checkFailedFPIS, bool(FPPQItem*, std::set<DatePair>&, bool));

  MOCK_METHOD0(clear, void());
};

using PriceWithSavedOrig = similaritin::PriceWithSavedFPPQItems<similaritin::DiagnosticWrapper,
                                                                MockFarePathValidator,
                                                                StubRevalidator>;

class SimilarItinPriceWithSavedFPPQItemsWrapper : public PriceWithSavedOrig
{
public:
  SimilarItinPriceWithSavedFPPQItemsWrapper(similaritin::Context& context,
                                            similaritin::DiagnosticWrapper& DiagnosticWrapper,
                                            MockFarePathValidator& validator)
    : PriceWithSavedOrig(context, DiagnosticWrapper, validator)
  {
  }
  using PriceWithSavedOrig::getGroupFarePathFactory;
  using PriceWithSavedOrig::priceItinWithCheaperOptions;
  using PriceWithSavedOrig::preparePaxFarePathFactoryBucket;
};
}
class SimilarItinPriceWithSavedFPPQItemsTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _diag = _memHandle.create<DiagCollector>();

    _diag->trx() = _trx;
    _diag->rootDiag() = &_trx->diagnostic();
    _diag->diagnosticType() = Diagnostic991;
    _diag->enable(Diagnostic991);
    _diag->activate();

    _trx->setTrxType(PricingTrx::MIP_TRX);
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _trx->diagnostic().diagnosticType() = Diagnostic991;
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _motherItin = _memHandle.create<Itin>();
    _trx->itin().push_back(_motherItin);
    _factoriesConfig = _memHandle.create<test::FactoriesConfigStub>();
    _similaritinContext = _memHandle.create<similaritin::Context>(*_trx, *_motherItin, *_diag);
    similaritin::DiagnosticWrapper* diagnostic =
        _memHandle.create<similaritin::DiagnosticWrapper>(*_trx, *_diag);
    _validator = _memHandle.create<MockFarePathValidator>();
    _similarItinPricing = _memHandle.create<SimilarItinPriceWithSavedFPPQItemsWrapper>(
        *_similaritinContext, *diagnostic, *_validator);
    _motherGFPF = _memHandle.create<GroupFarePathFactory>(*_trx);
    AccompaniedTravel* accompaniedTravel = _memHandle.create<AccompaniedTravel>();
    _motherGFPF->accompaniedTravel() = accompaniedTravel;
  }

  void TearDown()
  {
    _memHandle.clear();
    _pfpfBucket.clear();
  }

protected:
  FPPQItem* getFPPQItem(PaxType* paxType, Itin* itin, const MoneyAmount& amount)
  {
    FarePath* fp = _memHandle.create<FarePath>();
    fp->setTotalNUCAmount(amount);
    fp->paxType() = paxType;
    fp->itin() = itin;
    FPPQItem* fppqItem = _memHandle.create<FPPQItem>();
    fppqItem->farePath() = fp;

    return fppqItem;
  }

  PaxType* getPaxType(const std::string& paxTypeCode)
  {
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = paxTypeCode;

    return paxType;
  }

  TestMemHandle _memHandle;
  DiagCollector* _diag;
  PricingOptions* _options;
  PricingRequest* _request;
  PricingTrx* _trx;
  Itin* _motherItin;
  test::FactoriesConfigStub* _factoriesConfig;
  std::vector<PaxFarePathFactoryBase*> _pfpfBucket;
  similaritin::Context* _similaritinContext;
  SimilarItinPriceWithSavedFPPQItemsWrapper* _similarItinPricing;
  MockFarePathValidator* _validator;
  Agent* _agent;
  GroupFarePathFactory* _motherGFPF;
};

TEST_F(SimilarItinPriceWithSavedFPPQItemsTest, testPriceItinWithCheaperOptionsOnePaxGFP)
{
  MockPaxFarePathFactory pfpf(*_factoriesConfig);
  PaxType* paxType = getPaxType("ADT");
  pfpf.paxType() = paxType;
  _pfpfBucket.push_back(&pfpf);

  Itin similarItin;

  FPPQItem* fppqItem = getFPPQItem(paxType, &similarItin, 10.0);

  EXPECT_CALL(pfpf, getAlreadyBuiltFPPQItem(0)).WillOnce(Return((FPPQItem*)0));
  EXPECT_CALL(pfpf, getFPPQItem(0, _)).WillOnce(Return(fppqItem));

  EXPECT_CALL(*_validator, finalValidation()).WillOnce(Return(true));

  std::unique_ptr<GroupFarePathFactory> gfpf =
      _similarItinPricing->getGroupFarePathFactory(*_motherGFPF, _pfpfBucket);
  gfpf->usePooling() = false; // in tests using pooling causes core dumps
  ASSERT_TRUE(_similarItinPricing->priceItinWithCheaperOptions(*gfpf, similarItin));
  ASSERT_EQ(1, similarItin.farePath().size());
  ASSERT_TRUE(_diag->str().find("GROUP FARE PATH VALIDATION PASSED") != std::string::npos);
}

TEST_F(SimilarItinPriceWithSavedFPPQItemsTest, testPriceItinWithCheaperOptionsTwoPaxGFPNotPriced)
{
  Itin similarItin;

  MockPaxFarePathFactory adtPfpf(*_factoriesConfig);
  PaxType* adt = getPaxType("ADT");
  adtPfpf.paxType() = adt;
  _pfpfBucket.push_back(&adtPfpf);

  FPPQItem* adtFppqItem = getFPPQItem(adt, &similarItin, 10.0);

  EXPECT_CALL(adtPfpf, getAlreadyBuiltFPPQItem(0)).WillOnce(Return((FPPQItem*)0));
  EXPECT_CALL(adtPfpf, getFPPQItem(0, _)).WillOnce(Return(adtFppqItem));

  MockPaxFarePathFactory milPfpf(*_factoriesConfig);
  PaxType* mil = getPaxType("INF");
  milPfpf.paxType() = mil;
  _pfpfBucket.push_back(&milPfpf);

  FPPQItem* milFppqItem = getFPPQItem(mil, &similarItin, 23.0);

  EXPECT_CALL(milPfpf, getAlreadyBuiltFPPQItem(0)).WillOnce(Return((FPPQItem*)0));
  EXPECT_CALL(milPfpf, getFPPQItem(0, _)).WillOnce(Return(milFppqItem));

  EXPECT_CALL(*_validator, finalValidation()).WillOnce(Return(true)).WillOnce(Return(false));

  std::unique_ptr<GroupFarePathFactory> gfpf =
      _similarItinPricing->getGroupFarePathFactory(*_motherGFPF, _pfpfBucket);
  gfpf->usePooling() = false; // in tests using pooling causes core dumps
  ASSERT_FALSE(_similarItinPricing->priceItinWithCheaperOptions(*gfpf, similarItin));
  ASSERT_EQ(0, similarItin.farePath().size());
  ASSERT_TRUE(_diag->str().find("GROUP FARE PATH VALIDATION PASSED") == std::string::npos);
  ASSERT_TRUE(_diag->str().find("GROUP FARE PATH VALIDATION FAILED") != std::string::npos);
}

TEST_F(SimilarItinPriceWithSavedFPPQItemsTest, testPriceItinWithCheaperOptionsTwoPaxGFPPriced)
{
  Itin similarItin;

  MockPaxFarePathFactory adtPfpf(*_factoriesConfig);
  PaxType* adt = getPaxType("ADT");
  adtPfpf.paxType() = adt;
  _pfpfBucket.push_back(&adtPfpf);

  FPPQItem* adtFppqItem = getFPPQItem(adt, &similarItin, 10.0);

  EXPECT_CALL(adtPfpf, getAlreadyBuiltFPPQItem(0)).WillOnce(Return((FPPQItem*)0));
  EXPECT_CALL(adtPfpf, getFPPQItem(0, _)).WillOnce(Return(adtFppqItem));

  MockPaxFarePathFactory milPfpf(*_factoriesConfig);
  PaxType* mil = getPaxType("INF");
  milPfpf.paxType() = mil;
  _pfpfBucket.push_back(&milPfpf);

  FPPQItem* milFppqItem = getFPPQItem(mil, &similarItin, 23.0);

  EXPECT_CALL(milPfpf, getAlreadyBuiltFPPQItem(0)).WillOnce(Return((FPPQItem*)0));
  EXPECT_CALL(milPfpf, getFPPQItem(0, _)).WillOnce(Return(milFppqItem));

  EXPECT_CALL(*_validator, finalValidation()).WillOnce(Return(true)).WillOnce(Return(true));

  std::unique_ptr<GroupFarePathFactory> gfpf =
      _similarItinPricing->getGroupFarePathFactory(*_motherGFPF, _pfpfBucket);
  gfpf->usePooling() = false; // in tests using pooling causes core dumps
  ASSERT_TRUE(_similarItinPricing->priceItinWithCheaperOptions(*gfpf, similarItin));
  ASSERT_EQ(2, similarItin.farePath().size());
  ASSERT_TRUE(_diag->str().find("GROUP FARE PATH VALIDATION PASSED") != std::string::npos);
  ASSERT_FALSE(_diag->str().find("GROUP FARE PATH VALIDATION FAILED") != std::string::npos);
}
}
