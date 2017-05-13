// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxSP18.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Common/TaxRound.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/PfcMultiAirport.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"

using namespace tse;
using namespace std;

bool
TaxSP18::validateSequence(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t& travelSegStartIndex,
                          uint16_t& travelSegEndIndex,
                          bool checkSpn)
{

  return Tax::validateSequence(
      trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex, Tax::CHECK_SPN);
}

// ---------------------------------------
// Description:  validateTransit
// ---------------------------------------
bool
TaxSP18::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  const AirSeg* airSeg;
  _segmentOrderRoundTrip = 0;
  _segmentOrderStopOver = 0;
  const Itin* itin = taxResponse.farePath()->itin();

  if (itin->travelSeg().size() <= travelSegIndex)
    return false;

  TravelSeg* travelSeg = itin->travelSeg()[travelSegIndex];
  TravelSeg* tsRoundTrip = itin->travelSeg().front();

  const PfcMultiAirport* pfcMultiAirport = trx.dataHandle().getPfcMultiAirport(
      tsRoundTrip->origin()->loc(), trx.getRequest()->ticketingDT());
  std::vector<TravelSeg*>::const_iterator travelSegIterNext;
  std::vector<TravelSeg*>::const_iterator travelSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegNextIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegEndIter = itin->travelSeg().end();
  travelSegIter++;
  travelSegNextIter++;

  for (; travelSegIter != travelSegEndIter; travelSegIter++)
  {
    travelSegNextIter++;

    if (itin->segmentOrder(travelSeg) > itin->segmentOrder(*travelSegIter))
      continue;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

    if (!airSeg)
      continue;

    if ((*travelSegIter)->destination()->loc() != tsRoundTrip->origin()->loc() ||
        hasTransfer(travelSegIter, travelSegNextIter, travelSegEndIter, itin->geoTravelType()))
    {
      if (!pfcMultiAirport)
        continue;

      if ((*travelSegIter)->isForcedConx())
        continue;

      if ((*travelSegIter) != itin->travelSeg().back())
      {
        if (!(*travelSegIter)->isForcedStopOver())
        {
          travelSegNextIter = travelSegIter;
          travelSegNextIter++;
          if (!(*travelSegNextIter)
                   ->isStopOver((*travelSegIter), (*travelSegIter)->geoTravelType()))
            continue;
        }
        _segmentOrderStopOver = itin->segmentOrder(*travelSegIter);
      }
      std::vector<PfcCoterminal*>::const_iterator pfcCoterminalIter =
          pfcMultiAirport->coterminals().begin();
      std::vector<PfcCoterminal*>::const_iterator pfcCoterminalEndIter =
          pfcMultiAirport->coterminals().end();

      for (; pfcCoterminalIter != pfcCoterminalEndIter; pfcCoterminalIter++)
      {
        if ((*pfcCoterminalIter)->cotermLoc() == (*travelSegIter)->destination()->loc())
          break;
      }
      if (pfcCoterminalIter == pfcCoterminalEndIter)
        continue;
    }

    _segmentOrderRoundTrip = itin->segmentOrder(*travelSegIter);

    break;
  }

  if (utc::isAySPN1800OptionB(trx, taxSpecConfig()))
  {
    if (taxCodeReg.itineraryType() != ONEWAY_TRIP && taxCodeReg.itineraryType() != ROUND_TRIP)
      return true;
  }

  if (travelSegIter == travelSegEndIter)
  {
    if (taxCodeReg.itineraryType() == ONEWAY_TRIP)
      return true;
  }
  else
  {
    if (taxCodeReg.itineraryType() == ROUND_TRIP)
      return true;
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::ITINERARY, Diagnostic820);

  return false;
}

// ----------------------------------------------------------------------------
// Description:  TaxCreate
// ----------------------------------------------------------------------------

void
TaxSP18::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)

{
  return;
}
// ----------------------------------------------------------------------------
// Description:  TaxCreate
// ----------------------------------------------------------------------------

