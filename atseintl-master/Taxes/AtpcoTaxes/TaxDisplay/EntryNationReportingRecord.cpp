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

#include "TaxDisplay/EntryNationReportingRecord.h"

#include "Common/TaxName.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Code.h"
#include "Rules/TaxData.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include "TaxDisplay/Common/CommonEntries.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/FixedWidth.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/LineParams.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "TaxDisplay/ViewX2TaxReissue.h"
#include "TaxDisplay/ViewX2TaxReissueBuilder.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace tax
{
namespace display
{

EntryNationReportingRecord::EntryNationReportingRecord(
    ResponseFormatter& formatter,
    Services& services,
    const TaxDisplayRequest& request) :
        Entry(formatter, request.detailLevels),
        _taxRowCounter(1),
        _services(services),
        _request(request)
{
  using namespace std::placeholders;
  _detailLevelSequencer.pushCallback(std::bind(&EntryNationReportingRecord::bodyTaxDetails,
                                               this, _1, _2));
}

bool EntryNationReportingRecord::buildHeader() // in this case the header is used only to get data
{
  if(!getHeaderData())
    return false;

  getTaxData();
  if (_reportingRecords.empty())
  {
    _formatter.addLine(CommonEntries::getErrorNoData(_request));
    return false;
  }

  return true;
}

bool EntryNationReportingRecord::buildBody()
{
  if (_request.isReissued)
    return bodyTaxReissue();

  if(_detailLevelSequencer.hasDetailEntries())
    return _detailLevelSequencer.run();

  return bodyTaxTable();
}

bool EntryNationReportingRecord::buildFooter()
{
  _formatter.addBlankLine();
  _formatter.addLine("CARRIER YY - ALL OTHER CARRIERS NOT SPECIFIED");
  _formatter.addLine(CommonEntries::roundingInfo(_services.taxRoundingInfoService(),
                                                 _nationCode));

  if (!_request.isUserTN())
  {
    _formatter.addLine("TAX TYPES REPRESENT DIFFERENT APPLICATIONS FOR THE SAME TAX CODE");
    _formatter.addLine("TAX TYPES 001-099: AIR TRAVEL TAXES");
    _formatter.addLine("TAX TYPES 100 AND ABOVE: ANCILLARY TAXES");
  }

  _formatter.addLine("USE TX*# WHERE # IS LINE NUMBER TO VIEW DETAILS");
  _formatter.addLine("USE TXHELP FOR HELP ENTRY FORMATS");

  return true;
}

bool EntryNationReportingRecord::bodyTaxDetails(DetailEntryNo detailEntryNo, bool)
{
  if (detailEntryNo <= 0 || detailEntryNo > _reportingRecords.size())
  {
    _formatter.addLine("NO DATA EXISTS-MODIFY/REENTER OR REFER TO TXHELP");
    return false;
  }

  auto selectedRow = _reportingRecords.begin();
  std::advance(selectedRow, detailEntryNo - 1);

  CommonEntries::taxDetails(**selectedRow, // iterator to shared ptr
                            _services.locService(),
                            _services.taxCodeTextService(),
                            _request,
                            _formatter);

  return false; // we don't want footer with rounding
}

bool EntryNationReportingRecord::bodyTaxTable()
{
  _formatter.addLine("COUNTRY NAME - " + _nationName);
  _formatter.addLine("COUNTRY CODE - " + _nationCode.asString());

  LineParams headerParam = LineParams::withLeftMargin(4);
  _formatter.addBlankLine();

  if (_request.isUserTN())
  {
    _formatter.addLine("TAX       TAX TAX", headerParam);
    _formatter.addLine("CODE      CXR NAME", headerParam);
  }
  else
  {
    _formatter.addLine("TAX  TAX  TAX TAX", headerParam);
    _formatter.addLine("CODE TYPE CXR NAME", headerParam);
  }


  _formatter.addBlankLine();

  for(const ReportingRecordService::SharedConstSingleValue& reportingRecord : _reportingRecords)
  {
    Line line;
    line.params().setLeftMargin(18);
    line.params().setParagraphIndent(-18);
    line << fixedWidth(_taxRowCounter++,            4);

    if (_request.isUserTN())
    {
      line << fixedWidth(reportingRecord->taxCode.asString(),    10);
    }
    else
    {
      line << fixedWidth(reportingRecord->taxCode.asString(),    5);
      line << fixedWidth(reportingRecord->taxType.asString(),    5);
    }

    line << fixedWidth(reportingRecord->taxCarrier.asString(), 4);

    if (reportingRecord->entries.size() > 0)
      line << reportingRecord->entries.front().taxLabel;

    _formatter.addLine(line);
  }

  return true;
}

bool EntryNationReportingRecord::bodyTaxReissue()
{
  std::map<type::Nation, ViewX2TaxReissue::ReissueViewData> data;
  ViewX2TaxReissueBuilder builder(_services);
  if (_detailLevelSequencer.hasDetailEntries())
  {
    DetailEntryNo detailEntryNo = _detailLevelSequencer.getDetailEntries().front(); // there could be only 1 detail level
    if (detailEntryNo <= 0 || detailEntryNo > _reportingRecords.size())
    {
      _formatter.addLine("UNABLE TO IDENTIFY LINE #");
      return false;
    }

    auto selectedRow = _reportingRecords.begin();
    std::advance(selectedRow, detailEntryNo - 1);

    data = builder.getData(_request, {*selectedRow});
  }
  else
  {
    data = builder.getData(_request, _reportingRecords);
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

bool EntryNationReportingRecord::isValid(const RulesRecord& rulesRecord)
{
  return (_request.taxCode.empty() || _request.taxCode == rulesRecord.taxName.taxCode()) &&
         (_request.taxType.empty() || _request.taxType == rulesRecord.taxName.taxType()) &&
         (!_request.hasSetAnyCarrierCode() || _request.hasCarrierCode(rulesRecord.taxName.taxCarrier()));
}

bool EntryNationReportingRecord::getHeaderData()
{
  if(!_request.nationCode.empty())
  {
    _nationCode = _request.nationCode;
    _nationName = _services.locService().getNationName(_nationCode);
    if(_nationName.empty())
    {
      _formatter.addLine("COUNTRY CODE NOT FOUND");
      return false;
    }
  }
  else if(!_request.airportCode.empty())
  {
    type::Nation nationCode = _services.locService().getNation(_request.airportCode);
    if(nationCode.empty())
    {
      _formatter.addLine("AIRPORT/CITY CODE NOT FOUND");
      return false;
    }

    _nationCode = nationCode;
    _nationName = _services.locService().getNationName(_nationCode);
  }
  else if(!_request.nationName.empty())
  {
    type::Nation nationCode = _services.locService().getNationByName(_request.nationName);
    if(nationCode.empty())
    {
      _formatter.addLine("COUNTRY NAME NOT FOUND");
      return false;
    }

    _nationCode = nationCode;
    _nationName = _request.nationName;
  }
  else
  {
    _formatter.addLine("UNKNOWN ERROR");
    return false;
  }

  return true;
}

void EntryNationReportingRecord::getTaxData()
{
  for (auto tag : {type::TaxPointTag::Sale,
                   type::TaxPointTag::Departure,
                   // type::TaxPointTag::Delivery, - not needed for now
                   type::TaxPointTag::Arrival})
  {
    std::vector<RulesRecord> rulesRecords =
      _services.rulesRecordsService().getTaxRecords(_nationCode, tag, _request.requestDate);

    for (const RulesRecord& rule : rulesRecords)
    {
      if (!isValid(rule))
        continue;

      ReportingRecordService::SharedConstSingleValue reportingRecord =
              _services.reportingRecordService().getReportingRecord(rule.vendor,
                                                                    rule.taxName.nation(),
                                                                    rule.taxName.taxCarrier(),
                                                                    rule.taxName.taxCode(),
                                                                    rule.taxName.taxType());

      if (reportingRecord)
        _reportingRecords.insert(std::move(reportingRecord));
    }
  }


}

} /* namespace display */
} /* namespace tax */
