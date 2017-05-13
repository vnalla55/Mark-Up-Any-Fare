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

#include "Common/Assert.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/PartialTaxableFare.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace std;

namespace tse
{

namespace
{
Logger logger("atseintl.Taxes.PartialTaxableFare");
}

// ----------------------------------------------------------------------------
// Description:  locateLocalFare
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::locate(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  const AirSeg* airSeg;

  // lint -e{530}
  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg().begin()[travelSegIndex];

  if (!locatedThruFare(trx, taxResponse, *travelSeg))
  {
    LOG4CXX_WARN(logger, "Thru Fare Not Located");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);

    return false;
  }

  TSE_ASSERT(_fareUsage != nullptr);

  bool international = false;
  std::vector<TravelSeg*>::const_iterator travelSegGateWayI = _fareUsage->travelSeg().begin();

  for (; travelSegGateWayI != _fareUsage->travelSeg().end(); travelSegGateWayI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegGateWayI);

    if (!airSeg)
      continue;

    if ((!airSeg->origin()->bufferZoneInd()) &&
        (!LocUtil::isUS(*airSeg->origin()) || LocUtil::isUSTerritoryOnly(*airSeg->origin())))
    {
      international = true;
      break;
    }

    if ((!airSeg->destination()->bufferZoneInd()) &&
        (!LocUtil::isUS(*airSeg->destination()) ||
         LocUtil::isUSTerritoryOnly(*airSeg->destination())))
    {
      international = true;
      break;
    }
  }

  bool gatewayLocated = false;
  uint16_t taxExemptPrimaryTrvlSegStartOrder = 0;
  uint16_t taxExemptPrimaryTrvlSegEndOrder = 0;
  uint16_t taxExemptSecondaryTrvlSegStartOrder = 0;
  uint16_t taxExemptSecondaryTrvlSegEndOrder = 0;

  if (international)
  {
    gatewayLocated = locateGateways(trx,
                                    taxResponse,
                                    *travelSeg,
                                    taxExemptPrimaryTrvlSegStartOrder,
                                    taxExemptPrimaryTrvlSegEndOrder,
                                    taxExemptSecondaryTrvlSegStartOrder,
                                    taxExemptSecondaryTrvlSegEndOrder);
  }

  if (!gatewayLocated)
  {
    _taxablePartialFare = _thruTotalFare;
    _travelSegLocalStartOrder =
        taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().front());
    _travelSegLocalEndOrder =
        taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().back());
    _specialPercentage = assignSpecialPercentage(
        trx, taxResponse, taxCodeReg, _travelSegLocalStartOrder, _travelSegLocalEndOrder);
    return true;
  }

  if (travelSeg->origin()->bufferZoneInd())
  {
    LOG4CXX_INFO(logger, "Buffer Zones Countries Do Not Partial Tax");
    return false;
  }

  //
  // Tax Trx has a supplied fare and will not match ATSEI partial fares for super fare hammer
  // request currently.
  // Do not apply partial fare logic for Tax Trx requests.
  //

  if (TrxUtil::isPricingTaxRequest(&trx))
  {
    LOG4CXX_INFO(logger, "No Partial Taxes Allowed For TaxTrx Requests");
    return false;
  }

  std::set<CarrierCode> govCxrs;
  GoverningCarrier govCxrSel(&trx);

  if(trx.isIataFareSelectionApplicable())
  {
    TravelSeg* dummy = nullptr;
    govCxrSel.selectFirstCrossingGovCxr(_fareUsage->travelSeg(), govCxrs, FMDirection::UNKNOWN, dummy);
  }
  else
    govCxrSel.getGoverningCarrier(_fareUsage->travelSeg(), govCxrs);

  if (govCxrs.empty())
  {
    LOG4CXX_WARN(logger, "Governing Carrier Not Located");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);

    return false;
  }

  _specialPercentage = assignSpecialPercentage(
      trx, taxResponse, taxCodeReg, _travelSegLocalStartOrder, _travelSegLocalEndOrder);

  std::set<CarrierCode>::iterator govCxrI = govCxrs.begin();
  _taxablePartialFare = _thruTotalFare;

  if (!appliedPartial(trx,
                      taxResponse,
                      taxCodeReg,
                      *govCxrI,
                      taxExemptPrimaryTrvlSegStartOrder,
                      taxExemptPrimaryTrvlSegEndOrder))
  {
    if (appliedMileage(
            trx, taxResponse, taxCodeReg, _travelSegLocalStartOrder, _travelSegLocalEndOrder))
      return true;

    LOG4CXX_DEBUG(logger, "Fare and Mileage Not Valid");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);
    return false;
  }

  if (!_taxablePartialFare)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);
    return false;
  }

  if (!taxExemptSecondaryTrvlSegStartOrder)
    return true;

  if (appliedPartial(trx,
                     taxResponse,
                     taxCodeReg,
                     *govCxrI,
                     taxExemptSecondaryTrvlSegStartOrder,
                     taxExemptSecondaryTrvlSegEndOrder))
  {
    if (!_taxablePartialFare)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);
      return false;
    }
    return true;
  }

  _taxablePartialFare = _thruTotalFare;

  if (appliedMileage(trx,
                     taxResponse,
                     taxCodeReg,
                     taxExemptSecondaryTrvlSegStartOrder,
                     taxExemptSecondaryTrvlSegEndOrder))
    return true;

  LOG4CXX_DEBUG(logger, "Fare and Mileage Not Valid");

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);
  return false;
}

