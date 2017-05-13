//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Rules/Penalties.h"

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class FareMarket;

class FDPenalties : public Penalties
{
public:
  FDPenalties();
  virtual ~FDPenalties();

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) override;

};

} // namespace tse

