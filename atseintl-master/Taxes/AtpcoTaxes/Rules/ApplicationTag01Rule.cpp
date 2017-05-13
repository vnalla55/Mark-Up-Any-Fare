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
#include "Rules/ApplicationTag01Rule.h"
#include "Rules/ApplicationTag01Applicator.h"
#include "ServiceInterfaces/Services.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

ApplicationTag01Rule::ApplicationTag01Rule(type::Vendor const& vendor) : _vendor(vendor) {}

ApplicationTag01Rule::~ApplicationTag01Rule() {}

ApplicationTag01Rule::ApplicatorType
ApplicationTag01Rule::createApplicator(const type::Index& itinIndex,
                                     const Request& request,
                                     Services& services,
                                     RawPayments& /*itinPayments*/) const
{
  type::Timestamp ticketingDate(request.ticketingOptions().ticketingDate(),
                                request.ticketingOptions().ticketingTime());
  const Itin& itin(request.getItinByIndex(itinIndex));

  return ApplicatorType(*this,
                        itin,
                        services.locService(),
                        services.mileageService(),
                        ticketingDate);
}

std::string
ApplicationTag01Rule::getDescription(Services&) const
{
  return std::string("TAX PROCESSING APPLICATION TAG 01 RULE");
}

} // namespace tax
