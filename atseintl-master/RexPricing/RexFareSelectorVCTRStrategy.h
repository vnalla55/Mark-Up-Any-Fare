//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "RexPricing/RexFareSelectorStrategy.h"

namespace tse
{

class RexFareSelectorVCTRStrategy : public RexFareSelectorStrategy
{
  friend class RexFareSelectorVCTRStrategyTest;

public:
  RexFareSelectorVCTRStrategy(const RexBaseTrx& trx) : RexFareSelectorStrategy(trx) {}

protected:
  virtual bool select(FareCompInfo& fc,
                      Iterator begin,
                      Iterator end,
                      std::vector<PaxTypeFareWrapper>& selected) const override;

private:
  template <typename VctrSelector>
  bool selectStep(VctrSelector vctrSelector,
                  FareCompInfo& fc,
                  Iterator begin,
                  Iterator end,
                  std::vector<PaxTypeFareWrapper>& selected) const;
};

} // tse

