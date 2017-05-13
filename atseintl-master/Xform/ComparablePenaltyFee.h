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

#include "DataModel/ReissueCharges.h"

namespace tse
{
class RefundPenalty;
class RexPricingTrx;

struct ValidationFlags
{
  mutable bool highest;
  mutable bool notApplicable;
  mutable bool waived;

  ValidationFlags() : highest(false), notApplicable(false), waived(false) {}
};
class ComparablePenaltyFee : public PenaltyFee, public ValidationFlags
{
  RexPricingTrx& _trx;

public:
  virtual ~ComparablePenaltyFee() {}

  ComparablePenaltyFee(RexPricingTrx& trx) : _trx(trx) {}

  ComparablePenaltyFee(const ComparablePenaltyFee& rhs, RexPricingTrx& trx)
    : PenaltyFee(rhs), ValidationFlags(rhs), _trx(trx)
  {
  }

  ComparablePenaltyFee(const ComparablePenaltyFee& rhs)
    : PenaltyFee(rhs), ValidationFlags(rhs), _trx(rhs._trx)
  {
  }

  ComparablePenaltyFee(const PenaltyFee& rhs, RexPricingTrx& trx) : PenaltyFee(rhs), _trx(trx) {}
  ComparablePenaltyFee& operator=(const ComparablePenaltyFee& rhs);
  ComparablePenaltyFee& operator=(const PenaltyFee& rhs);
  bool operator>(const PenaltyFee& rhs) const;
  bool operator>(const ComparablePenaltyFee& rhs) const;
  bool operator<(const ComparablePenaltyFee& rhs) const;
};

} // end of tse namespace
