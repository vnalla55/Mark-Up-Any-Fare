#include "FareCalc/FcTaxInfo.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/TaxCodeReg.h"
#include "FareCalc/AccumulateAmount.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareAmountsConverter.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FcDispItem.h"
#include "FareCalc/FcUtil.h"
#include "FareCalc/PercentageComputator.h"
#include "FareCalc/TaxConfig.h"
#include "Taxes/Common/SplitTaxInfoProcessor.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

#include <algorithm>

using namespace tse::FareCalc;

namespace tse
{

FALLBACK_DECL(fallbackFixForRTPricingInSplit);

static Logger
logger("atseintl.FareCalc.FcTaxInfo");

void
FcTaxInfo::compute(TaxInfo& taxInfo) const
{
  collectZpTaxInfo(taxInfo);

  bool xtOutput = false;
  if (_trx->getRequest()->isTicketEntry())
  {
    xtOutput = collectTicketingTaxInfo(taxInfo);
  }

  collectXfTaxInfo(taxInfo, xtOutput);
}

template <typename TaxItems, typename ExemptOutputIter>
void
gatherTaxExempts(const TaxItems& taxItems, ExemptOutputIter output)
{
  for (const TaxItem* item : taxItems)
  {
    if (item->failCode() == TaxItem::EXEMPT_ALL_TAXES ||
        item->failCode() == TaxItem::EXEMPT_SPECIFIED_TAXES)
    {
      *output++ = item->taxCode().substr(0, 2);
    }
  }
}

void
FcTaxInfo::initialize(PricingTrx* trx,
                      CalcTotals* calcTotals,
                      const FareCalcConfig* fcConfig,
                      const TaxResponse* taxResponse)
{
  _trx = trx;
  _calcTotals = calcTotals;
  _fcConfig = fcConfig;
  _taxResponse = taxResponse;

  _taxAmount = getTaxOverride();

  LOG4CXX_DEBUG(logger, "tax override: " << _taxAmount);

  if (taxResponse == nullptr)
  {
    _taxNoDec = calcTotals->equivNoDec;
    _taxCurrencyCode = calcTotals->equivCurrencyCode;
  }
  else
  {
    const TaxResponse::TaxRecordVector& taxRecord = taxResponse->taxRecordVector();

    AccumulateAmount<TaxRecord> accTaxAmount(*_trx,
                                             _taxAmount,
                                             _supplementalFeeAmount,
                                             _trx->getOptions()->currencyOverride(),
                                             &TaxRecord::getTaxAmount);

    std::for_each(taxRecord.begin(), taxRecord.end(), accTaxAmount);

    LOG4CXX_DEBUG(logger, "total tax: " << _taxAmount);

    _taxNoDec = calcTotals->equivNoDec;
    _taxCurrencyCode = calcTotals->equivCurrencyCode;
    if (taxRecord.size() > 0)
    {
      _taxNoDec = taxRecord.front()->taxNoDec();
      _taxCurrencyCode = taxRecord.front()->taxCurrencyCode();
    }

    for (const auto elem : taxResponse->taxItemVector())
    {
      if (UNLIKELY(elem->specialProcessNo() == 64))
      {
        _dispSegmentFeeMsg = true;
      }
    }

    gatherTaxExempts(taxResponse->taxItemVector(),
                     std::inserter(_taxExemptCodes, _taxExemptCodes.end()));

    computeSplitTaxInfo4Pricing();
  }
}

const std::vector<TaxRecord*>&
FcTaxInfo::taxRecords() const
{
  if (_taxResponse != nullptr && _taxResponse->taxRecordVector().empty() == false &&
      _taxRecords.empty() == true)
  {
    std::remove_copy_if(_taxResponse->taxRecordVector().begin(),
                        _taxResponse->taxRecordVector().end(),
                        std::back_inserter(_taxRecords),
                        FcTaxInfo::Zerop());
  }

  return _taxRecords;
}

const std::vector<TaxItem*>&
FcTaxInfo::taxItems() const
{
  if (_taxResponse != nullptr)
    return _taxResponse->taxItemVector();

  return _taxItems;
}

const std::vector<TaxItem*>&
FcTaxInfo::changeFeeTaxItems() const
{
  if (_taxResponse != nullptr)
    return _taxResponse->changeFeeTaxItemVector();

  return _changeFeeTaxItems;
}

const std::vector<PfcItem*>&
FcTaxInfo::pfcItems() const
{
  if (_taxResponse != nullptr)
    return _taxResponse->pfcItemVector();

  return _pfcItems;
}

MoneyAmount
FcTaxInfo::getTaxOverride()
{
  _taxOverride = 0;

  std::for_each(_trx->getRequest()->taxOverride().begin(),
                _trx->getRequest()->taxOverride().end(),
                FareCalc::Accumulate<TaxOverride>(
                    std::mem_fun<const MoneyAmount&>(&TaxOverride::taxAmt), _taxOverride));

  return _taxOverride;
}

void
FcTaxInfo::collectZpTaxInfo(TaxInfo& taxInfo) const
{
  if (_taxResponse == nullptr)
    return;

  const std::vector<TaxItem*>& taxItems = _taxResponse->taxItemVector();

  for (const auto taxItem : taxItems)
  {
    if ((taxItem && taxItem->taxCode() != FareCalcConsts::TAX_CODE_ZP) || taxItem->failCode() != 0)
    {
      continue;
    }

    std::string zpTax = taxItem->taxLocalBoard();
    std::string publishedZpTax = taxItem->taxLocalBoard();

    if (_fcConfig->zpAmountDisplayInd() == FareCalcConsts::FC_YES || taxItem->taxAmount() < EPSILON)
    {
      if (taxItem->taxAmount() < EPSILON)
      {
        zpTax.append("0");
        publishedZpTax.append("0");
      }
      else
      {
        zpTax.append(FcDispItem::convertAmount(taxItem->taxAmount(), _taxNoDec, 0));
        publishedZpTax.append(FcDispItem::convertAmount(taxItem->taxAmt(), taxItem->taxNodec(), 0));
      }
    }

    taxInfo.zpTaxInfo.push_back(zpTax);
    taxInfo.publishedZpTaxInfo.push_back(publishedZpTax);
  }
}

void
FcTaxInfo::collectXfTaxInfo(TaxInfo& taxInfo, bool xtOutput) const
{
  if (_taxResponse == nullptr || _taxResponse->pfcItemVector().empty())
    return;

  // Ticketing Total XF tax info
  if (_trx->getRequest()->isTicketEntry())
  {
    std::vector<TaxRecord*>::const_iterator taxRecordI = _taxResponse->taxRecordVector().begin();
    MoneyAmount totalXfAmount = 0.0;
    for (; taxRecordI != _taxResponse->taxRecordVector().end(); ++taxRecordI)
    {
      if ((*taxRecordI)->taxCode() == FareCalcConsts::TAX_CODE_XF)
      {
        totalXfAmount = (*taxRecordI)->getTaxAmount();
        break;
      }
    }

    CurrencyCode paymentCurrency = _trx->getRequest()->ticketingAgent()->currencyCodeAgent();
    if (!_trx->getOptions()->currencyOverride().empty())
    {
      paymentCurrency = _trx->getOptions()->currencyOverride();
    }

    Money money(paymentCurrency);

    CurrencyNoDec noDec = money.noDec(_trx->ticketingDate());

    std::string xfInfo = FcDispItem::convertAmount(totalXfAmount, noDec, noDec);
    xfInfo.append(FareCalcConsts::TAX_CODE_XF);
    taxInfo.xfTaxInfo.push_back(xfInfo);
  }
  else
  {
    taxInfo.xfTaxInfo.push_back(FareCalcConsts::TAX_CODE_XF);
  }

  std::vector<PfcItem*>::const_iterator pfcItemI = _taxResponse->pfcItemVector().begin();
  const Itin& _itin = *_calcTotals->farePath->itin();

  for (; pfcItemI != _taxResponse->pfcItemVector().end(); pfcItemI++)
  {
    std::string xfTax = (*pfcItemI)->pfcAirportCode();

    if (_trx->getRequest()->isTicketEntry())
    {
      Money money((*pfcItemI)->pfcCurrencyCode());
      xfTax.append(FcDispItem::convertAmount(
          (*pfcItemI)->pfcAmount(), money.noDec(_trx->ticketingDate()), 0));
    }
    else
    {
      // ??? Why do we need to do it differently for non-ticketing entry ???
      MoneyAmount xfTaxAmount = (*pfcItemI)->pfcAmount();
      if (LIKELY((*pfcItemI)->pfcCurrencyCode() != _itin.calculationCurrency()))
      {
        CurrencyConversionFacade ccFacade;

        const Money sourceMoney((*pfcItemI)->pfcAmount(), (*pfcItemI)->pfcCurrencyCode());
        Money targetMoney(_itin.calculationCurrency());

        if (!ccFacade.convert(
                targetMoney, sourceMoney, *_trx, false, CurrencyConversionRequest::TAXES))
        {
          xfTaxAmount = targetMoney.value();
        }
      }

      if (xfTaxAmount < EPSILON)
      {
        xfTax.append("0");
      }
      else
      {
        xfTax.append(FcDispItem::convertAmount(xfTaxAmount, _calcTotals->fclNoDec, 0));
      }
    }

    taxInfo.xfTaxInfo.push_back(xfTax);
  }

  { // Save fclXF for ticketing logic:
    std::ostringstream os;
    std::copy(
        taxInfo.xfTaxInfo.begin(), taxInfo.xfTaxInfo.end(), std::ostream_iterator<std::string>(os));
    _calcTotals->fclXF = os.str();
  }
}

bool
FcTaxInfo::collectTicketingTaxInfo(TaxInfo& taxInfo) const
{
  bool firstTax = true;
  bool chargedTaxXF = false;
  int16_t rollCount = _trx->getRequest()->numberTaxBoxes();
  bool xtOutput = false;
  std::string _tempFareCalcItem;

  if (_taxResponse != nullptr)
  {
    std::vector<TaxRecord*>::const_iterator taxRecordI;

    // Go through Exempted Taxes first
    if (_trx->getRequest()->isExemptSpecificTaxes() || _trx->getRequest()->isExemptAllTaxes())
    {
      bool isLastDispExempt = false;

      std::set<TaxCode>::const_iterator taxCodeIter = _taxExemptCodes.begin();
      const std::set<TaxCode>::const_iterator taxCodeIterEnd = _taxExemptCodes.end();
      for (; taxCodeIter != taxCodeIterEnd; taxCodeIter++)
      {
        if (firstTax)
        {
          _calcTotals->fclXT = " " + FareCalcConsts::TAX_CODE_XT;
          firstTax = false;
          xtOutput = true;
        }
        if (!isLastDispExempt)
        {
          _tempFareCalcItem = " EXEMPT "; // tkt never use EX
          isLastDispExempt = true;
        }
        else
        {
          _tempFareCalcItem = "-";
        }

        _calcTotals->fclXT += _tempFareCalcItem;
        taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);

        _tempFareCalcItem = *taxCodeIter;
        _calcTotals->fclXT += _tempFareCalcItem;
        taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);
      }
    }

