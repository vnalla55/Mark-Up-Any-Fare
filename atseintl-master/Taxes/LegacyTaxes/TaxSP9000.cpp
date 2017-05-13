#include "Taxes/LegacyTaxes/TaxSP9000.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

namespace tse
{

bool
TaxSP9000::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  _hiddenBrdAirport = "";
  _hiddenOffAirport = "";

  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() == LOCTYPE_NONE))
    return true;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  if ((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() != LOCTYPE_NONE))
  {
    locIt->toSegmentNo(startIndex);

    if (locIt->hasNext())
    {
      locIt->next();
      if (locIt->isInLoc2(taxCodeReg, trx))
      {
        if (locIt->isHidden())
          _hiddenOffAirport = locIt->loc()->loc();

        return true;
      }
    }

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
    return false;
  }

  bool foundStart = false;
  for (locIt->toSegmentNo(startIndex); locIt->hasNext(); locIt->next())
  {
    if (locIt->isInLoc1(taxCodeReg, trx) && locIt->hasNext() && locIt->isNextSegAirSeg())
    {
      startIndex = locIt->nextSegNo();
      endIndex = startIndex;
      foundStart = true;

      if (locIt->isHidden())
        _hiddenBrdAirport = locIt->loc()->loc();

      break;
    }
  }

  if (foundStart)
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return true;

    locIt->next();

    if (taxCodeReg.nextstopoverrestr() == YES)
      for (; locIt->hasNext() && !locIt->isStop(); locIt->next())
        ;

    if (locIt->isInLoc2(taxCodeReg, trx))
    {
      if (locIt->isHidden())
        _hiddenOffAirport = locIt->loc()->loc();

      return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
  return false;
}

bool
TaxSP9000::validateTransitOnHiddenPoints(const TaxCodeReg& taxCodeReg) const
{
  if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
  {
    if (!_hiddenOffAirport.empty())
      return true;
  }
  else
  {
    if (!_hiddenBrdAirport.empty())
      return true;
  }

  return false;
}

bool
TaxSP9000::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  if (taxCodeReg.restrictionTransit().empty())
    return true;

  if (validateTransitOnHiddenPoints(taxCodeReg))
    return taxCodeReg.restrictionTransit().front().transitTaxonly();
  else
    return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}
}
