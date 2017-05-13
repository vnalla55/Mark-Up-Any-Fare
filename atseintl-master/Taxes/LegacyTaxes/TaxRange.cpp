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

#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TseUtil.h"

#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/Nation.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"

#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxRange.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"

namespace tse
{
namespace
{
Logger logger("atseintl.Taxes.TaxRange");
}

bool
TaxRange::validateRange(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& startIndex,
                        uint16_t& endIndex) const
{
  if ((taxCodeReg.rangeType() != FARE) && (taxCodeReg.rangeType() != MILEAGE) &&
      (!taxCodeReg.rangeincrement()))
    return true;

  if (taxCodeReg.rangeType() == FARE)
  {
    MoneyAmount fareAmount = 0.0;

    if (taxCodeReg.rangeInd() == TRIP)
    {
      fareAmount = retrieveTripFare(
          trx, taxResponse, *taxResponse.farePath(), taxCodeReg.taxCur(), startIndex);
    }

    if (taxCodeReg.rangeInd() == TOTAL)
    {
      fareAmount =
          retrieveTotalFare(trx, taxResponse, *taxResponse.farePath(), taxCodeReg.taxCur());
    }

    if ((taxCodeReg.lowRange() == 0) && (taxCodeReg.highRange() == 0) && (fareAmount > 0.0))
      return true;

    if ((taxCodeReg.lowRange() <= fareAmount) && (taxCodeReg.highRange() == 0) &&
        (fareAmount > 0.0))
      return true;

    if ((taxCodeReg.lowRange() == 0) && (taxCodeReg.highRange() >= fareAmount) &&
        (fareAmount > 0.0))
      return true;

    if ((taxCodeReg.lowRange() > fareAmount) || (taxCodeReg.highRange() < fareAmount) ||
        (fareAmount == 0.0))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::RANGE_FARE, Diagnostic817);

      return false;
    }
    return true;
  }

  if (taxCodeReg.rangeType() == MILEAGE)
  {
    const uint32_t tripMiles =
      retrieveRangeMiles(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    if ((taxCodeReg.highRange() == 0) && (taxCodeReg.lowRange() <= tripMiles))
      return true;

    if ((taxCodeReg.highRange() >= tripMiles) && (taxCodeReg.lowRange() == 0))
      return true;

    if ((taxCodeReg.lowRange() > tripMiles) || (taxCodeReg.highRange() < tripMiles))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::RANGE_MILES, Diagnostic817);

      return false;
    }
  }

  return true;
}

MoneyAmount
TaxRange::applyRange(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     const MoneyAmount& taxAmount,
                     const CurrencyCode& paymentCurrency,
                     uint16_t& startIndex,
                     uint16_t& endIndex,
                     TaxCodeReg& taxCodeReg) const

{
  MoneyAmount taxRangeAmount = 0;

  if (LIKELY(!taxCodeReg.rangeincrement()))
    return taxRangeAmount;

  if (taxCodeReg.rangeType() == FARE)
  {
    MoneyAmount fareAmount = 0;

    if (taxCodeReg.rangeInd() == TRIP)
    {
      fareAmount = retrieveTripFare(
          trx, taxResponse, *taxResponse.farePath(), taxCodeReg.taxCur(), startIndex);
    }

    if (taxCodeReg.rangeInd() == TOTAL)
    {
      fareAmount =
          retrieveTotalFare(trx, taxResponse, *taxResponse.farePath(), taxCodeReg.taxCur());
    }

    taxRangeAmount = taxAmount;

    if ((fareAmount) && (taxCodeReg.rangeincrement()))
    {
      uint64_t increment = static_cast<uint64_t>(fareAmount / taxCodeReg.rangeincrement());

      if (increment)
      {
        taxRangeAmount = increment * taxRangeAmount;

        increment =
            static_cast<uint64_t>(fareAmount) % static_cast<uint64_t>(taxCodeReg.rangeincrement());

        if (increment)
          taxRangeAmount += taxAmount;
      }
    }
    return taxRangeAmount;
  }

  if ((taxCodeReg.rangeType() == MILEAGE) && (taxCodeReg.rangeincrement()))
  {
    const uint32_t tripMiles =
      retrieveRangeMiles(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    const uint64_t increment = static_cast<uint64_t>(tripMiles / taxCodeReg.rangeincrement());
    taxRangeAmount += increment * taxRangeAmount;
  }

  return taxRangeAmount;
}

MoneyAmount
TaxRange::retrieveTripFare(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           FarePath& farePath,
                           const CurrencyCode& currencyCode,
                           uint16_t& startIndex) const
{
  CurrencyConversionFacade ccFacade;

  Money targetMoney(currencyCode);
  targetMoney.value() = 0;

  MoneyAmount moneyAmount = 0.0;

  // lint -e{530}
  TravelSeg* travelSeg = farePath.itin()->travelSeg()[startIndex];

  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;
  std::vector<TravelSeg*>::const_iterator travelSegFuI;

  for (pricingUnitI = farePath.pricingUnit().begin(); pricingUnitI != farePath.pricingUnit().end();
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
        if (farePath.itin()->segmentOrder(travelSeg) !=
            farePath.itin()->segmentOrder(*travelSegFuI))
          continue;

        moneyAmount = (*fareUsageI)->totalFareAmount();

        if (farePath.calculationCurrency() != farePath.baseFareCurrency())
        {
          Money targetMoneyOrigination(farePath.baseFareCurrency());
          targetMoneyOrigination.value() = 0;

          Money sourceMoneyCalculation(moneyAmount, farePath.calculationCurrency());

          if (!ccFacade.convert(targetMoneyOrigination,
                                sourceMoneyCalculation,
                                trx,
                                false,
                                CurrencyConversionRequest::DC_CONVERT))
          {
            LOG4CXX_WARN(logger,
                         "Currency Convertion Collection *** TaxRange::retrieveTripFare ***");

            TaxDiagnostic::collectErrors(
                trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
          }
          moneyAmount = targetMoneyOrigination.value();
        }

        if (farePath.baseFareCurrency() != currencyCode)
        {
          Money sourceMoney(moneyAmount, farePath.baseFareCurrency());

          if (!ccFacade.convert(
                  targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::DC_CONVERT))
          {
            Money sourceMoneyTaxes(moneyAmount, farePath.baseFareCurrency());

            if (!ccFacade.convert(
                    targetMoney, sourceMoneyTaxes, trx, false, CurrencyConversionRequest::TAXES))
            {
              LOG4CXX_WARN(logger,
                           "Currency Convertion Collection *** TaxRange::retrieveTripFare ***");

              TaxDiagnostic::collectErrors(
                  trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic817);
            }
          }
          moneyAmount = targetMoney.value();
        }
        return moneyAmount;
      }
    }
  }
  return moneyAmount;
}

