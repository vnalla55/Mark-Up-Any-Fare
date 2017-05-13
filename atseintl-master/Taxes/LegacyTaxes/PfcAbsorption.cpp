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

#include "Taxes/LegacyTaxes/PfcAbsorption.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PfcAbsorb.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
PfcAbsorption::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.PfcAbsorption"));

const string
PfcAbsorption::TAX_CODE_US1("US1");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

PfcAbsorption::PfcAbsorption()
  : _firstFareUsage(false), _type1(false), _type234(false), _pricingUnit(nullptr), _fareUsage(nullptr)
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

PfcAbsorption::~PfcAbsorption() {}

// ----------------------------------------------------------------------------
// Description:  PfcAbsorption
// ----------------------------------------------------------------------------

bool
PfcAbsorption::pfcAbsorptionApplied(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TravelSeg& travelSeg,
                                    const CurrencyCode& absorbCurrency,
                                    MoneyAmount absorbAmount,
                                    uint8_t locType)
{
  TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

  if (taxTrx)
    return false;

  if (!locateFareUsage(taxResponse, travelSeg))
    return false;

  if (!validPfcItin(trx, taxResponse, travelSeg, locType))
  {
    return false;

    //
    // Need example to process Default Logic correctly
    //

    if ((!_type1) || (_type234))
      return false;

    Itin* itin = taxResponse.farePath()->itin();

    if (itin->segmentOrder(_fareUsage->travelSeg().front()) == itin->segmentOrder(&travelSeg))
      return false;

    if (!_fareUsage->stopovers().empty())
      return false;
  }

  adjustFare(trx, taxResponse, absorbCurrency, absorbAmount);

  return true;
}

// ----------------------------------------------------------------------------
// Description:  isValidPfc
// ----------------------------------------------------------------------------

