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
#include "Common/MoneyUtil.h"
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "Rules/MathUtils.h"
#include "Rules/TaxMinMaxValueApplicator.h"

namespace tax
{
TaxMinMaxValueRule::TaxMinMaxValueRule(const type::CurrencyCode& taxValCurrency,
                                       const type::IntMoneyAmount taxValMin,
                                       const type::IntMoneyAmount taxValMax,
                                       const type::CurDecimals& taxValCurrDecimals)
  : _taxValCurrency(taxValCurrency),
    _taxValCurrencyDecimals(taxValCurrDecimals),
    _taxValMin(MathUtils::adjustDecimal(taxValMin, taxValCurrDecimals)),
    _taxValMax(MathUtils::adjustDecimal(taxValMax, taxValCurrDecimals))
{
}

TaxMinMaxValueRule::~TaxMinMaxValueRule()
{
}

TaxMinMaxValueRule::ApplicatorType
TaxMinMaxValueRule::createApplicator(type::Index const& /*itinIndex*/,
                                     const Request& request,
                                     Services& services,
                                     RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(
      this, services, request.ticketingOptions().paymentCurrency());
}

std::string
TaxMinMaxValueRule::getDescription(Services&) const
{
  std::ostringstream buf;
  buf << "LIMIT TAX VALUE TO RANGE " << amountToDouble(_taxValMin) << "-";
  if (_taxValMax != 0) // max limit
  {
    buf << amountToDouble(_taxValMax);
  }
  else // no max limit
  {
    buf << "UNLIMITED";
  }
  buf << " " << _taxValCurrency;

  return buf.str();
}
}
