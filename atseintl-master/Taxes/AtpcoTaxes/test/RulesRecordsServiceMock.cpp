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

#include "test/RulesRecordsServiceMock.h"

namespace tax
{

std::shared_ptr<const std::vector<RulesRecordsServiceMock::SharedTaxRulesRecord>>
RulesRecordsServiceMock::getTaxRecords(const type::TaxCode& taxCode,
                                       const type::Timestamp& /* ticketingDate */) const
{
  auto found = _taxRecords.find(taxCode);
  if (found != _taxRecords.end())
    return found->second;

  return std::shared_ptr<const std::vector<SharedTaxRulesRecord>>();
}

} /* namespace tax */
