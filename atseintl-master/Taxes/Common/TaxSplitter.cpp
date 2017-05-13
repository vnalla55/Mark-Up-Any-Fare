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

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TaxRound.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/TaxConfig.h"
#include "Taxes/Common/TaxSplitter.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

namespace tse
{
namespace
{
const ConfigSet<TaxCode>
defaultFareAmountTaxes("US1");
const ConfigSet<TaxCode>
defaultSegmentBasedTaxes("AY");
const ConfigSet<TaxCode> defaultFlatRateTaxes;
const ConfigSet<TaxCode> defaultTaxOnTaxSpecialTaxes;
}

FALLBACK_DECL(fallbackAYTax)

log4cxx::LoggerPtr
TaxSplitter::_logger(log4cxx::Logger::getLogger("atseintl.FareCalc.WnSnapFCUtil"));

TaxSplitter::TaxSplitter(ShoppingTrx& trx) : _trx(trx)
{
  if (trx.getRequest()->originBasedRTPricing())
  {
    _fareAmountTaxes = &FareCalcTaxConfig::fareAmountTaxes.getValue();
    _segmentBasedTaxes = &FareCalcTaxConfig::segmentBasedTaxes.getValue();
    _flatRateTaxes = &FareCalcTaxConfig::flatRateTaxes.getValue();
    _taxOnTaxSpecialTaxes = &FareCalcTaxConfig::taxOnTaxSpecialTaxes.getValue();
  }
  else
  {
    _fareAmountTaxes = &defaultFareAmountTaxes;
    _segmentBasedTaxes = &defaultSegmentBasedTaxes;
    _flatRateTaxes = &defaultFlatRateTaxes;
    _taxOnTaxSpecialTaxes = &defaultTaxOnTaxSpecialTaxes;
  }

  // Ensure that tax maps are not resized in threads
  for (const TaxCode& taxCode : *_fareAmountTaxes)
  {
    _taxMap[taxCode];
    _taxMapForItin[taxCode];
  }
}

void
TaxSplitter::setupTaxes(Itin* const primaryItin, Itin* subItin, const int legId)
{
  std::vector<FarePath*>::const_iterator fpIter = subItin->farePath().begin();
  std::vector<FarePath*>::const_iterator fpIterEnd = subItin->farePath().end();

  for (; fpIter != fpIterEnd; ++fpIter)
  {
    const FarePath* farePath = (*fpIter);
    FarePath::FarePathKey farePathKey(
        farePath->paxType(), INVALID_BRAND_INDEX, INVALID_BRAND_INDEX, nullptr, nullptr);

    setupTaxesForFarePath(primaryItin, subItin, legId, farePath, farePathKey);
  }
}

void
TaxSplitter::setupTaxesForFarePath(Itin* const primaryItin,
                                   Itin* subItin,
                                   const int legId,
                                   const FarePath* farePath,
                                   FarePath::FarePathKey& farePathKey)
{
  TaxResponse* const taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, subItin, farePathKey);
  TaxResponse* const taxResponsePi =
      ShoppingUtil::getTaxResponseForKey(_trx, primaryItin, farePathKey);
  ExtractedTaxItems extractedTaxItems;

