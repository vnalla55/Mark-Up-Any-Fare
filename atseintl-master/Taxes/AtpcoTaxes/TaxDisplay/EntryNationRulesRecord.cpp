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

#include "TaxDisplay/EntryNationRulesRecord.h"

#include "Common/FilterPredicates.h"
#include "DataModel/Services/ReportingRecord.h"
#include "DataModel/Services/RulesRecord.h"
#include "TaxDisplay/Common/DataUtils.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/FixedWidth.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/LineParams.h"
#include "TaxDisplay/ViewX1SequenceDetailBuilder.h"
#include "Rules/TaxData.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/NationService.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

namespace tax
{
namespace display
{
namespace
{

inline bool printErrNationNotFound(ResponseFormatter& formatter)
{
  formatter.addLine("REQUESTED COUNTRY NOT FOUND");
  return false;
}

inline bool validateData(const RulesRecord& rulesRecord,
                         const TaxDisplayRequest& request)
{
  return (request.taxCode.empty() || request.taxCode == rulesRecord.taxName.taxCode()) &&
         (request.taxType.empty() || request.taxType == rulesRecord.taxName.taxType()) &&
         (request.seqNo == 0 || request.seqNo == rulesRecord.seqNo) &&
         (!request.hasSetAnyCarrierCode() || request.hasCarrierCode(rulesRecord.taxName.taxCarrier())) &&
         validDatePredicate(rulesRecord.effDate, rulesRecord.discDate, rulesRecord.expiredDate, request.requestDate) &&
         (request.travelDate.date().is_blank_date() || validTravelDatePredicate(rulesRecord, request.travelDate.date()));
}

} // anonymous namespace

EntryNationRulesRecord::EntryNationRulesRecord(ResponseFormatter& formatter,
                                               Services& services,
                                               const TaxDisplayRequest& request) :
                                                 Entry(formatter, request.detailLevels),
                                                 _request(request),
                                                 _services(services)
{
}

bool EntryNationRulesRecord::buildHeader()
{
  if (!getTaxTableViewData())
    return false;

  if (_request.seqNo > 0 && _rulesRecords.size() == 1)
  {
    ViewX1SequenceDetailBuilder sequenceDetailBuilder(_request, _services);
    _view = sequenceDetailBuilder.build(*_rulesRecords.front(), _formatter);
    return _view->header(); // the user typed seqNo in the request, we've got it so don't hesitate, show it!
  }

  if (_detailLevelSequencer.hasDetailEntries())
  {
    if (!runSequencer())
      return false; // TODO: add error line
  }
  else
  {
    auto taxTableView = std::make_unique<ViewX1TaxTable>(_services.taxRoundingInfoService(),
                                                         _taxTableData,
                                                         _formatter);
    taxTableView->setSingleNationData({_nationCode, _nationName, _nationMessage});
    _view = std::move(taxTableView);
  }

  return _view->header();
}

bool EntryNationRulesRecord::buildBody()
{
  return _view->body();
}

bool EntryNationRulesRecord::buildFooter()
{
  return _view->footer();
}


bool EntryNationRulesRecord::buildTaxDetailView(DetailEntryNo entryNo,
                                                bool isThisLastCallback)
{
  if (!_taxDetailBuilder.setData(_taxTableData,
                                 _request.requestDate,
                                 _rulesRecords, entryNo))
  {
    _formatter.addLine("NO DATA EXISTS-MODIFY/REENTER OR REFER TO TXHELP");
    return false;
  }

  if (isThisLastCallback)
  {
    std::unique_ptr<ViewX1TaxDetail> view = _taxDetailBuilder.build(_nationName, _formatter);
    if (_request.showCategoryForEachTax)
    {
      if (_taxDetailBuilder.hasSingleSequence())
      {
        buildSequenceDetailView(1, true); // we only have one sequence and we want to display detail of it
        return true;
      }

      view->setCombinedView(std::make_shared<ViewX1SequenceDetailBuilder>(_request, _services));
    }

    _view = std::move(view);
  }

  return true;
}

bool EntryNationRulesRecord::buildSequenceDetailView(DetailEntryNo entryNo,
                                                     bool isThisLastCallback)
{
  const ViewX1TaxDetail::DataSet& taxDetailData = _taxDetailBuilder.getData();

  if (_UNLIKELY(taxDetailData.empty() || entryNo <= 0 || entryNo > taxDetailData.size()))
  {
    _formatter.addLine("NO DATA EXISTS-MODIFY/REENTER OR REFER TO TXHELP");
    return false;
  }

  if (isThisLastCallback)
  {
    ViewX1SequenceDetailBuilder sequenceDetailBuilder(_request, _services);
    _view = sequenceDetailBuilder.build(taxDetailData, _formatter, entryNo);
  }

  return true;
}

bool EntryNationRulesRecord::getTaxTableViewData()
{
  bool gotNationCode = getNationCode(_request, _services.locService(), _nationCode);
  bool gotNationName = getNationName(_nationCode,
                                     _request.nationName,
                                     _services.locService(),
                                     _nationName);

  if (!gotNationCode || !gotNationName)
    return printErrNationNotFound(_formatter);

  _nationMessage = _services.nationService().getMessage(_nationCode, _request.requestDate);

  for (auto taxPointTag : {type::TaxPointTag::Arrival,
                           // type::TaxPointTag::Delivery, - not needed for now
                           type::TaxPointTag::Departure,
                           type::TaxPointTag::Sale})
  {
    std::vector<RulesRecord> rulesRecords =
        _services.rulesRecordsService().getTaxRecords(_nationCode,
                                                      taxPointTag,
                                                      _request.requestDate);

    insertSortedTaxTableRow(rulesRecords);
  }

  return true;
}

void EntryNationRulesRecord::insertSortedTaxTableRow(std::vector<RulesRecord>& rulesRecords)
{
  _rulesRecords.reserve(rulesRecords.size());
  for (RulesRecord& rulesRecord : rulesRecords)
  {
    if (validateData(rulesRecord, _request))
    {
      ReportingRecordService::SharedConstSingleValue reportingRecord =
          _services.reportingRecordService().getReportingRecord(rulesRecord.vendor,
                                                                rulesRecord.taxName.nation(),
                                                                rulesRecord.taxName.taxCarrier(),
                                                                rulesRecord.taxName.taxCode(),
                                                                rulesRecord.taxName.taxType());

      const type::TaxLabel* taxLabel = nullptr;
      if (_LIKELY(reportingRecord && !reportingRecord->entries.empty()))
        taxLabel = &reportingRecord->entries.front().taxLabel;

      auto sharedRulesRecord = std::make_shared<const RulesRecord>(std::move(rulesRecord));
      _taxTableData.insert(std::forward_as_tuple(sharedRulesRecord->taxName,
                                                 taxLabel,
                                                 sharedRulesRecord->vendor));
      _rulesRecords.push_back(std::move(sharedRulesRecord));
    }
  }
  _rulesRecords.shrink_to_fit();
}

bool EntryNationRulesRecord::runSequencer()
{
  using namespace std::placeholders;
  _detailLevelSequencer.pushCallback(std::bind(&EntryNationRulesRecord::buildTaxDetailView,
                                               this, _1, _2));
  _detailLevelSequencer.pushCallback(std::bind(&EntryNationRulesRecord::buildSequenceDetailView,
                                               this, _1, _2));
  return _detailLevelSequencer.run();
}

} /* namespace display */
} /* namespace tax */
