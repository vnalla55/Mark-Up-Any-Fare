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

#include "Taxes/LegacyTaxes/TaxYQ.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1ValidatorYQ.h"

#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "Taxes/LegacyTaxes/EquipmentValidator.h"
#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"

#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxRange.h"
#include "Common/TaxRound.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"

#include "DBAccess/Loc.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Currency.h"
#include "Common/Money.h"
#include "DBAccess/NUCInfo.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/BSRCollectionResults.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/TrxUtil.h"
#include "Common/Global.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "Rules/RuleUtil.h"

#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Common/CurrencyConversionFacade.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ReissueCharges.h"

using namespace std;

log4cxx::LoggerPtr
tse::YQYR::Tax::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.YQYR.Tax"));

namespace tse
{
namespace YQYR
{
MoneyAmount
Tax::calculateFareDependendTaxableAmount(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1,
                                         CurrencyConversionFacade& ccFacade)
{
  MoneyAmount moneyAmount = fareAmountInNUC(trx, taxResponse);

  utc::FareType fareType(trx, taxSpecConfig());
  bool isFareTypeToValidate = fareType.mustBeValidate();

  // Alwasy unset for YQ
  if (/*serviceFeeRec1.taxfullFareInd() == YES ||*/ isFareTypeToValidate)
  {
    moneyAmount = 0.0;

    const FarePath* farePath = taxResponse.farePath();

    std::vector<PricingUnit*>::const_iterator pricingUnitI = farePath->pricingUnit().begin();
    std::vector<FareUsage*>::iterator fareUsageI;

    for (; pricingUnitI != farePath->pricingUnit().end(); pricingUnitI++)
    {

      for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
           fareUsageI != (*pricingUnitI)->fareUsage().end();
           fareUsageI++)
      {
        FareUsage& fareUsage = *(*fareUsageI);

        // if(isFareTypeToValidate && !fareType.validate(trx, taxResponse, fareUsage,
        // _travelSegStartIndex, _travelSegEndIndex ))
        if (isFareTypeToValidate && !fareType.validateFareTypes(fareUsage))
          continue;

        const PaxTypeFare* ptCurrFare = nullptr;

        if (fareUsage.adjustedPaxTypeFare() == nullptr)
        { // normal processing
          ptCurrFare = fareUsage.paxTypeFare();
        }
        else // Currency adjustment processing for Nigeria
        {
          ptCurrFare = fareUsage.adjustedPaxTypeFare();
        }
        const PaxTypeFare* ptFare = ptCurrFare->fareWithoutBase(); // real base fare
        moneyAmount += ptFare->fare()->nucFareAmount();
      }
    }
  }

  // Check for Plus Up Pricing
  if (taxResponse.farePath()->itin()->isPlusUpPricing())
  {
    ConsolidatorPlusUp* cPlusUp = taxResponse.farePath()->itin()->consolidatorPlusUp();
    moneyAmount +=
        cPlusUp->calcTaxablePlusUpAmount(trx, serviceFeeRec1.taxCode(), taxResponse.farePath());
  }

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
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, serviceFeeRec1, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    moneyAmount = targetMoneyOrigination.value();
  }

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
  {
    Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, serviceFeeRec1, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    moneyAmount = targetMoney.value();
  }
  return moneyAmount;
}

MoneyAmount
Tax::calculateChangeFeeTaxableAmount(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1,
                                     CurrencyConversionFacade& ccFacade)

{
  MoneyAmount changeFeeAmt = 0;

  const std::map<const PaxTypeFare*, PenaltyFee*>& penaltyFees =
      taxResponse.farePath()->reissueCharges()->penaltyFees;

  std::map<const PaxTypeFare*, PenaltyFee*>::const_iterator penaltyFeesI = penaltyFees.begin();
  std::map<const PaxTypeFare*, PenaltyFee*>::const_iterator penaltyFeesIEnd = penaltyFees.end();

  PenaltyFee* penaltyFee;

  for (; penaltyFeesI != penaltyFeesIEnd; penaltyFeesI++)
  {
    penaltyFee = penaltyFeesI->second;

    if (_paymentCurrency != penaltyFee->penaltyCurrency)
    {
      Money targetMoney(_paymentCurrency);
      targetMoney.value() = 0;

      Money sourceMoney(penaltyFee->penaltyAmount, penaltyFee->penaltyCurrency);

      if (!ccFacade.convert(targetMoney,
                            sourceMoney,
                            trx,
                            taxResponse.farePath()->itin()->useInternationalRounding(),
                            CurrencyConversionRequest::FARES))
      {
        LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

        TaxDiagnostic::collectErrors(
            trx, serviceFeeRec1, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
        continue;
      }
      else
      {
        changeFeeAmt += targetMoney.value();
      }
    }
    else
      changeFeeAmt += penaltyFee->penaltyAmount;
  }

  return changeFeeAmt;
}
} // YQYR namespace
} // tse namespace
