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

#include "Taxes/LegacyTaxes/TaxXS_00.h"

#include "Common/TrxUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace tse;
using namespace std;


namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxXS_00>
{
public:
  TaxOnTaxCollector(TaxXS_00& tax,
      CalculationDetails& details,
      std::vector<TaxItem*>& taxOnTaxItems,
      const std::vector<uint16_t>& validSegs)
    : Base(tax)
    , _moneyAmount(0.0)
    , _details(details)
    , _taxOnTaxItems(taxOnTaxItems)
    , _validSegs(validSegs)
    {}

  MoneyAmount getMoneyAmount() const { return _moneyAmount; }

  void collect(const TaxResponse::TaxItemVector& taxItemVec, const TaxCode& taxCode)
  {
    for (TaxItem* taxItem : taxItemVec)
    {
      if (taxCode == taxItem->taxCode())
      {
        if ( std::none_of(_validSegs.begin(), _validSegs.end(),
             [taxItem](int id)
             {return id >= taxItem->travelSegStartIndex() && id <= taxItem->travelSegEndIndex();}
           ))
          continue;

        _details.taxableTaxes.push_back(std::make_pair(taxItem->taxCode(), taxItem->taxAmount()));
        _moneyAmount += taxItem->taxAmount();
        _taxOnTaxItems.push_back(taxItem);
      }
    }
  }

protected:
  MoneyAmount                   _moneyAmount;
  CalculationDetails&           _details;
  std::vector<TaxItem*>&        _taxOnTaxItems;
  const std::vector<uint16_t>&  _validSegs;
};

} //namespace

bool
TaxXS_00::isDomesticSegment(const TravelSeg& travelSeg, const TaxCodeReg& taxCodeReg) const
{
  if (!travelSeg.isAir())
    return false;

  return (travelSeg.origin()->nation() == taxCodeReg.nation() &&
    travelSeg.destination()->nation() == taxCodeReg.nation());
}

bool
TaxXS_00::validateTransit(PricingTrx& /*trx*/, TaxResponse& /*taxResponse*/,
      TaxCodeReg& /*taxCodeReg*/, uint16_t /*travelSegIndex*/)
{
  return true;
}

DiagManager
TaxXS_00::getDiagManager(PricingTrx& trx, TaxCodeReg& taxCodeReg) const
{
  DiagManager diag(trx, Diagnostic818);
  if (diag.isActive())
  {
   const std::string& strVal = trx.diagnostic().diagParamMapItem("TX");
   if (!strVal.empty() && strVal != taxCodeReg.taxCode())
     diag.deActivate();
  }

  return std::move(diag);
}

bool
TaxXS_00::validateFinalGenericRestrictions(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t& /*startIndex*/, uint16_t& /*endIndex*/)
{
  DiagManager diag = getDiagManager(trx, taxCodeReg);

  diag << "***\n***" << taxCodeReg.taxCode() << " START SPN PROCESSING - VALIDATING***\n***\n";
  diag << "SEQUENCE NO - " << taxCodeReg.seqNo() << "\n";

  searchDomesticParts(taxResponse, taxCodeReg, diag);

  diag << "***\n***" << taxCodeReg.taxCode() << " END SPN PROCESSING - VALIDATING***\n***\n";

  return !_domSegments.empty();
}

