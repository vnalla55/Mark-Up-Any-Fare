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

#include "DataModel/Common/Types.h"
#include "BusinessRule.h"

namespace tax
{
class Request;
class SingleJourneyLimitApplicator;

class SingleJourneyLimitRule : public BusinessRule
{
public:
  typedef SingleJourneyLimitApplicator ApplicatorType;
  SingleJourneyLimitRule();
  virtual ~SingleJourneyLimitRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& rawPayments) const;

private:
};
}