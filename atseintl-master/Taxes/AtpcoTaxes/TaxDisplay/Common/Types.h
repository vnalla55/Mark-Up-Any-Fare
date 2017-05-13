// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#pragma once

#include "ServiceInterfaces/ReportingRecordService.h"

#include <set>
#include <string>

namespace tax
{
namespace display
{

using DetailLevels = std::string; // detail levels string, i.e. "10|5|2|1..."
using DetailEntryNo = unsigned int;
using CategoryNo = uint32_t;
using ReportingRecordsCompare = bool(*)(const ReportingRecordService::SharedConstSingleValue&,
                                        const ReportingRecordService::SharedConstSingleValue&);
using ReportingRecordsDataSet =
    std::set<ReportingRecordService::SharedConstSingleValue, ReportingRecordsCompare>;

enum class X1Category
{
  TAX = 1,
  DATE_APPLICATIONS,
  SALE_TICKET_DELIVERY,
  TAX_DETAILS,
  TICKET_VALUE,
  TAXABLE_UNIT_DETAILS,
  TRAVEL_APPLICATION,
  TAX_POINT_SPECIFICATION,
  TRAVEL_CARRIER_QUALIFYING_TAGS,
  SERVICE_BAGGAGE_TAX,
  TRAVEL_SECTOR_DETAILS,
  CONNECTION_TAGS,
  PROCESSING_APPLICATION_DETAIL,
  ENUM_CATEGORY_SIZE
};

enum class X2Category
{
  DEFINITION = 1,
  APPLICABLE_TO,
  TAX_RATE,
  EXEMPTIONS,
  COLLECTION_REMITTANCE,
  TAX_AUTHORITY,
  COMMENTS,
  SPECIAL_INSTRUCTIONS,
  REPORTING,
  ENUM_CATEGORY_SIZE
};

} // namespace display
} // namespace tax
