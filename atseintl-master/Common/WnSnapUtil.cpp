//----------------------------------------------------------------------------
//  File:        WnSnapUtil.cpp
//  Created:     2010-05-27
//
//  Description: WN SNAP utility class
//
//  Updates:
//
//  Copyright Sabre 2010
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

#include "Common/WnSnapUtil.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FarePathCopier.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/IbfAvailabilityTools.h"
#include "DBAccess/TaxCodeReg.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

#include <cmath>
#include <map>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackHalfRTPricingForIbf);
FALLBACK_DECL(fallbackSoldoutFixForHalfRTPricing);
FALLBACK_DECL(fallbackSoldoutOriginRT)

namespace
{
Logger
logger("atseintl.Common.WnSnapUtil");

ConfigurableValue<std::string>
entireTripPercentageTaxesCfg("SHOPPING_OPT", "ENTIRE_TRIP_PERCENTAGE_TAXES");

class TravelSegDiffLegId
{
public:
  TravelSegDiffLegId(int16_t legId) : _legId(legId) {}

  bool operator()(const TravelSeg* travelSeg) const
  {
    if (travelSeg->segmentType() == Arunk)
      return false;

    if (travelSeg->legId() != _legId)
      return true;

    return false;
  }

private:
  int16_t _legId;
};

class FareUsageDiffLegId
{
public:
  FareUsageDiffLegId(int16_t legId) : _legId(legId) {}

  bool operator()(const FareUsage* fareUsage) const
  {
    if (fareUsage->travelSeg().empty())
      return true;

    for (const TravelSeg* travelSeg : fareUsage->travelSeg())
    {
      if (travelSeg->segmentType() == Arunk)
        continue;

      if (travelSeg->legId() != _legId)
        return true;
    }

    return false;
  }

private:
  int16_t _legId;
};

class IsArunk
{
public:
  bool operator()(const TravelSeg* travelSeg) const
  {
    if (Arunk == travelSeg->segmentType())
    {
      return true;
    }

    return false;
  }
};
}

namespace
{
  void updateBrandInformation(const Itin* itin, Itin* subItin)
  {
    itinanalyzerutils::setItinLegs(subItin);
    subItin->brandCodes() = itin->brandCodes();
    for (BrandCode brandCode : subItin->brandCodes())
      IbfAvailabilityTools::updateIbfAvailabilitySoldoutsForItin(subItin, brandCode);
  }

  void copyApplicableFareMarkets(const Itin* itin, Itin* subItin)
  {
    for (FareMarket* fm : itin->fareMarket())
    {
      if (!fm || fm->travelSeg().empty())
        continue;

      const TravelSeg* fmStart = fm->travelSeg().front();

      if (fmStart->isAir() && fmStart->toAirSeg()->isFake())
        continue;

      const auto& segs = subItin->travelSeg();
      if (std::find(segs.begin(), segs.end(), fmStart) != segs.end())
        subItin->fareMarket().push_back(fm);
    }
  }
}

