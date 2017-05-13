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
#include <boost/lexical_cast.hpp>
#include "ServiceInterfaces/Services.h"
#include "Rules/OptionalServicePointOfDeliveryRule.h"
#include "Rules/OptionalServicePointOfDeliveryApplicator.h"
#include "DomainDataObjects/Request.h"
#include <limits>

namespace tax
{

OptionalServicePointOfDeliveryRule::OptionalServicePointOfDeliveryRule(LocZone const& locZone,
                                                                       const type::Vendor& vendor)
  : _locZone(locZone), _vendor(vendor)
{
}

OptionalServicePointOfDeliveryRule::~OptionalServicePointOfDeliveryRule() {}

OptionalServicePointOfDeliveryRule::ApplicatorType
OptionalServicePointOfDeliveryRule::createApplicator(type::Index const& /*itinIndex*/,
                                                     const Request& /*request*/,
                                                     Services& services,
                                                     RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services.locService());
}

std::string
OptionalServicePointOfDeliveryRule::getDescription(Services&) const
{
  return std::string("POINT OF DELIVERY RESTRICTED TO ") + _locZone.toString();
}
}
