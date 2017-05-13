// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxBE.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"

#include <vector>
#include <algorithm>

using namespace tse;

const LocCode TaxBE::BRUSSELS_CODE = "BRU";

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
TaxBE::TaxBE() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
TaxBE::~TaxBE() {}

// ----------------------------------------------------------------------------
// Description:  validateLocRestrictions
// ----------------------------------------------------------------------------
bool
TaxBE::validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex)
{

  Itin* itin = taxResponse.farePath()->itin();
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(itin->travelSeg()[startIndex]);

  if (!airSeg)
    return false;

  bool isHiddenStop = isHiddenStopInBRU(airSeg->hiddenStops());

  if (isHiddenStop)
    endIndex = startIndex;

  return isHiddenStop;
}

// ----------------------------------------------------------------------------
// Description:  isHiddenStopInBRU
// ----------------------------------------------------------------------------
bool
TaxBE::isHiddenStopInBRU(const HiddenStopsType& hiddenStops) const
{
  return std::any_of(hiddenStops.cbegin(),
                     hiddenStops.cend(),
                     [](const Loc* locCode)
                     { return locCode->loc() == BRUSSELS_CODE; });
}
