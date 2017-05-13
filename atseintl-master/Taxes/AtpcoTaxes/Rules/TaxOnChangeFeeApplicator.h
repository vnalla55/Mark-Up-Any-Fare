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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class PaymentDetail;
class Services;
class TaxOnChangeFeeRule;

class TaxOnChangeFeeApplicator : public BusinessRuleApplicator
{
public:
  TaxOnChangeFeeApplicator(TaxOnChangeFeeRule const& parent,
                           const Services& services);

  ~TaxOnChangeFeeApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const TaxOnChangeFeeRule& _taxOnChangeFeeRule;
  const Services& _services;
};
}
