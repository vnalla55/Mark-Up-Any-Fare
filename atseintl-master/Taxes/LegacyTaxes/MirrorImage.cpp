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

#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

MirrorImage::MirrorImage() : _taxSpecConfigRegs(nullptr), _shouldCheckCityMirror(false) {}

// ----------------------------------------------------------------------------
// Constructor with special configuration vector
// ----------------------------------------------------------------------------

MirrorImage::MirrorImage(const DateTime& date,
                         const std::vector<TaxSpecConfigReg*>* const taxSpecConfigRegs)
  : _taxSpecConfigRegs(taxSpecConfigRegs),
    _shouldCheckCityMirror(utc::shouldCheckCityMirror(date, _taxSpecConfigRegs))
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

MirrorImage::~MirrorImage() {}
// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
MirrorImage::isMirrorImage(const PricingTrx& trx,
                           const TaxResponse& taxResponse,
                           const TaxCodeReg& taxCodeReg,
                           uint16_t startIndex)
{
  TravelSeg* travelSegTo = getTravelSeg(taxResponse)[startIndex];
  TravelSeg* travelSegFrom = getTravelSeg(taxResponse)[startIndex];

  if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
  {
    if (travelSegFrom == getTravelSeg(taxResponse).back())
      return false;

    travelSegTo = getTravelSeg(taxResponse)[startIndex + 1];

    if (!travelSegTo->isAir())
      return false;
  }
  else
  {
    if (travelSegTo == getTravelSeg(taxResponse).front())
      return false;

    travelSegFrom = getTravelSeg(taxResponse)[startIndex - 1];

    if (!travelSegFrom->isAir())
      return false;
  }

  bool mirrorToOriginAirport = false;
  bool mirrorToOriginCity = false;
  bool mirrorToOriginNation = false;

  if (UNLIKELY(_shouldCheckCityMirror))
  {
    mirrorToOriginCity = LocUtil::isSameCity(
        travelSegFrom->origin()->loc(), travelSegTo->destination()->loc(), trx.dataHandle());
  }
  else
  {
    mirrorToOriginAirport = LocUtil::isInLoc(*travelSegFrom->origin(),
                                             LOCTYPE_CITY,
                                             travelSegTo->destination()->loc(),
                                             Vendor::SABRE,
                                             MANUAL,
                                             LocUtil::TAXES,
                                             GeoTravelType::International,
                                             EMPTY_STRING(),
                                             trx.getRequest()->ticketingDT());
  }

  const bool isSegFromInternational =
      travelSegFrom->origin()->nation() != travelSegFrom->destination()->nation();
  if (isSegFromInternational)
    mirrorToOriginNation =
        travelSegTo->destination()->nation() == travelSegFrom->origin()->nation();

  return mirrorToOriginAirport || mirrorToOriginCity || mirrorToOriginNation;
}

const std::vector<TravelSeg*>&
MirrorImage::getTravelSeg(const TaxResponse& taxResponse) const
{
  return _travelSeg ? *_travelSeg : taxResponse.farePath()->itin()->travelSeg();
}
