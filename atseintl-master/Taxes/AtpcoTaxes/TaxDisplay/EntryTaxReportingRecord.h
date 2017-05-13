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
#include "ServiceInterfaces/ReportingRecordService.h"
#include "TaxDisplay/Entry.h"
#include "TaxDisplay/Common/Types.h"

namespace tax
{
class Services;

namespace display
{

class ResponseFormatter;
class TaxDisplayRequest;

class EntryTaxReportingRecord : public Entry
{
  friend class EntryTaxReportingRecordTest;

public:
  EntryTaxReportingRecord(ResponseFormatter& formatter,
                          Services& services,
                          const TaxDisplayRequest& request);

  bool buildHeader() override;

private:
  ReportingRecordsDataSet getFilteredRecords(const ReportingRecordService::GroupValue& records);
  bool headerForMultipleRecords(const ReportingRecordsDataSet& records);
  bool printDetailedDisplay(const ReportingRecordsDataSet& records,
                            DetailEntryNo detailEntryNo,
                            bool /* isThisLastEntry */);
  bool printTaxReissue(const ReportingRecordsDataSet& records);

  Services& _services;
  const TaxDisplayRequest& _request;
};

} /* namespace display */
} /* namespace tax */
