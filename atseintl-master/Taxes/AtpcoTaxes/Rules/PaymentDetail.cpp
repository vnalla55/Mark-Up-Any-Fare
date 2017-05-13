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

#include "Rules/PaymentDetail.h"
#include "Rules/LimitGroup.h"

namespace tax
{

type::TaxApplicationLimit
PaymentDetail::getLimitType() const
{
  return _limitGroup->_limitType;
}

const BusinessRule*
PaymentDetail::getLimitRule() const
{
  return &_limitGroup->getLimitRule();
}

void
PaymentDetail::setCommandExempt(type::CalcRestriction exemptType)
{
  _commandExempt = _isSkipExempt ? type::CalcRestriction::Blank : exemptType;
}
}

