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

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FarePath.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/ChangeFeeAmount.h"
#include "test/include/TestConfigInitializer.h"

#include "gmock/gmock.h"

namespace tse
{
class ARefundChangeFeeAmount : public testing::Test
{
public:
  TestConfigInitializer cfg;
  RefundPricingTrx trx;
  TaxResponse taxResponse;
  TaxCodeReg taxCodeReg;
  CurrencyCode currencyCode = "USD";
  RefundPermutation permutation;

  void SetUp() override
  {
    trx.fullRefund() = true;
  }
};

TEST_F(ARefundChangeFeeAmount, IsNotValidIfNoRefundPermutation)
{
  RefundChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_FALSE(changeFeeAmount.validate());
}

TEST_F(ARefundChangeFeeAmount, IsNotValidIfTotalPenaltyIsZero)
{
  trx.setFullRefundWinningPermutation(permutation);

  RefundChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_FALSE(changeFeeAmount.validate());
}

TEST_F(ARefundChangeFeeAmount, IsValidIfTotalPenaltyIsGreaterThanEPSILON)
{
  permutation.totalPenalty() = Money(100, "PLN");
  trx.setFullRefundWinningPermutation(permutation);

  RefundChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_TRUE(changeFeeAmount.validate());
}

TEST_F(ARefundChangeFeeAmount, IsAmountEqualToZeroIfNotValid)
{
  RefundChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_THAT(changeFeeAmount.getAmountInPaymentCurrency(), testing::DoubleEq(0));
}

TEST_F(ARefundChangeFeeAmount, IsAmount100IfNoCurrencyConversion)
{
  permutation.totalPenalty() = Money(100, "USD");
  trx.setFullRefundWinningPermutation(permutation);

  RefundChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_THAT(changeFeeAmount.getAmountInPaymentCurrency(), testing::DoubleEq(100));
}


class AReissueChangeFeeAmount : public testing::Test
{
public:
  PricingTrx trx;
  TaxResponse taxResponse;
  TaxCodeReg taxCodeReg;
  CurrencyCode currencyCode = "USD";
  FarePath farePath;
  ReissueCharges reissueCharges;

  void SetUp() override
  {
    taxResponse.farePath() = &farePath;
  }
};

TEST_F(AReissueChangeFeeAmount, IsNotValidIfChangeFeeEqualsToZero)
{
  ReissueChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_FALSE(changeFeeAmount.validate());
}

TEST_F(AReissueChangeFeeAmount, IsNotValidIfIgnoreReissueChargesTrue)
{
  reissueCharges.changeFee = 100;
  farePath.reissueCharges() = &reissueCharges;
  farePath.ignoreReissueCharges() = true;

  ReissueChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_FALSE(changeFeeAmount.validate());
}

TEST_F(AReissueChangeFeeAmount, IsValidIfChangeFeeNonZeroAndNotIgnoreReissueChanges)
{
  reissueCharges.changeFee = 100;
  farePath.reissueCharges() = &reissueCharges;
  farePath.ignoreReissueCharges() = false;

  ReissueChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_TRUE(changeFeeAmount.validate());
}

TEST_F(AReissueChangeFeeAmount, IsAmountEqualToZeroIfNotValid)
{
  ReissueChangeFeeAmount changeFeeAmount(trx, taxResponse, taxCodeReg, currencyCode);
  ASSERT_THAT(changeFeeAmount.getAmountInPaymentCurrency(), testing::DoubleEq(0));
}

} // end of tse namespace
