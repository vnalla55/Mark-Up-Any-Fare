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
class JourneyIncludesApplicator;
class Request;
class Services;

class JourneyIncludesRule : public BusinessRule
{
public:
  typedef JourneyIncludesApplicator ApplicatorType;
  JourneyIncludesRule(type::TicketedPointTag const& ticketedPoint,
                      LocZone const& zone,
                      type::Vendor const& vendor,
                      bool mustBeStopAndNotOriginDestination);
  virtual ~JourneyIncludesRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::TicketedPointTag const& getTicketedPointTag() const { return _ticketedPoint; }
  LocZone const& getLocZone() const { return _locZone; }
  type::Vendor const& getVendor() const { return _vendor; }
  bool getMustBeStopAndNotOriginDestination() const { return _mustBeStopAndNotOriginDestination; }

private:
  type::TicketedPointTag _ticketedPoint;
  LocZone _locZone;
  type::Vendor _vendor;
  bool _mustBeStopAndNotOriginDestination;
};
}
