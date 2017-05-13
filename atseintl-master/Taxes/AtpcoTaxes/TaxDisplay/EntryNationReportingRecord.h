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

#include "DataModel/Common/Codes.h"
#include "DataModel/Services/ReportingRecord.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "TaxDisplay/Common/Types.h"
#include "TaxDisplay/Entry.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <set>

namespace tax
{
class RulesRecord;
class Services;
class TaxData;

namespace display
{
class ResponseFormatter;
class TaxDisplayRequest;

class EntryNationReportingRecord : public Entry
{
  friend class EntryNationReportingRecordTest;

public:
  EntryNationReportingRecord(ResponseFormatter& formatter,
                             Services& services,
                             const TaxDisplayRequest& request);

  bool buildHeader() override; // getting data
  bool buildBody() override; // main tax display table
  bool buildFooter() override; // rounding info

private:
  static bool comparator(const ReportingRecordService::SharedConstSingleValue& lhs,
                         const ReportingRecordService::SharedConstSingleValue& rhs)
  {
    const int taxCodeCompare = lhs->taxCode.compare(rhs->taxCode);
    if (taxCodeCompare != 0)
      return taxCodeCompare < 0;

    const int taxTypeCompare = lhs->taxType.compare(rhs->taxType);
    if (taxTypeCompare != 0)
      return taxTypeCompare < 0;

      return lhs->taxCarrier < rhs->taxCarrier;
  }

  bool bodyTaxTable();
  bool bodyTaxDetails(DetailEntryNo detailEntryNo,
                      bool /*isThisLastDetailEntry*/);
  bool bodyTaxReissue();
  bool getHeaderData();
  void getTaxData();
  bool isValid(const RulesRecord& rulesRecord);

  type::Nation _nationCode;
  type::NationName _nationName;
  ReportingRecordsDataSet _reportingRecords{&comparator};
  unsigned int _taxRowCounter;

  Services& _services;
  const TaxDisplayRequest& _request;
};

} /* namespace display */
} /* namespace tax */