  if (taxResponse != nullptr)
  {
    // STEP0: calcualte fare amount based taxes (for example US1)
    extractedTaxItems.fareAmountTaxItems.insert("US1");
    if (_trx.getRequest()->originBasedRTPricing())
    {
      for (TaxItem* taxItem : taxResponsePi->taxItemVector())
      {
        if (taxItem->taxType() == 'P')
        {
          if (!((taxItem->taxLocalBoard() == primaryItin->travelSeg().front()->origAirport()) &&
                (taxItem->taxLocalOff() == primaryItin->travelSeg().back()->destAirport())))
          {
            extractedTaxItems.fareAmountTaxItems.insert(taxItem->taxCode());
          }
        }
        if (!taxItem->taxOnTaxItems().empty())
        {
          extractedTaxItems.taxOnTaxItems.insert(taxItem->taxCode());
        }
      }
    }
    std::set<std::string>::iterator TaxItemsIt;
    for (TaxItemsIt = extractedTaxItems.fareAmountTaxItems.begin();
         TaxItemsIt != extractedTaxItems.fareAmountTaxItems.end();
         ++TaxItemsIt)
    {
      setupFareAmountTax(*TaxItemsIt, primaryItin, subItin, legId, farePath, farePathKey, false);
    }
    for (TaxItemsIt = extractedTaxItems.taxOnTaxItems.begin();
         TaxItemsIt != extractedTaxItems.taxOnTaxItems.end();
         ++TaxItemsIt)
    {
      setupTaxOnTax(*TaxItemsIt, farePath, legId, primaryItin, taxResponse);
    }

    for (const TaxCode& taxCode : *_flatRateTaxes)
    {
      setupFlatRateTaxes(taxCode, primaryItin, farePathKey);
    }

    for (const TaxCode& taxCode : *_segmentBasedTaxes)
    {
      setupSegmentBasedTax(taxCode, primaryItin, legId, farePathKey);
    }
    // STEP1: rebuild taxResponse->taxRecordVector() (TSM)
    rebuildTaxRecordVec(taxResponse);
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "WnSnapFCUtil::setupTaxes - TaxResponse not found.");
    return;
  }

  // STEP2: and update CalcTotals accordingly
  updateCalcTotals(subItin, farePath, taxResponse);
}

Itin*
TaxSplitter::getSubItin(Itin* const primaryItin, const int legId) const
{
  return (legId == 0) ? _trx.primeSubItinMap()[primaryItin].outboundItin
                      : _trx.primeSubItinMap()[primaryItin].inboundItin;
}

void
TaxSplitter::setupFareAmountTax(const TaxCode& taxCode,
                                Itin* const primaryItin,
                                Itin* subItin,
                                const int legId,
                                const FarePath* const farePath,
                                FarePath::FarePathKey& farePathKey,
                                bool checkTax)
{
  Itin* const oppositeDirSubItin = getSubItin(primaryItin, (legId == 0) ? 1 : 0);

  TaxSplitter::FarePathAmountMap& farePathAmountMap = _taxMap[taxCode];
  TaxSplitter::FarePathAmountMapForItin& farePathAmountMapForItin = _taxMapForItin[taxCode];
  if (farePathAmountMap.find(farePathKey) == farePathAmountMap.end())
  {
    MoneyAmount subItinTax = getTotalTax(subItin, farePathKey, taxCode, "");
    farePathAmountMap[farePathKey] = subItinTax;

    (farePathAmountMapForItin[subItin])[farePathKey] = subItinTax;

    if ((false == _trx.getRequest()->owPricingRTTaxProcess()) ||
        ((farePathAmountMapForItin[oppositeDirSubItin]).find(farePathKey) ==
         (farePathAmountMapForItin[oppositeDirSubItin]).end()))
    {
      farePathAmountMap[farePathKey] += getTotalTax(oppositeDirSubItin, farePathKey, taxCode, "");
    }
    else
    {
      farePathAmountMap[farePathKey] += (farePathAmountMapForItin[oppositeDirSubItin])[farePathKey];
    }
    if (farePathAmountMap[farePathKey] <= 0.0)
    {
      return;
    }
  }

  MoneyAmount subItinTotalFare = getFareAndSurchargeAmount(subItin, farePathKey, GeoTravelType::Domestic);

  MoneyAmount oppositeDirSubItinTotalFare =
      getFareAndSurchargeAmount(oppositeDirSubItin, farePathKey, GeoTravelType::Domestic);

  if ((0.0 == subItinTotalFare) && (0.0 == oppositeDirSubItinTotalFare))
  {
    subItinTotalFare = getFareAndSurchargeAmount(subItin, farePathKey, GeoTravelType::UnknownGeoTravelType);

    oppositeDirSubItinTotalFare =
        getFareAndSurchargeAmount(oppositeDirSubItin, farePathKey, GeoTravelType::UnknownGeoTravelType);
  }

  TaxItem* taxItem = findFirstDeleteOtherTax(subItin, farePathKey, taxCode, checkTax);

  if (!taxItem)
  {
    if (!copyTaxFromOppositeDirection(
            taxItem, subItin, farePath, oppositeDirSubItin, farePathKey, taxCode))
    {
      return;
    }
  }

  if (0.0 == subItinTotalFare && taxItem->taxOnTaxItems().empty())
  {
    return;
  }

  // Tax = Total tax * (curDir fare / total fare)
  if (0.0 == oppositeDirSubItinTotalFare)
  {
    taxItem->taxAmount() = farePathAmountMap[farePathKey];
  }
  else
  {
    taxItem->taxAmount() = farePathAmountMap[farePathKey] * subItinTotalFare /
                           (subItinTotalFare + oppositeDirSubItinTotalFare);
  }
  if (taxItem->maxTax() > 0)
  {
    MoneyAmount convertedAmount = taxItem->maxTax();
    if (taxItem->taxCur() != taxItem->paymentCurrency())
    {
      if (!convertCurrency(
              taxItem->maxTax(), taxItem->taxCur(), taxItem->paymentCurrency(), convertedAmount))
      {
        return;
      }
    }
    if (taxItem->taxAmount() >= convertedAmount)
    {
      taxItem->taxAmount() = taxItem->taxAmount() / 2;
    }
    else if (taxItem->taxAmount() * 2 >= convertedAmount)
    {
      taxItem->taxAmount() = convertedAmount / 2;
    }
  }
}

