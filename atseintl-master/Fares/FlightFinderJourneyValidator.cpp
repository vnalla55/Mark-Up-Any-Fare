//--------------------------------------------------------------------
//
//  File:        FlightFinderJourneyValidator.cpp
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//--------------------------------------------------------------------
#include "Fares/FlightFinderJourneyValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(fallbackPromotionalAvailability)
FALLBACK_DECL(fallbackFRRProcessingRetailerCode)

namespace
{
class SetFareMarketGlobalDirection
{
public:
  SetFareMarketGlobalDirection(const PricingUnit& pu, const GlobalDirection gd)
    : pu(pu), doRestore(false)
  {
    if (gd == GlobalDirection::NO_DIR)
      return;

    doRestore = true;

    setFareMarketGlobalDirection(0, *pu.fareUsage().front()->paxTypeFare()->fareMarket(), gd);

    if (pu.fareUsage().size() > 1)
      setFareMarketGlobalDirection(1, *pu.fareUsage().back()->paxTypeFare()->fareMarket(), gd);
  }

  ~SetFareMarketGlobalDirection()
  {
    if (!doRestore)
      return;

    restoreFareMarketGlobalDirection(0, *pu.fareUsage().front()->paxTypeFare()->fareMarket());

    if (pu.fareUsage().size() > 1)
      restoreFareMarketGlobalDirection(1, *pu.fareUsage().back()->paxTypeFare()->fareMarket());
  }

private:
  void setFareMarketGlobalDirection(size_t index, FareMarket& fm, const GlobalDirection gd)
  {
    oldGD[index] = fm.getGlobalDirection();
    fm.setGlobalDirection(gd);
  }

  void restoreFareMarketGlobalDirection(size_t index, FareMarket& fm)
  {
    fm.setGlobalDirection(oldGD[index]);
  }

  const PricingUnit& pu;
  GlobalDirection oldGD[2];
  bool doRestore;
};
}

using namespace std;

void
FlightFinderJourneyValidator::validate()
{
  // ----------- Rule validation per PU of the FarePath -----------
  TSELatencyData metrics(*_fFTrx, "FF FVO RULE VALIDATION");

  _outboundDateflightMap.swap(_fFTrx->outboundDateflightMap());

  FarePathVectType farePathVect;

  _fareMarketSegBackup.swap(_fFTrx->journeyItin()->travelSeg());

  FarePathSOPDataVectType farePathSOPDataVect;
  prepareSOPDataVect(farePathSOPDataVect);

  prepareFarePathDataVect(farePathSOPDataVect, farePathVect);

  validateFarePathVect(farePathVect);

  if (UNLIKELY((_fFTrx->diagnostic().diagnosticType() == Diagnostic965) &&
                !_fFTrx->ignoreDiagReq()))
  {
    showDiag965(farePathSOPDataVect);
  }

  updateOutboundDateflightMap(farePathSOPDataVect);

  checkFarePathValidity(farePathSOPDataVect);

  _fFTrx->journeyItin()->travelSeg().swap(_fareMarketSegBackup);
}

bool
FlightFinderJourneyValidator::isPUValidForFareRetailerCode(PricingUnit* pu,
                                                           FarePathType* farePathType,
                                                           uint8_t& farePathStatus,
                                                           bool isDiag965)
{
  if (fallback::fallbackFRRProcessingRetailerCode(_fFTrx))
    return true;

  for (FareUsage* fu : pu->fareUsage())
  {
    if (!RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_fFTrx, *fu->paxTypeFare()))
    {
      if (UNLIKELY(isDiag965))
        *(farePathType->errorMsg) = frcPUErrorMsg;

      farePathStatus = FP_NOT_VALID;
      return false;
    }
  }

  if (!RuleUtil::isPricingUnitValidForRetailerCode(*pu))
  {
    if (UNLIKELY(isDiag965))
      *(farePathType->errorMsg) = frcFPErrorMsg;

    farePathStatus = FP_NOT_VALID;
    return false;
  }

  return true;
}

bool
FlightFinderJourneyValidator::isFPValidForFareRetailerCode(FarePath& farePath,
                                                           FarePathType* farePathType,
                                                           uint8_t& farePathStatus,
                                                           bool isDiag965)
{
  if (!RuleUtil::isFarePathValidForRetailerCode(*_fFTrx, farePath))
  {
    if (UNLIKELY(isDiag965))
      *(farePathType->errorMsg) = frcFPErrorMsg;

    farePathStatus = FP_NOT_VALID;
    return false;
  }

  return true;
}


