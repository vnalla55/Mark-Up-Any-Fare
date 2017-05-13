//-------------------------------------------------------------------
// File:    PricingUtil.cpp
// Created: April 2005
// Authors: Andrew Ahmad
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/PricingUtil.h"

#include "BrandedFares/BrandInfo.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/FarePathCopier.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/FareUsageIter.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/DiscountPricing.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/NetRemitPricing.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/Shopping/Utils/SetIntersection.h"
#include "Rules/Commissions.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <algorithm>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackFRRFixNonItBtDirectTicketing);
FALLBACK_DECL(markupAnyFareOptimization);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(fallbackTagPY9matchITBTCCPayment);
FALLBACK_DECL(fallbackFixMslNetFareAmount);
FALLBACK_DECL(fixSpanishLargeFamilyForSRFE);

namespace
{
Logger
logger("atseintl.Pricing.PricingUtil");

static const std::string JLACCNTCODE = "RX78MS06TM";
static const std::string JLTKTDESGN = "JMBJL";
}

const char PricingUtil::SMF_VENDOR = 'T'; // Sabre MyFare vendor

Itin::ISICode
PricingUtil::calculateFarePathISICode(const PricingTrx& trx, FarePath& farePath)
{
  // If ISI code has already been calculated for fare path.
  if (farePath.intlSaleIndicator() != Itin::UNKNOWN)
  {
    return farePath.intlSaleIndicator();
  }

  Itin::ISICode isiCode = calculateISICode(trx, farePath);
  farePath.intlSaleIndicator() = isiCode;
  return isiCode;
}

Itin::ISICode
PricingUtil::calculateISICode(const PricingTrx& trx, const FarePath& farePath)
{
  const Loc* ticketingLoc;
  const Loc* saleLoc;
  const PricingRequest* request = trx.getRequest();
  DateTime today = trx.transactionStartTime();

  if (request->ticketPointOverride().size())
  {
    ticketingLoc = trx.dataHandle().getLoc(request->ticketPointOverride(), today);
  }
  else
  {
    ticketingLoc = request->ticketingAgent()->agentLocation();
  }

  if (request->salePointOverride().size())
  {
    saleLoc = trx.dataHandle().getLoc(request->salePointOverride(), today);
  }
  else
  {
    saleLoc = request->ticketingAgent()->agentLocation();
  }

  if (UNLIKELY(farePath.pricingUnit().empty()))
  {
    return Itin::UNKNOWN;
  }

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  for (; puIt != puItEnd; ++puIt)
  {
    const PricingUnit* pu = *puIt;

    if (UNLIKELY(pu->fareUsage().empty()))
    {
      return Itin::UNKNOWN;
    }

    std::vector<FareUsage*>::const_iterator fuIt = pu->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuItEnd = pu->fareUsage().end();

    for (; fuIt != fuItEnd; ++fuIt)
    {
      const FareUsage* fu = *fuIt;

      if (UNLIKELY(fu->travelSeg().empty()))
      {
        return Itin::UNKNOWN;
      }

      std::vector<TravelSeg*>::const_iterator tsIt = fu->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsItEnd = fu->travelSeg().end();
      const Loc* departurePoint = (*tsIt)->origin();

      for (; tsIt != tsItEnd; ++tsIt)
      {
        const TravelSeg* ts = *tsIt;

        // Find the first international segment...
        if (LocUtil::isInternational(*(ts->origin()), *(ts->destination())))
        {
          // Then check the departure point of the FareUsage
          //  against the Sales location and Ticketing location.
          if (LocUtil::isSameISINation(*departurePoint, *saleLoc))
          {
            if (LIKELY(LocUtil::isSameISINation(*departurePoint, *ticketingLoc)))
            {
              return Itin::SITI;
            }
            else
            {
              return Itin::SITO;
            }
          }
          else
          {
            if (UNLIKELY(LocUtil::isSameISINation(*departurePoint, *ticketingLoc)))
            {
              return Itin::SOTI;
            }
            else
            {
              return Itin::SOTO;
            }
          }
        }
      }
    }
  }

  // If we get here, then there are no international segments in the FarePath
  // Use the ISI Code from the Itin
  return farePath.itin()->intlSalesIndicator();
}

//-------------------------------------------------------------------
//
//
//   @method determineBaseFare
//
//   Description: Determines the base fare currency on a per fare path basis.
//                For Foreign domestic travel it will roll back the NUC amounts
//                to the original currency if they are all in one currency.
//
//   @param  FarePath      - farePath
//   @param  PricingTrx    - trx
//   @param  Itin          - itinerary
//
//   @return void
//-------------------------------------------------------------------
void
PricingUtil::determineBaseFare(FarePath* farePath, PricingTrx& trx, Itin* itin)
{
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::determineBaseFare");
  CurrencyCode originationCurrency(itin->originationCurrency());
  bool sameCurrencyCodes = false;
  bool multiCurrencyPricing = false;
  bool sameFareChgsCurCodes = true;
  multiCurrencyPricing = ItinUtil::applyMultiCurrencyPricing(&trx, *itin);
  LOG4CXX_DEBUG(logger, "MULTI CURRENCY PRICING VALUE: " << multiCurrencyPricing);
  multiCurrencyPricing |= hasAnyCAT25SpecialFares(farePath);
  LOG4CXX_DEBUG(logger,
                "MULTI CURRENCY PRICING VALUE AFTER CAT 25 CHECK: " << multiCurrencyPricing);
  MoneyAmount totalCat12SurchAmt = 0.0;
  MoneyAmount totalStopOverSurchAmt = 0.0;
  MoneyAmount totalTransfersSurchAmt = 0.0;
  bool hasNoFareConstructionChgs = PricingUtil::hasNoFareConstructionCharges(trx, *farePath);
  LOG4CXX_DEBUG(logger, "hasNoFareConstructionChgs value: " << hasNoFareConstructionChgs);

  farePath->applyNonIATARounding(trx);

  if (itin->geoTravelType() != GeoTravelType::Transborder)
  {
    sameCurrencyCodes = PricingUtil::farePathCurrencyCodesAreSame(trx,
                                                                  farePath,
                                                                  totalCat12SurchAmt,
                                                                  totalStopOverSurchAmt,
                                                                  totalTransfersSurchAmt,
                                                                  multiCurrencyPricing,
                                                                  sameFareChgsCurCodes);
  }

  if (multiCurrencyPricing)
  {
    if (sameCurrencyCodes && sameFareChgsCurCodes && hasNoFareConstructionChgs &&
        (itin->geoTravelType() == GeoTravelType::ForeignDomestic || itin->geoTravelType() == GeoTravelType::Domestic))
    {
      if (trx.fareCalcConfig()->domesticNUC() == NO &&
          farePath->itin()->calcCurrencyOverride().empty())
      {
        PricingUtil::currencyRollBack(
            *farePath, trx, totalCat12SurchAmt, totalStopOverSurchAmt, totalTransfersSurchAmt);
      }
    }
  }
  else if (itin->geoTravelType() == GeoTravelType::ForeignDomestic || itin->geoTravelType() == GeoTravelType::Domestic)
  {
    LOG4CXX_DEBUG(logger, "hasNoFareConstructionChgs value: " << hasNoFareConstructionChgs);
    LOG4CXX_DEBUG(logger, "sameCurrencyCodes value: " << sameCurrencyCodes);

    if (sameCurrencyCodes && sameFareChgsCurCodes && hasNoFareConstructionChgs)
    {
      if (trx.fareCalcConfig()->domesticNUC() == NO &&
          farePath->itin()->calcCurrencyOverride().empty())
      {
        PricingUtil::currencyRollBack(
            *farePath, trx, totalCat12SurchAmt, totalStopOverSurchAmt, totalTransfersSurchAmt);
      }
    }
    else if (!sameCurrencyCodes)
    {
      LOG4CXX_ERROR(
          logger, "DIFFERENT CURRENCY CODES FOR FOREIGN DOMESTIC MARKET - ISSUE SEPARATE TICKETS");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_PRICE_ISSUE_SEPARATE_TICKETS);
    }

    PricingUtil::setFarePathBaseFareCurrency(farePath, trx, originationCurrency, sameCurrencyCodes);
    sameCurrencyCodes &= hasNoFareConstructionChgs;
    sameCurrencyCodes &= sameFareChgsCurCodes;
    PricingUtil::setFarePathCalculationCurrency(
        farePath, trx, originationCurrency, multiCurrencyPricing, sameCurrencyCodes);
    return;
  }

  if (itin->geoTravelType() != GeoTravelType::Transborder)
  {
    if (!sameCurrencyCodes && (itin->geoTravelType() == GeoTravelType::International))
      PricingUtil::checkOriginationCurrency(trx, itin, farePath, originationCurrency);

    PricingUtil::setFarePathBaseFareCurrency(farePath, trx, originationCurrency, sameCurrencyCodes);
  }
  else
  {
    if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
         trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
        !((const PricingTrx&)trx).getOptions()->baseFareCurrencyOverride().empty())
    {
      farePath->baseFareCurrency() =
          ((const PricingTrx&)trx).getOptions()->baseFareCurrencyOverride();
    }
    else
      farePath->baseFareCurrency() = originationCurrency;

    LOG4CXX_DEBUG(logger,
                  "FAREPATH BASE FARE CURRENCY FOR TRANSBORDER: " << farePath->baseFareCurrency());
  }

  if ((itin->geoTravelType() == GeoTravelType::ForeignDomestic || itin->geoTravelType() == GeoTravelType::Domestic) &&
      multiCurrencyPricing)
  {
    sameCurrencyCodes &= hasNoFareConstructionChgs;
    sameCurrencyCodes &= sameFareChgsCurCodes;
  }

  PricingUtil::setFarePathCalculationCurrency(
      farePath, trx, originationCurrency, multiCurrencyPricing, sameCurrencyCodes);
}

//-------------------------------------------------------------------
//
//   @method checkOriginationCurrency
//
//   Description: Checks to see if the origination currency needs to be changed
//                for an international itinerary. This is due to
//                the final fare path containing multiple currency codes
//                or an arunk segment in the itinerary which might
//                mean international travel originates in a different
//                nation.
//
//   @param  PricingTrx   - trx.
//   @param  Itin         - itinerary.
//   @param  FarePath     - farePath.
//   @param  CurrencyCode - originationCurrency.
//
//   @return void
//
//-------------------------------------------------------------------
void
PricingUtil::checkOriginationCurrency(PricingTrx& trx,
                                      const Itin* itin,
                                      FarePath* farePath,
                                      CurrencyCode& originationCurrency)
{
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::checkOriginationCurrency");
  LocCode originMarket;
  NationCode nationCode;

  FareUsageIter fuIter(*farePath);

  for (FareUsageIter::iterator i = fuIter.begin(), iend = fuIter.end(); i != iend; ++i)
  {
    const FareUsage* fareUsage = *i;
    const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
    const FareMarket* fareMarket = paxTypeFare->fareMarket();

    if (fareMarket->geoTravelType() != GeoTravelType::International && fareMarket->geoTravelType() != GeoTravelType::Transborder)
      continue;

    if (fareMarket->origin()->nation() != fareMarket->destination()->nation())
    {
      originMarket = fareMarket->origin()->loc();
      const DateTime& travelDate = trx.adjustedTravelDate(ItinUtil::getTravelDate(itin->travelSeg()));
      const tse::Loc* loc = trx.dataHandle().getLoc(originMarket, travelDate);
      nationCode = loc->nation();
      CurrencyCode pricingCurrency;

      if (CurrencyUtil::getInternationalPricingCurrency(
              nationCode, pricingCurrency, trx.ticketingDate()))
      {
        if (!pricingCurrency.empty())
        {
          if (pricingCurrency != itin->originationCurrency())
            originationCurrency = pricingCurrency;
        }
      }

      return;
    }
  } // for each fare usage

  LOG4CXX_DEBUG(logger, "Leaving PricingUtil::checkOriginationCurrency");
}

