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

#include "Taxes/LegacyTaxes/TaxSP1801.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Common/Money.h"
#include "Common/CurrencyConversionFacade.h"
#include "DBAccess/PfcMultiAirport.h"

using namespace tse;

log4cxx::LoggerPtr
TaxSP1801::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxSP1801"));


TaxSP1801::TaxSP1801(): _validTransitLastSegOrder(0),
                        _cicrleClosingInitialized(false),
                        _isFMMHInternational(false),
                        _treatTrainBusAsNonAir(false){}

TaxSP1801::~TaxSP1801() {}

bool
TaxSP1801::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  _isFMMHInternational = utc::isAYTax_FM_MH_NationInt(trx, _taxSpecConfig);
  _treatTrainBusAsNonAir = utc::treatBusTrainAsNonAir(trx, _taxSpecConfig);

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  locIt->setSurfaceAsStop(false);
  locIt->toSegmentNo(travelSegIndex);
  locIt->setTreatTrainBusAsNonAir(_treatTrainBusAsNonAir);

  if (_validTransitLastSegOrder != 0 && (travelSegIndex == _validTransitLastSegOrder
        || (!locIt->isPrevSegAirSeg() && travelSegIndex == _validTransitLastSegOrder + 1)))
  {
    _validTransitLastSegOrder = travelSegIndex + 1;
    return true;
  }

  setStopTime(locIt);


  if (locIt->hasPrevious() && !locIt->isPrevSegAirSeg())
  {
    const PfcMultiAirport* pfcMultiAirport = trx.dataHandle().getPfcMultiAirport(
      locIt->loc()->loc(), trx.getRequest()->ticketingDT());

    locIt->previous();
    if (!isCoterminal(trx, pfcMultiAirport, locIt->loc()) || !locIt->isPrevSegAirSeg())
    {
      _validTransitLastSegOrder = travelSegIndex + 1;
      return true;
    }
    locIt->next();
  }

  if (locIt->isStop()
      || (utc::isAYTaxCapOption(trx, _taxSpecConfig) && isCT(trx, taxResponse, travelSegIndex)))
  {
    _validTransitLastSegOrder = travelSegIndex + 1;
    return true;
  }

  if (LIKELY(locIt->hasPrevious()))
  {
    locIt->previous();

    if (!locIt->isNextSegAirSeg() && isUS(locIt->loc()) && locIt->hasPrevious())
    {
      std::string intDomIntOption = utc::getAYTaxIntDomIntOption(trx, _taxSpecConfig);

      if (intDomIntOption == "A")
      {
        locIt->previous();
      }
      else if (intDomIntOption == "B")
      {
        const PfcMultiAirport* pfcMultiAirport = trx.dataHandle().getPfcMultiAirport(
          locIt->loc()->loc(), trx.getRequest()->ticketingDT());

        locIt->next();

        if (isCoterminal(trx, pfcMultiAirport, locIt->loc()))
          locIt->previous();

        locIt->previous();
      }
    }

    if (!isUS(locIt->loc()))
    {
      _validTransitLastSegOrder = travelSegIndex + 1;
      return true;
    }
  }

  _validTransitLastSegOrder = 0;
  return false;
}