    // Go through Non-exempted Taxes
    for (taxRecordI = _taxResponse->taxRecordVector().begin();
         taxRecordI != _taxResponse->taxRecordVector().end();
         ++taxRecordI)
    {
      if ((*taxRecordI)->taxCode() == FareCalcConsts::TAX_CODE_XF)
      {
        chargedTaxXF = true;
        continue;
      }

      if (((*taxRecordI)->taxCode() == FareCalcConsts::TAX_CODE_ZP) ||
          ((*taxRecordI)->taxRolledXTInd() != FareCalcConsts::FC_YES))
        continue;

      if ((*taxRecordI)->isTaxFeeExempt())
        continue;

      if ((*taxRecordI)->getTaxAmount() < EPSILON)
        continue;

      if (firstTax)
      {
        _calcTotals->fclXT = " " + FareCalcConsts::TAX_CODE_XT;
        firstTax = false;
        xtOutput = true;
      }

      _tempFareCalcItem = FcDispItem::convertAmount(
          (*taxRecordI)->getTaxAmount(), (*taxRecordI)->taxNoDec(), (*taxRecordI)->taxNoDec());
      _calcTotals->fclXT += _tempFareCalcItem;
      taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);

      _tempFareCalcItem = (*taxRecordI)->taxCode().substr(0, 2);
      _calcTotals->fclXT += _tempFareCalcItem;
      taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);
    }

    if (chargedTaxXF && rollCount > 0)
      rollCount--;
  }

  // SPR 12021 Additional XT for PQ display for Override Taxes

  std::vector<TaxOverride*>::const_iterator taxOvIter = _trx->getRequest()->taxOverride().begin();
  std::vector<TaxOverride*>::const_iterator taxOvEndIter = _trx->getRequest()->taxOverride().end();
  Money money(_calcTotals->taxCurrencyCode());

  if ((int16_t)_trx->getRequest()->taxOverride().size() <= rollCount)
    return xtOutput;

  for (; taxOvIter != taxOvEndIter; taxOvIter++, rollCount--)
  {
    if (!xtOutput && rollCount > 1)
      continue;

    if ((*taxOvIter)->taxAmt() < EPSILON)
      continue;

    if (firstTax)
    {
      _calcTotals->fclXT = " " + FareCalcConsts::TAX_CODE_XT;
      firstTax = false;
      xtOutput = true;
    }

    _tempFareCalcItem =
        FcDispItem::convertAmount((*taxOvIter)->taxAmt(), money.noDec(_trx->ticketingDate()));
    _calcTotals->fclXT += _tempFareCalcItem;
    taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);

    _tempFareCalcItem = (*taxOvIter)->taxCode().substr(0, 2);
    _calcTotals->fclXT += _tempFareCalcItem;
    taxInfo.xtTaxInfo.push_back(_tempFareCalcItem);
  }

  return xtOutput;
}

