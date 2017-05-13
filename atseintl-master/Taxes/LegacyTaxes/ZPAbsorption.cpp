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

#include "Taxes/LegacyTaxes/ZPAbsorption.h"

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
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSegAbsorb.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
ZPAbsorption::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.ZPAbsorption"));

const string
ZPAbsorption::TAX_CODE_US1("US1");
const string
ZPAbsorption::TAX_CODE_ZP("ZP");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

ZPAbsorption::ZPAbsorption() : _noSeg1(0), _noSeg2(0), _pricingUnit(nullptr), _fareUsage(nullptr) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

ZPAbsorption::~ZPAbsorption() {}

// ----------------------------------------------------------------------------
// Description:  ZPAbsorption
// ----------------------------------------------------------------------------

void
ZPAbsorption::applyZPAbsorption(PricingTrx& trx, TaxResponse& taxResponse)
{
  TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

  if (taxTrx)
    return;

  if (!locateZPTax(taxResponse))
    return;

  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if ((!(*travelSegI)->origin()->bufferZoneInd()) &&
        (!LocUtil::isUS(*(*travelSegI)->origin()) ||
         LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin())))
    {
      LOG4CXX_DEBUG(_logger, "International Itinerary *** ZPAbsorption::applyZPAbsorption ***");

      return;
    }
  }

  MoneyAmount absorbAmount;
  CurrencyCode absorbCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    absorbCurrency = trx.getOptions()->currencyOverride();
  }

  uint16_t segmentOrder = 0;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX) &&
        (trx.getRequest()->ticketingAgent() != nullptr ||
         trx.getRequest()->ticketingAgent()->agentTJR() != nullptr))
    {
      ExchangePricingTrx* exchangePricingTrx = dynamic_cast<ExchangePricingTrx*>(&trx);
      if (exchangePricingTrx != nullptr)
      { // check if the travel segment is in the map to skip the ZP on that travel segment
        std::map<const TravelSeg*, uint16_t>::const_iterator p;
        p = exchangePricingTrx->exchangeOverrides().dummyFCSegs().find(*travelSegI);
        if (p != exchangePricingTrx->exchangeOverrides().dummyFCSegs().end() && p->second > 0)
          continue; // no ZP absorption for dummy fare
      }
    }

    if (segmentOrder >= taxResponse.farePath()->itin()->segmentOrder(*travelSegI))
      continue;

    if (!locateFareUsage(taxResponse, **travelSegI))
      continue;

    if (!validZPItin(trx, taxResponse, **travelSegI))
      continue;

    absorbAmount = accumulateZP(trx, taxResponse);

    if (!absorbAmount)
      continue;

    adjustFare(trx, taxResponse, absorbCurrency, absorbAmount);

    segmentOrder = taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().back());
  }
}

// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
ZPAbsorption::locateZPTax(TaxResponse& taxResponse)
{
  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if ((*taxItemI)->taxCode() == TAX_CODE_ZP)
    {
      if ((*taxItemI)->taxAmount() > 1)
        return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// Description:  isValidZP
// ----------------------------------------------------------------------------

bool
ZPAbsorption::validZPItin(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  if (taxResponse.farePath()->getTotalNUCAmount() == 0)
    return false;

  std::vector<TravelSeg*>::const_iterator travelSegFuI = _fareUsage->travelSeg().begin();

  const AirSeg* airSeg;
  airSeg = dynamic_cast<const AirSeg*>(*travelSegFuI);

  if (!airSeg)
    return false;

  CarrierCode carrier = airSeg->marketingCarrierCode();

  for (; travelSegFuI != _fareUsage->travelSeg().end(); travelSegFuI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegFuI);

    if (!airSeg)
      return false;

    if (carrier != airSeg->marketingCarrierCode())
      return false;
  }

  travelSegFuI = _fareUsage->travelSeg().begin();

  const std::vector<TaxSegAbsorb*>& taxSegAbsorb =
      trx.dataHandle().getTaxSegAbsorb(carrier, (*travelSegFuI)->departureDT());

  if (taxSegAbsorb.empty())
  {
    LOG4CXX_DEBUG(_logger, "ZPAbsorb Database Retrieval *** ZPAbsorption::validZPItin ***");

    return false;
  }

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

  std::vector<TaxSegAbsorb*>::const_iterator taxSegAbsorbI = taxSegAbsorb.begin();

  for (; taxSegAbsorbI != taxSegAbsorb.end(); taxSegAbsorbI++)
  {
    if (!(*taxSegAbsorbI)->carrier().empty())
    {
      if ((*taxSegAbsorbI)->carrier() != carrier)
        continue;
    }

    if (((*taxSegAbsorbI)->fareTariffNo() != -1) &&
        ((*taxSegAbsorbI)->fareTariffNo() != fareTariff))
      continue;

    if ((!(*taxSegAbsorbI)->ruleNo().empty()) && ((*taxSegAbsorbI)->ruleNo() != ruleNumber))
      continue;

    if (!(*taxSegAbsorbI)->routing2().empty())
    {
      if (((*taxSegAbsorbI)->routing1() > routingNumber) ||
          ((*taxSegAbsorbI)->routing2() < routingNumber))
        continue;
    }
    else
    {
      if (!(*taxSegAbsorbI)->routing1().empty())
      {
        if ((*taxSegAbsorbI)->routing1() != routingNumber)
          continue;
      }
    }

    if (!RuleUtil::matchFareClass((*taxSegAbsorbI)->fareClass().c_str(), fareClass.c_str()))
      continue;

    if (!locMatch(trx,
                  (*taxSegAbsorbI)->loc1().locType(),
                  (*taxSegAbsorbI)->loc1().loc(),
                  (*taxSegAbsorbI)->loc2().locType(),
                  (*taxSegAbsorbI)->loc2().loc()))
      continue;

    if (!locViaMatch(trx,
                     travelSeg,
                     (*taxSegAbsorbI)->loc1().locType(),
                     (*taxSegAbsorbI)->loc1().loc(),
                     (*taxSegAbsorbI)->loc2().locType(),
                     (*taxSegAbsorbI)->loc2().loc(),
                     (*taxSegAbsorbI)->betwAndViaLoc1().locType(),
                     (*taxSegAbsorbI)->betwAndViaLoc1().loc(),
                     (*taxSegAbsorbI)->betwAndViaLoc2().locType(),
                     (*taxSegAbsorbI)->betwAndViaLoc2().loc()))
      continue;

    if ((*taxSegAbsorbI)->flt1() != -1)
    {
      if (!flightMatch(airSeg, (*taxSegAbsorbI)->flt1(), (*taxSegAbsorbI)->flt2()))
      {
        if (!flightMatch(airSeg, (*taxSegAbsorbI)->flt3(), (*taxSegAbsorbI)->flt4()))
          continue;
      }
    }

    break;
  }

  if (taxSegAbsorbI == taxSegAbsorb.end())
  {
    LOG4CXX_DEBUG(_logger, "No Valid TaxSegAbsorb *** ZPAbsorption::validZPItin ***");

    return false;
  }

  if ((*taxSegAbsorbI)->absorptionInd() == '1')
  {
    LOG4CXX_DEBUG(_logger, "Absorbtion Indicator *** ZPAbsorption::validZPItin ***");

    return false;
  }

  _noSeg1 = (*taxSegAbsorbI)->noSeg1();
  _noSeg2 = (*taxSegAbsorbI)->noSeg2();

  return true;
}

// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
ZPAbsorption::locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  FarePath* farePath = taxResponse.farePath();

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
        {
          _pricingUnit = *pricingUnitI;
          _fareUsage = *fareUsageI;
          return true;
        }
      }
    }
  }

  LOG4CXX_DEBUG(_logger, "No Fare Usage *** ZPAbsorption::locateFareUsage ***");

  return false;
}

// ----------------------------------------------------------------------------
// Description:  adjustFare
// ----------------------------------------------------------------------------

