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

#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "TaxDisplay/Common/Types.h"
#include "TaxDisplay/ViewX1TaxDetail.h"
#include "TaxDisplay/ViewX1TaxTable.h"

#include <vector>

namespace tax
{
namespace display
{
class ResponseFormatter;
class TaxDisplayRequest;

class ViewX1TaxDetailBuilder
{
public:
  ViewX1TaxDetailBuilder() = default;
  ~ViewX1TaxDetailBuilder() = default;

  bool setData(const ViewX1TaxTable::DataSet& taxTableData,
               const type::Timestamp& date,
               const std::vector<RulesRecordsService::SharedTaxRulesRecord>& rulesRecords,
               DetailEntryNo entryNo);

  const ViewX1TaxDetail::DataSet& getData() const { return _taxDetailData; }
  bool hasSingleSequence() const { return _taxDetailData.size() == 1; }

  std::unique_ptr<ViewX1TaxDetail>
  build(const type::NationName& nationName, ResponseFormatter& formatter);

private:
  ViewX1TaxDetail::DataSet _taxDetailData{&ViewX1TaxDetail::bySequenceNbCompare};
  const type::TaxLabel* _taxLabel{nullptr};
};

} // namespace display
} // namespace tax
