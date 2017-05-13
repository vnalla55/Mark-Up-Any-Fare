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

#include "DataModel/RefundPenalty.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Currency.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Xform/ComparablePenaltyFee.h"

namespace tse
{
ComparablePenaltyFee&
ComparablePenaltyFee::
operator=(const ComparablePenaltyFee& rhs)
{
  PenaltyFee::operator=(rhs);
  ValidationFlags::operator=(rhs);
  return *this;
}

ComparablePenaltyFee&
ComparablePenaltyFee::
operator=(const PenaltyFee& rhs)
{
  PenaltyFee::operator=(rhs);
  return *this;
}

bool
ComparablePenaltyFee::
operator>(const PenaltyFee& rhs) const
{
  if (this == &rhs)
    return false;
  ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
  return ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(
      (PenaltyFee&)lhs, (PenaltyFee&)rhs, ((PenaltyFee&)lhs).penaltyCurrency, _trx);
}

bool
ComparablePenaltyFee::
operator>(const ComparablePenaltyFee& rhs) const
{
  if (this == &rhs)
    return false;
  ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
  return ReissuePenaltyCalculator::isPenalty1HigherThanPenalty2(
      (PenaltyFee&)lhs, (PenaltyFee&)rhs, ((PenaltyFee&)lhs).penaltyCurrency, _trx);
}

bool
ComparablePenaltyFee::
operator<(const ComparablePenaltyFee& rhs) const
{
  if (this == &rhs)
    return false;
  ComparablePenaltyFee& lhs((ComparablePenaltyFee&)*this);
  return rhs.operator>(lhs);
}

} // end of tse namespace
