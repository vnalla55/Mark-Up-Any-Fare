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
#include "DBAccess/SectorDetailInfo.h"
#include "DataModel/Services/SectorDetail.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/SectorDetailServiceV2.h"

#include <boost/bind.hpp>

namespace tse
{
namespace
{
typedef SectorDetailServiceV2::Key Key;
typedef SectorDetailServiceV2::SharedConstValue SharedConstValue;

SharedConstValue
readData(const DateTime& ticketingDate, const Key& key)
{
  DataHandle dataHandle(ticketingDate);
  std::unique_ptr<tax::SectorDetail> result(new tax::SectorDetail());

  result->vendor = key.first;
  result->itemNo = key.second;

  const std::vector<const SectorDetailInfo*> tseCodeInfos =
      dataHandle.getSectorDetail(toTseVendorCode(key.first), static_cast<int>(key.second));

  result->entries.resize(tseCodeInfos.size());
  for (const SectorDetailInfo* info : tseCodeInfos)
  {
    DaoDataFormatConverter::convert(*info, result->entries[info->seqNo() - 1]);
  }

  return SharedConstValue(std::move(result));
}
}

SectorDetailServiceV2::SectorDetailServiceV2(const DateTime& ticketingDate)
  : _cache(boost::bind(&readData, ticketingDate, _1))
{
}

std::shared_ptr<const tax::SectorDetail>
SectorDetailServiceV2::getSectorDetail(const tax::type::Vendor& vendor, tax::type::Index itemNo) const
{
  return _cache.get(std::make_pair(vendor, itemNo));
}

} // namespace tax
