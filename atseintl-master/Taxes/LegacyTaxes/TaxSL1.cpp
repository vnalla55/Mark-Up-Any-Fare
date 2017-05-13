// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/TaxSL1.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "Common/SmallBitSet.h"
#include "DataModel/Itin.h"

namespace tse
{

TaxSL1::TaxSL1() {}

TaxSL1::~TaxSL1() {}

void
TaxSL1::taxCreate(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  TaxCodeReg& taxCodeReg,
                  uint16_t travelSegStartIndex,
                  uint16_t travelSegEndIndex)
{
  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  // lint -e{530}
  SmallBitSet<uint16_t, Itin::TripCharacteristics>& tripCharacteristics =
      taxResponse.farePath()->itin()->tripCharacteristics();

  if (!tripCharacteristics.isSet(Itin::RoundTrip))
    _taxAmount = _taxAmount * 2;
}
}
