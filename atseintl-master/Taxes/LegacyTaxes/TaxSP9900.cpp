// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Taxes/LegacyTaxes/TaxSP9900.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace tse
{

bool
TaxSP9900::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)
{
  if (!_sp99Processed)
  {
     _sp99Restriction = false;

     TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
     locIt->setSkipHidden(true);

     locIt->toSegmentNo(0);
     while (!locIt->isEnd())
     {
        if ((locIt->isStop() || locIt->isMirror())
            && locIt->loc()->loc() == "MPN")
        {
            _sp99Restriction = true;
        }
        locIt->next();
     }
     _sp99Processed = true;
  }

  if (_sp99Restriction)
     return false;
  else
     return Tax::validateLocRestrictions(trx, taxResponse,  taxCodeReg, startIndex, endIndex);
}

} /* end tse namespace */