template <typename Result>
double
getTotalConstructionAmountForSplitting(const Result& result)
{
  double totalConstructionAmount = 0.0;
  for (const typename Result::value_type& bucket : result)
  {
    totalConstructionAmount += bucket.second.construction.value();
  }
  return totalConstructionAmount;
}

void
FcTaxInfo::getTaxesSplitByLeg(TaxesPerLeg& result) const
{

  if (_calcTotals->farePath->isAnyFareUsageAcrossTurnaroundPoint())
  {
    calculateEstimatedBaseFaresForLegs(result);
  }
  else
  {
    calculateRealBaseFaresForLegs(result);
  }

  const int legCount = numberOfLegs();
  TSE_ASSERT(legCount > 0);

  const double totalConstructionAmount = getTotalConstructionAmountForSplitting(result);
  for (const TaxItem* item : taxItems())
  {
    assignTaxItemToResult(
        item, result, totalConstructionAmount, legCount, std::mem_fun(&TaxItem::legId));
  }

  for (const PfcItem* item : pfcItems())
  {
    result[item->legId()].pfcItems.push_back(item);
  }

  for (TaxesPerLeg::value_type& pair : result)
  {
    computeTaxSummariesAndTotals(pair.second);
  }

  if (_trx->hasPriceDynamicallyDeviated())
    calculateEffectivePriceDeviations(result);
}