void
FlightFinderJourneyValidator::validateFarePathVect(FarePathVectType& farePathVect)
{
  RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(
      PURuleValidation /*NormalValidation*/);
  RuleControllerWithChancelor<PricingUnitRuleController> fpRuleController(FPRuleValidation);

  std::set<DateTime> passedDates; // STEP 2 BFF DataTime == Passed Date on Outbound
  // STEP 4 BFF DataTime == Passed Date on Inbound

  uint8_t farePathStatus = FP_VALID;
  bool isDiag965 = false;
  bool isDiag966 = false;
  if (UNLIKELY(_fFTrx->diagnostic().diagnosticType() == Diagnostic965))
    isDiag965 = true;
  if (UNLIKELY(_fFTrx->diagnostic().diagnosticType() == Diagnostic555))
  {
    isDiag966 = true;
    showDiag966Header();
  }

  uint16_t farePathCounter = 1;
  FarePathVectType::iterator farePathIter = farePathVect.begin();
  for (; farePathIter != farePathVect.end(); ++farePathIter, ++farePathCounter)
  {
    if (UNLIKELY(isDiag966))
      showDiag966(farePathCounter);

    if ((_fFTrx->bffStep() == FlightFinderTrx::STEP_4 ||
         _fFTrx->bffStep() == FlightFinderTrx::STEP_2) &&
        skipFarePath(**farePathIter, passedDates))
    {
      *((*farePathIter)->farePathStatus) = FP_SKIPPED;
      continue;
    }

    if ((*farePathIter)->farePath->itin() == nullptr)
      prepareJourneyItin(**farePathIter);

    std::vector<std::vector<TravelSeg*> > backupFMTravelSegVect;
    std::vector<PricingUnit*>::iterator puIt = (*farePathIter)->farePath->pricingUnit().begin();
    const std::vector<PricingUnit*>::iterator puItEnd =
        (*farePathIter)->farePath->pricingUnit().end();
    for (; puIt != puItEnd; ++puIt)
    {
      if (!isPUValidForFareRetailerCode(*puIt, *farePathIter, farePathStatus, isDiag965))
        break;

      SetFareMarketGlobalDirection fmGlobalDirection(**puIt, (*farePathIter)->globalDirection);

      prepareFMTvlSegs(**puIt, backupFMTravelSegVect);
      farePathStatus = FP_VALID;
      // Itin *itin = (*((*farePathIter)->farePath)).itin();
      // FareUsage* failedFareUsage = 0;
      if (!puRuleController.validate(*_fFTrx, *((*farePathIter)->farePath), **puIt))
      // if(!puRuleController.validate(*_fFTrx, **puIt, failedFareUsage, *itin))
      {
        farePathStatus = FP_NOT_VALID; // invalidated by journayValidation
        if (UNLIKELY(isDiag965))
          *((*farePathIter)->invalidatedCategory) = getInvalidatingCategory(*puIt);

        break;
      }
      restoreFMTvlSegs(**puIt, backupFMTravelSegVect);
    }

    // if each PU is valid then perform FP validation
    if (farePathStatus == FP_VALID)
    {
      if (!isFPValidForFareRetailerCode(*(*farePathIter)->farePath, *farePathIter, farePathStatus,
                                        isDiag965))
        break;

      puIt = (*farePathIter)->farePath->pricingUnit().begin();
      for (; puIt != puItEnd; ++puIt)
      {
        SetFareMarketGlobalDirection fmGlobalDirection(**puIt, (*farePathIter)->globalDirection);

        prepareFMTvlSegs(**puIt, backupFMTravelSegVect);

        if (!fpRuleController.validate(*_fFTrx, *((*farePathIter)->farePath), **puIt))
        {
          farePathStatus = FP_NOT_VALID; // invalidated by journayValidation
          if (UNLIKELY(isDiag965))
            *((*farePathIter)->invalidatedCategory) = getInvalidatingCategory(*puIt);

          break;
        }

        restoreFMTvlSegs(**puIt, backupFMTravelSegVect);
      }
    }

    *((*farePathIter)->farePathStatus) = farePathStatus;

    if ((_fFTrx->bffStep() == FlightFinderTrx::STEP_4 ||
         _fFTrx->bffStep() == FlightFinderTrx::STEP_2) &&
        farePathStatus == FP_VALID)
    {
      storeFarePathStatusForSkip(**farePathIter, passedDates);
    }
  }

  if (UNLIKELY(isDiag966))
    showDiag966Footer();
}

void
FlightFinderJourneyValidator::prepareFMTvlSegs(
    const PricingUnit& pricingUnit, std::vector<std::vector<TravelSeg*> >& backupFMTravelSegVect)
{
  FareUsage* frontFareUsage = pricingUnit.fareUsage().front();
  backupFMTravelSegVect.clear();
  std::vector<TravelSeg*> backupFirstFUFMTravelSegVect;
  backupFirstFUFMTravelSegVect.swap(frontFareUsage->paxTypeFare()->fareMarket()->travelSeg());
  frontFareUsage->paxTypeFare()->fareMarket()->travelSeg().insert(
      frontFareUsage->paxTypeFare()->fareMarket()->travelSeg().end(),
      frontFareUsage->travelSeg().begin(),
      frontFareUsage->travelSeg().end());
  frontFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::DAY_TIME_RULE, false);
  if (frontFareUsage->travelSeg().front()->segmentType() == Open)
  {
    frontFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, true);
  }
  else
  {
    frontFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, false);
  }
  backupFMTravelSegVect.push_back(backupFirstFUFMTravelSegVect);
  if (pricingUnit.fareUsage().size() > 1)
  {
    std::vector<TravelSeg*> backupLastFUFMTravelSegVect;
    FareUsage* lastFareUsage = pricingUnit.fareUsage().back();
    backupLastFUFMTravelSegVect.swap(lastFareUsage->paxTypeFare()->fareMarket()->travelSeg());
    lastFareUsage->paxTypeFare()->fareMarket()->travelSeg().insert(
        lastFareUsage->paxTypeFare()->fareMarket()->travelSeg().end(),
        lastFareUsage->travelSeg().begin(),
        lastFareUsage->travelSeg().end());
    lastFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::DAY_TIME_RULE, false);
    if (lastFareUsage->travelSeg().back()->segmentType() == Open)
    {
      lastFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, true);
    }
    else
    {
      lastFareUsage->paxTypeFare()->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, false);
    }

    backupFMTravelSegVect.push_back(backupLastFUFMTravelSegVect);
  }
}