bool
PfcAbsorption::validPfcItin(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TravelSeg& travelSeg,
                            uint8_t locType)
{
  if (taxResponse.farePath()->getTotalNUCAmount() == 0)
    return false;

  const AirSeg* airSeg;
  airSeg = dynamic_cast<const AirSeg*>(&travelSeg);

  if (!airSeg)
    return false;

  CarrierCode carrier = airSeg->marketingCarrierCode();

  std::vector<TravelSeg*>::const_iterator travelSegI;

  for (travelSegI = _fareUsage->travelSeg().begin(); travelSegI != _fareUsage->travelSeg().end();
       travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      return false;

    if (carrier != airSeg->marketingCarrierCode())
      return false;

    if (UNLIKELY((trx.excTrxType() == PricingTrx::PORT_EXC_TRX) &&
        (trx.getRequest()->ticketingAgent() != nullptr ||
         trx.getRequest()->ticketingAgent()->agentTJR() != nullptr)))
    {
      ExchangePricingTrx* exchangePricingTrx = dynamic_cast<ExchangePricingTrx*>(&trx);
      if (exchangePricingTrx != nullptr)
      { // check if the travel segment is in the map to skip the PFC on that travel segment
        std::map<const TravelSeg*, uint16_t>::const_iterator p;
        p = exchangePricingTrx->exchangeOverrides().dummyFCSegs().find(*travelSegI);
        if (p != exchangePricingTrx->exchangeOverrides().dummyFCSegs().end() && p->second > 0)
          return false; // no PFC absorption for dummy fare
      }
    }
  }

  const std::vector<PfcAbsorb*>& pfcAbsorb = trx.dataHandle().getPfcAbsorb(
      travelSeg.origAirport(), carrier, trx.getRequest()->ticketingDT());

  if (pfcAbsorb.empty())
  {
    LOG4CXX_DEBUG(_logger, "pfcAbsorb Database Retrieval *** PfcAbsorption::validPfcItin ***");
    return false;
  }

  travelSegI = _fareUsage->travelSeg().begin();
  TariffNumber fareTariff = _fareUsage->paxTypeFare()->fareTariff(); // lint !e530
  RuleNumber ruleNumber = _fareUsage->paxTypeFare()->ruleNumber(); // lint !e530
  RoutingNumber routingNumber = _fareUsage->paxTypeFare()->routingNumber(); // lint !e530
  FareClassCode fareClass = _fareUsage->paxTypeFare()->fareClass(); // lint !e530

  // For CAT 25 calculated fares we need to swap the PaxTypeFare in the fareUsage object to point
  // to the base fare for rules validation

  if (_fareUsage->paxTypeFare()->isFareByRule() && !_fareUsage->paxTypeFare()->isSpecifiedFare() &&
      _fareUsage->paxTypeFare()->vendor() == Vendor::FMS)
  {
    const PaxTypeFare* ptFare = _fareUsage->paxTypeFare();

    const FBRPaxTypeFareRuleData* fbrPaxTypeFare = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrPaxTypeFare)
    {
      fareTariff = fbrPaxTypeFare->baseFare()->fareTariff();
      ruleNumber = fbrPaxTypeFare->baseFare()->ruleNumber();
      routingNumber = fbrPaxTypeFare->baseFare()->routingNumber();
      fareClass = fbrPaxTypeFare->baseFare()->fareClass();
    }
  }

  uint8_t locTypeFU = setPfcLocType(trx, taxResponse);

  std::vector<PfcAbsorb*>::const_iterator pfcAbsorbI = pfcAbsorb.begin();

  for (; pfcAbsorbI != pfcAbsorb.end(); pfcAbsorbI++)
  {
    if ((*pfcAbsorbI)->geoAppl() < locTypeFU)
      continue;

    if (((*pfcAbsorbI)->fareTariff() != -1) && ((*pfcAbsorbI)->fareTariff() != fareTariff))
      continue;

    if ((!(*pfcAbsorbI)->ruleNo().empty()) && ((*pfcAbsorbI)->ruleNo() != ruleNumber))
      continue;

    if (!(*pfcAbsorbI)->routing2().empty())
    {
      if (((*pfcAbsorbI)->routing1() > routingNumber) ||
          ((*pfcAbsorbI)->routing2() < routingNumber))
        continue;
    }
    else
    {
      if (!(*pfcAbsorbI)->routing1().empty())
      {
        if ((*pfcAbsorbI)->routing1() != routingNumber)
          continue;
      }
    }

    if (!cityMatch(trx, (*pfcAbsorbI)->absorbCity1(), (*pfcAbsorbI)->absorbCity2()))
      continue;

    if (!RuleUtil::matchFareClass((*pfcAbsorbI)->fareClass().c_str(), fareClass.c_str()))
      continue;

    if ((*pfcAbsorbI)->flt1() != -1)
    {
      if (!flightMatch(airSeg, (*pfcAbsorbI)->flt1(), (*pfcAbsorbI)->flt2()))
      {
        if (!flightMatch(airSeg, (*pfcAbsorbI)->flt3(), (*pfcAbsorbI)->flt4()))
          continue;
      }
    }

    bool absorbTypeIndicator = false;
    Itin* itin = taxResponse.farePath()->itin();

    switch ((*pfcAbsorbI)->absorbType())
    {
    case '1':
      _type1 = true;
      return false;

    case '2':
      _type234 = true;

      if (itin->segmentOrder(*travelSegI) == itin->segmentOrder(&travelSeg))
        absorbTypeIndicator = true;

      break;

    case '3':
      _type234 = true;

      if (itin->segmentOrder(*travelSegI) == itin->segmentOrder(&travelSeg))
      {
        absorbTypeIndicator = true;
        break;
      }

      if (!_fareUsage->stopovers().empty())
        absorbTypeIndicator = true;

      break;

    case '4':
      _type234 = true;

      if (itin->segmentOrder(*travelSegI) != itin->segmentOrder(&travelSeg))
        absorbTypeIndicator = true;

      break;

    default:
      break;
    }

    if (absorbTypeIndicator)
      continue;

    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
PfcAbsorption::locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  FarePath* farePath = taxResponse.farePath();
  _firstFareUsage = true;

  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<TravelSeg*>::const_iterator travelSegFuI;

  for (pricingUnitI = farePath->pricingUnit().begin();
       pricingUnitI != farePath->pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      for (travelSegFuI = (*fareUsageI)->travelSeg().begin();
           travelSegFuI != (*fareUsageI)->travelSeg().end();
           travelSegFuI++)
      {
        if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFuI) ==
            taxResponse.farePath()->itin()->segmentOrder(&travelSeg))
          break;
      }

      if (travelSegFuI == (*fareUsageI)->travelSeg().end())
      {
        _firstFareUsage = false;
        continue;
      }

      _pricingUnit = *pricingUnitI;
      _fareUsage = *fareUsageI;
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// Description:  adjustFare
// ----------------------------------------------------------------------------