//-------------------------------------------------------------------
//
//   @method farePathCurrencyCodesAreSame
//
//   Description: Checks to see if all currency codes for one passenger type
//                for an international itinerary the same.
//
//  @param  FarePath   - farePath
//
//  @return bool   - true - they are all the same, else false.
//
//-------------------------------------------------------------------
bool
tse::PricingUtil::farePathCurrencyCodesAreSame(PricingTrx& trx,
                                               const FarePath* farePath,
                                               MoneyAmount& cat12SurchargeAmount,
                                               MoneyAmount& stopOverSurchAmount,
                                               MoneyAmount& transfersSurchAmount,
                                               bool multiCurrencyPricing,
                                               bool& fareChgsSame)
{
  bool checkRC = true;
  unsigned int multCurrencies = 0;
  std::vector<CurrencyCode> puCurrencies;
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::farePathCurrencyCodesAreSame");
  LOG4CXX_DEBUG(logger, "Number of PU's : " << farePath->pricingUnit().size());
  const std::vector<PricingUnit*>& pu = farePath->pricingUnit();

  for (size_t i = 0; i < pu.size(); i++)
  {
    PricingUnit* priceableUnit = const_cast<PricingUnit*>(pu[i]);
    const std::vector<FareUsage*>& fareUsageVector = priceableUnit->fareUsage();

    // hasMultipleCurrency loops through all of the fare usages for this pu
    //
    if (priceableUnit->hasMultipleCurrency())
    {
      LOG4CXX_DEBUG(logger, "PU has multiple currencies");
      multCurrencies++;
    }
    else
    {
      LOG4CXX_DEBUG(logger, "PU currencies are the same");
      std::vector<FareUsage*>::const_iterator fuIter = fareUsageVector.begin();
      const std::vector<FareUsage*>::const_iterator fuIterEnd = fareUsageVector.end();

      for (; fuIter != fuIterEnd; fuIter++)
      {
        if ((*fuIter)->paxTypeFare()->isDummyFare())
          continue;

        puCurrencies.push_back((*fuIter)->paxTypeFare()->currency());
        break;
      }
    }

    unsigned int currencyCount = 0;
    fareRelatedCurrencyChgsAreSame(fareUsageVector,
                                   cat12SurchargeAmount,
                                   stopOverSurchAmount,
                                   transfersSurchAmount,
                                   currencyCount,
                                   fareChgsSame);

    if (LIKELY(!isJLExempt(trx, *farePath)))
      currencyCount = 0;

    if (multiCurrencyPricing)
      multCurrencies += currencyCount;
  }

  if (multCurrencies)
  {
    LOG4CXX_DEBUG(logger, "Multiple currencies returning false");
    return false;
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Currencies in each PU are same - checking currencies across PU's");

    if (LIKELY(!puCurrencies.empty()))
    {
      // lint -e{578}
      const CurrencyCode& currency = puCurrencies[0];
      LOG4CXX_DEBUG(logger, "First PU currency " << currency);

      for (size_t i = 0; i < puCurrencies.size(); i++)
      {
        if (currency != puCurrencies[i])
        {
          LOG4CXX_DEBUG(logger, "PU# " << i);
          LOG4CXX_DEBUG(logger, "PU Currency " << puCurrencies[i]);
          LOG4CXX_DEBUG(logger, "farePathCurrencyCodesAreSame returning false");
          return false;
        }
      }
    }
    else
      LOG4CXX_DEBUG(logger, "PU Currency array is empty");
  }

  LOG4CXX_DEBUG(logger, "Leaving PricingUtil::farePathCurrencyCodesAreSame");
  LOG4CXX_DEBUG(logger, "farePathCurrencyCodesAreSame returning true");
  return checkRC;
}

