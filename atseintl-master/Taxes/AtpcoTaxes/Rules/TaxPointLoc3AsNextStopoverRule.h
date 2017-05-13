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
class TaxPointLoc3AsNextStopoverApplicator;

class TaxPointLoc3AsNextStopoverRule : public BusinessRule
{
public:
  typedef TaxPointLoc3AsNextStopoverApplicator ApplicatorType;
  TaxPointLoc3AsNextStopoverRule(const LocZone& locZone3,
                                 type::Vendor vendor);

  virtual ~TaxPointLoc3AsNextStopoverRule();

  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

  const LocZone& locZone3() const { return _locZone3; }
  const type::Vendor vendor() const { return _vendor; }

private:
  LocZone _locZone3;
  type::Vendor _vendor;
};
}
