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

#include "Taxes/LegacyTaxes/TaxUS2.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "Common/RtwUtil.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/Common/PartialTaxableFare.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{
Logger TaxUS2::_logger("atseintl.Taxes.TaxUS2");

const char*
TaxUS2::TAX_CODE_US1("US1");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxUS2::TaxUS2()
  : _bufferZone(false),
    _stopOverUS(false),
    _furthestPointUS(false),
    _tripOriginUS(false),
    _terminateUS(false),
    _validUSPoint(false),
    _validInternationalPoint(false),
    _validCanadaPoint(false),
    _validMexicoPoint(false),
    _validAKHIPoint(false),
    _mostDistantUSInitialized(false),
    _mostDistantUS(true),
    _pointOfSaleLocation(nullptr)

{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxUS2::~TaxUS2() {}

// ----------------------------------------------------------------------------
// Description:  applyUS2
// ----------------------------------------------------------------------------

void
TaxUS2::applyUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (!validUS2(trx, taxResponse, taxCodeReg))
    return;

  if (!validateTicketDesignator(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return;

  if (!validateFareClass(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return;

  if (LocUtil::isUS(*_pointOfSaleLocation) && (!LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation)))
  {
    processUS2(trx, taxResponse, taxCodeReg);
    return;
  }

  if (_tripOriginUS && _furthestPointUS)
  {
    processUS2(trx, taxResponse, taxCodeReg);
    return;
  }

  internationalFromUS(trx, taxResponse, taxCodeReg);

  internationalToUS(trx, taxResponse, taxCodeReg);
}
// ----------------------------------------------------------------------------
// Description:  validUS2
// ----------------------------------------------------------------------------

bool
TaxUS2::validUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  bool whollyCanada = true;
  bool locAlaskaHawaii = false;
  const AirSeg* airSeg;

  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
    {
      if (LocUtil::isUS(*(*travelSegI)->destination()))
        _furthestPointUS = true;

      continue;
    }

    if (airSeg->origin()->bufferZoneInd() && airSeg->destination()->bufferZoneInd())
      _bufferZone = true;

    if (LocUtil::isUS(*airSeg->origin()) && !(LocUtil::isUSTerritoryOnly(*airSeg->origin())))
      _validUSPoint = true;

    if (LocUtil::isAlaska(*airSeg->origin()) || LocUtil::isHawaii(*airSeg->origin()))
      locAlaskaHawaii = true;

    if (LocUtil::isUS(*airSeg->destination()) &&
        !(LocUtil::isUSTerritoryOnly(*airSeg->destination())))
      _validUSPoint = true;

    if (LocUtil::isAlaska(*airSeg->destination()) || LocUtil::isHawaii(*airSeg->destination()))
      locAlaskaHawaii = true;

    if (!LocUtil::isUS(*airSeg->origin()) && !airSeg->origin()->bufferZoneInd())
      _validInternationalPoint = true;

    if (!LocUtil::isUS(*airSeg->destination()) && !airSeg->destination()->bufferZoneInd())
      _validInternationalPoint = true;

    if (LocUtil::isUSTerritoryOnly(*airSeg->origin()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
      _validInternationalPoint = true;

    if (!LocUtil::isCanada(*airSeg->origin()) || !LocUtil::isCanada(*airSeg->destination()))
      whollyCanada = false;

    if (LocUtil::isCanada(*airSeg->origin()) || LocUtil::isCanada(*airSeg->destination()))
      _validCanadaPoint = true;

    if (LocUtil::isMexico(*airSeg->origin()) || LocUtil::isMexico(*airSeg->destination()))
      _validMexicoPoint = true;

    if (!LocUtil::isUS(*airSeg->destination()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
      continue;

    if (airSeg->isFurthestPoint(*itin))
      _furthestPointUS = true;
  }

  if (whollyCanada)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::LOCATION_RESTRICTION, Diagnostic820);
    return false;
  }

  travelSegI = taxResponse.farePath()->itin()->travelSeg().end() - 1;

  if (LocUtil::isUS(*(*travelSegI)->destination()) &&
      !(LocUtil::isUSTerritoryOnly(*(*travelSegI)->destination())))
    _terminateUS = true;

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  if (LocUtil::isUS(*(*travelSegI)->origin()) &&
      !LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin()))
    _tripOriginUS = true;

  _pointOfSaleLocation = TrxUtil::saleLoc(trx);

  if (LocUtil::isUS(*_pointOfSaleLocation) && (!LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation)))
  {
    if ((!_validUSPoint || !_validInternationalPoint) && !locAlaskaHawaii)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::LOCATION_RESTRICTION, Diagnostic820);
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  processUS2
// ----------------------------------------------------------------------------

void
TaxUS2::processUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  bool boardUS = false;
  bool offUS = false;
  const AirSeg* airSeg;

  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI;
  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  bool originUS = (LocUtil::isUS(*(*travelSegI)->origin()) &&
                   !LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin()));

  // Mandate log PL#103610 originating inside buffer zone with nostopover to Hawaii will now charge.
  // PCC-AADFW PNR: YYC-x/SFO-x/KOA

  bool CAMXBufferZone = (LocUtil::isCanada(*(*travelSegI)->origin()) ||
                         LocUtil::isMexico(*(*travelSegI)->origin())) &&
                        (*travelSegI)->origin()->bufferZoneInd();

  if (CAMXBufferZone)
    originUS = true;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if (travelSegI != taxResponse.farePath()->itin()->travelSeg().begin())
    {
      travelSegFromI = travelSegI;
      travelSegFromI--;

      for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().begin();
           travelSegFromI--)
      {
        airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

        if (airSeg)
          break;
      }

      //
      // Special case stopover added for YUL-x/ORD-HNL-o/HNL-x/ORD-YUL charge one half US2
      //
      //

      if ((*travelSegFromI)->isFurthestPoint(*itin) ||
          ((*travelSegI)->isStopOver(
              (*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES)))
      {
        originUS = (LocUtil::isUS(*(*travelSegI)->origin()) &&
                    !LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin()));

        // Mandate log PL#103610 originating inside buffer zone with nostopover to Hawaii will now
        // charge.
        // PCC-AADFW PNR: YYC-x/SFO-x/KOA

        CAMXBufferZone = (LocUtil::isCanada(*(*travelSegI)->origin()) ||
                          LocUtil::isMexico(*(*travelSegI)->origin())) &&
                         (*travelSegI)->origin()->bufferZoneInd();

        if (CAMXBufferZone)
          originUS = true;
      }
    }

    boardUS = (LocUtil::isUS(*(*travelSegI)->origin()) &&
               !LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin()));

    offUS = (LocUtil::isUS(*(*travelSegI)->destination()) &&
             !LocUtil::isUSTerritoryOnly(*(*travelSegI)->destination()));

    if (_validInternationalPoint)
    {
      if (!boardUS)
      {
        if (!offUS)
        {
          travelSegI = applyIntToInt(trx, taxResponse, taxCodeReg, travelSegI);
          continue;
        }

        travelSegI = applyIntToUS(trx, taxResponse, taxCodeReg, travelSegI);
        continue;
      }

      if (!offUS)
      {
        applyUSToInt(trx, taxResponse, taxCodeReg, travelSegI);
        continue;
      }
    }

    travelSegI = applyUSToUS(trx, taxResponse, taxCodeReg, travelSegI, originUS, boardUS);
  }
}

