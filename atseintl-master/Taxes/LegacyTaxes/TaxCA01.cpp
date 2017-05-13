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

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxCA01.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace utc;
using namespace std;


const string
TaxCA01::TAX_CODE_CA1("CA1");
const string
TaxCA01::TAX_CODE_CA2("CA2");
const string
TaxCA01::TAX_CODE_CA3("CA3");

TaxCA01::~TaxCA01() {}

enum TaxCA01::LocCategory
TaxCA01::checkLocCategory(const Loc& loc)
{
  if (LocUtil::isCanada(loc) || LocUtil::isStPierreMiquelon(loc))
  {
    return CANADA;
  }
  if (LocUtil::isUS(loc))
  {
    if (LocUtil::isHawaii(loc) || LocUtil::isUSTerritoryOnly(loc))
    {
      return OTHER;
    }
    else
    {
      return US;
    }
  }
  return OTHER;
}

void
TaxCA01::ItinAnalyzer::typeAnalyze(TravelSeg* travelSeg)
{
  enum LocCategory origin = checkLocCategory(*(travelSeg->origin()));
  enum LocCategory dest = checkLocCategory(*(travelSeg->destination()));

  if (origin == CANADA && dest == US && itinType != INTERNATIONAL)
  {
    itinType = TRANSBORDER;
  }
  else if (origin != OTHER && dest == OTHER)
  {
    itinType = INTERNATIONAL;
    geoType = GeoTravelType::International;
  }
  else if (origin == OTHER && itinType != TRANSBORDER && itinType != INTERNATIONAL)
  {
    itinType = INBOUND_INTERNATIONAL;
    geoType = GeoTravelType::International;
  }
}

void
TaxCA01::ItinAnalyzer::journeyAnalyze(PricingTrx& trx, TravelSeg* travelSeg)
{
  itinAnalisisVector.resize(index + 1);

  AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
  if (!airSeg)
  {
    ++index;
    return;
  }

  itinAnalisisVector[index].stopOver = isStopOver(travelSeg, travelSegPrev);

  if (itinAnalisisVector[index].stopOver &&
      enplanementsSet.find((Loc*)travelSeg->origin()) != enplanementsSet.end())
  {
    journeyOriginIndex = index;
    enplanementsSet.clear();
  }

  if (enplanementsSet.find((Loc*)travelSeg->destination()) != enplanementsSet.end())
  {
    itinAnalisisVector[journeyOriginIndex].loopTrip = true;
  }

  if (checkLocCategory(*(travelSeg->origin())) != CANADA ||
      checkLocCategory(*(travelSeg->destination())) != CANADA)
  {
    itinAnalisisVector[journeyOriginIndex].allCanadian = false;
  }

  if (checkLocCategory(*(travelSeg->origin())) == CANADA && itinAnalisisVector[index].stopOver)
  {
    _fromAbroadLoc = nullptr;
  }

  // international transit
  if (checkLocCategory(*(travelSeg->origin())) != CANADA &&
      checkLocCategory(*(travelSeg->destination())) == CANADA)
  {
    _fromAbroadLoc = travelSeg->origin();
  }

  if (checkLocCategory(*(travelSeg->origin())) == CANADA &&
      checkLocCategory(*(travelSeg->destination())) != CANADA)
  {
    if (_fromAbroadLoc != nullptr)
    {
      if (((_fromAbroadLoc->area() != IATA_AREA1) &&
           (_fromAbroadLoc->area() == travelSeg->destination()->area())) ||
          ((!_fromAbroadLoc->city().empty()) &&
           (_fromAbroadLoc->city() == travelSeg->destination()->city())) ||
          (_fromAbroadLoc == travelSeg->destination()))
      {
        itinAnalisisVector[index].stopOver = true;
      }
    }
  }

  if (checkLocCategory(*(travelSeg->origin())) != CANADA)
    _caStop = false;
  else if (itinAnalisisVector[index].stopOver)
    _caStop = true;

  itinAnalisisVector[index].journeyOriginIndex = journeyOriginIndex;

  if (checkLocCategory(*(travelSeg->origin())) == CANADA)
    enplanementsSet.insert((Loc*)travelSeg->origin());

  itinAnalisisVector[index].caStop = _caStop;
  travelSegPrev = travelSeg;
  ++index;
}

