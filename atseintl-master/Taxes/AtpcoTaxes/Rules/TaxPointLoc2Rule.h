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
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class LocZone;
class Request;
class TaxPointLoc2Applicator;

class TaxPointLoc2Rule : public BusinessRule
{
public:
  typedef TaxPointLoc2Applicator ApplicatorType;
  TaxPointLoc2Rule(const LocZone& locZone,
                   const type::Vendor& vendor);
  virtual ~TaxPointLoc2Rule();

  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

  const LocZone& getLocZone() const { return _locZone; }
  const type::Vendor& getVendor() const { return _vendor; }

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
