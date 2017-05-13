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

#include "TaxDisplay/EntryTaxReportingRecord.h"

#include "Common/SafeEnumToString.h"
#include "Common/Timestamp.h"
#include "DataModel/Services/ReportingRecord.h"
#include "ServiceInterfaces/Services.h"
#include "TaxDisplay/Common/CommonEntries.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/FixedWidth.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "TaxDisplay/ViewX2TaxReissue.h"
#include "TaxDisplay/ViewX2TaxReissueBuilder.h"

#include <algorithm>
#include <functional>

namespace tax
{
namespace display
{
namespace
{

inline bool printErrTaxNotFound(ResponseFormatter& formatter)
{
  formatter.addLine("REQUESTED TAX NOT FOUND");
  return false;
}

inline bool isValid(const ReportingRecordService::SharedSingleValue& record,
                    const TaxDisplayRequest& request)
{
  return (request.taxType.empty() || request.taxType == record->taxType) &&
         (!request.hasSetAnyCarrierCode() || request.hasCarrierCode(record->taxCarrier));
}

bool reportingRecordSetCompare(const ReportingRecordService::SharedConstSingleValue& lhs,
                               const ReportingRecordService::SharedConstSingleValue& rhs)
{
  const int nationCompare = lhs->nation.compare(rhs->nation);
  if (nationCompare != 0)
    return nationCompare < 0;

  const int taxCodeCompare = lhs->taxCode.compare(rhs->taxCode);
  if (taxCodeCompare != 0)
    return taxCodeCompare < 0;

  const int taxTypeCompare = lhs->taxType.compare(rhs->taxType);
  if (taxTypeCompare != 0)
    return taxTypeCompare < 0;

    return lhs->taxCarrier < rhs->taxCarrier;
}
} // anonymous namespace

EntryTaxReportingRecord::EntryTaxReportingRecord(
    ResponseFormatter& formatter,
    Services& services,
    const TaxDisplayRequest& request) :
        Entry(formatter, request.detailLevels),
        _services(services),
        _request(request)
{
}

bool EntryTaxReportingRecord::buildHeader()
{
  ReportingRecordService::SharedConstGroupValue unfilteredRecords =
      _services.reportingRecordService().getReportingRecords(_request.taxCode);

  if (!unfilteredRecords || unfilteredRecords->empty())
    return printErrTaxNotFound(_formatter);

  ReportingRecordsDataSet records = getFilteredRecords(*unfilteredRecords);
  if (records.empty())
    return printErrTaxNotFound(_formatter);

  if (_request.isReissued)
  {
    return printTaxReissue(records);
  }

  if (_detailLevelSequencer.hasDetailEntries())
  {
    using namespace std::placeholders;
    _detailLevelSequencer.pushCallback(std::bind(&EntryTaxReportingRecord::printDetailedDisplay,
                                                 this, records, _1, _2));
    return _detailLevelSequencer.run();
  }

  return headerForMultipleRecords(records);
}

ReportingRecordsDataSet
EntryTaxReportingRecord::getFilteredRecords(const ReportingRecordService::GroupValue& records)
{
  ReportingRecordsDataSet ret(reportingRecordSetCompare);

  for (const ReportingRecordService::SharedSingleValue& record : records)
  {
    if (isValid(record, _request))
      ret.insert(record);
  }

  return ret;
}

bool EntryTaxReportingRecord::headerForMultipleRecords(const ReportingRecordsDataSet& records)
{
  _formatter.addBlankLine();

  if (_request.isUserTN())
  {
    _formatter.addLine("COUNTRY TAX       TAX TAX",  LineParams::withLeftMargin(4));
    _formatter.addLine("CODE    CODE      CXR NAME", LineParams::withLeftMargin(4));
  }
  else
  {
    _formatter.addLine("COUNTRY TAX  TAX  TAX TAX",  LineParams::withLeftMargin(4));
    _formatter.addLine("CODE    CODE TYPE CXR NAME", LineParams::withLeftMargin(4));
  }
  _formatter.addBlankLine();

  unsigned int lineNo = 1;
  for (const ReportingRecordService::SharedConstSingleValue& record : records)
  {
    if (!record)
      continue;

    Line taxLine;
    taxLine.params().setLeftMargin(26);
    taxLine.params().setParagraphIndent(-26);
    taxLine << fixedWidth(lineNo++, 4)
            << fixedWidth(record->nation.asString(),     8);

    if (_request.isUserTN())
    {
      taxLine << fixedWidth(record->taxCode.asString(),    10);
    }
    else
    {
      taxLine << fixedWidth(record->taxCode.asString(),    5)
              << fixedWidth(record->taxType.asString(),    5);
    }

    taxLine << fixedWidth(record->taxCarrier.asString(), 4);

    if (!record->entries.empty())
       taxLine << record->entries.front().taxLabel;

    _formatter.addLine(taxLine);

    if (_request.showCategoryForEachTax)
    {
      CommonEntries::taxDetailsCategories(*record, _services.taxCodeTextService(), _request, _formatter);
      _formatter.addBlankLine();
    }
  }

  _formatter.addBlankLine();

  _formatter.addLine("CARRIER YY – ALL OTHER CARRIERS NOT SPECIFIED");
  if (!_request.isUserTN())
  {
    _formatter.addLine("TAX TYPES REPRESENT DIFFERENT APPLICATIONS FOR THE SAME TAX CODE");
    _formatter.addLine("TAX TYPES 001-099: AIR TRAVEL TAXES");
    _formatter.addLine("TAX TYPES 100 AND ABOVE: ANCILLARY TAXES");
  }

  _formatter.addLine("USE TX*# WHERE # IS LINE NUMBER TO VIEW DETAILS");
  _formatter.addLine("USE TXHELP FOR HELP ENTRY FORMATS");

  return false; // at this point we have displayed all we wanted to
}

bool EntryTaxReportingRecord::printDetailedDisplay(const ReportingRecordsDataSet& records,
                                                   DetailEntryNo detailEntryNo,
                                                   bool /* isThisLastEntry */)
{
  if (detailEntryNo > records.size())
    return printErrTaxNotFound(_formatter);

  auto selectedRow = records.begin();
  std::advance(selectedRow, detailEntryNo - 1);

  CommonEntries::taxDetails(**selectedRow,
                            _services.locService(),
                            _services.taxCodeTextService(),
                            _request,
                            _formatter);

  return false; // at this point we have displayed all we wanted to
}

bool EntryTaxReportingRecord::printTaxReissue(const ReportingRecordsDataSet& records)
{
  std::map<type::Nation, ViewX2TaxReissue::ReissueViewData> data;
  ViewX2TaxReissueBuilder builder(_services);
  if (_detailLevelSequencer.hasDetailEntries())
  {
    DetailEntryNo detailEntryNo = _detailLevelSequencer.getDetailEntries().front(); // there could be only 1 detail level
    if (detailEntryNo <= 0 || detailEntryNo > records.size())
    {
      _formatter.addLine("UNABLE TO IDENTIFY LINE #");
      return false;
    }

    auto selectedRow = records.begin();
    std::advance(selectedRow, detailEntryNo - 1);

    data = builder.getData(_request, {*selectedRow});
  }
  else
  {
    data = builder.getData(_request, records);
  }

  if (!builder.hasGotAnyReissues())
  {
    _formatter.addLine("NO DATA EXISTS-MODIFY/REENTER OR REFER TO TXHELP");
    return false;
  }

  ViewX2TaxReissue view(data, _formatter);
  view.body();
  return false; // we've displayed all we wanted to
}

} /* namespace display */
} /* namespace tax */