//------------------------------------------------------------------------------
//
//   @method setFarePathBaseFareCurrency
//
//   Description: Sets the base fare on a per fare path basis
//
//   @param  FarePath      - farePath
//   @param  PricingTrx    - trx
//   @param  CurrencyCode  - originationCurrency - where international travel
//                           originates
//
//   @param  bool          - sameCurrencyCodes, true - all currency codes for this
//                           fare path are the same, else false
//
//   @return void
//------------------------------------------------------------------------------
void
PricingUtil::setFarePathBaseFareCurrency(FarePath* farePath,
                                         PricingTrx& trx,
                                         CurrencyCode& originationCurrency,
                                         bool sameCurrencyCodes)
{
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::setFarePathBaseFareCurrency");

  if (!farePath)
    return;

  if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX || trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
       trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
      !((const PricingTrx&)trx).getOptions()->baseFareCurrencyOverride().empty())
  {
    farePath->baseFareCurrency() =
        ((const PricingTrx&)trx).getOptions()->baseFareCurrencyOverride();
    return;
  }

  CurrencyCode pricingCurrency;
  const Itin* itin = farePath->itin();
  NationCode originNation = ItinUtil::originNation(*itin);

  if (sameCurrencyCodes)
  {
    const FareUsage* fareUsage = (farePath->pricingUnit().front())->fareUsage().front();

    if (trx.getRequest()->originBasedRTPricing())
    {
      std::vector<PricingUnit*>::const_iterator puIt = farePath->pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator puItEnd = farePath->pricingUnit().end();
      bool foundRealFare = false;

      for (; puIt != puItEnd; ++puIt)
      {
        const PricingUnit* pu = *puIt;
        std::vector<FareUsage*>::const_iterator fuIt = pu->fareUsage().begin();
        std::vector<FareUsage*>::const_iterator fuItEnd = pu->fareUsage().end();

        for (; fuIt != fuItEnd; ++fuIt)
        {
          const FareUsage* fu = *fuIt;

          if (fu->paxTypeFare()->isDummyFare())
          {
            continue;
          }
          else
          {
            fareUsage = fu;
            foundRealFare = true;
            break;
          }
        }

        if (foundRealFare)
        {
          break;
        }
      }
    }

    farePath->baseFareCurrency() = fareUsage->paxTypeFare()->currency();
    LOG4CXX_DEBUG(logger,
                  "ONLY ONE CURRENCY FOR THIS FARE PATH: " << fareUsage->paxTypeFare()->currency());

    if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX && farePath->baseFareCurrency() == NUC)
    {
      // happens if all PaxTypeFare are dummy fare,
      // and calcultation currency was NUC
      LOG4CXX_DEBUG(logger, "BASE FARE CURRENCY CAN NOT BE NUC - CHG TO ORIGIN CUR\n");
      farePath->baseFareCurrency() = originationCurrency;
    }
  }
  else
  {
    if (itin->geoTravelType() == GeoTravelType::International)
    {
      LOG4CXX_DEBUG(logger,
                    "SETTING BASE FARE CURRENCY TO ORIGINATION CURRENCY: " << originationCurrency);
      farePath->baseFareCurrency() = originationCurrency;
      LOG4CXX_DEBUG(logger, "FARE PATH CURRENCY: " << farePath->baseFareCurrency());
    }
    else
    {
      if (CurrencyUtil::getDomesticPricingCurrency(
              originNation, pricingCurrency, trx.ticketingDate()))
      {
        LOG4CXX_DEBUG(logger,
                      "SETTING BASE FARE CURRENCY TO PRICING CURRENCY: " << pricingCurrency);
        farePath->baseFareCurrency() = pricingCurrency;
      }
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving PricingUtil::setFarePathBaseFareCurrency");
}

//------------------------------------------------------------------------------
//
//   @method currencyRollBack
//
//   Description:  For Foreign Domestic itinerary( in future also might include
//                 Domestic) and if the AAA location allows multi-currency
//                 pricing we calculate everything in NUCs. Then if the final
//                 fare path is priced in only one currency we roll back the
//                 total nuc amounts on the PaxType fare to the original
//                 fare amounts.
//
//   @param  FarePath      - farePath
//   @param  PricingTrx    - trx
//   @param  MoneyAmount   - accumulated CAT 12 surcharge amount
//   @param  MoneyAmount   - accumulated stop overs surcharge amount
//   @param  MoneyAmount   - accumulated transfers surcharge amount
//
//   @return void
//------------------------------------------------------------------------------
void
PricingUtil::currencyRollBack(FarePath& farePath,
                              PricingTrx& trx,
                              MoneyAmount& totalCat12SurchAmt,
                              MoneyAmount& totalStopOversSurchAmt,
                              MoneyAmount& totalTransfersSurchAmt)
{
  LOG4CXX_DEBUG(logger, "ENTERED CURRENCY ROLL BACK");
  std::vector<PricingUnit*>& pu = farePath.pricingUnit();
  MoneyAmount totalNUCAmount = 0.0;
  MoneyAmount totalTaxAmount = 0.0;

  farePath.rollBackSurcharges();

  farePath.deactivateNonIATARounding();

  for (size_t i = 0; i < pu.size(); i++)
  {
    PricingUnit* priceableUnit = pu[i];
    totalTaxAmount += priceableUnit->taxAmount();
    std::vector<FareUsage*>& fareUsageVec = priceableUnit->fareUsage();

    for (size_t j = 0; j < fareUsageVec.size(); j++)
    {
      FareUsage* fareUsage = fareUsageVec[j];
      PaxTypeFare* paxFare = fareUsage->paxTypeFare();
      PaxTypeFare* newFare = paxFare->clone(trx.dataHandle(), false);
      Fare* fare = paxFare->fare()->clone(trx.dataHandle());

      fare->nucFareAmount() = getRollBackedNucAmount(trx, fareUsage, *priceableUnit,
                                                     farePath.itin()->useInternationalRounding());

      fare->rexSecondNucFareAmount() = fare->nucFareAmount();
      newFare->setFare(fare);
      totalNUCAmount += newFare->totalFareAmount();
      fareUsage->paxTypeFare() = newFare;

      if (fareUsage->paxTypeFare()->isNegotiated())
      {
        NegPaxTypeFareRuleData* negPaxTypeFare = fareUsage->paxTypeFare()->getNegRuleData();

        if (negPaxTypeFare && negPaxTypeFare->nucNetAmount() > 0)
        {
          negPaxTypeFare->nucNetAmount() = negPaxTypeFare->netAmount();
        }
      }

      LOG4CXX_DEBUG(logger, "TOTAL NUC AMOUNT B/F fare chgs roll back: " << totalNUCAmount);
      rollBackFareRelatedCharges(trx, fareUsage);
      LOG4CXX_DEBUG(logger, "TOTAL NUC AMOUNT A/F fare chgs roll back: " << totalNUCAmount);
    }
  }

  LOG4CXX_DEBUG(logger, "TOTAL NUC AMOUNT : " << totalNUCAmount);

  farePath.resetTotalNUCAmount(totalNUCAmount + totalCat12SurchAmt + totalStopOversSurchAmt +
                               totalTransfersSurchAmt + totalTaxAmount);

  if(farePath.getSpanishResidentUpperBoundDiscAmt() > 0)
    SRFEUtil::applyDomesticSpanishResidentDiscount(trx, farePath);
  else
  {
    if (!fallback::fixSpanishLargeFamilyForSRFE(&trx))
      SRFEUtil::applySpanishLargeFamilyDiscount(trx, farePath);
  }

  LOG4CXX_DEBUG(logger, "FAREPATH TOTAL NUC AMOUNT: " << farePath.getTotalNUCAmount());
  farePath.plusUpAmount() = totalCat12SurchAmt;
  LOG4CXX_DEBUG(logger, "FAREPATH TOTAL PLUS UP AMOUNT: " << farePath.plusUpAmount());
  LOG4CXX_DEBUG(logger, "LEAVING CURRENCY ROLL BACK");
}

//------------------------------------------------------------------------------
//
//   @method rollBackFareRelatedCharges
//
//   Description:  For Foreign Domestic itinerary we should only roll back
//                 fare related charges for multi-currency pricing.
//
//   @param  FareUsage      - fareUsage
//
//   @return void
//------------------------------------------------------------------------------
void
PricingUtil::rollBackFareRelatedCharges(const PricingTrx& trx, FareUsage* fareUsage)
{
  LOG4CXX_DEBUG(logger, "ENTERED MCP ROLL BACK OF FARE RELATED CHARGES");
  fareUsage->surchargeAmt() = 0.0;
  fareUsage->surchargeAmtUnconverted() = 0.0;
  fareUsage->stopOverAmt() = 0.0;
  fareUsage->stopOverAmtUnconverted() = 0.0;
  fareUsage->transferAmt() = 0.0;
  fareUsage->transferAmtUnconverted() = 0.0;

  std::vector<SurchargeData*>& surchargeData = fareUsage->surchargeData();
  SurchargeDataPtrVecIC surchargeDataIter = surchargeData.begin();

  for (; surchargeDataIter != surchargeData.end(); surchargeDataIter++)
  {
    SurchargeData* surcharge = (*surchargeDataIter);
    LOG4CXX_DEBUG(logger, "CAT 12 AMOUNT ROLLED BACK FROM : " << surcharge->amountNuc());
    LOG4CXX_DEBUG(logger, "CAT 12 AMOUNT ROLLED BACK TO : " << surcharge->amountSelected());
    surcharge->amountNuc() = surcharge->amountSelected();
    fareUsage->accumulateSurchargeAmt(surcharge->amountNuc());
    fareUsage->accumulateSurchargeAmtUnconverted(surcharge->amountSelected());
    LOG4CXX_DEBUG(logger, "CAT 12 AMOUNT : " << surcharge->amountNuc());
  }

  FareUsage::StopoverSurchargeMultiMap::iterator scIter;
  FareUsage::StopoverSurchargeMultiMap::iterator scIterEnd;
  scIter = fareUsage->stopoverSurcharges().begin();
  scIterEnd = fareUsage->stopoverSurcharges().end();

  for (; scIter != scIterEnd; ++scIter)
  {
    FareUsage::StopoverSurcharge* sos = const_cast<FareUsage::StopoverSurcharge*>((*scIter).second);
    LOG4CXX_DEBUG(logger, "CAT 8 AMOUNT ROLLED BACK FROM : " << sos->amount());
    LOG4CXX_DEBUG(logger, "CAT 8 AMOUNT ROLLED BACK TO : " << sos->unconvertedAmount());
    sos->amount() = sos->unconvertedAmount();
    fareUsage->accumulateStopOverAmt(sos->amount());
    fareUsage->accumulateStopOverAmtUnconverted(sos->unconvertedAmount());
    LOG4CXX_DEBUG(logger, "CAT 8 AMOUNT : " << sos->amount());
  }

  FareUsage::TransferSurchargeMultiMap::iterator transferSurchargeIter =
      fareUsage->transferSurcharges().begin();
  FareUsage::TransferSurchargeMultiMap::iterator transferSurchargeIterEnd =
      fareUsage->transferSurcharges().end();

  for (; transferSurchargeIter != transferSurchargeIterEnd; ++transferSurchargeIter)
  {
    FareUsage::TransferSurcharge* trs =
        const_cast<FareUsage::TransferSurcharge*>((*transferSurchargeIter).second);
    LOG4CXX_DEBUG(logger, "CAT 9 AMOUNT ROLLED BACK FROM : " << trs->amount());
    LOG4CXX_DEBUG(logger, "CAT 9 AMOUNT ROLLED BACK TO : " << trs->unconvertedAmount());
    trs->amount() = trs->unconvertedAmount();
    fareUsage->accumulateTransferAmt(trs->amount());
    fareUsage->accumulateTransferAmtUnconverted(trs->unconvertedAmount());
    LOG4CXX_DEBUG(logger, "CAT 9 : " << trs->amount());
  }

  LOG4CXX_DEBUG(logger, "LEAVING MCP ROLL BACK OF FARE RELATED CHARGES");
}
//------------------------------------------------------------------------------
//
//   @method setFarePathCalculationCurrency
//
//   Description: Determines the calculation currency for a given fare path
//
//   @param  FarePath      - farePath
//   @param  PricingTrx    - trx
//   @param  CurrencyCode  - originationCurrency - where international travel
//                           originates
//   @param  bool          - whether or not this is multi-currency pricing
//   @param  bool          - sameCurrencyCodes, true - all currency codes for this
//                           fare path are the same, else false
//
//   @return void
//------------------------------------------------------------------------------
void
PricingUtil::setFarePathCalculationCurrency(FarePath* farePath,
                                            PricingTrx& trx,
                                            CurrencyCode& originationCurrency,
                                            bool isMultiCurrencyPricing,
                                            bool sameCurrencyCodes)
{
  LOG4CXX_DEBUG(logger, "ENTERED SET FARE PATH CALC CUR");
  std::vector<PricingUnit*>& pu = farePath->pricingUnit();
  const std::vector<FareUsage*>& fareUsageVector = (pu.front())->fareUsage();
  const FareUsage* fareUsage = fareUsageVector.front();

  if (!farePath->itin()->calcCurrencyOverride().empty())
  {
    // ticketing requires this currency as calculation currency
    farePath->calculationCurrency() = farePath->itin()->calcCurrencyOverride();
  }
  else if (sameCurrencyCodes && (farePath->itin()->geoTravelType() == GeoTravelType::ForeignDomestic ||
                                 farePath->itin()->geoTravelType() == GeoTravelType::Domestic) &&
           (trx.fareCalcConfig()->domesticNUC() == NO))
  {
    LOG4CXX_DEBUG(
        logger,
        "SETTING CALC CUR FOR SCP FOREIGN DOM. TO: " << fareUsage->paxTypeFare()->currency());
    farePath->calculationCurrency() = fareUsage->paxTypeFare()->currency();
    if (trx.getRequest()->originBasedRTPricing() && fareUsage->paxTypeFare()->isDummyFare())
    {
      const std::vector<FareUsage*>& shoppingFareUsageVector = (pu.back())->fareUsage();
      const FareUsage* shoppingFareUsage = shoppingFareUsageVector.back();
      farePath->calculationCurrency() = shoppingFareUsage->paxTypeFare()->currency();
    }
  }
  else
  {
    farePath->calculationCurrency() = NUC;
    LOG4CXX_DEBUG(logger, "SETTING CALC CUR TO NUC ");
  }

  LOG4CXX_DEBUG(logger, "LEAVING SET FARE PATH CALC CUR");
}

//------------------------------------------------------------------------------
//
//   @method hasAnyCAT25SpecialFares
//
//   Description: Checks if there are any special cat 15 fares for all fare paths
//                These special fares are generated by fare by rule and need
//                to be handled like multi-currency pricing even though the
//                location might not be setup to handle mult-currency pricing.
//
//   @param  FarePath  - farePath
//
//   @return bool
//------------------------------------------------------------------------------
bool
PricingUtil::hasAnyCAT25SpecialFares(FarePath* farePath)
{
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::hasAnyCAT25SpecialFares");
  LOG4CXX_DEBUG(logger, "Number of PU's : " << farePath->pricingUnit().size());
  const std::vector<PricingUnit*>& pu = farePath->pricingUnit();

  for (size_t i = 0; i < pu.size(); i++)
  {
    PricingUnit* priceableUnit = const_cast<PricingUnit*>(pu[i]);
    const std::vector<FareUsage*>& fareUsageVec = priceableUnit->fareUsage();

    for (size_t j = 0; j < fareUsageVec.size(); j++)
    {
      FareUsage* fareUsage = fareUsageVec[j];
      PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();

      if (UNLIKELY(paxTypeFare->isSpecifiedCurFare()))
      {
        LOG4CXX_DEBUG(logger, "FOUND SPECIAL CAT 25 FARE");
        return true;
      }
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving PricingUtil::hasAnyCAT25SpecialFares");
  return false;
}

//------------------------------------------------------------------------------
//
//   @method fareRelatedCurrencyChgsAreSame
//
//   Description: Determines if all of the currency codes are the same for
//                cat 8,9 and 12
//
//   @param  fareUsageVector - vector of fare usages
//   @param  MoneyAmount     - cat 12 surcharge amount
//   @param  MoneyAmount     - stopover surcharge amount
//   @param  MoneyAmount     - transfers surcharge amount
//   @param  unsigned int    - return parameter
//
//   @return void
//------------------------------------------------------------------------------
void
PricingUtil::fareRelatedCurrencyChgsAreSame(const std::vector<FareUsage*>& fareUsageVector,
                                            MoneyAmount& cat12SurchargeAmount,
                                            MoneyAmount& stopOverSurchAmount,
                                            MoneyAmount& transfersSurchAmount,
                                            unsigned int& multCurrencies,
                                            bool& fareChgsSame)
{
  LOG4CXX_DEBUG(logger, "ENTERED fareRelatedCurrencyChgsAreSame");
  CurrencyConverter cc;
  const FareUsage* fUsage = fareUsageVector.front();
  const CurrencyCode& firstFareCurrency = fUsage->paxTypeFare()->currency();

  for (size_t j = 0; j < fareUsageVector.size(); j++)
  {
    FareUsage* fareUsage = fareUsageVector[j];
    std::vector<SurchargeData*>& surchargeData = fareUsage->surchargeData();
    SurchargeDataPtrVecIC surchargeDataIter = surchargeData.begin();

    for (; surchargeDataIter != surchargeData.end(); surchargeDataIter++)
    {
      SurchargeData* surcharge = (*surchargeDataIter);

      if (!surcharge->selectedTkt())
        continue;

      cat12SurchargeAmount += surcharge->amountSelected();
      LOG4CXX_DEBUG(logger, "CAT 12 AMOUNT : " << surcharge->amountSelected());
      LOG4CXX_DEBUG(logger, "CAT 12 AMOUNT : " << cat12SurchargeAmount);
      LOG4CXX_DEBUG(logger, "CAT 12 CURRENCY : " << surcharge->currSelected());

      if (surcharge->currSelected() != firstFareCurrency &&
          (!cc.isZeroAmount(surcharge->amountSelected())))
      {
        LOG4CXX_DEBUG(
            logger,
            "CAT 12 CURRENCY IS NOT THE SAME AS FARE CURRENCY: " << surcharge->currSelected());
        fareChgsSame &= false;
        multCurrencies++;
      }
    }

    FareUsage::StopoverSurchargeMultiMapCI scIter = fareUsage->stopoverSurcharges().begin();
    FareUsage::StopoverSurchargeMultiMapCI scIterEnd = fareUsage->stopoverSurcharges().end();

    for (; scIter != scIterEnd; ++scIter)
    {
      const FareUsage::StopoverSurcharge* sos = (*scIter).second;
      stopOverSurchAmount += sos->unconvertedAmount();
      LOG4CXX_DEBUG(logger, "STOPOVER  CURRENCY : " << sos->unconvertedCurrencyCode());

      if (sos->unconvertedCurrencyCode() != firstFareCurrency &&
          (!cc.isZeroAmount(sos->unconvertedAmount())))
      {
        LOG4CXX_DEBUG(logger, "STOPOVER  CURRENCY NOT SAME AS FARE CURRENCY: ");
        fareChgsSame &= false;
        multCurrencies++;
      }
    }

    FareUsage::TransferSurchargeMultiMapCI transferSurchargeIter =
        fareUsage->transferSurcharges().begin();
    FareUsage::TransferSurchargeMultiMapCI transferSurchargeIterEnd =
        fareUsage->transferSurcharges().end();

    for (; transferSurchargeIter != transferSurchargeIterEnd; ++transferSurchargeIter)
    {
      const FareUsage::TransferSurcharge* trs = (*transferSurchargeIter).second;
      transfersSurchAmount += trs->unconvertedAmount();
      LOG4CXX_DEBUG(logger, "TRANSFER  CURRENCY : " << trs->unconvertedCurrencyCode());

      if (trs->unconvertedCurrencyCode() != firstFareCurrency &&
          (!cc.isZeroAmount(trs->unconvertedAmount())))
      {
        LOG4CXX_DEBUG(logger, "TRANSFER  CURRENCY NOT SAME AS FARE CURRENCY: ");
        fareChgsSame &= false;
        multCurrencies++;
      }
    }
  }

  LOG4CXX_DEBUG(logger, "LEAVING fareRelatedCurrencyChgsAreSame");
}

bool
PricingUtil::processCat35Combination(PricingTrx& trx, FarePath& fPath)
{
  NegotiatedFareRuleUtil nfru;

  if (!nfru.processNegFareITBT(trx, fPath))
  {
    return false; // WPA entry may returns false --> farePath should be not used.
  }

  return true;
}

//------------------------------------------------------------------------------
//
// Description: Check if there is a Carrier Fare corresponding to this
// YY-Fare
//
//------------------------------------------------------------------------------
bool
PricingUtil::cxrFareTypeExists(const MergedFareMarket& mfm,
                               const PaxTypeFare& paxTypeFare,
                               const PricingUnit::Type puType,
                               const Directionality puDir,
                               const PaxType* paxType,
                               const std::vector<CarrierCode>& valCxrList)
{
   for(CarrierCode cxr: valCxrList)
   {
      if(cxrFareTypeExists(mfm, paxTypeFare, puType, puDir, paxType, cxr))
         return true;
   }
   return false;
}

bool
PricingUtil::cxrFareTypeExists(const MergedFareMarket& mfm,
                               const PaxTypeFare& paxTypeFare,
                               const PricingUnit::Type puType,
                               const Directionality puDir,
                               const PaxType* paxType,
                               const CarrierCode& valCxr)
{
  if (paxTypeFare.carrier() != INDUSTRY_CARRIER)
    return true;

  if (!mfm.cxrFareExist())
  {
    return false;
  }

  // we need to see if valid cxr-fare with same fareType,
  // directionality and fare with  proper ow-rt tag exists.
  MergedFareMarket::CxrFareTypeBitSet bitSet = getMergedFareMarketPuBitSet(puType, puDir);


  if (mfm.cxrFareTypeExists(paxType, paxTypeFare.fcaFareType(), bitSet, valCxr))
  {
    return true;
  }

  return false;
}

bool
PricingUtil::cxrFareTypeExists_old(const MergedFareMarket& mfm,
                               const PaxTypeFare& paxTypeFare,
                               const PricingUnit::Type puType,
                               const Directionality puDir,
                               const PaxType* paxType)
{
  if (paxTypeFare.carrier() != INDUSTRY_CARRIER)
    return true;

  if (!mfm.cxrFareExist())
    return false;

  // we need to see if valid cxr-fare with same fareType,
  // directionality and fare with  proper ow-rt tag exists.
  MergedFareMarket::CxrFareTypeBitSet bitSet = getMergedFareMarketPuBitSet(puType, puDir);

  if (mfm.cxrFareTypeExists_old(paxType, paxTypeFare.fcaFareType(), bitSet))
    return true;

  return false;
}

MergedFareMarket::CxrFareTypeBitSet
PricingUtil::getMergedFareMarketPuBitSet(const PricingUnit::Type puType,
                                         const Directionality puDir)
{
  MergedFareMarket::CxrFareTypeBitSet bitSet;

  if (puDir == FROM)
  {
    if (puType == PricingUnit::Type::ONEWAY)
    {
      bitSet.set(MergedFareMarket::OB_OW_TRIP_TYPE);
    }
    else
    {
      bitSet.set(MergedFareMarket::OB_OJRC_TRIP_TYPE);
    }
  }
  else if (LIKELY(puDir == TO))
  {
    if (puType == PricingUnit::Type::ONEWAY)
    {
      bitSet.set(MergedFareMarket::IB_OW_TRIP_TYPE);
    }
    else
    {
      bitSet.set(MergedFareMarket::IB_OJRC_TRIP_TYPE);
    }
  }

  return bitSet;
}

//------------------------------------------------------------------------------
//
//   @method hasNoFareConstructionCharges
//
//   Description: Checks to see if there are any differential, class of service,
//                or min fare construction charges.
//
//   @param  FarePath      - farePath
//   @param  PricingTrx    - trx
//
//   @return bool          - true - there are no fare construction charges, else false
//------------------------------------------------------------------------------
bool
PricingUtil::hasNoFareConstructionCharges(PricingTrx& trx, FarePath& farePath)
{
  LOG4CXX_DEBUG(logger, "Entered PricingUtil::hasNoFareConstructionCharges");
  CurrencyConverter ccConverter;
  // For COM and DMC across PU plus ups.
  const std::vector<FarePath::PlusUpInfo*>& fpPlusUps = farePath.plusUpInfoList();
  std::vector<FarePath::PlusUpInfo*>::const_iterator fpPlusUpIter = fpPlusUps.begin();

  for (; fpPlusUpIter != fpPlusUps.end(); fpPlusUpIter++)
  {
    if ((!ccConverter.isZeroAmount((*fpPlusUpIter)->minFarePlusUp()->plusUpAmount)))
    {
      LOG4CXX_DEBUG(
          logger,
          "COM OR DMC MIN FARE PLUS UP AMOUNT: " << (*fpPlusUpIter)->minFarePlusUp()->plusUpAmount);
      return false;
    }
  }

  // For OSC
  const std::vector<FarePath::OscPlusUp*>& oscPlusUps = farePath.oscPlusUp();
  std::vector<FarePath::OscPlusUp*>::const_iterator oscIter = oscPlusUps.begin();

  for (; oscIter != oscPlusUps.end(); oscIter++)
  {
    if ((!ccConverter.isZeroAmount((*oscIter)->plusUpAmount)))
    {
      LOG4CXX_DEBUG(logger, "OSC MIN FARE PLUS UP AMOUNT: " << (*oscIter)->plusUpAmount);
      return false;
    }
  }

  // For RSC
  const std::vector<FarePath::RscPlusUp*>& rscPlusUps = farePath.rscPlusUp();
  std::vector<FarePath::RscPlusUp*>::const_iterator rscIter = rscPlusUps.begin();

  for (; rscIter != rscPlusUps.end(); rscIter++)
  {
    if ((!ccConverter.isZeroAmount((*rscIter)->plusUpAmount)))
    {
      LOG4CXX_DEBUG(logger, "RSC MIN FARE PLUS UP AMOUNT: " << (*rscIter)->plusUpAmount);
      return false;
    }
  }

  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();

  for (; puIter != farePath.pricingUnit().end(); ++puIter)
  {
    const PricingUnit* pu = *puIter;
    std::vector<FareUsage*>::const_iterator fuIter = pu->fareUsage().begin();

    for (; fuIter != pu->fareUsage().end(); ++fuIter)
    {
      if ((!ccConverter.isZeroAmount((*fuIter)->differentialAmt())))
      {
        LOG4CXX_DEBUG(logger, "DIFFERENTIAL AMOUNT: " << (*fuIter)->differentialAmt());
        return false;
      }

      // Minimum Fare Plus up in Fare Usage
      const MinFarePlusUp& fuPlusUps = (*fuIter)->minFarePlusUp();
      MoneyAmount fuPlusUp = fuPlusUps.getSum(HIP);

      if ((!ccConverter.isZeroAmount(fuPlusUp)))
      {
        LOG4CXX_DEBUG(logger, "HIP MIN FARE AMOUNT: " << fuPlusUp);
        return false;
      }

      fuPlusUp = fuPlusUps.getSum(BHC);

      if (UNLIKELY((!ccConverter.isZeroAmount(fuPlusUp))))
      {
        LOG4CXX_DEBUG(logger, "BHC MIN FARE AMOUNT: " << fuPlusUp);
        return false;
      }

      fuPlusUp = fuPlusUps.getSum(COM);

      if (UNLIKELY((!ccConverter.isZeroAmount(fuPlusUp))))
      {
        LOG4CXX_DEBUG(logger, "COM MIN FARE AMOUNT: " << fuPlusUp);
        return false;
      }

      fuPlusUp = fuPlusUps.getSum(DMC);

      if (UNLIKELY((!ccConverter.isZeroAmount(fuPlusUp))))
      {
        LOG4CXX_DEBUG(logger, "DMC MIN FARE AMOUNT: " << fuPlusUp);
        return false;
      }
    }

    // Minimum Fare Plus up in PU
    const MinFarePlusUp& puPlusUps = pu->minFarePlusUp();
    MoneyAmount puPlusUp = puPlusUps.getSum(CTM);

    if ((!ccConverter.isZeroAmount(puPlusUp)))
    {
      LOG4CXX_DEBUG(logger, "CTM MIN FARE AMOUNT: " << puPlusUp);
      return false;
    }

    puPlusUp = puPlusUps.getSum(CPM);

    if ((!ccConverter.isZeroAmount(puPlusUp)))
    {
      LOG4CXX_DEBUG(logger, "CPM MIN FARE AMOUNT: " << puPlusUp);
      return false;
    }

    puPlusUp = puPlusUps.getSum(COP);

    if ((!ccConverter.isZeroAmount(puPlusUp)))
    {
      LOG4CXX_DEBUG(logger, "COP MIN FARE AMOUNT: " << puPlusUp);
      return false;
    }

    puPlusUp = puPlusUps.getSum(OJM);

    if ((!ccConverter.isZeroAmount(puPlusUp)))
    {
      LOG4CXX_DEBUG(logger, "OJM MIN FARE AMOUNT: " << puPlusUp);
      return false;
    }

    if (pu->hrtojNetPlusUp())
    {
      puPlusUp = pu->hrtojNetPlusUp()->plusUpAmount;

      if ((!ccConverter.isZeroAmount(puPlusUp)))
      {
        LOG4CXX_DEBUG(logger, "HRTOJ NET FARE AMOUNT: " << puPlusUp);
        return false;
      }
    }

    puPlusUp = puPlusUps.getSum(HRT);

    if ((!ccConverter.isZeroAmount(puPlusUp)))
    {
      LOG4CXX_DEBUG(logger, "HRT MIN FARE AMOUNT: " << puPlusUp);
      return false;
    }

    if (pu->hrtcNetPlusUp())
    {
      puPlusUp = pu->hrtcNetPlusUp()->plusUpAmount;

      if ((!ccConverter.isZeroAmount(puPlusUp)))
      {
        LOG4CXX_DEBUG(logger, "HIGHEST RT NET FARE AMOUNT: " << puPlusUp);
        return false;
      }
    }
  }

  // There are no fare construction charges
  //
  return true;
}

void
PricingUtil::collectEndorsements(PricingTrx& trx, FarePath& farePath)
{
  FarePathDownToFareUsage farePathDownToFareUsage(trx, farePath);
  std::for_each(
      farePath.pricingUnit().begin(), farePath.pricingUnit().end(), farePathDownToFareUsage);
}

//------------------------------------------------------------------------------
bool
PricingUtil::isFullCPTrx(const PricingTrx& trx)
{
  if (LIKELY(trx.getTrxType() != PricingTrx::PRICING_TRX))
    return false;

  if (trx.fxCnException())
    return false; // always choose the cheapest one for CnException

  Itin* const itin = trx.itin().front();

  if (!itin)
    return false;

  std::vector<TravelSeg*>::const_iterator tvlSegI = itin->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = itin->travelSeg().end();

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    if (((*tvlSegI)->segmentType() != Air) && ((*tvlSegI)->segmentType() != Open))
      continue;

    if ((*tvlSegI)->fareBasisCode().empty())
      return false;
  }

  return true;
}

//------------------------------------------------------------------------------
void
PricingUtil::checkTrxAborted(PricingTrx& trx,
                             const uint32_t& threshold,
                             const uint32_t& curCombCount,
                             bool& maxNbrCombMsgSet)
{
  TrxAborter* const aborter = trx.aborter();
  if (UNLIKELY(aborter == nullptr))
    return;

  const bool isShopping =
      ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.excTrxType() != PricingTrx::AR_EXC_TRX)) ||
      (trx.getTrxType() == PricingTrx::IS_TRX);

  if (UNLIKELY(!isShopping && (!maxNbrCombMsgSet && curCombCount > threshold)))
  {
    maxNbrCombMsgSet = true;
    aborter->setErrorCode(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
    aborter->setErrorMsg("MAX NBR COMBINATIONS EXCEEDED/USE SEGMENT SELECT");
  }

  if (fallback::reworkTrxAborter(&trx))
    tse::checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  const uint32_t memCheckTrxInterval = TrxUtil::getMemCheckTrxInterval(trx);
  const uint32_t memCheckInterval = TrxUtil::getMemCheckInterval(trx);

  TrxUtil::checkTrxMemoryAborted(trx, curCombCount, memCheckInterval, memCheckTrxInterval);
}

//------------------------------------------------------------------------------

void
PricingUtil::determineCat27TourCode(FarePath& farePath)
{
  const std::vector<PricingUnit*>& pricingUnits = farePath.pricingUnit();
  std::vector<PricingUnit*>::const_iterator iterPU = pricingUnits.begin();
  std::vector<PricingUnit*>::const_iterator iterPUEnd = pricingUnits.end();

  while (iterPU != iterPUEnd)
  {
    const std::vector<FareUsage*>& fareUsage = (*iterPU)->fareUsage();
    std::vector<FareUsage*>::const_iterator iterFUEnd = fareUsage.end();
    std::vector<FareUsage*>::const_iterator iterFU = fareUsage.begin();

    while (iterFU != iterFUEnd)
    {
      std::string tourCode;
      RuleUtil::getCat27TourCode((*iterFU)->paxTypeFare(), tourCode);

      if (!tourCode.empty())
      {
        farePath.cat27TourCode() = tourCode;
        return;
      }

      ++iterFU;
    }

    ++iterPU;
  }
}

void
PricingUtil::discountOrPlusUpPricing(PricingTrx& trx, FarePath& farePath)
{
  if (TrxUtil::newDiscountLogic(trx))
  {
    const PricingRequest& request = *trx.getRequest();

    if (LIKELY(!request.isDAorPAentry() && !request.isDPorPPentry()))
      return;

    DiscountPricing discountPricing(trx, farePath);
    discountPricing.process();
  }
  else
  {
    DiscountPricing discountPricing(trx, farePath);
    discountPricing.processOld();
  }
}

//------------------------------------------------------------------------------
bool
PricingUtil::finalPricingProcess(PricingTrx& trx, FarePath& farePath)
{
  if (farePath.itin()->isPlusUpPricing())
  {
    PricingUtil::processConsolidatorPlusUp(trx, farePath);
  }

  PricingUtil::determineBaseFare(&farePath, trx, farePath.itin());
  for (FarePath* const fp : farePath.gsaClonedFarePaths())
    PricingUtil::determineBaseFare(fp, trx, fp->itin());

  PricingUtil::calculateFarePathISICode(trx, farePath);

  PricingUtil::discountOrPlusUpPricing(trx, farePath);

  for (FarePath* const fp : farePath.gsaClonedFarePaths())
    PricingUtil::discountOrPlusUpPricing(trx, *fp);

  // cat27 Tour Code
  determineCat27TourCode(farePath);

  // Collect endorsements for Cat18
  if (LIKELY(!TrxUtil::optimusNetRemitEnabled(trx) || !farePath.endorsementsCollected()))
    PricingUtil::collectEndorsements(trx, farePath);

  // Check for Net Remit
  // Check for Cat35 IT/BT
  bool ret = PricingUtil::processCat35Combination(trx, farePath);
  if (LIKELY(ret))
  {
    NegotiatedFareRuleUtil nfru;
    const PricingRequest* priReqP = trx.getRequest();
    bool tfsfForDirTktgOrPricing =
        TrxUtil::isCat35TFSFEnabled(trx) && priReqP &&
        ((!priReqP->isTicketEntry()) // pricing
         ||
         (!priReqP->isFormOfPaymentCard() && // direct tktng with cash fop
          !trx.getOptions()->isCat35Sell()));

    if (fallback::fallbackFRRFixNonItBtDirectTicketing(&trx))
    {
      if ((tfsfForDirTktgOrPricing) && nfru.isNetTicketingWithItBtData(trx, farePath))
      {
        processNetFarePath(trx, farePath);
      }
      else if (tfsfForDirTktgOrPricing && nfru.isRegularNet(farePath))
      {
        bool isRegularNet = true;
        bool processNetFarePathForNonItBtDirectTicketing =
          trx.getRequest()->isTicketEntry();

        if (!TrxUtil::isExchangeOrTicketing(trx) || processNetFarePathForNonItBtDirectTicketing)
          processNetFarePath(trx, farePath, isRegularNet);
      }
      else if (LIKELY(!trx.getRequest()->ticketingAgent()->axessUser()))
      {
        NetRemitPricing netRemitPricing(trx, farePath);
        ret = netRemitPricing.process();
      }
    }
    else
    {
      bool isDirectTicketing = trx.getRequest()->isTicketEntry();
      bool isRegularNet = true;
      if ((tfsfForDirTktgOrPricing) && nfru.isNetTicketingWithItBtData(trx, farePath))
      {
        processNetFarePath(trx, farePath);
      }

      else if (!fallback::fallbackTagPY9matchITBTCCPayment(&trx) &&
               nfru.isNetTicketingWithItBtData(trx, farePath) &&
               priReqP->isFormOfPaymentCard() && isDirectTicketing)
      {
        processNetFarePath(trx, farePath, isRegularNet);
      }
      else if (nfru.isRegularNet(farePath))
      {
        if (!TrxUtil::isExchangeOrTicketing(trx) || isDirectTicketing)
          processNetFarePath(trx, farePath, isRegularNet);
      }
      else if (LIKELY(!trx.getRequest()->ticketingAgent()->axessUser()))
      {
        NetRemitPricing netRemitPricing(trx, farePath);
        ret = netRemitPricing.process();
      }
    }
  }

  bool processFRRForDirectTicketing = trx.getRequest()->isTicketEntry();
  if (!TrxUtil::isExchangeOrTicketing(trx) || processFRRForDirectTicketing)
  {
    bool isMslAdjusted = trx.getOptions()->isMslRequest();

    if (isMslAdjusted || adjustedSellingCalcDataExists(farePath))
      processAdjustedSellingFarePath(trx, farePath, isMslAdjusted);

    for (FarePath* gsaFarePath : farePath.gsaClonedFarePaths())
    {
      if (gsaFarePath && (isMslAdjusted || adjustedSellingCalcDataExists(*gsaFarePath)))
        processAdjustedSellingFarePath(trx, *gsaFarePath, isMslAdjusted);
    }
  }

  if ((ItinUtil::isDomesticPeru(&trx, farePath.itin())) && (farePath.calculationCurrency() != NUC))
    ret = roundPeruTotalAmount(trx, farePath);

  if (farePath.netRemitFarePath() != nullptr)
    farePath.clearGsaClonedFarePaths();

  return ret;
}

bool
PricingUtil::adjustedSellingCalcDataExists(const FarePath& farePath)
{
  for (PricingUnit* pu : farePath.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      if (fu->paxTypeFare()->getAdjustedSellingCalcData())
        return true;

  return false;
}

bool
PricingUtil::getManualAdjustmentAmountsPerFUHelper(const std::vector<MoneyAmount>& perFuAmount,
                                                   MoneyAmount adjustmentAmount,
                                                   std::vector<MoneyAmount>& perFuAdjustmentAmts)
{
  size_t fuSize = perFuAmount.size();
  if (!fuSize) // nothing to do. Return true to prevent exception
    return true;

  perFuAdjustmentAmts.assign(fuSize, 0.0);

  if (std::abs(adjustmentAmount) < EPSILON)
    return true;

  // if manual adjustment amount is positive, or first FU absorbs negative amount, just add it to
  // the first FU
  if ((perFuAmount.front() + adjustmentAmount) > -EPSILON)
  {
    perFuAdjustmentAmts.front() = adjustmentAmount;
    return true;
  }

  // Manual adjustment amount is negative, and bigger then first one. See if total or max fu
  // can absorb it
  MoneyAmount total = 0.0;
  for (size_t i = 0; i < fuSize; ++i)
  {
    MoneyAmount fuAmount = perFuAmount[i];

    if (perFuAmount[i] + adjustmentAmount > -EPSILON)
    {
      perFuAdjustmentAmts[i] = adjustmentAmount;
      return true;
    }

    total += fuAmount;
  }

  if ((total + adjustmentAmount) < -EPSILON)
    return false;

  // None of the individual FUs could absorb it. Distribute it among FUs
  for (size_t i = 0; i < fuSize; ++i)
  {
    MoneyAmount fuAmount = perFuAmount[i];
    if ((fuAmount + adjustmentAmount) > -EPSILON)
    {
      perFuAdjustmentAmts[i] = adjustmentAmount;
      return true;
    }

    perFuAdjustmentAmts[i] = -fuAmount;
    adjustmentAmount += fuAmount;
  }

  return true;
}

// Takes care of negative amounts
MoneyAmount
PricingUtil::convertCurrencyForMsl(PricingTrx& trx,
                                   MoneyAmount amount,
                                   const CurrencyCode& targetCurrency,
                                   const CurrencyCode& sourceCurrency)
{
  if (targetCurrency == sourceCurrency)
    return amount;

  bool isNegative = amount < 0.0;
  if (isNegative)
    amount = -amount;

  CurrencyConversionFacade ccFacade;
  Money targetValue(targetCurrency);
  Money sourceValue(amount, sourceCurrency);
  if (ccFacade.convert(targetValue, sourceValue, trx, 0, CurrencyConversionRequest::NO_ROUNDING))
    amount = targetValue.value();

  if (isNegative)
    amount = -amount;

  return amount;
}

CurrencyCode
PricingUtil::getPaymentCurrency(PricingTrx& trx)
{
  CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!trx.getOptions()->currencyOverride().empty())
    paymentCurrency = trx.getOptions()->currencyOverride();

  return paymentCurrency;
}

MoneyAmount
PricingUtil::getManualAdjustmentAmount(PricingTrx& trx, FarePath& farePath)
{
  MoneyAmount adjustmentAmount = farePath.paxType()->mslAmount();

  CurrencyCode paymentCurrency = getPaymentCurrency(trx);
  CurrencyCode farePathCurrency = farePath.calculationCurrency();

  if (farePathCurrency == paymentCurrency)
    return adjustmentAmount;

  return convertCurrencyForMsl(trx, adjustmentAmount, farePathCurrency, paymentCurrency);
}

void
PricingUtil::updateAslMslDifference(PricingTrx& trx, FarePath& farePath)
{
  if (farePath.aslMslDiffAmount() == 0.0)
    return;

  CurrencyCode paymentCurrency = getPaymentCurrency(trx);
  CurrencyCode farePathCurrency = farePath.calculationCurrency();

  if (farePathCurrency == paymentCurrency)
    return;

  farePath.aslMslDiffAmount() = convertCurrencyForMsl(trx,
                                                      farePath.aslMslDiffAmount(),
                                                      paymentCurrency,
                                                      farePathCurrency);
}

void
PricingUtil::getManualAdjustmentAmountsPerFU(PricingTrx& trx,
                                             FarePath& farePath,
                                             std::vector<MoneyAmount>& perFuAdjustmentAmts)
{
  MoneyAmount adjustmentAmount = getManualAdjustmentAmount(trx, farePath);

  std::vector<MoneyAmount> perFuAmount;
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      // if calculation currrency is NUC, ptf->nucFareAmounts are *really* NUC
      // Otherwise, they are in fare currency (same as ptf->fareAmount) though named as "nucFare*"
      MoneyAmount fuAmount = fu->paxTypeFare()->nucFareAmount();
      if (farePath.calculationCurrency() != NUC)
        fuAmount = getFuAmountInCalculationCurrency(trx, farePath, *fu->paxTypeFare(), fuAmount);

      perFuAmount.push_back(fuAmount);
    }
  }

  if (!getManualAdjustmentAmountsPerFUHelper(perFuAmount, adjustmentAmount, perFuAdjustmentAmts))
    throw ErrorResponseException(
      ErrorResponseException::ADJUSTED_AMOUNT_EXCEEDS_BASE_FARE_WITHOUT_SURCHARGES);
}

