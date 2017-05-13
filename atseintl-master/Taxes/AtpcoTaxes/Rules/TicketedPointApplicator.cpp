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
#include "Rules/TicketedPointApplicator.h"

#include "Rules/PaymentDetail.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include <algorithm>

namespace tax
{

TicketedPointApplicator::TicketedPointApplicator(BusinessRule const* parent, GeoPath const& geoPath)
  : BusinessRuleApplicator(parent), _geoPath(geoPath)
{
}

TicketedPointApplicator::~TicketedPointApplicator() {}

namespace
{
bool
isTicketedArrival(Geo const& geo)
{
  return geo.loc().tag() == type::TaxPointTag::Arrival &&
         geo.unticketedTransfer() == type::UnticketedTransfer::No;
}

bool
isTicketedDeparture(Geo const& geo)
{
  return geo.loc().tag() == type::TaxPointTag::Departure &&
         geo.unticketedTransfer() == type::UnticketedTransfer::No;
}
}

bool
TicketedPointApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (paymentDetail.unticketedTransfer() == type::UnticketedTransfer::Yes)
    return false;

  std::vector<Geo>::const_iterator currentLoc2 =
      _geoPath.geos().begin() + paymentDetail.getTaxPointEnd().id();

  if (currentLoc2->unticketedTransfer() == type::UnticketedTransfer::No)
    return true;

  std::vector<Geo>::const_iterator newLoc2;

  if (currentLoc2->loc().tag() == type::TaxPointTag::Arrival)
  {
    newLoc2 = std::find_if(currentLoc2 + 1, _geoPath.geos().end(), isTicketedArrival);
    if (newLoc2 == _geoPath.geos().end())
      return false;
  }
  else if (currentLoc2->loc().tag() == type::TaxPointTag::Departure)
  {
    std::vector<Geo>::const_reverse_iterator rCurrentLoc2(currentLoc2);
    newLoc2 = std::find_if(rCurrentLoc2, _geoPath.geos().rend(), isTicketedDeparture).base();
    if (newLoc2 == _geoPath.geos().begin())
      return false;
    --newLoc2;
  }

  Geo const& geo2 = _geoPath.geos()[newLoc2 - _geoPath.geos().begin()];
  paymentDetail.setTaxPointLoc2(&geo2);
  paymentDetail.setTaxPointEnd(geo2);

  return true;
}
}