// ----------------------------------------------------------------------------
// Description:  retrieveTotalFare
// ----------------------------------------------------------------------------

MoneyAmount
TaxRange::retrieveTotalFare(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            FarePath& farePath,
                            const CurrencyCode& currencyCode) const
{
  CurrencyConversionFacade ccFacade;

  Money targetMoney(currencyCode);
  targetMoney.value() = 0;

  MoneyAmount moneyAmount = farePath.getTotalNUCAmount();

  if (farePath.calculationCurrency() != farePath.baseFareCurrency())
  {
    Money targetMoneyOrigination(farePath.baseFareCurrency());
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, farePath.calculationCurrency());

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          false,
                          CurrencyConversionRequest::DC_CONVERT))
    {
      LOG4CXX_WARN(logger, "Currency Convertion Collection *** TaxRange::retrieveTotalFare ***");

      TaxDiagnostic::collectErrors(
          trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic817);
    }
    moneyAmount = targetMoneyOrigination.value();
  }

  if (farePath.baseFareCurrency() != currencyCode)
  {
    Money sourceMoney(moneyAmount, farePath.baseFareCurrency());

    if (!ccFacade.convert(
            targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::DC_CONVERT))
    {
      Money sourceMoneyTaxes(moneyAmount, farePath.baseFareCurrency());

      if (!ccFacade.convert(
              targetMoney, sourceMoneyTaxes, trx, false, CurrencyConversionRequest::TAXES))
      {
        LOG4CXX_WARN(logger, "Currency Convertion Collection *** TaxRange::retrieveTripFare ***");

        TaxDiagnostic::collectErrors(
            trx, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic817);
      }
    }
    moneyAmount = targetMoney.value();
  }

  return moneyAmount;
}

uint32_t
TaxRange::retrieveRangeMiles(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex) const
{
  std::vector<TravelSeg*>::iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  GlobalDirection gd = GlobalDirection::XX;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(
      &trx, taxResponse.farePath()->itin()->travelDate(), **travelSegI, gd);

  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_DEBUG(logger, "TPMCollector::process(): GlobalDirection Not Found");
    return 0;
  }

  if ((taxCodeReg.rangeInd() != STOP_OVER) ||
      ((*travelSegI) == taxResponse.farePath()->itin()->travelSeg().back()) ||
      ((*travelSegI)->isForcedStopOver()))

  {
    return LocUtil::getTPM(*(*travelSegI)->origin(),
                           *(*travelSegI)->destination(),
                           gd,
                           trx.getRequest()->ticketingDT(),
                           trx.dataHandle());
  }

  //  Ticket Point Miles is point to point until Stopover is reached

  bool transit;
  const AirSeg* airSeg;

  MirrorImage mirrorImage;
  TransitValidator transitValidator;

  uint16_t index = startIndex;
  uint32_t tripMiles = 0;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    index++;
    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &trx, taxResponse.farePath()->itin()->travelDate(), **travelSegI, gd);

    if (gd == GlobalDirection::XX)
    {
      LOG4CXX_DEBUG(logger, "TPMCollector::process(): GlobalDirection Not Found");
      return 0;
    }

    tripMiles += LocUtil::getTPM(*(*travelSegI)->origin(),
                                 *(*travelSegI)->destination(),
                                 gd,
                                 trx.getRequest()->ticketingDT(),
                                 trx.dataHandle());

    if ((*travelSegI) == taxResponse.farePath()->itin()->travelSeg().back())
      break;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      break;

    if ((*travelSegI)->isForcedConx())
      continue;

    if ((*travelSegI)->isForcedStopOver())
      break;

    if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, index))
      break;

    transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index);

    if (!transit)
      break;
  }

  endIndex = index;
  endIndex--;

  return tripMiles;
}
}