void
TaxCA01::ItinAnalyzer::startEndAnalyze(const TravelSeg& segStart, const TravelSeg& segEnd)
{
  _isStartEndIntrntl = checkLocCategory(*segStart.origin()) == OTHER &&
    checkLocCategory(*segEnd.destination()) == OTHER;
}

bool
TaxCA01::soldInCanada(PricingTrx& trx)
{
  const Loc* pointOfSaleLocation = TrxUtil::ticketingLoc(trx);
  if (trx.getRequest()->ticketPointOverride().empty())
  {
    pointOfSaleLocation = TrxUtil::saleLoc(trx);
  }
  return LocUtil::isCanada(*pointOfSaleLocation);
}

bool
TaxCA01::doesXGApply(const Itin& itin)
{
  if (itinAnalyzer.getItinType() == INTERNATIONAL ||
      (!LocUtil::isCanada(*(itin.travelSeg().front()->origin()))))
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool
TaxCA01::ItinAnalyzer::charge(uint16_t segIndex, bool bInterntl, const PricingTrx& trx)
{
  if (!bInterntl)
  {
    uint16_t& chargeCount =
          itinAnalisisVector[itinAnalisisVector[segIndex].journeyOriginIndex].chargeCount;

    if (chargeCount >= 2)
      return false;
    else if (itinAnalisisVector[segIndex].stopOver ||
             (itinAnalisisVector[itinAnalisisVector[segIndex].journeyOriginIndex].loopTrip &&
              itinAnalisisVector[itinAnalisisVector[segIndex].journeyOriginIndex].allCanadian) ||
             (itinAnalisisVector[segIndex].caStop && chargeCount == 0))
    {
      ++chargeCount;
      if (chargeCount == 2)
        correctFlag = true;
      return true;
    }
    else
      return false;
  }
  else if ((!_intrntlCharged) && itinAnalisisVector[segIndex].caStop)
  {
    _intrntlCharged = true;
    return true;
  }
  else
    return false;
}

bool
TaxCA01::ItinAnalyzer::isStopOver(TravelSeg* currentSeg, TravelSeg* previousSeg)
{
  if (previousSeg == nullptr)
    return true;
  if (previousSeg->isForcedConx())
    return false;

  if (currentSeg->isStopOver(previousSeg, geoType, TravelSeg::OTHER) ||
      previousSeg->isForcedStopOver())
  {
    if (itinType == INBOUND_INTERNATIONAL && checkLocCategory(*(currentSeg->origin())) == CANADA)
      geoType = GeoTravelType::Domestic;
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------------
// Description:  validateCarrierExemption
// ----------------------------------------------------------------------------
bool
TaxCA01::validateCarrierExemption(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  validateLocation
// ----------------------------------------------------------------------------
bool
TaxCA01::validateLocRestrictions(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  TripTypesValidator
// ----------------------------------------------------------------------------

bool
TaxCA01::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)
{
  Itin& itin = *(taxResponse.farePath()->itin());

  if (!_isItineraryAnalyzed)
  {
    std::vector<TravelSeg*>::iterator tsi;
    for (tsi = itin.travelSeg().begin(); tsi != itin.travelSeg().end(); ++tsi)
      itinAnalyzer.typeAnalyze(*tsi);
    for (tsi = itin.travelSeg().begin(); tsi != itin.travelSeg().end(); ++tsi)
      itinAnalyzer.journeyAnalyze(trx, *tsi);
    _soldInCanada = soldInCanada(trx);
    itinAnalyzer.startEndAnalyze(*itin.travelSeg().front(), *itin.travelSeg().back());
    _isItineraryAnalyzed = true;
  }


  if (!Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, startIndex))
    return false;

  enum TaxDiagnostic::FailCodes diagCode;
  diagCode = validateSegment(trx, itin, taxCodeReg, startIndex, endIndex);

  if (diagCode != TaxDiagnostic::NONE && diagCode != TaxDiagnostic::OTHER)
  {
    TaxDiagnostic::collectErrors(trx, taxCodeReg, taxResponse, diagCode, Diagnostic820);
  }
  if (diagCode == TaxDiagnostic::NONE)
  {
    endIndex = startIndex;
    return true;
  }
  else
    return false;

} // end of validateTrip

enum TaxDiagnostic::FailCodes
TaxCA01::validateSegment(PricingTrx& trx,
                         Itin& itin,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)

{

  AirSeg* airSeg = dynamic_cast<AirSeg*>(itin.travelSeg()[startIndex]);
  if (UNLIKELY(!airSeg))
    return TaxDiagnostic::OTHER;

  bool bCa2OnDomesticSegments = startIndex
      && taxCodeReg.taxCode() == TAX_CODE_CA2
      && itinAnalyzer.getItinType() != DOMESTIC
      && itinAnalyzer.getItinType() != TRANSBORDER
      && (itinAnalyzer.getOriginIndex(startIndex) || !itinAnalyzer.getCaStop(0));

  if (!bCa2OnDomesticSegments)
  {
    if (!_soldInCanada && (itinAnalyzer.getItinType() == DOMESTIC ||
                          itinAnalyzer.getItinType() == INBOUND_INTERNATIONAL ||
                          (LocUtil::isCanada(*(itin.travelSeg()[startIndex]->origin())) &&
                           LocUtil::isCanada(*(itin.travelSeg()[startIndex]->destination())))))
    {
      if (taxCodeReg.taxCode() != TAX_CODE_CA3 ||
          !itinAnalyzer.isStartEndIntrntl())
            return TaxDiagnostic::OTHER;
    }
  }

  // CA is charged only in Canadian listed airports
  if (UNLIKELY(taxCodeReg.loc1Type() == LOCTYPE_NONE))
    return TaxDiagnostic::OTHER;

  bool geoMatch = LocUtil::isInLoc(*(itin.travelSeg()[startIndex]->origin()),
                              taxCodeReg.loc1Type(),
                              taxCodeReg.loc1(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::TAXES);

  if ((!geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
      (geoMatch && taxCodeReg.loc1ExclInd() == YES) ||
      (!geoMatch && taxCodeReg.loc1ExclInd() != YES))
  {
    return TaxDiagnostic::LOCATION_RESTRICTION;
  }

  if ( itinAnalyzer.getItinType() != INTERNATIONAL )
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return TaxDiagnostic::OTHER;

    geoMatch = LocUtil::isInLoc(*(itin.travelSeg()[startIndex]->destination()),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES);

    if ((!geoMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
        (geoMatch && taxCodeReg.loc2ExclInd() == YES) ||
        (!geoMatch && taxCodeReg.loc2ExclInd() != YES))
    {
      return TaxDiagnostic::LOCATION_RESTRICTION;
    }
  }

  // Perform each specific routine Taxes
  if (taxCodeReg.taxCode() == TAX_CODE_CA3)
  {
    if (itinAnalyzer.getItinType() != INTERNATIONAL)
      return TaxDiagnostic::OTHER;
  }
  else
  {
    if (itinAnalyzer.getItinType() == INTERNATIONAL)
      return TaxDiagnostic::OTHER;

    if (taxCodeReg.tripType() == TAX_BETWEEN)
    {
      if (itinAnalyzer.getItinType() != DOMESTIC &&
          itinAnalyzer.getItinType() != INBOUND_INTERNATIONAL)
        return TaxDiagnostic::OTHER;
    }
    else
    {
      if (itinAnalyzer.getItinType() != TRANSBORDER)
        return TaxDiagnostic::OTHER;
    }
  }

  if (taxCodeReg.taxCode() == TAX_CODE_CA1 && (!doesXGApply(itin)))
    return TaxDiagnostic::OTHER;

  if (taxCodeReg.taxCode() == TAX_CODE_CA2 && doesXGApply(itin))
    return TaxDiagnostic::OTHER;

  if (itinAnalyzer.charge(startIndex, taxCodeReg.taxCode() == TAX_CODE_CA3, trx))
    return TaxDiagnostic::NONE;
  else
    return TaxDiagnostic::OTHER;
}

// ----------------------------------------------------------------------------
// Description:  AdjustTax
// ----------------------------------------------------------------------------

void
TaxCA01::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (itinAnalyzer.correctFlag)
  {
    itinAnalyzer.correctFlag = false;

    MoneyAmount maxTax = taxUtil::convertCurrency(trx, taxCodeReg.maxTax(), _paymentCurrency,
        taxCodeReg.taxCur(), taxCodeReg.taxCur(), CurrencyConversionRequest::TAXES, false);

    if (2.0 * _taxAmount > maxTax)
    {
      _taxAmount = maxTax - _taxAmount;
    }
  }
}