void
FlightFinderJourneyValidator::restoreFMTvlSegs(
    const PricingUnit& pricingUnit, std::vector<std::vector<TravelSeg*> >& backupFMTravelSegVect)
{
  FareUsage* frontFareUsage = pricingUnit.fareUsage().front();
  frontFareUsage->paxTypeFare()->fareMarket()->travelSeg().swap(backupFMTravelSegVect.front());

  if (pricingUnit.fareUsage().size() > 1)
  {
    FareUsage* lastFareUsage = pricingUnit.fareUsage().back();
    lastFareUsage->paxTypeFare()->fareMarket()->travelSeg().swap(backupFMTravelSegVect.back());
  }
}

void
FlightFinderJourneyValidator::prepareJourneyItin(const FarePathType& farePathData)
{
  Itin*& journeyItin = _fFTrx->journeyItin();

  journeyItin->travelSeg().clear();
  journeyItin->travelSeg().insert(journeyItin->travelSeg().end(),
                                  farePathData.travelSegVVect.begin(),
                                  farePathData.travelSegVVect.end());
  farePathData.farePath->itin() = _fFTrx->journeyItin();
}

PricingUnit*
FlightFinderJourneyValidator::createPricingUnit(
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& PUData)
{
  PricingUnit* pu = _fFTrx->dataHandle().create<PricingUnit>();

  uint8_t legNum = 0;
  std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >::iterator pUDataIter =
      PUData.begin();
  for (; pUDataIter != PUData.end(); ++pUDataIter, ++legNum)
  {
    FareUsage* fu = _fFTrx->dataHandle().create<FareUsage>();
    fu->paxTypeFare() = pUDataIter->second;
    fu->travelSeg().insert(
        fu->travelSeg().begin(), pUDataIter->first.begin(), pUDataIter->first.end());
    fu->inbound() = legNum;
    pu->fareUsage().push_back(fu);
    pu->travelSeg().insert(pu->travelSeg().end(), fu->travelSeg().begin(), fu->travelSeg().end());
    pu->geoTravelType() = pUDataIter->second->fareMarket()->geoTravelType();
  }

  pu->paxType() = getActualPaxType(PUData);

  if (PUData.size() == 1)
  {
    pu->puType() = PricingUnit::Type::ONEWAY;
  }
  else
  {
    pu->puType() = PricingUnit::Type::ROUNDTRIP;
  }

  return pu;
}

void
FlightFinderJourneyValidator::setTravelSegVect(
    std::vector<TravelSeg*>& travelSegVVect,
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& PUData)
{
  std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >::const_iterator pUDataIter =
      PUData.begin();
  for (; pUDataIter != PUData.end(); ++pUDataIter)
  {
    travelSegVVect.insert(travelSegVVect.end(), pUDataIter->first.begin(), pUDataIter->first.end());
  }
}
void
FlightFinderJourneyValidator::getTravelSeg(std::vector<TravelSeg*>& travelSeg,
                                           const uint32_t legId,
                                           const uint32_t& originalSopIndex)
{

  std::vector<ShoppingTrx::SchedulingOption>& sop = _fFTrx->legs()[legId].sop();
  const Itin* sopItin = sop[originalSopIndex].itin();
  travelSeg.insert(travelSeg.end(), sopItin->travelSeg().begin(), sopItin->travelSeg().end());
}

GlobalDirection
FlightFinderJourneyValidator::getGlobalDirection(const uint32_t legId, const uint32_t sopIndex)
{
  const ShoppingTrx::SchedulingOption& sop = _fFTrx->legs()[legId].sop()[sopIndex];
  return sop.globalDirection();
}

void
FlightFinderJourneyValidator::prepareSOPDataVect(FarePathSOPDataVectType& farePathSOPDataVect)
{
  if (!_outboundDateflightMap.empty())
  {
    FlightFinderTrx::OutBoundDateFlightMapIC itODFM = _outboundDateflightMap.begin();
    for (; itODFM != _outboundDateflightMap.end(); ++itODFM)
    {
      if ((itODFM->second)->iBDateFlightMap.size() > 0)
      {
        uint16_t outboundSopPosition = 0;
        std::vector<FlightFinderTrx::SopInfo*>::const_iterator sopOIter =
            itODFM->second->flightInfo.flightList.begin();
        for (; sopOIter != itODFM->second->flightInfo.flightList.end();
             ++sopOIter, ++outboundSopPosition)
        {
          addFarePathSOPData(farePathSOPDataVect,
                             &(*itODFM),
                             &(*itODFM->second->iBDateFlightMap.begin()),
                             outboundSopPosition,
                             (*sopOIter)->sopIndex,
                             true);
        }
        FlightFinderTrx::InboundDateFlightMap::const_iterator itIDFM =
            itODFM->second->iBDateFlightMap.begin();
        for (; itIDFM != (itODFM->second)->iBDateFlightMap.end(); ++itIDFM)
        {
          uint16_t inboundSopPosition = 0;
          std::vector<FlightFinderTrx::SopInfo*>::const_iterator sopIIter =
              itIDFM->second->flightList.begin();
          for (; sopIIter != itIDFM->second->flightList.end(); ++sopIIter, ++inboundSopPosition)
          {
            addFarePathSOPData(farePathSOPDataVect,
                               &(*itODFM),
                               &(*itIDFM),
                               inboundSopPosition,
                               (*sopIIter)->sopIndex,
                               false);
          }
        }
      }
      else
      {
        uint16_t outboundSopPosition = 0;
        std::vector<FlightFinderTrx::SopInfo*>::const_iterator sopOIter =
            itODFM->second->flightInfo.flightList.begin();
        for (; sopOIter != itODFM->second->flightInfo.flightList.end();
             ++sopOIter, ++outboundSopPosition)
        {
          addFarePathSOPData(
              farePathSOPDataVect, &(*itODFM), nullptr, outboundSopPosition, (*sopOIter)->sopIndex, true);
        }
      }
    }
  }
}

