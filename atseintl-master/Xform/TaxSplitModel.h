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

#include "FareCalc/FcTaxInfo.h"
#include "Xform/AbstractTaxSummaryInfo.h"
#include "Xform/PfcTaxSplitData.h"
#include "Xform/TaxSplitData.h"

#include <map>
#include <memory>
#include <vector>

namespace tse
{

class FareUsage;
class PricingTrx;
class CalcTotals;

namespace FareCalc
{
  class SplitTaxInfo;
}

using TaxesPerFareUsage = std::map<const FareUsage*, std::vector<std::shared_ptr<AbstractTaxSplitData>>>;
using TaxSummaryInfoPerFareUsage = std::map<const FareUsage*, std::vector<std::shared_ptr<AbstractTaxSummaryInfo>>>;

class TaxSplitModel
{
  PricingTrx& _pricingTrx;
  CalcTotals& _calcTotals;
  TaxesPerFareUsage _fareUsage2SplitTaxInfo;
  TaxSummaryInfoPerFareUsage _fareUsage2TaxSummaryInfo;

  void
  addTaxItems(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo);

  void
  addPfcItems(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo);

  void
  addTaxRecords(const FareUsage* fareUsage, const FareCalc::SplitTaxInfo& splitTaxInfo);

public:
  TaxSplitModel(PricingTrx& pricingTrx, CalcTotals& calcTotals);

  void
  buildTaxesPerFareUsageMap(const FareCalc::FcTaxInfo::TaxesPerFareUsage& taxGrouping);

  const std::vector<std::shared_ptr<AbstractTaxSplitData>>&
  getTaxBreakdown(const FareUsage& fareUsage) const;

  const std::vector<std::shared_ptr<AbstractTaxSummaryInfo>>&
  getTaxSummaryInfo(const FareUsage& fareUsage) const;

  bool
  contains(const FareUsage* fareUsage) const;
};

}