// ----------------------------------------------------------------------------
// Description:  applyIntToInt
// ----------------------------------------------------------------------------

std::vector<TravelSeg*>::const_iterator
TaxUS2::applyIntToInt(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      std::vector<TravelSeg*>::const_iterator travelSegI)
{
  if ((!LocUtil::isUS(*_pointOfSaleLocation)) ||
      LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation) || _validInternationalPoint)
    return travelSegI;

  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;

  for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegFromI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

    if (!airSeg)
      continue;

    if (LocUtil::isUSTerritoryOnly(*(*travelSegFromI)->destination()))
      continue;

    if (LocUtil::isUS(*_pointOfSaleLocation) &&
        (!LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation)) && !_validInternationalPoint)
    {
      if (LocUtil::isCanada(*(*travelSegFromI)->origin()) ||
          LocUtil::isMexico(*(*travelSegFromI)->origin()))
        continue;
    }

    if ((*travelSegFromI)->isForcedConx())
      continue;

    travelSegToI = travelSegFromI;
    travelSegToI++;

    for (; travelSegToI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegToI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

      if (airSeg)
        break;
    }

    if (travelSegToI == taxResponse.farePath()->itin()->travelSeg().end())
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

      return travelSegFromI;
    }

    if ((*travelSegFromI)->isForcedStopOver() ||
        (*travelSegToI)
            ->isStopOver((*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES))
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

      return travelSegFromI;
    }
  }
  return travelSegI;
}

