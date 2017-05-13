//-------------------------------------------------------------------
//
//  File:        AirSeg.cpp
//  Created:     March 8, 2004
//  Authors:
//
//  Description: Itinerary air segment
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Initializers for new members
//
//  Copyright Sabre 2004
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

#include "DataModel/AirSeg.h"

#include "Common/MCPCarrierUtil.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"

namespace tse
{

const char AirSeg::MARRIAGE_START = 'S';
const char AirSeg::MARRIAGE_MIDDLE = 'P';
const char AirSeg::MARRIAGE_END = 'E';
const char AirSeg::MARRIAGE_NONE = ' ';

AirSeg::AirSeg()
{
  _segmentType = Air;
}

bool
AirSeg::flowJourneyCarrier() const
{
  if (UNLIKELY(!unflown()))
    return false;
  if (UNLIKELY(carrierPref() == nullptr))
    return false;
  return ((carrierPref()->flowMktJourneyType() == YES) &&
          !(journeyByMarriageCarrier() && (marriageStatus() == MARRIAGE_NONE)));
}

//-------------------------------------------------------------------
bool
AirSeg::localJourneyCarrier() const
{
  if (UNLIKELY(!unflown()))
    return false;
  if (UNLIKELY(carrierPref() == nullptr))
    return false;
  return (carrierPref()->localMktJourneyType() == YES);
}

bool
AirSeg::isSoloShoppingCarrier() const
{
  if (UNLIKELY(carrierPref() == nullptr))
    return false;

  return (carrierPref()->activateSoloShopping() == YES);
}

bool
AirSeg::isSoloPricingCarrier() const
{
  if (UNLIKELY(carrierPref() == nullptr))
    return false;

  return (carrierPref()->activateSoloPricing() == YES);
}

bool
AirSeg::journeyByMarriageCarrier() const
{
  return MCPCarrierUtil::isJourneyByMarriageCarrier(carrier());
}

bool
AirSeg::isAir() const
{
  return true;
}

AirSeg*
AirSeg::clone(DataHandle& dh) const
{
  AirSeg* as = dh.create<AirSeg>();
  *as = *this;
  return as;
}
}
