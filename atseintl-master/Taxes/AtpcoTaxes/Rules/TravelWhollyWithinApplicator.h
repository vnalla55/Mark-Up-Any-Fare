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
#include "Common/LocZone.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class TravelWhollyWithinRule;
class PaymentDetail;
class GeoPath;
class LocService;

class TravelWhollyWithinApplicator : public BusinessRuleApplicator
{
public:
  TravelWhollyWithinApplicator(TravelWhollyWithinRule const& rule,
                               GeoPath const& geoPath,
                               LocService const& locService);
  ~TravelWhollyWithinApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  TravelWhollyWithinRule const& _travelWhollyWithinRule;

  GeoPath const& _geoPath;
  LocService const& _locService;
};

} // namespace tax
