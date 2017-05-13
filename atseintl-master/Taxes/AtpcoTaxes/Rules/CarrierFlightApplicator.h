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
#include "Rules/BusinessRuleApplicator.h"

#include <memory>

namespace tax
{

class CarrierFlightRule;
class PaymentDetail;
class Flight;
class CarrierFlight;
class GeoPath;
class FlightUsage;

class CarrierFlightApplicator : public BusinessRuleApplicator
{
public:
  CarrierFlightApplicator(const CarrierFlightRule& rule,
                          const GeoPath& geoPath,
                          const std::vector<FlightUsage>& flightUsages,
                          std::shared_ptr<const CarrierFlight> carrierFlightBefore,
                          std::shared_ptr<const CarrierFlight> carrierFlightAfter);
  ~CarrierFlightApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

  bool isFlightNumberInRange(type::FlightNumber flightNumber,
                             type::FlightNumber flightFrom,
                             type::FlightNumber flightTo) const;
  bool isFlightMatchingConditions(const Flight* flight,
                                  std::shared_ptr<const CarrierFlight> carrierFlight) const;

protected:
  bool matchDirection(type::Index loc1Id, bool beforeTable, bool departureTag) const;
  bool matches(type::Index loc1Id, bool departureTag) const;

private:
  const GeoPath& _geoPath;
  const std::vector<FlightUsage>& _flightUsages;
  std::shared_ptr<const CarrierFlight> _carrierFlightBefore;
  std::shared_ptr<const CarrierFlight> _carrierFlightAfter;

  const CarrierFlightRule& _carrierFlightRule;
};

} // namespace tax