void
TaxXS_00::searchDomesticParts(TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, DiagManager& diag)
{
  TaxLocIterator& locIt = *getLocIterator(*(taxResponse.farePath()), taxCodeReg);
  _domSegments.reserve(locIt.numOfSegs());

  int nStart = -1;
  while(locIt.hasNext())
  {
    if (nStart < 0 && isDomesticSegment(*locIt.nextSeg(), taxCodeReg) && locIt.isStop())
    {
      nStart = locIt.nextSegNo();
      diag << "START DOMESTIC SEGMENT FOUND-" << locIt.nextSegNo() << "\n";
    }

    if (nStart >= 0 && !isDomesticSegment(*locIt.nextSeg(), taxCodeReg))
    {
      TaxLocIterator locItBack(locIt);
      while(locItBack.hasPrevious())
      {
        if (locItBack.prevSegNo() >= nStart && locItBack.isStop())
        {
          diag << "END DOMESTIC SEGMENT FOUND-" << locItBack.prevSegNo() << "\n";
          while (nStart <= locItBack.prevSegNo())
            _domSegments.push_back(nStart++);
          break;
        }
        else if (locItBack.prevSegNo() <= nStart)
        {
          diag << "NO STOPOVER\n";
          break;
        }
        locItBack.previous();
      }

      nStart = -1;
    }

    locIt.next();
  }

  //means that itin ends in SE
  if (nStart >= 0)
  {
    diag << "END DOMESTIC SEGMENT FOUND-" << locIt.prevSegNo() << "\n";
    while (nStart <= locIt.prevSegNo())
      _domSegments.push_back(nStart++);
  }
}

void
TaxXS_00::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
  DiagManager diag = getDiagManager(trx, taxCodeReg);

  diag << "***\n***" << taxCodeReg.taxCode() << " START SPN PROCESSING - TAXCREATE***\n***\n";
  diag << "SEQUENCE NO - " << taxCodeReg.seqNo() << "\n";

  Tax::taxCreate(trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex);

  _taxSplitDetails.setFareSumAmount(0.0);
  _calculationDetails.fareInCalculationCurrencyAmount = 0;
  _calculationDetails.fareInBaseFareCurrencyAmount = 0;
  _calculationDetails.fareInPaymentCurrencyAmount = 0;

  _taxAmount = _taxableFare = _taxableBaseFare = 0;
  _failCode = TaxDiagnostic::NONE;

  for (auto it = _domSegments.cbegin(); it != _domSegments.cend(); )
  {
    uint16_t startIdx = *it;
    FareUsage* fareUsage = locateFare(taxResponse.farePath(), *it + 1);
    if(!fareUsage)
    {
      diag << "ERROR - NO FARE FOR SEGMENT-" << *it << " AND SKIPS THIS SEGMENT\n";
      ++it;
      continue;
    }

    MoneyAmount taxableFare = fareUsage->totalFareAmount();
    if (diag.isActive())
    {
      const TravelSeg* seg = taxResponse.farePath()->itin()->travelSeg()[*it];
      diag << "SEGMENT-" << seg->origin()->loc() << seg->destination()->loc()
           << " IN FAREBASIS-" << fareUsage->paxTypeFare()->createFareBasis(trx)
           << " TOTAL AMOUNT-" << taxableFare << "\n";
    }

    uint32_t unMilesDom = 0;
    uint32_t unMilesTotal = 0;

    for (const TravelSeg* segFare : fareUsage->travelSeg())
    {
      if ( !segFare->isAir())
        continue;

      if (diag.isActive())
        diag.collector().flushMsg();

      uint32_t unMiles = taxUtil::calculateMiles(trx, taxResponse,
        *(segFare->origin()), *(segFare->destination()), fareUsage->travelSeg());

      diag << "-" << unMiles << " MILES\n";

      unMilesTotal += unMiles;

      if ( it != _domSegments.end() &&
          (taxResponse.farePath()->itin()->segmentOrder(segFare) == (*it +1)))
      {
        unMilesDom += unMiles;
        ++it;
      }
    }

    _taxableFare += taxableFareAmount(trx, taxResponse, taxableFare, unMilesTotal,
        unMilesDom, startIdx, diag);
  }

  diag << "TAXABLE FARE AMOUNT-" << _taxableFare << "\n";

  _thruTotalFare = _taxableFare;
  _taxSplitDetails.setFareSumAmount(_taxableFare);

  diag << "***\n***" << taxCodeReg.taxCode() << " END SPN PROCESSING - TAXCREATE***\n***\n";
}