void
PfcAbsorption::adjustFare(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          const CurrencyCode& absorbCurrency,
                          MoneyAmount absorbAmount)
{
  std::vector<TravelSeg*>::const_iterator travelSegBeginFUI = _fareUsage->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator travelSegEndFUI = _fareUsage->travelSeg().end() - 1;
  std::vector<TravelSeg*>::const_iterator travelSegLocalBoardI;
  std::vector<TravelSeg*>::const_iterator travelSegLocalOffI;

  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if (!(((*taxItemI)->taxCode() == TAX_CODE_US1) && ((*taxItemI)->failCode() == 0)))
      continue;

    travelSegLocalBoardI =
        taxResponse.farePath()->itin()->travelSeg().begin() + ((*taxItemI)->travelSegStartIndex());
    travelSegLocalOffI =
        taxResponse.farePath()->itin()->travelSeg().begin() + ((*taxItemI)->travelSegEndIndex());

    if ((taxResponse.farePath()->itin()->segmentOrder(*travelSegBeginFUI) >=
         taxResponse.farePath()->itin()->segmentOrder(*travelSegLocalBoardI)) &&
        (taxResponse.farePath()->itin()->segmentOrder(*travelSegEndFUI) <=
         taxResponse.farePath()->itin()->segmentOrder(*travelSegLocalOffI)))
    {
      RoundingFactor roundingUnit = (*taxItemI)->taxcdRoundUnit();
      RoundingRule roundingRule = (*taxItemI)->taxcdRoundRule();

      Money moneyPayment(absorbCurrency);

      TaxRound taxRound;

      MoneyAmount result1 = taxRound.applyTaxRound(
          (absorbAmount * (*taxItemI)->taxAmt()), absorbCurrency, roundingUnit, roundingRule);

      MoneyAmount result2 = taxRound.applyTaxRound(
          (result1 * (*taxItemI)->taxAmt()), absorbCurrency, roundingUnit, roundingRule);

      MoneyAmount taxAdjustment = result1 - result2;
      MoneyAmount fareAdjustment = absorbAmount - taxAdjustment;

      if (taxResponse.farePath()->getTotalNUCAmount() < fareAdjustment)
        return;

      CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

      if (!trx.getOptions()->currencyOverride().empty())
        paymentCurrency = trx.getOptions()->currencyOverride();

      if (paymentCurrency != absorbCurrency)
      {
        CurrencyConversionFacade ccFacade;
        Money sourceMoney(taxAdjustment, absorbCurrency);
        Money targetMoney(paymentCurrency);
        ccFacade.convert(
            targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES, false);
        taxAdjustment = targetMoney.value();
      }

      if ((*taxItemI)->taxAmount() > taxAdjustment)
      {
        (*taxItemI)->taxAmount() -= taxAdjustment;
      }
      else
      {
        fareAdjustment = taxAdjustment - (*taxItemI)->taxAmount();
        (*taxItemI)->taxAmount() = 0.0;
      }

      _fareUsage->absorptionAdjustment() += fareAdjustment;
      taxResponse.farePath()->decreaseTotalNUCAmount(fareAdjustment);
      return;
    }
  }
  _fareUsage->absorptionAdjustment() += absorbAmount;
}

