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

#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/LocService.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TravelWhollyWithinRule.h"
#include "Rules/TravelWhollyWithinApplicator.h"

namespace tax
{

TravelWhollyWithinApplicator::TravelWhollyWithinApplicator(TravelWhollyWithinRule const& rule,
                                                           GeoPath const& geoPath,
                                                           LocService const& locService)
  : BusinessRuleApplicator(&rule),
    _travelWhollyWithinRule(rule),
    _geoPath(geoPath),
    _locService(locService)
{
}

TravelWhollyWithinApplicator::~TravelWhollyWithinApplicator() {}

bool
TravelWhollyWithinApplicator::apply(PaymentDetail& /* paymentDetail */) const
{
  bool ret(false);

  for(const Geo & geo : _geoPath.geos())
  {
    if ((_travelWhollyWithinRule.getTicketedPointTag() ==
         type::TicketedPointTag::MatchTicketedPointsOnly) &&
        (geo.unticketedTransfer() == type::UnticketedTransfer::Yes))
      continue;
    else
      ret = _locService.isInLoc(geo.loc().code(),
                              _travelWhollyWithinRule.getLocZone(),
                              _travelWhollyWithinRule.getVendor());

    if (!ret)
      break;
  }

  return ret;
}
}