void
WnSnapUtil::splitItinsByDirection(PricingTrx& trx, bool useOriginalItin)
{
  for (auto& map : trx.primeSubItinMap())
  {
    Itin* itin = map.first;
    PricingTrx::SubItinValue& subItinValue = map.second;

    Itin* subItinOut = nullptr;
    Itin* subItinIn = nullptr;
    bool foundOutbound = false;
    bool foundInbound = false;

    trx.dataHandle().get(subItinOut);
    trx.dataHandle().get(subItinIn);

    subItinOut->calculationCurrency() = itin->calculationCurrency();
    subItinOut->originationCurrency() = itin->originationCurrency();
    subItinOut->calcCurrencyOverride() = itin->calcCurrencyOverride();

    subItinIn->calculationCurrency() = itin->calculationCurrency();
    subItinIn->originationCurrency() = itin->originationCurrency();
    subItinIn->calcCurrencyOverride() = itin->calcCurrencyOverride();

    if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
    {
      if (trx.isAltDates())
      {
        subItinOut->datePair() = itin->datePair();
        subItinIn->datePair() = itin->datePair();
      }

      subItinOut->geoTravelType() = itin->geoTravelType();
      subItinIn->geoTravelType() = itin->geoTravelType();

      subItinOut->useInternationalRounding() = itin->useInternationalRounding();
      subItinIn->useInternationalRounding() = itin->useInternationalRounding();
    }

    if (useOriginalItin)
    {
      // Outbound
      getTravelSegmentsBasedOnItin(itin, subItinOut, 0, foundOutbound);

      // Inbound
      getTravelSegmentsBasedOnItin(itin, subItinIn, 1, foundInbound);

      subItinOut->itinNum() = itin->itinNum();
      subItinIn->itinNum() = itin->itinNum();
    }
    else
    {
      bool firstCxr = startFromFirstCxr(itin);

      // First carrier, outbound
      getTravelSegmentsBasedOnItin(subItinValue.firstCxrItin, subItinOut, 0, foundOutbound);

      // Second carrier, outbound
      getTravelSegmentsBasedOnItin(subItinValue.secondCxrItin, subItinOut, 0, foundOutbound);

      if (firstCxr)
      {
        // First carrier, inbound
        getTravelSegmentsBasedOnItin(subItinValue.firstCxrItin, subItinIn, 1, foundInbound);

        // Second carrier, inbound
        getTravelSegmentsBasedOnItin(subItinValue.secondCxrItin, subItinIn, 1, foundInbound);
      }
      else
      {
        // Second carrier, inbound
        getTravelSegmentsBasedOnItin(subItinValue.secondCxrItin, subItinIn, 1, foundInbound);

        // First carrier, inbound
        getTravelSegmentsBasedOnItin(subItinValue.firstCxrItin, subItinIn, 1, foundInbound);
      }
    }

    ValidatingCarrierUpdater validatingCarrier(trx);

    if (foundOutbound)
    {
      subItinOut->setTravelDate(TseUtil::getTravelDate(subItinOut->travelSeg()));
      subItinOut->bookingDate() = TseUtil::getBookingDate(subItinOut->travelSeg());
      validatingCarrier.update(*subItinOut);
      subItinOut->legID().push_back(itin->legID()[0]);

      if (!fallback::fallbackSoldoutFixForHalfRTPricing(&trx))
      {
        // We need this information in order to correctly display soldout status per brand in IBF
        if (trx.getRequest()->originBasedRTPricing() && trx.getRequest()->isBrandedFaresRequest())
        {
          copyApplicableFareMarkets(itin, subItinOut);
          updateBrandInformation(itin, subItinOut);
        }
      }

      trx.subItinVecOutbound().push_back(subItinOut);
    }

    if (foundInbound)
    {
      subItinIn->setTravelDate(TseUtil::getTravelDate(subItinIn->travelSeg()));
      subItinIn->bookingDate() = TseUtil::getBookingDate(subItinIn->travelSeg());
      validatingCarrier.update(*subItinIn);

      const std::pair<int, int>& legID = itin->legID().size() > 1 ?
           itin->legID()[1] : itin->legID()[0];

      subItinIn->legID().push_back(legID);

      if (!fallback::fallbackSoldoutFixForHalfRTPricing(&trx))
      {
        // We need this information in order to correctly display soldout status per brand in IBF
        if (trx.getRequest()->originBasedRTPricing() && trx.getRequest()->isBrandedFaresRequest())
        {
          copyApplicableFareMarkets(itin, subItinIn);
          updateBrandInformation(itin, subItinIn);
        }
      }

      trx.subItinVecInbound().push_back(subItinIn);
    }

    if (foundOutbound)
    {
      subItinValue.outboundItin = subItinOut;
    }

    if (foundInbound)
    {
      subItinValue.inboundItin = subItinIn;
    }
  }
}

