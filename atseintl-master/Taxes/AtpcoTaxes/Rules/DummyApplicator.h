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
#pragma once

#include "Rules/BusinessRuleApplicator.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

class DummyApplicator : public BusinessRuleApplicator
{
public:
  DummyApplicator(const BusinessRule& rule, bool returnValue, std::string failMessage = "")
    : BusinessRuleApplicator(&rule), _returnValue(returnValue), _failMessage(failMessage)
  {
  }

  bool apply(PaymentDetail& paymentDetail) const
  {
    if (!_returnValue)
      paymentDetail.applicatorFailMessage() = _failMessage;
    return _returnValue;
  }

private:
  bool _returnValue;
  std::string _failMessage;
};

} // namespace tax
