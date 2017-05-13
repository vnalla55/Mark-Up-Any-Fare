// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxOI_01.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DBAccess/Loc.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/TravelSeg.h"

using namespace tse;

bool
TaxOI_01::validateLocRestrictions(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t& startIndex,
                                  uint16_t& endIndex)
{
  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[startIndex];

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return false;

  if (taxUtil::hasHiddenStopInLoc(airSeg, LOC_NRT) &&
      !TravelSegUtil::NationEqual()(travelSeg, JAPAN))
  {
    endIndex = startIndex;
    return true;
  }
  else
    return Tax::validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxOI_01::validateTripTypes(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex)

{

  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[startIndex];

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return false;

  if (taxUtil::hasHiddenStopInLoc(airSeg, LOC_NRT) &&
      !TravelSegUtil::NationEqual()(travelSeg, JAPAN))
    return true;
  else
    return Tax::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxOI_01::validateTransit(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegIndex)
{
  bool isTransit = Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (travelSegIndex >= 1)
  {
    TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];
    TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

    if (dynamic_cast<const AirSeg*>(travelSegFrom) && dynamic_cast<const AirSeg*>(travelSegTo))
    {
      if (taxUtil::isTransitSeq(taxCodeReg))
        isTransit = validateTransitSeq(isTransit, taxCodeReg, travelSegFrom, travelSegTo);
      else
        isTransit = validateStopoverSeq(isTransit, taxCodeReg, travelSegFrom, travelSegTo);
    }
  }

  if (!isTransit)
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic813);

  return isTransit;
}

bool
TaxOI_01::validateCarrierExemption(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t travelSegIndex)
{
  bool isValid = false;

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];
  TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if (travelSegIndex > 0)
    travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

  if (travelSegIndex == 0)
    isValid = Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, travelSegIndex);
  else if (taxCodeReg.seqNo() != 300)
  {

    bool isInternationalSeg = !(TravelSegUtil::OriginNationEqual()(travelSegFrom, JAPAN) &&
                                TravelSegUtil::DestinationNationEqual()(travelSegFrom, JAPAN));

    if (isInternationalSeg)
      isValid = true;
    else
      isValid = Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, travelSegIndex - 1);

    if (!isValid)
    {
      bool isSameDay = validateSameDayRestrictions(taxCodeReg, travelSegFrom, travelSegTo);

      if (!travelSegFrom->isForcedConx())
        if (travelSegFrom->isForcedStopOver() || !isSameDay)
          isValid = true;
    }
  }
  else // for seq #300
  {

    isValid = Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, travelSegIndex);
    bool isSameDay = validateSameDayRestrictions(taxCodeReg, travelSegFrom, travelSegTo);

    if (isValid)
    {
      if (travelSegFrom->isForcedStopOver())
        isValid = true;
      else if (isSameDay || travelSegFrom->isForcedConx())
        isValid = false;
    }
  }

  if (!isValid)
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CARRIER_EXEMPTION, Diagnostic821);

  return isValid;
}

bool
TaxOI_01::validateSameDayRestrictions(TaxCodeReg& taxCodeReg,
                                      TravelSeg* travelSegFrom,
                                      TravelSeg* travelSegTo) const
{

  bool isValid = false;
  bool sameDayInd = false;

  if (!taxCodeReg.restrictionTransit().empty())
    sameDayInd = (taxCodeReg.restrictionTransit().front().sameDayInd() == YES);

  bool isSameDay = (travelSegTo->departureDT().day() == travelSegFrom->arrivalDT().day() &&
                    travelSegTo->departureDT().month() == travelSegFrom->arrivalDT().month());

  if (sameDayInd || taxCodeReg.seqNo() == 300)
    isValid = isSameDay;

  return isValid;
}

bool
TaxOI_01::validateTransitSeq(bool isTransit,
                             const TaxCodeReg& taxCodeReg,
                             TravelSeg* travelSegFrom,
                             TravelSeg* travelSegTo) const
{
  bool isValid = isTransit;

  if (travelSegFrom->isForcedStopOver() ||
      (travelSegTo->segmentType() == Open || travelSegFrom->segmentType() == Open))
    isValid = false;
  else if (travelSegFrom->isForcedConx())
    isValid = true;

  return isValid;
}

bool
TaxOI_01::validateStopoverSeq(bool isTransit,
                              const TaxCodeReg& taxCodeReg,
                              TravelSeg* travelSegFrom,
                              TravelSeg* travelSegTo) const
{
  bool isValid = isTransit;

  if (taxCodeReg.seqNo() == 300)
    isValid = true;
  else if (travelSegFrom->isForcedConx())
    isValid = false;
  else if (travelSegFrom->isForcedStopOver() ||
           (travelSegTo->segmentType() == Open || travelSegFrom->segmentType() == Open))
    isValid = true;

  return isValid;
}