MoneyAmount
TaxSplitter::getTotalTax(const Itin* const itin,
                         const FarePath::FarePathKey& farePathKey,
                         const TaxCode& taxCode,
                         const CarrierCode& carrierCode)
{
  if (itin == nullptr)
  {
    return 0.0;
  }

  MoneyAmount result = 0.0;
  TaxResponse* taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, itin, farePathKey);

  if (taxResponse == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "WnSnapFCUtil::getTotalTax - Tax Response is NULL.");
    return 0.0;
  }

  std::vector<TaxItem*>::iterator taxItemIt = taxResponse->taxItemVector().begin();
  std::vector<TaxItem*>::iterator taxItemItEnd = taxResponse->taxItemVector().end();

  for (; taxItemIt != taxItemItEnd; ++taxItemIt)
  {
    TaxItem* taxItem = (*taxItemIt);

    if (taxItem->taxCode() != taxCode)
    {
      continue;
    }

    if (_trx.getRequest()->originBasedRTPricing() == false)
    {
      // Special logic for AY
      if (taxCode.equalToConst("AY"))
      {
        if (taxItem->taxLocalBoard() != taxItem->taxLocalOff())
        {
          continue;
        }
      }
    }

    if (!carrierCode.empty() && (taxItem->carrierCode() != carrierCode))
    {
      continue;
    }

    result += taxItem->taxAmount();
  }

  return result;
}

MoneyAmount
TaxSplitter::getFareAndSurchargeAmount(const Itin* const itin,
                                       const FarePath* const farePath,
                                       const GeoTravelType geoTravelType)
{
  MoneyAmount result = 0.0;

  std::vector<PricingUnit*>::const_iterator puIt = farePath->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath->pricingUnit().end();

  for (; puIt != puItEnd; ++puIt)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuItEnd = (*puIt)->fareUsage().end();

    for (; fuIt != fuItEnd; ++fuIt)
    {
      if ((GeoTravelType::UnknownGeoTravelType == geoTravelType) ||
          ((*fuIt)->paxTypeFare()->fareMarket()->geoTravelType() == geoTravelType))
      {
        result += (*fuIt)->paxTypeFare()->nucFareAmount();
        result += getSurcharge(itin, farePath, *fuIt);
      }
    }
  }

  return result;
}

MoneyAmount
TaxSplitter::getFareAndSurchargeAmount(Itin* itin,
                                       const FarePath::FarePathKey& farePathKey,
                                       const GeoTravelType geoTravelType)
{
  if (nullptr == itin)
  {
    return 0.0;
  }

  const FarePath* const farePath = ShoppingUtil::getFarePathForKey(_trx, itin, farePathKey);

  if (nullptr == farePath)
  {
    LOG4CXX_DEBUG(_logger, "WnSnapFCUtil::getFareAndSurchargeAmount - FarePath object is NULL.");
    return 0.0;
  }

  return getFareAndSurchargeAmount(itin, farePath, geoTravelType);
}

