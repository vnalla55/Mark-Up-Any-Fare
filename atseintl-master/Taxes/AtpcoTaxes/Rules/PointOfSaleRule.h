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
class PointOfSaleApplicator;
class Request;
class Services;

class PointOfSaleRule : public BusinessRule
{
public:
  typedef PointOfSaleApplicator ApplicatorType;
  PointOfSaleRule(const LocZone& locZone, const type::Vendor& vendor);

  virtual ~PointOfSaleRule();

  virtual std::string getDescription(Services& services) const override;

  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const LocZone& getLocZone() const { return _locZone; }
  const type::Vendor& getVendor() const { return _vendor; }

  const type::AirportCode& getPointOfSaleLoc(const Request& request,
                                             const type::Index& itinIndex) const;

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
