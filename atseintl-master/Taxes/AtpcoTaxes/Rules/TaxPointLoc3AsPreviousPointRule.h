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
class LocZone;
class Request;
class TaxPointLoc3AsPreviousPointApplicator;

class TaxPointLoc3AsPreviousPointRule : public BusinessRule
{
public:
  typedef TaxPointLoc3AsPreviousPointApplicator ApplicatorType;
  TaxPointLoc3AsPreviousPointRule(const LocZone& locZone, const type::Vendor& vendor);
  virtual ~TaxPointLoc3AsPreviousPointRule();

  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