bool
TaxSP1801::validateFareClass(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t travelSegIndex)
{
  _validTransitLastSegOrder = 0;
  return Tax::validateFareClass(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxSP1801::isConUS(const Loc* loc)
{
  return    LocUtil::isUS(*loc) && !LocUtil::isUSPossession(*loc)
         && !LocUtil::isAlaska(*loc) && !LocUtil::isHawaii(*loc)
         && !LocUtil::isUSTerritoryOnly(*loc);
}

bool
TaxSP1801::isUS(const Loc* loc)
{
  return (LocUtil::isUS(*loc) || LocUtil::isUSPossession(*loc))
          && (!_isFMMHInternational
              || (loc->nation() != "FM" && loc->nation() != "MH"));
}

bool
TaxSP1801::isCoterminal(PricingTrx& trx, const PfcMultiAirport* pfcMultiAirport,
                        const Loc* loc) const
{
  if (pfcMultiAirport)
  {
    if (pfcMultiAirport->loc().loc() == loc->loc())
      return true;

    for (PfcCoterminal* coterminal : pfcMultiAirport->coterminals())
    {
      if (coterminal->cotermLoc() == loc->loc())
        return true;
    }
  }

  return false;
}

void
TaxSP1801::findCircleClosing(PricingTrx& trx, TaxResponse& taxResponse)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  locIt->setSurfaceAsStop(false);
  locIt->toFront();
  locIt->setTreatTrainBusAsNonAir(_treatTrainBusAsNonAir);
  const Loc* firstLoc = locIt->loc();
  uint16_t unLastValidSegId = locIt->segNo()-1;

  if(_treatTrainBusAsNonAir)
  {
    locIt->toBack();
    while(locIt->hasPrevious() && !locIt->isPrevSegAirSeg())
      locIt->previous();
    unLastValidSegId = locIt->prevSegNo();

    locIt->toFront();
    while(locIt->hasNext() && !locIt->isNextSegAirSeg())
      locIt->next();
  }

  const PfcMultiAirport* pfcMultiAirport = trx.dataHandle().getPfcMultiAirport(
         locIt->loc()->loc(), trx.getRequest()->ticketingDT());

  while (locIt->hasNext())
  {
    locIt->next();
    setStopTime(locIt);
    if ((locIt->isStop() || locIt->prevSegNo()==unLastValidSegId)
        && (locIt->loc()->loc() == firstLoc->loc()
            || isCoterminal(trx, pfcMultiAirport, locIt->loc())))
    {
      _circleClosings.push_back(locIt->segNo());
    }
  }
  _cicrleClosingInitialized = true;
}

bool
TaxSP1801::isCT(PricingTrx& trx, TaxResponse& taxResponse, uint16_t travelSegIndex)
{
  if (!_cicrleClosingInitialized)
    findCircleClosing(trx, taxResponse);

  if (std::upper_bound(_circleClosings.begin(), _circleClosings.end(), travelSegIndex) != _circleClosings.end())
    return true;

  return false;
}

bool
TaxSP1801::validateFinalGenericRestrictions(PricingTrx& trx,
                                            TaxResponse& taxResponse,
                                            TaxCodeReg& taxCodeReg,
                                            uint16_t& travelSegStartIndex,
                                            uint16_t& travelSegEndIndex)
{
  if (taxCodeReg.maxTax() <= taxCodeReg.taxAmt())
    return true;

  if (isCT(trx, taxResponse, travelSegStartIndex))
  {
    std::vector<uint16_t>::const_iterator it
      = std::upper_bound(_circleClosings.begin(), _circleClosings.end(), travelSegStartIndex);

    uint16_t circleStart = 0;
    if (it != _circleClosings.begin())
      circleStart = *(it - 1);

    uint16_t count = 1;
    for (const TaxItem* taxItem : taxResponse.taxItemVector())
    {
      if (taxItem->taxCode() == taxCodeReg.taxCode()
          && taxItem->travelSegStartIndex() >= circleStart
          && taxItem->travelSegStartIndex() < *it)
        ++count;
    }
    if (taxCodeReg.taxAmt() * count <= taxCodeReg.maxTax())
      return true;
  }
  else
    return true;

  return false;
}

void
TaxSP1801::setStopTime (TaxLocIterator* locIt)
{
  locIt->setStopHours(4);

  if (locIt->hasPrevious())
  {
     locIt->previous();

     if (!isConUS(locIt->loc()))
        locIt->setStopHours(12);

     if (!locIt->isNextSegAirSeg() && locIt->hasPrevious())
     {
        locIt->previous();

         if (!isConUS(locIt->loc()))
           locIt->setStopHours(12);

         locIt->next();
     }

     locIt->next();
  }

  if (locIt->hasNext())
  {
     locIt->next();

     if (!isConUS(locIt->loc()))
       locIt->setStopHours(12);

     locIt->previous();
  }

  if (!isConUS(locIt->loc()))
     locIt->setStopHours(12);
}


