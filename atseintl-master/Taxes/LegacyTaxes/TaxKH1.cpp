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

#include "Taxes/LegacyTaxes/TaxKH1.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
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
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxKH1::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxKH1"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
TaxKH1::TaxKH1() : _fareUsage(nullptr) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
TaxKH1::~TaxKH1() {}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

void
TaxKH1::taxCreate(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  TaxCodeReg& taxCodeReg,
                  uint16_t travelSegStartIndex,
                  uint16_t travelSegEndIndex)
{
  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegStartIndex];

  if (travelSeg == nullptr)
    return;

  if (TrxUtil::isPricingTaxRequest(&trx))
  {
    LOG4CXX_INFO(_logger, "No Partial Taxes Allowed For TaxTrx Requests");
    return;
  }

  const Itin* itin = taxResponse.farePath()->itin();
  const AirSeg* airSeg;

  if (!locateFareUsage(taxResponse, *travelSeg))
  {
    LOG4CXX_WARN(_logger, "Bad Fare Usage");
    return;
  }

  airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return;

  MoneyAmount moneyAmount = _fareUsage->paxTypeFare()->totalFareAmount();

  if (!_fareUsage->paxTypeFare()->isForeignDomestic())
  {
    std::vector<TravelSeg*> travelSegFMVec;
    travelSegFMVec.push_back(travelSeg);

    const FareMarket* fareMarket = TrxUtil::getFareMarket(
        trx,
        airSeg->marketingCarrierCode(),
        travelSegFMVec,
        _fareUsage ? _fareUsage->paxTypeFare()->retrievalDate() : trx.ticketingDate(),
        itin);

    if (!fareMarket)
    {
      LOG4CXX_WARN(_logger,
                   "Bad FareMarket For PaxType: " << taxResponse.farePath()->paxType()->paxType());
      return;
    }

    const PaxTypeBucket* paxTypeCortege =
        fareMarket->paxTypeCortege(taxResponse.farePath()->paxType());

    if (!paxTypeCortege)
    {
      LOG4CXX_WARN(_logger,
                   "Bad FareMarket For PaxType: " << taxResponse.farePath()->paxType()->paxType());
      return;
    }

    const std::vector<PaxTypeFare*>& paxTypeFare = paxTypeCortege->paxTypeFare();

    std::vector<PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFare.begin();
    std::vector<PaxTypeFare*>::const_iterator paxTypeFareEndI = paxTypeFare.end();
    std::vector<PaxTypeFare*>::const_iterator paxTypeFareSetI = paxTypeFare.end();

    const CarrierCode itinVcxr = taxResponse.validatingCarrier();

    for (; paxTypeFareI != paxTypeFareEndI; paxTypeFareI++)
    {
      if ((_fareUsage->isInbound()) && ((*paxTypeFareI)->directionality() == FROM))
        continue;

      if ((_fareUsage->isOutbound()) && ((*paxTypeFareI)->directionality() == TO))
        continue;

      if ( !(*paxTypeFareI)->isValid() ||
          !isValidForValidatingCarrier(trx, (**paxTypeFareI), itinVcxr) )
        continue;

      paxTypeFareSetI = paxTypeFareI;
      break;
    }

    if (paxTypeFareSetI == paxTypeFareEndI)
    {
      LOG4CXX_DEBUG(
          _logger,
          "No Matching Fare Located For PaxType: " << taxResponse.farePath()->paxType()->paxType());
      return;
    }

    moneyAmount = (*paxTypeFareSetI)->totalFareAmount();
  }
  const CurrencyCode& calculationCurrency = taxResponse.farePath()->calculationCurrency();
  const CurrencyCode& baseFareCurrency = taxResponse.farePath()->baseFareCurrency();

  _calculationDetails.baseFareSumAmount = moneyAmount;
  _calculationDetails.calculationCurrency = calculationCurrency;
  _calculationDetails.fareInCalculationCurrencyAmount = moneyAmount;

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  CurrencyConversionFacade ccFacade;

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  if (calculationCurrency != baseFareCurrency)
  {
    Money targetMoneyOrigination(taxResponse.farePath()->baseFareCurrency());
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, taxResponse.farePath()->calculationCurrency());

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &_calculationDetails.calculationToBaseFareResults))
    {
      LOG4CXX_WARN(_logger,
                   "Currency Convertion Failure To Convert: "
                       << taxResponse.farePath()->calculationCurrency() << " To "
                       << taxResponse.farePath()->baseFareCurrency());
    }
    moneyAmount = targetMoneyOrigination.value();
  }

  _calculationDetails.baseFareCurrency = baseFareCurrency;
  _calculationDetails.fareInBaseFareCurrencyAmount = moneyAmount;

  if (baseFareCurrency != _paymentCurrency)
  {
    Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &_calculationDetails.baseFareToPaymentResults))
    {
      LOG4CXX_WARN(_logger,
                   "Currency Convertion Failure To Convert: "
                       << taxResponse.farePath()->baseFareCurrency() << " To " << _paymentCurrency);
    }
    moneyAmount = targetMoney.value();
  }

  _calculationDetails.fareInPaymentCurrencyAmount = moneyAmount;

  _taxSplitDetails.setIsSet(true);

  _failCode = TaxDiagnostic::NONE;
  _partialTax = true;
  _thruTotalFare = moneyAmount;
  _taxablePartialFare = moneyAmount;
  _travelSegPartialStartOrder = itin->segmentOrder(travelSeg);
  _travelSegPartialEndOrder = itin->segmentOrder(travelSeg);
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegStartIndex;
  _taxableFare = moneyAmount;
  _taxAmount = taxCodeReg.taxAmt() * _taxableFare;

  Money decMoney(_paymentCurrency);
  _paymentCurrencyNoDec = decMoney.noDec(trx.ticketingDate());

  doTaxRound(trx, taxCodeReg);

  TaxApply taxApply;
  taxApply.initializeTaxItem(trx, *this, taxResponse, taxCodeReg);
  _failCode = TaxDiagnostic::NO_TAX_ADDED;
}

bool
TaxKH1::isValidForValidatingCarrier(
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

/// ----------------------------------------------------------------------------
// Description:  locatedFareUsage
// ----------------------------------------------------------------------------

bool
TaxKH1::locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  const FarePath* farePath = taxResponse.farePath();

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