MoneyAmount
TaxSplitter::getSurcharge(const Itin* const itin,
                          const FarePath* const farePath,
                          const FareUsage* fareUsage)
{
  MoneyAmount result = 0.0;

  FareCalcCollector* fareCalcCollector = FareCalcUtil::getFareCalcCollectorForItin(_trx, itin);

  if (fareCalcCollector != nullptr)
  {
    CalcTotals* calcTotal = fareCalcCollector->findCalcTotals(farePath);

    if (calcTotal != nullptr)
    {
      std::vector<TravelSeg*>::const_iterator tsIt = fareUsage->travelSeg().begin();

      std::vector<TravelSeg*>::const_iterator tsItEnd = fareUsage->travelSeg().end();

      for (; tsIt != tsItEnd; ++tsIt)
      {
        std::map<const TravelSeg*, std::vector<SurchargeData*>>::const_iterator surchargesIt =
            calcTotal->surcharges.find(*tsIt);

        if (surchargesIt != calcTotal->surcharges.end())
        {
          std::vector<SurchargeData*>::const_iterator surIt = surchargesIt->second.begin();
          const std::vector<SurchargeData*>::const_iterator surItEnd = surchargesIt->second.end();

          for (; surIt != surItEnd; ++surIt)
          {
            result += (*surIt)->amountNuc() * (*surIt)->itinItemCount();
          }
        }
      }
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "TaxSplitter::getSurcharge - CalcTotals not found.");
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::getSurcharge - FareCalcCollector not found.");
  }

  return result;
}

TaxItem*
TaxSplitter::findFirstDeleteOtherTax(const TaxCode& taxCode,
                                     const bool checkTax,
                                     TaxResponse* taxResponse)
{
  TaxItem* firstTax = nullptr;

  std::vector<TaxItem*>& taxItemVec = taxResponse->taxItemVector();
  std::vector<TaxItem*>::iterator taxItemIt = taxItemVec.begin();

  while (taxItemIt != taxItemVec.end())
  {
    if ((*taxItemIt)->taxCode() == taxCode)
    {
      if (checkTax && !appliesToEntireTrip(*taxItemIt))
      {
        ++taxItemIt;
        continue;
      }

      if (!firstTax)
      {
        firstTax = *taxItemIt;
        ++taxItemIt;
      }
      else
      {
        taxItemIt = taxItemVec.erase(taxItemIt);
      }
    }
    else
    {
      ++taxItemIt;
    }
  }

  return firstTax;
}

TaxItem*
TaxSplitter::findFirstDeleteOtherTax(const Itin* const itin,
                                     const FarePath::FarePathKey& farePathKey,
                                     const TaxCode& taxCode,
                                     const bool checkTax)
{
  TaxResponse* taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, itin, farePathKey);

  if (!taxResponse)
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::findFirstDeleteOtherTax - Tax Response is NULL.");
    return nullptr;
  }

  return findFirstDeleteOtherTax(taxCode, checkTax, taxResponse);
}

bool
TaxSplitter::copyTaxFromOppositeDirection(TaxItem*& taxItem,
                                          const Itin* const subItin,
                                          const FarePath* const farePath,
                                          const Itin* const oppositeDirSubItin,
                                          FarePath::FarePathKey& farePathKey,
                                          const TaxCode& taxCode)
{
  // copy US1 tax from opposite direction
  TaxItem* oppositeDirTaxItem = findFirstTax(oppositeDirSubItin, farePathKey, taxCode, "");

  if (nullptr == oppositeDirTaxItem)
  {
    // there is no tax in whole itinerary
    return false;
  }

  // find taxResponse
  TaxResponse* taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, subItin, farePathKey);

  if (nullptr == taxResponse)
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::copyTaxFromOppositeDirection - Tax Response is NULL.");
    return false;
  }

  // add us1 tax from opposite direction
  _trx.dataHandle().get(taxItem);
  (*taxItem) = (*oppositeDirTaxItem);
  const_cast<int16_t&>(taxItem->legId()) = subItin->travelSeg().front()->legId();
  taxResponse->taxItemVector().push_back(taxItem);

  // TODO: set tax's segment to first domestic in us1tax
  setSegmentAndCxrInTax(taxItem, farePath);
  return true;
}

