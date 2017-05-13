// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyFacades/CopyTaxResponse.h"
#include "Taxes/LegacyFacades/ItinSelector.h"

#include "gmock/gmock.h"

namespace tse
{

class ItinSelectorMock : public ItinSelector
{
public:
  ItinSelectorMock(PricingTrx& trx) : ItinSelector(trx) {}

  MOCK_CONST_METHOD0(isExcItin, bool());
  MOCK_CONST_METHOD0(isRefundTrx, bool());
  MOCK_CONST_METHOD0(isCat33FullRefund, bool());
  MOCK_CONST_METHOD0(get, std::vector<Itin*>());
};

class ACopyTaxResponse : public testing::Test
{
public:
  PricingTrx trx;
  Itin itin;
  TaxResponse firstTaxResponse, secondTaxResponse;

  void SetUp() override
  {
    itin.mutableTaxResponses().push_back(&firstTaxResponse);
    itin.mutableTaxResponses().push_back(&secondTaxResponse);
  }
};

TEST_F(ACopyTaxResponse, IsCopyToExcItinFalseIfNotRefundTrx)
{
  ItinSelectorMock itinSelector(trx);
  CopyTaxResponse copyTaxResponse(trx, itinSelector);
  EXPECT_CALL(itinSelector, isRefundTrx()).WillOnce(testing::Return(false));

  ASSERT_FALSE(copyTaxResponse.isCopyToExcItin());
}

TEST_F(ACopyTaxResponse, IsCopyToExcItinTrueIfRefundAndExcItin)
{
  ItinSelectorMock itinSelector(trx);
  CopyTaxResponse copyTaxResponse(trx, itinSelector);
  EXPECT_CALL(itinSelector, isRefundTrx()).WillOnce(testing::Return(true));
  EXPECT_CALL(itinSelector, isExcItin()).WillOnce(testing::Return(true));

  ASSERT_TRUE(copyTaxResponse.isCopyToExcItin());
}

TEST_F(ACopyTaxResponse, IsCopyToNewItinIfNewItinAndGSA)
{
  trx.setValidatingCxrGsaApplicable(true);
  ItinSelectorMock itinSelector(trx);
  CopyTaxResponse copyTaxResponse(trx, itinSelector);

  ASSERT_TRUE(copyTaxResponse.isCopyToNewItin());
}

TEST_F(ACopyTaxResponse, IsExcItinTaxResponseSizeEqualsTo2)
{
  ItinSelectorMock itinSelector(trx);
  EXPECT_CALL(itinSelector, get()).WillOnce(testing::Return(std::vector<Itin*>{ &itin }));

  CopyTaxResponse copyTaxResponse(trx, itinSelector);
  copyTaxResponse.copyToExcItin();

  ASSERT_THAT(trx.getExcItinTaxResponse().size(), testing::Eq(2));
}

TEST_F(ACopyTaxResponse, IsNewItinTaxResponseSizeEqualsTo2)
{
  ItinSelectorMock itinSelector(trx);
  EXPECT_CALL(itinSelector, get()).WillOnce(testing::Return(std::vector<Itin*>{ &itin }));

  CopyTaxResponse copyTaxResponse(trx, itinSelector);
  copyTaxResponse.copyToNewItin();

  ASSERT_THAT(trx.taxResponse().size(), testing::Eq(2));
}

} // end of tse namespace
