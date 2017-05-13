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

#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcTaxInfo.h"
#include "FareCalc/TaxConfig.h"
#include "Taxes/Common/SplitTaxInfoProcessor.h"
#include <boost/math/special_functions/round.hpp>

namespace tse
{

namespace
{
void
roundToDecimals(const CurrencyNoDec noDec, MoneyAmount& moneyAmount)
{
  double dec = pow(10, noDec);
  moneyAmount = boost::math::round(dec * moneyAmount) / dec;
}
}

SplitTaxInfoProcessor::SplitTaxInfoProcessor(const FareCalc::FcTaxInfo& fcTaxInfo,
                                             CalcTotals& calcTotals,
                                             PricingTrx& pricingTrx,
                                             const TaxResponse* taxResponse,
                                             const FareCalcConfig* fcConfig,
                                             const CurrencyCode& taxCurrencyCode)
    : _fcTaxInfo(fcTaxInfo),
      _taxResponse(taxResponse),
      _fcConfig(fcConfig),
      _calcTotals(calcTotals),
      _itin(_calcTotals.farePath->itin()),
      _pricingTrx(pricingTrx),
      _taxCurrencyCode(taxCurrencyCode),
      _converter(nullptr),
      _fareComponentCount(getFareComponentCount()),
      _taxItems(),
      _pfcItems()
{
  CurrencyCode ignoredCurrencyCode;
  CurrencyNoDec ignoreCurrencyNoDec;
  _converter.reset(new FareCalc::FareAmountsConverter(
      &_pricingTrx, _calcTotals.farePath, _fcConfig, &_calcTotals,
      ignoredCurrencyCode, ignoreCurrencyNoDec));
}

FareCalc::FcTaxInfo::TaxesPerFareUsage
SplitTaxInfoProcessor::splitByFareComponent()
{
  FareCalc::FcTaxInfo::TaxesPerFareUsage result;
  getTaxesSplitByFareUsage(result);
  return result;
}

double
getTotalConstructionAmountForSplitting(FareCalc::FcTaxInfo::TaxesPerFareUsage& result)
{
  double totalConstructionAmount = 0.0;
  for (auto& bucket : result)
    totalConstructionAmount += bucket.second.construction.value();

  return totalConstructionAmount;
}

int
SplitTaxInfoProcessor::getFareComponentCount() const
{
  int result = 0;

  for (const PricingUnit *pricingUnit : _calcTotals.farePath->pricingUnit())
    result += pricingUnit->fareUsage().size();

  return result;
}

TravelSegToFareUsage
SplitTaxInfoProcessor::getTravelSeg2FareUsageMap(FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage) const
{
  TravelSegToFareUsage travelSeg2FareUsage;

  for (const PricingUnit * pu : _calcTotals.farePath->pricingUnit())
  {
    for (const FareUsage * fu : pu->fareUsage())
    {
      for (const TravelSeg * seg : fu->travelSeg())
        travelSeg2FareUsage.insert(std::make_pair(seg, fu));

      FareCalc::SplitTaxInfo& info = taxesPerFareUsage[fu];

      info.construction =
          Money(fu->totalFareAmount(), _calcTotals.farePath->calculationCurrency());
      info.mileage = fu->paxTypeFare()->mileage();
    }
  }

  return travelSeg2FareUsage;
}

void
SplitTaxInfoProcessor::getTaxesSplitByFareUsage(FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage)
{
  TravelSegToFareUsage travelSeg2FareUsage = getTravelSeg2FareUsageMap(taxesPerFareUsage);
  const double totalConstructionAmount = getTotalConstructionAmountForSplitting(taxesPerFareUsage);

  ParentTaxToSplits parentTaxToSplits;

  for (const TaxItem *item : taxItems())
  {
    const FareUsage* fareUsage = findFareUsage(travelSeg2FareUsage, item);
    addTaxItemsToSplitTaxInfo(item, totalConstructionAmount, fareUsage, taxesPerFareUsage, parentTaxToSplits);
  }

  fineTuneSplitsSum(parentTaxToSplits);

  for (const PfcItem *item : pfcItems())
  {
    TSE_ASSERT(item->travelSeg() != nullptr);

    const FareUsage* fareUsage = travelSeg2FareUsage[item->travelSeg()];
    taxesPerFareUsage[fareUsage].pfcItems.push_back(item);
  }

  for (auto& pair : taxesPerFareUsage)
  {
    computeTaxSummaries(pair.second);
    computeSplitTotals(pair.second);
  }
}

const FareUsage*
SplitTaxInfoProcessor::findFareUsage(const TravelSegToFareUsage& seg2fu, const TaxItem* item) const
{
  const uint16_t segmentIndex = item->travelSegStartIndex();
  TSE_ASSERT(segmentIndex < _itin->travelSeg().size());

  const TravelSeg* itemsSegment = _itin->travelSeg()[segmentIndex];
  TravelSegToFareUsage::const_iterator i = seg2fu.find(itemsSegment);
  TSE_ASSERT(i != seg2fu.end());

  return i->second;
}

TaxItem*
SplitTaxInfoProcessor::getFlatTaxItem(const TaxItem* item) const
{
  assert(_fareComponentCount != 0);

  TaxItem* newItem(&_pricingTrx.dataHandle().safe_create<TaxItem>(*item));
  newItem->taxAmount() = item->taxAmount() / _fareComponentCount;
  roundToDecimals(newItem->taxNodec(), newItem->taxAmount());

  return newItem;
}

MoneyAmount
SplitTaxInfoProcessor::getFactor(const MoneyAmount& amount, const MoneyAmount& totalConstructionAmount) const
{
  if (!Money::isZeroAmount(totalConstructionAmount))
  {
    return amount / totalConstructionAmount;
  }
  else
  {
    return 0.0;
  }
}

TaxItem*
SplitTaxInfoProcessor::getPercentageTaxItem(const TaxItem* item, const MoneyAmount& factor) const
{
  TaxItem* newItem(&_pricingTrx.dataHandle().safe_create<TaxItem>(*item));
  newItem->taxAmount() = item->taxAmount() * factor;
  roundToDecimals(newItem->taxNodec(), newItem->taxAmount());

  return newItem;
}

void
SplitTaxInfoProcessor::addTaxItemsToSplitTaxInfo(const TaxItem* item,
                                                 double totalConstructionAmount,
                                                 const FareUsage* fareUsage,
                                                 FareCalc::FcTaxInfo::TaxesPerFareUsage& taxesPerFareUsage,
                                                 ParentTaxToSplits& parentTaxToSplits) const
{
  if (isFlatTax(item))
  {
    for (auto& each : taxesPerFareUsage)
    {
      TaxItem* newItem = getFlatTaxItem(item);
      each.second.taxItems.push_back(newItem);
      parentTaxToSplits[item].push_back(newItem);
    }
  }
  else if (isPercentageTax(item))
  {
    for (auto& each : taxesPerFareUsage)
    {
      MoneyAmount factor = getFactor(each.second.construction.value(), totalConstructionAmount);

      TaxItem* newItem = getPercentageTaxItem(item, factor);
      each.second.taxItems.push_back(newItem);
      parentTaxToSplits[item].push_back(newItem);
    }
  }
  else
  {
    taxesPerFareUsage[fareUsage].taxItems.push_back(item);
  }
}

bool
SplitTaxInfoProcessor::isFlatTax(const TaxItem* item) const
{
  if (item->taxCode().equalToConst("UO2"))
  {
    return item->taxType() == 'F';
  }

  return FareCalcTaxConfig::flatRateTaxes.getValue().has(item->taxCode());
}

bool
SplitTaxInfoProcessor::isPercentageTax(const TaxItem* item) const
{
  if (item->taxCode().equalToConst("UO2"))
  {
    return item->taxType() == 'P';
  }

  return FareCalcTaxConfig::fareAmountTaxes.getValue().has(item->taxCode());
}

const std::vector<TaxItem*>&
SplitTaxInfoProcessor::taxItems() const
{
  if (_taxResponse != nullptr)
    return _taxResponse->taxItemVector();

  return _taxItems;
}

const std::vector<PfcItem*>&
SplitTaxInfoProcessor::pfcItems() const
{
  if (_taxResponse != nullptr)
    return _taxResponse->pfcItemVector();

  return _pfcItems;
}

void
gatherTaxExempts(FareCalc::SplitTaxInfo& taxes)
{
  for (const TaxItem * item : taxes.taxItems)
  {
    if (item->failCode() == TaxItem::EXEMPT_ALL_TAXES ||
        item->failCode() == TaxItem::EXEMPT_SPECIFIED_TAXES)
    {
      taxes.taxExempts.push_back(item->taxCode().substr(0, 2));
    }
  }

  std::sort(taxes.taxExempts.begin(), taxes.taxExempts.end());
  taxes.taxExempts.erase(std::unique(taxes.taxExempts.begin(), taxes.taxExempts.end()),
                         taxes.taxExempts.end());
}

void
SplitTaxInfoProcessor::computeTaxSummaries(FareCalc::SplitTaxInfo& taxes) const
{
  TaxResponse taxResponse;
  taxResponse.farePath() = const_cast<FarePath*>(_calcTotals.farePath);

  for (const TaxItem *item : taxes.taxItems)
  {
    taxResponse.taxItemVector().push_back(const_cast<TaxItem*>(item));
  }

  for (const PfcItem *item : taxes.pfcItems)
  {
    taxResponse.pfcItemVector().push_back(const_cast<PfcItem*>(item));
  }

  TaxRecord tr;
  tr.buildTicketLine(_pricingTrx, taxResponse, false, true);

  taxes.taxRecords.assign(taxResponse.taxRecordVector().begin(),
                          taxResponse.taxRecordVector().end());

  gatherTaxExempts(taxes);
}

Money
SplitTaxInfoProcessor::getSumOfTaxes(FareCalc::SplitTaxInfo& taxes) const
{
  Money sum(_taxCurrencyCode);
  for (const TaxRecord * record : taxes.taxRecords)
    sum.value() += record->getTaxAmount();

  return sum;
}

Money
SplitTaxInfoProcessor::getTaxesBaseFare(FareCalc::SplitTaxInfo& taxes) const
{
  if (_calcTotals.convertedBaseFareCurrencyCode == taxes.construction.code())
  {
    return taxes.construction;
  }
  else
  {
    return _converter->roundUp(_converter->convertConstructionToBaseFare(taxes.construction, false));
  }
}

Money
SplitTaxInfoProcessor::getTaxesEquivalent(FareCalc::SplitTaxInfo& taxes) const
{
  if (taxes.baseFare.code() == TrxUtil::getEquivCurrencyCode(_pricingTrx))
  {
    return taxes.baseFare;
  }
  else
  {
    return _converter->roundUp(_converter->convertBaseFareToEquivalent(taxes.baseFare));
  }
}

void
SplitTaxInfoProcessor::computeSplitTotals(FareCalc::SplitTaxInfo& taxes) const
{
  taxes.baseFare = getTaxesBaseFare(taxes);
  taxes.equivalent = getTaxesEquivalent(taxes);
  taxes.totalTax = getSumOfTaxes(taxes);
  taxes.total = taxes.totalTax + taxes.equivalent;
}

void
SplitTaxInfoProcessor::fineTuneSplitsSum(ParentTaxToSplits& parentTaxToSplits) const
{
  for (auto& eachTax : parentTaxToSplits)
  {
    MoneyAmount sumOfSplits = 0;
    for (auto& eachSplit : eachTax.second)
    {
      sumOfSplits += eachSplit->taxAmount();
    }

    MoneyAmount diff = eachTax.first->taxAmount() - sumOfSplits;
    eachTax.second.back()->taxAmount() += diff;
  }
}

}
