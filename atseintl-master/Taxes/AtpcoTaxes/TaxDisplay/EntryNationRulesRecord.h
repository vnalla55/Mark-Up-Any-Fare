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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "ServiceInterfaces/Services.h"
#include "TaxDisplay/Entry.h"
#include "TaxDisplay/View.h"
#include "TaxDisplay/ViewX1TaxDetailBuilder.h"
#include "TaxDisplay/ViewX1TaxTable.h"

#include <set>

namespace tax
{
class Services;

namespace display
{
class TaxDisplayRequest;

class EntryNationRulesRecord : public Entry
{
  friend class EntryNationRulesRecordTest;

public:
  EntryNationRulesRecord(ResponseFormatter& formatter,
                         Services& services,
                         const TaxDisplayRequest& request);

  bool buildHeader() override;
  bool buildBody() override;
  bool buildFooter() override;

private:
  bool buildTaxDetailView(DetailEntryNo entryNo, bool isThisLastCallback);
  bool buildSequenceDetailView(DetailEntryNo entryNo, bool isThisLastCallback);

  bool getTaxTableViewData();
  void insertSortedTaxTableRow(std::vector<RulesRecord>& rulesRecords);

  bool runSequencer();

  type::Nation _nationCode;
  type::NationName _nationName;
  type::NationMessage _nationMessage;
  std::vector<std::shared_ptr<const RulesRecord>> _rulesRecords;
  ViewX1TaxTable::DataSet _taxTableData{&ViewX1TaxTable::byTaxCodeCompare};
  ViewX1TaxDetailBuilder _taxDetailBuilder;

  std::unique_ptr<View> _view;
  const TaxDisplayRequest& _request;
  Services& _services;
};

} /* namespace display */
} /* namespace tax */