// ----------------------------------------------------------------------------
// Description:  locatedThruFare
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::locatedThruFare(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  const AirSeg* airSeg;
  CurrencyConversionFacade ccFacade;

  _thruTotalFare = 0.0;

  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<TravelSeg*>::const_iterator travelSegFuI;

  for (pricingUnitI = taxResponse.farePath()->pricingUnit().begin();
       pricingUnitI != taxResponse.farePath()->pricingUnit().end();
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
        airSeg = dynamic_cast<const AirSeg*>(*travelSegFuI);

        if (!airSeg)
          continue;

        if (taxResponse.farePath()->itin()->segmentOrder(&travelSeg) !=
            taxResponse.farePath()->itin()->segmentOrder(*travelSegFuI))
          continue;

        _fareUsage = *fareUsageI;

        TravelSeg* travelSegFront = (*fareUsageI)->travelSeg().front();
        TravelSeg* travelSegBack = (*fareUsageI)->travelSeg().back();

        _travelSegThruStartOrder = taxResponse.farePath()->itin()->segmentOrder(travelSegFront);
        _travelSegThruEndOrder = taxResponse.farePath()->itin()->segmentOrder(travelSegBack);

        _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

        if (!trx.getOptions()->currencyOverride().empty())
        {
          _paymentCurrency = trx.getOptions()->currencyOverride();
        }

        Money targetMoney(_paymentCurrency);

        targetMoney.value() = 0;

        //  Fare Usage has total Fare Amount function for Taxes

        MoneyAmount moneyAmount = (*fareUsageI)->totalFareAmount();

        if (taxResponse.farePath()->calculationCurrency() !=
            taxResponse.farePath()->baseFareCurrency())
        {
          Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
          targetMoneyOrigination.value() = 0;

          Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

          if (!ccFacade.convert(targetMoneyOrigination,
                                sourceMoneyCalculation,
                                trx,
                                taxResponse.farePath()->itin()->useInternationalRounding()))
          {
            LOG4CXX_WARN(logger,
                         "Currency Convertion Failure To Convert: "
                             << taxResponse.farePath()->calculationCurrency() << " To "
                             << taxResponse.farePath()->baseFareCurrency());

            TaxDiagnostic::collectErrors(
                trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic822);
          }
          moneyAmount = targetMoneyOrigination.value();
        }

        if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
        {
          Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

          if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
          {
            LOG4CXX_WARN(logger,
                         "Currency Convertion Failure To Convert: "
                             << taxResponse.farePath()->baseFareCurrency() << " To "
                             << _paymentCurrency);

            TaxDiagnostic::collectErrors(
                trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic822);
          }
          moneyAmount = targetMoney.value();
        }
        _thruTotalFare = moneyAmount;

        if (_fareUsage->hasSideTrip() && !_hasSideTrip)
        {
          _hasSideTrip = true;

          MoneyAmount sideTripTotal = _thruTotalFare;
          uint16_t travelSegThruStartOrder = _travelSegThruStartOrder;
          uint16_t travelSegThruEndOrder = _travelSegThruEndOrder;
          FareUsage* fareUsage = _fareUsage;

          std::vector<PricingUnit*>::const_iterator puI;
          puI = _fareUsage->sideTripPUs().begin();

          std::vector<PricingUnit*>::const_iterator puEndI;
          puEndI = _fareUsage->sideTripPUs().end();

          for (; puI != puEndI; puI++)
          {
            if ((*puI)->fareUsage().empty())
              continue;

            fareUsageI = (*puI)->fareUsage().begin();

            if ((*fareUsageI)->travelSeg().empty())
              continue;

            travelSegFuI = (*fareUsageI)->travelSeg().begin();

            if (locatedThruFare(trx, taxResponse, **travelSegFuI))
              sideTripTotal += _thruTotalFare;
          }
          _travelSegThruStartOrder = travelSegThruStartOrder;
          _travelSegThruEndOrder = travelSegThruEndOrder;
          _fareUsage = fareUsage;

          _hasSideTrip = false;
          _thruTotalFare = sideTripTotal;
        }

        return true;
      }
    } // END FOR LOOP for Fare Usage
  } // END FOR LOOP for Pricing Unit
  return false;
}

