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

namespace tax
{
class Request;
class SpecialTaxProcessingApplicator;

class SpecialTaxProcessingRule : public BusinessRule
{
public:
  typedef SpecialTaxProcessingApplicator ApplicatorType;
  SpecialTaxProcessingRule(const type::TaxProcessingApplTag& applicationTag);
  virtual ~SpecialTaxProcessingRule();
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& request,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;
  virtual std::string getDescription(Services& services) const override;

private:
  type::TaxProcessingApplTag _applicationTag;
};

} /* namespace tax */

