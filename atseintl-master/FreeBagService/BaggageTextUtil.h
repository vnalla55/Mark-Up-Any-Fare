//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "Common/TrxUtil.h"

namespace tse
{
namespace BaggageTextUtil
{

// TODO: Move such kind of logic from response/diagnostic serializers
inline bool
isMarketingCxrUsed(const PricingTrx& trx, bool usDot, bool defer)
{
  if (usDot)
    return true;
  if (TrxUtil::isIataReso302MandateActivated(trx))
    return !defer;
  return defer;
}

}
} // ns tse

