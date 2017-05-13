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

#include "DBAccess/DataHandle.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/ServiceFeeSecurityServiceV2.h"

#include <boost/bind.hpp>

namespace tse
{
namespace
{
typedef ServiceFeeSecurityServiceV2::Key Key;
typedef ServiceFeeSecurityServiceV2::Value Value;
typedef ServiceFeeSecurityServiceV2::SharedConstValue SharedConstValue;

SharedConstValue
readData(const DateTime& ticketingDate, const Key& key)
{
  DataHandle dataHandle(ticketingDate);
  std::unique_ptr<Value> result(new Value());

  const std::vector<SvcFeesSecurityInfo*>& tseSfsInfos =
      dataHandle.getSvcFeesSecurity(toTseVendorCode(key.first), static_cast<int>(key.second));

  result->resize(tseSfsInfos.size());
  for (const SvcFeesSecurityInfo* info : tseSfsInfos)
  {
    DaoDataFormatConverter::convert(*info, (*result)[info->seqNo() - 1]);
  }

  return SharedConstValue(std::move(result));
}
}

ServiceFeeSecurityServiceV2::ServiceFeeSecurityServiceV2(const DateTime& ticketingDate)
  : _cache(boost::bind(&readData, ticketingDate, _1))
{
}

std::shared_ptr<const tax::ServiceFeeSecurityItems>
ServiceFeeSecurityServiceV2::getServiceFeeSecurity(const tax::type::Vendor& vendor,
                                                   const tax::type::Index& itemNo) const
{
  return _cache.get(std::make_pair(vendor, itemNo));
}

} // namespace tse