// ----------------------------------------------------------------------------
// Description:  applyIntToUS
// ----------------------------------------------------------------------------

std::vector<TravelSeg*>::const_iterator
TaxUS2::applyIntToUS(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     std::vector<TravelSeg*>::const_iterator travelSegI)
{
  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;

  for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegFromI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

    if (!airSeg)
      continue;

    if (!LocUtil::isUS(*airSeg->destination()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
      return travelSegI;

    if (LocUtil::isUSTerritoryOnly(*airSeg->origin()))
    {
      travelSegToI = travelSegFromI;

      for (; (*travelSegToI) != taxResponse.farePath()->itin()->travelSeg().back();)
      {
        travelSegToI++;

        airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

        if (airSeg)
          break;
      }

      if ((*travelSegFromI)->origin()->loc() == (*travelSegToI)->destination()->loc())
      {
        createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

        return travelSegFromI;
      }
    }

    if ((*travelSegFromI)->isForcedConx())
      continue;

    travelSegToI = travelSegFromI;

    for (; (*travelSegToI) != taxResponse.farePath()->itin()->travelSeg().back();)
    {
      travelSegToI++;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

      if (!airSeg)
        continue;

      if (airSeg->isForcedConx())
        continue;

      break;
    }

    if (((*travelSegFromI) == taxResponse.farePath()->itin()->travelSeg().back()) ||
        (*travelSegFromI)->isForcedStopOver() ||
        (*travelSegToI)
            ->isStopOver((*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES) ||
        RtwUtil::isRtwArunk(trx, *travelSegToI))
    {
      if ((*travelSegFromI)->destination()->bufferZoneInd())
        continue;

      if ((*travelSegI)->origin()->bufferZoneInd())
      {
        std::vector<TravelSeg*>::const_iterator travelSegPrevI = travelSegI;

        for (; (*travelSegPrevI) != taxResponse.farePath()->itin()->travelSeg().front();)
        {
          travelSegPrevI--;

          airSeg = dynamic_cast<const AirSeg*>(*travelSegPrevI);

          if (!airSeg)
            return travelSegI;

          if (!(*travelSegPrevI)->origin()->bufferZoneInd() ||
              !(*travelSegPrevI)->destination()->bufferZoneInd())
            break;
        }
      }

      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

      return travelSegFromI;
    }

    // AAADFW    PNR = LHR-JFK-EWR-LHR Same Day Apply Tax
    // AAADFW    PNR = YYZ-IAD-CLT-STT Same Day US Do Not Apply Tax

    if ((*travelSegI)->origin()->nation() == (*travelSegToI)->destination()->nation())
    {
      if (LocUtil::isUS(*(*travelSegI)->origin()))
        continue;

      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

      return travelSegFromI;
    }
  }
  return travelSegI;
}

// ----------------------------------------------------------------------------
// Description:  applyUSToInt
// ----------------------------------------------------------------------------

void
TaxUS2::applyUSToInt(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     std::vector<TravelSeg*>::const_iterator travelSegI)
{
  const AirSeg* airSeg;

  if (travelSegI == taxResponse.farePath()->itin()->travelSeg().begin())
  {
    if ((*travelSegI)->origin()->bufferZoneInd())
      return;

    if ((*travelSegI)->destination()->bufferZoneInd())
    {
      std::vector<TravelSeg*>::const_iterator travelSegNext = travelSegI;
      if ((*travelSegI) != taxResponse.farePath()->itin()->travelSeg().back())
      {
        travelSegNext++;
        airSeg = dynamic_cast<const AirSeg*>(*travelSegNext);

        if (!airSeg)
          return;
      }
    }

    createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegI);

    return;
  }

  bool tripOriginUS = false;

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;

  for (; travelSegToI != taxResponse.farePath()->itin()->travelSeg().begin(); travelSegToI--)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

    if (!airSeg)
      continue;

    if (!LocUtil::isUS(*airSeg->origin()) || LocUtil::isUSTerritoryOnly(*airSeg->origin()))
      return;

    if (LocUtil::isUSTerritoryOnly(*airSeg->destination()))
    {
      travelSegFromI = travelSegToI;

      for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().begin();)
      {
        travelSegFromI--;

        airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

        if (airSeg)
          break;
      }

      if ((*travelSegFromI)->origin()->loc() == (*travelSegToI)->destination()->loc())
      {
        createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);
        return;
      }
    }

    if ((*travelSegToI)->isForcedConx())
      continue;

    travelSegFromI = travelSegToI;

    for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().begin();)
    {
      travelSegFromI--;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

      if (!airSeg)
        continue;

      if (airSeg->isForcedConx())
        continue;

      if ((airSeg->origin()->bufferZoneInd()) && (airSeg->destination()->bufferZoneInd()))
        continue;

      break;
    }

    if (travelSegFromI == taxResponse.farePath()->itin()->travelSeg().begin())
    {
      if (((*travelSegFromI)->origin()->bufferZoneInd()) &&
          ((*travelSegFromI)->destination()->bufferZoneInd()))
        return;

      if (LocUtil::isUS(*(*travelSegFromI)->origin()) &&
          !LocUtil::isUSTerritoryOnly(*(*travelSegFromI)->origin()))
        tripOriginUS = true;
    }

    if ((tripOriginUS) || (*travelSegFromI)->isForcedStopOver() ||
        (*travelSegToI)
            ->isStopOver((*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES))
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegI);
      return;
    }

    // AAADFW    PNR = LHR-JFK-EWR-LHR Same Day

    if ((*travelSegI)->destination()->nation() == (*travelSegFromI)->origin()->nation())
    {
      if (LocUtil::isUS(*(*travelSegI)->destination()))
        continue;

      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegI);
      return;
    }
  }
}

