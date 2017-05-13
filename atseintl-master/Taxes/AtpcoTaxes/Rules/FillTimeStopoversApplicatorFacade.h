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
#pragma once

#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class FillTimeStopoversRule;
class Itin;
class PaymentDetail;

class FillTimeStopoversApplicatorFacade : public BusinessRuleApplicator
{
public:
  FillTimeStopoversApplicatorFacade(const FillTimeStopoversRule*, const Itin&);

  bool apply(PaymentDetail& paymentDetail) const;
private:
  const FillTimeStopoversRule* _fillTimeStopoversRule;
  const Itin* _itin;
};
}

