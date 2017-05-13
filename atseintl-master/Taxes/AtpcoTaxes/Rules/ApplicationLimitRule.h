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

#include "Rules/BusinessRule.h"

namespace tax
{
class ApplicationLimitApplicator;
class Request;

class ApplicationLimitRule : public BusinessRule
{
public:
  typedef ApplicationLimitApplicator ApplicatorType;
  ApplicationLimitRule(type::TaxApplicationLimit taxApplicationLimit);
  ~ApplicationLimitRule() override {}
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;
  std::string getDescription(Services& services) const override;
  type::TaxApplicationLimit taxApplicationLimit() const { return _taxApplicationLimit; }

private:
  type::TaxApplicationLimit _taxApplicationLimit;
};

} /* namespace tax */