bool
WnSnapUtil::startFromFirstCxr(Itin* itin)
{
  bool bFirstCxr = false;
  CarrierCode firstCxr = "";

  for (const TravelSeg* travelSeg : itin->travelSeg())
  {
    const AirSeg* airSeg = travelSeg->toAirSeg();

    if (nullptr == airSeg)
      continue;

    if (firstCxr.empty())
    {
      firstCxr = airSeg->carrier();
    }
    else if (airSeg->legId() == 1)
    {
      if (airSeg->carrier() == firstCxr)
        bFirstCxr = true;
      else
        bFirstCxr = false;

      break;
    }
  }

  return bFirstCxr;
}

void
WnSnapUtil::getTravelSegmentsBasedOnItin(Itin* itin, Itin* resultItin, int16_t legId, bool& bFound)
{
  if (itin == nullptr)
    return;

  for (TravelSeg* travelSeg : itin->travelSeg())
  {
    if (travelSeg->segmentType() == Arunk)
    {
      resultItin->travelSeg().push_back(travelSeg);
    }
    else if (travelSeg->legId() == legId)
    {
      resultItin->travelSeg().push_back(travelSeg);
      bFound = true;
    }
  }
}

void
WnSnapUtil::createFarePathsForOutIn(PricingTrx& trx, bool useOriginalItin)
{
  bool foundCompletePath = false;

  for (auto& map : trx.primeSubItinMap())
  {
    PricingTrx::SubItinValue& subItinValue = map.second;
    Itin* originalItin = map.first;

    if (useOriginalItin)
    {
      if (subItinValue.outboundItin != nullptr)
      {
        bool arunkFound = false;

        if (createFarePathBasedOnItin(
                trx, originalItin, subItinValue.outboundItin, subItinValue.inboundItin, 0))
        {
          arunkFound = true;
        }

        if (!arunkFound)
        {
          subItinValue.outboundItin->travelSeg().erase(
              std::remove_if(subItinValue.outboundItin->travelSeg().begin(),
                             subItinValue.outboundItin->travelSeg().end(),
                             IsArunk()),
              subItinValue.outboundItin->travelSeg().end());
        }
      }

      if (subItinValue.inboundItin != nullptr)
      {
        bool arunkFound = false;

        if (createFarePathBasedOnItin(
                trx, originalItin, subItinValue.outboundItin, subItinValue.inboundItin, 1))
        {
          arunkFound = true;
        }

        if (!arunkFound)
        {
          subItinValue.inboundItin->travelSeg().erase(
              std::remove_if(subItinValue.inboundItin->travelSeg().begin(),
                             subItinValue.inboundItin->travelSeg().end(),
                             IsArunk()),
              subItinValue.inboundItin->travelSeg().end());
        }
      }
    }
    else
    {
      if (false == completeFarePaths(trx, subItinValue.firstCxrItin, subItinValue.secondCxrItin))
      {
        continue;
      }

      bool firstCxr = startFromFirstCxr(originalItin);

      if (subItinValue.outboundItin != nullptr)
      {
        bool arunkFound = false;

        if (subItinValue.firstCxrItin != nullptr)
        {
          if (createFarePathBasedOnItin(trx,
                                        subItinValue.firstCxrItin,
                                        subItinValue.outboundItin,
                                        subItinValue.inboundItin,
                                        0))
          {
            arunkFound = true;
          }
        }

        if (subItinValue.secondCxrItin != nullptr)
        {
          if (createFarePathBasedOnItin(trx,
                                        subItinValue.secondCxrItin,
                                        subItinValue.outboundItin,
                                        subItinValue.inboundItin,
                                        0))
          {
            arunkFound = true;
          }
        }

        if (!arunkFound)
        {
          subItinValue.outboundItin->travelSeg().erase(
              std::remove_if(subItinValue.outboundItin->travelSeg().begin(),
                             subItinValue.outboundItin->travelSeg().end(),
                             IsArunk()),
              subItinValue.outboundItin->travelSeg().end());
        }
      }

      if (subItinValue.inboundItin != nullptr)
      {
        bool arunkFound = false;

        if (firstCxr)
        {
          if (subItinValue.firstCxrItin != nullptr)
          {
            if (createFarePathBasedOnItin(trx,
                                          subItinValue.firstCxrItin,
                                          subItinValue.outboundItin,
                                          subItinValue.inboundItin,
                                          1))
            {
              arunkFound = true;
            }
          }

          if (subItinValue.secondCxrItin != nullptr)
          {
            if (createFarePathBasedOnItin(trx,
                                          subItinValue.secondCxrItin,
                                          subItinValue.outboundItin,
                                          subItinValue.inboundItin,
                                          1))
            {
              arunkFound = true;
            }
          }
        }
        else
        {
          if (subItinValue.secondCxrItin != nullptr)
          {
            if (createFarePathBasedOnItin(trx,
                                          subItinValue.secondCxrItin,
                                          subItinValue.outboundItin,
                                          subItinValue.inboundItin,
                                          1))
            {
              arunkFound = true;
            }
          }

          if (subItinValue.firstCxrItin != nullptr)
          {
            if (createFarePathBasedOnItin(trx,
                                          subItinValue.firstCxrItin,
                                          subItinValue.outboundItin,
                                          subItinValue.inboundItin,
                                          1))
            {
              arunkFound = true;
            }
          }
        }

        if (!arunkFound)
        {
          subItinValue.inboundItin->travelSeg().erase(
              std::remove_if(subItinValue.inboundItin->travelSeg().begin(),
                             subItinValue.inboundItin->travelSeg().end(),
                             IsArunk()),
              subItinValue.inboundItin->travelSeg().end());
        }
      }
    }

    if (true == completeFarePaths(trx, subItinValue.outboundItin, subItinValue.inboundItin))
    {
      foundCompletePath = true;
    }
  }

  if (false == foundCompletePath)
  {
    bool breakTransaction = true;
    if (!fallback::fallbackSoldoutOriginRT(&trx))
    {
      // don't throw exception if origin based RT pricing for IBF/CS/etc. and all legs are fixed
      // to get full soldout results
      if (trx.getRequest()->originBasedRTPricing() && trx.getRequest()->isBrandedFaresRequest())
        breakTransaction = false;
    }
    if (breakTransaction)
      throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }
}