// ----------------------------------------------------------------------------
// Description:  applyUSToUS
// ----------------------------------------------------------------------------

std::vector<TravelSeg*>::const_iterator
TaxUS2::applyUSToUS(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    std::vector<TravelSeg*>::const_iterator travelSegI,
                    bool originUS,
                    bool boardUS)
{
  if (LocUtil::isAlaska(*(*travelSegI)->origin()) &&
      LocUtil::isAlaska(*(*travelSegI)->destination()))
    return travelSegI;

  if (LocUtil::isHawaii(*(*travelSegI)->origin()) &&
      LocUtil::isHawaii(*(*travelSegI)->destination()))
    return travelSegI;

  bool transBoarder = false;

  if (!LocUtil::isUS(*_pointOfSaleLocation) || LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation))
  {
    if (!originUS)
      return travelSegI;

    if ((*travelSegI)->origin()->bufferZoneInd() && !(*travelSegI)->destination()->bufferZoneInd())
      transBoarder = true;

    if (!(*travelSegI)->origin()->bufferZoneInd() && (*travelSegI)->destination()->bufferZoneInd())
      transBoarder = true;
  }
  else
  {
    // PCC=AAADFW  PNR: YVR-LAX-HNL NoStopover No Charge *** OLD STATEMENT ****
    // Above statement has been changed by mandate log PL#103610
    // inside buffer zone Origin will know be treated as board US for POS US.
    // PCC-AADFW PNR: YYC-x/SFO-x/KOA will now charge 7/07

    bool CAMXBufferZone = (LocUtil::isCanada(*(*travelSegI)->origin()) ||
                           LocUtil::isMexico(*(*travelSegI)->origin())) &&
                          (*travelSegI)->origin()->bufferZoneInd();

    if (CAMXBufferZone)
      boardUS = true;
  }

  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;

  for (; (*travelSegToI) != taxResponse.farePath()->itin()->travelSeg().back();)
  {
    travelSegToI++;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

    if (airSeg)
      break;
  }

  bool stopOver = true;
  bool nextOffUS = true;

  if (travelSegToI != taxResponse.farePath()->itin()->travelSeg().end())
  {
    stopOver = ((*travelSegToI)->isStopOver(
        (*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES));

    nextOffUS = (*travelSegToI)->destination()->bufferZoneInd() ||
                (LocUtil::isUS(*(*travelSegToI)->destination()) &&
                 !LocUtil::isUSTerritoryOnly(*(*travelSegToI)->destination()));
  }

  if (!stopOver && !originUS)
    return travelSegI;

  for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().begin();)
  {
    travelSegFromI--;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

    if (airSeg)
      break;
  }

  stopOver = true;

  if (travelSegFromI != travelSegI)
  {
    stopOver = ((*travelSegI)->isStopOver(
        (*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES));

    // ADD mirror image POS YOW trip MHTPHL-PHLYOW-YOWPHL-PHLMHT PL#104168
    //
    if ((*travelSegFromI)->origin()->loc() == (*travelSegI)->destination()->loc())
      stopOver = true;
  }

  if (!stopOver && !originUS)
    return travelSegI;

  if ((!transBoarder) &&
      (!boardUS || !LocUtil::isAlaska(*(*travelSegI)->destination()) || !nextOffUS))
  {
    if (!boardUS || !LocUtil::isHawaii(*(*travelSegI)->destination()))
    {
      if (!LocUtil::isAlaska(*(*travelSegI)->origin()) &&
          !LocUtil::isHawaii(*(*travelSegI)->origin()))
        return travelSegI;

      nextOffUS = (*travelSegI)->destination()->bufferZoneInd() ||
                  (LocUtil::isUS(*(*travelSegI)->destination()) &&
                   !LocUtil::isUSTerritoryOnly(*(*travelSegI)->destination()));

      if (!nextOffUS && (!LocUtil::isAlaska(*(*travelSegI)->destination()) ||
                         !LocUtil::isHawaii(*(*travelSegI)->destination())))
        return travelSegI;
    }
  }

  travelSegFromI = travelSegI;

  for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegFromI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

    if (!airSeg)
      continue;

    if ((*travelSegFromI)->isForcedConx())
      continue;

    if ((*travelSegFromI) == taxResponse.farePath()->itin()->travelSeg().back())
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

      return travelSegFromI;
    }

    travelSegToI = travelSegFromI;

    for (; (*travelSegToI) != taxResponse.farePath()->itin()->travelSeg().back();)
    {
      travelSegToI++;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

      if (airSeg)
        break;
    }

    //
    // Same Day travel SEA-ANC-SEA should charge twice half price. charge for SEA-ANC and ANC-SEA
    //
    Itin* itin = taxResponse.farePath()->itin();

    if ((*travelSegFromI)->isFurthestPoint(*itin) &&
        (LocUtil::isAlaska(*(*travelSegFromI)->destination()) ||
         LocUtil::isHawaii(*(*travelSegFromI)->destination())))
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);
      return travelSegFromI;
    }

    // ADD mirror image POS YOW trip MHTPHL-PHLYOW-YOWPHL-PHLMHT PL#104168
    //
    stopOver = ((*travelSegToI)->isStopOver(
        (*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES));

    if ((*travelSegFromI)->origin()->loc() == (*travelSegToI)->destination()->loc())
      stopOver = true;

    if ((*travelSegFromI)->isForcedStopOver() || stopOver)
    {
      createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);
      return travelSegFromI;
    }
  }
  return travelSegI;
}

