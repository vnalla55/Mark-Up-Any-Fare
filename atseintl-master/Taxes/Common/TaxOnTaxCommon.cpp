//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Taxes/Common/TaxOnTaxCommon.h"

#include "Common/TrxUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Common/TaxRound.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/Common/TaxSplitDetails.h"

namespace tse
{

TaxOnTaxCommon::TaxOnTaxCommon(bool skipTaxOnTaxIfNoFare,
                               bool requireTaxOnTaxSeqMatch,
                               bool requireTaxOnDomTaxMatch,
                               std::pair<uint16_t, uint16_t> indexRange,
                               CalculationDetails& calculationDetails,
                               TaxSplitDetails& taxSplitDetails)
    : _skipTaxOnTaxIfNoFare(skipTaxOnTaxIfNoFare),
      _requireTaxOnTaxSeqMatch(requireTaxOnTaxSeqMatch),
      _requireTaxOnDomTaxMatch(requireTaxOnDomTaxMatch),
      _indexRange(indexRange),
      _calculationDetails(calculationDetails),
      _taxSplitDetails(taxSplitDetails)
{
}

bool
TaxOnTaxCommon::useTaxOnTax(TaxTypeCode taxType,
                            const std::vector<std::string>& taxOnTaxCode,
                            Indicator taxOnTaxExcl)
{
  return taxType == PERCENTAGE && (!taxOnTaxCode.empty() || taxOnTaxExcl == YES);
}

void
TaxOnTaxCommon::calculateTaxOnTax(Trx& trx,
                                  TaxResponse& taxResponse,
                                  MoneyAmount& taxAmount,
                                  MoneyAmount& taxableFare,
                                  const NationCode& nation,
                                  const std::vector<std::string>& taxOnTaxCode,
                                  const Indicator& taxOnTaxExcl,
                                  const MoneyAmount& taxAmt,
                                  std::vector<TaxItem*>& taxOnTaxItems,
                                  bool taxOnlyTaxesFromCurrentSegment,
                                  const bool isShoppingTaxRequest)
{
  if (_skipTaxOnTaxIfNoFare && taxableFare == 0.0)
    return;

  MoneyAmount taxFromTaxItem = calculateTaxFromTaxItem(taxResponse, taxOnTaxCode,
      nation, taxOnTaxItems, taxOnlyTaxesFromCurrentSegment, isShoppingTaxRequest);

  calculateTaxAmtAndTaxableFare(
      trx,
      taxFromTaxItem,
      taxOnTaxExcl,
      taxAmount,
      taxableFare,
      taxAmt,
      isShoppingTaxRequest);
}

void
TaxOnTaxCommon::ItemCollector::collect(const std::string& taxCode, const std::vector<TaxItem*>& taxesToCheck)
{
  for (TaxItem* taxItem : taxesToCheck)
  {
    if ((taxCode == taxItem->taxCode()) && _taxOnTax.hasTaxOnTaxSeqMatched(taxItem) &&
        _taxOnTax.hasTaxOnDomTaxMatched(_taxResponse, _nation, taxItem) &&
        _taxOnTax.validateShoppingRestrictions(taxCode, _isShoppingTaxRequest))
    {
      if (_taxOnlyTaxesFromCurrentSegment)
      {
        // When tax on tax is split we apply it to other taxes only on a particular segment
        if(!(taxItem->travelSegStartIndex() == _taxOnTax.getIndexRange().first))
          continue;
      }

      if (_taxOnTax.filter().isFilteredItem(_taxResponse, *taxItem))
        continue;

      _details.taxableTaxes.push_back(std::make_pair(taxCode, taxItem->taxAmount()));
      _moneyAmount += taxItem->taxAmount();
      _taxOnTaxItems.push_back(taxItem);
    }
  }
}

MoneyAmount
TaxOnTaxCommon::calculateTaxFromTaxItem(const TaxResponse& taxResponse,
                                  const std::vector<std::string>& taxOnTaxCode,
                                  const NationCode& nation,
                                  std::vector<TaxItem*>& taxOnTaxItems,
                                  bool taxOnlyTaxesFromCurrentSegment,
                                  const bool isShoppingTaxRequest) const
{
  ItemCollector collector(taxResponse, nation, _calculationDetails,
                          taxOnTaxItems, isShoppingTaxRequest, *this, taxOnlyTaxesFromCurrentSegment);

  const bool includeExternalTaxes = (isShoppingTaxRequest && taxResponse.farePath() &&
                                     !taxResponse.farePath()->getExternalTaxes().empty());
  for (const std::string& taxCode : taxOnTaxCode)
  {
    if (includeExternalTaxes)
      collector.collect(taxCode, taxResponse.farePath()->getExternalTaxes());

    collector.collect(taxCode, taxResponse.taxItemVector());
  }

  _calculationDetails.taxableTaxSumAmount = collector.getMoneyAmount();
  return collector.getMoneyAmount();
}

void
TaxOnTaxCommon::calculateTaxAmtAndTaxableFare(Trx& trx,
                                              MoneyAmount moneyAmount,
                                              Indicator taxOnTaxExcl,
                                              MoneyAmount& taxAmount,
                                              MoneyAmount& taxableFare,
                                              const MoneyAmount& taxAmt,
                                              const bool isShoppingRequest) const
{
  if ((taxOnTaxExcl == YES) ||
      (isShoppingRequest && _taxSplitDetails.checkShoppingRestrictions()))
  {
    taxableFare = moneyAmount;
    taxAmount = taxableFare * taxAmt;
  }
  else
  {
    taxableFare += moneyAmount;
    taxAmount = taxableFare * taxAmt;
  }
}

bool
TaxOnTaxCommon::checkTaxCode(const std::string& taxCode, TaxItem* taxItem) const
{
  if (taxCode.length() == 3 && taxCode[2] == '*')
    return std::equal(taxItem->taxCode().begin(), taxItem->taxCode().begin() + 2, taxCode.begin());

  return taxCode == taxItem->taxCode();
}

bool
TaxOnTaxCommon::validateShoppingRestrictions(const std::string& taxCode,
                                             const bool isShoppingTaxRequest) const
{
  if (!isShoppingTaxRequest || !_taxSplitDetails.isTotShopRestEnabled())
    return true;

  if ((taxCode == "YQF") || (taxCode == "YQI") || (taxCode == "YQX") || (taxCode == "YRF") || (taxCode == "YRI"))
    return _taxSplitDetails.isTotShopRestIncludeBaseFare();

  for(TaxItem* taxItem : _calculationDetails.taxableTaxItems)
  {
    if (checkTaxCode(taxCode, taxItem))
      return true;
  }

  return false;
}

bool
TaxOnTaxCommon::hasTaxOnTaxSeqMatched(const TaxItem* item) const
{
  if (!_requireTaxOnTaxSeqMatch)
    return true;

  if (item->travelSegEndIndex() < _indexRange.first ||
      item->travelSegStartIndex() > _indexRange.second)
  {
    return false;
  }

  return true;
}

bool
TaxOnTaxCommon::hasTaxOnDomTaxMatched(const TaxResponse& taxResponse,
                                      const NationCode& nation,
                                      const TaxItem* item) const
{
  if (!_requireTaxOnDomTaxMatch)
    return true;

  const uint16_t& startIndex = item->travelSegStartIndex();
  const uint16_t& endIndex = item->travelSegEndIndex();

  const Itin* itin = taxResponse.farePath()->itin();

  TravelSeg* travelSeg;

  for (uint16_t index = startIndex; index <= endIndex; index++)
  {
    travelSeg = itin->travelSeg()[index];

    if (travelSeg->origin()->nation() != nation || travelSeg->destination()->nation() != nation)
    {
      return false;
    }
  }

  return true;
}
}