struct FareUsageForLeg
{
  typedef std::map<const TravelSeg*, const FareUsage*> SegmentToFareUsage;

  const Itin* itin;
  const SegmentToFareUsage& seg2fu;
  std::function<const FareUsage*(const TaxItem*)> strategy;

  FareUsageForLeg(const Itin* _itin, const SegmentToFareUsage& _seg2fu, bool isOriginBasedRTPricing)
    : itin(_itin), seg2fu(_seg2fu)
  {
    strategy = isOriginBasedRTPricing ? originBasedPricingStrategy : regularStrategy;
  }

  const FareUsage* operator()(const TaxItem* item)
  {
    return strategy(item);
  }

  private:

  std::function<const FareUsage*(const TaxItem*)> regularStrategy = [&](const TaxItem* item)
  {
    // SEGMENT TAX
    const uint16_t segmentIndex = item->travelSegStartIndex();
    TSE_ASSERT(segmentIndex < itin->travelSeg().size());

    const TravelSeg* itemsSegment = itin->travelSeg()[segmentIndex];
    SegmentToFareUsage::const_iterator i = seg2fu.find(itemsSegment);
    TSE_ASSERT(i != seg2fu.end());

    return i->second;
  };

  std::function<const FareUsage*(const TaxItem*)> originBasedPricingStrategy = [&](const TaxItem* item)
  {
    TSE_ASSERT(itin != nullptr);
    const TravelSeg* itemsSegment = item->travelSegStart();

    TSE_ASSERT(itemsSegment != nullptr);

    SegmentToFareUsage::const_iterator i = seg2fu.find(itemsSegment);
    TSE_ASSERT(i != seg2fu.end());

    return i->second;
  };

};

