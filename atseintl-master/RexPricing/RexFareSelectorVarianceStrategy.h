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

class PreSelectedFaresStore;

class RexFareSelectorVarianceStrategy : public RexFareSelectorStrategy
{
  friend class RexFareSelectorVarianceStrategyTest;

public:
  RexFareSelectorVarianceStrategy(const RexBaseTrx& trx, PreSelectedFaresStore& store)
    : RexFareSelectorStrategy(trx), _store(store)
  {
  }

private:
  virtual bool select(FareCompInfo& fc,
                      Iterator begin,
                      Iterator end,
                      std::vector<PaxTypeFareWrapper>& selected) const override;

  virtual bool getPreSelectedFares(const FareCompInfo& fc,
                                   RexDateSeqStatus status,
                                   std::vector<PaxTypeFareWrapper>& preSelected) const override;

  PreSelectedFaresStore& _store;
};

} // tse

