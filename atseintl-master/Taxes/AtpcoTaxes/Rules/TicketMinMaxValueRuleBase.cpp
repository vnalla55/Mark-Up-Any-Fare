// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/CodeIO.h"
#include "Rules/MathUtils.h"
#include "Rules/TicketMinMaxValueRuleBase.h"

namespace tax
{
class Services;

TicketMinMaxValueRuleBase::TicketMinMaxValueRuleBase(
    const type::TktValApplQualifier& tktValApplQualifier,
    const type::CurrencyCode& tktValCurrency,
    const type::IntMoneyAmount tktValMin,
    const type::IntMoneyAmount tktValMax,
    const type::CurDecimals tktValCurrDecimals)
  : _tktValApplQualifier(tktValApplQualifier),
    _tktValCurrency(tktValCurrency),
    _tktValMin(MathUtils::adjustDecimal(tktValMin, tktValCurrDecimals)),
    _tktValMax(MathUtils::adjustDecimal(tktValMax, tktValCurrDecimals))
{
}

TicketMinMaxValueRuleBase::~TicketMinMaxValueRuleBase()
{
}

std::string
TicketMinMaxValueRuleBase::getDescription(Services&) const
{
  std::ostringstream buf;
  buf << "APPLY TAX IF TICKET VALUE IS ";
  if (_tktValMin && _tktValMax == 0) // no max limit
  {
    buf << "ABOVE " << amountToDouble(_tktValMin) << " " << _tktValCurrency;
  }
  else // max limit present
  {
    buf << "BETWEEN " << amountToDouble(_tktValMin) << " AND " << amountToDouble(_tktValMax) << " "
        << _tktValCurrency;
  }

  return buf.str();
}
}
