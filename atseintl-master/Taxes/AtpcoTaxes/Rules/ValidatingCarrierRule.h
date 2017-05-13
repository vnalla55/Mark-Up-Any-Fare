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
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class ValidatingCarrierApplicator;

class ValidatingCarrierRule : public BusinessRule
{
public:
  typedef ValidatingCarrierApplicator ApplicatorType;
  ValidatingCarrierRule(type::Index const& carrierAppl, type::Vendor const& vendor);
  virtual ~ValidatingCarrierRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::Index carrierAppl() const { return _carrierAppl; }
  type::Vendor vendor() const { return _vendor; }

private:
  type::Index _carrierAppl;
  type::Vendor _vendor;
};
}
