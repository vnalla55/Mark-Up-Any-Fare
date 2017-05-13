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

#include <boost/bind.hpp>

#include "DBAccess/DataHandle.h"
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/ServiceBaggageInfo.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/ServiceBaggageServiceV2.h"

namespace tse
{
namespace
{
typedef ServiceBaggageServiceV2::Key Key;
typedef ServiceBaggageServiceV2::SharedConstValue SharedConstValue;

SharedConstValue
readData(const DateTime& ticketingDate, const Key& key)
{
  DataHandle dataHandle(ticketingDate);
  std::unique_ptr<tax::ServiceBaggage> result(new tax::ServiceBaggage());

  result->vendor = key.first;
  result->itemNo = key.second;

  const std::vector<const ServiceBaggageInfo*> tseServiceBaggageInfos =
      dataHandle.getServiceBaggage(toTseVendorCode(key.first), static_cast<int>(key.second));

  result->entries.resize(tseServiceBaggageInfos.size());
  for (const ServiceBaggageInfo* info : tseServiceBaggageInfos)
  {
    DaoDataFormatConverter::convert(*info, result->entries[info->seqNo() - 1]);
  }

  return SharedConstValue(std::move(result));
}
}

ServiceBaggageServiceV2::ServiceBaggageServiceV2(const DateTime& ticketingDate)
  : _cache(boost::bind(&readData, ticketingDate, _1))
{
}

std::shared_ptr<const tax::ServiceBaggage>
ServiceBaggageServiceV2::getServiceBaggage(const tax::type::Vendor& vendor,
                                           const tax::type::Index& itemNo) const
{
  return _cache.get(std::make_pair(vendor, itemNo));
}

} // namespace tax
