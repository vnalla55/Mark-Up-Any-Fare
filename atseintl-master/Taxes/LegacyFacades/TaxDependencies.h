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
#pragma once
#include "AtpcoTaxes/Common/Timestamp.h"
#include <string>
#include <vector>

namespace tax
{
  class IoTaxName;
  class OutputTaxDetails;
  class ServiceBaggageService;
  class RulesRecordsService;
}

namespace tse
{

class TaxDependencies
{
  const tax::RulesRecordsService& _x1Service;
  const tax::ServiceBaggageService& _svcBagService;
  tax::type::Timestamp _ticketingDate;

public:
  explicit TaxDependencies(const tax::RulesRecordsService& x1, const tax::ServiceBaggageService& sb,
                           const tax::type::Timestamp& ticketingDate)
    : _x1Service(x1), _svcBagService(sb), _ticketingDate(ticketingDate) {}

  std::vector<std::string>
  taxDependencies(const tax::OutputTaxDetails& taxDetail) const;
  // Given the tax ID 'T', return the list of all non-OC taxes that 'T' may be applied to as
  // tax-on-tax. This data is read from ServiceAndBaggage table. The result may contain 2 or three-
  // letter tax codes (as defined by sabeTaxCode) or two letters with asterisk, like 'US*' indicating
  // that any tax with tax code US is a dependency.
};

} // namespace tse