bool
WnSnapUtil::completeFarePaths(const PricingTrx& trx, Itin* itinFirst, Itin* itinSecond)
{
  for (const PaxType* paxType : trx.paxType())
  {
    if (itinFirst != nullptr)
    {
      if (!farePathForPaxFound(paxType, itinFirst->farePath()))
        return false;
    }

    if (itinSecond != nullptr)
    {
      if (!farePathForPaxFound(paxType, itinSecond->farePath()))
        return false;
    }
  }

  return true;
}

bool
WnSnapUtil::farePathForPaxFound(const PaxType* paxType, const std::vector<FarePath*>& farePathVec)
{
  for (const FarePath* farePath : farePathVec)
  {
    if (farePath->paxType()->paxType() == paxType->paxType())
      return true;
  }

  return false;
}

bool
WnSnapUtil::createFarePathBasedOnItin(
    PricingTrx& trx, Itin* itin, Itin* outboundItin, Itin* inboundItin, int16_t legId)
{
  bool foundArunk = false;

  Itin* resultItin = (legId == 0) ? outboundItin : inboundItin;

  FarePathCopier farePathCopier(trx.dataHandle());
  for (FarePath* farePath : itin->farePath())
  {
    if (!trx.snapRequest())
    {
      uint16_t brandIndex = farePath->brandIndex();
      if (!fallback::fallbackHalfRTPricingForIbf(&trx))
      {
        // in half RT Pricing artificial legs are created and then, later
        // multiple fares per itin are rejoined together based on brandIndex
        // in IBF we use brandCode instead of brandIndex so here,
        // an artificial brandIndex has to be used.
        if (trx.getRequest()->isParityBrandsPath())
            brandIndex = trx.getBrandOrder().at(farePath->getBrandCode());
      }

      farePath->brandIndexPair() = std::make_pair(brandIndex, brandIndex);
      farePath->parentItinPair() = std::make_pair(outboundItin, inboundItin);
    }

    FarePath* newFarePath = farePathCopier.getDuplicate(*farePath);

    if (!trx.snapRequest())
    {
      newFarePath->brandIndexPair() = farePath->brandIndexPair();
      newFarePath->parentItinPair() = farePath->parentItinPair();
    }

    newFarePath->pricingUnit().clear();
    getApplicablePricingUnits(trx, farePath, legId, newFarePath->pricingUnit());

    if (newFarePath->pricingUnit().empty())
      continue;

    if (cleanupArunkSegments(newFarePath))
      foundArunk = true;

    FarePath* existingFarePath =
      ShoppingUtil::getFarePathForKey(trx, resultItin, FarePath::buildKey(trx, newFarePath));

    if (existingFarePath)
    {
      existingFarePath->pricingUnit().insert(existingFarePath->pricingUnit().end(),
                                             newFarePath->pricingUnit().begin(),
                                             newFarePath->pricingUnit().end());

      copyTaxResponse(trx,
                      itin,
                      resultItin,
                      existingFarePath,
                      legId,
                      existingFarePath->brandIndexPair().first,
                      existingFarePath->brandIndexPair().second,
                      existingFarePath->parentItinPair().first,
                      existingFarePath->parentItinPair().second);
    }
    else
    {
      newFarePath->itin() = resultItin;

      copyTaxResponse(trx,
                      itin,
                      resultItin,
                      newFarePath,
                      legId,
                      newFarePath->brandIndexPair().first,
                      newFarePath->brandIndexPair().second,
                      newFarePath->parentItinPair().first,
                      newFarePath->parentItinPair().second);

      resultItin->farePath().push_back(newFarePath);
    }
  }

  if (!trx.getRequest()->originBasedRTPricing() ||
      !TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    calcTotalAmountForFarePaths(resultItin);
  }

  return foundArunk;
}

