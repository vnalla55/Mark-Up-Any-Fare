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
#include "FareCalc/FareAmountsConverter.h"

#include <boost/core/noncopyable.hpp>
#include <memory>

namespace tse
{
class CalcTotals;
class TaxItem;
class Itin;

typedef std::map<const TravelSeg*, const FareUsage*> TravelSegToFareUsage;

class SplitTaxInfoProcessor : private boost::noncopyable
{
  friend class SplitTaxInfoProcessorTest;
public:
  typedef std::vector<TaxItem*> SplitsFromTax;
  typedef std::map<const TaxItem*, SplitsFromTax> ParentTaxToSplits;

  SplitTaxInfoProcessor(const FareCalc::FcTaxInfo& fcTaxInfo,
                        CalcTotals& calcTotals,
                        PricingTrx& pricingTrx,
                        const TaxResponse* taxResponse,
                        const FareCalcConfig* fcConfig,
                        const CurrencyCode& taxCurrencyCode);

  FareCalc::FcTaxInfo::TaxesPerFareUsage
  splitByFareComponent();

private:
  const FareCalc::FcTaxInfo& _fcTaxInfo;
  const TaxResponse* _taxResponse;
  const FareCalcConfig* _fcConfig;
  CalcTotals& _calcTotals;
  const Itin* _itin;
  PricingTrx& _pricingTrx;
  CurrencyCode _taxCurrencyCode;
  std::unique_ptr<FareCalc::FareAmountsConverter> _converter;
  int _fareComponentCount = {0};

  std::vector<TaxItem*> _taxItems;
  std::vector<PfcItem*> _pfcItems;

  void
  getTaxesSplitByFareUsage(FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage);

  const FareUsage*
  findFareUsage(const TravelSegToFareUsage& seg2fu, const TaxItem* item) const;

  TaxItem*
  getFlatTaxItem(const TaxItem* item) const;

  MoneyAmount
  getFactor(const MoneyAmount& amount, const MoneyAmount& totalConstructionAmount) const;

  TaxItem*
  getPercentageTaxItem(const TaxItem* item, const MoneyAmount& factor) const;

  void
  addTaxItemsToSplitTaxInfo(const TaxItem* item,
                            double totalConstructionAmount,
                            const FareUsage* fareUsage,
                            FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage,
                            ParentTaxToSplits& parentTaxToSplits) const;

  bool
  isFlatTax(const TaxItem* item) const;

  bool
  isPercentageTax(const TaxItem* item) const;

  const std::vector<TaxItem*>&
  taxItems() const;

  const std::vector<PfcItem*>&
  pfcItems() const;

  void
  computeTaxSummaries(FareCalc::SplitTaxInfo& taxes) const;

  void
  computeSplitTotals(FareCalc::SplitTaxInfo& taxes) const;

  int
  getFareComponentCount() const;

  TravelSegToFareUsage
  getTravelSeg2FareUsageMap(FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage) const;

  Money
  getSumOfTaxes(FareCalc::SplitTaxInfo& taxes) const;

  Money
  getTaxesBaseFare(FareCalc::SplitTaxInfo& taxes) const;

  Money
  getTaxesEquivalent(FareCalc::SplitTaxInfo& taxes) const;

  void
  fineTuneSplitsSum(ParentTaxToSplits& parentTaxToSplits) const;
};

}