void
TaxSplitter::setSegmentAndCxrInTax(TaxItem*& taxItem, const FarePath* const farePath)
{
  // find first domestic travelSeg
  TravelSeg* firstDomesticTravelSeg = nullptr;
  std::vector<PricingUnit*>::const_iterator puIt = farePath->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath->pricingUnit().end();

  for (; (puIt != puItEnd) && (!firstDomesticTravelSeg); ++puIt)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = (*puIt)->fareUsage().end();

    for (; (fuIt != fuItEnd) && (!firstDomesticTravelSeg); ++fuIt)
    {
      if ((*fuIt)->paxTypeFare()->fareMarket()->geoTravelType() == GeoTravelType::Domestic)
      {
        firstDomesticTravelSeg = (*fuIt)->travelSeg().front();
      }
    }
  }

  // update tax data according to found travelSeg
  if (firstDomesticTravelSeg)
  {
    if (firstDomesticTravelSeg->isAir())
    {
      AirSeg* as = static_cast<AirSeg*>(firstDomesticTravelSeg);
      const_cast<LocCode&>(taxItem->taxLocalBoard()) = as->origin()->loc();
      const_cast<CarrierCode&>(taxItem->carrierCode()) = as->marketingCarrierCode();
    }
    else
    {
      LOG4CXX_DEBUG(_logger,
                    "TaxSplitter::setSegmentAndCxrInTax - firstDomesticTravelSeg is not Air");
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::setSegmentAndCxrInTax - firstDomesticTravelSeg is NULL");
  }
}

void
TaxSplitter::setupSegmentBasedTax(const TaxCode& taxCode,
                                  Itin* const primaryItin,
                                  const int legId,
                                  FarePath::FarePathKey& farePathKey)
{
  if (legId != 0)
    return; // we do it only for outbound

  if (_trx.getRequest()->originBasedRTPricing())
  {
    setupSegmentBasedTax(
        taxCode, primaryItin, getCarrier(primaryItin->travelSeg().front()), farePathKey);
  }
  else if (false == _trx.getRequest()->owPricingRTTaxProcess())
  {
    setupSegmentBasedTax(taxCode, primaryItin, getFirstCxr(primaryItin), farePathKey);
    setupSegmentBasedTax(taxCode, primaryItin, getSecondCxr(primaryItin), farePathKey);
  }
  else
  {
    setupSegmentBasedTax(
        taxCode, primaryItin, getCarrier(primaryItin->travelSeg().front()), farePathKey);
  }
}

void
TaxSplitter::setupSegmentBasedTax(const TaxCode& taxCode,
                                  Itin* const primaryItin,
                                  const CarrierCode& carrierCode,
                                  FarePath::FarePathKey& farePathKey)
{
  Itin* const outItin = getSubItin(primaryItin, 0);
  Itin* const inItin = getSubItin(primaryItin, 1);

  const int outCxrSegNum = getCxrSegNumInUSA(outItin, carrierCode);
  const int inCxrSegNum = getCxrSegNumInUSA(inItin, carrierCode);

  const int totalCxrSegNum = outCxrSegNum + inCxrSegNum;

  // Update tax proportionally to segments
  const MoneyAmount outTaxAmount(getTotalTax(outItin, farePathKey, taxCode, carrierCode)),
      inTaxAmount(getTotalTax(inItin, farePathKey, taxCode, carrierCode)),
      totalTaxAmount(outTaxAmount + inTaxAmount);

  if (totalTaxAmount == 0.0)
  {
    return;
  }

  TaxItem* outTax = findFirstDeleteOtherTax(outItin, farePathKey, taxCode, true);
  TaxItem* inTax = findFirstDeleteOtherTax(inItin, farePathKey, taxCode, true);

  if ((!outTax) && (!inTax))
  {
    if (_trx.getRequest()->originBasedRTPricing())
    {
      outTax = findFirstDeleteOtherTax(outItin, farePathKey, taxCode, false);
      inTax = findFirstDeleteOtherTax(inItin, farePathKey, taxCode, false);
    }

    if ((!outTax) && (!inTax))
    {
      return;
    }
  }

  if (!outTax)
  {
    if (!createTax(outTax, outItin, farePathKey, inTax, 0))
    {
      return;
    }
  }

  if (fallback::fallbackAYTax(&_trx))
    outTax->taxAmount() = totalTaxAmount * outCxrSegNum / totalCxrSegNum;
  else
    outTax->taxAmount() = outTaxAmount;

  if (!inTax)
  {
    if (!createTax(inTax, inItin, farePathKey, outTax, 1))
    {
      return;
    }
  }

  if (fallback::fallbackAYTax(&_trx))
    inTax->taxAmount() = totalTaxAmount * inCxrSegNum / totalCxrSegNum;
  else
    inTax->taxAmount() = inTaxAmount;
}