MoneyAmount
PricingUtil::getFuAmountInCalculationCurrency(PricingTrx& trx,
                                              const FarePath& farePath,
                                              const PaxTypeFare& ptf,
                                              MoneyAmount amountInPtfCurrency)
{
  if (farePath.calculationCurrency() == ptf.currency())
    return amountInPtfCurrency;

  return convertCurrencyForMsl(trx,
                               amountInPtfCurrency,
                               farePath.calculationCurrency(),
                               ptf.currency());
}

MoneyAmount
PricingUtil::getAdjustmentAmountInFareCurrency(PricingTrx& trx,
                                               const FarePath& farePath,
                                               const PaxTypeFare& ptf,
                                               MoneyAmount amountInCalculationCurrency)
{
  CurrencyCode farePathCurrency = farePath.calculationCurrency();

  if (farePathCurrency == ptf.currency())
    return amountInCalculationCurrency;

  return convertCurrencyForMsl(trx,
                               amountInCalculationCurrency,
                               ptf.currency(),
                               farePathCurrency);
}

MoneyAmount
PricingUtil::getAslMileageDiff(bool isMslAdjusted, PaxTypeFare& newPtf, FarePath& newFarePath)
{
  if (!((newPtf.mileageSurchargeAmt() > 0) && (newPtf.mileageSurchargePctg() > 0)))
    return 0.0;

  MoneyAmount newMileageSurcharge = (newPtf.nucFareAmount() * newPtf.mileageSurchargePctg())/100.0;
  CurrencyUtil::truncateNUCAmount(newMileageSurcharge);

  MoneyAmount mileageDiff = newMileageSurcharge - newPtf.mileageSurchargeAmt();

  if (isMslAdjusted)
  {
    newFarePath.aslMslDiffAmount() += mileageDiff;
    return 0.0; // MSL does not have mileage recalculation
  }

  newPtf.mileageSurchargeAmt() = newMileageSurcharge;
  return mileageDiff;
}

