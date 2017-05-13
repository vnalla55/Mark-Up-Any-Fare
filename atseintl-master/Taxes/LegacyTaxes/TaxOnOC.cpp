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

#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxOnOC.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"

#include "Taxes/LegacyTaxes/CarrierValidatorOC.h"
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
#include "Common/Logger.h"
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
#include "DBAccess/TaxSpecConfigReg.h"
#include "ServiceFees/OCFees.h"
#include "Common/ServiceFeeUtil.h"

#include "Common/FallbackUtil.h"

using namespace tse;
using namespace std;

static Logger
loggerTaxOnOC("atseintl.Taxes.TaxOnOC");

bool
TaxOnOC::validateCarrierExemption(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex)
{

  CarrierValidatorOC carrierValidator;
  carrierValidator.setOcFees(_ocFees);

  return carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxOnOC::validateSequence(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t& travelSegStartIndex,
                          uint16_t& travelSegEndIndex,
                          bool checkSpn)
{
  std::vector<OCFees::TaxItem>::const_iterator iTaxItem = _ocFees->getTaxes().begin();
  std::vector<OCFees::TaxItem>::const_iterator iETaxItem = _ocFees->getTaxes().end();

  for (; iTaxItem != iETaxItem; iTaxItem++)
  {
    if (iTaxItem->getTaxCode() == taxCodeReg.taxCode())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_SEGMENT, Diagnostic809);
      return false;
    }
  }

  return true;
}

void
TaxOnOC::taxCreate(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t travelSegStartIndex,
                   uint16_t travelSegEndIndex)
{

  CurrencyConversionFacade ccFacade;

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());

  _taxAmount = taxCodeReg.taxAmt();
  _taxableFare = 0.0;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;

  if (_taxAmount == 0.0)
    return;

  if (taxCodeReg.taxType() == PERCENTAGE)
  {
    _taxableFare = calculateTaxableAmount(trx, taxResponse, taxCodeReg, ccFacade);
    _taxAmount *= _taxableFare;

    return;
  }

  if (taxCodeReg.taxType() == FIXED)
  {
    if (taxCodeReg.taxCur() == _paymentCurrency)
      return;

    Money sourceMoney(taxCodeReg.taxAmt(), taxCodeReg.taxCur());
    BSRCollectionResults bsrResults;

    if (!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &bsrResults))
    {
      LOG4CXX_WARN(loggerTaxOnOC, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }

    _taxAmount = targetMoney.value();
  }
}

MoneyAmount
TaxOnOC::calculateTaxableAmount(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                CurrencyConversionFacade& ccFacade)
{
  MoneyAmount moneyAmount = _ocFees->feeAmount();

  ServiceFeeUtil util(trx);
  Money targetMoney = util.convertOCFeeCurrency(_ocFees);
  moneyAmount = targetMoney.value();
  _paymentCurrency = targetMoney.code();

  return moneyAmount;
}
