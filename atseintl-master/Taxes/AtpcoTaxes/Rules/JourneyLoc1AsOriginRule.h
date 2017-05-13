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
class JourneyLoc1AsOriginApplicator;
class Request;
class Services;

class JourneyLoc1AsOriginRule : public BusinessRule
{
public:
  typedef JourneyLoc1AsOriginApplicator ApplicatorType;

  JourneyLoc1AsOriginRule(LocZone const& zone, type::Vendor const& vendor);

  virtual std::string getDescription(Services& services) const override;

  //virtual for JourneyLoc1AsOriginExRule
  virtual ApplicatorType createApplicator(type::Index const& itinIndex,
                                          const Request& request,
                                          Services& services,
                                          RawPayments& /*itinPayments*/) const;

  LocZone const& getLocZone() const { return _locZone; }
  type::Vendor const& getVendor() const { return _vendor; }

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