FarePath*
PricingUtil::cloneFarePathForAdjusted(PricingTrx& trx, FarePath& farePath)
{
  FarePathCopier farePathCopier(trx.dataHandle());
  FarePath* newFarePath = farePathCopier.getDuplicate(farePath);

  if (!fallback::fallbackFixMslNetFareAmount(&trx) && farePath.collectedNegFareData())
  {
    CollectedNegFareData* cNegFareData;
    trx.dataHandle().get(cNegFareData);

    *cNegFareData = *farePath.collectedNegFareData();
    newFarePath->collectedNegFareData() = cNegFareData;
  }

  return newFarePath;
}

void
PricingUtil::processAdjustedSellingFarePath(PricingTrx& trx,
                                            FarePath& farePath,
                                            bool isMslAdjusted)

{
  std::vector<MoneyAmount> fuAdjAmtVec;
  if (isMslAdjusted)
    getManualAdjustmentAmountsPerFU(trx, farePath, fuAdjAmtVec);

  FarePath* newFarePath = cloneFarePathForAdjusted(trx, farePath);

  int fuIndex = 0;
  for (PricingUnit* pu : newFarePath->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      bool fuAslDataExists = fu->paxTypeFare()->getAdjustedSellingCalcData() != nullptr;
      MoneyAmount mslAdjustedAmount = isMslAdjusted ? fuAdjAmtVec[fuIndex++] : 0.0;

      if (!fuAslDataExists && (!isMslAdjusted || (mslAdjustedAmount == 0.0)))
        continue;

      PaxTypeFare& origPtf = *fu->paxTypeFare();
      PaxTypeFare* newPTFare = origPtf.clone(trx.dataHandle());
      Fare* newFare = origPtf.fare()->clone(trx.dataHandle());
      FareInfo* newFareInfo = origPtf.fare()->fareInfo()->clone(trx.dataHandle());
      newFare->setFareInfo(newFareInfo);
      newPTFare->setFare(newFare);
      fu->paxTypeFare() = newPTFare;

      MoneyAmount aslAdjustedAmount = 0.0;
      bool isNUC = (farePath.calculationCurrency() == NUC);

      if (fuAslDataExists)
      {
        if (isNUC)
          aslAdjustedAmount = origPtf.getAdjustedSellingCalcData()->getCalculatedNucAmt();
        else
          aslAdjustedAmount = origPtf.getAdjustedSellingCalcData()->getCalculatedAmt();
      }

      if (isMslAdjusted)
      {
        MoneyAmount mslAdjustmentAmtInFareCurrency =
          getAdjustmentAmountInFareCurrency(trx, farePath, origPtf, mslAdjustedAmount);

        if (isNUC)
          newPTFare->nucFareAmount() = origPtf.nucFareAmount() + mslAdjustedAmount;
        else
          newPTFare->nucFareAmount() = origPtf.nucFareAmount() + mslAdjustmentAmtInFareCurrency;

        if (fuAslDataExists)
          newFarePath->aslMslDiffAmount() += newPTFare->nucFareAmount() - aslAdjustedAmount;

        newFareInfo->fareAmount() = origPtf.fareAmount() + mslAdjustmentAmtInFareCurrency;
      }
      else // only ASL
      {
        newPTFare->nucFareAmount() = aslAdjustedAmount;
        newFareInfo->fareAmount() = origPtf.getAdjustedSellingCalcData()->getCalculatedAmt();
      }

      newPTFare->nucOriginalFareAmount() = newPTFare->isRoundTrip() ?
                                             newPTFare->nucFareAmount() * 2.0 :
                                             newPTFare->nucFareAmount();

      newFareInfo->originalFareAmount() = newPTFare->isRoundTrip() ?
                                            newPTFare->fareAmount() * 2.0 :
                                            newPTFare->fareAmount();

      MoneyAmount mileageDiff =
        fuAslDataExists ? getAslMileageDiff(isMslAdjusted, *newPTFare, *newFarePath) : 0.0;

      MoneyAmount diff = newPTFare->nucFareAmount() - origPtf.nucFareAmount() + mileageDiff;
      pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() + diff);
      newFarePath->increaseTotalNUCAmount(diff);

      if(!fallback::markupAnyFareOptimization(&trx))
      {
        farePath.setTotalNUCMarkupAmount(farePath.getTotalNUCMarkupAmount() + diff);
      }
    }
  }

  if (isMslAdjusted)
    updateAslMslDifference(trx, *newFarePath);

  PricingUtil::determineBaseFare(newFarePath, trx, farePath.itin());
  for (FarePath* const fp : newFarePath->gsaClonedFarePaths())
    PricingUtil::determineBaseFare(fp, trx, fp->itin());

  newFarePath->setAdjustedSellingFarePath();
  farePath.adjustedSellingFarePath() = newFarePath;

  displayAdjustedSellingFarePath(trx, farePath);
}