// ----------------------------------------------------------------------------
// Description:  internationalFromUS
// ----------------------------------------------------------------------------

void
TaxUS2::internationalFromUS(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  const AirSeg* airSeg;
  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI;
  std::vector<TravelSeg*>::const_iterator travelSegI = itin->travelSeg().begin();

  for (; travelSegI != itin->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if ((!(LocUtil::isUS(*airSeg->origin())) || LocUtil::isUSTerritoryOnly(*airSeg->origin())))
      continue;

    if ((LocUtil::isUS(*airSeg->destination()) &&
         !LocUtil::isUSTerritoryOnly(*airSeg->destination())))
      continue;

    //
    // Do Not Charge YYZLAX-x/LAXSYD-SYDJFK-x/EWRYYZ
    //

    if (!_tripOriginUS)
    {
      travelSegFromI = travelSegI;
      std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
      std::vector<TravelSeg*>::const_iterator tsFromI;
      std::vector<TravelSeg*>::const_iterator tsToI;

      for (; travelSegFromI != itin->travelSeg().begin(); travelSegToI--)
      {
        travelSegFromI--;
        tsFromI = travelSegFromI;
        tsToI = travelSegToI;

        airSeg = dynamic_cast<const AirSeg*>(*tsFromI);

        if (!airSeg)
        {
          if (tsFromI == itin->travelSeg().begin())
            break;

          tsFromI--;
        }

        airSeg = dynamic_cast<const AirSeg*>(*tsToI);

        if (!airSeg)
        {
          if (*tsToI == itin->travelSeg().back())
            break;

          tsToI++;
        }
        //
        // Mirror Image POS - LHR same day travel LHR/MIA/LHR or KIN/MIA/KIN or SJU/MUA/SJU
        //
        if ((*tsFromI)->origin()->nation() == (*tsToI)->destination()->nation() &&
            (*tsFromI)->origin()->nation() != (*tsToI)->origin()->nation())
          break;

        if ((LocUtil::isUS(*(*tsFromI)->destination())) && (LocUtil::isUS(*(*tsToI)->origin())))
        {
          if (LocUtil::isUSTerritoryOnly(*(*tsFromI)->origin()))
          {
            if (LocUtil::isUSTerritoryOnly(*(*tsToI)->destination()))
            {
              if ((*tsFromI)->origin()->loc() == (*tsToI)->destination()->loc())
                break;
            }
          }
        }

        if ((*tsFromI)->isForcedConx())
          continue;

        if ((*tsFromI)->isForcedStopOver() ||
            (*tsToI)->isStopOver(*tsFromI, (*tsFromI)->geoTravelType(), TravelSeg::TAXES))
          break;
      }

      if (travelSegFromI == travelSegToI)
        continue;

      if ((!LocUtil::isUS(*(*travelSegToI)->origin())) ||
          (LocUtil::isUSTerritoryOnly(*(*travelSegToI)->origin())))
        continue;
    }

    //    if (!airSeg->furthestPoint() && !_tripOriginUS)
    //       continue;

    travelSegFromI = travelSegI;

    createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

    //
    // Problem Log SPR-70876 Rule can only charge once outbound/inbound for POS outside US
    // Problem Log SPR-96917 can charge for than outbound/inbound for POS outside US

    //    if (!LocUtil::isUS(*_pointOfSaleLocation))
    //       break;
  }
}

