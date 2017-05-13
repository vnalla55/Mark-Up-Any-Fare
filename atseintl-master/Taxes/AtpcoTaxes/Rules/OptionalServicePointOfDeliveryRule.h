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

#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{

class OptionalServicePointOfDeliveryApplicator;
class Request;
class Services;

class OptionalServicePointOfDeliveryRule : public BusinessRule
{
public:
  typedef OptionalServicePointOfDeliveryApplicator ApplicatorType;
  OptionalServicePointOfDeliveryRule(LocZone const& locZone, const type::Vendor& vendor);

  virtual ~OptionalServicePointOfDeliveryRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  LocZone const& getLocZone() const { return _locZone; }

  const type::Vendor& getVendor() const { return _vendor; }

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
