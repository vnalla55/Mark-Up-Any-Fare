// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/Types.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/Line.h"

namespace tax
{

class LocService;
class TaxCodeTextService;
class ReportingRecord;
class TaxRoundingInfoService;

namespace display
{

class ResponseFormatter;

class CommonEntries
{
public:
  static void taxDetails(const ReportingRecord& reportingRecord,
                         const LocService& locService,
                         const TaxCodeTextService& taxCodeTextService,
                         const TaxDisplayRequest& request,
                         ResponseFormatter& formatter);

  static void taxDetailsCategories(const ReportingRecord& reportingRecord,
                                   const TaxCodeTextService& taxCodeTextService,
                                   const TaxDisplayRequest& request,
                                   ResponseFormatter& formatter);

  static Line roundingInfo(const TaxRoundingInfoService& roundingInfoService,
                           type::Nation nationCode);

  static std::string getErrorNoData(const TaxDisplayRequest& request);
};

} /* namespace display */
} /* namespace tax */
