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

#include "Common/Money.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RexPricingTrx.h"
#include "Xform/ComparablePenaltyFee.h"
#include "Xform/ReissueExchangeModel.h"

#include "gmock/gmock.h"

namespace tse
{
class AChangeFeeModel : public testing::Test
{
public:
  RexPricingTrx trx;
  ComparablePenaltyFee fee;
  ChangeFeeModel model;

  AChangeFeeModel() : fee(trx), model(fee, trx) {}
};

TEST_F(AChangeFeeModel, IsSkipped)
{
  ASSERT_FALSE(model.isSkipped());
}

TEST_F(AChangeFeeModel, IsAmountEqualToExpected)
{
  fee.penaltyAmount = 123.45;
  ASSERT_THAT(model.getAmount(), testing::DoubleEq(123.45));
}

TEST_F(AChangeFeeModel, IsZeroFee)
{
  ASSERT_FALSE(model.isZeroFee());
}

TEST_F(AChangeFeeModel, IsNonZeroFee)
{
  ASSERT_FALSE(model.isNonZeroFee());
}

TEST_F(AChangeFeeModel, IsChangeFeeNotApplicable)
{
  fee.notApplicable = false;
  ASSERT_FALSE(model.isChangeFeeNotApplicable());
}

TEST_F(AChangeFeeModel, IsChangeFeeWaived)
{
  fee.waived = true;
  ASSERT_TRUE(model.isChangeFeeWaived());
}

TEST_F(AChangeFeeModel, IsHighestChangeFeeIndicator)
{
  fee.highest = false;
  ASSERT_FALSE(model.isHighestChangeFeeIndicator());
}

TEST_F(AChangeFeeModel, IsChangeFeeCurrencyEqualToExpected)
{
  fee.penaltyCurrency = "USD";
  ASSERT_EQ(model.getChangeFeeCurrency(), "USD");
}

class AZeroFeeChangeFeeModel : public testing::Test
{
public:
  PricingTrx trx;
  RefundPermutation permutation;
  ZeroFeeChangeFeeModel model;

  AZeroFeeChangeFeeModel() : model(permutation, trx) {}
};

TEST_F(AZeroFeeChangeFeeModel, IsSkipped)
{
  ASSERT_FALSE(model.isSkipped());
}

TEST_F(AZeroFeeChangeFeeModel, IsAmountEqualToExpected)
{
  permutation.totalPenalty() = Money(10, "USD");
  ASSERT_THAT(model.getAmount(), testing::DoubleEq(10));
}

TEST_F(AZeroFeeChangeFeeModel, IsPrecisionEqualToExpected)
{
  ASSERT_THAT(model.getPrecision(), 2);
}

TEST_F(AZeroFeeChangeFeeModel, IsZeroFee)
{
  ASSERT_TRUE(model.isZeroFee());
}

TEST_F(AZeroFeeChangeFeeModel, IsNonZeroFee)
{
  ASSERT_FALSE(model.isNonZeroFee());
}

TEST_F(AZeroFeeChangeFeeModel, IsChangeFeeNotApplicable)
{
  permutation.waivedPenalty() = false;
  ASSERT_TRUE(model.isChangeFeeNotApplicable());
}

TEST_F(AZeroFeeChangeFeeModel, IsChangeFeeWaived)
{
  permutation.waivedPenalty() = true;
  ASSERT_TRUE(model.isChangeFeeWaived());
}

TEST_F(AZeroFeeChangeFeeModel, IsHighestChangeFeeIndicator)
{
  ASSERT_TRUE(model.isHighestChangeFeeIndicator());
}

TEST_F(AZeroFeeChangeFeeModel, IsChangeFeeCurrencyEqualToExpected)
{
  permutation.totalPenalty() = Money(10, "PLN");
  ASSERT_THAT(model.getChangeFeeCurrency(), testing::Eq("PLN"));
}

TEST_F(AZeroFeeChangeFeeModel, IsRefundable)
{
  ASSERT_TRUE(model.isRefundable());
}

class ANonZeroFeeChangeFeeModel : public testing::Test
{
public:
  PricingTrx trx;
  RefundPenalty::Fee fee;
  NonZeroFeeChangeFeeModel model;

  ANonZeroFeeChangeFeeModel() : fee(Money(10, "USD")), model(fee, trx) {}
};

TEST_F(ANonZeroFeeChangeFeeModel, IsSkipped)
{
  ASSERT_FALSE(model.isSkipped());
}

TEST_F(ANonZeroFeeChangeFeeModel, IsAmountEqualToExpected)
{
  ASSERT_THAT(model.getAmount(), testing::DoubleEq(10));
}

TEST_F(ANonZeroFeeChangeFeeModel, IsPrecisionEqualToExpected)
{
  ASSERT_THAT(model.getPrecision(), 2);
}

TEST_F(ANonZeroFeeChangeFeeModel, IsZeroFee)
{
  ASSERT_FALSE(model.isZeroFee());
}

TEST_F(ANonZeroFeeChangeFeeModel, IsNonZeroFee)
{
  ASSERT_TRUE(model.isNonZeroFee());
}

TEST_F(ANonZeroFeeChangeFeeModel, IsHighestChangeFeeIndicator)
{
  ASSERT_FALSE(model.isHighestChangeFeeIndicator());
}

TEST_F(ANonZeroFeeChangeFeeModel, IsChangeFeeCurrencyEqualToExpected)
{
  ASSERT_THAT(model.getChangeFeeCurrency(), testing::Eq("USD"));
}

TEST_F(ANonZeroFeeChangeFeeModel, isRefundable)
{
  ASSERT_TRUE(model.isRefundable());
}

} // end of tse namespace
