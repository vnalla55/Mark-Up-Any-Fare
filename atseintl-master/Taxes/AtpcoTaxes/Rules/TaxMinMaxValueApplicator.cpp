// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include <sstream>

#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "Rules/MathUtils.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxMinMaxValueApplicator.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "Rules/TaxPointUtils.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
TaxMinMaxValueApplicator::TaxMinMaxValueApplicator(const TaxMinMaxValueRule* rule,
                                                   const Services& services,
                                                   const type::CurrencyCode& paymentCurrency)
  : BusinessRuleApplicator(rule),
    _services(services),
    _taxMinMaxValueRule(rule),
    _paymentCurrency(paymentCurrency)
{
}

TaxMinMaxValueApplicator::~TaxMinMaxValueApplicator()
{
}

bool
TaxMinMaxValueApplicator::apply(PaymentDetail& paymentDetail) const
{
  type::MoneyAmount taxValMinConverted = 0;
  type::MoneyAmount taxValMaxConverted = 0;

  try
  {
    type::Money money;
    money._currency = _taxMinMaxValueRule->taxValCurrency();

    money._amount = _taxMinMaxValueRule->taxValMin();
    taxValMinConverted = _services.currencyService().convertTo(_paymentCurrency, money);

    money._amount = _taxMinMaxValueRule->taxValMax();
    taxValMaxConverted = _services.currencyService().convertTo(_paymentCurrency, money);
  }
  catch (const std::runtime_error& exception)
  {
    std::ostringstream msg;
    msg << "CONVERSION " << _taxMinMaxValueRule->taxValCurrency() << "-" << _paymentCurrency
        << " FAILED";

    paymentDetail.applicatorFailMessage() = msg.str();
    return false;
  }

  paymentDetail.exchangeDetails().minTaxAmount = _taxMinMaxValueRule->taxValMin();
  paymentDetail.exchangeDetails().maxTaxAmount = _taxMinMaxValueRule->taxValMax();
  paymentDetail.exchangeDetails().minMaxTaxCurrency = _taxMinMaxValueRule->taxValCurrency();
  paymentDetail.exchangeDetails().minMaxTaxCurrencyDecimals =
      _taxMinMaxValueRule->taxValCurrencyDecimals();

  if (_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    type::MoneyAmount& taxEquivalentAmount = paymentDetail.taxEquivalentAmount();
    if ((taxValMinConverted != 0) && (taxEquivalentAmount < taxValMinConverted))
    {
      taxEquivalentAmount = taxValMinConverted;
    }
    if ((taxValMaxConverted != 0) && (taxValMaxConverted < taxEquivalentAmount))
    {
      taxEquivalentAmount = taxValMaxConverted;
    }
  }
  else
  {
    MathUtils::clamp(taxValMinConverted, taxValMaxConverted, paymentDetail.taxEquivalentAmount());
    MathUtils::clamp(taxValMinConverted, taxValMaxConverted, paymentDetail.taxEquivalentWithMarkupAmount());
  }

  return true;
}

} // namespace tax
