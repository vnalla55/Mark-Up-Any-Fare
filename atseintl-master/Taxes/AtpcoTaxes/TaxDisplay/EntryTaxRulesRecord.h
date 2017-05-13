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
#include "ServiceInterfaces/RulesRecordsService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Entry.h"
#include "TaxDisplay/ViewX1TaxTable.h"
#include "TaxDisplay/ViewX1TaxDetail.h"
#include "TaxDisplay/ViewX1TaxDetailBuilder.h"

#include <memory>
#include <string>

namespace tax
{
class Services;

namespace display
{
class EntryTaxRulesRecord : public Entry
{
  friend class EntryTaxRulesRecordTest;

public:
  EntryTaxRulesRecord(ResponseFormatter& formatter,
                      Services& services,
                      const TaxDisplayRequest& request) :
                        Entry(formatter, request.detailLevels),
                        _request(request),
                        _services(services) {}

  bool buildHeader() override;
  bool buildBody() override;
  bool buildFooter() override;

private:
  bool buildTaxDetailView(DetailEntryNo entryNo, bool isThisLastCallback);
  bool buildSequenceDetailView(DetailEntryNo entryNo, bool isThisLastCallback);

  bool getTaxTableData();

  bool runSequencer();

  std::vector<RulesRecordsService::SharedTaxRulesRecord> _rulesRecords;
  std::vector<ReportingRecordService::SharedConstSingleValue> _reportingRecords;

  bool _hasSingleNationRecords{true};
  type::Nation _nationCode;
  type::NationName _nationName;
  type::NationMessage _nationMessage;
  const TaxDisplayRequest& _request;
  Services& _services;
  ViewX1TaxTable::DataSet _taxTableData{&ViewX1TaxTable::byTaxCodeCompare};
  ViewX1TaxDetailBuilder _taxDetailBuilder;
  std::unique_ptr<View> _view;
};

} /* namespace display */
} /* namespace tax */
