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

#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TaxCodeCabin.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "Common/FallbackUtil.h"

namespace tse
{

namespace
{
BookingCode
getRebookBkg(const PricingTrx& trx,
             const TaxResponse& taxResponse,
             const TravelSeg& travelSeg)
{
  if (LIKELY(trx.getRequest()->isLowFareRequested() || trx.getRequest()->isLowFareNoAvailability()))
  {
    const Itin* itin = taxResponse.farePath()->itin();

    for (const PricingUnit* pricingUnit : taxResponse.farePath()->pricingUnit())
    {
      for (const FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        uint16_t segNo = 0;
        for (const TravelSeg* travelSegI : fareUsage->travelSeg())
        {
          if ((itin->segmentOrder(travelSegI) == itin->segmentOrder(&travelSeg)))
          {
            if (segNo < fareUsage->paxTypeFare()->segmentStatus().size())
            {
              const PaxTypeFare::SegmentStatus& segStatus =
                fareUsage->paxTypeFare()->segmentStatus().at(segNo);
              if (!(segStatus._bkgCodeReBook.empty()) &&
                  (segStatus._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)))
              {
                return segStatus._bkgCodeReBook;
              }
            }
          }
          segNo++;
        }
      }
    }
  }
  return travelSeg.getBookingCode();
}

const std::string EVERY_CLASSES("*");
const CarrierCode ANYCARRIER = "";
} // namespace

CabinValidator::CabinValidator()
  : _bkgCodeRebook(EMPTY_STRING())
{
}

CabinValidator::~CabinValidator() {}

bool
CabinValidator::validateCabinRestriction(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         const TaxCodeReg& taxCodeReg,
                                         TravelSeg* travelSeg)
{
  if (taxCodeReg.cabins().empty())
    return true;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  if (!airSeg)
    return false;

  const TaxCodeCabin* taxCodeCabin = getMatchingTaxCodeCabin(*airSeg, taxCodeReg, trx, taxResponse);

  if (taxCodeCabin && taxCodeCabin->exceptInd() != YES)
    return true;

  TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CLASS_OF_SERVICE, Diagnostic820);
  return false;
}

const TaxCodeCabin*
CabinValidator::getMatchingTaxCodeCabin(const AirSeg& airSeg,
                                        const TaxCodeReg& taxCodeReg,
                                        const PricingTrx& trx,
                                        const TaxResponse& taxResponse)
{
  _bkgCodeRebook = getRebookBkg(trx, taxResponse, airSeg);

  for (const TaxCodeCabin* taxCodeCabin : taxCodeReg.cabins())
  {
    if (airSeg.carrier() != taxCodeCabin->carrier())
      continue;

    if (validateFlightandDirection(trx, taxResponse, taxCodeReg, airSeg, *taxCodeCabin))
      return taxCodeCabin;
  }

  for (const TaxCodeCabin* taxCodeCabin : taxCodeReg.cabins())
  {
    if (taxCodeCabin->carrier() != ANYCARRIER)
      continue;

    if (_bkgCodeRebook == taxCodeCabin->classOfService() ||
        taxCodeCabin->classOfService() == EVERY_CLASSES)
    {
      return taxCodeCabin;
    }
  }

  return nullptr;
}

bool
CabinValidator::validateFlightandDirection(const PricingTrx& trx,
                                           const TaxResponse& taxResponse,
                                           const TaxCodeReg& taxCodeReg,
                                           const AirSeg& airSeg,
                                           const TaxCodeCabin& taxCodeCabin) const
{
  bool boardMatch = false;
  if (taxCodeCabin.loc1().locType() != LOCTYPE_NONE)
  {
    boardMatch = LocUtil::isInLoc(*airSeg.origin(),
                                  taxCodeCabin.loc1().locType(),
                                  taxCodeCabin.loc1().loc(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());
  }

  bool offMatch = LocUtil::isInLoc(*airSeg.destination(),
                                   taxCodeCabin.loc2().locType(),
                                   taxCodeCabin.loc2().loc(),
                                   Vendor::SABRE,
                                   MANUAL,
                                   LocUtil::TAXES,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  bool fltMatch = true;
  if (taxCodeCabin.flight1() != 0 && taxCodeCabin.flight2() != 0)
  {
    fltMatch = (airSeg.marketingFlightNumber() >= taxCodeCabin.flight1() &&
                airSeg.marketingFlightNumber() <= taxCodeCabin.flight2());
  }

  bool bkgMatch = (_bkgCodeRebook == taxCodeCabin.classOfService()) ||
                  (taxCodeCabin.classOfService() == EVERY_CLASSES);
  bool isWithinCountry = ItinUtil::isDomesticOfNation(taxResponse.farePath()->itin(),
                                                      airSeg.origin()->nation());

  if (taxCodeCabin.directionalInd() == BETWEEN)
    return bkgMatch;
  else if (taxCodeCabin.directionalInd() == WITHIN)
    return UNLIKELY((taxCodeCabin.classOfService() == EVERY_CLASSES) &&
                    isWithinCountry && boardMatch);
  else if (taxCodeCabin.directionalInd() == FROM)
    return bkgMatch && boardMatch && offMatch && fltMatch;
  else
    return false;
}

}