void
FlightFinderJourneyValidator::addFarePathSOPData(
    FarePathSOPDataVectType& farePathSOPDataVect,
    const std::pair<const DateTime, FlightFinderTrx::OutBoundDateInfo*>* outboundData,
    const std::pair<const DateTime, FlightFinderTrx::FlightDataInfo*>* inboundData,
    uint16_t sopPosition,
    uint32_t sopIndex,
    bool thisIsOutbound)
{
  uint16_t farePosition = 0;
  const ShoppingTrx::AltDatePairs& altDatesMap = _fFTrx->altDatePairs();

  if (thisIsOutbound)
  {
    std::vector<TravelSeg*> tvlSegOut;
    std::vector<TravelSeg*> tvlSegIn;

    getTravelSeg(tvlSegOut, 0, sopIndex);
    const GlobalDirection gd = getGlobalDirection(0, sopIndex);

    if (inboundData && !_fFTrx->isAltDates()) // RT
    {
      tvlSegIn.push_back(_fareMarketSegBackup.back());
    }

    std::vector<PaxTypeFare*>::iterator itOPTFs =
        outboundData->second->flightInfo.flightList[sopPosition]->paxTypeFareVect.begin();
    for (;
         itOPTFs != outboundData->second->flightInfo.flightList[sopPosition]->paxTypeFareVect.end();
         ++itOPTFs, ++farePosition)
    {
      if (inboundData) // RT
      {
        std::vector<PaxTypeFare*>::iterator itIPTFs =
            inboundData->second->flightList.front()->paxTypeFareVect.begin();
        for (; itIPTFs != inboundData->second->flightList.front()->paxTypeFareVect.end(); ++itIPTFs)
        {
          if (!_fFTrx->isAltDates())
          {
            FarePathSOPDataType farePathSOPData;

            farePathSOPData.sopElementData.outboundData = outboundData;
            farePathSOPData.sopElementData.inboundData = inboundData;

            farePathSOPData.sopElementData.outboundSopPosition = sopPosition;
            farePathSOPData.sopElementData.inboundSopPosition = 0;

            farePathSOPData.farePathData.push_back(std::make_pair(tvlSegOut, *itOPTFs));

            // it's RT, for outbound add inbound open seg
            farePathSOPData.farePathData.push_back(std::make_pair(tvlSegIn, *itIPTFs));

            farePathSOPData.invalidatedCategory = 0;
            farePathSOPData.errorMsg.clear();
            farePathSOPData.farePathStatus = FP_VALID;
            farePathSOPData.thisIsOutbound = thisIsOutbound;
            farePathSOPData.sopElementData.farePosition = farePosition;

            farePathSOPData.globalDirection = gd;

            farePathSOPDataVect.push_back(farePathSOPData);
          }
          else
          {
            // it's RT, for outbound add inbound open seg
            FlightFinderTrx::InboundDateFlightMap::const_iterator itIDFM =
                outboundData->second->iBDateFlightMap.begin();
            for (; itIDFM != (outboundData->second)->iBDateFlightMap.end(); ++itIDFM)
            {
              FarePathSOPDataType farePathSOPData;

              farePathSOPData.sopElementData.outboundData = outboundData;
              farePathSOPData.sopElementData.inboundData = &(*itIDFM); // inboundData;

              farePathSOPData.sopElementData.outboundSopPosition = sopPosition;
              farePathSOPData.sopElementData.inboundSopPosition = 0;

              farePathSOPData.farePathData.push_back(std::make_pair(tvlSegOut, *itOPTFs));

              Itin* journeyItin = ShoppingAltDateUtil::getJourneyItin(
                  altDatesMap, std::make_pair(outboundData->first, itIDFM->first));
              tvlSegIn.clear();
              tvlSegIn.push_back(journeyItin->travelSeg().back());
              farePathSOPData.farePathData.push_back(std::make_pair(tvlSegIn, *itIPTFs));

              farePathSOPData.invalidatedCategory = 0;
              farePathSOPData.errorMsg.clear();
              farePathSOPData.farePathStatus = FP_VALID;
              farePathSOPData.thisIsOutbound = thisIsOutbound;
              farePathSOPData.sopElementData.farePosition = farePosition;

              farePathSOPData.globalDirection = gd;

              farePathSOPDataVect.push_back(farePathSOPData);
            }
          }
        }
      }
      else // OW
      {
        FarePathSOPDataType farePathSOPData;

        farePathSOPData.sopElementData.outboundData = outboundData;
        farePathSOPData.sopElementData.inboundData = inboundData;

        farePathSOPData.sopElementData.outboundSopPosition = sopPosition;
        farePathSOPData.sopElementData.inboundSopPosition = 0;

        farePathSOPData.farePathData.push_back(std::make_pair(tvlSegOut, *itOPTFs));

        farePathSOPData.invalidatedCategory = 0;
        farePathSOPData.errorMsg.clear();
        farePathSOPData.farePathStatus = FP_VALID;
        farePathSOPData.thisIsOutbound = thisIsOutbound;
        farePathSOPData.sopElementData.farePosition = farePosition;

        farePathSOPData.globalDirection = gd;

        farePathSOPDataVect.push_back(farePathSOPData);
      }
    }
  }
  else
  {
    std::vector<TravelSeg*> tvlSegOut;

    if (!_fFTrx->isAltDates())
    {
      tvlSegOut.push_back(_fareMarketSegBackup.front());
    }
    else
    {
      Itin* journeyItin = ShoppingAltDateUtil::getJourneyItin(
          altDatesMap, std::make_pair(outboundData->first, inboundData->first));
      tvlSegOut.push_back(journeyItin->travelSeg().front());
    }

    std::vector<TravelSeg*> tvlSegIn;
    getTravelSeg(tvlSegIn, 1, sopIndex);
    const GlobalDirection gd = getGlobalDirection(1, sopIndex);

    std::vector<PaxTypeFare*>::iterator itIPTFs =
        inboundData->second->flightList[sopPosition]->paxTypeFareVect.begin();
    for (; itIPTFs != inboundData->second->flightList[sopPosition]->paxTypeFareVect.end();
         ++itIPTFs, ++farePosition)
    {
      std::vector<PaxTypeFare*>::iterator itOPTFs =
          outboundData->second->flightInfo.flightList.front()->paxTypeFareVect.begin();
      for (; itOPTFs != outboundData->second->flightInfo.flightList.front()->paxTypeFareVect.end();
           ++itOPTFs)
      {
        FarePathSOPDataType farePathSOPData;

        farePathSOPData.sopElementData.outboundData = outboundData;
        farePathSOPData.sopElementData.inboundData = inboundData;

        farePathSOPData.sopElementData.outboundSopPosition = 0;
        farePathSOPData.sopElementData.inboundSopPosition = sopPosition;
        if (outboundData->second->iBDateFlightMap.size() >
            0) // it's RT, for inbound add outbound open seg
        {
          farePathSOPData.farePathData.push_back(std::make_pair(tvlSegOut, *itOPTFs));
        }

        farePathSOPData.farePathData.push_back(std::make_pair(tvlSegIn, *itIPTFs));

        farePathSOPData.invalidatedCategory = 0;
        farePathSOPData.errorMsg.clear();
        farePathSOPData.farePathStatus = FP_VALID;
        farePathSOPData.thisIsOutbound = thisIsOutbound;
        farePathSOPData.sopElementData.farePosition = farePosition;

        farePathSOPData.globalDirection = gd;

        farePathSOPDataVect.push_back(farePathSOPData);
      }
    }
  }
}

