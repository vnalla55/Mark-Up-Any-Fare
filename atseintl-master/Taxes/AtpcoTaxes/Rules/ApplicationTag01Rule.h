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

#include "Common/LocZone.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class ApplicationTag01Applicator;
class Request;
class Services;

class ApplicationTag01Rule : public BusinessRule
{
public:
  typedef ApplicationTag01Applicator ApplicatorType;
  ApplicationTag01Rule(const type::Vendor& vendor);
  virtual ~ApplicationTag01Rule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const type::Vendor& getVendor() const { return _vendor; }

private:
  type::Vendor _vendor;
};
}
