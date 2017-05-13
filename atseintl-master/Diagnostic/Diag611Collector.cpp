//----------------------------------------------------------------------------
//  Copyright Sabre 2006
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
#include "Diagnostic/Diag611Collector.h"

#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"

#include <iomanip>

namespace tse
{
void
Diag611Collector::printHeader()
{
  if (_active)
  {
    if (nullptr == dynamic_cast<NoPNRPricingTrx*>(_trx))
    {
      ((DiagCollector&)*this)
          << "********* WPA HALF ROUND TRIP COMBINATION ANALYSIS ************\n";
    }
    else
    {
      ((DiagCollector&)*this) << "********* WQ HALF ROUND TRIP COMBINATION ANALYSIS ************\n";
    }
  }
}
}
