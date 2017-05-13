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
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/MileageService.h"

#include <set>

namespace tax
{

class Itin;
class GeoPath;
class FlightUsage;
class Flight;
class ConnectionsTagsRule;

class ConnectionsTagsApplicator : public BusinessRuleApplicator
{
  typedef bool StopoverChecker(const Geo& arrival, const Geo& departure);
  typedef bool FlightStopoverChecker(const Flight* arrival, const Flight* departure);

public:
  ConnectionsTagsApplicator(const ConnectionsTagsRule& rule,
                            const Itin& itin,
                            const type::Timestamp travelDate,
                            const MileageService& mileageService);

  ~ConnectionsTagsApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const ConnectionsTagsRule& _rule;
  const std::set<type::ConnectionsTag>& _connectionsTagsSet;
  const Itin& _itin;
  const GeoPath& _geoPath;
  const std::vector<FlightUsage>& _flightUsages;
  const type::Timestamp _travelDate;
  const MileageService& _mileageService;

  void applyGroundTransportAsStopover(TaxPointsProperties& properties) const;
  void applyDifferentMarketingCarrierAsStopover(TaxPointsProperties& properties) const;

  template <FlightStopoverChecker checker>
  bool isFlightStopover(const Geo& arrivalGeo, const Geo& departureGeo) const;

  void applyTurnaroundPointForConnectionAsStopover(PaymentDetail& paymentDetail) const;
  void applyTurnaroundPointAsStopover(PaymentDetail& paymentDetail) const;
  void applyFareBreakAsStopover(TaxPointsProperties& taxPointsProperties) const;
  void applyFarthestFareBreakAsStopover(TaxPointsProperties& properties) const;
  const Geo* getFarthestFareBreak(const TaxPointsProperties& properties) const;
  const Geo* getFarthestFareBreakOrStopoverExceptTurnaround(PaymentDetail& paymentDetail,
                                                            const type::CityCode& turnaroundCityCode) const;
  bool doubleOccurrence(type::Index geoId,
                        TaxPointsProperties& properties) const;
};

} // namespace tax