void
PricingUtil::displayAdjustedSellingFarePath(PricingTrx& trx, const FarePath& fp)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic693)
    return;

  if (!fp.adjustedSellingFarePath())
    return;

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diag = factory->create(trx);

  diag->enable(Diagnostic693);

  *diag << " \n******** ADJUSTED SELLING LEVEL FARE PATH - ANALYSIS ********\n";
  *diag << "PRICING FARE PATH ------------------------------------------\n";
  *diag << fp;

  *diag << "ADJUSTED SELLING LEVEL FARE PATH----------------------------\n";
  *diag << *fp.adjustedSellingFarePath();

  diag->flushMsg();

  diag->disable(Diagnostic693);
}

//------------------------------------------------------------------------------
bool
PricingUtil::finalFPathCreationForAxess(PricingTrx& trx, FarePath& farePath, FarePath& fPathAxess)
{
  if (fPathAxess.intlSurfaceTvlLimit())
    return false;

  determineBaseFare(&fPathAxess, trx, fPathAxess.itin());
  calculateFarePathISICode(trx, fPathAxess);

  PricingUtil::discountOrPlusUpPricing(trx, fPathAxess);

  // Collect endorsements for Cat18
  collectEndorsements(trx, fPathAxess);

  // Save REF's for Axess_FarePath and Original_FarePath
  farePath.axessFarePath() = &fPathAxess;
  fPathAxess.originalFarePathAxess() = &farePath;
  NetRemitPricing netRemitPricing(trx, fPathAxess);
  netRemitPricing.displayFarePathAxess(fPathAxess);

  if ((ItinUtil::isDomesticPeru(&trx, farePath.itin())) &&
      (fPathAxess.calculationCurrency() != NUC))
    return roundPeruTotalAmount(trx, fPathAxess);

  return true;
}

