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

#include "Taxes/LegacyTaxes/TaxGB01.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"


namespace tse
{
  FIXEDFALLBACK_DECL(fallbackGBTaxEquipmentException);
}

using namespace tse;
using namespace std;

const uint16_t GREAT_BRITIAN_GB3701 = 3701;
const uint16_t GREAT_BRITIAN_GB3801 = 3801;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxGB01::TaxGB01()
{
  _nLowCabinTaxCodeSPN = GREAT_BRITIAN_GB3701;
  _nHighCabinTaxCodeSPN = GREAT_BRITIAN_GB3801;
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxGB01::~TaxGB01() {}

bool
TaxGB01::validateLocRestrictions(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex)
{
  return TaxGB::validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxGB01::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  return TaxGB::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxGB01::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)

{
  return TaxGB::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxGB01::isStopOver(const TravelSeg* pTvlSeg, const TravelSeg* pTvlSegPrev, bool isTravelWithinUK)
{
  // always return stop over if equipment
  if(fallback::fixed::fallbackGBTaxEquipmentException())
  {
    const std::string equipment("TRN,TGV,ICE");
    size_t fpos = std::string::npos;

    if (!(pTvlSegPrev->equipmentType().empty()) &&
        (fpos = equipment.find(pTvlSegPrev->equipmentType())) != std::string::npos)
      return true;
  }
  else if (TaxGB::isSpecialEquipment(pTvlSegPrev->equipmentType()))
    return true;

  DateTime dtDepart = pTvlSeg->departureDT();
  DateTime dtArrival = pTvlSegPrev->arrivalDT();

  DateTime nextDay = pTvlSegPrev->arrivalDT().addDays(1);

  int32_t departHr = dtDepart.hours();
  int32_t departMin = dtDepart.minutes();
  int64_t ttlDepartMin = ((departHr * 60) + departMin); // lint !e647

  int32_t arrivalHr = dtArrival.hours();
  int32_t arrivalMin = dtArrival.minutes();
  int64_t ttlArrivalMin = (arrivalHr * 60) + arrivalMin; // lint !e647

  // Data 1700 hours and 1000 hours Check
  int32_t seventeenHr = 17;
  int32_t seventeenMin = 0;
  int64_t ttlSeventeenMin = ((seventeenHr * 60) + seventeenMin); // lint !e647

  int32_t tenHr = 10;
  int32_t tenMin = 0;
  int64_t ttlTenMin = ((tenHr * 60) + tenMin); // lint !e647

  int64_t connectTime = 0;
  int64_t connectionMin = 0;

  connectTime = DateTime::diffTime(pTvlSeg->departureDT(), pTvlSegPrev->arrivalDT());
  connectionMin = (connectTime / 60);

  int32_t transitTotalMinDom = 360; // 6 hours = 360 mins = 21600 secs
  int32_t transitTotalMinIntl = 1440; // 24 hours = 1440 mins = 86400 secs

  if (isTravelWithinUK) // checek 6 hours
  {
    if ((connectionMin > transitTotalMinDom) &&
        !((ttlArrivalMin > ttlSeventeenMin) && (ttlDepartMin < ttlTenMin) &&
          (pTvlSeg->departureDT().day() == nextDay.day() &&
           pTvlSeg->departureDT().month() == nextDay.month())))
      return true;
    else
      return false;
  }
  else
  {
    if ((connectionMin > transitTotalMinIntl))
      return true;
    else
      return false;
  }

  return false;
}

bool
TaxGB01::isForcedStopOver(const TravelSeg* pTvlSeg, const TravelSeg* pTvlSegPrev)
{
  if (pTvlSegPrev->isForcedStopOver() || pTvlSegPrev->segmentType() == Open ||
      pTvlSeg->segmentType() == Open)
    return true;

  return false;
}
