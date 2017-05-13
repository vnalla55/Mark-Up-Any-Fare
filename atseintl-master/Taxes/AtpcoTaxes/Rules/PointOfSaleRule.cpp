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

#include "Rules/PointOfSaleRule.h"
#include "Rules/PointOfSaleApplicator.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

PointOfSaleRule::PointOfSaleRule(const LocZone& locZone, const type::Vendor& vendor)
  : _locZone(locZone), _vendor(vendor)
{
}

PointOfSaleRule::~PointOfSaleRule() {}

PointOfSaleRule::ApplicatorType
PointOfSaleRule::createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, getPointOfSaleLoc(request, itinIndex), services.locService());
}

const type::AirportCode&
PointOfSaleRule::getPointOfSaleLoc(const Request& request,
                                   const type::Index& itinIndex) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  return request.pointsOfSale()[itin.pointOfSaleRefId()].loc();
}

std::string
PointOfSaleRule::getDescription(Services&) const
{
  return std::string("POINT OF SALE RESTRICTED TO ") + _locZone.toString();
}
}
