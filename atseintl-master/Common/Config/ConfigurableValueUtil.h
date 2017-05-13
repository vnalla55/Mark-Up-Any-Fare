//----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Util/BranchPrediction.h"

namespace tse
{
class Trx;

namespace ConfigurableValueUtil
{
const Trx* getTrxFromTLS(bool checked);
void checkTrx(const Trx* trx);

inline
const Trx*
getTrx(const Trx* trx, bool checked = false)
{
 if (LIKELY(trx))
    return trx;

  return getTrxFromTLS(checked);
}
}

}