// ----------------------------------------------------------------------------
// Description:  internationalToUS
// ----------------------------------------------------------------------------

void
TaxUS2::internationalToUS(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  const AirSeg* airSeg;
  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegFromI;
  std::vector<TravelSeg*>::const_iterator travelSegI = itin->travelSeg().begin();

  for (; travelSegI != itin->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if ((LocUtil::isUS(*airSeg->origin()) && !LocUtil::isUSTerritoryOnly(*airSeg->origin())))
      continue;

    if ((!LocUtil::isUS(*airSeg->destination()) ||
         LocUtil::isUSTerritoryOnly(*airSeg->destination())))
      continue;

    if (!_terminateUS)
    {
      travelSegFromI = travelSegI;
      std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;
      std::vector<TravelSeg*>::const_iterator tsFromI;
      std::vector<TravelSeg*>::const_iterator tsToI;

      travelSegToI++;

      for (; travelSegToI != itin->travelSeg().end(); travelSegFromI++, travelSegToI++)
      {
        tsFromI = travelSegFromI;
        tsToI = travelSegToI;

        airSeg = dynamic_cast<const AirSeg*>(*tsFromI);

        if (!airSeg)
        {
          if (tsFromI == itin->travelSeg().begin())
            break;

          tsFromI--;
        }

        airSeg = dynamic_cast<const AirSeg*>(*tsToI);

        if (!airSeg)
        {
          if (*tsToI == itin->travelSeg().back())
            break;

          tsToI++;
        }
        //
        // Mirror Image POS - LHR same day travel LHR/MIA/LHR or KIN/MIA/KIN or SJU/MUA/SJU
        //
        if ((*tsFromI)->origin()->nation() == (*tsToI)->destination()->nation() &&
            (*tsFromI)->origin()->nation() != (*tsToI)->origin()->nation())
          break;

        if ((LocUtil::isUS(*(*tsFromI)->destination())) && (LocUtil::isUS(*(*tsToI)->origin())))
        {
          if (LocUtil::isUSTerritoryOnly(*(*tsFromI)->origin()))
          {
            if (LocUtil::isUSTerritoryOnly(*(*tsToI)->destination()))
            {
              if ((*tsFromI)->origin()->loc() == (*tsToI)->destination()->loc())
                break;
            }
          }
        }

        if ((*tsFromI)->isForcedConx())
          continue;

        if ((*tsFromI)->isForcedStopOver() ||
            (*tsToI)->isStopOver(*tsFromI, (*tsFromI)->geoTravelType(), TravelSeg::TAXES))
          break;
      }

      if (travelSegToI == itin->travelSeg().end())
        continue;

      if ((!LocUtil::isUS(*(*travelSegFromI)->destination())) ||
          (LocUtil::isUSTerritoryOnly(*(*travelSegFromI)->destination())))
        continue;
    }

    //    if (!airSeg->furthestPoint() && !_terminateUS)
    //       continue;

    travelSegFromI = travelSegI;

    createUS2(trx, taxResponse, taxCodeReg, **travelSegI, **travelSegFromI);

    //
    // Problem Log SPR-70876 Rule can only charge once outbound/inbound for POS outside US
    // Problem Log SPR-96917 can charge for than outbound/inbound for POS outside US

    //  if (!LocUtil::isUS(*_pointOfSaleLocation))
    //     break;
  }
}