void
WnSnapUtil::getApplicablePricingUnits(PricingTrx& trx,
                                      const FarePath* farePath,
                                      const int16_t legId,
                                      std::vector<PricingUnit*>& outPUVector)
{
  for (const PricingUnit* pu : farePath->pricingUnit())
  {
    bool foundTripPartOnReqLeg = false;
    bool foundTripPartOnOtherLegs = false;

    pricingUnitOnLegWithId(pu, legId, foundTripPartOnReqLeg, foundTripPartOnOtherLegs);

    if (foundTripPartOnReqLeg)
    {
      if (!foundTripPartOnOtherLegs)
      {
        outPUVector.push_back(pu->clone(trx.dataHandle()));
      }
      else
      {
        PricingUnit* singleLegPu = getPricingUnitForLeg(trx, pu, legId);

        if (singleLegPu != nullptr)
        {
          outPUVector.push_back(singleLegPu);
        }
      }
    }
  }
}

bool
WnSnapUtil::containArunkSeg(const std::vector<FareUsage*>& fuVec)
{
  for (const FareUsage* fu : fuVec)
  {
    for (const TravelSeg* travelSeg : fu->travelSeg())
    {
      if (Arunk == travelSeg->segmentType())
        return true;
    }
  }

  return false;
}

bool
WnSnapUtil::cleanupArunkSegments(FarePath* farePath)
{
  bool foundArunkInFp = false;

  for (PricingUnit* pu : farePath->pricingUnit())
  {
    if (containArunkSeg(pu->fareUsage()))
      foundArunkInFp = true;
    else
      pu->travelSeg().erase(
          std::remove_if(pu->travelSeg().begin(), pu->travelSeg().end(), IsArunk()),
          pu->travelSeg().end());
  }

  return foundArunkInFp;
}

