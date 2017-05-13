#include "Taxes/LegacyTaxes/TaxXG_10.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"

using namespace tse;

bool
TaxXG_10::validateTripTypes(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex)
{

  if (!validateFromTo(trx, taxResponse, taxCodeReg, startIndex, endIndex))
  {
    return false;
  }

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  locIt->toSegmentNo(startIndex);
  locIt->next();

  if (!locIt->isPrevSegAirSeg())
  {
    while (locIt->hasNext())
    {
      locIt->next();
      if (locIt->isPrevSegAirSeg())
      {
        startIndex = locIt->prevSegNo();
        break;
      }
    }
  }
  endIndex = startIndex;
  return true;
}

bool
TaxXG_10::validateXG(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  // Check for XG/XG3/XG4 taxes.
  bool geoMatch = true;
  bool originMatch = true;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  const Loc& orig = *(*travelSegI)->origin();

  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  geoMatch = LocUtil::isInLoc(orig,
                              taxCodeReg.loc1Type(),
                              taxCodeReg.loc1(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::TAXES,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              trx.getRequest()->ticketingDT());

  if ((!geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
      (geoMatch && taxCodeReg.loc1ExclInd() == YES) ||
      (!geoMatch && taxCodeReg.loc1ExclInd() != YES))
  {
    originMatch = false;
  }

  // Check for Needed to Zeroes out the Base Fare
  if (((_otherIntl) || (_hawaiianPt) || (_origUS) || (!_origCAMaritime && !_origCANotMaritime)) ||
      (!originMatch) || (taxCodeReg.taxCode() == TAX_CODE_XG3))
  {
    _zeroesBaseFare = true;
  }

  if ((taxCodeReg.taxCode() == TAX_CODE_XG4) && (_otherIntl))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Apply ONLY originating in Maritime Provinces
// ----------------------------------------------------------------------------
bool
TaxXG_10::validateXG1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  // Check if XG1 - if not Originating in CA/Maritime, no XG1 applies
  if (!_origCAMaritime)
    return false;

  // Check if XG1 - if there is no US Point, no XG1 applies
  if (!_usPoint)
    return false;

  bool geoMatch = true;
  bool originMatch = true;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  const Loc& orig = *(*travelSegI)->origin();

  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  geoMatch = LocUtil::isInLoc(orig,
                              taxCodeReg.loc1Type(),
                              taxCodeReg.loc1(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::TAXES,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              trx.getRequest()->ticketingDT());

  if ((!geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
      (geoMatch && taxCodeReg.loc1ExclInd() == YES) ||
      (!geoMatch && taxCodeReg.loc1ExclInd() != YES))
  {
    originMatch = false;
  }

  // Check for Needed to Zeroes out the Base Fare
  if ((_otherIntl) || (_hawaiianPt) || (_origUS) || (!originMatch) ||
      (!_origCAMaritime && !_origCANotMaritime))
  {
    _zeroesBaseFare = true;
  }

  return true;
}
