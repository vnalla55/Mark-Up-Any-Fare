// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/TseConsts.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/Common/ReissueTaxInfoBuilder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
using testing::_;
using testing::Return;
using testing::ReturnRef;

class ReissueTaxInfoBuilderTest : public testing::Test
{
protected:
  class MyDataHandle : public DataHandleMock
  {
  public:
    MOCK_METHOD2(getTaxReissue, std::vector<TaxReissue*>& (const TaxCode&, const DateTime&));
    MOCK_METHOD2(getLoc, Loc*(const LocCode&, const DateTime&));
  };

public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandle>();

    DateTime date(2014, 1, 1);
    _date = _memHandle.create<DateTime>(date);

    _request = _memHandle.create<PricingRequest>();
    _request->ticketingDT() = *_date;

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_request);
  }

  void TearDown()
  {
    _memHandle.clear();
  }

protected:
  TestMemHandle _memHandle;
  MyDataHandle* _dataHandle;
  DateTime* _date;
  PricingRequest* _request;
  PricingTrx* _trx;
};

TEST_F(ReissueTaxInfoBuilderTest, testReissueTaxInfo_setDefaultValues)
{
  ReissueTaxInfoBuilder::ReissueTaxInfo info;
  info.setDefaultValues();

  ASSERT_FALSE(info.reissueRestrictionApply);
  ASSERT_FALSE(info.taxApplyToReissue);
  ASSERT_TRUE(info.reissueTaxRefundable);
  ASSERT_EQ(tse::CurrencyCode(""), info.reissueTaxCurrency);
  ASSERT_EQ(tse::MoneyAmount(0), info.reissueTaxAmount);
}

TEST_F(ReissueTaxInfoBuilderTest, testReissueTaxInfo_setMatchedValues)
{
  TaxReissue taxReissue;
  taxReissue.refundInd() = NO;
  taxReissue.currencyCode() = "NOR";
  taxReissue.taxAmt() = tse::MoneyAmount(14);

  ReissueTaxInfoBuilder::ReissueTaxInfo info;
  info.setMatchedValues(taxReissue);

  ASSERT_TRUE(info.taxApplyToReissue);
  ASSERT_FALSE(info.reissueTaxRefundable);
  ASSERT_EQ(tse::CurrencyCode("NOR"), info.reissueTaxCurrency);
  ASSERT_EQ(tse::MoneyAmount(14), info.reissueTaxAmount);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_ExchangePricingTrx)
{
  ExchangePricingTrx exTrx;
  exTrx.reqType() = AGENT_PRICING_MASK;

  const TaxCode taxCode;

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(exTrx, taxCode);
  ASSERT_FALSE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_FALSE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_taxReissueVecEmpty)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_FALSE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_FALSE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_matchEmptyRestrictions)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxReissue taxReissue;
  taxReissue.taxType() = "000";
  taxReissueVec.push_back(&taxReissue);

  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_TRUE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_TRUE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_matchReissueLocation)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxReissue taxReissue;
  taxReissue.reissueLoc() = "DFW";
  taxReissue.reissueLocType() = LocType(LOCTYPE_AIRPORT);
  taxReissue.taxType() = "000";
  taxReissueVec.push_back(&taxReissue);

  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  LocCode locCode = "NOTEMPTY";
  EXPECT_CALL(*_dataHandle, getLoc(locCode, *_date)).WillOnce(Return(loc));

  _request->ticketPointOverride() = locCode;  //for LocUtil::isInLoc

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_TRUE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_TRUE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_matchReissueLocation_excluded)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxReissue taxReissue;
  taxReissue.reissueLoc() = "DFW";
  taxReissue.taxType() = "000";
  taxReissue.reissueExclLocInd() = YES;
  taxReissue.reissueLocType() = LocType(LOCTYPE_AIRPORT);
  taxReissueVec.push_back(&taxReissue);

  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  LocCode locCode = "NOTEMPTY";
  EXPECT_CALL(*_dataHandle, getLoc(locCode, *_date)).WillOnce(Return(loc));

  _request->ticketPointOverride() = locCode;  //for LocUtil::isInLoc

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_TRUE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_FALSE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_matchTicketingCarrier)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxReissue taxReissue;
  taxReissue.taxType() = "000";
  taxReissueVec.push_back(&taxReissue);

  taxReissue.validationCxr().push_back("AB");
  _request->validatingCarrier() = "AB";

  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_TRUE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_TRUE(reissueTaxInfo.taxApplyToReissue);
}

TEST_F(ReissueTaxInfoBuilderTest, testBuild_matchTicketingCarrier_excluded)
{
  std::vector<TaxReissue*> taxReissueVec;
  TaxReissue taxReissue;
  taxReissue.tktlCxrExclInd() = YES;
  taxReissue.taxType() = "000";
  taxReissueVec.push_back(&taxReissue);

  taxReissue.validationCxr().push_back("AB");
  _request->validatingCarrier() = "AB";

  TaxCode taxCode;
  EXPECT_CALL(*_dataHandle, getTaxReissue(taxCode, *_date)).WillOnce(ReturnRef(taxReissueVec));

  const ReissueTaxInfoBuilder::ReissueTaxInfo reissueTaxInfo = ReissueTaxInfoBuilder().build(*_trx, taxCode);
  ASSERT_TRUE(reissueTaxInfo.reissueRestrictionApply);
  ASSERT_FALSE(reissueTaxInfo.taxApplyToReissue);
}

} // namespace tse