void
TaxSplitter::setupTaxOnTax(const TaxCode& taxCode,
                           const FarePath* const farePath,
                           const int legId,
                           Itin* const itin,
                           TaxResponse* const taxResponse)
{
  Itin* const subItin = getSubItin(itin, legId);

  for (TaxItem* taxItem : taxResponse->taxItemVector())
  {
    if (taxItem->taxCode() != taxCode)
    {
      continue;
    }

    if (!TaxOnTax::useTaxOnTax(
            taxItem->taxType(), taxItem->taxOnTaxCode(), taxItem->taxOnTaxExcl()))
    {
      continue;
    }

    MoneyAmount taxableFare = taxItem->getFareSumAmount();
    MoneyAmount taxAmount = taxItem->taxAmt();

    if (taxableFare < 0)
    {
      taxableFare = getFareAndSurchargeAmount(subItin, farePath, GeoTravelType::UnknownGeoTravelType);

      MoneyAmount convertedAmount;
      if (farePath->calculationCurrency() != farePath->baseFareCurrency())
      {
        if (convertCurrency(taxableFare,
                            farePath->calculationCurrency(),
                            farePath->baseFareCurrency(),
                            convertedAmount))
        {
          taxableFare = convertedAmount;
        }
      }
      if (farePath->baseFareCurrency() != taxItem->paymentCurrency())
      {
        if (convertCurrency(taxableFare,
                            farePath->baseFareCurrency(),
                            taxItem->paymentCurrency(),
                            convertedAmount))
        {
          taxableFare = convertedAmount;
        }
      }
    }

    if (taxItem->useTaxableTaxSumAmount())
    {
      taxAmount =
          taxItem->taxAmt() * (taxableFare + taxItem->calculationDetails().taxableTaxSumAmount);
    }
    else
    {
      TaxOnTax taxOnTax(*taxItem);
      taxOnTax.calculateTaxOnTax(_trx,
                                 *taxResponse,
                                 taxAmount,
                                 taxableFare,
                                 *taxItem,
                                 Tax::shouldSplitPercentageTax(_trx, taxCode),
                                 true);
    }

    {
      taxItem->calculationDetails().taxToAdjustAmount = taxAmount;
      TaxCodeReg taxCodeReg;
      taxItem->taxItemInfo().copyTo(taxCodeReg);
      taxAmount = AdjustTax::applyAdjust(_trx,
                                         *taxResponse,
                                         taxAmount,
                                         taxItem->paymentCurrency(),
                                         taxCodeReg,
                                         taxItem->calculationDetails());
    }

    taxItem->taxAmount() = taxAmount;
    if (taxItem->paymentCurrency().equalToConst("AED"))
    {
      TaxRound taxRound;
      RoundingFactor roundingUnit = taxItem->taxcdRoundUnit();
      CurrencyNoDec roundingNoDec = taxItem->taxNodec();
      RoundingRule roundingRule = taxItem->taxcdRoundRule();

      taxRound.retrieveNationRoundingSpecifications(
          _trx, roundingUnit, roundingNoDec, roundingRule);
      MoneyAmount roundAmount = 0;
      roundAmount =
          taxRound.applyTaxRound(taxAmount, taxItem->paymentCurrency(), roundingUnit, roundingRule);
      if (roundAmount > 0)
      {
        taxItem->taxAmount() = roundAmount;
      }
    }
  }
}