void
FlightFinderJourneyValidator::prepareFarePathDataVect(FarePathSOPDataVectType& farePathSOPDataVect,
                                                      FarePathVectType& farePathVect)
{
  FarePathSOPDataVectType::iterator sopDataIter = farePathSOPDataVect.begin();
  for (; sopDataIter != farePathSOPDataVect.end(); ++sopDataIter)
  {
    FarePathType* fPData = buildFarePath(sopDataIter->farePathData,
                                         &(sopDataIter->farePathStatus),
                                         &(sopDataIter->invalidatedCategory),
                                         &(sopDataIter->errorMsg),
                                         nullptr,
                                         sopDataIter->globalDirection);
    farePathVect.push_back(fPData);
  }
}

FlightFinderJourneyValidator::FarePathType*
FlightFinderJourneyValidator::buildFarePath(
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData,
    uint8_t* farePathStatus,
    size_t* invalidatedCategory,
    std::string* errorMsg,
    Itin* itin,
    const GlobalDirection globalDirection)
{
  FarePathType* fPData = _fFTrx->dataHandle().create<FarePathType>();
  FarePath* farePath = _fFTrx->dataHandle().create<FarePath>();

  if (((_fFTrx->legs().size() == 1) && (farePathData.size() == 1)) || // OW or RT - 1pu 2fu
      ((_fFTrx->legs().size() == 2) && (farePathData.size() == 2)))
  {
    PricingUnit* pu;
    pu = createPricingUnit(farePathData);
    farePath->pricingUnit().push_back(pu);
    farePath->setTotalNUCAmount(calculateFarePathNUCAmount(farePathData));
    farePath->paxType() = getActualPaxType(farePathData);
    farePath->itin() = itin;
  }

  fPData->farePath = farePath;
  fPData->farePathStatus = farePathStatus;
  fPData->invalidatedCategory = invalidatedCategory;
  fPData->errorMsg = errorMsg;
  fPData->globalDirection = globalDirection;
  setTravelSegVect(fPData->travelSegVVect, farePathData);

  return fPData;
}

void
FlightFinderJourneyValidator::addOutbound(const uint16_t& farePosition,
                                          const uint16_t& outboundSopPosition,
                                          const DateTime& outboundDepartureDT,
                                          const FlightFinderTrx::FlightDataInfo* flightDataInfo)
{
  FlightFinderTrx::SopInfo* sopInfo = flightDataInfo->flightList[outboundSopPosition];

  if (_fFTrx->outboundDateflightMap().count(outboundDepartureDT) == 0)
  {
    FlightFinderTrx::OutBoundDateInfo* outBoundDateInfo =
        _fFTrx->dataHandle().create<FlightFinderTrx::OutBoundDateInfo>();

    outBoundDateInfo->flightInfo.flightList.push_back(buildSopInfo(farePosition, sopInfo, true));
    _fFTrx->outboundDateflightMap()[outboundDepartureDT] = outBoundDateInfo;
  }
  else
  {
    std::vector<FlightFinderTrx::SopInfo*>& flightList =
        _fFTrx->outboundDateflightMap()[outboundDepartureDT]->flightInfo.flightList;
    if (!alreadyAddedToFlightList(flightList, sopInfo))
    {
      flightList.push_back(buildSopInfo(farePosition, sopInfo, true));
    }
  }
}

