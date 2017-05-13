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

#include "Common/XMLConstruct.h"
#include "DataModel/PricingTrx.h"
#include "Xform/ReissueExchangeFormatter.h"
#include "Xform/ReissueExchangeModel.h"

#include "gmock/gmock.h"

namespace tse
{
class ChangeFeeModelMock : public AbstractChangeFeeModel
{
public:
  ChangeFeeModelMock(PricingTrx& trx) : AbstractChangeFeeModel(trx) {}
  MOCK_CONST_METHOD0(isSkipped, bool());
  MOCK_CONST_METHOD0(getAmount, MoneyAmount());
  MOCK_CONST_METHOD0(getPrecision, CurrencyNoDec());
  MOCK_CONST_METHOD0(getAmountInPaymentCurrency, MoneyAmount());
  MOCK_CONST_METHOD0(getPaymentPrecision, CurrencyNoDec());
  MOCK_CONST_METHOD0(isZeroFee, bool());
  MOCK_CONST_METHOD0(isNonZeroFee, bool());
  MOCK_CONST_METHOD0(isChangeFeeNotApplicable, bool());
  MOCK_CONST_METHOD0(isChangeFeeWaived, bool());
  MOCK_CONST_METHOD0(isHighestChangeFeeIndicator, bool());
  MOCK_CONST_METHOD0(getChangeFeeCurrency, CurrencyCode());
  MOCK_CONST_METHOD0(getPaymentCurrency, CurrencyCode());
  MOCK_CONST_METHOD0(isRefundable, bool());
};

class AReissueExchangeFormatter : public testing::Test
{
public:
  PricingTrx trx;
  XMLConstruct construct;
  ReissueExchangeFormatter formatter;

  AReissueExchangeFormatter() : formatter(trx, construct) {}
};

TEST_F(AReissueExchangeFormatter, IsBlankResultIfIsSkipped)
{
  ChangeFeeModelMock model(trx);
  EXPECT_CALL(model, isSkipped()).WillOnce(testing::Return(true));
  formatter.formatChangeFee(model);
  ASSERT_THAT(construct.getXMLData(), testing::Eq(""));
}

TEST_F(AReissueExchangeFormatter, IsResultEqualToExpectedIfNotSkipped)
{
  ChangeFeeModelMock model(trx);

  EXPECT_CALL(model, isSkipped()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, getAmount()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, isZeroFee()).WillRepeatedly(testing::Return(false));
  EXPECT_CALL(model, isNonZeroFee()).WillRepeatedly(testing::Return(false));
  EXPECT_CALL(model, getChangeFeeCurrency()).WillOnce(testing::Return(CurrencyCode("USD")));
  EXPECT_CALL(model, isHighestChangeFeeIndicator()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, isChangeFeeNotApplicable()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, isChangeFeeWaived()).WillOnce(testing::Return(false));

  formatter.formatChangeFee(model);
  ASSERT_THAT(construct.getXMLData(),
              testing::Eq("<CHG C77=\"200.00\" PXL=\"F\" PXK=\"F\" C78=\"USD\" PXJ=\"F\"/>"));
}

} // end of tse namespace
