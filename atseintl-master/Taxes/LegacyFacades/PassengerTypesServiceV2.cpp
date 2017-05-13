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
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/PassengerTypesServiceV2.h"

#include <boost/bind.hpp>

namespace tse
{
namespace
{
typedef PassengerTypesServiceV2::Key Key;
typedef PassengerTypesServiceV2::SharedConstValue SharedConstValue;

SharedConstValue
readData(const DateTime& ticketingDate, const Key& key)
{
  DataHandle dataHandle(ticketingDate);
  std::unique_ptr<tax::PassengerTypeCodeItems> result(new tax::PassengerTypeCodeItems());

  const std::vector<const PaxTypeCodeInfo*> tseCodeInfos =
      dataHandle.getPaxTypeCode(toTseVendorCode(key.first), static_cast<int>(key.second));

  result->resize(tseCodeInfos.size());
  for (const PaxTypeCodeInfo* info : tseCodeInfos)
  {
    DaoDataFormatConverter::convert(*info, (*result)[info->seqNo() - 1]);
  }

  return SharedConstValue(std::move(result));
}
}

PassengerTypesServiceV2::PassengerTypesServiceV2(const DateTime& ticketingDate)
  : _cache(boost::bind(&readData, ticketingDate, _1))
{
}

std::shared_ptr<const tax::PassengerTypeCodeItems>
PassengerTypesServiceV2::getPassengerTypeCode(const tax::type::Vendor& vendor,
                                              const tax::type::Index& itemNo) const
{
  return _cache.get(std::make_pair(vendor, itemNo));
}

} // namespace tse
