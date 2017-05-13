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

class Request;
class Services;
class TravelWhollyWithinApplicator;

class TravelWhollyWithinRule : public BusinessRule
{
public:
  typedef TravelWhollyWithinApplicator ApplicatorType;
  TravelWhollyWithinRule(type::TicketedPointTag const& ticketedPoint,
                         LocZone const& zone,
                         type::Vendor const& vendor);
  virtual ~TravelWhollyWithinRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::TicketedPointTag const& getTicketedPointTag() const { return _ticketedPoint; }
  LocZone const& getLocZone() const { return _locZone; }
  type::Vendor const& getVendor() const { return _vendor; }

private:
  type::TicketedPointTag _ticketedPoint;
  LocZone _locZone;
  type::Vendor _vendor;
};
}