template <typename Result, typename BucketFunctor>
void
FcTaxInfo::assignTaxItemToResult(const TaxItem* item,
                                 Result& result,
                                 double totalConstructionAmount,
                                 int splitBy,
                                 BucketFunctor getBucket) const
{
  if (isFlatTax(item))
  {
    // FLAT TAX
    TaxItem* newItem(&_trx->dataHandle().safe_create<TaxItem>(*item));
    newItem->taxAmount() = item->taxAmount() / splitBy;

    for (typename Result::value_type& bucket : result)
    {
      bucket.second.taxItems.push_back(newItem);
    }
  }
  else if (isPercentageTax(item))
  {
    // PERCENTAGE TAX
    for (typename Result::value_type& bucket : result)
    {
      TaxItem* newItem(&_trx->dataHandle().safe_create<TaxItem>(*item));

      double factor;
      if (!Money::isZeroAmount(totalConstructionAmount))
      {
        factor = bucket.second.construction.value() / totalConstructionAmount;
      }
      else
      {
        factor = 0.0;
      }

      newItem->taxAmount() = item->taxAmount() * factor;

      bucket.second.taxItems.push_back(newItem);
    }
  }
  else
  {
    // SEGMENT TAX
    const bool brandedHalfRTPricing = _trx->getRequest() &&
                                      _trx->getRequest()->originBasedRTPricing() &&
                                      _trx->getRequest()->isBrandedFaresRequest();

    if (!fallback::fallbackFixForRTPricingInSplit(_trx) && brandedHalfRTPricing)
    {
      // in origin based round trip pricing we don't want to process taxes on a dummy leg
      // especially that this leg has already been removed at this point of processing
      const TravelSeg* travelSegStart = item->travelSegStart();
      TSE_ASSERT(travelSegStart != nullptr);
      if (!travelSegStart->toAirSeg()->isFake())
        result[getBucket(item)].taxItems.push_back(item);
    }
    else
    {
      result[getBucket(item)].taxItems.push_back(item);
    }
  }
}

void
FcTaxInfo::getTaxesSplitByFareUsage(TaxesPerFareUsage& result) const
{
  typedef std::map<const TravelSeg*, const FareUsage*> SegmentToFareUsage;
  SegmentToFareUsage seg2fu;

  // (LegId, Airport) -> Segment
  typedef std::pair<uint16_t, LocCode> PfcItemKey;
  typedef std::map<PfcItemKey, const FareUsage*> PfcToFareUsage;
  PfcToFareUsage pfc2fu;

  int fareComponentCount = 0;

  for (const PricingUnit* pu : _calcTotals->farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const TravelSeg* seg : fu->travelSeg())
      {
        seg2fu.insert(std::make_pair(seg, fu));
        pfc2fu.insert(std::make_pair(std::make_pair(seg->legId(), seg->origAirport()), fu));
      }

      fareComponentCount++;

      SplitTaxInfo& info = result[fu];

      info.construction =
          Money(fu->totalFareAmount(), _calcTotals->farePath->calculationCurrency());
      info.mileage = fu->paxTypeFare()->mileage();

      if (_trx->hasPriceDynamicallyDeviated())
        info.effectiveDeviation =
            Money(-fu->getDiscAmount(), _calcTotals->farePath->calculationCurrency());
    }
  }

  const double totalConstructionAmount = getTotalConstructionAmountForSplitting(result);
  const Itin* itin = _calcTotals->farePath->itin();

  const bool useSpecialProcessingForOriginBasedRTPricing =
     _trx->getRequest()->originBasedRTPricing() && !fallback::fallbackFixForRTPricingInSplit(_trx);

  for (TaxItem* item : taxItems())
  {
    assignTaxItemToResult(
        item, result, totalConstructionAmount, fareComponentCount,
           FareUsageForLeg(itin, seg2fu, useSpecialProcessingForOriginBasedRTPricing));
  }

  for (const PfcItem* item : pfcItems())
  {
    PfcToFareUsage::const_iterator i =
        pfc2fu.find(std::make_pair(item->legId(), item->pfcAirportCode()));
    TSE_ASSERT(i != pfc2fu.end());

    result[i->second].pfcItems.push_back(item);
  }

  for (TaxesPerFareUsage::value_type& pair : result)
  {
    computeTaxSummariesAndTotals(pair.second);
  }
}