void
FlightFinderJourneyValidator::addInbound(const uint16_t& farePosition,
                                         const uint16_t& inboundSopPosition,
                                         const DateTime& outboundDepartureDT,
                                         const DateTime& inboundDepartureDT,
                                         const FlightFinderTrx::FlightDataInfo* flightDataInfo)
{
  FlightFinderTrx::OutBoundDateFlightMap::const_iterator outboundIter =
      _fFTrx->outboundDateflightMap().find(outboundDepartureDT);
  if (outboundIter != _fFTrx->outboundDateflightMap().end())
  {
    FlightFinderTrx::SopInfo* sopInfo = flightDataInfo->flightList[inboundSopPosition];

    if (outboundIter->second->iBDateFlightMap.count(inboundDepartureDT) == 0)
    {
      FlightFinderTrx::FlightDataInfo* flightInfo =
          _fFTrx->dataHandle().create<FlightFinderTrx::FlightDataInfo>();
      flightInfo->flightList.push_back(buildSopInfo(farePosition, sopInfo, false));

      outboundIter->second->iBDateFlightMap[inboundDepartureDT] = flightInfo;
    }
    else
    {
      std::vector<FlightFinderTrx::SopInfo*>& flightList =
          outboundIter->second->iBDateFlightMap[inboundDepartureDT]->flightList;
      if (!alreadyAddedToFlightList(flightList, sopInfo))
      {
        flightList.push_back(buildSopInfo(farePosition, sopInfo, false));
      }
    }
  }
}

FlightFinderTrx::SopInfo*
FlightFinderJourneyValidator::buildSopInfo(const uint16_t& farePosition,
                                           FlightFinderTrx::SopInfo* sopInfo,
                                           bool thisIsOutbound)
{
  FlightFinderTrx::SopInfo* newSopInfo = _fFTrx->dataHandle().create<FlightFinderTrx::SopInfo>();

  if (!(_fFTrx->bffStep() == FlightFinderTrx::STEP_4 ||
        (thisIsOutbound && _fFTrx->bffStep() == FlightFinderTrx::STEP_6)))
  {
    newSopInfo->bkgCodeDataVect.push_back(sopInfo->bkgCodeDataVect[farePosition]);
  }

  newSopInfo->paxTypeFareVect.push_back(sopInfo->paxTypeFareVect[farePosition]);
  newSopInfo->sopIndex = sopInfo->sopIndex;

  return sopInfo;
}

void
FlightFinderJourneyValidator::updateOutboundDateflightMap(
    FarePathSOPDataVectType& farePathSOPDataVect)
{
  _fFTrx->outboundDateflightMap().clear();

  FarePathSOPDataVectType::iterator sopDataIter = farePathSOPDataVect.begin();
  for (; sopDataIter != farePathSOPDataVect.end(); ++sopDataIter)
  {
    if (sopDataIter->farePathStatus == FP_VALID)
    {
      if (sopDataIter->thisIsOutbound)
      {
        addOutbound(sopDataIter->sopElementData.farePosition,
                    sopDataIter->sopElementData.outboundSopPosition,
                    sopDataIter->sopElementData.outboundData->first.date(),
                    &(sopDataIter->sopElementData.outboundData->second->flightInfo));
      }
      else
      {
        addInbound(sopDataIter->sopElementData.farePosition,
                   sopDataIter->sopElementData.inboundSopPosition,
                   sopDataIter->sopElementData.outboundData->first.date(),
                   sopDataIter->sopElementData.inboundData->first.date(),
                   sopDataIter->sopElementData.inboundData->second);
      }
    }
  }

  if (_fFTrx->legs().size() > 1)
  {
    removeOutboundsWithoutInbounds();
  }
}

void
FlightFinderJourneyValidator::checkFarePathValidity(
    const FarePathSOPDataVectType& farePathSOPDataVect)
{
  if (fallback::fallbackPromotionalAvailability(_fFTrx))
    return;

  bool allFarePathInvalid = true;
  FarePathSOPDataVectType::const_iterator sopDataIter = farePathSOPDataVect.begin();
  for (; sopDataIter != farePathSOPDataVect.end(); ++sopDataIter)
  {
    if (sopDataIter->farePathStatus == FP_VALID)
    {
      allFarePathInvalid = false;
      break;
    }
  }

  if (allFarePathInvalid == true)
  {
    throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                 "No valid flight/date found");
  }
}

void
FlightFinderJourneyValidator::removeOutboundsWithoutInbounds()
{
  FlightFinderTrx::OutBoundDateFlightMap::iterator outboundIter =
      _fFTrx->outboundDateflightMap().begin();
  while (outboundIter != _fFTrx->outboundDateflightMap().end())
  {
    if (outboundIter->second->iBDateFlightMap.empty())
    {
      _fFTrx->outboundDateflightMap().erase(outboundIter);
      outboundIter = _fFTrx->outboundDateflightMap().begin();
    }
    else
    {
      ++outboundIter;
    }
  }
}

bool
FlightFinderJourneyValidator::alreadyAddedToFlightList(
    const std::vector<FlightFinderTrx::SopInfo*>& flightList,
    const FlightFinderTrx::SopInfo* sopInfo)
{
  std::vector<FlightFinderTrx::SopInfo*>::const_iterator sopIter = flightList.begin();
  for (; sopIter != flightList.end(); ++sopIter)
  {
    if ((*sopIter)->sopIndex == sopInfo->sopIndex)
    {
      return true;
    }
  }

  return false;
}

size_t
FlightFinderJourneyValidator::getInvalidatingCategory(PricingUnit* pu)
{
  std::vector<FareUsage*>::const_iterator fuIter = pu->fareUsage().begin();
  for (; fuIter != pu->fareUsage().end(); ++fuIter)
  {
    if ((*fuIter)->paxTypeFare())
    {
      const size_t numCategories = 50;
      size_t cat = 1;

      for (cat = 1; cat != numCategories; ++cat)
      {
        if ((*fuIter)->paxTypeFare()->isCategoryValid(cat) == false)
        {
          break;
        }
      }
      if (cat < numCategories)
      {
        return cat;
      }
    }
  }

  return 0;
}

