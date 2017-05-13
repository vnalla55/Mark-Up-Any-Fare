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

#include "Rules/BusinessRule.h"
#include "DataModel/Common/Types.h"

namespace tax
{
class Request;
class ThirdPartyTagApplicator;

class ThirdPartyTagRule : public BusinessRule
{
public:
  typedef ThirdPartyTagApplicator ApplicatorType;
  ThirdPartyTagRule(const type::PaidBy3rdPartyTag& paidBy3rdPartyTag);
  virtual ~ThirdPartyTagRule();

  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& request,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

private:
  type::PaidBy3rdPartyTag _paidBy3rdPartyTag;
};
}