// ----------------------------------------------------------------------------
// Description:  locateGateWays
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::locateGateways(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TravelSeg& travelSeg,
                                   uint16_t& taxExemptPrimaryTrvlSegStartOrder,
                                   uint16_t& taxExemptPrimaryTrvlSegEndOrder,
                                   uint16_t& taxExemptSecondaryTrvlSegStartOrder,
                                   uint16_t& taxExemptSecondaryTrvlSegEndOrder)
{
  TSE_ASSERT(_fareUsage != nullptr);

  const AirSeg* airSeg;
  bool gatewayLocated = false;
  std::vector<TravelSeg*>::const_iterator travelSegGateWayI = _fareUsage->travelSeg().begin();

  for (; travelSegGateWayI != _fareUsage->travelSeg().end(); travelSegGateWayI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegGateWayI);

    if (!airSeg)
      continue;

    if ((travelSeg.origin()->nation() != airSeg->origin()->nation()) &&
        (travelSeg.origin()->nation() != airSeg->destination()->nation()))
    {
      if (!LocUtil::isUSTerritoryOnly(*airSeg->origin()) &&
          !LocUtil::isUSTerritoryOnly(*airSeg->destination()))
        continue;
    }

    if ((travelSeg.origin()->nation() != airSeg->origin()->nation()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->origin()))
    {
      if (LocUtil::isPuertoRico(*airSeg->origin()) || LocUtil::isPuertoRico(*airSeg->destination()))
        continue;

      if (LocUtil::isMexico(*airSeg->origin()) || LocUtil::isMexico(*airSeg->destination()))
        continue;

      taxExemptPrimaryTrvlSegEndOrder = taxResponse.farePath()->itin()->segmentOrder(airSeg);
      _travelSegLocalStartOrder = taxResponse.farePath()->itin()->segmentOrder(&travelSeg);

      for (; travelSegGateWayI != _fareUsage->travelSeg().end() - 1;)
      {
        travelSegGateWayI++;

        airSeg = dynamic_cast<const AirSeg*>(*travelSegGateWayI);

        if (!airSeg)
          continue;

        if ((travelSeg.origin()->nation() != airSeg->destination()->nation()) ||
            LocUtil::isUSTerritoryOnly(*airSeg->destination()))
        {
          travelSegGateWayI--;
          break;
        }
      }

      _travelSegLocalEndOrder = taxResponse.farePath()->itin()->segmentOrder(*travelSegGateWayI);

      taxExemptPrimaryTrvlSegStartOrder =
          taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().front());

      gatewayLocated = true;
      break;
    }

    if ((travelSeg.origin()->nation() != airSeg->destination()->nation()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
    {
      taxExemptPrimaryTrvlSegStartOrder = taxResponse.farePath()->itin()->segmentOrder(airSeg);

      for (; travelSegGateWayI != _fareUsage->travelSeg().begin();)
      {
        travelSegGateWayI--;

        airSeg = dynamic_cast<const AirSeg*>(*travelSegGateWayI);

        if (!airSeg)
          continue;

        if (!_travelSegLocalEndOrder)
          _travelSegLocalEndOrder = taxResponse.farePath()->itin()->segmentOrder(airSeg);

        if ((travelSeg.origin()->nation() != airSeg->origin()->nation()) ||
            LocUtil::isUSTerritoryOnly(*airSeg->origin()))
        {
          travelSegGateWayI++;
          break;
        }
      }

      if (!_travelSegLocalEndOrder)
        _travelSegLocalEndOrder = taxResponse.farePath()->itin()->segmentOrder(*travelSegGateWayI);

      _travelSegLocalStartOrder = taxResponse.farePath()->itin()->segmentOrder(*travelSegGateWayI);

      taxExemptPrimaryTrvlSegEndOrder =
          taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().back());

      return true;
    }
  }

  if (!gatewayLocated)
    return false;

  travelSegGateWayI = _fareUsage->travelSeg().begin();

  for (; travelSegGateWayI != _fareUsage->travelSeg().end(); travelSegGateWayI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegGateWayI) <=
        taxExemptPrimaryTrvlSegEndOrder)
      continue;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegGateWayI);

    if (!airSeg)
      continue;

    if ((travelSeg.origin()->nation() != airSeg->destination()->nation()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
    {
      taxExemptSecondaryTrvlSegStartOrder = taxResponse.farePath()->itin()->segmentOrder(airSeg);
      taxExemptSecondaryTrvlSegEndOrder =
          taxResponse.farePath()->itin()->segmentOrder(_fareUsage->travelSeg().back());

      break;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  appliedPartial
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::appliedPartial(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   const CarrierCode& govCarrier,
                                   const uint16_t taxExemptStartOrder,
                                   const uint16_t taxExemptEndOrder)
{
  TSE_ASSERT(_fareUsage != nullptr);

  std::vector<TravelSeg*>::const_iterator travelSegFromI = _fareUsage->travelSeg().begin();

  for (; travelSegFromI != _fareUsage->travelSeg().end(); travelSegFromI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFromI) == taxExemptStartOrder)
      break;
  }

  std::vector<TravelSeg*>::const_iterator travelSegToI = _fareUsage->travelSeg().begin();

  for (; travelSegToI != _fareUsage->travelSeg().end(); travelSegToI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegToI) == taxExemptEndOrder)
    {
      travelSegToI++;
      break;
    }
  }

  std::vector<TravelSeg*> travelSegFUVec;

  copy(travelSegFromI, travelSegToI, back_inserter(travelSegFUVec));

  const FareMarket* fareMarket = TrxUtil::getFareMarket(
      trx,
      govCarrier,
      travelSegFUVec,
      _fareUsage ? _fareUsage->paxTypeFare()->retrievalDate() : trx.ticketingDate(),
      taxResponse.farePath()->itin());

  if (!fareMarket)
    return false;
    
  if (!appliedFare(trx, *(taxResponse.farePath()), fareMarket))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  appliedFare
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::appliedFare(PricingTrx& trx,
                                FarePath& farePath,
                                const FareMarket* fareMarket)
{ // lint -e{1561}
  const PaxTypeBucket* paxTypeCortege = fareMarket->paxTypeCortege(farePath.paxType());

  if (!paxTypeCortege)
  {
    LOG4CXX_WARN(logger,
                 "Bad FareMarket For PaxType: " << farePath.paxType()->paxType());
    return false;
  }

  bool qualifiedFareLevel1 = false;
  bool qualifiedFareLevel2 = false;
  bool unQualifiedFareLevel3 = false;
  bool unQualifiedFareLevel4 = false;
  bool unQualifiedFareLevel5 = false;
  bool unQualifiedFareLevel6 = false;

  const std::vector<PaxTypeFare*>& paxTypeFare = paxTypeCortege->paxTypeFare();
  std::vector<PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFare.begin();
  std::vector<PaxTypeFare*>::const_iterator paxTypeFareSetI = paxTypeFare.end();

  const CarrierCode itinVcxr = farePath.itin()->validatingCarrier();

  TSE_ASSERT(_fareUsage != nullptr);
  for (; paxTypeFareI != paxTypeFare.end(); paxTypeFareI++)
  {
    
    if ((_fareUsage->paxTypeFare()->directionality() != (*paxTypeFareI)->directionality()) &&
        (_fareUsage->paxTypeFare()->directionality() != BOTH))
      continue;

    const bool isValidForItinVcxr = isValidForValidatingCarrier(trx, (**paxTypeFareI), itinVcxr);

    if (((*paxTypeFareI)->isValid()) && isValidForItinVcxr &&
        (_fareUsage->paxTypeFare()->owrt() == (*paxTypeFareI)->fare()->owrt()) &&
        (_fareUsage->paxTypeFare()->fareClass() == (*paxTypeFareI)->fareClass()))
    {
      paxTypeFareSetI = paxTypeFareI;
      qualifiedFareLevel1 = true;
      break;
    }

    if (qualifiedFareLevel2)
      continue;

    if (((*paxTypeFareI)->isValid()) && isValidForItinVcxr &&
        (_fareUsage->paxTypeFare()->owrt() == (*paxTypeFareI)->fare()->owrt()) &&
        (_fareUsage->paxTypeFare()->fcaFareType() == (*paxTypeFareI)->fcaFareType()))
    {
      paxTypeFareSetI = paxTypeFareI;
      qualifiedFareLevel2 = true;
      continue;
    }

    if (unQualifiedFareLevel3)
      continue;

    if ((_fareUsage->paxTypeFare()->owrt() == (*paxTypeFareI)->fare()->owrt()) &&
        (_fareUsage->paxTypeFare()->fareClass() == (*paxTypeFareI)->fareClass()))
    {
      paxTypeFareSetI = paxTypeFareI;
      unQualifiedFareLevel3 = true;
      continue;
    }

    if (unQualifiedFareLevel4)
      continue;

    if ((_fareUsage->paxTypeFare()->owrt() == (*paxTypeFareI)->fare()->owrt()) &&
        (_fareUsage->paxTypeFare()->fcaFareType() == (*paxTypeFareI)->fcaFareType()))
    {
      paxTypeFareSetI = paxTypeFareI;
      unQualifiedFareLevel4 = true;
      continue;
    }

    if (unQualifiedFareLevel5)
      continue;

    if (_fareUsage->paxTypeFare()->fareClass() == (*paxTypeFareI)->fareClass())
    {
      paxTypeFareSetI = paxTypeFareI;
      unQualifiedFareLevel5 = true;
      continue;
    }

    if (unQualifiedFareLevel6)
      continue;

    if (_fareUsage->paxTypeFare()->fcaFareType() == (*paxTypeFareI)->fcaFareType())
    {
      paxTypeFareSetI = paxTypeFareI;
      unQualifiedFareLevel6 = true;
      continue;
    }
  }

  _needsReprice = fareMarket->canValidateFBRBaseFares() && !(qualifiedFareLevel1 || qualifiedFareLevel2);

  if (_needsReprice)
    return false;

  if (paxTypeFareSetI == paxTypeFare.end())
  {
    LOG4CXX_DEBUG(
        logger,
        "No Matching Fare Located For PaxType: " << farePath.paxType()->paxType());
    return false;
  }

  MoneyAmount moneyAmount = (*paxTypeFareSetI)->totalFareAmount();

  const std::vector<TravelSeg*>& travelSegs = fareMarket->travelSeg();
  std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator travelSegIterEnd = travelSegs.end();

  for (; travelSegIter != travelSegIterEnd; travelSegIter++)
  {
    moneyAmount += addStopOverSurcharge(*travelSegIter);
    moneyAmount += addTransferSurcharge(*travelSegIter);
  }

  moneyAmount += addCAT12Surcharge(fareMarket);

  TSE_ASSERT(_fareUsage != nullptr);

  if (_fareUsage->minFarePlusUpAmt())
  {
    if (applyMinFarePlusUp(fareMarket))
      moneyAmount += _fareUsage->minFarePlusUpAmt();
  }

  CurrencyConversionFacade ccFacade;

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  if (farePath.calculationCurrency() != farePath.baseFareCurrency())
  {
    Money targetMoneyOrigination(farePath.baseFareCurrency());
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, farePath.calculationCurrency());

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          farePath.itin()->useInternationalRounding()))
    {
      LOG4CXX_WARN(logger,
                   "Currency Convertion Failure To Convert: "
                       << farePath.calculationCurrency() << " To "
                       << farePath.baseFareCurrency());
    }
    moneyAmount = targetMoneyOrigination.value();
  }

  if (farePath.baseFareCurrency() != _paymentCurrency)
  {
    Money sourceMoney(moneyAmount, farePath.baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
    {
      LOG4CXX_WARN(logger,
                   "Currency Convertion Failure To Convert: "
                       << farePath.baseFareCurrency() << " To " << _paymentCurrency);
    }
    moneyAmount = targetMoney.value();
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(2);
    stream << "-" << moneyAmount << " MARKET:" << (*paxTypeFareSetI)->fareMarket()->origin()->loc();
    stream << (*paxTypeFareSetI)->fareMarket()->destination()->loc()
           << " FB:" << (*paxTypeFareSetI)->createFareBasis(trx);
    stream << " V:" << (*paxTypeFareSetI)->fare()->vendor()
           << " C:" << (*paxTypeFareSetI)->fare()->carrier();
    stream << " T:" << (*paxTypeFareSetI)->fare()->fareTariff()
           << " R:" << (*paxTypeFareSetI)->fare()->ruleNumber() << std::endl;
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  MoneyAmount partialAmount = _thruTotalFare - _taxablePartialFare + moneyAmount;

  if (_thruTotalFare <= partialAmount)
  {
    LOG4CXX_DEBUG(logger,
                  "Partial Tax Fare :" << partialAmount
                                       << " Is Greater Than Or Equal To Thru Fare: "
                                       << _thruTotalFare);

    _taxablePartialFare = 0.0;
    return true;
  }
  _taxablePartialFare -= moneyAmount;
  return true;
}

bool
PartialTaxableFare::isValidForValidatingCarrier(
    const PricingTrx& trx, const PaxTypeFare& ptf, const CarrierCode& vcxr) const
{
  if ( !trx.isValidatingCxrGsaApplicable() )
    return true;

  bool isValid = true;
  const std::vector<CarrierCode>& ptfVcxrList = ptf.validatingCarriers();
  if ( !ptfVcxrList.empty() &&
       std::find( ptfVcxrList.begin(), ptfVcxrList.end(), vcxr ) == ptfVcxrList.end() )
  {
    isValid = false;
  }

  return isValid;
}
// ----------------------------------------------------------------------------
// Description:  appliedMileage
// ----------------------------------------------------------------------------

bool
PartialTaxableFare::appliedMileage(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   const uint16_t startOrder,
                                   const uint16_t endOrder)
{
  TSE_ASSERT(_fareUsage != nullptr);

  std::vector<TravelSeg*>::const_iterator travelSegFromI = _fareUsage->travelSeg().begin();

  for (; travelSegFromI != _fareUsage->travelSeg().end(); travelSegFromI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFromI) == startOrder)
      break;
  }

  std::vector<TravelSeg*>::const_iterator travelSegToI = _fareUsage->travelSeg().begin();

  for (; travelSegToI != _fareUsage->travelSeg().end(); travelSegToI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegToI) == endOrder)
      break;
  }

  if ((travelSegFromI == _fareUsage->travelSeg().end()) ||
      (travelSegToI == _fareUsage->travelSeg().end()) || (startOrder > endOrder))
  {
    LOG4CXX_DEBUG(logger, "Travel Segment Not Located");
    return false;
  }

  std::vector<TravelSeg*> partialTravelSegs;

  GlobalDirection gd = GlobalDirection::XX;
  partialTravelSegs.insert(partialTravelSegs.begin(), travelSegFromI, (travelSegToI + 1));
  DateTime travelDate = taxResponse.farePath()->itin()->travelDate();
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, partialTravelSegs, gd);

  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_DEBUG(logger, "GlobalDirection Not Located");
    return false;
  }

  _partialLocalMiles = LocUtil::getTPM(*(*travelSegFromI)->origin(),
                                       *(*travelSegToI)->destination(),
                                       gd,
                                       trx.getRequest()->ticketingDT(),
                                       trx.dataHandle());

  travelSegFromI = _fareUsage->travelSeg().begin();
  travelSegToI = _fareUsage->travelSeg().end() - 1;

  partialTravelSegs.clear();
  partialTravelSegs.insert(partialTravelSegs.begin(), travelSegFromI, (travelSegToI + 1));
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, partialTravelSegs, gd);

  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_DEBUG(logger,
                  "GlobalDirection Not Located From: " << (*travelSegFromI)->origin()->loc()
                                                       << " To "
                                                       << (*travelSegToI)->destination()->loc());
    return false;
  }

  _partialThruMiles = LocUtil::getTPM(*(*travelSegFromI)->origin(),
                                      *(*travelSegToI)->destination(),
                                      gd,
                                      trx.getRequest()->ticketingDT(),
                                      trx.dataHandle());

  if ((_partialThruMiles < _partialLocalMiles) || (_partialThruMiles == 0))
  {
    LOG4CXX_DEBUG(logger,
                  "Partial Thru Miles: " << _partialThruMiles
                                         << " Is Less Than Partial Local Miles: "
                                         << _partialLocalMiles);
    return false;
  }

  MoneyAmount moneyAmount =
      static_cast<uint64_t>((_thruTotalFare * _partialLocalMiles) / _partialThruMiles);

  if (moneyAmount < 0.0)
  {
    LOG4CXX_DEBUG(logger, "Mileage Calculation Tax Is Not Applicable");
    return false;
  }
  _taxablePartialFare = moneyAmount;
  return true;
}

