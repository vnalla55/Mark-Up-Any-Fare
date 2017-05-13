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
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/ServiceBaggageApplicator.h"
#include "Rules/ServiceBaggageRule.h"

namespace tax
{

ServiceBaggageRule::ServiceBaggageRule(type::Index const& itemNo, type::Vendor const& vendor)
  : _itemNo(itemNo), _vendor(vendor)
{
}

ServiceBaggageRule::ApplicatorType
ServiceBaggageRule::createApplicator(type::Index const& itinIndex,
                                     const Request& request,
                                     Services& services,
                                     RawPayments& rawPayments) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  const YqYrPath* yqYrPath = itin.yqYrPath();
  assert ((yqYrPath != nullptr) == itin.yqYrPathGeoPathMappingRefId().has_value());

  GeoPathMapping const* yqYrGeoPathMapping = nullptr;
  if (itin.yqYrPathGeoPathMappingRefId().has_value())
    yqYrGeoPathMapping = &request.geoPathMappings()[itin.yqYrPathGeoPathMappingRefId().value()];

  std::shared_ptr<ServiceBaggage const> serviceBaggage;
  assert(_itemNo != 0);
  serviceBaggage = services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo);

  return ApplicatorType(this,
                        &request.yqYrs(),
                        yqYrPath,
                        yqYrGeoPathMapping,
                        serviceBaggage,
                        rawPayments);
}

std::string
ServiceBaggageRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "TAX MUST SATISFY RESTRICTIONS FROM SERVICE BAGGAGE TABLE\n"
      << " ITEM " << _itemNo << " FROM VENDOR " << _vendor << "\n"
      << " ITEM " << _itemNo;

  RuleDescriptionFormater::format(buf,
                                  services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo));

  return buf.str();
}
}