// ----------------------------------------------------------------------------
// Description:  createUS2
// ----------------------------------------------------------------------------

void
TaxUS2::createUS2(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  TaxCodeReg& taxCodeReg,
                  TravelSeg& travelSegFrom,
                  TravelSeg& travelSegTo)
{
  _travelSegStartIndex = 0;
  _travelSegEndIndex = 0;

  bool CAMXOutsideBufferZone = false;
  const AirSeg* airSeg;
  Itin* itin = taxResponse.farePath()->itin();

  std::vector<TravelSeg*>::const_iterator travelSegI = itin->travelSeg().begin();

  for (uint16_t index = 0; travelSegI != itin->travelSeg().end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if (itin->segmentOrder(*travelSegI) == itin->segmentOrder(&travelSegFrom))
    {
      _travelSegStartIndex = index;
      _travelSegEndIndex = index;
    }

    if (itin->segmentOrder(*travelSegI) > itin->segmentOrder(&travelSegFrom))
    {
      CAMXOutsideBufferZone = (LocUtil::isCanada(*(*travelSegI)->destination()) ||
                               LocUtil::isMexico(*(*travelSegI)->destination())) &&
                              !(*travelSegI)->destination()->bufferZoneInd();
    }
  }

  if (taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath())))
  {
    if (! taxUtil::doesUS2Apply(_travelSegStartIndex, _travelSegStartIndex, trx, taxResponse, taxCodeReg))
      return;
  }

  if(!isMostDistantUS(trx, taxResponse))
  {
    if (!taxUtil::isSurfaceSegmentAFactor(taxResponse, _travelSegStartIndex))
      return;
  }

  if (!validateCarrierExemption(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return;

  taxCreate(trx, taxResponse, taxCodeReg, _travelSegStartIndex, _travelSegEndIndex);

  doTaxRound(trx, taxCodeReg);

  bool splitUS2 = false;
  bool CAMXBufferZone = false;

  if (LocUtil::isUS(*_pointOfSaleLocation) && (!LocUtil::isUSTerritoryOnly(*_pointOfSaleLocation)))
  {
    CAMXBufferZone = (LocUtil::isCanada(*travelSegTo.destination()) ||
                      LocUtil::isMexico(*travelSegTo.destination())) &&
                     travelSegTo.destination()->bufferZoneInd();

    if (!CAMXBufferZone)
    {
      CAMXBufferZone =
          (LocUtil::isCanada(*travelSegTo.origin()) || LocUtil::isMexico(*travelSegTo.origin())) &&
          travelSegTo.origin()->bufferZoneInd();
    }
  }

  if (LocUtil::isAlaska(*travelSegFrom.origin()) || LocUtil::isHawaii(*travelSegFrom.origin()))
  {
    if (CAMXBufferZone)
      splitUS2 = true;

    if (LocUtil::isUS(*travelSegTo.destination()) &&
        !(LocUtil::isUSTerritoryOnly(*travelSegTo.destination())))
      splitUS2 = true;
  }
  else
  {
    if (LocUtil::isAlaska(*travelSegTo.destination()) ||
        LocUtil::isHawaii(*travelSegTo.destination()))
    {
      if (CAMXBufferZone)
        splitUS2 = true;

      if (LocUtil::isUS(*travelSegFrom.origin()) &&
          !LocUtil::isUSTerritoryOnly(*travelSegFrom.origin()))
        splitUS2 = true;
    }
  }

  if (CAMXOutsideBufferZone && CAMXBufferZone)
    splitUS2 = false;

  if (_validInternationalPoint)
    splitUS2 = false;

  if (splitUS2)
  {
    _taxAmount = calculateHalfTaxAmount(trx, taxResponse, _paymentCurrency, taxCodeReg);
    doTaxRound(trx, taxCodeReg);
  }

  TaxApply taxApply;
  taxApply.initializeTaxItem(trx, *this, taxResponse, taxCodeReg);
}

MoneyAmount
TaxUS2::calculateHalfTaxAmount(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               CurrencyCode& paymentCurrency,
                               TaxCodeReg& taxCodeReg)
{
  MoneyAmount taxAmount = taxCodeReg.taxAmt() / 2;
  RoundingFactor roundingUnit = 0.1;

  RoundingRule roundingRule = UP;
  DateTime ruleChangeDate = DateTime(2010, 1, 1, 0, 0, 0);
  if (trx.getRequest()->ticketingDT() < ruleChangeDate)
    roundingRule = DOWN;

  if (!taxCodeReg.specConfigName().empty())
  {
    std::string rndConf = utc::getSpecConfigParameter(
        trx, taxCodeReg.specConfigName(), "HALFTAXROUND", trx.getRequest()->ticketingDT());
    if (!rndConf.empty())
    {
      if (rndConf[0] == 'D')
        roundingRule = DOWN;
      else if (rndConf[0] == 'U')
        roundingRule = UP;
      else if (rndConf[0] == 'N')
        roundingRule = NONE;
    }
  }

  TaxRound taxRound;
  MoneyAmount taxAmt =
      taxRound.applyTaxRound(taxAmount, taxCodeReg.taxCur(), roundingUnit, roundingRule);
  if (taxAmt)
  {
    taxAmount = taxAmt;
  }
  if (taxCodeReg.taxCur() != paymentCurrency)
  {
    CurrencyConversionFacade ccFacade;
    Money targetMoney(paymentCurrency);
    targetMoney.value() = 0;
    Money sourceMoney(taxAmount, taxCodeReg.taxCur());
    BSRCollectionResults bsrResults;
    if (!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &bsrResults))
    {
      LOG4CXX_WARN(_logger,
                   "Currency Convertion Collection *** TaxUS2::calculateHalfTaxAmount ***");
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    taxAmount = targetMoney.value();
  }
  return taxAmount;
}

bool
TaxUS2::isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (!_mostDistantUSInitialized)
  {
    _mostDistantUS = taxUtil::isMostDistantUS(trx, taxResponse);
    _mostDistantUSInitialized = true;
  }
  return _mostDistantUS;
}

} //tse