void
TaxSplitter::updateCalcTotals(Itin* const itin,
                              const FarePath* const farePath,
                              const TaxResponse* const taxResponse)
{
  FareCalcCollector* fareCalcCollector = FareCalcUtil::getFareCalcCollectorForItin(_trx, itin);

  if (fareCalcCollector != nullptr)
  {
    CalcTotals* calcTotals = fareCalcCollector->findCalcTotals(farePath);

    if (calcTotals != nullptr)
    {
      // STEP2: and update CalcTotals accordingly
      const_cast<std::vector<TaxRecord*>&>(calcTotals->taxRecords()) =
          taxResponse->taxRecordVector();
      const_cast<std::vector<PfcItem*>&>(calcTotals->pfcItems()) = taxResponse->pfcItemVector();
      const_cast<std::vector<TaxItem*>&>(calcTotals->taxItems()) = taxResponse->taxItemVector();
      calcTotals->getMutableFcTaxInfo().initialize(&_trx, calcTotals, nullptr, taxResponse);
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "TaxSplitter::updateCalcTotals - CalcTotals not found.");
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::updateCalcTotals - FareCalcCollector not found.");
  }
}

void
TaxSplitter::rebuildTaxRecordVec(TaxResponse* const taxResponse)
{
  taxResponse->taxRecordVector().clear();
  TaxRecord taxRecord;
  taxRecord.buildTicketLine(_trx, *taxResponse);
}

bool
TaxSplitter::createTax(TaxItem*& taxItem,
                       Itin* const itin,
                       const FarePath::FarePathKey& farePathKey,
                       const TaxItem* const taxItemToBeCopied,
                       const int legId)
{
  taxItem = nullptr;
  _trx.dataHandle().get(taxItem);

  *taxItem = *taxItemToBeCopied;
  const_cast<LocCode&>(taxItem->taxLocalBoard()) =
      getFirstLocCode(itin, taxItemToBeCopied->carrierCode());
  const_cast<int16_t&>(taxItem->legId()) = legId;

  TaxResponse* taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, itin, farePathKey);

  if (!taxResponse)
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::createTax - inTaxResponse is NULL");
    return false;
  }

  taxResponse->taxItemVector().push_back(taxItem);

  return (taxItem != nullptr);
}

CarrierCode
TaxSplitter::getFirstCxr(Itin* const primaryItin)
{
  CarrierCode result = "";

  const Itin* const firstCxrItin = _trx.primeSubItinMap()[primaryItin].firstCxrItin;

  if (firstCxrItin != nullptr)
  {
    result = getCarrier(firstCxrItin->travelSeg().front());
  }

  return result;
}

CarrierCode
TaxSplitter::getSecondCxr(Itin* const primaryItin)
{
  CarrierCode result = "";
  const Itin* const secondCxrItin = _trx.primeSubItinMap()[primaryItin].secondCxrItin;

  if (secondCxrItin != nullptr)
  {
    result = getCarrier(secondCxrItin->travelSeg().front());
  }

  return result;
}

CarrierCode
TaxSplitter::getCarrier(const TravelSeg* const ts)
{
  if (ts->isAir())
  {
    const AirSeg* const as = static_cast<const AirSeg*>(ts);
    return as->marketingCarrierCode();
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::getCarrier - firstTravelSeg is not AirSeg");
    return "";
  }
}

int
TaxSplitter::getCxrSegNumInUSA(Itin* const itin, const CarrierCode& carrierCode)
{
  int result = 0;

  std::vector<TravelSeg*>::const_iterator tsIt = itin->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tsItEnd = itin->travelSeg().end();

  for (; tsIt != tsItEnd; ++tsIt)
  {
    if ((*tsIt)->isAir())
    {
      const AirSeg* const as = static_cast<const AirSeg*>(*tsIt);

      if ((as->marketingCarrierCode() == carrierCode) && (as->origin()->nation().equalToConst("US")))
      {
        ++result;
      }
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "TaxSplitter::getCxrSegNumInUSA - travelSeg is not AirSeg");
    }
  }

  return result;
}

