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
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxOnTicketingFeeApplicator;

class TaxOnTicketingFeeRule : public BusinessRule
{
public:
  typedef TaxOnTicketingFeeApplicator ApplicatorType;
  TaxOnTicketingFeeRule(const type::PercentFlatTag& percentFlatTag);

  std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const type::PercentFlatTag& percentFlatTag() const { return _percentFlatTag; }
  bool isPercent() const;

private:
  type::PercentFlatTag _percentFlatTag;
};
}
