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

#include "Common/RexPricingPhase.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"

#include "gmock/gmock.h"

namespace tse
{
TEST(ARexPricingPhase, IsNotExcPhaseIfNotPricingTrx)
{
  MultiExchangeTrx trx;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isExcPhase());
}

TEST(ARexPricingPhase, IsNotNewPhaseIfNotPricingTrx)
{
  MultiExchangeTrx trx;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isNewPhase());
}

TEST(ARexPricingPhase, IsExcPhaseIfRefundAndEXCITIN)
{
  RefundPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_TRUE(phase.isExcPhase());
}

TEST(ARexPricingPhase, IsNotNewPhaseIfRefundAndEXCITIN)
{
  RefundPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isNewPhase());
}

TEST(ARexPricingPhase, IsNotExcPhaseIfRefundAndNEWITIN)
{
  RefundPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isExcPhase());
}

TEST(ARexPricingPhase, IsNewPhaseIfRefundAndNEWITIN)
{
  RefundPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_TRUE(phase.isNewPhase());
}

TEST(ARexPricingPhase, IsExcPhaseIfRexPricingAndEXCITIN)
{
  RexPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_TRUE(phase.isExcPhase());
}

TEST(ARexPricingPhase, IsNotNewPhaseIfRexPricingAndEXCITIN)
{
  RexPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isNewPhase());
}

TEST(ARexPricingPhase, IsNotExcPhaseIfRexPricingAndNEWITIN)
{
  RexPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_FALSE(phase.isExcPhase());
}

TEST(ARexPricingPhase, IsNewPhaseIfRexPricingAndNEWITIN)
{
  RexPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  RexPricingPhase phase(&trx);
  ASSERT_TRUE(phase.isNewPhase());
}

} // end of tse namespace