TaxLocIterator*
TaxXS_00::getLocIterator(FarePath& farePath, const TaxCodeReg& taxCodeReg)
{
  TaxLocIterator* locIt = Tax::getLocIterator(farePath);
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(0);

  if (!taxCodeReg.restrictionTransit().empty())
    locIt->setStopHours(taxCodeReg.restrictionTransit().front().transitHours());
  else
    locIt->setStopHours(24);

  return locIt;
}

FareUsage*
TaxXS_00::locateFare(const FarePath* farePath, uint16_t segId) const
{
  for (const PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      for (const TravelSeg* travelSeg : fareUsage->travelSeg())
      {
        if (farePath->itin()->segmentOrder(travelSeg) == segId)
          return fareUsage;
      }
    }
  }

  return nullptr;
}

MoneyAmount
TaxXS_00::taxableFareAmount(PricingTrx& trx, TaxResponse& taxResponse, MoneyAmount taxableFare,
    uint32_t unMilesTotal, uint32_t unMilesDomestic, uint16_t startIdx, DiagManager& diag)
{
  if (unMilesDomestic == unMilesTotal)
  {
    diag << "FULLY DOMESTIC FARE - TAKE TOTAL AMOUNT\n";
  }
  else
  {
    double ratio = unMilesTotal >= 0 ? (double)unMilesDomestic/unMilesTotal : 1.0;

    diag << "PRORATED METHOD\n"
         << "TOTAL MILES-" << unMilesTotal << " DOMESTIC MILES-"
         << unMilesDomestic << " RATIO-" << ratio << "\n"
         << "TOTAL AMOUNT-" << taxableFare << " TAXABLE AMOUNT-"
         << taxableFare * ratio << "\n";

    taxableFare *= ratio;
  }

  Percent discPercent = trx.getRequest()->discountPercentage(startIdx);
  if (discPercent >= 0 && discPercent <= 100)
  {
    taxableFare *= (1.0 - discPercent / 100.0);
    diag << "DISCOUNT-" << discPercent * 100.0 << " DISCOUNTED AMOUNT-"
         << taxableFare << taxResponse.farePath()->calculationCurrency() << "\n";
  }

  _calculationDetails.fareInCalculationCurrencyAmount += taxableFare;

  if (diag.isActive())
    diag.collector().flushMsg();

  taxableFare = taxUtil::convertCurrency(trx, taxableFare, _paymentCurrency,
        taxResponse.farePath()->calculationCurrency(),
        taxResponse.farePath()->baseFareCurrency(),
        CurrencyConversionRequest::FARES,
        taxResponse.farePath()->itin()->useInternationalRounding());

  _calculationDetails.fareInPaymentCurrencyAmount += taxableFare;

  diag << "CONVERTED TO " << taxableFare << _paymentCurrency << "\n";

  return taxableFare;
}

void
TaxXS_00::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxOnTaxCollector collector(*this, _calculationDetails, _taxOnTaxItems, _domSegments);

  _applyFeeOnTax = true;
  _calculationDetails.isTaxOnTax = true;

  const bool taxShoppingRequest = TrxUtil::isShoppingTaxRequest(&trx)
      && taxResponse.farePath() && !taxResponse.farePath()->getExternalTaxes().empty();

  for (const TaxCode& taxCode : taxCodeReg.taxOnTaxCode())
  {
    collector.collect(taxResponse.taxItemVector(), taxCode);
    if (taxShoppingRequest)
      collector.collect(taxResponse.farePath()->getExternalTaxes(), taxCode);
  }

  _calculationDetails.taxableTaxSumAmount = collector.getMoneyAmount();

  _taxableFare += collector.getMoneyAmount();
  _taxAmount = AdjustTax::applyAdjust(
          trx, taxResponse, _taxableFare * taxCodeReg.taxAmt(), _paymentCurrency,
          taxCodeReg, _calculationDetails);

  doTaxRound(trx, taxCodeReg);

  _taxSplitDetails.setUseTaxableTaxSumAmount(true);
  _mixedTax = true;
}