// ----------------------------------------------------------------------------
// Description:  assignSpecialPercentage
// ----------------------------------------------------------------------------

MoneyAmount
PartialTaxableFare::assignSpecialPercentage(PricingTrx& trx,
                                            TaxResponse& taxResponse,
                                            TaxCodeReg& taxCodeReg,
                                            const uint16_t startOrder,
                                            const uint16_t endOrder)
{
  TSE_ASSERT(_fareUsage != nullptr);

  std::vector<TravelSeg*>::const_iterator travelSegFromI = _fareUsage->travelSeg().begin();

  for (; travelSegFromI != _fareUsage->travelSeg().end(); travelSegFromI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFromI) == startOrder)
      break;
  }

  std::vector<TravelSeg*>::const_iterator travelSegToI = _fareUsage->travelSeg().begin();

  for (; travelSegToI != _fareUsage->travelSeg().end(); travelSegToI++)
  {
    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegToI) == endOrder)
      break;
  }

  if ((travelSegFromI == _fareUsage->travelSeg().end()) ||
      (travelSegToI == _fareUsage->travelSeg().end()))
  {
    LOG4CXX_DEBUG(logger, "Travel Segment Not Located");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTIAL_TAX, Diagnostic822);

    return false;
  }

  if (!(LocUtil::isHawaii(*(*travelSegFromI)->origin()) ||
        LocUtil::isHawaii(*(*travelSegToI)->destination()) ||
        LocUtil::isAlaska(*(*travelSegFromI)->origin()) ||
        LocUtil::isAlaska(*(*travelSegToI)->destination())))
    return 0.0;

  if ((LocUtil::isHawaii(*(*travelSegFromI)->origin()) &&
       LocUtil::isHawaii(*(*travelSegToI)->destination())) ||
      (LocUtil::isAlaska(*(*travelSegFromI)->origin()) &&
       LocUtil::isAlaska(*(*travelSegToI)->destination())))
    return 0.0;

  if (LocUtil::isHawaii(*(*travelSegFromI)->origin()) &&
      !LocUtil::isHawaii(*(*travelSegToI)->destination()))
    return taxUtil::locateHiFactor(trx, (*travelSegToI)->offMultiCity());

  if (!LocUtil::isHawaii(*(*travelSegFromI)->origin()) &&
      LocUtil::isHawaii(*(*travelSegToI)->destination()))
    return taxUtil::locateHiFactor(trx, (*travelSegFromI)->boardMultiCity());

  if (LocUtil::isAlaska(*(*travelSegFromI)->origin()) &&
      !LocUtil::isAlaska(*(*travelSegToI)->destination()))
    return taxUtil::locateAkFactor(trx, (*travelSegFromI)->origin(), (*travelSegToI)->offMultiCity());

  return taxUtil::locateAkFactor(
      trx, (*travelSegToI)->destination(), (*travelSegFromI)->boardMultiCity());
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void PartialTaxableFare::addCAT12Surcharge
// Description:   Get CAT12 surcharge amount for the current TravelSeg.
// </PRE>
// ----------------------------------------------------------------------------