void
ZPAbsorption::adjustFare(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         const CurrencyCode& absorbCurrency,
                         MoneyAmount absorbAmount)
{
  TravelSeg* travelSegFront = _fareUsage->travelSeg().front();
  TravelSeg* travelSegBack = _fareUsage->travelSeg().back();

  TravelSeg* travelSegLocalBoard;
  TravelSeg* travelSegLocalOff;

  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if (!(((*taxItemI)->taxCode() == TAX_CODE_US1) && ((*taxItemI)->failCode() == 0)))
      continue;

    travelSegLocalBoard =
        taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegStartIndex()];
    travelSegLocalOff =
        taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegEndIndex()];

    if ((taxResponse.farePath()->itin()->segmentOrder(travelSegFront) >=
         taxResponse.farePath()->itin()->segmentOrder(travelSegLocalBoard)) &&
        (taxResponse.farePath()->itin()->segmentOrder(travelSegBack) <=
         taxResponse.farePath()->itin()->segmentOrder(travelSegLocalOff)))
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
// Description:  locMatch
// ----------------------------------------------------------------------------

bool
ZPAbsorption::locMatch(
    PricingTrx& trx, LocTypeCode& loc1Type, LocCode& loc1, LocTypeCode& loc2Type, LocCode& loc2)
{
  if (loc1.empty() && loc2.empty())
    return true;

  TravelSeg* travelSegFuFront = _fareUsage->travelSeg().front();
  TravelSeg* travelSegFuBack = _fareUsage->travelSeg().back();

  if (!(loc1.empty()) && !(loc2.empty()))
  {
    bool locOrigin = LocUtil::isInLoc(*travelSegFuFront->origin(),
                                      loc1Type,
                                      loc1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    bool locDestination = LocUtil::isInLoc(*travelSegFuBack->destination(),
                                           loc2Type,
                                           loc2,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

    if (locOrigin && locDestination)
      return true;

    locOrigin = LocUtil::isInLoc(*travelSegFuFront->origin(),
                                 loc2Type,
                                 loc2,
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*travelSegFuBack->destination(),
                                      loc1Type,
                                      loc1,
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

  if (!loc1.empty())
  {
    bool locOrigin = LocUtil::isInLoc(*travelSegFuFront->origin(),
                                      loc1Type,
                                      loc1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    bool locDestination = LocUtil::isInLoc(*travelSegFuBack->destination(),
                                           loc1Type,
                                           loc1,
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
// Description:  locViaMatch
// ----------------------------------------------------------------------------

bool
ZPAbsorption::locViaMatch(PricingTrx& trx,
                          TravelSeg& travelSeg,
                          LocTypeCode& loc1Type,
                          LocCode& loc1,
                          LocTypeCode& loc2Type,
                          LocCode& loc2,
                          LocTypeCode& betwAndViaLoc1Type,
                          LocCode& betwAndViaLoc1,
                          LocTypeCode& betwAndViaLoc2Type,
                          LocCode& betwAndViaLoc2)
{
  if (betwAndViaLoc1.empty() && betwAndViaLoc2.empty())
    return true;

  if (!(betwAndViaLoc1.empty()) && !(betwAndViaLoc2.empty()))
  {
    bool locOrigin = LocUtil::isInLoc(*travelSeg.origin(),
                                      betwAndViaLoc1Type,
                                      betwAndViaLoc1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES);

    bool locDestination = LocUtil::isInLoc(*travelSeg.destination(),
                                           betwAndViaLoc2Type,
                                           betwAndViaLoc2,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

    if (locOrigin && locDestination)
      return true;

    locOrigin = LocUtil::isInLoc(*travelSeg.origin(),
                                 betwAndViaLoc2Type,
                                 betwAndViaLoc2,
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*travelSeg.destination(),
                                      betwAndViaLoc1Type,
                                      loc1,
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

  if (!betwAndViaLoc1.empty())
  {
    std::vector<TravelSeg*>::const_iterator travelSegFuI = _fareUsage->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSegFuLastI = _fareUsage->travelSeg().end() - 1;

    bool locOrigin = LocUtil::isInLoc(*(*travelSegFuI)->origin(),
                                      loc1Type,
                                      loc1,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    bool locViaOrigin = LocUtil::isInLoc(*(*travelSegFuI)->origin(),
                                         betwAndViaLoc1Type,
                                         betwAndViaLoc1,
                                         Vendor::SABRE,
                                         MANUAL,
                                         LocUtil::TAXES,
                                         GeoTravelType::International,
                                         EMPTY_STRING(),
                                         trx.getRequest()->ticketingDT());

    if (locOrigin && locViaOrigin)
      return true;

    bool locDestination = LocUtil::isInLoc(*(*travelSegFuLastI)->destination(),
                                           loc1Type,
                                           loc1,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

    bool locViaDestination = LocUtil::isInLoc(*(*travelSegFuLastI)->destination(),
                                              betwAndViaLoc1Type,
                                              betwAndViaLoc1,
                                              Vendor::SABRE,
                                              MANUAL,
                                              LocUtil::TAXES,
                                              GeoTravelType::International,
                                              EMPTY_STRING(),
                                              trx.getRequest()->ticketingDT());

    if (locDestination && locViaDestination)
      return true;

    locOrigin = LocUtil::isInLoc(*(*travelSegFuI)->origin(),
                                 loc2Type,
                                 loc2,
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locViaOrigin = LocUtil::isInLoc(*(*travelSegFuI)->origin(),
                                    betwAndViaLoc1Type,
                                    betwAndViaLoc1,
                                    Vendor::SABRE,
                                    MANUAL,
                                    LocUtil::TAXES,
                                    GeoTravelType::International,
                                    EMPTY_STRING(),
                                    trx.getRequest()->ticketingDT());

    if (locOrigin && locViaOrigin)
      return true;

    locDestination = LocUtil::isInLoc(*(*travelSegFuLastI)->destination(),
                                      loc2Type,
                                      loc2,
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    locViaDestination = LocUtil::isInLoc(*(*travelSegFuLastI)->destination(),
                                         betwAndViaLoc1Type,
                                         betwAndViaLoc1,
                                         Vendor::SABRE,
                                         MANUAL,
                                         LocUtil::TAXES,
                                         GeoTravelType::International,
                                         EMPTY_STRING(),
                                         trx.getRequest()->ticketingDT());

    if (locDestination && locViaDestination)
      return true;

    const AirSeg* airSeg;

    for (; travelSegFuI < travelSegFuLastI; travelSegFuI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegFuI);

      if (!airSeg)
        continue;

      locViaDestination = LocUtil::isInLoc(*(*travelSegFuI)->destination(),
                                           betwAndViaLoc1Type,
                                           betwAndViaLoc1,
                                           Vendor::SABRE,
                                           MANUAL,
                                           LocUtil::TAXES,
                                           GeoTravelType::International,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT());

      if (locViaDestination)
        return true;
    }
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// Description:  fltNotValid
// ----------------------------------------------------------------------------

bool
ZPAbsorption::flightMatch(const AirSeg* airSeg, FlightNumber flight1, FlightNumber flight2)
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
// Description:  accumulateZP
// ----------------------------------------------------------------------------

MoneyAmount
ZPAbsorption::accumulateZP(PricingTrx& trx, TaxResponse& taxResponse)
{
  uint32_t segNo = 0;
  MoneyAmount moneyAmount = 0.0;

  TravelSeg* travelSegFront = _fareUsage->travelSeg().front();
  TravelSeg* travelSegBack = _fareUsage->travelSeg().back();

  TravelSeg* travelSegLocalBoard;
  TravelSeg* travelSegLocalOff;

  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if (!(((*taxItemI)->taxCode() == TAX_CODE_ZP) && ((*taxItemI)->failCode() == 0)))
      continue;

    travelSegLocalBoard =
        taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegStartIndex()];
    travelSegLocalOff =
        taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegEndIndex()];

    if (taxResponse.farePath()->itin()->segmentOrder(travelSegLocalBoard) <
        taxResponse.farePath()->itin()->segmentOrder(travelSegFront))
      continue;

    if (!(taxResponse.farePath()->itin()->segmentOrder(travelSegLocalBoard) >=
              taxResponse.farePath()->itin()->segmentOrder(travelSegFront) &&
          taxResponse.farePath()->itin()->segmentOrder(travelSegLocalOff) <=
              taxResponse.farePath()->itin()->segmentOrder(travelSegBack)))
      continue;

    segNo++;
    if (segNo >= _noSeg1 && segNo <= _noSeg2)
    {
      moneyAmount += (*taxItemI)->taxAmt();
    }
  }
  return moneyAmount;
}
