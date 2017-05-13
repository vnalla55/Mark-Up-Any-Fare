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
class CarrierFlightApplicator;
class Request;

class CarrierFlightRule : public BusinessRule
{
public:
  typedef CarrierFlightApplicator ApplicatorType;
  CarrierFlightRule(type::Index const& carrierFlightItemBefore,
                    type::Index const& carrierFlightItemAfter,
                    type::TaxPointTag const& taxPointTag,
                    type::Vendor const& vendor);
  virtual ~CarrierFlightRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::Index const& carrierFlightItemBefore() const { return _carrierFlightItemBefore; }
  type::Index const& carrierFlightItemAfter() const { return _carrierFlightItemAfter; }
  type::TaxPointTag const& taxPointTag() const { return _taxPointTag; }
  type::Vendor vendor() const { return _vendor; }

private:
  void addItemDescription(std::ostringstream& buf, type::Index item, Services& services) const;

  type::Index _carrierFlightItemBefore;
  type::Index _carrierFlightItemAfter;
  type::TaxPointTag _taxPointTag;
  type::Vendor _vendor;
};
}