MoneyAmount
FlightFinderJourneyValidator::calculateFarePathNUCAmount(
    const std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData)
{
  MoneyAmount moneyAmount = 0;
  std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >::const_iterator farePathDataIter =
      farePathData.begin();
  for (; farePathDataIter != farePathData.end(); ++farePathDataIter)
  {
    if (farePathDataIter->second)
    {
      moneyAmount += farePathDataIter->second->nucFareAmount();
    }
  }

  return moneyAmount;
}

PaxType*
FlightFinderJourneyValidator::getActualPaxType(
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData)
{
  if (farePathData.front().second)
  {
    return farePathData.front().second->actualPaxType();
  }
  else
  {
    return farePathData.back().second->actualPaxType();
  }
}

void
FlightFinderJourneyValidator::showDiag965(FarePathDataVectType& farePathDataVect)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(*_fFTrx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic965);

  diag << "***************************************************" << endl;
  diag << "Diagnostic 965 : RULE VALIDATION BEGIN" << endl;
  diag << "***************************************************" << endl;

  if (farePathDataVect.empty())
    diag << "FarePath is empty" << endl;
  else
  {
    uint16_t farePathCounter = 1;
    FarePathDataVectType::iterator farePathDataIter = farePathDataVect.begin();
    for (; farePathDataIter != farePathDataVect.end(); ++farePathDataIter, ++farePathCounter)
    {
      diag << "Fare Path: " << farePathCounter << endl;

      if (farePathDataIter->datePair->second.isEmptyDate())
        diag << "OutboundDate: " << farePathDataIter->datePair->first.date();
      else
        diag << "OutboundDate: " << farePathDataIter->datePair->first.date()
             << " InboundDate: " << farePathDataIter->datePair->second.date();

      // if(farePathDataIter->fBInfo->flightBitStatus == FP_VALID)
      if (farePathDataIter->farePathStatus == FP_VALID)
        diag << " - VALID";
      else
      {
        diag << " - NOT VALID";
        if (farePathDataIter->invalidatedCategory != 0)
          diag << " CAT " << farePathDataIter->invalidatedCategory;

        if (!farePathDataIter->errorMsg.empty())
          diag << ": " <<  farePathDataIter->errorMsg;
      }
      diag << "\nTotal NUC amount: " << calculateFarePathNUCAmount(farePathDataIter->farePathData)
           << endl;
      diag << "Outbound Fare Basis "
           << farePathDataIter->farePathData.front().second->createFareBasis(*_fFTrx, false);
      if (farePathDataIter->farePathData.size() > 1)
        diag << " - Inbound Fare Basis "
             << farePathDataIter->farePathData.back().second->createFareBasis(*_fFTrx, false);
      diag << endl;
    }
  }

  diag << "***************************************************" << endl;
  diag << "Diagnostic 965 : RULE VALIDATION END" << endl;
  diag << "***************************************************" << endl;
  diag << endl;

  diag.flushMsg();
  diag.disable(Diagnostic965);
}

void
FlightFinderJourneyValidator::showDiag965(FarePathSOPDataVectType& farePathSOPDataVect)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(*_fFTrx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic965);

  diag << "***************************************************" << endl;
  diag << "Diagnostic 965 : RULE VALIDATION BEGIN" << endl;
  diag << "***************************************************" << endl;

  if (farePathSOPDataVect.empty())
  {
    diag << "FarePath is empty" << endl;
  }
  else
  {
    uint16_t farePathCounter = 1;
    FarePathSOPDataVectType::iterator farePathDataIter = farePathSOPDataVect.begin();
    for (; farePathDataIter != farePathSOPDataVect.end(); ++farePathDataIter, ++farePathCounter)
    {
      diag << "Fare Path: " << farePathCounter;
      if (farePathDataIter->farePathStatus == FP_VALID)
      {
        diag << " - VALID";
      }
      else if (farePathDataIter->farePathStatus == FP_SKIPPED)
      {
        diag << " - SKIPPED";
      }
      else
      {
        diag << " - NOT VALID";
        if (farePathDataIter->invalidatedCategory != 0)
          diag << " CAT " << farePathDataIter->invalidatedCategory;

        if (!farePathDataIter->errorMsg.empty())
          diag << ": " << farePathDataIter->errorMsg;
      }
      diag << "\nTotal NUC amount: " << calculateFarePathNUCAmount(farePathDataIter->farePathData)
           << endl;
      diag << "Outbound Fare Basis "
           << farePathDataIter->farePathData.front().second->createFareBasis(*_fFTrx, false);
      if (farePathDataIter->farePathData.size() > 1)
      {
        diag << " - Inbound Fare Basis "
             << farePathDataIter->farePathData.back().second->createFareBasis(*_fFTrx, false);
      }
      diag << endl;
      diag << getTravelSegmentsInfo(*farePathDataIter) << endl;
    }

    diag << "***************************************************" << endl;
    diag << "Diagnostic 965 : RULE VALIDATION END" << endl;
    diag << "***************************************************" << endl;
    diag << endl;

    diag.flushMsg();
    diag.disable(Diagnostic965);
  }
}