// ----------------------------------------------------------------------------
// Description:  cityNotValid
// ----------------------------------------------------------------------------

bool
PfcAbsorption::cityMatch(PricingTrx& trx, LocCode& absorbCity1, LocCode& absorbCity2)
{
  if (absorbCity1.empty() && absorbCity2.empty())
    return true;

  const LocTypeCode marketLocType = LOCTYPE_CITY;

  TravelSeg* travelSegFront = _fareUsage->travelSeg().front();
  TravelSeg* travelSegBack = _fareUsage->travelSeg().back();

  if (!(absorbCity1.empty()) && !(absorbCity2.empty()))
  {
    bool locOrigin = LocUtil::isInLoc(*travelSegFront->origin(),
                                      marketLocType,
                                      absorbCity1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    bool locDestination = LocUtil::isInLoc(*travelSegBack->destination(),
                                           marketLocType,
                                           absorbCity2,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

    if (locOrigin && locDestination)
      return true;

    locOrigin = LocUtil::isInLoc(*travelSegFront->origin(),
                                 marketLocType,
                                 absorbCity2,
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*travelSegBack->destination(),
                                      marketLocType,
                                      absorbCity1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    if (locOrigin && locDestination)
      return true;

    return false;
  }

  if (!absorbCity1.empty())
  {
    bool locOrigin = LocUtil::isInLoc(*travelSegFront->origin(),
                                      marketLocType,
                                      absorbCity1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    bool locDestination = LocUtil::isInLoc(*travelSegBack->destination(),
                                           marketLocType,
                                           absorbCity1,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

    if (!locOrigin && !locDestination)
      return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Description:  fltNotValid
// ----------------------------------------------------------------------------

bool
PfcAbsorption::flightMatch(const AirSeg* airSeg, FlightNumber flight1, FlightNumber flight2)
{
  if ((flight1 == 0) && (flight2 == 0))
    return true;

  if ((flight2 == 0) && (flight1 == airSeg->marketingFlightNumber()))
    return true;

  if ((flight1 > airSeg->marketingFlightNumber()) || (airSeg->marketingFlightNumber() > flight2))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  setPfcLocType
// ----------------------------------------------------------------------------

uint8_t
PfcAbsorption::setPfcLocType(PricingTrx& trx, TaxResponse& taxResponse)
{
  bool boardPointUS = false;
  bool boardPointUSTerritory = false;
  bool boardPointUSPossesion = false;

  std::vector<TravelSeg*>::const_iterator travelSegFuI = _fareUsage->travelSeg().begin();

  for (; travelSegFuI != _fareUsage->travelSeg().end(); travelSegFuI++)
  {
    if ((LocUtil::isUS(*(*travelSegFuI)->origin())) &&
        !(LocUtil::isUSTerritoryOnly(*(*travelSegFuI)->origin())))
      boardPointUS = true;

    if ((LocUtil::isUSTerritoryOnly(*(*travelSegFuI)->origin())))
      boardPointUSTerritory = true;

    if ((LocUtil::isUSPossession(*(*travelSegFuI)->origin())) &&
        !(LocUtil::isUS(*(*travelSegFuI)->origin())))
      boardPointUSPossesion = true;
  }

  if ((boardPointUS) && (!boardPointUSTerritory) && (!boardPointUSPossesion))
    return GEO_TYPE_1;

  if ((boardPointUS) && ((boardPointUSTerritory) || (boardPointUSPossesion)))
    return GEO_TYPE_2;

  if ((!boardPointUS) && ((boardPointUSTerritory) || (boardPointUSPossesion)))
    return GEO_TYPE_3;

  return GEO_TYPE_4;
}
