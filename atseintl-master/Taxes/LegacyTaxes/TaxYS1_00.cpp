// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxYS1_00.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "DataModel/Agent.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/Tax.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxYS1_00::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxYS1_00"));

void
TaxYS1_00::taxCreate(PricingTrx& trx,
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

  _taxAmountAdjusted = taxCodeReg.taxAmt();
  _taxableFareAdjusted = 0.0;

  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;

  if (_taxAmount == 0.0)
    return;

  MoneyAmount moneyAmount;
  NUCCurrencyConverter nucConverter;
  CurrencyNoDec convertedAmtNoDec;
  CurrencyCode convertedCurrencyCode;
  ExchRate roeRate;
  CurrencyNoDec roeRateNoDec;
  DateTime nucEffectiveDate;
  DateTime nucDiscontinueDate;

  if (!nucConverter.convertBaseFare(trx,
                                    *(taxResponse.farePath()),
                                    taxResponse.farePath()->getTotalNUCAmount(),
                                    moneyAmount,
                                    convertedAmtNoDec,
                                    convertedCurrencyCode,
                                    roeRate,
                                    roeRateNoDec,
                                    nucEffectiveDate,
                                    nucDiscontinueDate,
                                    taxResponse.farePath()->itin()->useInternationalRounding(),
                                    true))
  {
    LOG4CXX_WARN(_logger, "Currency Convertion Collection *** TaxYS1_00::taxCreate ***");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
  }

  if (taxResponse.farePath()->baseFareCurrency() != _paymentCurrency)
  {
    Money sourceMoney(moneyAmount, taxResponse.farePath()->baseFareCurrency());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** TaxYS1_00::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    moneyAmount = targetMoney.value();
  }

  _taxableFare = moneyAmount;
  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

  _taxableFareAdjusted = moneyAmount;
  _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();

  return;
}
