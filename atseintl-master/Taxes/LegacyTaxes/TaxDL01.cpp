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

#include "Common/Logger.h"

#include "Taxes/LegacyTaxes/TaxDL01.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"

#include "DBAccess/DataHandle.h"

#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Currency.h"
#include "Common/Money.h"
#include "DBAccess/NUCInfo.h"
#include "Common/ErrorResponseException.h"
#include "Common/CurrencyConversionFacade.h"

#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"

#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "Common/TaxRound.h"

#include "Common/Global.h"

#include "Common/FallbackUtil.h"

using namespace tse;
using namespace std;

Logger TaxDL01::_logger("atseintl.Taxes.TaxDL");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
TaxDL01::TaxDL01() : USH("USH"), RGA("RGA"), _fareUsage(nullptr), _fdRequest(false) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
TaxDL01::~TaxDL01() {}

// ----------------------------------------------------------------------------
// Description:  validateTripTypes
// ----------------------------------------------------------------------------

bool
TaxDL01::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)

{
  uint16_t segmentOrder = 0;
  bool locOrigin;
  bool locDestination;

  _taxAmount = taxCodeReg.taxAmt();
  _travelSegStartIndex = 0;
  _travelSegEndIndex = 0;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    if (segmentOrder >= taxResponse.farePath()->itin()->segmentOrder(*travelSegI))
      continue;

    if (!(*travelSegI)->isAir())
      continue;

    locOrigin = LocUtil::isInLoc(*(*travelSegI)->origin(),
                                 taxCodeReg.loc1Type(),
                                 taxCodeReg.loc1(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                      taxCodeReg.loc2Type(),
                                      taxCodeReg.loc2(),
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    if (!locOrigin || !locDestination)
      continue;

    if (!locateFareUsage(taxResponse, **travelSegI))
    {
      LOG4CXX_DEBUG(_logger, "Fare Not Found *** TaxDL::validateTripTypes ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);

      continue;
    }

    TravelSeg* travelSegFront = _fareUsage->travelSeg().front();
    TravelSeg* travelSegBack = _fareUsage->travelSeg().back();

    segmentOrder = taxResponse.farePath()->itin()->segmentOrder(travelSegBack);

    locOrigin = LocUtil::isInLoc(*travelSegFront->origin(),
                                 taxCodeReg.loc1Type(),
                                 taxCodeReg.loc1(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());

    locDestination = LocUtil::isInLoc(*travelSegBack->destination(),
                                      taxCodeReg.loc2Type(),
                                      taxCodeReg.loc2(),
                                      Vendor::SABRE,
                                      MANUAL,
                                      LocUtil::TAXES,
                                      GeoTravelType::International,
                                      EMPTY_STRING(),
                                      trx.getRequest()->ticketingDT());

    if (!locOrigin || !locDestination)
      continue;

    uint32_t segmentOrder = taxResponse.farePath()->itin()->segmentOrder(*travelSegI);
    _tvlSegToFareAmount[segmentOrder - 1] = _fareUsage->totalFareAmount();
  }

  if (taxCodeReg.taxAmt() == 0.0 && taxCodeReg.taxType() == PERCENTAGE)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

void
TaxDL01::taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex)
{
  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  CurrencyConversionFacade ccFacade;
  Money targetMoney(_paymentCurrency);

  _paymentCurrencyNoDec = targetMoney.noDec(trx.getRequest()->ticketingDT());

  std::map<uint32_t, MoneyAmount>::iterator iter = _tvlSegToFareAmount.begin();
  for(; iter != _tvlSegToFareAmount.end(); ++iter)
  {
    CurrencyConversionFacade ccFacade;
    Money targetMoney(_paymentCurrency);
    MoneyAmount moneyAmount = iter->second;

    if (taxResponse.farePath()->calculationCurrency() != taxResponse.farePath()->baseFareCurrency())
    {
      Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
      targetMoneyOrigination.value() = 0;

      Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

      if (!ccFacade.convert(targetMoneyOrigination,
                            sourceMoneyCalculation,
                            trx,
                            taxResponse.farePath()->itin()->useInternationalRounding()))
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic820);
      }
      moneyAmount = targetMoneyOrigination.value();
    }

    if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
    {
      moneyAmount = convertAmoutToPaymentCurrency(trx, moneyAmount,
                                                  taxResponse.farePath()->baseFareCurrency());
    }
    _taxableFare += moneyAmount;

    if(_fdRequest)
        break;
  }

  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

  if (_taxAmount == 0.0)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);
  }

}

/// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
TaxDL01::locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg)
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
        if (taxResponse.farePath()->itin()->segmentOrder(*travelSegFuI) !=
            taxResponse.farePath()->itin()->segmentOrder(&travelSeg))
          continue;

        _fareUsage = *fareUsageI;
        return true;
      }
    }
  }
  return false;
}

void
TaxDL01::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{


  std::map<TaxItem*, MoneyAmount> previousTaxState;


  const FarePath* farePath = taxResponse.farePath();
  MoneyAmount moneyAmount = farePath->getTotalNUCAmount();
  bool isExcludeCity = false;

  for (const std::string& taxCode : taxCodeReg.taxOnTaxCode())
  {
    for (TaxItem* taxItem : taxResponse.taxItemVector())
    {
      if (taxCode == taxItem->taxCode() &&
          taxItem->taxCode().equalToConst("QN") &&
          taxItem->taxAmt() != 0)
      {
        for (const PricingUnit* pu : farePath->pricingUnit())
        {
          for (const FareUsage* fu : pu->fareUsage())
          {
            for (const TravelSeg* seg : fu->travelSeg())
            {
              if(seg->origin()->loc() == USH ||
                 seg->origin()->loc() == RGA ||
                 seg->destination()->loc() == USH ||
                 seg->destination()->loc() == RGA)
              {
                isExcludeCity = true;
                moneyAmount -= fu->paxTypeFare()->nucFareAmount();
                break;
              }
            }
          }
        }

        if(!isExcludeCity)
        {
          break;
        }

        previousTaxState[taxItem] = taxItem->taxAmount();

        if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
        {
          moneyAmount = convertAmoutToPaymentCurrency(trx, moneyAmount,
                                                      taxResponse.farePath()->baseFareCurrency());
        }

        taxItem->taxAmount() = moneyAmount * taxItem->taxAmt();
        TaxRound taxRound;
        RoundingFactor roundingUnit = taxCodeReg.taxcdRoundUnit();
        CurrencyNoDec roundingNoDec = taxCodeReg.taxcdRoundUnitNodec();
        RoundingRule roundingRule = taxCodeReg.taxcdRoundRule();

        if ((taxCodeReg.taxCur() != _paymentCurrency) || (roundingRule == EMPTY))
        {
          taxRound.retrieveNationRoundingSpecifications(trx, roundingUnit, roundingNoDec, roundingRule);
        }

        taxItem->taxAmount() = taxRound.applyTaxRound(taxItem->taxAmount(), _paymentCurrency,
                                                      roundingUnit, roundingRule);
      }
    }
  }

  Tax::applyTaxOnTax(trx, taxResponse, taxCodeReg);

  std::map<TaxItem*, MoneyAmount>::iterator iter = previousTaxState.begin();
  for(; iter != previousTaxState.end(); ++iter)
  {
    iter->first->taxAmount() = iter->second;
  }
}

MoneyAmount
TaxDL01::convertAmoutToPaymentCurrency(PricingTrx& trx, MoneyAmount moneyAmount,
                                       CurrencyCode currencyCode)
{
  CurrencyConversionFacade ccFacade;
  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;
  Money sourceMoney(moneyAmount, currencyCode);

  if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
  {
    LOG4CXX_DEBUG(_logger, "TaxDL01::convertAmoutToPaymentCurrency: Error in conversion money amount to "
                            << currencyCode);
  }
  return targetMoney.value();

}