void
FcTaxInfo::computeTaxSummaries(SplitTaxInfo& taxes) const
{
  // buildTicketLine does not modify TaxItem, PfcItem and FarePath but they are stored as non-const
  // pointers in TaxResponse, I found no nice way to pass them in without these casts.
  TaxResponse taxResponse;
  taxResponse.farePath() = const_cast<FarePath*>(_calcTotals->farePath);

  for (const TaxItem* item : taxes.taxItems)
  {
    taxResponse.taxItemVector().push_back(const_cast<TaxItem*>(item));
  }

  for (const PfcItem* item : taxes.pfcItems)
  {
    taxResponse.pfcItemVector().push_back(const_cast<PfcItem*>(item));
  }

  TaxRecord tr;
  tr.buildTicketLine(*_trx, taxResponse, false, true); // I have no idea what 'false, true' means...
  // I've copied it from another place.

  taxes.taxRecords.assign(taxResponse.taxRecordVector().begin(),
                          taxResponse.taxRecordVector().end());

  gatherTaxExempts(taxes.taxItems, std::back_inserter(taxes.taxExempts));
  std::sort(taxes.taxExempts.begin(), taxes.taxExempts.end());
  taxes.taxExempts.erase(std::unique(taxes.taxExempts.begin(), taxes.taxExempts.end()),
                         taxes.taxExempts.end());
}

void
FcTaxInfo::computeSplitTotals(SplitTaxInfo& taxes) const
{
  Money sum(taxCurrencyCode());
  for (const TaxRecord* record : taxes.taxRecords)
  {
    sum.value() += record->getTaxAmount();
  }

  CurrencyCode ignore1;
  CurrencyNoDec ignore2;
  FareAmountsConverter converter(
      _trx, _calcTotals->farePath, _fcConfig, _calcTotals, ignore1, ignore2);

  // No need to convert currencies
  if (_calcTotals->convertedBaseFareCurrencyCode == taxes.construction.code())
  {
    taxes.baseFare = taxes.construction;
  }
  else
  {
    taxes.baseFare =
        converter.roundUp(converter.convertConstructionToBaseFare(taxes.construction, false));
  }

  // No need to convert currencies
  if (taxes.baseFare.code() == TrxUtil::getEquivCurrencyCode(*_trx))
  {
    taxes.equivalent = taxes.baseFare;
  }
  else
  {
    taxes.equivalent =
        converter.roundUp(converter.convertBaseFareToEquivalent(taxes.baseFare, false));
  }

  taxes.totalTax = sum;

  taxes.total = taxes.totalTax + taxes.equivalent;

  if (_trx->hasPriceDynamicallyDeviated())
    taxes.effectiveDeviation = taxes.equivalent -
                               converter.convertConstructionToEquivalent(
                                   taxes.construction.value() - taxes.effectiveDeviation.value());
}

void
FcTaxInfo::computeTaxSummariesAndTotals(SplitTaxInfo& taxes) const
{
  computeTaxSummaries(taxes);
  computeSplitTotals(taxes);
}

int
FcTaxInfo::numberOfLegs() const
{
  int result = 0;
  for (const PricingUnit* pu : _calcTotals->farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const TravelSeg* seg : fu->travelSeg())
      {
        if (seg->legId() >= result)
          result = seg->legId() + 1;
      }
    }
  }
  return result;
}