PricingUnit*
WnSnapUtil::getPricingUnitForLeg(PricingTrx& trx, const PricingUnit* pu, const int16_t legId)
{
  PricingUnit* resPu = pu->clone(trx.dataHandle());

  if (resPu == nullptr)
    return nullptr;

  resPu->travelSeg().erase(std::remove_if(resPu->travelSeg().begin(),
                                          resPu->travelSeg().end(),
                                          TravelSegDiffLegId(legId)),
                           resPu->travelSeg().end());

  resPu->fareUsage().erase(std::remove_if(resPu->fareUsage().begin(),
                                          resPu->fareUsage().end(),
                                          FareUsageDiffLegId(legId)),
                           resPu->fareUsage().end());

  return resPu;
}

void
WnSnapUtil::pricingUnitOnLegWithId(const PricingUnit* pricingUnit,
                                   const int16_t legId,
                                   bool& foundTripPartOnReqLeg,
                                   bool& foundTripPartOnOtherLegs)
{
  foundTripPartOnReqLeg = false;
  foundTripPartOnOtherLegs = false;

  for (TravelSeg* travelSeg : pricingUnit->travelSeg())
  {
    if (travelSeg->segmentType() == Arunk)
      continue;

    if (travelSeg->legId() == legId)
      foundTripPartOnReqLeg = true;
    else
      foundTripPartOnOtherLegs = true;
  }
}

bool
WnSnapUtil::applyToEntireTrip(TaxItem const* taxItem,
                              const Itin* outItin,
                              const Itin* inItin,
                              PricingTrx& trxForFallbackOnly)
{
  if (taxItem->taxLocalBoard() == taxItem->taxLocalOff())
  {
    return true;
  }

  if (taxItem->taxLocalBoard() == outItin->travelSeg().front()->origAirport() &&
      taxItem->taxLocalOff() == inItin->travelSeg().back()->destAirport())
  {
    return true;
  }

  return false;
}

void
WnSnapUtil::copyTaxResponse(PricingTrx& trx,
                            Itin* itin,
                            Itin* resultItin,
                            FarePath* farePath,
                            int16_t legId,
                            const uint16_t outBrandIndex,
                            const uint16_t inBrandIndex,
                            const Itin* outItin,
                            const Itin* inItin)
{
  FarePath::FarePathKey farePathKey(
      farePath->paxType(), outBrandIndex, inBrandIndex, outItin, inItin);

  TaxResponse* taxResp = ShoppingUtil::getTaxResponseForKey(trx, itin, farePathKey);

  if (nullptr == taxResp)
  {
    LOG4CXX_ERROR(logger, "WnSnapUtil::copyTaxResponse - TaxResponse in primary itin not found.");
    return;
  }

  bool newTaxResp = false;
  TaxResponse* resultTaxResp = ShoppingUtil::getTaxResponseForKey(trx, resultItin, farePathKey);

  if (nullptr == resultTaxResp)
  {
    newTaxResp = true;
    trx.dataHandle().get(resultTaxResp);

    resultTaxResp->farePath() = farePath;
    resultTaxResp->paxTypeCode() = farePath->paxType()->paxType();
  }

  for (PfcItem* pfcItem : taxResp->pfcItemVector())
  {
    if (pfcItem->legId() == legId)
      resultTaxResp->pfcItemVector().push_back(pfcItem);
  }

  for (TaxRecord* taxRecord : taxResp->taxRecordVector())
  {
    if (taxRecord->legId() == legId)
      resultTaxResp->taxRecordVector().push_back(taxRecord);
  }

  typedef boost::char_separator<char> Separator;
  typedef boost::tokenizer<Separator> Tokenizer;
  Tokenizer const tokenizer(entireTripPercentageTaxesCfg.getValue(), Separator("|"));

  for (TaxItem* taxItem : taxResp->taxItemVector())
  {
    if (taxItem->legId() == legId)
    {
      resultTaxResp->taxItemVector().push_back(taxItem);
    }
    else
    {
      if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
      {
        if (trx.getRequest()->originBasedRTPricing())
        {
          if (WnSnapUtil::applyToEntireTrip(taxItem, outItin, inItin, trx))
          {
            if (taxItem->taxType() == 'F')
            {
              taxItem->taxAmount() = divideAmountWithRounding(taxItem);
            }
            else if (taxItem->taxType() == 'P')
            {
              Tokenizer::const_iterator const found =
                  std::find(tokenizer.begin(), tokenizer.end(), taxItem->taxCode());

              if (found != tokenizer.end())
              {
                taxItem->taxAmount() /= 2;
              }
            }

            resultTaxResp->taxItemVector().push_back(taxItem);
          }
        }
      }
    }
  }

  if (newTaxResp)
  {
    resultItin->mutableTaxResponses().push_back(resultTaxResp);
  }
}