std::string
FlightFinderJourneyValidator::getTravelSegmentsInfo(FarePathSOPDataType& farePathSOPData)
{
  std::ostringstream output;

  if (farePathSOPData.farePathData.size() > 1) // rt
  {
    if (farePathSOPData.thisIsOutbound)
    {
      output << " Outbound Date: " << farePathSOPData.sopElementData.outboundData->first.date()
             << " SOP "
             << farePathSOPData.sopElementData.outboundData->second->flightInfo.flightList
                    [farePathSOPData.sopElementData.outboundSopPosition]->sopIndex
             << " - Inbound Open Segment" << std::endl;
    }
    else
    {
      output << " Outbound Open Segment - "
             << "Inbound Date:" << farePathSOPData.sopElementData.inboundData->first.date()
             << " SOP " << farePathSOPData.sopElementData.inboundData->second->flightList
                               [farePathSOPData.sopElementData.inboundSopPosition]->sopIndex
             << std::endl;
    }
  }
  else // ow
  {
    output << "SOP " << farePathSOPData.sopElementData.outboundData->second->flightInfo.flightList
                            [farePathSOPData.sopElementData.outboundSopPosition]->sopIndex
           << std::endl;
  }

  std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >::iterator farePathDataIter =
      farePathSOPData.farePathData.begin();
  for (; farePathDataIter != farePathSOPData.farePathData.end(); ++farePathDataIter)
  {
    output << getSOPInfo(farePathDataIter->first);
  }

  return output.str();
}

std::string
FlightFinderJourneyValidator::getSOPInfo(std::vector<TravelSeg*>& tvlSegmentVect)
{
  std::ostringstream output;

  std::string buffer("");
  std::vector<TravelSeg*>::const_iterator travelSegIter = tvlSegmentVect.begin();
  for (size_t count = 1; travelSegIter != tvlSegmentVect.end(); ++travelSegIter, ++count)
  {
    const AirSeg* curSeg = dynamic_cast<const AirSeg*>(*travelSegIter);
    if (curSeg == nullptr)
    {
      continue;
    }

    buffer = curSeg->origAirport() + curSeg->destAirport();
    const DateTime& depDT = curSeg->departureDT();
    const DateTime& arrDT = curSeg->arrivalDT();
    std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
    std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");

    output << "  SEGMENT #" << count << "  - ";

    if (curSeg->segmentType() != Arunk)
    {
      output << std::setw(4) << curSeg->carrier() << std::setw(6) << curSeg->flightNumber()
             << std::setw(8) << buffer << std::setw(6) << depDT.dateToString(DDMMM, "")
             << std::setw(7) << depDTStr << std::setw(7) << arrDTStr << std::setw(2)
             << ((curSeg->segmentType() == Open) ? "O" : "") << "\n";
    }
    else
    {
      output << std::setw(4) << " " << std::setw(6) << "ARUNK" << std::setw(8) << buffer
             << std::setw(6) << depDT.dateToString(DDMMM, "") << std::setw(7) << depDTStr
             << std::setw(7) << arrDTStr << std::setw(2)
             << ((curSeg->segmentType() == Open) ? "O" : "") << "\n";
    }
  }

  return output.str();
}

bool
FlightFinderJourneyValidator::skipFarePath(const FarePathType& farePathData,
                                           std::set<DateTime>& passedDates)
{
  if (_fFTrx->bffStep() == FlightFinderTrx::STEP_4)
  {
    // first segment is open
    TravelSeg* outboundFirstTvlSeg =
        farePathData.farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front();
    if (outboundFirstTvlSeg->segmentType() == Open)
    {
      // forst segment after open segment has departure time that was already validated
      TravelSeg* inboundFirstTvlSeg =
          farePathData.farePath->pricingUnit().front()->fareUsage().back()->travelSeg().front();
      if (passedDates.count(inboundFirstTvlSeg->departureDT().date()) != 0)
      {
        return true;
      }
    }
  }
  else if (_fFTrx->bffStep() == FlightFinderTrx::STEP_2)
  {
    TravelSeg* outboundFirstTvlSeg =
        farePathData.farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front();
    if (passedDates.count(outboundFirstTvlSeg->departureDT().date()) != 0)
    {
      return true;
    }
  }

  return false;
}

void
FlightFinderJourneyValidator::storeFarePathStatusForSkip(const FarePathType& farePathData,
                                                         std::set<DateTime>& passedDates)
{
  if (_fFTrx->bffStep() == FlightFinderTrx::STEP_4)
  {
    // first segment is open
    TravelSeg* outboundFirstTvlSeg =
        farePathData.farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front();
    if (outboundFirstTvlSeg->segmentType() == Open)
    {
      TravelSeg* inboundFirstTvlSeg =
          farePathData.farePath->pricingUnit().front()->fareUsage().back()->travelSeg().front();
      if (inboundFirstTvlSeg->segmentType() != Open)
      {
        passedDates.insert(inboundFirstTvlSeg->departureDT().date());
      }
    }
  }
  else if (_fFTrx->bffStep() == FlightFinderTrx::STEP_2)
  {
    // first segment is open
    TravelSeg* outboundFirstTvlSeg =
        farePathData.farePath->pricingUnit().front()->fareUsage().front()->travelSeg().front();
    passedDates.insert(outboundFirstTvlSeg->departureDT().date());
  }
}

void
FlightFinderJourneyValidator::showDiag966Header()
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(*_fFTrx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic555);

  diag << "***************************************************" << endl;
  diag << "Diagnostic 966 : RULE VALIDATION PU STATUSES BEGIN" << endl;
  diag << "***************************************************" << endl;

  diag.flushMsg();
}

void
FlightFinderJourneyValidator::showDiag966Footer()
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(*_fFTrx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic555);

  diag << "***************************************************" << endl;
  diag << "Diagnostic 966 : RULE VALIDATION PU STATUSES BEGIN" << endl;
  diag << "***************************************************" << endl;

  diag.flushMsg();
}

void
FlightFinderJourneyValidator::showDiag966(uint16_t farePathCounter)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(*_fFTrx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic555);

  diag << "Fare Path: " << farePathCounter << endl;

  diag.flushMsg();
}

} // tse