//------------------------------------------------------------------------------
bool
PricingUtil::roundPeruTotalAmount(PricingTrx& trx, FarePath& farePath)
{
  const NUCInfo* nucInfo = nullptr;
  CurrencyCode baseFareCurrency(farePath.baseFareCurrency());
  Money baseFare(farePath.getTotalNUCAmount(), farePath.baseFareCurrency());
  farePath.unroundedTotalNUCAmount() = farePath.getTotalNUCAmount();
  CarrierCode carrier;

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    RexBaseTrx& rexTrx = static_cast<RexBaseTrx&>(trx);
    DataHandle dataHandle(rexTrx.originalTktIssueDT());
    nucInfo = dataHandle.getNUCFirst(baseFareCurrency, carrier, rexTrx.originalTktIssueDT());
  }
  else
    nucInfo = trx.dataHandle().getNUCFirst(baseFareCurrency, carrier, trx.ticketingDate());

  if (!nucInfo)
  {
    LOG4CXX_ERROR(logger, "NO NUC INFORMATION FOR CURRENCY: " << farePath.baseFareCurrency());
    return false;
  }

  if (farePath.applyNonIATARounding(trx))
    baseFare.setApplyNonIATARounding();

  CurrencyConverter cc;
  RoundingFactor roundingFactor = nucInfo->_roundingFactor;
  RoundingRule roundingRule = nucInfo->_roundingRule;
  bool roundRC = cc.round(baseFare, roundingFactor, roundingRule);

  if (!roundRC)
  {
    LOG4CXX_ERROR(logger, "Rounding failed = NUC to local currency ");
    return false;
  }

  farePath.setTotalNUCAmount(baseFare.value());
  return true;
}

//------------------------------------------------------------------------------
// Description: This method returns the proper PaxTypeFare pointer based upon
//              Category 25 Category Override Tag rules.  If the PaxTypeFare
//              in question is not a FareByRule, the regular
//              PaxTypeFare object will be returned.  If the PaxTypeFare is
//              a Calculated FareByRule and the Category 10 override
//              tag is set to 'B' the base Fare of the FBR PaxTypeFare will
//              be returned.
//------------------------------------------------------------------------------
//
const PaxTypeFare*
PricingUtil::determinePaxTypeFare(const PaxTypeFare* ptFare)
{
  if (ptFare->isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPtfRd = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (UNLIKELY(!fbrPtfRd))
    {
      LOG4CXX_FATAL(logger, "INVALID PaxTypeFare");
      throw ErrorResponseException(ErrorResponseException::NO_ERROR, "Invalid PaxTypeFare");
    }
    else
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPtfRd->ruleItemInfo());

      if (UNLIKELY(!fbrItemInfo))
      {
        LOG4CXX_FATAL(logger, "INVALID PaxTypeFare");
        throw ErrorResponseException(ErrorResponseException::NO_ERROR, "Invalid PaxTypeFare");
      }
      else
      {
        bool fbrCalcFare = !fbrPtfRd->isSpecifiedFare();
        const Indicator& cat10Ind = fbrItemInfo->ovrdcat10();

        if (fbrCalcFare && cat10Ind == 'B')
        {
          return fbrPtfRd->baseFare();
        }
      }
    }
  }

  return ptFare;
}

