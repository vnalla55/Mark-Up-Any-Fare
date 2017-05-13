// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include <set>

namespace tax
{
class Services;
class BusinessRule;
class PaymentDetail;

class PreviousTicketApplicator : public BusinessRuleApplicator
{
  const Services& _services;
  const type::Percent& _percent;

  std::set<std::string> getParentTaxes(const TaxName& taxName) const;

  bool isInTaxesForPreviousTicket(const std::set<std::string>& parentTaxes) const;

public:
  PreviousTicketApplicator(const BusinessRule* parent,
                           const Services& services,
                           const type::Percent& percent)
    : BusinessRuleApplicator(parent), _services(services), _percent(percent)
  {
  }

  bool apply(PaymentDetail& paymentDetail) const;
};
}
