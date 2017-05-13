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

#include "FareCalc/CalcTotals.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Xform/PfcTaxSplitData.h"
#include "Xform/TaxSplitData.h"
#include "Xform/TaxSplitModel.h"
#include "Xform/AbstractTaxSummaryInfo.h"

#include <stdexcept>

namespace tse
{

TaxSplitModel::TaxSplitModel(PricingTrx& pricingTrx, CalcTotals& calcTotals)
    : _pricingTrx(pricingTrx), _calcTotals(calcTotals), _fareUsage2SplitTaxInfo()
{
  buildTaxesPerFareUsageMap(_calcTotals.getFcTaxInfo().getFareUsage2SplitTaxInfo4Pricing());
}

void
TaxSplitModel::addTaxItems(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo)
{
  for (const TaxItem* taxItem : splitTaxInfo.taxItems)
  {
    if (taxItem->taxAmount())
    {
      _fareUsage2SplitTaxInfo[fareUsage].push_back(
          TaxSplitData::create(_pricingTrx, _calcTotals, splitTaxInfo, *taxItem, fareUsage));
    }
  }
}

void
TaxSplitModel::addPfcItems(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo)
{
  for (const PfcItem* pfcItem : splitTaxInfo.pfcItems)
  {
    if (pfcItem->pfcAmount())
    {
      _fareUsage2SplitTaxInfo[fareUsage].push_back(
          PfcTaxSplitData::create(_pricingTrx, _calcTotals, *pfcItem));
    }
  }
}

void
TaxSplitModel::addTaxRecords(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo)
{
  for (const TaxRecord* taxRecord : splitTaxInfo.taxRecords)
  {
    if (taxRecord->getTaxAmount())
    {
      _fareUsage2TaxSummaryInfo[fareUsage].push_back(
        AbstractTaxSummaryInfo::create(_calcTotals, *taxRecord, _pricingTrx));
    }
  }
}

void
TaxSplitModel::buildTaxesPerFareUsageMap(const FareCalc::FcTaxInfo::TaxesPerFareUsage& taxGrouping)
{
  for (auto& each : taxGrouping)
  {
    _fareUsage2SplitTaxInfo[each.first];
    _fareUsage2TaxSummaryInfo[each.first];

    addTaxItems(each.first, each.second);
    addPfcItems(each.first, each.second);
    addTaxRecords(each.first, each.second);
  }
}

bool
TaxSplitModel::contains(const FareUsage* fareUsage) const
{
  const auto& taxInfoMap = _calcTotals.getFcTaxInfo().getFareUsage2SplitTaxInfo4Pricing();
  return taxInfoMap.find(fareUsage) != taxInfoMap.cend();
}

const std::vector<std::shared_ptr<AbstractTaxSplitData>>&
TaxSplitModel::getTaxBreakdown(const FareUsage& fareUsage) const
{
  return _fareUsage2SplitTaxInfo.at(&fareUsage);
}

const std::vector<std::shared_ptr<AbstractTaxSummaryInfo>>&
TaxSplitModel::getTaxSummaryInfo(const FareUsage& fareUsage) const
{
  return _fareUsage2TaxSummaryInfo.at(&fareUsage);
}

} // end of tse namespace