//------------------------------------------------------------------------------
// Description: This method returns the boolean as result for the Divide party
//              test. This test will be done for the WPNC family entries, if
//              more than one pax type in the entry and for WP type entries.
//------------------------------------------------------------------------------
//
bool
PricingUtil::isDivideParty(const PricingTrx& trx, std::vector<FarePath*>& groupFP)
{
  for (std::vector<FarePath*>::const_iterator i = groupFP.begin(), iend = groupFP.end(); i != iend;
       ++i)
  {
    const Itin* itin = (*i)->itin();

    if (UNLIKELY(itin == nullptr))
      return false;

    if (LIKELY(!(*i)->bookingCodeRebook.empty()))
      continue;

    (*i)->bookingCodeRebook.resize(itin->travelSeg().size());

    for (std::vector<PricingUnit*>::const_iterator ip = (*i)->pricingUnit().begin(),
                                                   ipend = (*i)->pricingUnit().end();
         ip != ipend;
         ++ip)
    {
      for (std::vector<FareUsage*>::const_iterator ipf = (*ip)->fareUsage().begin(),
                                                   ipfend = (*ip)->fareUsage().end();
           ipf != ipfend;
           ++ipf)
      {
        (*i)->collectBookingCode(trx, *ipf);
      }
    }
  }

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(trx.paxType().begin(),
                                                         trx.paxType().end());
  bool firstPaxType = true;
  FarePath* firstFP = nullptr;
  //    FarePath* fpTest = 0;

  for (std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = inOrderPaxType.begin(),
                                                               ptend = inOrderPaxType.end();
       pti != ptend;
       ++pti)
  {
    if (UNLIKELY(trx.altTrxType() != PricingTrx::WP))
    {
      firstPaxType = true;
      firstFP = nullptr;
    }

    FarePath* fp = nullptr;

    for (std::vector<FarePath*>::const_iterator i = groupFP.begin(), iend = groupFP.end();
         i != iend;
         ++i)
    {
      fp = *i;

      if (fp->paxType() != *pti || !fp->processed() || fp->bookingCodeRebook.empty())
      {
        continue;
      }

      if (firstPaxType)
      {
        firstFP = *i;
        firstPaxType = false;
      }

      fp = *i;

      if (UNLIKELY(!fp))
        continue;

      if (UNLIKELY(!fp->processed()))
        continue;

      for (std::vector<FarePath*>::const_iterator i = groupFP.begin(), iend = groupFP.end();
           i != iend && trx.altTrxType() == PricingTrx::WP;
           ++i)
      {
        //                fpTest = *i;
        if (fp != firstFP && !fp->bookingCodeRebook.empty() &&
            fp->bookingCodeRebook != (*i)->bookingCodeRebook)
        {
          return true;
          break;
        }
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
// Description: This method does special processing for Consolidator Plus Up
//              Pricing
//------------------------------------------------------------------------------
void
PricingUtil::processConsolidatorPlusUp(PricingTrx& trx, FarePath& farePath)
{
  DiagManager diagManager(trx, Diagnostic864);
  DiagCollector* diag = diagManager.isActive() ? &diagManager.collector() : nullptr;
  farePath.itin()->consolidatorPlusUp()->qualifyFarePath(trx, farePath, diag);

  if (diag)
    diag->flushMsg();
}

//------------------------------------------------------------------------------
// Description: This method returns the fare amount in local currency.
// When the round trip amount in local currency ends in an odd number,
// round down the outbound fare component and round up the inbound fare
// component in fare calculation line.
//------------------------------------------------------------------------------
MoneyAmount
PricingUtil::getRollBackedNucAmount(PricingTrx& trx,
                                    FareUsage* fareUsage,
                                    PricingUnit& priceableUnit,
                                    bool useIntlRounding)
{
  PaxTypeFare* ptFare = fareUsage->paxTypeFare();
  MoneyAmount originalFareAmount = ptFare->fare()->originalFareAmount();

  if (ptFare->isDiscounted() || ptFare->isFareByRule() || ptFare->isNegotiated())
  {
    CurrencyRoundingUtil curRoundingUtil;
    Money target(originalFareAmount, ptFare->currency());

    if (curRoundingUtil.round(target, trx, useIntlRounding))
    {
      originalFareAmount = target.value();
    }
  }

  bool isRT = false;

  if (ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    isRT = true;

  if (!isRT)
  {
    return originalFareAmount;
  }

  if (!isMirrorImage(priceableUnit))
  {
    return (originalFareAmount / 2);
  }

  MoneyAmount hrtAmt = originalFareAmount / 2;
  CurrencyUtil::truncateNonNUCAmount(hrtAmt, ptFare->fare()->fareInfo()->noDec());

  if (fareUsage->isOutbound())
  {
    return hrtAmt;
  }

  MoneyAmount rtAmt = hrtAmt * 2;

  if (originalFareAmount - rtAmt > EPSILON)
  {
    return (originalFareAmount - hrtAmt);
  }

  return hrtAmt;
}

bool
PricingUtil::isJLExempt(PricingTrx& trx, const FarePath& farePath)
{
  return (trx.getRequest()->ticketingAgent()->axessUser() && trx.getOptions()->forceCorpFares() &&
          isJLExemptAccntCode(trx) && isJLExemptTktDesig(trx, farePath));
}

bool
PricingUtil::isJLExemptAccntCode(PricingTrx& trx)
{
  // check if input account code is 'RX78MS06TM'
  if (!trx.getRequest()->isMultiAccCorpId())
  {
    if (trx.getRequest()->accountCode().empty())
      return false;

    if (trx.getRequest()->accountCode() == JLACCNTCODE)
      return true;
  }

  if (trx.getRequest()->isMultiAccCorpId())
  {
    const std::vector<std::string>& accCodeVec = trx.getRequest()->accCodeVec();

    if (accCodeVec.empty())
      return false;

    std::vector<std::string>::const_iterator vecIter;

    for (vecIter = accCodeVec.begin(); vecIter != accCodeVec.end(); vecIter++)
    {
      if ((*vecIter) == JLACCNTCODE)
        return true;
    }
  }

  return false;
}

bool
PricingUtil::isJLExemptTktDesig(PricingTrx& trx, const FarePath& farePath)
{
  if (!trx.getRequest()->isTktDesignatorEntry())
    return false;

  // check input ticket designators if any is "JMBJL"
  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  for (; puIt != puItEnd; puIt++)
  {
    std::vector<tse::TravelSeg*>::const_iterator iTs = (*puIt)->travelSeg().begin();
    std::vector<tse::TravelSeg*>::const_iterator iTsEnd = (*puIt)->travelSeg().end();

    for (; iTs != iTsEnd; iTs++)
    {
      int16_t segOrder = (*iTs)->segmentOrder();
      TktDesignator tktD = trx.getRequest()->tktDesignator(segOrder);

      if (!tktD.empty() && tktD == JLTKTDESGN)
        return true;
    }
  }

  return false;
}

// Allows HRTC only for SMF.
// We do not need this check when both ATPCO and SITA supports HRTC
bool
PricingUtil::allowHRTCForVendor(const PricingTrx& trx, const PaxTypeFare* fare)
{
  if (trx.dataHandle().getVendorType(fare->vendor()) == SMF_VENDOR)
    return true;

  return false;
}

// returns true only if all FU are allowed for HRTC
bool
PricingUtil::allowHRTCForVendor(const PricingTrx& trx, const PricingUnit& pu)
{
  for (std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
       fuIt != pu.fareUsage().end();
       ++fuIt)
  {
    if (!allowHRTCForVendor(trx, (*fuIt)->paxTypeFare()))
      return false;
  }

  return true;
}

//@brief: Check whether two segment/FC are mirror image of each other.
// We use this logic to decide whether to apply truncate logic(truncate+originalAmt-hrt)
bool
PricingUtil::isMirrorImage(const PricingUnit& pu)
{
  if (pu.puType() != PricingUnit::Type::ROUNDTRIP || pu.fareUsage().size() != 2)
    return false;

  const std::vector<FareUsage*>& fuVec = pu.fareUsage();
  const Fare& fare1 = *fuVec[0]->paxTypeFare()->fare();
  const Fare& fare2 = *fuVec[1]->paxTypeFare()->fare();

  if (pu.geoTravelType() == GeoTravelType::International)
  {
    return (fare1.tcrRuleTariff() == fare2.tcrRuleTariff() && fare1.owrt() == fare2.owrt());
  }

  // domestic
  return (fare1.tcrRuleTariff() == fare2.tcrRuleTariff() && fare1.carrier() != CarrierCode("*J"));
}

MoneyAmount
PricingUtil::readSurchargeAndConvertCurrency(PricingTrx& trx,
                                             const Itin& curItin,
                                             const FareUsage& fareUsage)
{
  MoneyAmount surAmt = 0;
  const std::vector<SurchargeData*>& surchargeData = fareUsage.surchargeData();

  if (!surchargeData.empty())
  {
    std::vector<SurchargeData*>::const_iterator sI = surchargeData.begin();
    std::vector<SurchargeData*>::const_iterator sEnd = surchargeData.end();

    for (; sI != sEnd; ++sI)
    {
      surAmt += (*sI)->amountNuc();
    }
    CurrencyCode farePathCurrency = curItin.calculationCurrency();

    if (farePathCurrency != NUC && (farePathCurrency != USD))
    {
      CurrencyConversionFacade ccFacade;
      Money fareCurrency(farePathCurrency);
      Money taxCurrency(surAmt, NUC);

      if (ccFacade.convert(fareCurrency, taxCurrency, trx))
      {
        surAmt = fareCurrency.value();
      }
    }
  }

  return surAmt;
}

MoneyAmount
PricingUtil::readTaxAndConvertCurrency(PricingTrx& trx,
                                       const TaxResponse *taxResponse,
                                       Itin& curItin,
                                       const PaxTypeFare& ptf,
                                       PricingUnit& prU,
                                       bool& noBookingCodeTax,
                                       BookingCode& bkCode1,
                                       BookingCode& bkCode2,
                                       BookingCode& bkCode3,
                                       DiagCollector& diag)
{
  noBookingCodeTax = false;
  MoneyAmount taxAmt = 0;

  if (UNLIKELY(taxResponse == nullptr))
  {
    noBookingCodeTax = true;
    return taxAmt;
  }

  ShoppingTrx* shpTrx = dynamic_cast<ShoppingTrx*>(&trx);
  bool isSOLEnabled = (shpTrx) ? (shpTrx->isSumOfLocalsProcessingEnabled()) : false;

  TaxResponse::TaxItemVector::const_iterator taxItemIter = taxResponse->taxItemVector().begin();
  TaxResponse::TaxItemVector::const_iterator taxItemEndIter = taxResponse->taxItemVector().end();
  CurrencyCode taxCurrencyCode;
  uint16_t fareSegOrderStart = curItin.segmentOrder(ptf.fareMarket()->travelSeg().front());
  uint16_t fareSegOrderEnd = curItin.segmentOrder(ptf.fareMarket()->travelSeg().back());

  for (; taxItemIter != taxItemEndIter; taxItemIter++)
  {
    const TaxItem* taxItem = *taxItemIter;
    if (taxItem->taxAmount() == 0)
      continue;

    if ((fareSegOrderStart <= taxItem->segmentOrderStart()) &&
        (fareSegOrderEnd >= taxItem->segmentOrderEnd()))
    {
      if (UNLIKELY(taxItem->feeApplInd() == '1'))
      {
        const bool codeYQF = taxItem->taxCode().equalToConst("YRF") || taxItem->taxCode().equalToConst("YRI") ||
                             taxItem->taxCode().equalToConst("YQF");
        const bool codeYQI = taxItem->taxCode().equalToConst("YQI");

        if (isSOLEnabled)
        {
          if (codeYQF || codeYQI)
          {
            taxAmt += taxItem->taxAmount();
            taxCurrencyCode = taxItem->paymentCurrency();
            setBookingCodeFromTax(*taxItem, noBookingCodeTax, bkCode1, bkCode2, bkCode3, diag);
          }
        }
        else
        {
          if (ptf.fareMarket()->direction() == FMDirection::OUTBOUND &&
              ((codeYQF && !prU.isOutBoundYQFTaxCharged()) ||
               (codeYQI && !prU.isOutBoundYQITaxCharged())))
          {
            if (codeYQF && !taxItem->taxCode().equalToConst("YRI"))
            {
              prU.isOutBoundYQFTaxCharged() = true;
            }

            if (codeYQI)
            {
              prU.isOutBoundYQITaxCharged() = true;
            }

            taxAmt += taxItem->taxAmount();
            taxCurrencyCode = taxItem->paymentCurrency();
            setBookingCodeFromTax(*taxItem, noBookingCodeTax, bkCode1, bkCode2, bkCode3, diag);
          }

          if (ptf.fareMarket()->direction() == FMDirection::INBOUND &&
              ((codeYQF && !prU.isInBoundYQFTaxCharged()) ||
               (codeYQI && !prU.isInBoundYQITaxCharged())))
          {
            if (codeYQF && !taxItem->taxCode().equalToConst("YRI"))
            {
              prU.isInBoundYQFTaxCharged() = true;
            }

            if (codeYQI)
            {
              prU.isInBoundYQITaxCharged() = true;
            }

            taxAmt += taxItem->taxAmount();
            taxCurrencyCode = taxItem->paymentCurrency();
            setBookingCodeFromTax(*taxItem, noBookingCodeTax, bkCode1, bkCode2, bkCode3, diag);
          }
        }
      }
      else
      {
        taxAmt += taxItem->taxAmount();
        taxCurrencyCode = taxItem->paymentCurrency();
        setBookingCodeFromTax(*taxItem, noBookingCodeTax, bkCode1, bkCode2, bkCode3, diag);
      }
    }
    else if ((fareSegOrderStart >= taxItem->segmentOrderStart()) &&
             (fareSegOrderEnd <= taxItem->segmentOrderEnd()))
    {
      uint16_t numSegFare = fareSegOrderEnd - fareSegOrderStart;
      uint16_t numSegTax = taxItem->segmentOrderEnd() - taxItem->segmentOrderStart();
      uint16_t divider = numSegTax - numSegFare + 1;
      taxAmt += (taxItem->taxAmount()) / divider;
      taxCurrencyCode = taxItem->paymentCurrency();
      setBookingCodeFromTax(*taxItem, noBookingCodeTax, bkCode1, bkCode2, bkCode3, diag);
    }
  }

  if (taxAmt == 0)
  {
    noBookingCodeTax = true;
    return taxAmt;
  }

  CurrencyCode farePathCurrency = curItin.calculationCurrency();

  if (LIKELY((farePathCurrency != taxCurrencyCode)))
  {
    CurrencyConversionFacade ccFacade;
    Money fareCurrency(farePathCurrency);
    Money taxCurrency(taxAmt, taxCurrencyCode);

    if (LIKELY(ccFacade.convert(fareCurrency, taxCurrency, trx)))
    {
      taxAmt = fareCurrency.value();
    }
  }

  return taxAmt;
}

MoneyAmount
PricingUtil::convertCurrency(const PricingTrx& trx,
                             const MoneyAmount& amount,
                             const CurrencyCode& targetCurrency,
                             const CurrencyCode& sourceCurrency)
{
  if (targetCurrency != sourceCurrency)
  {
    CurrencyConversionFacade ccFacade;
    Money targetValue(targetCurrency);
    Money sourceValue(amount, sourceCurrency);
    if (ccFacade.convert(targetValue, sourceValue, trx))
    {
      return targetValue.value();
    }
  }

  return amount;
}

void
PricingUtil::setBookingCodeFromTax(const TaxItem& taxItem,
                                   bool& noBookingCodeTax,
                                   BookingCode& bkCode1,
                                   BookingCode& bkCode2,
                                   BookingCode& bkCode3,
                                   DiagCollector& diag)
{
  printTaxItem(taxItem, diag);

  if (LIKELY(taxItem.bookingCode1().empty()))
  {
    noBookingCodeTax = true;
    return;
  }

  bkCode1 = taxItem.bookingCode1();

  if (!taxItem.bookingCode2().empty())
  {
    bkCode2 = taxItem.bookingCode2();
  }

  if (!taxItem.bookingCode3().empty())
  {
    bkCode3 = taxItem.bookingCode3();
  }

  return;
}

void
PricingUtil::printTaxItem(const TaxItem& taxItem, DiagCollector& diag)
{
  if (LIKELY(!diag.isActive()))
    return;

  diag << taxItem.taxCode() << " " << taxItem.segmentOrderStart() << "-"
       << taxItem.segmentOrderEnd() << " " << taxItem.taxAmount() << " "
       << taxItem.paymentCurrency() << " " << taxItem.seqNo() << std::endl;
}

void
PricingUtil::processNetFarePath(PricingTrx& trx, FarePath& farePath, bool isRegularNet)
{
  farePath.netFarePath() = createNetFarePath(trx, farePath, isRegularNet);
}

NetFarePath*
PricingUtil::createNetFarePath(PricingTrx& trx, FarePath& farePath, bool isRegularNet)
{
  NetFarePath* nfp = trx.dataHandle().create<NetFarePath>();
  if (nfp)
  {
    nfp->initialize(&trx, &farePath);
    if ((ItinUtil::isDomesticPeru(&trx, farePath.itin())) &&
        (farePath.calculationCurrency() != NUC))
      roundPeruTotalAmount(trx, *nfp);
  }
  if (isRegularNet)
  {
    nfp->regularNet() = true;
  }
  return nfp;
}

bool
PricingUtil::validateSpanishDiscountInFP(FarePath* farePath)
// This util method returns true if all or none  of the pax
// type fares have spanish discount for a given fare path
{
  if (!farePath)
    return true;

  bool isFirstFare = true;
  bool isDiscountedFare = false;

  for (PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (isFirstFare)
      {
        isDiscountedFare = fareUsage->paxTypeFare()->isSpanishDiscountEnabled();
        isFirstFare = false;
      }
      else
      {
        if (fareUsage->paxTypeFare()->isSpanishDiscountEnabled() != isDiscountedFare)
          return false;
      }
    }
  }
  return true;
}

BrandCodeSet
PricingUtil::getCommonBrandsFromPU(PricingTrx& trx, const PricingUnit* pricingUnit)
{
  if (pricingUnit->fareUsage().size() == 0)
    return BrandCodeSet();

  utils::SetIntersection<BrandCode> setIntersection;

  for (FareUsage* fareUsage : pricingUnit->fareUsage())
  {
    std::vector<BrandCode> currFareBrands;
    fareUsage->paxTypeFare()->getValidBrands(trx, currFareBrands);

    BrandCodeSet currFareBrandCodes;
    currFareBrandCodes.insert(currFareBrands.begin(), currFareBrands.end());

    setIntersection.addSet(currFareBrandCodes);
    if (setIntersection.get().empty())
      break;
  }

  return setIntersection.get();
}

// This function takes two vectors of CarrierCodes and update list1 with the intersation
// of those two vectors. list2 remain unchanged

void
PricingUtil::intersectCarrierList(std::vector<CarrierCode>& list1,
                                  const std::vector<CarrierCode>& list2)
{
  std::vector<CarrierCode> newList;

  for (const CarrierCode& cxrCode1 : list1)
  {
    for (const CarrierCode& cxrCode2 : list2)
    {
      if (cxrCode1 == cxrCode2)
      {
        newList.push_back(cxrCode1);
        break;
      }
    }
  }

  list1.swap(newList);
}

} // tse
