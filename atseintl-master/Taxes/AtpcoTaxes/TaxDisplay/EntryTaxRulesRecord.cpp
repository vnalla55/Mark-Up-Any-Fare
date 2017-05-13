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

#include "TaxDisplay/EntryTaxRulesRecord.h"

#include "Common/FilterPredicates.h"
#include "Common/TaxDisplayRequest.h"
#include "DataModel/Services/ReportingRecord.h"
#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/NationService.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/Services.h"
#include "Util/BranchPrediction.h"
#include "TaxDisplay/Common/CommonEntries.h"
#include "TaxDisplay/ViewX1SequenceDetailBuilder.h"

#include <memory>
#include <iterator>

namespace tax
{
namespace display
{
namespace
{
bool validate(const RulesRecordsService::SharedTaxRulesRecord& rule,
              const TaxDisplayRequest& request)
{
 return (request.taxType.empty() || request.taxType == rule->taxName.taxType()) &&
        (!request.hasSetAnyCarrierCode() || request.hasCarrierCode(rule->taxName.taxCarrier())) &&
        (request.seqNo == 0 || request.seqNo == rule->seqNo) &&
        validDatePredicate(rule->effDate, rule->discDate, rule->expiredDate, request.requestDate) &&
        (request.travelDate.date().is_blank_date() || validTravelDatePredicate(*rule, request.travelDate.date()));
}
}

bool EntryTaxRulesRecord::buildHeader()
{
  if (!getTaxTableData())
    return false;

  if (_request.seqNo > 0 && _rulesRecords.size() == 1)
  {
    ViewX1SequenceDetailBuilder sequenceDetailBuilder(_request, _services);
    _view = sequenceDetailBuilder.build(*_rulesRecords.front(), _formatter);
    _view->header();
    return true; // the user typed seqNo in the request, we've got it so don't hesitate, show it!
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
    if (_hasSingleNationRecords)
    {
      _nationName = _services.locService().getNationName(_nationCode);
      _nationMessage = _services.nationService().getMessage(_nationCode, _request.requestDate);
      taxTableView->setSingleNationData({_nationCode, _nationName, _nationMessage});
    }

    _view = std::move(taxTableView);
    }

  return _view->header();
}

bool EntryTaxRulesRecord::buildBody()
{
  return _view->body();
}

bool EntryTaxRulesRecord::buildFooter()
{
  return _view->footer();
}

bool EntryTaxRulesRecord::buildTaxDetailView(DetailEntryNo entryNo,
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
    _view = _taxDetailBuilder.build(_nationName, _formatter);

  return true;
}

bool EntryTaxRulesRecord::buildSequenceDetailView(DetailEntryNo entryNo,
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

bool EntryTaxRulesRecord::getTaxTableData()
{
  const std::shared_ptr<const std::vector<RulesRecordsService::SharedTaxRulesRecord>> rulesRecords =
      _services.rulesRecordsService().getTaxRecords(_request.taxCode, _request.requestDate);

  if (!rulesRecords || rulesRecords->empty())
  {
    _formatter.addLine(CommonEntries::getErrorNoData(_request));
    return false;
  }

  type::Nation tempNation;
  _rulesRecords.reserve(rulesRecords->size());
  for (const RulesRecordsService::SharedTaxRulesRecord& rulesRecord : *rulesRecords)
  {
    if (_LIKELY(rulesRecord))
    {
      if (!validate(rulesRecord, _request))
        continue;

      ReportingRecordService::SharedConstSingleValue reportingRecord =
          _services.reportingRecordService().getReportingRecord(rulesRecord->vendor,
                                                                rulesRecord->taxName.nation(),
                                                                rulesRecord->taxName.taxCarrier(),
                                                                rulesRecord->taxName.taxCode(),
                                                                rulesRecord->taxName.taxType());

      const type::TaxLabel* label = nullptr;
      if (_LIKELY(reportingRecord && !reportingRecord->entries.empty()))
      {
        label = &reportingRecord->entries.front().taxLabel;
        _reportingRecords.push_back(reportingRecord);
      }

      _taxTableData.insert(std::forward_as_tuple(rulesRecord->taxName,
                                                 label,
                                                 rulesRecord->vendor));
      _rulesRecords.push_back(rulesRecord);

      if (tempNation.empty())
        tempNation = rulesRecord->taxName.nation();

      if (_hasSingleNationRecords)
        _hasSingleNationRecords = tempNation == rulesRecord->taxName.nation();
    }
  }

  if (_rulesRecords.empty())
  {
    _formatter.addLine(CommonEntries::getErrorNoData(_request));
    return false;
  }

  _rulesRecords.shrink_to_fit();

  if (_hasSingleNationRecords)
    _nationCode = tempNation;
  return true;
}

bool EntryTaxRulesRecord::runSequencer()
{
  using namespace std::placeholders;
  _detailLevelSequencer.pushCallback(std::bind(&EntryTaxRulesRecord::buildTaxDetailView,
                                               this, _1, _2));
  _detailLevelSequencer.pushCallback(std::bind(&EntryTaxRulesRecord::buildSequenceDetailView,
                                               this, _1, _2));
  return _detailLevelSequencer.run();
}

} /* namespace display */
} /* namespace tax */
