// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/FLBVisitor.h"

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
void
FLBVisitor::apply(ShoppingTrx& trx, FarePath& fp, FLBVisitor& visitor)
{
  for (PricingUnit* pu : fp.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare& ptf = *fu->paxTypeFare();
      const uint16_t legIndex = ptf.fareMarket()->legIndex();
      const PaxTypeFare::FlightBitmap& flbm = ptf.flightBitmap();
      const ApplicableSOP* asops = ptf.fareMarket()->getApplicableSOPs();
      TSE_ASSERT(asops != nullptr);

      for (const ApplicableSOP::value_type& cxrSops : *asops)
      {
        const ItinIndex::Key carrierKey = cxrSops.first;
        const SOPUsages& sopUsages = cxrSops.second;

        ptf.setComponentValidationForCarrier(carrierKey, trx.isAltDates(), trx.mainDuration());

        if (UNLIKELY(trx.isIataFareSelectionApplicable()))
          TSE_ASSERT(ptf.flightBitmap().size() == sopUsages.size() || ptf.flightBitmap().empty());
        else
          TSE_ASSERT(flbm.size() == sopUsages.size() || flbm.empty());

        for (size_t flbIdx = 0; flbIdx < sopUsages.size(); ++flbIdx)
        {
          const WhatNext cont = visitor.visit(ptf, legIndex, flbIdx, carrierKey, sopUsages);

          if (cont == CONTINUE)
            continue;
          if (cont == NEXT_PTF)
            break;
          if (LIKELY(cont == STOP))
            return;

          TSE_ASSERT(!"logic error");
        }
      }
    }
  }
}

} // ns tse