TaxItem*
TaxSplitter::findFirstTax(const Itin* const itin,
                          const FarePath::FarePathKey& farePathKey,
                          const TaxCode& taxCode,
                          const CarrierCode& carrierCode)
{
  if (nullptr == itin)
  {
    return nullptr;
  }

  TaxResponse* taxResponse = ShoppingUtil::getTaxResponseForKey(_trx, itin, farePathKey);

  if (nullptr == taxResponse)
  {
    LOG4CXX_DEBUG(_logger, "TaxSplitter::findFirstTax - Tax Response is NULL");
    return nullptr;
  }

  TaxItem* result = nullptr;
  std::vector<TaxItem*>::iterator taxItemIt = taxResponse->taxItemVector().begin();
  const std::vector<TaxItem*>::iterator taxItemItEnd = taxResponse->taxItemVector().end();

  for (; taxItemIt != taxItemItEnd; ++taxItemIt)
  {
    TaxItem* taxItem = (*taxItemIt);

    if (taxItem->taxCode() == taxCode)
    {
      if ((carrierCode.empty()) || (taxItem->carrierCode() == carrierCode))
      {
        result = taxItem;
        break;
      }
    }
  }

  return result;
}

LocCode
TaxSplitter::getFirstLocCode(Itin* const itin, const CarrierCode& carrierCode)
{
  LocCode result = "";

  std::vector<TravelSeg*>::const_iterator tsIt = itin->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tsItEnd = itin->travelSeg().end();

  for (; tsIt != tsItEnd; ++tsIt)
  {
    if ((*tsIt)->isAir())
    {
      const AirSeg* const as = static_cast<const AirSeg*>(*tsIt);

      if (as->marketingCarrierCode() == carrierCode)
      {
        result = as->origAirport();
        break;
      }
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "TaxSplitter::getFirstLocCode - travelSeg is not AirSeg");
    }
  }

  return result;
}

void
TaxSplitter::clearTaxMaps()
{
  for (FarePathAmountMapPerTax::value_type& mapElement : _taxMap)
  {
    mapElement.second.clear();
  }
}

void
TaxSplitter::setupFlatRateTaxes(const TaxCode& taxCode,
                                Itin* const primaryItin,
                                FarePath::FarePathKey& farePathKey)
{
  Itin* const outItin = getSubItin(primaryItin, 0);
  Itin* const inItin = getSubItin(primaryItin, 1);

  if (!outItin || !inItin)
  {
    return;
  }

  const MoneyAmount totalTaxAmount = getTotalTax(outItin, farePathKey, taxCode, "") +
                                     getTotalTax(inItin, farePathKey, taxCode, "");

  if (totalTaxAmount == 0.0)
  {
    return;
  }

  TaxItem* outTax = findFirstDeleteOtherTax(outItin, farePathKey, taxCode, false);
  TaxItem* inTax = findFirstDeleteOtherTax(inItin, farePathKey, taxCode, false);

  if (!outTax && !inTax)
  {
    return;
  }

  if (!outTax)
  {
    if (!createTax(outTax, outItin, farePathKey, inTax, 0))
    {
      return;
    }
  }

  outTax->taxAmount() = totalTaxAmount / 2;

  if (!inTax)
  {
    if (!createTax(inTax, inItin, farePathKey, outTax, 1))
    {
      return;
    }
  }

  inTax->taxAmount() = totalTaxAmount / 2;
}

bool
TaxSplitter::appliesToEntireTrip(const TaxItem* const taxItem) const
{
  return (taxItem->taxLocalBoard() == taxItem->taxLocalOff());
}

bool
TaxSplitter::convertCurrency(const MoneyAmount fromAmount,
                             CurrencyCode fromCurrencyCode,
                             CurrencyCode toCurrencyCode,
                             MoneyAmount& toAmount)
{
  Money source(fromAmount, fromCurrencyCode);
  Money target(toCurrencyCode);
  CurrencyConversionFacade ccFacade;
  if (ccFacade.convert(target, source, _trx))
  {
    toAmount = target.value();
    return true;
  }
  return false;
}

} // namespace tse