MoneyAmount
PartialTaxableFare::addCAT12Surcharge(const FareMarket* fareMarket)
{
  TSE_ASSERT(_fareUsage != nullptr);

  MoneyAmount moneyAmount = 0.0;

  std::vector<SurchargeData*>::const_iterator surchargeIter = _fareUsage->surchargeData().begin();
  std::vector<SurchargeData*>::const_iterator surchargeIterEnd = _fareUsage->surchargeData().end();

  for (; surchargeIter != surchargeIterEnd; surchargeIter++)
  {
    const std::vector<TravelSeg*>& travelSegs = fareMarket->travelSeg();
    std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
    std::vector<TravelSeg*>::const_iterator travelSegIterEnd = travelSegs.end();

    for (; travelSegIter != travelSegIterEnd; travelSegIter++)
    {
      if ((*surchargeIter)->selectedTkt() &&
          (*travelSegIter)->origAirport() == (*surchargeIter)->brdAirport() &&
          (*travelSegIter)->destAirport() == (*surchargeIter)->offAirport())
      {
        moneyAmount += (*surchargeIter)->amountNuc() * (*surchargeIter)->itinItemCount();
      }
    }
  }
  return moneyAmount;
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void PartialTaxableFare::addStopOverSurcharge
// Description:   Get stop over surcharge amount for the current TravelSeg.
// </PRE>
// ----------------------------------------------------------------------------

MoneyAmount
PartialTaxableFare::addStopOverSurcharge(const TravelSeg* tvlS)
{
  TSE_ASSERT(_fareUsage != nullptr);

  FareUsage::StopoverSurchargeMultiMapCI stopoverSurchargeIter =
      _fareUsage->stopoverSurcharges().find(tvlS);

  MoneyAmount moneyAmount = 0.0;

  if (stopoverSurchargeIter == _fareUsage->stopoverSurcharges().end())
    return moneyAmount;

  const FareUsage::StopoverSurcharge* stopoverSurcharge = stopoverSurchargeIter->second;

  return moneyAmount += stopoverSurcharge->amount();
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void PartialTaxableFare::addTransferSurcharge
// Description:   Get stop over surcharge amount for the current TravelSeg.
// </PRE>
// ----------------------------------------------------------------------------

MoneyAmount
PartialTaxableFare::addTransferSurcharge(const TravelSeg* tvlS)
{
  TSE_ASSERT(_fareUsage != nullptr);

  FareUsage::TransferSurchargeMultiMapCI transferSurchargeIter =
      _fareUsage->transferSurcharges().find(tvlS);

  MoneyAmount moneyAmount = 0.0;

  if (transferSurchargeIter == _fareUsage->transferSurcharges().end())
    return moneyAmount;

  const FareUsage::TransferSurcharge* transferSurcharge = transferSurchargeIter->second;

  return moneyAmount += transferSurcharge->amount();
}

bool
PartialTaxableFare::applyMinFarePlusUp(const FareMarket* fareMarket)
{
  return false;

  // todo call internal min fares for following itinerary type:
  // Minimum fare must apply to the international portion of travel
  // the only way to determine this is to pass the market to min fares
  // internal service. These type of no breads should be rare.
  // ORD-DFW-NRT-SYD no fare breaks and min fare applies to NRT-SYD

  bool boardPoint = false;
  bool offPoint = false;

  TSE_ASSERT(_fareUsage != nullptr);
  const MinFarePlusUp& mfpuV = _fareUsage->minFarePlusUp();
  MinFarePlusUp::const_iterator mfpuI = mfpuV.find(HIP); // It should be only one occurance

  if (mfpuI == mfpuV.end())
    return false;

  MinFarePlusUpItem& mfpuR = *mfpuI->second;

  const std::vector<TravelSeg*>& travelSegs = fareMarket->travelSeg();
  std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator travelSegIterEnd = travelSegs.end();

  for (; travelSegIter != travelSegIterEnd; travelSegIter++)
  {
    const LocCode& offMultiCity = FareMarketUtil::getOffMultiCity(*fareMarket, **travelSegIter);
    const LocCode& boardMultiCity = FareMarketUtil::getBoardMultiCity(*fareMarket, **travelSegIter);

    if (boardMultiCity == mfpuR.boardPoint)
      boardPoint = true;

    if (offMultiCity == mfpuR.offPoint)
      offPoint = true;
  }

  if (boardPoint && offPoint)
    return true;

  return false;
}
} //tse