bool
FcTaxInfo::isFlatTax(const TaxItem* item) const
{
  return FareCalcTaxConfig::flatRateTaxes.getValue().has(item->taxCode());
}

bool
FcTaxInfo::isPercentageTax(const TaxItem* item) const
{
  return FareCalcTaxConfig::fareAmountTaxes.getValue().has(item->taxCode());
}

void
FcTaxInfo::calculateRealBaseFaresForLegs(TaxesPerLeg& result) const
{
  const CurrencyCode& calcCurrency = _calcTotals->farePath->calculationCurrency();

  for (const PricingUnit* pu : _calcTotals->farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      int16_t legId = fu->travelSeg().front()->legId();
      SplitTaxInfo& bucket = result[legId];

      bucket.construction =
          Money(bucket.construction.value() + fu->totalFareAmount(), calcCurrency);
      bucket.mileage += fu->paxTypeFare()->mileage();
    }
  }
}

void
FcTaxInfo::calculateEstimatedBaseFaresForLegs(TaxesPerLeg& result) const
{
  const CurrencyCode& calcCurrency = _calcTotals->farePath->calculationCurrency();

  for (const PricingUnit* pu : _calcTotals->farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      // if fare component is within the leg - just add it's amount to the leg it's part of
      if (!fu->isAcrossTurnaroundPoint())
      {
        int16_t legId = fu->travelSeg().front()->legId();
        SplitTaxInfo& bucket = result[legId];

        bucket.construction =
          Money(bucket.construction.value() + fu->totalFareAmount(), calcCurrency);
        bucket.mileage += fu->paxTypeFare()->mileage();
      }
      else
      {
        // Split fare and mileage depending on the mileage of individual travel segments that fall into each leg
        PercentageComputator<int16_t, double> fareSplitter(fu->totalFareAmount());
        PercentageComputator<int16_t, int> mileageSplitter(fu->paxTypeFare()->mileage());

        for (const TravelSeg* seg : fu->travelSeg())
        {
          const Loc* loc1 = seg->origin();
          const Loc* loc2 = seg->destination();
          uint32_t segmentMileage = LocUtil::getTPM(
              *loc1, *loc2, GlobalDirection::ZZ, seg->departureDT(), _trx->dataHandle());
          fareSplitter.addToKey(seg->legId(), segmentMileage);
          mileageSplitter.addToKey(seg->legId(), segmentMileage);
        }
        // Read collected split data and put it into each leg's bucket
        typedef std::pair<int16_t, double> AmountPerLeg;
        for (const AmountPerLeg& amountPerLeg : fareSplitter.getOutput())
        {
          SplitTaxInfo& bucket = result[amountPerLeg.first];
          bucket.construction =
            Money(bucket.construction.value() + amountPerLeg.second, calcCurrency);
          bucket.mileage += mileageSplitter.getOutputForKey(amountPerLeg.first);
        }
      }
    }
  }
}

void
FcTaxInfo::computeSplitTaxInfo4Pricing()
{
  if (LIKELY(_trx->getTrxType() != PricingTrx::PRICING_TRX))
    return;

  if (TrxUtil::isSplitTaxesByFareComponentEnabled(*_trx) &&
      _trx->getOptions()->isSplitTaxesByFareComponent())
  {
    SplitTaxInfoProcessor splitProcessor(*this, *_calcTotals, *_trx,
        _taxResponse, _fcConfig, _taxCurrencyCode);
    _fareUsage2SplitTaxInfo4Pricing = splitProcessor.splitByFareComponent();
  }
}

void
FcTaxInfo::calculateEffectivePriceDeviations(TaxesPerLeg& result) const
{
  CurrencyCode ignore1;
  CurrencyNoDec ignore2;
  const FarePath* fp = _calcTotals->farePath;
  FareAmountsConverter cnv(_trx, fp, _fcConfig, _calcTotals, ignore1, ignore2);
  for (auto& taxesPerLeg : result)
    taxesPerLeg.second.effectiveDeviation =
        taxesPerLeg.second.equivalent -
        cnv.convertConstructionToEquivalent(taxesPerLeg.second.construction.value() -
                                            fp->getDynamicPriceDeviationForLeg(taxesPerLeg.first));
}
}
