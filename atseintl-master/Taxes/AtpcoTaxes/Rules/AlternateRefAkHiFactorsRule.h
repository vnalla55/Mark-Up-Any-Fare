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

#include "Rules/BusinessRule.h"

namespace tax
{
class AlternateRefAkHiFactorsApplicator;
class Request;
class Services;

class AlternateRefAkHiFactorsRule : public BusinessRule
{
public:
  typedef AlternateRefAkHiFactorsApplicator ApplicatorType;
  AlternateRefAkHiFactorsRule();
  virtual ~AlternateRefAkHiFactorsRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;
};
}