void
TaxSP18::taxCreate(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t travelSegStartIndex,
                   uint16_t travelSegEndIndex)
{
  _taxAmount = 0.0;
  const AirSeg* airSeg;

  const Itin* itin = taxResponse.farePath()->itin();

  if (itin->travelSeg().size() <= travelSegStartIndex)
    return;

  TravelSeg* travelSeg = itin->travelSeg()[travelSegStartIndex];
  std::vector<TravelSeg*>::const_iterator travelSegIter =
      taxResponse.farePath()->itin()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegEndIter =
      taxResponse.farePath()->itin()->travelSeg().end();

  for (; travelSegIter != travelSegEndIter; travelSegIter++)
  {
    if (itin->segmentOrder(travelSeg) == itin->segmentOrder(*travelSegIter))
      break;
  }

  if (travelSegIter == travelSegEndIter)
    return;

  CarrierValidator carrierValidator;
  travelSegEndIndex = travelSegStartIndex;
  uint16_t numberOfFees = 1;
  bool locMatch = false;

  if ((*travelSegIter) != taxResponse.farePath()->itin()->travelSeg().back())
  {
    std::vector<TravelSeg*>::const_iterator travelSegToIter = travelSegIter;

    for (; (*travelSegToIter) != taxResponse.farePath()->itin()->travelSeg().back();
         travelSegEndIndex++)
    {
      travelSegToIter++;
      airSeg = dynamic_cast<const AirSeg*>(*travelSegToIter);

      if (!airSeg)
        continue;

      locMatch = LocUtil::isInLoc(*(*travelSegToIter)->origin(),
                                  taxCodeReg.loc1Type(),
                                  taxCodeReg.loc1(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());

      if (taxCodeReg.itineraryType() == ROUND_TRIP)
      {
        if (locMatch)
        {
          if (carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegEndIndex + 1))
            numberOfFees++;
        }

        if (itin->segmentOrder(*travelSegToIter) != _segmentOrderRoundTrip)
          continue;

        travelSegEndIndex++;
        break;
      }

      if (!locMatch)
        break;

      travelSegIter = travelSegToIter;

      for (; travelSegIter != taxResponse.farePath()->itin()->travelSeg().begin();)
      {
        travelSegIter--;
        airSeg = dynamic_cast<const AirSeg*>(*travelSegIter);

        if (!airSeg)
          continue;

        break;
      }

      if ((*travelSegToIter)->isForcedStopOver())
        break;

      TravelSeg::Application application = TravelSeg::OTHER;

      if ((*travelSegToIter)->geoTravelType() == GeoTravelType::International)
        application = TravelSeg::TAXES;

      if ((*travelSegToIter)
              ->isStopOver((*travelSegIter), (*travelSegIter)->geoTravelType(), application) &&
          !(*travelSegToIter)->isForcedConx())
        break;

      if (!carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegEndIndex + 1))
        continue;

      numberOfFees++;
    }
  }

  if ((taxCodeReg.maxTax() > 0.0) && (taxCodeReg.taxAmt() > 0.0))
  {
    uint16_t maxNumberOfFees = static_cast<uint16_t>(taxCodeReg.maxTax() / taxCodeReg.taxAmt());

    if (numberOfFees > maxNumberOfFees)
      numberOfFees = maxNumberOfFees;
  }

  Tax tax;

  tax.taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  _taxAmount = tax.taxAmount();
  _taxableFare = tax.taxableFare();
  _paymentCurrency = tax.paymentCurrency();
  _paymentCurrencyNoDec = tax.paymentCurrencyNoDec();
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;
  _intermediateCurrency = tax.intermediateCurrency();
  _intermediateNoDec = tax.intermediateNoDec();
  _exchangeRate1 = tax.exchangeRate1();
  _exchangeRate1NoDec = tax.exchangeRate1NoDec();
  _exchangeRate2 = tax.exchangeRate2();
  _exchangeRate2NoDec = tax.exchangeRate2NoDec();
  _intermediateUnroundedAmount = tax.intermediateUnroundedAmount();
  _intermediateAmount = tax.intermediateAmount();

  _taxAmount *= numberOfFees;
}

bool
TaxSP18::hasTransfer(const std::vector<TravelSeg*>::const_iterator& travelSegCurrent,
                     const std::vector<TravelSeg*>::const_iterator& travelSegNext,
                     const std::vector<TravelSeg*>::const_iterator& travelSegEnd,
                     const GeoTravelType& geoTravelType) const
{

  bool isTransfer = false;

  if (travelSegNext != travelSegEnd)
  {
    if ((*travelSegCurrent)->isForcedConx())
      isTransfer = true;
    else if ((*travelSegCurrent)->isForcedStopOver())
      isTransfer = false;
    else
      isTransfer = !(*travelSegNext)->isStopOver(*travelSegCurrent, geoTravelType);
  }
  else
    isTransfer = false;

  return isTransfer;
}