void
WnSnapUtil::calcTotalAmountForFarePaths(Itin* itin)
{
  for (FarePath* farePath : itin->farePath())
  {
    farePath->setTotalNUCAmount(0.0);
    for (PricingUnit* pu : farePath->pricingUnit())
    {
      pu->setTotalPuNucAmount(0.0);

      for (FareUsage* fareUsage : pu->fareUsage())
        pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() + fareUsage->totalFareAmount());

      farePath->increaseTotalNUCAmount(pu->getTotalPuNucAmount());
    }
  }
}

void
WnSnapUtil::addArunkSegments(PricingTrx& trx, Itin* itin)
{
  if (itin->travelSeg().size() <= 1)
    return;

  std::vector<TravelSeg*> outTravelSeg;

  std::vector<TravelSeg*>::iterator travelSegIter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::iterator travelSegIterNext = travelSegIter + 1;
  std::vector<TravelSeg*>::iterator travelSegIterEnd = itin->travelSeg().end();

  for (; travelSegIterNext != travelSegIterEnd; ++travelSegIter, ++travelSegIterNext)
  {
    outTravelSeg.push_back(*travelSegIter);

    if (((*travelSegIter)->offMultiCity() != (*travelSegIterNext)->boardMultiCity()) ||
        ((*travelSegIter)->destination()->loc() != (*travelSegIterNext)->origin()->loc()))
    {
      outTravelSeg.push_back(buildArunkSegment(trx, *travelSegIter, *travelSegIterNext));
    }
  }

  outTravelSeg.push_back(*travelSegIter);

  itin->travelSeg().swap(outTravelSeg);
}

ArunkSeg*
WnSnapUtil::buildArunkSegment(PricingTrx& trx, TravelSeg* firstTs, TravelSeg* secondTs)
{
  ArunkSeg* arunkSeg = trx.dataHandle().create<ArunkSeg>();

  arunkSeg->segmentType() = Arunk;
  arunkSeg->origin() = firstTs->destination();
  arunkSeg->destination() = secondTs->origin();
  arunkSeg->origAirport() = firstTs->destAirport();
  arunkSeg->destAirport() = secondTs->origAirport();
  arunkSeg->boardMultiCity() = firstTs->offMultiCity();
  arunkSeg->offMultiCity() = secondTs->boardMultiCity();
  arunkSeg->bookedCabin().setEconomyClass();
  arunkSeg->setBookingCode(DUMMY_BOOKING);
  firstTs->stopOver() = true;
  arunkSeg->bookingDT() = firstTs->bookingDT();

  return arunkSeg;
}

void
WnSnapUtil::buildSubItinVecWithEmptyValues(PricingTrx& trx)
{
  for (Itin* itin : trx.itin())
    trx.primeSubItinMap()[itin] = {};
}

MoneyAmount
WnSnapUtil::divideAmountWithRounding(TaxItem* taxItem)
{
  /*When we split taxes between outbound and inbound when amount value is odd
   * we need to add small value to avoid undercharge
   */

  if (Money::isZeroAmount(taxItem->taxAmount()))
    return taxItem->taxAmount();

  MoneyAmount newValue(taxItem->taxAmount());

  double factor = std::pow(10.0, taxItem->paymentCurrencyNoDec());

  newValue /= 2.0;

  return (static_cast<int>(newValue * factor + 0.5) / factor);
}
}
