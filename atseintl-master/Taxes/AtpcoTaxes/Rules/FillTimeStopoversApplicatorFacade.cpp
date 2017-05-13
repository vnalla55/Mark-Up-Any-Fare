// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/FillTimeStopoversApplicatorFacade.h"

#include "Rules/DummyApplicator.h"
#include "Rules/FillTimeStopoversApplicator.h"
#include "Rules/FillTimeStopoversRule.h"

namespace tax
{
FillTimeStopoversApplicatorFacade::FillTimeStopoversApplicatorFacade(
    const FillTimeStopoversRule* rule, const Itin& itin)
  : BusinessRuleApplicator(rule), _fillTimeStopoversRule(rule), _itin(&itin)
{
}

bool
FillTimeStopoversApplicatorFacade::apply(PaymentDetail& paymentDetail) const
{
  try
  {
    return FillTimeStopoversApplicator(_fillTimeStopoversRule, *_itin).apply(paymentDetail);
  }
  catch (const std::range_error& error)
  {
    return DummyApplicator(*_fillTimeStopoversRule, false, error.what()).apply(paymentDetail);
  }
}
}

