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

#include "TaxDisplay/ViewX1TaxDetailBuilder.h"

#include "TaxDisplay/Common/TaxDisplayRequest.h"

#include <iterator>

namespace tax
{
namespace display
{
namespace

{
inline bool copyRulesPredicate(const std::shared_ptr<const RulesRecord>& rulesRecord,
                               const TaxName& taxName,
                               const type::Vendor& vendor,
                               const type::Timestamp& /*ticketingDate*/)
{
  return rulesRecord->taxName.taxCode() == taxName.taxCode() &&
         rulesRecord->taxName.taxType() == taxName.taxType() &&
         rulesRecord->taxName.nation() == taxName.nation() &&
         rulesRecord->vendor == vendor;
}

} // anonymous namespace



std::unique_ptr<ViewX1TaxDetail>
ViewX1TaxDetailBuilder::build(const type::NationName& nationName,
                              ResponseFormatter& formatter)
{
  return std::make_unique<ViewX1TaxDetail>(_taxDetailData,
                                           nationName,
                                           _taxLabel,
                                           formatter);
}

bool ViewX1TaxDetailBuilder::setData(
    const ViewX1TaxTable::DataSet& taxTableData,
    const type::Timestamp& date,
    const std::vector<RulesRecordsService::SharedTaxRulesRecord>& rulesRecords,
    DetailEntryNo entryNo)
{
  if (entryNo <= 0 || entryNo > taxTableData.size())
    return false;

  auto selectedRow = taxTableData.begin();
  std::advance(selectedRow, entryNo - 1);

  using std::placeholders::_1;
  const TaxName& taxName = std::get<const TaxName&>(*selectedRow);
  const type::Vendor& vendor = std::get<const type::Vendor&>(*selectedRow);
  std::copy_if(rulesRecords.begin(),
               rulesRecords.end(),
               std::inserter(_taxDetailData, _taxDetailData.end()),
               std::bind(copyRulesPredicate, _1, taxName, vendor, std::cref(date)));

  _taxLabel = std::get<const type::TaxLabel*>(*selectedRow);
  return true;
}

} // namespace display
} // namespace tax



