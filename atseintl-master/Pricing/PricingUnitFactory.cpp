
// File:    PricingUnitFactory.cpp
// Created: July 2004
// Authors: Mohammad Hossan
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/PricingUnitFactory.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FxCnException.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Fares/FareByRuleValidator.h"
#include "Pricing/Combinations.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/StructuredFareRulesUtils.h"
#include "Rules/FarePUResultContainer.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Util/Algorithm/Container.h"

#include <algorithm>

namespace tse
{

namespace
{
bool
isDiagForMaxPenalty(const Diagnostic& diag)
{
  return diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL,
                             Diagnostic::MAX_PEN);
}
}

FALLBACK_DECL(fallbackDisplaySelectiveItinNum)
FALLBACK_DECL(noDelayedValidationForAwardRequest)
FALLBACK_DECL(failPricingUnitsInIs)
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackPrevFailedFareTuning)
FALLBACK_DECL(fallbackPrevFailedFarePairTuning)

const BookingCode PricingUnitFactory::ALL_BOOKING_CODE = "*";

static Logger
logger("atseintl.Pricing.PricingUnitFactory");

typedef std::map<const PaxTypeFare*, std::set<DatePair> > ALTDATE_IS_FAIL_FARE_MAP;

void
PricingUnitFactory::JourneyPULowerBound::setJourneyPULowerBound(MoneyAmount lbAmt)
{
  const boost::lock_guard<boost::mutex> g(_mutex);
  if (_journeyPULowerBound < 0)
  {
    _journeyPULowerBound = lbAmt;
    LOG4CXX_DEBUG(logger, "_journeyPULowerBound=" << _journeyPULowerBound)
    return;
  }

  if (lbAmt < _journeyPULowerBound)
  {
    _journeyPULowerBound = lbAmt;
  }
}

bool
PricingUnitFactory::initPricingUnitPQ(DiagCollector& diag)
{
  TSELatencyData metrics(*_trx, "PO INIT PU PQ");
  diag.enable(pu(), Diagnostic603, Diagnostic601, Diagnostic605);

  const ArrayVector<uint16_t> fareIndices(_pu->fareMarket().size(), 0);

  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(*_trx);

  if (LIKELY(fcConfig))
  {
    _multiCurrencyPricing = ItinUtil::applyMultiCurrencyPricing(_trx, *_itin);

    if (_multiCurrencyPricing)
    {
      LOG4CXX_DEBUG(logger, "PU FACTORY WILL PERFORM MULTI CURRENCY PRICING");
    }
    else
    {
      LOG4CXX_DEBUG(logger, "PU FACTORY WILL PERFORM SINGLE CURRENCY PRICING");
    }
  }

  if (UNLIKELY(TrxUtil::reuseFarePUResult() && _pu->fareMarket().size() > 1 && !_farePUResultContainer))
    _farePUResultContainer = FarePUResultContainer::createFarePUResultContainer(*_trx);

  PricingOptions* options = _trx->getOptions();

  if (UNLIKELY(!options->alternateCurrency().empty()))
  {
    _altCurrencyCode = options->alternateCurrency();
    _altCurrencyRequest = true;
  }

  // 1st param 0, since init, no prev PUPQItem from which to build the next
  // 3nd param 0, expansionPoint (xPoint) is 0 for init
  //
  if (UNLIKELY(!buildPricingUnit(nullptr, fareIndices, 0, diag)))
  {
    LOG4CXX_DEBUG(logger, "Init PUPQ failed, NO more valid fare");
    return false;
  }

  _reqPUCount = puCountDiagParam();

  return true;
}

PUPQItem*
PricingUnitFactory::getPUPQItem(uint32_t idx, DiagCollector& diag)
{
  TSELatencyData metrics(*_trx, "PO GET PU");

  ++_getPUPQItemCounter;

  if (idx < _puCount)
    return _validPUPQItem[idx];

  if (UNLIKELY(_shutdownFactory))
    return nullptr;

  buildNextLevelPricinUnit(diag);

  if (idx < _puCount)
  {
    return _validPUPQItem[idx];
  }
  else if (_pausedPUPQItem != nullptr)
  {
    return _pausedPUPQItem;
  }
  else
  {
    return nullptr;
  }
}

bool
PricingUnitFactory::buildNextLevelPricinUnit(DiagCollector& diag)
{
  if ((_pausedPUPQItem != nullptr) && validatePausedPricingUnit(diag))
  {
    return true;
  }

  if (pqEmpty())
  {
    LOG4CXX_INFO(logger, "buildNextLevelPricinUnit: _lthPUPQ Empty");
    _done = true;
    return false;
  }

  uint32_t genCount = 0;
  bool validFound = false;

  PUPQItem* pupqItem = pqTop();

  _curCombTried = 0; // during this request

  do
  {
    PricingUtil::checkTrxAborted(*_trx, _maxNbrCombMsgThreshold, _puCombTried, _maxNbrCombMsgSet);

    pqPop();

    if (!checkAltDatesMIPCutOff(*pupqItem))
    {
      // Too expensive, no need to continue the loop
      return true;
    }

    ++_puCombTried; // grand total
    ++_curCombTried; // during this request

    bool valid = pupqItem->isValid();

    if (!valid)
    {
      diag.enable(pupqItem->pu(), Diagnostic603, Diagnostic605);
      if (UNLIKELY(diag.isActive()))
      {
        if (!DiagnosticUtil::filter(*_trx, *pupqItem->pricingUnit()))
        {
          displayPricingUnit(*pupqItem->pricingUnit(), diag);
          if (diag.diagnosticType() == Diagnostic605)
            displayPrevFailedFC(*pupqItem->pricingUnit(), diag);
          diag.printLine();
        }
      }
    }

    if (valid)
    {
      pupqItem->clearReusedFareUsage();
      bool allowDelayedValidation = true;
      if(!fallback::noDelayedValidationForAwardRequest(_trx) &&
          _trx->awardRequest() )
      {
        allowDelayedValidation = false; 
      }
      valid = isPricingUnitValid(*pupqItem, allowDelayedValidation, diag);
    }

    // When diag param PU > 1 is passed in, it would not do
    // short-ckt. will generate all combination
    //
    if (!stopBuildingPU() || _reqPUCount > 1)
    {
      buildNextPricingUnitSet(*pupqItem, diag);
    }

    if (valid)
    {
      if (UNLIKELY(pupqItem->cloneFareUsage(*_trx, _fuPool) == false))
      {
        // failed to clone, system out of memory?
        return false;
      }
      if (!pupqItem->paused())
      {
        _validPUPQItem.push_back(pupqItem);
        ++_puCount;
        setJourneyPULowerBound(*pupqItem);
      }
      validFound = true;
    }

    if (!valid)
    {
      // need more testing for Cxr-Fare combo search
      // if(_pu->cxrFarePreferred() && pupqItem->priorityStatus().farePriority() == DEFAULT_PRIORITY)
      //{
      //  _failedCxrFareIndices.insert(std::vector<uint16_t>(pupqItem->fareIndices().begin(),
      // pupqItem->fareIndices().end()));
      //}

      releasePUPQItem(pupqItem);
    }

    if (_shutdownFactory)
    {
      return true;
    }

    if (pqEmpty())
    {
      _done = true;
      LOG4CXX_INFO(logger, "buildNextLevelPricinUnit: _puPQ Empty");
      return true;
    }
    pupqItem = pqTop();

    ++genCount;

  } while (!validFound || genCount < _reqPUCount);

  return true;
}

bool
PricingUnitFactory::validatePausedPricingUnit(DiagCollector& diag)
{
  PUPQItem* pupqItem = _pausedPUPQItem;
  _pausedPUPQItem = nullptr;

  pupqItem->paused() = false;

  bool allowDelayedValidation = true;
  if(!fallback::noDelayedValidationForAwardRequest(_trx) &&
      _trx->awardRequest())
  {
    allowDelayedValidation = false;
  }
  bool valid = isPricingUnitValid(*pupqItem, allowDelayedValidation, diag);
  if (valid)
  {
    if (!pupqItem->paused())
    {
      _validPUPQItem.push_back(pupqItem);
      ++_puCount;
      return true;
    }
    _pausedPUPQItem = pupqItem;
    return true;
  }

  releasePUPQItem(pupqItem);
  return false;
}

void
PricingUnitFactory::buildNextPricingUnitSet(const PUPQItem& pupqItem, DiagCollector& diag)
{
  if (UNLIKELY(_done))
  {
    LOG4CXX_INFO(logger, "_done = true, No more PU to build");
    return;
  }

  const uint16_t totalMkt = _pu->fareMarket().size();

  const ArrayVector<uint16_t> fareIndices(pupqItem.fareIndices());

  uint16_t xPoint = pupqItem.xPoint();
  for (; xPoint < totalMkt; ++xPoint)
  {
    PricingUtil::checkTrxAborted(*_trx, _maxNbrCombMsgThreshold, _puCombTried, _maxNbrCombMsgSet);

    buildPricingUnit(&pupqItem, fareIndices, xPoint, diag);
  } // for

  return;
}

bool
PricingUnitFactory::buildPricingUnit(const PUPQItem* prevPUPQItem,
                                     const ArrayVector<uint16_t>& fareIndices,
                                     const uint16_t xPoint,
                                     DiagCollector& diag)
{
  bool initStage = false;
  if (prevPUPQItem == nullptr)
  {
    initStage = true;
  }

  // lint --e{413}
  // this instrumentation is disabled, as it was called many times,
  // and slowed the server down
  // TSELatencyData metrics( *_trx, "PO BUILDING PU" );

  PUPQItem* pupqItem = constructPUPQItem();
  if (UNLIKELY(pupqItem == nullptr))
  {
    return false;
  }

  bool noGatewayFare = _trx->fxCnException();
  if (_trx->getTrxType() != PricingTrx::IS_TRX)
  {
    FxCnException fxCnEx(*_trx, *_itin);
    noGatewayFare = noGatewayFare && fxCnEx.checkThruFare();
  }

  bool checkSameFareDate =
      ((_trx->excTrxType() == PricingTrx::AR_EXC_TRX ||
        _trx->excTrxType() == PricingTrx::AF_EXC_TRX) &&
       _pu->fareMarket().size() > 1 && static_cast<RexBaseTrx*>(_trx)->repriceWithSameFareDate());

  const uint16_t totalMkt = _pu->fareMarket().size();
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    bool fareFound = false;
    uint16_t fareIdx = fareIndices[mktIdx];

    // if this mkt is the expansionPoint, start from the next fare for this mkt
    if (!initStage)
    {
      if (mktIdx == xPoint)
      {
        ++fareIdx;
      }
      else
      {
        // The fare of the current index was used in a previous combination
        // Therefore, NO Fare Component level validation e.g. dir, currency, etc.
        // is needed
        //
        bool hasPrevFailedFareUsage = false;
        if (LIKELY(usePrevFareUsage(mktIdx, *prevPUPQItem, false, *pupqItem, diag, 
                                    hasPrevFailedFareUsage, checkSameFareDate)))
        {
          if(fallback::fallbackPrevFailedFarePairTuning(_trx))
	  {
            if(hasPrevFailedFareUsage &&  mktIdx < xPoint &&
               !fallback::fallbackPrevFailedFareTuning(_trx)) 
            {
              diag.enable(pupqItem->pu(), Diagnostic605);
              if (UNLIKELY(diag.isActive()))
              {
                if (!DiagnosticUtil::filter(*_trx, *pupqItem->pricingUnit()))
                {
                  displayPartiallyBuiltFailedPU(*pupqItem->pricingUnit(), diag);
                  diag.printLine();
                }
              }
              releasePUPQItem(pupqItem);
              return true;
            }
	  }
	  else
	  {
	     if(hasFailedFare(*pupqItem, hasPrevFailedFareUsage, mktIdx, xPoint))
	     {
                diag.enable(pupqItem->pu(), Diagnostic605);
                if (UNLIKELY(diag.isActive()))
                {
                  if (!DiagnosticUtil::filter(*_trx, *pupqItem->pricingUnit()))
                  {
                    displayPartiallyBuiltFailedPU(*pupqItem->pricingUnit(), diag);
                    diag.printLine();
                  }
                }
                releasePUPQItem(pupqItem);
                return true;
	     }
	  }
          continue;
        }
      }
    }

    const FareType dummyFareType;
    if (!buildFareUsage(nullptr,
                        mktIdx,
                        fareIdx,
                        fareFound,
                        *pupqItem,
                        nullptr,
                        false,
                        dummyFareType,
                        diag,
                        noGatewayFare,
                        checkSameFareDate))
    {
      // no more valid fare in this mkt
      if (!fareFound)
        diag << " NO MORE VALID FARE FOUND IN THIS MARKET FOR THIS PU" << std::endl;

      releasePUPQItem(pupqItem);
      return true;
    }
  } // end of loop for all the fareMarket

  copyPUTemplateInfo(*pupqItem);

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(*_trx)))
  {
    pupqItem->rebookClassesExists() = pupqItem->pricingUnit()->isRebookedClassesStatus();
    if (!checkRebookedClassesForRex(*pupqItem))
      pupqItem->isValid() = false;
  }

  pupqItem->xPoint() = xPoint; // next pu will expand from the next mkt
  setNegotiatedFarePriority(pupqItem);

  pqPush(pupqItem);

  return true;
}

bool
PricingUnitFactory::usePrevFareUsage(const uint16_t mktIdx,
                                     const PUPQItem& prevPUPQItem,
                                     bool cxrFarePhase,
                                     PUPQItem& pupqItem,
                                     DiagCollector& diag,
                                     bool& hasPrevFailedFareUsage,
                                     bool rexCheckSameFareDate)
{
  const std::vector<FareUsage*>& fareUsg = prevPUPQItem.pricingUnit()->fareUsage();

  if (UNLIKELY(mktIdx >= fareUsg.size()))
  {
    return false;
  }

  const uint32_t fareIdx = prevPUPQItem.fareIndices()[mktIdx];
  PricingUnit& prU = *pupqItem.pricingUnit();

  FareUsage* oldFareUsage = fareUsg[mktIdx];
  PaxTypeFare* paxTypeFare = oldFareUsage->paxTypeFare();

  if (UNLIKELY(rexCheckSameFareDate && !prU.fareUsage().empty() &&
      !RexBaseTrx::isSameFareDate(prU.fareUsage().front()->paxTypeFare(), paxTypeFare)))
  {
    pupqItem.isValid() = false;
  }

  if (UNLIKELY(!isValidFareTypeGroup(*paxTypeFare)))
  {
    return false;
  }

  if (oldFareUsage->travelSeg().empty())
  {
    // when prevPUPQItem is not valid, we avoid builing it completely
    // now we want reuse this FareUsage
    //
    oldFareUsage->travelSeg() = paxTypeFare->fareMarket()->travelSeg();
  }
  oldFareUsage->incrementRefCount();
  prU.fareUsage().push_back(oldFareUsage);
  if (UNLIKELY(oldFareUsage->isKeepFare()))
    prU.hasKeepFare() = true; // Rex Pricing flag
  if (!checkPrevValidationResult(paxTypeFare))
  {
    hasPrevFailedFareUsage = true;
    pupqItem.isValid() = false;
  }
  else
  {
    prU.travelSeg().insert(
        prU.travelSeg().end(), oldFareUsage->travelSeg().begin(), oldFareUsage->travelSeg().end());

    if (paxTypeFare->isSpecial())
    {
      prU.puFareType() = PricingUnit::SP;
    }
  }

  const MergedFareMarket& mfm = *(_pu->fareMarket()[mktIdx]);

  const Directionality puDir = _pu->fareDirectionality()[mktIdx];

  pupqItem.fareIndices().push_back(fareIdx);

  prU.setTotalPuNucAmount(prU.getTotalPuNucAmount() + paxTypeFare->totalFareAmount());

  MoneyAmount taxPerMarketBKCode = 0;

  if (TrxUtil::usesPrecalculatedTaxes(*_trx) && _taxPerFareBreak)
  {
    const CxrPrecalculatedTaxes& taxes =
        paxTypeFare->fareMarket()->paxTypeCortege(_paxType)->cxrPrecalculatedTaxes();
    taxPerMarketBKCode += taxes.getLowerBoundTotalTaxAmount(*paxTypeFare);
  }

  prU.setTotalPuNucAmount(prU.getTotalPuNucAmount() + taxPerMarketBKCode);
  prU.taxAmount() += taxPerMarketBKCode;
  prU.accumulateBaggageLowerBound(paxTypeFare->baggageLowerBound());

  prU.mileage() += paxTypeFare->mileage();

  bool lwPrio;
  setPriority(mfm, pupqItem, *paxTypeFare, puDir, cxrFarePhase, lwPrio);

  if (checkIS(*paxTypeFare) == false)
  {
    // Not good for any of the Dates to cover, no need to validate this
    pupqItem.isValid() = false;
  }

  if (!checkISIsValidFare(*paxTypeFare))
  {
    pupqItem.isValid() = false;
  }

  if (isOnlyNonStops() && !isNonStopFare(*paxTypeFare))
  {
    pupqItem.isValid() = false;
  }

  return true;
}

bool
PricingUnitFactory::buildFareUsage(const PaxTypeFare* primaryPaxTypeFare,
                                   const uint16_t mktIdx,
                                   uint16_t fareIdx,
                                   bool& fareFound,
                                   PUPQItem& pupqItem,
                                   const PaxTypeFare::SegmentStatusVec* segStatus,
                                   bool cxrFarePhase,
                                   const FareType& fareType,
                                   DiagCollector& diag,
                                   bool fxCnException,
                                   bool rexCheckSameFareDate)
{
  return buildFareUsageOld(primaryPaxTypeFare,
                           mktIdx,
                           fareIdx,
                           fareFound,
                           pupqItem,
                           segStatus,
                           cxrFarePhase,
                           fareType,
                           diag,
                           fxCnException,
                           rexCheckSameFareDate);
}

bool
PricingUnitFactory::sameBookingCodes(const PaxTypeFare::SegmentStatusVec& segStatus1,
                                     const PaxTypeFare::SegmentStatusVec& segStatus2)
{
  return (segStatus1.size() == segStatus2.size()) &&
         std::equal(segStatus1.cbegin(),
                    segStatus1.cend(),
                    segStatus2.cbegin(),
                    [](const PaxTypeFare::SegmentStatus& st1, const PaxTypeFare::SegmentStatus& st2)
                    { return st1._bkgCodeReBook == st2._bkgCodeReBook; });
}

void
PricingUnitFactory::setUpNewFUforCat31(FareUsage& fu, PricingUnit& pu, const PaxTypeFare& ptf) const
{
  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(*_trx)))
  {
    if (ptf.retrievalInfo() != nullptr && ptf.retrievalInfo()->keep())
    {
      fu.isKeepFare() = true;
      pu.hasKeepFare() = true;
      fu.ruleFailed() = !ptf.isValidNoBookingCode();
      pu.ruleFailedButSoftPassForKeepFare() = fu.ruleFailedButSoftPassForKeepFare();
    }
  }
}

void
PricingUnitFactory::copyPUTemplateInfo(PUPQItem& pupqItem)
{
  PricingUnit& prU = *pupqItem.pricingUnit();
  pupqItem.pu() = _pu;
  prU.paxType() = _paxType;
  prU.puType() = _pu->puType();
  prU.puSubType() = _pu->puSubType();
  prU.geoTravelType() = _pu->geoTravelType();
  prU.sameNationOJ() = _pu->sameNationOJ();
  prU.ojSurfaceStatus() = _pu->ojSurfaceStatus();
  prU.turnAroundPoint() = _pu->turnAroundPoint();
  prU.noPUToEOE() = _pu->noPUToEOE();
  prU.hasSideTrip() = _pu->hasSideTrip();
  prU.itinWithinScandinavia() = _pu->itinWithinScandinavia();
  prU.setFlexFaresGroupId(_pu->getFlexFaresGroupId());
  if (!_pu->intlOJToOW().empty())
  {
    prU.intlOJToOW() = _pu->intlOJToOW();
  }
}

void
PricingUnitFactory::setPriority(const MergedFareMarket& mfm,
                                PUPQItem& pupqItem,
                                const PaxTypeFare& paxTypeFare,
                                const Directionality puDir,
                                bool cxrFarePhase,
                                bool& lowerPriorityFare)
{
  PriorityStatus& status = pupqItem.mutablePriorityStatus();

  bool unpreferedYY = mfm.cxrFarePreferred() && paxTypeFare.carrier() == INDUSTRY_CARRIER;
  if (unpreferedYY)
  {
    if (!cxrFarePhase)
    {
      std::vector<CarrierCode> puValCxr;
      getPUValCxr(pupqItem, puValCxr);
      if (PricingUtil::cxrFareTypeExists(
              mfm, paxTypeFare, _pu->puType(), puDir, _paxType, puValCxr))
      {
        // We are setting it to lower priority if any ValCxr has valid Cxr-Fare
        // Later in FarePathFactory we need to recalculate again for the whole FP

        lowerPriorityFare = true;
        status.setFarePriority(PRIORITY_LOW); // lower priority
      }
    }
  }

  if (_paxType->paxType() != ADULT && _paxType->paxType() != paxTypeFare.fcasPaxType())
  {
    status.setPaxTypeFarePriority(PRIORITY_LOW); // lowering priority
  }

  // Set the FareByRule priority
  //
  const FBR_PRIORITY fbrPriority = status.fareByRulePriority();
  if (paxTypeFare.isFareByRule())
  {
    if (fbrPriority == FBR_PRIORITY_PUB_OR_CARRIER_ONLY)
    {
      status.setFareByRulePriority(FBR_PRIORITY_MIXED);
    }
    else if (fbrPriority == FBR_PRIORITY_DEFAULT)
    {
      status.setFareByRulePriority(FBR_PRIORITY_FBR_ONLY);
    }
  }
  else
  {
    if (fbrPriority == FBR_PRIORITY_FBR_ONLY)
    {
      status.setFareByRulePriority(FBR_PRIORITY_MIXED);
    }
    else if (fbrPriority == FBR_PRIORITY_DEFAULT)
    {
      status.setFareByRulePriority(FBR_PRIORITY_PUB_OR_CARRIER_ONLY);
    }
  }

  if (paxTypeFare.carrier() == INDUSTRY_CARRIER)
  {
    status.setFareCxrTypePriority(PRIORITY_LOW);
  }

  status.setPtfRank(status.ptfRank() + paxTypeFare.paxType()->paxTypeInfo().paxTypeStatus());

  if (UNLIKELY(_trx->getOptions()->isZeroFareLogic()))
    pupqItem.updateCabinPriority(paxTypeFare.cabin());
}

bool
PricingUnitFactory::getFareMktPaxTypeBucket(MergedFareMarket& mfm,
                                            const uint16_t fareIdx,
                                            uint16_t& relativeIdx,
                                            FareMarket*& fm,
                                            PaxTypeBucket*& paxTypeCortege)
{
  uint16_t paxFareSZ = 0;
  bool found = false;
  std::vector<FareMarket*>::iterator it = mfm.mergedFareMarket().begin();
  const std::vector<FareMarket*>::iterator itEnd = mfm.mergedFareMarket().end();
  for (; it != itEnd; ++it)
  {
    fm = *it;
    paxTypeCortege = fm->paxTypeCortege(_paxType);
    if (UNLIKELY(paxTypeCortege == nullptr))
    {
      continue;
    }

    std::vector<PaxTypeFare*>& paxTypeFareVect = paxTypeCortege->paxTypeFare();
    paxFareSZ += paxTypeFareVect.size();
    if (fareIdx < paxFareSZ)
    {
      found = true;
      break;
    }
    else
    {
      relativeIdx = fareIdx - paxFareSZ;
    }
  }
  if (!found)
  {
    paxTypeCortege = nullptr;
    fm = nullptr;
    return false;
  }

  return true;
}

bool
PricingUnitFactory::checkRec2Cat10Indicator(const uint16_t mktIdx,
                                            const PaxTypeFare& ptFare,
                                            std::string& failReason) const
{
  const PaxTypeFare& paxTypeFare = *PricingUtil::determinePaxTypeFare(&ptFare);

  const CombinabilityRuleInfo* pCat10 = paxTypeFare.rec2Cat10();
  if (pCat10 == nullptr)
    return true;

  if (pCat10->applInd() == 'X' &&
      (!_pu->noPUToEOE() || _pu->puType() != PricingUnit::Type::ONEWAY)) // NOT_APPLICABLE
  {
    failReason = "FAILED COMBINATION - RECORD 2 CAT 10 NOT APPLICABLE\n";
    return false;
  }

  if (_pu->noPUToEOE())
  {
    // there is only one PU in the MainTrip
    // 'M' for ATPCO, 'D' for SITA
    if (pCat10->eoeInd() == 'M' || pCat10->eoeInd() == 'D')
    {
      failReason = "REC2 SCORBOARD CHECK: EOE REQUIRED\n";
      return false;
    }
  }

  else if (!_pu->possibleSideTripPU())
  {
    // for possible ST-PU we need to wait for the FarePath to know if
    // the pu is in MT or ST

    // more than one PU needed to build MainTrip part of the FaraPath
    // 'N' for ATPCO, 'B' for SITA
    if (pCat10->eoeInd() == 'N' || pCat10->eoeInd() == 'B')
    {
      failReason = "REC2 SCORBOARD CHECK: EOE NOT PERMITTED\n";
      return false;
    }
  }

  if (_pu->puType() == PricingUnit::Type::CIRCLETRIP)
  {
    return checkCTRec2Cat10Indicator(*pCat10, paxTypeFare, failReason);
  }
  else if (_pu->puType() == PricingUnit::Type::OPENJAW)
  {
    return checkOJRec2Cat10Indicator(mktIdx, *pCat10, paxTypeFare, failReason);
  }
  else if (_pu->puType() == PricingUnit::Type::ROUNDTRIP)
  {
    return checkRTRec2Cat10Indicator(*pCat10, paxTypeFare, failReason);
  }

  return true;
}

bool
PricingUnitFactory::checkCTRec2Cat10Indicator(const CombinabilityRuleInfo& rec2Cat10,
                                              const PaxTypeFare& paxTypeFare,
                                              std::string& failReason) const
{
  if (rec2Cat10.ct2plusInd() == 'N')
  {
    failReason = "REC2 SCORBOARD CHECK: NOT VALID FOR CT\n";
    return false;
  }

  if (_pu->puGovCarrier() == PU::MULTI_CARRIER &&
      rec2Cat10.ct2plusSameCarrierInd() == Combinations::SAME_CARRIER &&
      paxTypeFare.carrier() != INDUSTRY_CARRIER)
  {
    failReason = "REC2 SCORBOARD CHECK: NOT SAME CXR CT-PU\n";
    return false;
  }

  return true;
}

bool
PricingUnitFactory::checkRTRec2Cat10Indicator(const CombinabilityRuleInfo& rec2Cat10,
                                              const PaxTypeFare& paxTypeFare,
                                              std::string& failReason) const
{
  if ((rec2Cat10.ct2Ind() == 'X') ||
      (rec2Cat10.ct2Ind() == 'N' &&
       paxTypeFare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED)) // NOT Tag 2, MirroImage not possible
  {
    failReason = "REC2 SCORBOARD CHECK: NOT VALID FOR RT\n";
    return false;
  }

  if (_pu->puGovCarrier() == PU::MULTI_CARRIER &&
      rec2Cat10.ct2SameCarrierInd() == Combinations::SAME_CARRIER &&
      paxTypeFare.carrier() != INDUSTRY_CARRIER)
  {
    failReason = "REC2 SCORBOARD CHECK: NOT SAME CXR RT-PU\n";
    return false;
  }

  return true;
}

bool
PricingUnitFactory::checkOJRec2Cat10Indicator(const uint16_t mktIdx,
                                              const CombinabilityRuleInfo& rec2Cat10,
                                              const PaxTypeFare& paxTypeFare,
                                              std::string& failReason) const
{
  const DateTime& travelDate = paxTypeFare.fareMarket()->travelDate();
  if (isSamePointApplicable(mktIdx, rec2Cat10, travelDate))
  {
    // Don't fail the fare yet
    failReason = "SAME POINT TABLE APPLICABLE\n";
    return true;
  }

  if (_pu->puGovCarrier() == PU::MULTI_CARRIER &&
      rec2Cat10.dojSameCarrierInd() == Combinations::SAME_CARRIER &&
      paxTypeFare.carrier() != INDUSTRY_CARRIER)
  {
    failReason = "REC2 SCORBOARD CHECK: NOT SAME CXR OJ-PU\n";
    return false;
  }

  if (_pu->puSubType() != PricingUnit::DEST_OPENJAW && rec2Cat10.sojorigIndestInd() == 'D')
  {
    failReason = "REC2 SCORBOARD CHECK: DEST OPEN JAW REQ\n";
    return false;
  }
  else if (_pu->puSubType() != PricingUnit::ORIG_OPENJAW && rec2Cat10.sojorigIndestInd() == 'O')
  {
    failReason = "REC2 SCORBOARD CHECK: ORIGIN OPEN JAW REQ\n";
    return false;
  }

  if (_pu->sameNationOJ())
  {
    if (UNLIKELY(_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW &&
        (rec2Cat10.dojInd() == 'U' || rec2Cat10.dojInd() == 'V')))
    {
      failReason = "REC2 SCORBOARD CHECK: DIFF COUNTRY DOJ REQUIRED\n";
      return false;
    }
    else if (UNLIKELY(_pu->puSubType() != PricingUnit::DOUBLE_OPENJAW &&
             (rec2Cat10.sojInd() == 'U' || rec2Cat10.sojInd() == 'V')))
    {
      failReason = "REC2 SCORBOARD CHECK: DIFF COUNTRY SOJ REQUIRED\n";
      return false;
    }
  }
  else
  {
    if (rec2Cat10.sojInd() == 'S' || rec2Cat10.sojInd() == 'T')
    {
      failReason = "REC2 SCORBOARD CHECK: SAME COUNTRY SOJ REQ\n";
      return false;
    }
    if (_pu->puSubType() == PricingUnit::ORIG_OPENJAW &&
        (rec2Cat10.sojInd() == 'W' || rec2Cat10.sojInd() == 'X'))
    {
      failReason = "REC2 SCORBOARD CHECK: SAME COUNTRY SOJ REQUIRED\n";
      return false;
    }
    else if (_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW &&
             _pu->sameNationOrigSurfaceOJ() == false &&
             (rec2Cat10.dojInd() == 'W' || rec2Cat10.dojInd() == 'X'))
    {
      failReason = "REC2 SCORBOARD CHECK: SAME COUNTRY DOJ REQUIRED\n";
      return false;
    }
  }

  if (_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW)
  {
    if (rec2Cat10.dojInd() == 'N')
    {
      failReason = "REC2 SCORBOARD CHECK: NOT VALID FOR DOJ\n";
      return false;
    }
  }
  else
  {
    if (rec2Cat10.sojInd() == 'N')
    {
      failReason = "REC2 SCORBOARD CHECK: NOT VALID FOR SOJ\n";
      return false;
    }
  }

  return true;
}

///----------------------------------------------------------------------------
bool
PricingUnitFactory::isSamePointApplicable(const uint16_t mktIdx,
                                          const CombinabilityRuleInfo& rec2Cat10,
                                          const DateTime& travelDate) const
{
  if (rec2Cat10.samepointstblItemNo() == 0)
  {
    return false;
  }

  if (_pu->puSubType() == PricingUnit::ORIG_OPENJAW ||
      _pu->puSubType() == PricingUnit::DOUBLE_OPENJAW)
  {
    if (mktIdx == 0 || mktIdx == (_pu->fcCount() - 1))
    {
      const LocCode& loc1 = _pu->fareMarket().front()->boardMultiCity();
      const LocCode& loc2 = _pu->fareMarket().back()->offMultiCity();
      if (RuleUtil::validateSamePoint(*_trx,
                                      rec2Cat10.vendorCode(),
                                      rec2Cat10.samepointstblItemNo(),
                                      loc1,
                                      loc2,
                                      travelDate))
        return true;
    }
  }

  if (_pu->puSubType() == PricingUnit::DEST_OPENJAW ||
      _pu->puSubType() == PricingUnit::DOUBLE_OPENJAW)
  {
    const uint16_t leg1DestFMIdx = _pu->ojLeg1FCCount() - 1;
    const uint16_t leg2OrigFMIdx = leg1DestFMIdx + 1;
    if (mktIdx == leg1DestFMIdx || mktIdx == leg2OrigFMIdx)
    {
      const LocCode& loc1 = _pu->fareMarket()[leg1DestFMIdx]->offMultiCity();
      const LocCode& loc2 = _pu->fareMarket()[leg2OrigFMIdx]->boardMultiCity();
      if (RuleUtil::validateSamePoint(*_trx,
                                      rec2Cat10.vendorCode(),
                                      rec2Cat10.samepointstblItemNo(),
                                      loc1,
                                      loc2,
                                      travelDate))
        return true;
    }
  }

  return false;
}

/*----------------------------------------------------------------------------
 * Validate if all fares in PricingUnit belongs to proper, just processed
 * fare type group. This group has to be present in WQCC Fare Type Designation
 * Matrix (_noPNROptions).
 *----------------------------------------------------------------------------*/
// bool PricingUnitFactory::isValidFareTypeGroup(const PricingUnit& pu, const PaxTypeFare& ptf)
bool
PricingUnitFactory::isValidFareTypeGroup(const PaxTypeFare& ptf) const
{
  bool isValid = false;

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(_trx);

  if (LIKELY(!noPNRTrx || !noPNRTrx->isNoMatch()))
    isValid = true;
  else
  {
    NoPNRPricingTrx::FareTypes::FTGroup fareTypeGroup =
        noPNRTrx->fareTypes().getFareTypeGroup(ptf.fcaFareType());
    if (fareTypeGroup != NoPNRPricingTrx::FareTypes::FTG_NONE &&
        noPNRTrx->processedFTGroup() == fareTypeGroup)
      isValid = true;
  }

  return isValid;
}

/*----------------------------------------------------------------------------
 * For Domestic-CT-PU  both  NL & SP are allowed
 * For International-CT-PU
 *     If FC count = 2 both NL & SP  are allowed
 *     IF FC count > 2 only NL, unless AA exception applies
 *
 * NOTE: ForeignDomestic will not be considered as Intl here
 *
 *----------------------------------------------------------------------------*/
bool
PricingUnitFactory::checkCTFareRestriction(const PricingUnit& pu)
{
  if (pu.puFareType() != PricingUnit::SP || pu.geoTravelType() != GeoTravelType::International)
    return true;

  const CarrierCode& firstPTFCarrier = pu.fareUsage().front()->paxTypeFare()->carrier();
  // if the first carrier is not AA we can early check for return
  // without iterating thru all fare usages
  if (firstPTFCarrier != SPECIAL_CARRIER_AA)
  {
    if (_pu->fcCount() > _maxSPCTFareCompCount || _spCTPUTried >= _maxSPCTPUCount)
      return false;
  }
  ++_spCTPUTried;
  return true;
}

bool
PricingUnitFactory::checkCurrency(const PaxTypeFare& paxTypeFare,
                                  const PaxTypeBucket& paxTypeCortege,
                                  const FareMarket& fm,
                                  const Directionality dir,
                                  bool isMultiCurrency) const
{
  if (UNLIKELY(_altCurrencyRequest))
  {
    if (fm.geoTravelType() == GeoTravelType::Transborder && paxTypeFare.directionality() == BOTH &&
        paxTypeCortege.isMarketCurrencyPresent())
    {
      if (dir == FROM)
        return (_altCurrencyCode == paxTypeFare.currency() &&
                _altCurrencyCode == paxTypeCortege.outboundCurrency());
      else if (dir == TO)
        return (_altCurrencyCode == paxTypeFare.currency() &&
                _altCurrencyCode == paxTypeCortege.inboundCurrency());
    }
    else
      return (_altCurrencyCode == paxTypeFare.currency());
  }

  if (isMultiCurrency)
    return checkMultiCurrency(paxTypeFare, paxTypeCortege, fm, dir);

  return checkSingleCurrency(paxTypeFare, paxTypeCortege, fm, dir);
}

bool
PricingUnitFactory::checkMultiCurrency(const PaxTypeFare& paxTypeFare,
                                       const PaxTypeBucket& paxTypeCortege,
                                       const FareMarket& fm,
                                       const Directionality dir) const
{
  // Only do this for WPD*XXX entries or Transborder entries
  // Otherwise skip these checks.

  if (LIKELY((_itin->geoTravelType() != GeoTravelType::Transborder)))
    return true;

  if (dir == FROM)
  {
    return checkNonDirectionalFare(
        paxTypeFare, paxTypeCortege, paxTypeCortege.outboundCurrency(), fm.geoTravelType());
  }
  else if (dir == TO)
  {
    return checkNonDirectionalFare(
        paxTypeFare, paxTypeCortege, paxTypeCortege.inboundCurrency(), fm.geoTravelType());
  }

  return false;
}

bool
PricingUnitFactory::checkNonDirectionalFare(const PaxTypeFare& paxTypeFare,
                                            const PaxTypeBucket& paxTypeCortege,
                                            const CurrencyCode& currency,
                                            GeoTravelType geoTravleType) const
{
  if (paxTypeFare.directionality() == BOTH && geoTravleType == GeoTravelType::Transborder)
  {
    if (paxTypeCortege.isMarketCurrencyPresent())
      return (paxTypeFare.currency() == currency);
    return true;
  }
  else
  {
    return (paxTypeFare.currency() == currency);
  }
}

bool
PricingUnitFactory::checkSingleCurrency(const PaxTypeFare& paxTypeFare,
                                        const PaxTypeBucket& paxTypeCortege,
                                        const FareMarket& fm,
                                        const Directionality dir) const
{
  // If the Journey is US/Canada, then perform the PaxType checks.
  // Otherwise skip these checks.
  //
  if (_itin->geoTravelType() != GeoTravelType::Transborder)
  {
    // No requirement is known for currency bypass of Alpha+Alpha+Numeric

    // PaxType (GV1, TV1, etc.), Should it be Alpha+Numeric+Numeric?
    // Now added one below for CNN (C10).  - Mohammad 5/17/07

    // If requested PAX type is Alpha+Alpha+Numeric , don't check currency
    PaxTypeCode reqPaxType = _paxType->paxType();
    if (LIKELY(reqPaxType.size() == 3))
    {
      if (UNLIKELY(isalpha(reqPaxType[0]) && isalpha(reqPaxType[1]) && isDigit(reqPaxType[2])))
      {
        return true;
      }
    }

    if (_trx->paxType().size() > 1 &&
        (_trx->paxType()[0]->paxType() == ADULT || _trx->paxType()[0]->paxType() == MIL ||
         _trx->paxType()[0]->paxType() == SRC))
    {
      // If primary PaxType is ADT/MIL/SRC then do not check currency for
      // Alpha+Numeric+Numeric (C10), CNN or INF PaxType
      // GroupFarePathFactory will make sure that all the PaxTypes are priced with
      // same currency

      if (LIKELY(reqPaxType.size() == 3))
      {
        if (UNLIKELY(isalpha(reqPaxType[0]) && isDigit(reqPaxType[1]) && isDigit(reqPaxType[2])))
        {
          return true;
        }
      }

      // If requested PAX type is INS/INF/CNN/SNN, don't check currency
      if (reqPaxType == CHILD || reqPaxType == INFANT || reqPaxType == INS || reqPaxType == SNN)
      {
        return true;
      }
    }
  }

  if (dir == FROM)
  {
    return (
        checkNonDirectionalFare(
            paxTypeFare, paxTypeCortege, paxTypeCortege.outboundCurrency(), fm.geoTravelType()) ||
        checkAseanCurrencies(fm.outBoundAseanCurrencies(), paxTypeFare.currency()));
  }

  if (LIKELY(dir == TO))
  {
    return (
        checkNonDirectionalFare(
            paxTypeFare, paxTypeCortege, paxTypeCortege.inboundCurrency(), fm.geoTravelType()) ||
        checkAseanCurrencies(fm.inBoundAseanCurrencies(), paxTypeFare.currency()));
  }
  return false;
}

bool
PricingUnitFactory::checkAseanCurrencies(const std::vector<CurrencyCode>& aseanCurrencies,
                                         const CurrencyCode& currency) const
{
  return std::find(aseanCurrencies.begin(), aseanCurrencies.end(), currency) !=
         aseanCurrencies.end();
}

bool
PricingUnitFactory::hasBrandParity(const PricingUnit& pricingUnit) const
{
  return !(PricingUtil::getCommonBrandsFromPU(*_trx, &pricingUnit).empty());
}

//----------------------------------------------------------------------------
bool
PricingUnitFactory::performPULevelCombinabilityAndRuleValidation(PUPQItem& pupqItem,
                                                                 bool allowDelayedValidation,
                                                                 DiagCollector& diag)
{
  bool valid = true;
  PricingUnit& prU = *pupqItem.pricingUnit();

  const bool validatePU = continuePUScopeValidation(pupqItem);
  if (validatePU || (!allowDelayedValidation))
  {
    valid = checkPULevelCombinability(prU, diag);

    if (valid)
    {
      pupqItem.cat10Status() = PUPQItem::PUValidationStatus::PASSED;
      //----------------- Cat25 Same Rule/Tariff check ----------------
      valid = FareByRuleValidator::checkSameTariffRule(prU);

      if (UNLIKELY(!valid && diag.isActive()))
        diag << "FBR-TARIFF:F\n";
    }
    else
    {
      pupqItem.cat10Status() = PUPQItem::PUValidationStatus::FAILED;
    }

    if (valid && !fallback::fallbackFRRProcessingRetailerCode(_trx))
    {
      valid = RuleUtil::isPricingUnitValidForRetailerCode(prU);
      if (UNLIKELY(!valid && diag.isActive()))
        diag << "FAILED : PU LEVEL SPCC MATCH\n";
    }

    if (valid)
    {
      valid = performPULevelRuleValidation(prU, diag);
      diag.enable(&prU, Diagnostic605);

      if (valid && !prU.isCmdPricing())
      {
        //----------------- Negotiated Fare Combination check ----------------
        NegotiatedFareCombinationValidator combinationValidator(*_trx);

        valid = combinationValidator.validate(prU);

        if (UNLIKELY(!valid && diag.isActive()))
          diag << "FAILED " << combinationValidator.getWarningMessage() << "\n";
      }

      if (valid)
      {
        pupqItem.ruleReValStatus() = PUPQItem::PUValidationStatus::PASSED;
        if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
          diag << " PASSED PU-SCOPE RULE RE-VALIDATION\n";
      }
      else
      {
        pupqItem.ruleReValStatus() = PUPQItem::PUValidationStatus::FAILED;
        if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
          diag << " FAILED PU-SCOPE RULE RE-VALIDATION\n";
      }
    }
  }
  else
  {
    pupqItem.ruleReValStatus() = PUPQItem::PUValidationStatus::UNKNOWN;
    if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
    {
      diag << "DELAY PU VALIDATION" << std::endl;
    }
  }

  return valid;
}

//----------------------------------------------------------------------------
bool
PricingUnitFactory::checkPULevelCombinability(PricingUnit& prU, DiagCollector& diag)
{
  FareUsage* failedFareUsage;
  FareUsage* failedTargetFareUsage;

  bool ret = (_combinations->process(prU, failedFareUsage, failedTargetFareUsage, diag, _itin) ==
              CVR_PASSED);

  if (!ret)
  {
    // set the command pricing failed flag
    if (UNLIKELY(prU.isCmdPricing()))
    {
      prU.setCmdPrcFailedFlag(RuleConst::COMBINABILITY_RULE);
      if (diag.isActive())
        diag << " -- PU COMBINABILITY PASS BY COMMAND PRICING" << std::endl;
      ret = true;

      bool brandedFailedNormalFare = _trx->getRequest()->originBasedRTPricing() &&
                                     failedFareUsage && failedTargetFareUsage &&
                                     !failedFareUsage->paxTypeFare()->isDummyFare() &&
                                     !failedTargetFareUsage->paxTypeFare()->isDummyFare();
      if (brandedFailedNormalFare)
      {
        ret = false;
        if (diag.isActive())
          diag << " -- PU COMBINABILITY FAIL BY BRANDED - NORMAL FARES " << std::endl;
      }
    }
    else
    {
      if (prU.puType() == _pu->puType())
      {
        saveCat10FailedFare(failedFareUsage, failedTargetFareUsage);
      }
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
void
PricingUnitFactory::saveCat10FailedFare(const FareUsage* fareUsage1, const FareUsage* fareUsage2)
{
  if ((fareUsage1 != nullptr) && (fareUsage2 == nullptr) && (_pu->fcCount() > 1))
  {
    _failedFareSet.insert(fareUsage1->paxTypeFare());
  }
  else if ((_pu->fcCount() > 2) && (fareUsage1 != nullptr) && (fareUsage2 != nullptr))
  {
    const PaxTypeFare* paxTypeFare1 = fareUsage1->paxTypeFare();
    const PaxTypeFare* paxTypeFare2 = fareUsage2->paxTypeFare();

    std::map<const PaxTypeFare*, std::set<const PaxTypeFare*> >::iterator it =
        _cat10FailedFare.find(paxTypeFare1);
    if (it != _cat10FailedFare.end())
    {
      it->second.insert(paxTypeFare2);
    }
    else
    {
      std::set<const PaxTypeFare*> newSet;
      newSet.insert(paxTypeFare2);
      typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*> > CAT10RES_MAP;
      _cat10FailedFare.insert(CAT10RES_MAP::value_type(paxTypeFare1, newSet));
    }
  }
}

//----------------------------------------------------------------------------
bool
PricingUnitFactory::checkPrevValidationResult(PricingUnit& prU, DiagCollector& diag) const
{
  if (_pu->fcCount() < 3)
    return true;

  if (_cat10FailedFare.empty())
    return true;

  std::vector<FareUsage*>::iterator it = prU.fareUsage().begin();
  const std::vector<FareUsage*>::iterator itEnd = prU.fareUsage().end();
  const std::vector<FareUsage*>::iterator itLast = prU.fareUsage().end() - 1;

  for (; it != itLast; ++it)
  {
    typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*> > CAT10RES_MAP;
    const PaxTypeFare* paxTypeFare1 = (*it)->paxTypeFare();
    CAT10RES_MAP::const_iterator fSetIt = _cat10FailedFare.find(paxTypeFare1);
    if (fSetIt == _cat10FailedFare.end())
    {
      continue;
    }
    const std::set<const PaxTypeFare*>& failedSet = fSetIt->second;
    std::vector<FareUsage*>::iterator nextIt = it;
    for (++nextIt; nextIt != itEnd; ++nextIt)
    {
      const PaxTypeFare* paxTypeFare2 = (*nextIt)->paxTypeFare();
      if (failedSet.count(paxTypeFare2) != 0)
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << prU << std::endl;
          diag << "INVALID BY PREV PU-CAT10 VALIDATION\n";
        }
        return false;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool
PricingUnitFactory::hasFailedFare(const PUPQItem& pupqItem, bool hasPrevFailedFareUsage,
                                  uint16_t mktIdx, const uint16_t xPoint) const
{
  if(_isISPhase)
     return false;

  // check for single failed-fare from rule/cat-10 validation

  if( !fallback::fallbackPrevFailedFareTuning(_trx) &&
      hasPrevFailedFareUsage &&  mktIdx < xPoint  )
  {
    return true;
  }

  // check below for failed-fare-pair of Cat-10 validation

  if (_pu->fcCount() < 3)
    return false;

  if (_cat10FailedFare.empty())
    return false;

  const PricingUnit& prU = *pupqItem.pricingUnit();

  std::vector<FareUsage*>::const_iterator it = prU.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator itEnd = prU.fareUsage().end();
  const std::vector<FareUsage*>::const_iterator itLast = prU.fareUsage().end() - 1;

  mktIdx = 0;
  for (; it != itLast && mktIdx < xPoint && it != itEnd; ++it, ++mktIdx)
  {
    typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*> > CAT10RES_MAP;
    const PaxTypeFare* paxTypeFare1 = (*it)->paxTypeFare();
    CAT10RES_MAP::const_iterator fSetIt = _cat10FailedFare.find(paxTypeFare1);
    if (fSetIt == _cat10FailedFare.end())
    {
      continue;
    }
    uint16_t nextMktIdx = mktIdx+1;
    const std::set<const PaxTypeFare*>& failedSet = fSetIt->second;
    std::vector<FareUsage*>::const_iterator nextIt = it;
    for (++nextIt; nextIt != itEnd && nextMktIdx < xPoint; ++nextIt, ++nextMktIdx)
    {
      const PaxTypeFare* paxTypeFare2 = (*nextIt)->paxTypeFare();
      if (failedSet.count(paxTypeFare2) != 0)
      {
        return true;
      }
    }
  }

  return false;
}

//---------------------------------------------------------------------------
//
//
//---------------------------------------------------------------------------
bool
PricingUnitFactory::checkOJSurfaceRestriction(PricingUnit& prU, DiagCollector& diag)
{
  const PricingUnit::PUFareType puFareType = prU.puFareType();
  const bool doubleOJ = (prU.puSubType() == PricingUnit::DOUBLE_OPENJAW);

  if (doubleOJ || prU.puSubType() == PricingUnit::ORIG_OPENJAW)
  {
    if (doubleOJ)
    {
      if (puFareType == PricingUnit::NL && (!_pu->sameNationOJ()))
      {
        // doubleOJ, check origin surface
        //
        if (_pu->sameNationOrigSurfaceOJ() == false && _pu->allowNOJInZone210() == false)
        {
          // This PU will not go for Combinablity check therefore
          // put in diag now
          diag.enable(_pu, Diagnostic605);
          if (diag.isActive())
          {
            diag << " INVALID PU: NOT SP-PU" << std::endl;
          }
          return false;
        }
      }
    }
    else // ORIG_OPENJAW
    {
      if (puFareType == PricingUnit::NL && (!_pu->sameNationOJ()))
      {
        if (_pu->allowNOJInZone210() == false && _pu->inDiffCntrySameSubareaForOOJ() == false)
        {
          // This PU will not go for Combinablity check therefore
          // put in diag now
          diag.enable(_pu, Diagnostic605);
          if (diag.isActive())
          {
            diag << " INVALID PU: NOT SP-PU" << std::endl;
          }
          return false;
        }
      }
    }
  }

  return true;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::checkMileagePercentage(const PaxTypeFare& paxTypeFare1,
                                           const PaxTypeFare& paxTypeFare2,
                                           DiagCollector& diag)
{
  if (paxTypeFare1.mileageSurchargePctg() != paxTypeFare2.mileageSurchargePctg())
  {
    // mileageSurchargePctg are NOT EQUAL, returning false, convert RT to CT
    // Do not apply IATA exception
    if (diag.isActive())
    {
      diag << " UN-EQUAL MILEAGE SURCHARGE";

      diag << std::endl;
    }

    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::checkCarrierApplySameNUC(const FareMarket& fm1, const FareMarket& fm2)
{
  const CarrierPreference* const cxr1Pref = fm1.governingCarrierPref();
  const CarrierPreference* const cxr2Pref = fm2.governingCarrierPref();

  return ((cxr1Pref && cxr1Pref->applysamenuctort() == 'Y') ||
          (cxr2Pref && cxr2Pref->applysamenuctort() == 'Y'));
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::isPUWithInUSCA(const FareMarket& fm1, const FareMarket& fm2)
{
  return (fm1.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) &&
          fm2.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA));
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::checkFaresCombOfSPAndNL(const PaxTypeFare& paxTypeFare1,
                                            const PaxTypeFare& paxTypeFare2,
                                            DiagCollector& diag)
{
  if ((paxTypeFare1.isSpecial() && paxTypeFare2.isNormal()) ||
      (paxTypeFare1.isNormal() && paxTypeFare2.isSpecial()))
  {
    if (diag.isActive())
    {
      diag << " APPLY CT SAME NUC IATA EXCEPTION - NRML-SPCL OR SPCL-NRML\n";
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::sameNUCAmount(const PaxTypeFare& paxTypeFare1, const PaxTypeFare& paxTypeFare2)
{
  const double diff = (paxTypeFare1.nucFareAmount() - paxTypeFare1.getSoloSurcharges()) -
                      (paxTypeFare2.nucFareAmount() - paxTypeFare2.getSoloSurcharges());

  if ((diff >= 0.0 && diff < EPSILON) || (diff <= 0.0 && -diff < EPSILON))
  {
    return true;
  }

  if (fabs(diff) < 0.02)
  {
    // 1 penny difference, between Outbound and Inbound fare
    if (paxTypeFare1.fareClass() == paxTypeFare2.fareClass() &&
        paxTypeFare1.vendor() == paxTypeFare2.vendor() &&
        paxTypeFare1.tcrRuleTariff() == paxTypeFare2.tcrRuleTariff() &&
        paxTypeFare1.carrier() == paxTypeFare2.carrier() &&
        paxTypeFare1.ruleNumber() == paxTypeFare2.ruleNumber() &&
        paxTypeFare1.footNote1() == paxTypeFare2.footNote1() &&
        paxTypeFare1.footNote2() == paxTypeFare2.footNote2())
    {
      // same fare, keep it RT, don't convert to CT
      return true;
    }
  }
  return false;
}

void
PricingUnitFactory::displayConversionMessage(DiagCollector& diag,
                                             const PricingUnit& prU,
                                             const std::string& itins)
{
  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
  {
    diag << " RT DEFINITION FAILED: CONVERTED RT TO CT";

    if (!itins.empty())
      diag << " FOR ITIN " << itins;

    diag << prU << std::endl;
    diag << std::endl;
  }
}

bool
PricingUnitFactory::processRTCTIATAExceptions(PricingUnit& prU, DiagCollector& diag)
{
  TSELatencyData metrics(*_trx, "PO RT DEF CHK");

  const PaxTypeFare& paxTypeFare1 = *prU.fareUsage().front()->paxTypeFare();
  const PaxTypeFare& paxTypeFare2 = *prU.fareUsage().back()->paxTypeFare();
  bool convertAll = false;
  std::set<uint16_t> changedItins;

  if (!checkRTCTIATAExceptionsCommon(prU, paxTypeFare1, paxTypeFare2, diag))
    convertAll = true;

  if (!convertAll)
    convertAll = !checkMileagePercentage(paxTypeFare1, paxTypeFare2, diag);

  if (convertAll || !changedItins.empty())
  {
    prU.puType() = PricingUnit::Type::CIRCLETRIP;
    displayConversionMessage(diag, prU, DiagnosticUtil::containerToString(changedItins));
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::checkRTCTIATAExceptionsCommon(const PricingUnit& prU,
                                                  const PaxTypeFare& paxTypeFare1,
                                                  const PaxTypeFare& paxTypeFare2,
                                                  DiagCollector& diag)
{
  const FareMarket& fm1 = *paxTypeFare1.fareMarket();
  const FareMarket& fm2 = *paxTypeFare2.fareMarket();

  if (isPUWithInUSCA(fm1, fm2))
    return true;

  if (sameNUCAmount(paxTypeFare1, paxTypeFare2))
    return true;

  bool applySameNUC = false;
  if (checkCarrierApplySameNUC(fm1, fm2))
  {
    applySameNUC = true;
  }
  else
  {
    if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
    {
      diag << " APPLY RT IATA EXCEPTION - CXR-PREF: NOT SAME NUC - RT \n";
    }
    return true;
  }

  const CarrierPreference* const cxr1Pref = fm1.governingCarrierPref();
  const CarrierPreference* const cxr2Pref = fm2.governingCarrierPref();

  if (LIKELY(applySameNUC))
  {
    // Normal + Special or Special + Normal - CT
    if (checkFaresCombOfSPAndNL(paxTypeFare1, paxTypeFare2, diag))
      return false;

    if (cxr1Pref->carrier() != cxr2Pref->carrier()) // Different Carrier - RT
    {
      if (diag.isActive() && !DiagnosticUtil::filter(*_trx, prU))
      {
        diag << " APPLY RT IATA EXCEPTION - CXR-PREF: DIFFERENT CARRIERS \n";
      }
      return true;
    }
    else // Same Carrier
    {
      if (!paxTypeFare1.canTreatAsWithSameCabin(paxTypeFare2)) // different cabin - RT
      {
        if (diag.isActive() && !DiagnosticUtil::filter(*_trx, prU))
        {
          diag << " APPLY RT IATA EXCEPTION - DIFFERENT CABIN \n";
        }
        return true;
      }
      else // Same Sabin
      {
        // Different Rec1 DOWType - RT or Different Cat 3 - RT
        if (isPuRT(paxTypeFare1, paxTypeFare2, diag, RuleConst::DAY_TIME_RULE) ||
            isPuRT(paxTypeFare1, paxTypeFare2, diag, RuleConst::SEASONAL_RULE))
        {
          return true;
        }

        if (diag.isActive() && !DiagnosticUtil::filter(*_trx, prU))
        {
          diag << " APPLY RT IATA EXCEPTION - NO EXCEPTIONS MET - DIFF NUC - CT\n";
        }
        return false;
      }
    }
  }
  return true;
}

bool
PricingUnitFactory::getMileage(const FareMarket& fm,
                               const DateTime& travelDate,
                               uint32_t& miles,
                               DiagCollector& diag)

{
  GlobalDirection gd;
  DataHandle dataHandle(_trx->ticketingDate());

  bool mileageFound = true;
  miles = 0;
  for (TravelSeg* tvlSeg : fm.travelSeg())
  {
    const LocCode& city1 = FareMarketUtil::getBoardMultiCity(fm, *tvlSeg);
    const LocCode& city2 = FareMarketUtil::getOffMultiCity(fm, *tvlSeg);

    getGlobalDirection(travelDate, *tvlSeg, gd);

    uint32_t ppMiles = 0;
    if (!getMileage(city1, city2, gd, travelDate, ppMiles, diag))
    {
      mileageFound = false;
      break;
    }
    miles += ppMiles;
  }

  diag.enable(Diagnostic661);
  if (diag.isActive())
  {
    if (!mileageFound)
      miles = 0; // don't want to display partial mileage of a FM

    diag << "TPM MILEAGE OF " << fm.boardMultiCity() << " - " << fm.offMultiCity() << " - " << miles
         << std::endl;
  }

  return mileageFound;
}

//---------------------------------------------------------------
bool
PricingUnitFactory::getMileage(const LocCode& city1,
                               const LocCode& city2,
                               GlobalDirection& gd,
                               const DateTime& travelDate,
                               uint32_t& miles,
                               DiagCollector& diag)
{
  DataHandle dataHandle(_trx->ticketingDate());
  bool mileageFound = true;

  const Mileage* mileage = dataHandle.getMileage(city1, city2, TPM, gd, travelDate);

  if (mileage)
  {
    miles = mileage->mileage();
  }
  else
  {
    mileage = dataHandle.getMileage(city1, city2, MPM, gd, travelDate);
    if (mileage)
    {
      miles = TseUtil::getTPMFromMPM(mileage->mileage());
    }
    else
    {
      miles = 0;
      mileageFound = false;
      LOG4CXX_WARN(logger, "Both TPM and MPM getMileage failed, will use GCM ");
    }
  }

  diag.enable(Diagnostic661);
  if (diag.isActive())
  {
    diag << "TPM MILEAGE OF " << city1 << " - " << city2 << " - " << miles << std::endl;
  }

  return mileageFound;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::releasePUPQItem(PUPQItem* pupqItem)
{
  if (UNLIKELY(!pupqItem))
    return false;

  PricingUnit* pUnit = pupqItem->pricingUnit();

  for (auto* fareUsage : pUnit->fareUsage())
  {
    if (fareUsage->decrementRefCount(_dataHandle))
    {
      _fuPool.destroy(fareUsage);
    }
  }

  _puPool.destroy(pUnit);
  _pupqPool.destroy(pupqItem);
  return true;
}

PUPQItem*
PricingUnitFactory::constructPUPQItem()
{
  PUPQItem* pupqItem = _pupqPool.construct();
  PricingUnit* prU = _puPool.construct();

  pupqItem->pricingUnit() = prU;
  return pupqItem;
}

FareUsage*
PricingUnitFactory::constructFareUsage()
{
  FareUsage* fu = _fuPool.construct();

  if (UNLIKELY(_trx->getTrxType() == PricingTrx::MIP_TRX && _trx->isFlexFare()))
    fu->setFlexFaresGroupId(_pu->getFlexFaresGroupId());

  return fu;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::stopBuildingPU()
{
  if (_fcCount <= 3)
  {
    return false;
  }

  // need other stat not just elapsed time
  // should consider the time spent in
  // this PU factory, then we need to know relative
  // position of this factory compared to other options

  if (_puCombTried > SHORT_CKT_COMB_COUNT && ((float)_puCount / (float)_puCombTried < 0.5))
  {
    if ((_puCombTried % 20) != 0)
    {
      // allow time() system call 1 in 20 times
      return false;
    }

    if ((time(nullptr) - _stopTime) > _shortCktTimeOut)
    {
      // majority of combination is failing
      LOG4CXX_INFO(logger, "Shutdown PricingUnitFactory");
      _shutdownFactory = true;
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
uint32_t
PricingUnitFactory::puCountDiagParam()
{
  if (LIKELY(_trx->diagnostic().diagnosticType() == DiagnosticNone))
    return 0;

  if (_trx->diagnostic().diagnosticType() == Diagnostic603 ||
      _trx->diagnostic().diagnosticType() == Diagnostic605 ||
      _trx->diagnostic().diagnosticType() == Diagnostic910)
  {
    const std::map<std::string, std::string>& dgParamMap = _trx->diagnostic().diagParamMap();
    const auto it = dgParamMap.find("PU");

    if (it != dgParamMap.end())
      return atoi(it->second.c_str());
  }

  return 0;
}

bool
PricingUnitFactory::isValidCarrierForOpenJawPU(const PU* pu, const PaxTypeFare& paxTypeFare) const
{
  if (pu->invalidateYYForTOJ()) // TOJ btw 2 areas
  {
    if (paxTypeFare.carrier() == INDUSTRY_CARRIER)
      return false;

    if (UNLIKELY(alg::contains(pu->invalidCxrForOJ(), paxTypeFare.carrier())))
      return false;
  }
  return true;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::isValidFareForCxrSpecificOpenJaw(const PaxTypeFare& paxTypeFare) const
{
  if (_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW)
  {
    if (UNLIKELY(_pu->specialEuropeanDoubleOJ()))
    {
      if (paxTypeFare.isNormal())
        return false; // skip normal fares (carrier or YY)
      if (std::find(_pu->invalidCxrForOJ().begin(),
                    _pu->invalidCxrForOJ().end(),
                    paxTypeFare.carrier()) != _pu->invalidCxrForOJ().end())
        return false;
    }
  }
  if (LIKELY(TrxUtil::isSpecialOpenJawActivated(*_trx)))
  {
    if (_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW ||
        _pu->puSubType() == PricingUnit::DEST_OPENJAW ||
        _pu->puSubType() == PricingUnit::ORIG_OPENJAW)
    {
      if (!isValidCarrierForOpenJawPU(_pu, paxTypeFare))
        return false;
    }
  }
  else
  {
    if (_pu->puSubType() == PricingUnit::DOUBLE_OPENJAW ||
        _pu->puSubType() == PricingUnit::DEST_OPENJAW)
    {
      if (!isValidCarrierForOpenJawPU(_pu, paxTypeFare))
        return false;
    }
  }

  if (!TrxUtil::isdiffCntyNMLOpenJawActivated(*_trx))
  {
    if (_pu->puSubType() == PricingUnit::ORIG_OPENJAW && _pu->inDiffCntrySameSubareaForOOJ())
    {
      if (paxTypeFare.isNormal())
      {
          if (paxTypeFare.carrier() == INDUSTRY_CARRIER)
            return false;

          const CarrierPreference* cxrPref = paxTypeFare.fareMarket()->governingCarrierPref();
          if (cxrPref && (cxrPref->applyNormalFareOJInDiffCntrys() != 'Y'))
            return false;
      }
    }
  }
  return true;
}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::isValidForXORequest(const PaxTypeFare& paxTypeFare,
                                        const PaxTypeBucket& paxTypeCortege,
                                        DiagCollector& diag) const
{
  // For fare type pricing, ignore XO qualifier
  if (UNLIKELY(_trx->getOptions()->isFareFamilyType()))
    return true;

  // lint !e578
  bool paxTypeMatch = false;

  if (UNLIKELY(paxTypeCortege.requestedPaxType()->vendorCode() == Vendor::SABRE))
  {
    if (PaxTypeUtil::sabreVendorPaxType(*_trx, *(paxTypeCortege.requestedPaxType()), paxTypeFare))
      paxTypeMatch = true;
  }
  else
  {
    PaxTypeCode farePaxType = paxTypeFare.fcasPaxType();
    if (farePaxType.empty())
      farePaxType = ADULT;

    if (farePaxType == paxTypeCortege.requestedPaxType()->paxType())
      paxTypeMatch = true;
  }

  if (!paxTypeMatch)
  {
    if (LIKELY(!diag.isActive()))
      return false;

    // for diag 601
    const std::map<std::string, std::string>& dgParamMap = _trx->diagnostic().diagParamMap();
    const std::map<std::string, std::string>::const_iterator it = dgParamMap.find("FC");

    if (it == dgParamMap.end())
      return false;

    if (it->second == paxTypeFare.fareClass())
    {
      diag.printLine();
      diag << paxTypeFare << std::endl;
      diag << "THIS PAXTYPE FARE IS INVALID - ";

      if (_trx->getOptions()->forceCorpFares())
      {
        diag << "XC";
      }
      else
      {
        diag << "XO";
      }
      diag << " PARAM - PAX TYPES MUST MATCH\n"
           << "PAXTYPE REQUESTED - " << paxTypeCortege.requestedPaxType()->paxType() << "\n"
           << "PAXTYPE IN FARE - ";
      if (paxTypeFare.fcasPaxType().empty())
        diag << "*** \n";
      else
        diag << paxTypeFare.fcasPaxType() << std::endl;
    }
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------

bool
PricingUnitFactory::performPULevelRuleValidation(PricingUnit& prU, DiagCollector& diag)
{
  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic555) &&
      !isDiagForMaxPenalty(_trx->diagnostic()))
  {
    diag.enable(Diagnostic555);
    diag.printLine();
    if (!DiagnosticUtil::filter(*_trx, prU))
    {
      diag << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
      diag << " " << std::endl;
      diag << "PRICING UNIT " << prU;
      diag << " " << std::endl;
    }
    diag.flushMsg();
  }

  FareUsage* failedFareUsage = nullptr;
  bool valid = false;

  if (_isISPhase)
  {
    ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(*_trx);
    if (shoppingTrx.isAltDates())
    {
      RuleControllerWithChancelor<PricingUnitRuleController> ruleController(PURuleValidationISALT);
      valid = ruleController.validate(*_trx, prU, failedFareUsage, *_itin);
    }
    else
    {
      RuleControllerWithChancelor<PricingUnitRuleController> ruleController(PURuleValidationIS);
      valid = ruleController.validate(*_trx, prU, failedFareUsage, *_itin);
    }
  }
  else
  {
    if (!isDiagForMaxPenalty(_trx->diagnostic()))
    {
      diag.enable(Diagnostic555);
      diag.printLine();
      diag << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
      diag << " " << std::endl;
      diag << "PRICING UNIT " << prU;
      diag << " " << std::endl;
    }
    bool ignoreRuleForKeepFare =
        (_trx->excTrxType() == PricingTrx::AR_EXC_TRX &&
         (static_cast<RexPricingTrx*>(_trx))->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE &&
         prU.hasKeepFare());

    RuleControllerWithChancelor<PricingUnitRuleController> ruleController(PURuleValidation, _trx);
    if (UNLIKELY(TrxUtil::reuseFarePUResult()))
      ruleController.setFareResultContainer(_farePUResultContainer);

    valid = ruleController.validate(*_trx, prU, failedFareUsage, *_itin);

    if (UNLIKELY(ignoreRuleForKeepFare && !valid)) // check new result
    {
      if (prU.ruleFailedButSoftPassForKeepFare()) // PU fails only on keep fares
      {
        failedFareUsage = nullptr;
        valid = true;
      }
    }
  }

  if (failedFareUsage != nullptr && _pu->fcCount() > 1)
  {
    _failedFareSet.insert(failedFareUsage->paxTypeFare());
  }

  return valid;
}

void
PricingUnitFactory::setPUScopeCat10Status(const uint32_t idx, PUPQItem::PUValidationStatus status)
{
  if (idx < _puCount)
  {
    _validPUPQItem[idx]->cat10Status() = status;
  }
}

//----------------------------------------------------------------------------
PUPQItem::PUValidationStatus
PricingUnitFactory::getPUScopeCat10Status(const uint32_t idx)
{
  if (idx < _puCount)
  {
    return _validPUPQItem[idx]->cat10Status();
  }

  return PUPQItem::PUValidationStatus::UNKNOWN;
}

//----------------------------------------------------------------------------

void
PricingUnitFactory::setPUScopeRuleReValStatus(const uint32_t idx,
                                              PUPQItem::PUValidationStatus status)
{
  if (idx < _puCount)
  {
    _validPUPQItem[idx]->ruleReValStatus() = status;
  }
}

//----------------------------------------------------------------------------
PUPQItem::PUValidationStatus
PricingUnitFactory::getPUScopeRuleReValStatus(const uint32_t idx, PricingUnit& pu)
{
  if (idx < _puCount && pu.isPUWithSameFares(*(_validPUPQItem[idx]->pricingUnit())))
  {
    PUPQItem::PUValidationStatus status = _validPUPQItem[idx]->ruleReValStatus();
    if (status == PUPQItem::PUValidationStatus::PASSED)
      copyPUCat5Result(pu, *(_validPUPQItem[idx]->pricingUnit()));

    return status;
  }

  return PUPQItem::PUValidationStatus::UNKNOWN;
}

//----------------------------------------------------------------------------
void
PricingUnitFactory::display601DiagHeader(const MergedFareMarket& mfm,
                                         const uint16_t fareIdx,
                                         DiagCollector& diag)

{
  if((_trx->getTrxType() == PricingTrx::MIP_TRX) && (this->_itin != nullptr))
  {
    if((_trx->diagnostic().diagParamIsSet("ITIN_NUM", "")) ||
       (_trx->diagnostic().diagParamIsSet("ITIN_NUM",  std::to_string(this->_itin->itinNum())))
       )
    {
      diag.printLine();
      diag << "ITIN-NO: " << this->_itin->itinNum() << " \n";
    }
    else
    {
      return;
    }
  }

  diag.printLine();
  diag << mfm << std::endl;
  diag << "REQUESTED PAXTYPE: " << _paxType->paxType() << "     "
       << (_pu->puType() == PricingUnit::Type::OPENJAW
               ? "101/OJ"
               : (_pu->puType() == PricingUnit::Type::ROUNDTRIP
                      ? "102/RT"
                      : (_pu->puType() == PricingUnit::Type::CIRCLETRIP
                             ? "103/CT"
                             : (_pu->puType() == PricingUnit::Type::ONEWAY ? "104/OW" : " "))))
       << "    START-FARE-INDEX: " << (fareIdx + 1) << std::endl;

  if (trx() && trx()->getTrxType() == PricingTrx::MIP_TRX && trx()->isFlexFare())
  {
    diag << "FLEX FARES GROUP ID: " << _pu->getFlexFaresGroupId() << std::endl;
  }

  if (mfm.mergedFareMarket().size() > 0)
  {
    const FareMarket* const firstFm = mfm.mergedFareMarket().front();
    if (firstFm && !firstFm->fareBasisCode().empty())
      diag << "FARE CLASS " << firstFm->fareBasisCode() << " FOR PRICING\n";
  }
}

//----------------------------------------------------------------------------

void
PricingUnitFactory::display601DiagMsg(const PaxTypeFare& paxTypeFare,
                                      const PaxTypeBucket& paxTypeCortege,
                                      const FareMarket& fm,
                                      const uint16_t mktIdx,
                                      const bool valid,
                                      const bool ctNLSPFareRestPass,
                                      const std::string& prevRuleStatus,
                                      DiagCollector& diag) const
{
  if(_trx->getTrxType() == PricingTrx::MIP_TRX &&
     this->_itin != nullptr &&
     !_trx->diagnostic().diagParamIsSet("ITIN_NUM", "") &&
     !_trx->diagnostic().diagParamIsSet("ITIN_NUM", std::to_string(this->_itin->itinNum())))
  {
    return;
  }

  if (DiagnosticUtil::filter(*_trx, paxTypeFare))
    return;

  diag.printLine();
  diag << paxTypeFare << std::endl;

  diag << " OB CURR:";
  diag << paxTypeCortege.outboundCurrency();

  if (!fm.outBoundAseanCurrencies().empty())
  {
    diag << " OB ASEAN: ";
    for (const auto currency : fm.outBoundAseanCurrencies())
      diag << currency << " ";
  }

  diag << " IB CURR:";
  diag << paxTypeCortege.inboundCurrency();

  if (!fm.inBoundAseanCurrencies().empty())
  {
    diag << " IB ASEAN: ";

    for (const auto currencyCode : fm.inBoundAseanCurrencies())
      diag << currencyCode << " ";
  }

  diag << " FARE CURR:" + paxTypeFare.currency();
  if (_trx->getRequest()->getBrandedFareSize() > 1)
  {
    diag << " BRAND IDS: " << AirlineShoppingUtils::getBrandIndexes(*_trx, paxTypeFare);
  }

  diag << std::endl;

  diag << "FARE OWRT VALUE:" << paxTypeFare.owrt()
       << (ctNLSPFareRestPass ? "" : "         CT-NLSP-REST:F") << std::endl;

  if (paxTypeFare.directionality() == FROM)
    diag << "FARE DIR: F" << std::endl;
  else if (paxTypeFare.directionality() == TO)
    diag << "FARE DIR: T" << std::endl;
  else
    diag << "FARE DIR: NONE" << std::endl;

  diag << "FARE CLASS:" << paxTypeFare.fareClass() << " MARKET FARE BASIS:" << fm.fareBasisCode()
       << std::endl;

  const Directionality puDir = _pu->fareDirectionality()[mktIdx];

  if (fm.geoTravelType() == GeoTravelType::Transborder && paxTypeFare.directionality() == BOTH &&
      !paxTypeCortege.isMarketCurrencyPresent() &&
      paxTypeCortege.outboundCurrency() != paxTypeFare.currency())
    diag << "NO " << paxTypeCortege.outboundCurrency() << " FARE ON THE MARKET" << std::endl;

  diag << prevRuleStatus;
  if (valid)
  {
    diag << "VALID FARE PUTYPE: " << _pu->puType() << "  DIR: " << (puDir == 1 ? 'O' : 'I')
         << std::endl;
  }
  else
  {
    diag << "INVALID FARE PUTYPE: " << _pu->puType() << "  DIR: " << (puDir == 1 ? 'O' : 'I')
         << std::endl;
  }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void
PricingUnitFactory::displayPricingUnit(const PricingUnit& prU, DiagCollector& diag) const
{
  if(!fallback::fallbackDisplaySelectiveItinNum(_trx))
  {
      if((_trx->getTrxType() == PricingTrx::MIP_TRX) && (this->_itin != nullptr))
      {
          if((_trx->diagnostic().diagParamIsSet("ITIN_NUM", "")) ||
             (_trx->diagnostic().diagParamIsSet("ITIN_NUM",  std::to_string(this->_itin->itinNum())))
            )
          {
              diag << "ITIN-NO: " << this->_itin->itinNum() << " \n";
          }
          else
          {
              return;
          }
      }
   }

  diag.displayPuItins(prU);

  diag << " REQUESTED PAXTYPE: " << _paxType->paxType();
  if (_paxType->age() > 0)
    diag << " AGE: " << _paxType->age();
  diag << "    AMOUNT: " << prU.getTotalPuNucAmount();
  if (_trx->getRequest() && _trx->getRequest()->isBrandedFaresRequest())
  {
    diag << "    BRAND: " << pu()->fareMarket().front()->brandCode();
  }

  if (_trx && _trx->getRequest() && _trx->getRequest()->getBrandedFareSize() > 1)
  {
    diag << " BRAND IDS: " << AirlineShoppingUtils::getBrandIndexes(*_trx, prU);
  }

  if (_trx && _trx->getTrxType() == PricingTrx::MIP_TRX && _trx->isFlexFare())
  {
    diag << "\n FLEX FARES GROUP ID: " << prU.getFlexFaresGroupId();
  }

  diag << std::endl;
  diag << prU << std::endl;
}

//----------------------------------------------------------------------------
void
PricingUnitFactory::displayPrevFailedFC(const PricingUnit& prU, DiagCollector& diag) const
{
  diag << " PU FAILED BY PREV VALIDATION";
  for (const auto* fareUsage : prU.fareUsage())
  {
    if (_failedFareSet.count(fareUsage->paxTypeFare()) != 0)
    {
      diag << " FC: " << fareUsage->paxTypeFare()->fareClass()
           << " MKT: " << fareUsage->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
           << fareUsage->paxTypeFare()->fareMarket()->offMultiCity();

      break; // displaying the first one only
    }
  }
  diag << "\n";
}

//----------------------------------------------------------------------------
void
PricingUnitFactory::displayPartiallyBuiltFailedPU(const PricingUnit& prU, DiagCollector& diag) const
{
  diag << " REQUESTED PAXTYPE: " << _paxType->paxType();
  if (_paxType->age() > 0)
    diag << " AGE: " << _paxType->age();
  diag << "    AMOUNT: " << prU.getTotalPuNucAmount() << "\n";;

  uint16_t i = 0;
  for(auto fm : _pu->fareMarket())
  {
    ++i;
    diag << " " << _itin->segmentOrder(fm->travelSeg().front()) << "--"
                << _itin->segmentOrder(fm->travelSeg().back()) << ":"
                << fm->boardMultiCity() << " "
                << fm->offMultiCity() << " ";


    if (i % 4 == 0)
      diag << "\n";
  }
  if (i % 4 != 0)
    diag << "\n";

  for (auto fu : prU.fareUsage())
  {
     diag << *fu << "\n";;
  }
  diag << " PARTIALLY BUILT PU \n";

  displayPrevFailedFC(prU, diag);

}

//---------------------------------------------------------------------------
bool
PricingUnitFactory::continuePUScopeValidation(PUPQItem& pupqItem) const
{
  PricingUnit& prU = *pupqItem.pricingUnit();

  if (UNLIKELY(_puScopeValidationEnabled))
    return true;

  if (!_initStage)
    return ((_curCombTried <= MAX_CURR_COMB_VAL_COUNT) || _delta >= prU.getTotalPuNucAmount());

  // In init stage. We always delay validation for altdates, because
  // for 49 (7x7) date-pairs, there could be 49*9=441 Itin and out of 441 we only want to
  // price 49 cheapest ones

  if ((_trx->getTrxType() == PricingTrx::MIP_TRX) &&
      (_delayPUValidationMIP || (_trx->isAltDates() && !_trx->altDatePairs().empty())))
  {
    return false;
  }

  if (_puCombTried > INIT_COMB_VAL_COUNT)
  {
    const MoneyAmount amount = _journeyPULowerBound->journeyPULowerBound();
    if (amount > 0 && prU.getTotalPuNucAmount() > amount)
      return false;
  }

  return (_puCombTried < _puScopeValMaxNum);
}

PUPQItem*
PricingUnitFactory::getNextCxrFarePUPQItem(PUPQItem& prevPUPQItem,
                                           bool isXPoint,
                                           const CarrierCode& valCxr,
                                           DiagCollector& diag)
{
  if (UNLIKELY(_enableCxrFareSearchTuning == false))
  {
    // if we want to fall back to old code
    return getNextCxrFarePUPQItemOld(prevPUPQItem, diag);
  }

  std::string puCxrFareType;
  std::deque<bool> cxrFareRest;
  if (UNLIKELY(!getCxrFareRestriction(prevPUPQItem, cxrFareRest, valCxr, puCxrFareType)))
  {
    return nullptr;
  }

  return getNextCxrFarePUPQItemImpl(prevPUPQItem, isXPoint, puCxrFareType, valCxr, cxrFareRest, diag);

}

PUPQItem*
PricingUnitFactory::getNextCxrFarePUPQItemImpl(PUPQItem& prevPUPQItem,
                                               bool isXPoint,
                                               const std::string& puCxrFareType,
                                               const CarrierCode& valCxr,
                                               const std::deque<bool>& cxrFareRest,
                                               DiagCollector& diag)
{
  CXRFareCombo& cxrFareCombo = _cxrFareComboMap[puCxrFareType];

  const size_t size = cxrFareCombo.validCxrPUPQItem().size();
  if (size > 0)
  {
    size_t cxrFareComboIdx = 0;
    if (prevPUPQItem.cxrFareComboIdx() >= 0)
        cxrFareComboIdx =
            isXPoint ? prevPUPQItem.cxrFareComboIdx() + 1 : prevPUPQItem.cxrFareComboIdx();

    if (size > cxrFareComboIdx)
    {
      PUPQItem* pupqItem = cxrFareCombo.validCxrPUPQItem()[cxrFareComboIdx];
      if(valCxr.empty())
      {
           return pupqItem;
      }
      else
      {
        std::vector<CarrierCode> vcVect;
        getPUValCxr(*pupqItem, vcVect);

        if(std::find(vcVect.begin(), vcVect.end(), valCxr) != vcVect.end())
        {
           return pupqItem;
        }
      }
    }
  }

  if (cxrFareCombo.done())
  {
    return nullptr;
  }

  PUPQ& cxrFarePUPQ = cxrFareCombo.cxrFarePUPQ();
  if (size == 0)
  {
    ArrayVector<uint16_t> fareIndices(_pu->fareMarket().size(), 0);

    // 2nd param initStage is true;
    if (!buildCxrFarePricingUnit(
            cxrFarePUPQ, true, prevPUPQItem, fareIndices, cxrFareRest, 0, diag))
    {
      return nullptr;
    }
  }

  PUPQItem* item = getNextCxrFarePricinUnit(cxrFarePUPQ, cxrFareRest, diag);
  if (item == nullptr)
  {
    cxrFareCombo.done() = true;
    return nullptr;
  }

  if (item && item->priorityStatus().farePriority() > DEFAULT_PRIORITY)
  {
    return nullptr;
  }

  item->cxrFareComboIdx() = size;
  cxrFareCombo.validCxrPUPQItem().push_back(item);

  return item;
}

PUPQItem*
PricingUnitFactory::getSameFareBasisPUPQItem(const PricingUnit& primaryPU,
                                             uint16_t& puIdx,
                                             DiagCollector& diag)
{
  PUPQItem* pupqItem = constructPUPQItem();

  const uint16_t totalMkt = _pu->fareMarket().size();
  TSE_ASSERT(totalMkt == primaryPU.fareUsage().size());
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    const PaxTypeFare* primaryPTF = primaryPU.fareUsage()[mktIdx]->paxTypeFare();
    uint16_t fareIdx = 0;

    const FareType dummyFareType;
    bool fareFound = false;
    if (!buildFareUsage(
            primaryPTF, mktIdx, fareIdx, fareFound, *pupqItem, nullptr, false, dummyFareType, diag))
    {
      // no more valid fare in this mkt
      if (!fareFound)
        diag << " NO MORE VALID FARE FOUND IN THIS MARKET FOR THIS PU" << std::endl;

      releasePUPQItem(pupqItem);
      return nullptr;
    }

  } // end of loop for all the fareMarket

  copyPUTemplateInfo(*pupqItem);

  // Copy validating carriers
  if (_trx->isValidatingCxrGsaApplicable() )
  {
    if (!isValidPUForValidatingCxr(*pupqItem->pricingUnit(), diag) )
    {
      releasePUPQItem(pupqItem);
      return nullptr;
    }
  }

  _validPUPQItem.push_back(pupqItem);
  puIdx = _validPUPQItem.size() - 1;

  return pupqItem;
}

// This method is to optimize the INF fare search of 0 amount that matches
// PrimayPaxType Fare, primaryPaxTypeFare will be 0 for all other paxType
// except INF
bool
PricingUnitFactory::matchPrimaryPaxTypeFareForINF(const PaxTypeFare* primaryPaxTypeFare,
                                                  const PaxTypeFare* paxTypeFare) const
{
  if (primaryPaxTypeFare == nullptr)
  {
    // not INF PaxType
    return true;
  }

  if (paxTypeFare->totalFareAmount() > EPSILON)
  {
    return false;
  }

  if (isFakeFareMarket(primaryPaxTypeFare->fareMarket()) &&
      isFakeFareMarket(paxTypeFare->fareMarket()))
  {
    return true;
  }

  PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

  if (paxTypeFare->isDiscounted())
  {
    paxTypeFareRuleData = paxTypeFare->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
  }
  else if (paxTypeFare->isFareByRule() && !paxTypeFare->isSpecifiedFare())
  {
    paxTypeFareRuleData = paxTypeFare->paxTypeFareRuleData(RuleConst::FARE_BY_RULE);
  }

  if (paxTypeFareRuleData)
  {
    PaxTypeFare* baseFare = paxTypeFareRuleData->baseFare();
    if (baseFare && baseFare == primaryPaxTypeFare)
    {
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
// Check if it's dummy fare market added because of "OriginBasedRT" logic enabled
//----------------------------------------------------------------------------*/

bool
PricingUnitFactory::isFakeFareMarket(const FareMarket* fm) const
{
  if (!_trx->getRequest()->originBasedRTPricing())
    return false;
  if (fm->travelSeg().size() != 1)
    return false;
  const AirSeg* airSeg = fm->travelSeg().front()->toAirSeg();
  return airSeg && airSeg->isFake();
}

PUPQItem*
PricingUnitFactory::getNextCxrFarePUPQItemOld(PUPQItem& pupqItem, DiagCollector& diag)
{
  bool initStage = false;
  ArrayVector<uint16_t> fareIndices;
  std::deque<bool> cxrFareRest;
  if (pupqItem.priorityStatus().farePriority() != DEFAULT_PRIORITY)
  {
    initStage = true;
    while (!_cxrFarePUPQ.empty())
    {
      PUPQItem* item = _cxrFarePUPQ.top();
      _cxrFarePUPQ.pop();
      releasePUPQItem(item);
    }

    if (!getInitCxrFareIndices(pupqItem, fareIndices))
    {
      return nullptr;
    }
  }
  else
  {
    fareIndices = pupqItem.fareIndices();
  }

  std::string tmp;
  if (!getCxrFareRestriction_old(pupqItem, cxrFareRest, tmp))
  {
    return nullptr;
  }

  if (!buildCxrFarePricingUnit(
          _cxrFarePUPQ, initStage, pupqItem, fareIndices, cxrFareRest, 0, diag))
  {
    return nullptr;
  }

  return getNextCxrFarePricinUnit_old(_cxrFarePUPQ, diag);
}

bool
PricingUnitFactory::getInitCxrFareIndices(PUPQItem& pupqItem, ArrayVector<uint16_t>& fareIndices)
{
  std::deque<bool> cxrFareRest;
  bool cxrRestExists = false;

  const uint16_t totalMkt = _pu->fareMarket().size();
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    MergedFareMarket& mfm = *_pu->fareMarket()[mktIdx];

    const PaxTypeFare& paxTypeFare = *pupqItem.pricingUnit()->fareUsage()[mktIdx]->paxTypeFare();

    if (mfm.cxrFarePreferred() && paxTypeFare.carrier() != INDUSTRY_CARRIER)
    {
      fareIndices.push_back(pupqItem.fareIndices()[mktIdx]);
      cxrRestExists = true;
    }
    else if (mfm.cxrFarePreferred() &&
             PricingUtil::cxrFareTypeExists_old(
                 mfm, paxTypeFare, _pu->puType(), _pu->fareDirectionality()[mktIdx], _paxType))
    {
      fareIndices.push_back(pupqItem.fareIndices()[mktIdx]);
      cxrRestExists = true;
    }
    else
    {
      fareIndices.push_back(0);
    }
  }
  return cxrRestExists;
}

bool
PricingUnitFactory::getPUValCxr(PUPQItem& pupqItem, std::vector<CarrierCode>& puValCxr)
{

  if(!pupqItem.pricingUnit()->validatingCarriers().empty())
  {
     puValCxr = pupqItem.pricingUnit()->validatingCarriers();
  }
  else
  {
     for (const FareUsage* fu : pupqItem.pricingUnit()->fareUsage())
     {
       if(fu->paxTypeFare()->validatingCarriers().empty())
       {
         //Non GSA path or single VC Path
         CarrierCode blankCC;
         puValCxr.push_back(blankCC);
         return false;
       }
       if(puValCxr.empty())
       {
         _itin->getValidatingCarriers(*_trx, puValCxr);
       }
       PricingUtil::intersectCarrierList(puValCxr, fu->paxTypeFare()->validatingCarriers());
       if(puValCxr.empty())
       {
         break;
       }
     }
  }
  return true;
}

bool
PricingUnitFactory::getCxrFareRestriction(PUPQItem& pupqItem,
                                          std::deque<bool>& cxrFareRest,
                                          const CarrierCode& valCxr,
                                          std::string& puCxrFareType)
{
  bool cxrRestExists = false;
  puCxrFareType = valCxr;
  puCxrFareType += "|";
  const uint16_t totalMkt = _pu->fareMarket().size();
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    MergedFareMarket& mfm = *_pu->fareMarket()[mktIdx];

    const PaxTypeFare& paxTypeFare = *pupqItem.pricingUnit()->fareUsage()[mktIdx]->paxTypeFare();

    if (mfm.cxrFarePreferred() &&
        PricingUtil::cxrFareTypeExists(
            mfm, paxTypeFare, _pu->puType(), _pu->fareDirectionality()[mktIdx], _paxType, valCxr))
    {
      cxrFareRest.push_back(true);
      puCxrFareType += paxTypeFare.fcaFareType().c_str();
      cxrRestExists = true;
    }
    else
    {
      cxrFareRest.push_back(false);
    }
  }

  if (UNLIKELY(!cxrRestExists))
  {
    LOG4CXX_DEBUG(logger, " WHY PUPQItem had LOW Fare Priotiry?")
    return false;
  }

  return true;
}


bool
PricingUnitFactory::getCxrFareRestriction_old(PUPQItem& pupqItem,
                                          std::deque<bool>& cxrFareRest,
                                          std::string& puCxrFareType)
{
  bool cxrRestExists = false;

  const uint16_t totalMkt = _pu->fareMarket().size();
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    MergedFareMarket& mfm = *_pu->fareMarket()[mktIdx];

    const PaxTypeFare& paxTypeFare = *pupqItem.pricingUnit()->fareUsage()[mktIdx]->paxTypeFare();

    if (mfm.cxrFarePreferred() &&
        PricingUtil::cxrFareTypeExists_old(
            mfm, paxTypeFare, _pu->puType(), _pu->fareDirectionality()[mktIdx], _paxType))
    {
      cxrFareRest.push_back(true);
      puCxrFareType += paxTypeFare.fcaFareType().c_str();
      cxrRestExists = true;
    }
    else
    {
      cxrFareRest.push_back(false);
    }
  }

  if (!cxrRestExists)
  {
    LOG4CXX_DEBUG(logger, " WHY PUPQItem had LOW Fare Priotiry?")
    return false;
  }

  return true;
}

void
PricingUnitFactory::buildNextCxrFarePricingUnitSet(PUPQ& cxrFarePUPQ,
                                                   const PUPQItem& prevPUPQItem,
                                                   const std::deque<bool>& cxrFareRest,
                                                   DiagCollector& diag)
{
  const uint16_t totalMkt = _pu->fareMarket().size();
  const ArrayVector<uint16_t> fareIndices(prevPUPQItem.fareIndices());

  uint16_t xPoint = prevPUPQItem.xPoint();
  for (; xPoint < totalMkt; ++xPoint)
  {
    PricingUtil::checkTrxAborted(*_trx, _maxNbrCombMsgThreshold, _puCombTried, _maxNbrCombMsgSet);

    buildCxrFarePricingUnit(
            cxrFarePUPQ, false, prevPUPQItem, fareIndices, cxrFareRest, xPoint, diag);
  }

  return;
}

bool
PricingUnitFactory::buildCxrFarePricingUnit(PUPQ& cxrFarePUPQ,
                                            const bool initStage,
                                            const PUPQItem& prevPUPQItem,
                                            const ArrayVector<uint16_t>& fareIndices,
                                            const std::deque<bool>& cxrFareRest,
                                            const uint16_t xPoint,
                                            DiagCollector& diag)
{
  PUPQItem* pupqItem = constructPUPQItem();

  bool noGatewayFare = _trx->fxCnException();

  if (_trx->getTrxType() != PricingTrx::IS_TRX)
  {
    FxCnException fxCnEx(*_trx, *_itin);
    noGatewayFare = noGatewayFare && fxCnEx.checkThruFare();
  }

  const uint16_t totalMkt = _pu->fareMarket().size();
  for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
  {
    bool fareFound = false;

    const PaxTypeFare& paxTypeFare =
        *prevPUPQItem.pricingUnit()->fareUsage()[mktIdx]->paxTypeFare();
    const FareType& fareType = paxTypeFare.fcaFareType();
    bool seekCxrFare = (cxrFareRest[mktIdx] && paxTypeFare.carrier() == INDUSTRY_CARRIER);
    uint16_t fareIdx = fareIndices[mktIdx];

    // if this mkt is the expansionPoint, start from the next fare for this mkt
    if (mktIdx == xPoint)
    {
      if (!initStage)
      {
        ++fareIdx;
      }

      if (cxrFareRest[mktIdx])
        seekCxrFare = true;
    }
    else
    {
      // The fare of the current index was used in a previous combination
      // Therefore, NO Fare Component level validation e.g. dir, currency, etc.
      // is needed
      //
      if (!seekCxrFare)
      {
        bool hasPrevFailedFareUsage = false;
        if (usePrevFareUsage(mktIdx, prevPUPQItem, true, *pupqItem, diag, hasPrevFailedFareUsage))
        {
          if(fallback::fallbackPrevFailedFarePairTuning(_trx))
	  {
            if(hasPrevFailedFareUsage && mktIdx < xPoint &&
               !fallback::fallbackPrevFailedFareTuning(_trx))
            {
              releasePUPQItem(pupqItem);
              return true;
            }
	  }
	  else
	  {
	     if(hasFailedFare(*pupqItem, hasPrevFailedFareUsage, mktIdx, xPoint))
	     {
               releasePUPQItem(pupqItem);
               return true;
	     }
	  }
          continue;
        }
      }
    }

    if (!buildFareUsage(nullptr,
                        mktIdx,
                        fareIdx,
                        fareFound,
                        *pupqItem,
                        seekCxrFare ? &paxTypeFare.segmentStatus() : nullptr,
                        true,               //true for CxrFare Phase
                        fareType,
                        diag,
                        noGatewayFare,
                        false))
    {
      // no more valid fare in this mkt
      if (!fareFound)
        diag << " NO MORE VALID FARE FOUND IN THIS MARKET FOR THIS PU" << std::endl;

      releasePUPQItem(pupqItem);
      return false;
    }
  }

  copyPUTemplateInfo(*pupqItem);

  if (RexPricingTrx::isRexTrxAndNewItin(*_trx))
  {
    pupqItem->rebookClassesExists() = pupqItem->pricingUnit()->isRebookedClassesStatus();
    if (!checkRebookedClassesForRex(*pupqItem))
      pupqItem->isValid() = false;
  }

  pupqItem->xPoint() = xPoint; // next pu will expand from the next mkt

  cxrFarePUPQ.push(pupqItem);

  return true;
}

//--------------------------------------------------------------------------------------
PUPQItem*
PricingUnitFactory::getNextCxrFarePricinUnit(PUPQ& cxrFarePUPQ,
                                             const std::deque<bool>& cxrFareRest,
                                             DiagCollector& diag)
{
  if (cxrFarePUPQ.empty())
  {
    LOG4CXX_DEBUG(logger, "buildNextLevelPricinUnit: cxrFarePUPQ Empty");
    return nullptr;
  }

  bool validFound = false;

  PUPQItem* pupqItem = cxrFarePUPQ.top();

  while (!validFound)
  {
    PricingUtil::checkTrxAborted(*_trx, _maxNbrCombMsgThreshold, _puCombTried, _maxNbrCombMsgSet);

    cxrFarePUPQ.pop();

    bool valid = pupqItem->isValid();
    if (!valid)
    {
      diag.enable(pupqItem->pu(), Diagnostic603, Diagnostic605);
      if (diag.isActive() && !DiagnosticUtil::filter(*_trx, *pupqItem->pricingUnit()))
      {
        displayPricingUnit(*pupqItem->pricingUnit(), diag);
        if (diag.diagnosticType() == Diagnostic605)
          displayPrevFailedFC(*pupqItem->pricingUnit(), diag);
        diag.printLine();
      }
    }
    if (valid)
    {
      pupqItem->clearReusedFareUsage();
      valid = isPricingUnitValid(*pupqItem, false, diag);
    }

    buildNextCxrFarePricingUnitSet(cxrFarePUPQ, *pupqItem, cxrFareRest, diag);

    if (valid)
    {
      if (pupqItem->cloneFareUsage(*_trx, _fuPool) == false)
      {
        // failed to clone, system out of memory?
        return nullptr;
      }
      return pupqItem;
    }

    if (!valid)
    {
      releasePUPQItem(pupqItem);
    }

    if (cxrFarePUPQ.empty())
    {
      return nullptr;
    }

    pupqItem = cxrFarePUPQ.top();
  }

  return nullptr;
}

PUPQItem*
PricingUnitFactory::getNextCxrFarePricinUnit_old(PUPQ& cxrFarePUPQ, DiagCollector& diag)
{
  if (cxrFarePUPQ.empty())
  {
    LOG4CXX_DEBUG(logger, "buildNextLevelPricinUnit: cxrFarePUPQ Empty");
    return nullptr;
  }

  bool validFound = false;

  PUPQItem* pupqItem = cxrFarePUPQ.top();

  std::deque<bool> cxrFareRest;
  std::string puCxrFareType;
  getCxrFareRestriction_old(*pupqItem, cxrFareRest, puCxrFareType);

  while (!validFound)
  {
    PricingUtil::checkTrxAborted(*_trx, _maxNbrCombMsgThreshold, _puCombTried, _maxNbrCombMsgSet);

    cxrFarePUPQ.pop();

    // need more testing for Cxr-Fare combo search
    // if(_failedCxrFareIndices.count(std::vector<uint16_t>(pupqItem->fareIndices().begin(),
    // pupqItem->fareIndices().end())) != 0)
    //{
    //  pupqItem->isValid() = false;
    //}

    bool valid = pupqItem->isValid();
    if (!valid)
    {
      diag.enable(pupqItem->pu(), Diagnostic603, Diagnostic605);
      if (diag.isActive() && !DiagnosticUtil::filter(*_trx, *pupqItem->pricingUnit()))
      {
        displayPricingUnit(*pupqItem->pricingUnit(), diag);
        if (diag.diagnosticType() == Diagnostic605)
          displayPrevFailedFC(*pupqItem->pricingUnit(), diag);
        diag.printLine();
      }
    }

    if (valid)
    {
      pupqItem->clearReusedFareUsage();
      valid = isPricingUnitValid(*pupqItem, false, diag);
    }

    buildNextCxrFarePricingUnitSet(cxrFarePUPQ, *pupqItem, cxrFareRest, diag);

    if (valid)
    {
      if (pupqItem->cloneFareUsage(*_trx, _fuPool) == false)
      {
        // failed to clone, system out of memory?
        return nullptr;
      }
      return pupqItem;
    }

    if (!valid)
    {
      releasePUPQItem(pupqItem);
    }

    if (cxrFarePUPQ.empty())
    {
      return nullptr;
    }

    pupqItem = cxrFarePUPQ.top();
  }

  return nullptr;
}

//  This is only for Shopping IS
bool
PricingUnitFactory::checkIS(const PaxTypeFare& paxTypeFare) const
{
  if (_trx->getTrxType() != PricingTrx::IS_TRX)
  {
    return true;
  }

  if (_trx->isAltDates())
  {
    return checkAltDatesIS(paxTypeFare);
  }

  return _failedFareSet.count(&paxTypeFare) == 0;
}

//  This is only for Shopping AltDate IS
bool
PricingUnitFactory::checkAltDatesIS(const PaxTypeFare& paxTypeFare) const
{
  bool failedForDatePair = false;

  ALTDATE_IS_FAIL_FARE_MAP::const_iterator fSetIt = _altDateISFailedFare.find(&paxTypeFare);
  if (UNLIKELY(fSetIt != _altDateISFailedFare.end()))
  {
    failedForDatePair = true;
  }

  bool valid = false;
  for (const auto& altDatePair : _trx->altDatePairs())
  {
    if (UNLIKELY(failedForDatePair))
    {
      if (alg::contains(fSetIt->second, altDatePair.first))
        continue;
    }

    if (altDatePair.second->numOfSolutionNeeded > 0)
    {
      if (paxTypeFare.getAltDatePass(altDatePair.first))
      {
        valid = true;
        break;
      }
    }
  }

  return valid;
}

//  This is only for Shopping IS
bool
PricingUnitFactory::checkAltDatesIS(const PricingUnit& pu) const
{
  if (!_isISPhase)
    return true;

  if (UNLIKELY(_trx->getTrxType() != PricingTrx::IS_TRX))
  {
    return true;
  }

  if (!_trx->isAltDates())
  {
    return true;
  }

  return std::any_of(_trx->altDatePairs().cbegin(),
                     _trx->altDatePairs().cend(),
                     [&pu](const PricingTrx::AltDatePairs::value_type& adItem)
                     {
    return adItem.second->numOfSolutionNeeded > 0 && pu.isADDatePassValidation(adItem.first);
  });
}

//  This is only for Shopping  AltDate IS
void
PricingUnitFactory::saveISFailedFare(const PaxTypeFare* paxTypeFare, const DatePair& datePair)
{
  auto it = _altDateISFailedFare.find(paxTypeFare);
  if (it != _altDateISFailedFare.end())
  {
    it->second.insert(datePair);
  }
  else
  {
    std::set<DatePair> newSet;
    newSet.insert(datePair);
    _altDateISFailedFare.insert(ALTDATE_IS_FAIL_FARE_MAP::value_type(paxTypeFare, newSet));
  }
}

void
PricingUnitFactory::clear()
{
  _getPUPQItemCounter = 0;
  _done = false;
  _puCount = 0;
  _puCombTried = 0;
  _delta = -1;
  _pausedPUPQItem = nullptr;
  _validPUPQItem.clear();
  _failedFareSet.clear();
  _cat10FailedFare.clear();
  if (_farePUResultContainer)
    _farePUResultContainer->clear();
  while (!pqEmpty())
  {
    pqPop();
  }
}

//  This is only for Shopping  AltDate MIP
bool
PricingUnitFactory::checkAltDatesMIPCutOff(const PUPQItem& pupqItem)
{
  if (_trx->getTrxType() != PricingTrx::MIP_TRX)
  {
    return true;
  }

  if (!_trx->isAltDates())
  {
    return true;
  }

  if (_trx->altDateCutOffNucThreshold() > 0 &&
      (pupqItem.pricingUnit()->getTotalPuNucAmount() - _trx->altDateCutOffNucThreshold() > EPSILON))
  {
    _trx->setCutOffReached();
    return false;
  }

  return true;
}

bool
PricingUnitFactory::checkFareTag(const PaxTypeFare& paxTypeFare,
                                 GeoTravelType geoTravelType,
                                 bool fxCnException,
                                 DiagCollector& diag,
                                 std::string& failReason) const
{
  //---------------- Check Fare Tag  -----------------
  if (_pu->puType() == PricingUnit::Type::ONEWAY)
  {
    // OW can have only T-1 or T-3 for both Domestic/Transboarder and Intl/ForeignD
    //
    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED || // Tag 1
        paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) // Tag 3
    {
      return true;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        failReason = "FAILED: FARE OWRT TAG CHECK\n";
      return false;
    }
  }
  else if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
  {
    if (UNLIKELY(fxCnException && paxTypeFare.validForCmdPricing(fxCnException))) // Treat it as domestic
    {
      return true;
    }
    else
    {
      // Intl,ForeignD do not allow T-3 for RT/CT/OJ
      //
      if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED || // Tag 1
          paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) // Tag 2
      {
        return true;
      }
      else
      {
        if (UNLIKELY(diag.isActive()))
          failReason = "FAILED: FARE OWRT TAG CHECK\n";
        return false;
      }
    }
  }

  return true;
}
/*----------------------------------------------------------------------------
 * When IndustryFare has a FareType-Matched Tag-2 CXR-Fare, indicated by
 * IndustryFare::_matchFareTypeOfTag2CxrFare being true, then it can not be
 * used for RT/CT/OJ PU. It can be used for OW PU only
 *----------------------------------------------------------------------------*/

bool
PricingUnitFactory::checkIndustryFareValidity(const PaxTypeFare& paxTypeFare) const
{
  if (_pu->puType() == PricingUnit::Type::ONEWAY)
  {
    return true;
  }

  if (paxTypeFare.fare()->isIndustry())
  {
    const IndustryFare* yyFare = dynamic_cast<const IndustryFare*>(paxTypeFare.fare());
    if (UNLIKELY(yyFare != nullptr && yyFare->matchFareTypeOfTag2CxrFare()))
    {
      return false;
    }
  }

  return true;
}

/*----------------------------------------------------------------------------
 * Japan domestic CNX fare can only be used if journey originated in Japan
 * VTCR for CNX Fares Are:
 *    Vendor: ATP,  Tariff: 302, CXR: Any Rule: 3500
 *----------------------------------------------------------------------------*/
bool
PricingUnitFactory::checkJLCNXFareSelection(const PaxTypeFare& paxTypeFare,
                                            const FareMarket& fareMarket) const
{
  if (fareMarket.geoTravelType() != GeoTravelType::ForeignDomestic || fareMarket.origin()->nation() != JAPAN)
  {
    return true;
  }

  bool cnxFare = false;
  if (paxTypeFare.vendor() == ATPCO_VENDOR_CODE &&
      (paxTypeFare.tcrRuleTariff() == 302 || paxTypeFare.tcrRuleTariff() == 918) &&
      paxTypeFare.ruleNumber() == "3500")
  {
    cnxFare = true;
  }

  if (!cnxFare)
  {
    return true;
  }

  return (_itin->geoTravelType() == GeoTravelType::International &&
          _itin->travelSeg().front()->origin()->nation() == JAPAN);
  // journey originated in Japan
  }

bool
PricingUnitFactory::checkRebookedClassesForRex(PUPQItem& pupqItem) const
{
  return _trx->getRequest()->isLowFareRequested() || !pupqItem.rebookClassesExists();
}

bool
PricingUnitFactory::checkRebookedClassesForRex(const PaxTypeFare& paxTypeFare) const
{
  return _trx->getRequest()->isLowFareRequested() ||
         paxTypeFare.bkgCodeTypeForRex() != PaxTypeFare::BKSS_REBOOKED;
}

void
PricingUnitFactory::clearFactoryForRex(DiagCollector& diag)
{
  if (!_clearingProcessedForRex)
  {
    clear();
    _cxrFareComboMap.clear();
    while (!_cxrFarePUPQ.empty())
      _cxrFarePUPQ.pop();

    initPricingUnitPQ(diag);
    _clearingProcessedForRex = true;
  }
}

void
PricingUnitFactory::copyPUCat5Result(PricingUnit& toPU, const PricingUnit& fromPU)
{
  toPU.latestTktDT() = fromPU.latestTktDT();

  std::vector<FareUsage*>::const_iterator fromFUI = fromPU.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fromFUIEnd = fromPU.fareUsage().end();
  std::vector<FareUsage*>::iterator toFUI = toPU.fareUsage().begin();
  std::vector<FareUsage*>::iterator toFUIEnd = toPU.fareUsage().end();

  for (; toFUI != toFUIEnd, fromFUI != fromFUIEnd; toFUI++, fromFUI++)
  {
    if (!(*fromFUI)->segmentStatus().empty())
    {
      if (UNLIKELY((*fromFUI)->segmentStatus().front()._bkgCodeSegStatus.isSet(
              PaxTypeFare::BKSS_NOT_YET_PROCESSED)))
        (*toFUI)->segmentStatus() = (*toFUI)->paxTypeFare()->segmentStatus();
      else
        (*toFUI)->segmentStatus() = (*fromFUI)->segmentStatus();
    }

    if (!(*fromFUI)->segmentStatusRule2().empty())
    {
      if ((*fromFUI)->segmentStatusRule2().front()._bkgCodeSegStatus.isSet(
              PaxTypeFare::BKSS_NOT_YET_PROCESSED))
        (*toFUI)->segmentStatusRule2() = (*toFUI)->paxTypeFare()->segmentStatusRule2();
      else
        (*toFUI)->segmentStatusRule2() = (*fromFUI)->segmentStatusRule2();
    }
    if (_trx->getRequest()->isSFR())
      structuredFareRulesUtils::copyAdvanceResAndTktData(**fromFUI, **toFUI);
  }
}

void
PricingUnitFactory::setNegotiatedFarePriority(PUPQItem* pupqItem)
{
  PricingUnit& pricingUnit = *pupqItem->pricingUnit();
  if (pricingUnit.fareUsage().size() < 2)
  {
    return;
  }
  RuleNumber ruleNumber;
  uint32_t r3Cat35ItemNo = 0;

  for (const auto* fareUsage : pricingUnit.fareUsage())
  {
    const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
    if (!paxTypeFare->isNegotiated())
    {
      continue;
    }
    if (ruleNumber.empty())
    {
      ruleNumber = paxTypeFare->ruleNumber();
      r3Cat35ItemNo = getCat35Record3ItemNo(*paxTypeFare);
      continue;
    }
    if (paxTypeFare->ruleNumber() != ruleNumber ||
        getCat35Record3ItemNo(*paxTypeFare) != r3Cat35ItemNo)
    {
      pupqItem->mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
      return;
    }
  }
}

uint32_t
PricingUnitFactory::getCat35Record3ItemNo(const PaxTypeFare& paxTypeFare)
{
  const PaxTypeFareRuleData* ptfRuleData =
      paxTypeFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);
  if (UNLIKELY(!ptfRuleData))
    return 0;

  const CategoryRuleItemInfo* crItemInfo = ptfRuleData->categoryRuleItemInfo();
  return crItemInfo->itemNo();
}

bool
PricingUnitFactory::useCxrInCxrFltTbl(const std::vector<TravelSeg*>& tvlSegs,
                                      const VendorCode& vendor,
                                      int carrierFltTblItemNo,
                                      const DateTime& ticketingDate) const
{
  return RuleUtil::useCxrInCxrFltTbl(tvlSegs, vendor, carrierFltTblItemNo, ticketingDate);
}

bool
PricingUnitFactory::getGlobalDirection(DateTime travelDate,
                                       TravelSeg& tvlSeg,
                                       GlobalDirection& globalDir) const
{
  return GlobalDirectionFinderV2Adapter::getGlobalDirection(_trx, travelDate, tvlSeg, globalDir);
}

bool
PricingUnitFactory::isPuRT(const PaxTypeFare& paxTypeFare1,
                           const PaxTypeFare& paxTypeFare2,
                           DiagCollector& diag,
                           uint16_t catNum)
{
  RoundTripCheck::CompareResult result = RoundTripCheck::BLANK_IND;
  if ((result = RoundTripCheck::checkRec1Indicator(paxTypeFare1, paxTypeFare2, catNum)) ==
      RoundTripCheck::BLANK_IND)
  {
    const PaxTypeFareRuleData* ptfRd1 = paxTypeFare1.paxTypeFareRuleData(catNum);
    const PaxTypeFareRuleData* ptfRd2 = paxTypeFare2.paxTypeFareRuleData(catNum);

    const GeneralFareRuleInfo* gfrInfo1 = nullptr;
    const GeneralFareRuleInfo* gfrInfo2 = nullptr;

    if (!ptfRd1)
      gfrInfo1 = getRecord2(paxTypeFare1, catNum);
    else
      gfrInfo1 = dynamic_cast<const GeneralFareRuleInfo*>(ptfRd1->categoryRuleInfo());

    if (!ptfRd2)
      gfrInfo2 = getRecord2(paxTypeFare2, catNum);
    else
      gfrInfo2 = dynamic_cast<const GeneralFareRuleInfo*>(ptfRd2->categoryRuleInfo());

    result = RoundTripCheck::checkNullPtr(gfrInfo1, gfrInfo2);
    if (RoundTripCheck::NOT_NULL == result)
      result = RoundTripCheck::checkRec2Indicator(gfrInfo1, gfrInfo2, catNum);
  }

  if (RoundTripCheck::DIFFERENT_IND == result)
  {
    if (diag.isActive())
    {
      if (catNum == RuleConst::DAY_TIME_RULE)
        diag << " APPLY RT IATA EXCEPTION - DIFFERENT CAT 2 OB AND IB - RT\n";
      else if (catNum == RuleConst::SEASONAL_RULE)
        diag << " APPLY RT IATA EXCEPTION - CAT 3 PRESENT IB OR OB  - RT\n";
    }
    return true;
  }
  return false;
}

/*
 * check db/cache for record 2
 * @return GeneralFareRuleInfo*  or null
 */
const GeneralFareRuleInfo*
PricingUnitFactory::getRecord2(const PaxTypeFare& ptf, uint16_t catNum) const
{
  bool isLocSwapped(false);
  return RuleUtil::getGeneralFareRuleInfo(*_trx, ptf, catNum, isLocSwapped);
}

RoundTripCheck::CompareResult
RoundTripCheck::checkRec1Indicator(const PaxTypeFare& paxTypeFare1,
                                   const PaxTypeFare& paxTypeFare2,
                                   uint16_t catNum)
{
  if (RuleConst::DAY_TIME_RULE == catNum)
  {
    return compareIndicators(paxTypeFare1.fcaDowType(), paxTypeFare2.fcaDowType());
  }
  else if (RuleConst::SEASONAL_RULE == catNum)
  {
    return compareIndicators(paxTypeFare1.fcaSeasonType(), paxTypeFare2.fcaSeasonType());
  }
  return BLANK_IND;
}

/*
 * Compare record2 data between fares. If matched it is CT otherwise RT.
 * If data is blank, it check record 3 information.
 */
RoundTripCheck::CompareResult
RoundTripCheck::checkRec2Indicator(const GeneralFareRuleInfo* gfrInfo1,
                                   const GeneralFareRuleInfo* gfrInfo2,
                                   uint16_t catNum)
{
  RoundTripCheck::CompareResult result = BLANK_IND;
  if (RuleConst::DAY_TIME_RULE == catNum)
    result = compareIndicators(gfrInfo1->dowType(), gfrInfo2->dowType());
  else if (RuleConst::SEASONAL_RULE == catNum)
    result = compareIndicators(gfrInfo1->seasonType(), gfrInfo2->seasonType());

  if (BLANK_IND == result)
    result =
        checkItemInfo(gfrInfo1->categoryRuleItemInfoSet(), gfrInfo2->categoryRuleItemInfoSet());
  return result;
}

// It check Rec2 for Cat2 and Cat3. It compares dowType and seasonTypes
RoundTripCheck::CompareResult
RoundTripCheck::checkRec2Indicator(const PaxTypeFare& ptf1,
                                   const PaxTypeFare& ptf2,
                                   uint16_t catNum)
{
  const PaxTypeFareRuleData* ptfRuleData1 = ptf1.paxTypeFareRuleData(catNum);
  const PaxTypeFareRuleData* ptfRuleData2 = ptf2.paxTypeFareRuleData(catNum);

  bool ptf1valid = false, ptf2valid = false;
  if (ptfRuleData1 && ptfRuleData1->categoryRuleInfo())
    ptf1valid = true;

  if (ptfRuleData2 && ptfRuleData2->categoryRuleInfo())
    ptf2valid = true;

  if ((!ptf1valid && ptf2valid) || (ptf1valid && !ptf2valid))
    return DIFFERENT_IND;
  else if (!ptf1valid && !ptf2valid)
    return BLANK_IND;

  const GeneralFareRuleInfo* gfrInfo1 =
      dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData1->categoryRuleInfo());
  const GeneralFareRuleInfo* gfrInfo2 =
      dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData2->categoryRuleInfo());
  if (gfrInfo1 && gfrInfo2)
  {
    if (RuleConst::DAY_TIME_RULE == catNum)
      return compareIndicators(gfrInfo1->dowType(), gfrInfo2->dowType());
    else if (RuleConst::SEASONAL_RULE == catNum)
      return compareIndicators(gfrInfo1->seasonType(), gfrInfo2->seasonType());
  }
  return BLANK_IND;
}

/*
 * \brief It checks Rec 3 itemNo as described below
 * CT:
 * THEN 12345 OR 67890
 * THEN 12345 OR 67890
 *
 * RT:
 * THEN 12345 OR 67890
 * THEN 12345 OR 45568
 *
 * THEN 12345 OR  67890
 * THEN 12345 AND 67890
 *
 * THEN 12345 OR 67890
 * THEN 12345
 *
 * THEN 12345
 * THEN 45568
 *
 * This method check relationInd beside itemNo.
 */
RoundTripCheck::CompareResult
RoundTripCheck::checkRec3ItemNo(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2, uint16_t catNum)
{
  const PaxTypeFareRuleData* ptfRd1 = ptf1.paxTypeFareRuleData(catNum);
  const PaxTypeFareRuleData* ptfRd2 = ptf2.paxTypeFareRuleData(catNum);

  bool ptf1Exists = false, ptf2Exists = false;
  if (ptfRd1 && ptfRd1->categoryRuleItemInfo())
    ptf1Exists = true;

  if (ptfRd2 && ptfRd2->categoryRuleItemInfo())
    ptf2Exists = true;

  if ((ptf1Exists && !ptf2Exists) || (!ptf1Exists && ptf2Exists))
    return DIFFERENT_IND;
  else if (!ptf1Exists && !ptf2Exists)
    return SAME_IND;

  if (ptfRd1->categoryRuleItemInfo()->itemNo() != ptfRd2->categoryRuleItemInfo()->itemNo())
    return DIFFERENT_IND;

  const CategoryRuleInfo* cri1 = ptfRd1->categoryRuleInfo();
  const CategoryRuleInfo* cri2 = ptfRd2->categoryRuleInfo();

  CompareResult result = checkNullPtr(cri1, cri2);
  if (NOT_NULL != result)
    return result;

  return checkItemInfo(cri1->categoryRuleItemInfoSet(), cri2->categoryRuleItemInfoSet());
}

/*
 * check CategoryRuleItemInfo's itemNo and relationInd
 */
RoundTripCheck::CompareResult
RoundTripCheck::checkItemInfo(const std::vector<CategoryRuleItemInfoSet*>& criiSetVec1,
                              const std::vector<CategoryRuleItemInfoSet*>& criiSetVec2)
{
  CompareResult result;
  if (criiSetVec1.size() != criiSetVec2.size())
    return DIFFERENT_IND;

  for (std::vector<CategoryRuleItemInfoSet*>::const_iterator is1 = criiSetVec1.begin(),
                                                             is2 = criiSetVec2.begin();
       is1 != criiSetVec1.end() && is2 != criiSetVec2.end();
       ++is1, ++is2)
  {
    CategoryRuleItemInfoSet* criiSet1 = *is1;
    CategoryRuleItemInfoSet* criiSet2 = *is2;

    // return if both ptr are null or one is null and other not.
    if ((result = checkNullPtr(criiSet1, criiSet2)) != NOT_NULL)
      return result;

    const std::vector<CategoryRuleItemInfo>& criiVec1 = *criiSet1;
    const std::vector<CategoryRuleItemInfo>& criiVec2 = *criiSet2;

    if (criiVec1.size() != criiVec2.size())
      return DIFFERENT_IND;

    for (std::vector<CategoryRuleItemInfo>::const_iterator it1 = criiVec1.begin(),
                                                            it2 = criiVec2.begin();
         it1 != criiVec1.end() && it2 != criiVec2.end();
         ++it1, ++it2)
    {
      const CategoryRuleItemInfo& crii1 = *it1;
      const CategoryRuleItemInfo& crii2 = *it2;

      if (crii1.relationalInd() != crii2.relationalInd() || crii1.itemNo() != crii2.itemNo())
        return DIFFERENT_IND;
    }
  }
  return SAME_IND;
}

bool
PricingUnitFactory::checkISIsValidFare(PaxTypeFare& paxTypeFare) const
{
  if (!_isISPhase)
  {
    return true;
  }

  ShoppingTrx& shoppingTrx = static_cast<ShoppingTrx&>(*_trx);
  if (!shoppingTrx.isSumOfLocalsProcessingEnabled() || shoppingTrx.isAltDates())
  {
    return true;
  }

  const FareMarket& fareMarket = *paxTypeFare.fareMarket();
  const ApplicableSOP* applicableSOP = fareMarket.getApplicableSOPs();
  if (UNLIKELY(!applicableSOP))
    return true;

  for (const ApplicableSOP::value_type& cxrSops : *applicableSOP)
  {
    if (paxTypeFare.flightBitmapPerCarrier().find(cxrSops.first) ==
        paxTypeFare.flightBitmapPerCarrier().end())
       return true;

    paxTypeFare.setComponentValidationForCarrier(
        cxrSops.first, shoppingTrx.isAltDates(), shoppingTrx.mainDuration());

    if (!paxTypeFare.isFltIndependentValidationFVO())
    {
      // not processed yet
      return true;
    }

    if (paxTypeFare.isValid() && !paxTypeFare.flightBitmap().empty())
    {
      // Fail PTF if long connect flag is set and diversity signals for no more long connect SOPs
      return !paxTypeFare.isLongConnectFare() || !shoppingTrx.isNoMoreLngConOptionsNeeded();
    }
  }

  return false;
}

bool
PricingUnitFactory::isNonStopFare(const PaxTypeFare& ptf) const
{
  return !ptf.isInvalidForNonStops();
}

bool
PricingUnitFactory::isNonStopPU(const PricingUnit& pu, DiagCollector& dc) const
{
  for (const FareUsage* fu : pu.fareUsage())
  {
    if (fu->paxTypeFare()->isInvalidForNonStops())
    {
      if (dc.isActive() && !DiagnosticUtil::filter(*_trx, pu))
        dc << " FAILED: INVALID FLIGHT BITMAP (NS)\n";
      return false;
    }
  }

  return true;
}

bool
PricingUnitFactory::isValidPUForValidatingCxr(PricingUnit& pu, DiagCollector& diag)
{
  std::vector<CarrierCode> puCxrVec;
  _itin->getValidatingCarriers(*_trx, puCxrVec);

  if (pu.fareUsage().front()->paxTypeFare()->validatingCarriers().empty())
  {
    // If the fare doesn't have a validating Cxr List,
    // We don't need to process GSA validation and just let this PU pass
    pu.validatingCarriers().clear();
    return true;
  }

  for (const FareUsage* fu : pu.fareUsage())
  {
    // Intersect with already being processed list of Validating Cxr
    PricingUtil::intersectCarrierList(puCxrVec, fu->paxTypeFare()->validatingCarriers());

    if (puCxrVec.empty())
    {
      // Empty intersection list means, this PU is not good for further processing
      pu.validatingCarriers().clear();
      if (UNLIKELY(diag.isActive()))
      {
        diag << " INVALID PU DUE TO NO VALIDATING CXR";
        diag << std::endl;
      }
      return false;
    }
  }

  // We are done with processing all FU of this PU. If we are here, that means the list is not
  // empty.
  // Let's update the PU level Validating Cxr list
  if (UNLIKELY(diag.isActive()))
  {
    diag << " VALIDATING CXR: ";
    for (const auto& puCxr : puCxrVec)
      diag << puCxr << "  ";
    diag << std::endl;
  }

  pu.validatingCarriers() = puCxrVec;
  return true;
}

bool
PricingUnitFactory::isFareValid(PaxTypeFare& paxTypeFare,
                                const PaxTypeBucket& paxTypeCortege,
                                const FareMarket& fm,
                                const uint16_t mktIdx,
                                bool& prevRuleFailed,
                                DiagCollector& diag,
                                bool fxCnException)
{
  bool valid = true;
  std::string failReason;
  bool ctNLSPFareRestPass = true;

  if (UNLIKELY(paxTypeFare.isDummyFare()))
  {
    if (_trx->getRequest()->originBasedRTPricing() && _pu->puType() == PricingUnit::Type::ONEWAY)
    {
      // OW can have only T-1 or T-3 for both Domestic/Transboarder and Intl/ForeignD
      //
      if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) // Tag 2
        return false;
    }
    return true;
  }

  if (!checkIS(paxTypeFare))
    return false;

  if (!checkISIsValidFare(paxTypeFare))
  {
    valid = false;
    if (UNLIKELY(diag.isActive()))
    {
      failReason = std::string("FAILED: INVALID FLIGHT BITMAP\n");
    }
  }

  if ((valid || fallback::failPricingUnitsInIs(_trx)) && isOnlyNonStops() &&
      !isNonStopFare(paxTypeFare))
  {
    valid = false;
    if (diag.isActive())
    {
      failReason = std::string("FAILED: INVALID FLIGHT BITMAP (NS)");
    }
  }

  // For China exception processing: if the fare is qualified CN
  // fare, then do not check Cat10
  if ((valid || fallback::failPricingUnitsInIs(_trx)) &&
      LIKELY(!fxCnException || !paxTypeFare.validForCmdPricing(fxCnException)))
    valid = checkRec2Cat10Indicator(mktIdx, paxTypeFare, failReason);

  //---------------------- reuse prev validation result ----------------------
  if (valid && !checkPrevValidationResult(&paxTypeFare))
  {
    valid = false;
    prevRuleFailed = true;
    if (UNLIKELY(diag.isActive()))
      failReason = std::string("FAILED: PREV PU RULE VALIDATION\n");
  }

  if (valid && !fallback::fallbackFRRProcessingRetailerCode(_trx))
  {
    valid = RuleUtil::isFareValidForRetailerCodeQualiferMatch(*_trx, paxTypeFare);
    if (UNLIKELY(!valid && diag.isActive()))
      failReason = std::string("FAILED: RETAILER CODE MATCH\n");
  }

  //---------------------- cruise PFA currency check ----------------------
  const Customer* const agentTJR = _trx->getRequest()->ticketingAgent()->agentTJR();
  if (valid && agentTJR != nullptr)
  {
    if (agentTJR->noRollupPfaBulkTkt() == YES)
    {
      if (UNLIKELY((_paxType->paxType() == PFA && paxTypeFare.paxType()->paxType() != PFA) ||
          (_paxType->paxType() == CBC &&
           (paxTypeFare.paxType()->paxType() != CBC && paxTypeFare.paxType()->paxType() != PFA)) ||
          (_paxType->paxType() == CBI &&
           (paxTypeFare.paxType()->paxType() != CBC && paxTypeFare.paxType()->paxType() != PFA &&
            paxTypeFare.paxType()->paxType() != CBI))))
      {
        valid = false;
        if (diag.isActive())
          failReason = std::string("FAILED: PAXTYPE PFA CHECK\n");
      }
    }
  }

  //---------------- Check SP CT PU Restriction -----------------
  if (valid && _pu->puType() == PricingUnit::Type::CIRCLETRIP)
  {
    if (_pu->geoTravelType() == GeoTravelType::International && paxTypeFare.isSpecial() &&
        (_pu->puGovCarrier() != PU::ALL_AA_CARRIER || paxTypeFare.carrier() != SPECIAL_CARRIER_AA))
    {
      if (_pu->fcCount() > _maxSPCTFareCompCount)
      {
        ctNLSPFareRestPass = false;
        valid = false;
      }
    }

    if (UNLIKELY(!valid && diag.isActive()))
      failReason = std::string("FAILED: SP CT PU RESTRICTION\n");
  }

  //---------------- Check Fare Tag  -----------------
  if (valid)
    valid = checkFareTag(paxTypeFare, fm.geoTravelType(), fxCnException, diag, failReason);

  //---------------- Check Currency  ----------------
  if (valid)
  {
    if (paxTypeFare.isSpecifiedCurFare())
    {
      valid = true;
    }
    else
    {
      const Directionality puDir = _pu->fareDirectionality()[mktIdx];

      valid = checkCurrency(paxTypeFare,
                            paxTypeCortege,
                            fm,
                            puDir,
                            _multiCurrencyPricing && fm.geoTravelType() != GeoTravelType::Transborder);

      if (UNLIKELY(!valid && diag.isActive()))
        failReason = std::string("FAILED: CURRENCY CHECK\n");

      if (UNLIKELY(!valid && paxTypeFare.isCmdPricing()))
      {
        if (!_altCurrencyRequest ||
            (_altCurrencyRequest && _altCurrencyCode == paxTypeFare.currency()))
        {
          paxTypeFare.setCmdPrcCurrencySelectionFailedFlag();
          valid = true;
          // Or it will be set as COMBINABILITY later
        }
      }
    }
  }

  //---------------- Check FareMarket fareBasisCode ----------------
  if (UNLIKELY(valid && (!fm.fareBasisCode().empty() || fm.isMultiPaxUniqueFareBasisCodes()) &&
               fm.fbcUsage() == COMMAND_PRICE_FBC))
  {
    valid = paxTypeFare.validForCmdPricing(fxCnException);

    if (!valid)
    {
      if (diag.isActive())
        failReason = std::string("FAILED: NOT VALID FOR CMD PRICING\n");
    }
  }

  //---------------- validate Record 8 same carrier/Table 986 ----------------
  if (valid && paxTypeFare.isFareByRule() && _trx->getTrxType() == PricingTrx::MIP_TRX)
  {
    const FareByRuleApp& fbrApp = paxTypeFare.fbrApp();
    if (fbrApp.sameCarrier() != BLANK)
    {
      valid = RuleUtil::useSameCarrier(_itin->travelSeg());

      if (!valid)
      {
        if (diag.isActive())
          failReason = std::string("FAILED: SAME CARRIER CHECK\n");
      }
    }
    else if (UNLIKELY(fbrApp.carrierFltTblItemNo() != 0))
    {
      const FareByRuleApp& fbrApp = paxTypeFare.fbrApp();
      valid = RuleUtil::useCxrInCxrFltTbl(
          _itin->travelSeg(), fbrApp.vendor(), fbrApp.carrierFltTblItemNo(), _trx->ticketingDate());

      if (!valid)
      {
        if (diag.isActive())
          failReason = std::string("FAILED: REC8 CXR/FLT TABLE CHECK\n");
      }
    }
  }

  //----------- Check fare for Account code or Corp ID for XC qualifier -------
  if (valid && _trx->getOptions()->forceCorpFares())
  {
    if ((paxTypeFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF) ||
        ((paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF) && !paxTypeFare.matchedCorpID()))
    {
      valid = (_trx->getRequest()->isWpNettRequested() &&
               _trx->getRequest()->ticketingAgent()->axessUser() && !paxTypeFare.isNegotiated());
      if (!valid && diag.isActive())
        failReason = std::string("FAILED: FARE NOT APPLICABLE FOR THE XC QUALIFIER\n");
    }
  }

  //---- Check Ethnic/Offshore/Worker private fare (PTC is not ADT/NEG)
  //----   when 'Nation France' is coded in cat15/Cat35 security
  //----   and there is no matched Corp_id in Cat1/Rec8/Cat25_if1/Cat15_if1/Cat35_if1/Cat35

  if (UNLIKELY(valid && _trx->getRequest()->ticketingAgent()->cwtUser() && paxTypeFare.isCWTMarketFare()))
  {
    valid = false;
    if (diag.isActive())
      failReason = std::string("FAILED: CWT MARKET FARE NOT APPLICABLE WITH N FR FOR ADT/NEG\n");
  }

  //----------- Check JL/JC CNX Fare selection  ------
  if (valid)
  {
    if (UNLIKELY(checkJLCNXFareSelection(paxTypeFare, fm) == false))
    {
      valid = false;
      if (diag.isActive())
        failReason = std::string("FAILED: CNX FARE - JOURNEY ORIGIN NOT IN JAPAN\n");
    }
  }

  //----------- Check YY Fare validity  ------
  if (valid)
  {
    if (UNLIKELY(checkIndustryFareValidity(paxTypeFare) == false))
    {
      valid = false;
      if (diag.isActive())
        failReason = std::string("FAILED: YY FARE HAS A MATCHED TAG-2 CXR-FARE\n");
    }
  }

  //---------------- Put Diagnostic Msg ----------------
  diag.enable(_pu, Diagnostic601);
  if (UNLIKELY(diag.isActive()))
  {
    display601DiagMsg(
        paxTypeFare, paxTypeCortege, fm, mktIdx, valid, ctNLSPFareRestPass, failReason, diag);
    diag.flushMsg();
  }

  return valid;
}

bool
PricingUnitFactory::isPricingUnitValid(PUPQItem& pupqItem,
                                       const bool allowDelayedValidation,
                                       DiagCollector& diag)
{
  PricingUnit& prU = *pupqItem.pricingUnit();

  if (UNLIKELY(_trx->getRequest()->originBasedRTPricing()))
  {
    bool foundFakeFM = false;
    std::vector<FareUsage*>::const_iterator fuIt = prU.fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = prU.fareUsage().end();
    FareUsage* fakeFareUsage = nullptr;
    FareUsage* realFareUsage = nullptr;
    PaxTypeFare* realPTF = nullptr;
    uint16_t fakeFareIndex = 0;
    bool tag2Found = false;

    while (!foundFakeFM && (fuIt != fuItEnd))
    {
      if ((*fuIt)->paxTypeFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      {
        tag2Found = true;
      }

      if ((*fuIt)->paxTypeFare()->fareMarket()->useDummyFare())
      {
        foundFakeFM = true;
        fakeFareUsage = *fuIt;
      }
      else
      {
        realFareUsage = *fuIt;
        realPTF = realFareUsage->paxTypeFare();
        ++fuIt;
        ++fakeFareIndex;
      }
    }

    if (foundFakeFM)
    {
      if ((prU.puType() == PricingUnit::Type::ROUNDTRIP || prU.puType() == PricingUnit::Type::CIRCLETRIP ||
           prU.puType() == PricingUnit::Type::OPENJAW) &&
          (prU.geoTravelType() == GeoTravelType::Domestic || prU.geoTravelType() == GeoTravelType::Transborder) && (!tag2Found))
      {
        return false;
      }

      bool validCat10 = true;
      if ((prU.puType() == PricingUnit::Type::CIRCLETRIP) && (prU.fareUsage().size() > 2) &&
          (fakeFareIndex < prU.fareUsage().size()))
      {
        if (realFareUsage)
        {
          prU.fareUsage()[fakeFareIndex] = realFareUsage;
        }
        else
        {
          prU.fareUsage()[fakeFareIndex] = prU.fareUsage()[1];
          realFareUsage = prU.fareUsage()[1];
          realPTF = prU.fareUsage()[1]->paxTypeFare();
        }
        validCat10 = checkPULevelCombinability(prU, diag);
        prU.fareUsage()[fakeFareIndex] = fakeFareUsage;

        if (realFareUsage && realPTF)
        {
          realFareUsage->paxTypeFare() = realPTF;
        }
      }
      if (validCat10)
      {
        return true;
      }
      else
      {
        return false;
      }
    } // found fake
  } // if branded-round-trip

  if (!checkAltDatesIS(prU))
    return false;

  if (_delta > 0 && (prU.getTotalPuNucAmount() - _delta > EPSILON) && allowDelayedValidation)
  {
    pupqItem.paused() = true;
    _pausedPUPQItem = &pupqItem;

    return true;
  }

  diag.enable(&prU, Diagnostic603, Diagnostic605);
  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
  {
    displayPricingUnit(prU, diag);
  }

  diag.enable(&prU, Diagnostic603);
  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
  {
    diag.printLine();
  }

  PricingUnit::Type puType = prU.puType();
  const GeoTravelType geoTravelType = prU.geoTravelType();

  diag.enable(&prU, Diagnostic605);

  bool valid = checkPrevValidationResult(prU, diag);

  if (valid && isOnlyNonStops())
    valid = isNonStopPU(prU, diag);

  if (UNLIKELY(valid && _trx->getRequest()->isParityBrandsPath()))
    valid = isValidForIbf(prU, diag);

  if (UNLIKELY(valid && RexPricingTrx::isRexTrxAndNewItin(*_trx)))
  {
    valid = checkRebookedClassesForRex(pupqItem);
    if (!valid)
      diag << " REBOOKED CLS FAILED: TYPE IS NOT AS BOOKED\n";
  }

  if (valid && _trx->isValidatingCxrGsaApplicable())
  {
    valid = isValidPUForValidatingCxr(prU, diag);
  }

  //---------- OPENJAW PU Validation -------------
  if (LIKELY(TrxUtil::isSpecialOpenJawActivated(*_trx)))
  {
    if (valid && puType == PricingUnit::Type::OPENJAW && _pu->isSpecialOpenJaw())
      valid = checkSOJFareRestriction(prU);
  }

  if (!TrxUtil::isdiffCntyNMLOpenJawActivated(*_trx) &&
      valid && puType == PricingUnit::Type::OPENJAW && geoTravelType == GeoTravelType::International)
  {
      valid = checkOJSurfaceRestriction(prU, diag);
  }

  //---------- ROUNDTRIP PU Validation -------------
  else if ((valid && puType == PricingUnit::Type::ROUNDTRIP) && processRTCTIATAExceptions(prU, diag))
    puType = PricingUnit::Type::CIRCLETRIP;

  //---------- CIRCLETRIP PU Validation -------------
  if (valid && (puType == PricingUnit::Type::CIRCLETRIP))
  {
    // Max number of FC allowed in SP-CT-PU is from
    // config file
    if (!checkCTFareRestriction(prU))
    {
      std::set<uint16_t> ctItins;

      valid = false;

      if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
      {
        diag << " FAILED: CT-NLSP-REST ";
        if (!ctItins.empty())
          diag << "FOR ITIN " << DiagnosticUtil::containerToString(ctItins);
        diag << std::endl;
      }
    }
  }

  // ------------------- Check if Tag-2 Fare Needed -------------
  //
  if (valid && (geoTravelType == GeoTravelType::Domestic || geoTravelType == GeoTravelType::Transborder))
  {
    if (puType == PricingUnit::Type::ROUNDTRIP || puType == PricingUnit::Type::CIRCLETRIP ||
        puType == PricingUnit::Type::OPENJAW)
    {
      valid =
          std::any_of(prU.fareUsage().cbegin(),
                      prU.fareUsage().cend(),
                      [](const auto* fareUsage)
                      { return fareUsage->paxTypeFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED; });

      if (!valid)
      {
        if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
        {
          diag << " INVALID PU: ATLEAST ONE TAG-2 FARE NEEDED" << std::endl;
        }
        // Atleast one Tag 2 needed for Domestic & Transborder
        // For Intl/Foreign Domestic we don't care whether all
        // are Tag-1 or Tag-2
      }
    }
  }

  // -------- Check if Cmd Pricing -----------
  if (UNLIKELY(!valid && prU.isCmdPricing()))
  {
    // For command pricing, we consider this combinability failure and
    // let pass
    prU.setCmdPrcFailedFlag(RuleConst::COMBINABILITY_RULE);
    if (diag.isActive() && !DiagnosticUtil::filter(*_trx, prU))
    {
      diag << " -- PU VALIDATION PASS BY COMMAND PRICING" << std::endl;
    }
    valid = true;
  }

  if (valid)
    valid = performPULevelCombinabilityAndRuleValidation(pupqItem, allowDelayedValidation, diag);

  diag.enable(&prU, Diagnostic605);
  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, prU)))
  {
    if (valid && pupqItem.priorityStatus().farePriority() > 1)
      diag << " PU PRIORITY: LOW" << std::endl;
    diag.printLine();
  }
  diag.flushMsg();

  return valid;
}

bool
PricingUnitFactory::isValidForIbf(const PricingUnit& pricingUnit, DiagCollector& diag)
{
  bool valid = true;

  if (_trx->getTrxType() == PricingTrx::IS_TRX)
    valid = hasBrandParity(pricingUnit);

  if (diag.isActive() && !DiagnosticUtil::filter(*_trx, pricingUnit))
  {
    diag << (valid ? " PASSED" : " FAILED:");

    if (_trx->getTrxType() == PricingTrx::IS_TRX)
      diag << " BRAND PARITY\n";
    else
      diag << " IBF CROSS CABIN CHECK\n";
  }

  return valid;
}

bool
PricingUnitFactory::buildFareUsageOld(const PaxTypeFare* primaryPaxTypeFare,
                                      const uint16_t mktIdx,
                                      uint16_t fareIdx,
                                      bool& fareFound,
                                      PUPQItem& pupqItem,
                                      const PaxTypeFare::SegmentStatusVec* segStatus,
                                      bool cxrFarePhase,
                                      const FareType& fareType,
                                      DiagCollector& diag,
                                      bool fxCnException,
                                      bool rexCheckSameFareDate)
{
  MergedFareMarket& mfm = *(_pu->fareMarket()[mktIdx]);

  diag.enable(_pu, Diagnostic601);
  if (UNLIKELY(diag.isActive()))
  {
    display601DiagHeader(mfm, fareIdx, diag);
    if (fxCnException && _trx->diagnostic().diagParamMap()["DD"] == "CNX")
    {
      diag << "CHINA EXCEPTION APPLIED\n";
    }
  }

  // const PaxTypeCode reqPaxType = _paxType->paxType();
  const Directionality puDir = _pu->fareDirectionality()[mktIdx];

  PaxTypeBucket* paxTypeCortege = nullptr;
  FareMarket* fm = nullptr;
  uint16_t relativeIdx = fareIdx;
  getFareMktPaxTypeBucket(mfm, fareIdx, relativeIdx, fm, paxTypeCortege);

  if (!paxTypeCortege)
    return false;

  const bool isCmdPricingFM =
      (fm && (!fm->fareBasisCode().empty() || fm->isMultiPaxUniqueFareBasisCodes()) &&
       fm->fbcUsage() == COMMAND_PRICE_FBC);
  PaxTypeFare* lowestPaxTypeFareForCP = nullptr;

  std::vector<PaxTypeFare*>& paxTypeFareVect = paxTypeCortege->paxTypeFare();
  const uint16_t paxFareSZ = paxTypeFareVect.size();

  PaxTypeFare* validPaxTypeFare = nullptr; // will be set with one we found valid
  bool validFareFound = false;

  // VITA-traditional VC project
  std::set<CarrierCode> vcs;
  bool traditionalVC = false;
  if(_trx->useTraditionalValidatingCxr() && !_itin->traditionalValidatingCxr().empty())
  {
    ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(*_itin, _itin->traditionalValidatingCxr(), vcs);
    if (vcs.empty())
    {
      vcs.insert(_itin->traditionalValidatingCxr());
    }
    traditionalVC = true;
  }

  uint16_t save_relativeIdx = relativeIdx;
  uint16_t save_fareIdx = fareIdx;

  for (relativeIdx = save_relativeIdx, fareIdx = save_fareIdx; relativeIdx < paxFareSZ;
       fareIdx++, relativeIdx++)
  {
    PaxTypeFare* paxTypeFare = paxTypeFareVect[relativeIdx];

    if(traditionalVC && !paxTypeFare->validatingCarriers().empty() &&
       !isFareValidForTraditionalVC(*paxTypeFare, vcs))
      continue;

    if (UNLIKELY(isOnlyOwFares() && (paxTypeFare->owrt() != ONE_WAY_MAY_BE_DOUBLED) &&
        (paxTypeFare->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED)))
      continue;

    //---------------- Check Fare Directionality  ----------------
    // Fare directionality must not be opposite to PU directionality.
    // In Carnival SOL one-way fares queue, fares for all legs cannot
    // be inbound because each leg is treated as outbound.
    const Directionality fareDir = paxTypeFare->directionality();

    if (UNLIKELY(isOnlyOwFares()))
    {
      if (paxTypeFare->isOutboundFareForCarnivalInbound())
      {
        if (puDir == FROM)
          continue;
      }
      else
      {
        if (fareDir == TO)
          continue;
      }
    }
    else
    {
      if (UNLIKELY(paxTypeFare->isOutboundFareForCarnivalInbound()))
        continue;

      if ((puDir != fareDir) && (fareDir != BOTH))
        continue;
    }

    if (!isValidFareForCxrSpecificOpenJaw(*paxTypeFare))
      continue;

    if (!matchPrimaryPaxTypeFareForINF(primaryPaxTypeFare, paxTypeFare))
    {
      continue;
    }

    if (UNLIKELY(!isValidFareTypeGroup(*paxTypeFare)))
    {
      continue;
    }

    if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(*_trx)))
    {
      paxTypeFare->bkgCodeTypeForRex() = paxTypeFare->getRebookedClassesStatus();
      if (!checkRebookedClassesForRex(*paxTypeFare))
      {
        continue;
      }
    }

    if (segStatus &&
        (paxTypeFare->carrier() == INDUSTRY_CARRIER || paxTypeFare->fcaFareType() != fareType ||
         !sameBookingCodes(*segStatus, paxTypeFare->segmentStatus())))
    {
      continue;
    }

    // Check if it's XO request, if so requested PaxType
    // must match to actual PaxType in the PaxTypeFare
    if ((_trx->getOptions()->isXoFares() || _trx->getOptions()->forceCorpFares()))
    {
      if (!isValidForXORequest(*paxTypeFare, *paxTypeCortege, diag))
      {
        continue;
      }
    }

    fareFound = true;

    // 3.5 command pricing
    if (UNLIKELY(isCmdPricingFM))
    {
      if (!paxTypeFare->validForCmdPricing(fxCnException,
                                           (_trx->excTrxType() == PricingTrx::AR_EXC_TRX ||
                                            _trx->excTrxType() == PricingTrx::AF_EXC_TRX)
                                               ? _trx
                                               : nullptr))
      {
        continue;
      }

      //---- Check Ethnic/Offshore/Worker private fare (PTC is not ADT/NEG)
      //----   when 'Nation France' is coded in cat15/Cat35 security
      //----   and there is no matched Corp_id in Cat1/Rec8/Cat25_if1/Cat15_if1/Cat35_if1/Cat35

      if (_trx->getRequest()->ticketingAgent()->cwtUser() && paxTypeFare->isCWTMarketFare())
      {
        continue;
      }
    }

    bool prevRuleFailed = false;

    if (!isFareValid(*paxTypeFare,
                     *paxTypeCortege,
                     *fm,
                     mktIdx,
                     prevRuleFailed,
                     diag,
                     fxCnException))
    {
      if (prevRuleFailed)
      {
        // This paxTypeFare is in the failedPaxTypeFareSet
        // Don't use it any more

        // Mark failed if one pass, or on second pass of two
        if (LIKELY(!_trx->fxCnException()))
        {
          pupqItem.isValid() = false;
        }

        if (UNLIKELY(fxCnException))
        {
          pupqItem.isValid() = false;
        }
      }
      else
      {
        if (LIKELY(!isCmdPricingFM))
        {
          continue;
        }

        // 3.5 command pricing, if none fare is valid with Tag1/2/3
        // requirement, we get the lowest one for WP
        // Other transaction, WPA or WP-NO-MATCH or WPA-NO-MATCH
        // command pricing, we do not want to ignore
        // tag failure, PL 13115
        if (lowestPaxTypeFareForCP == nullptr)
        {
          if (_trx->altTrxType() != PricingTrx::WP)
          {
            std::string failReason;
            if (!checkFareTag(*paxTypeFare, fm->geoTravelType(), fxCnException, diag, failReason))
            {
              continue;
            }
          }

          // paxTypeFareVect is already sorted by fare amount, so the first
          // one matches Fare Market fareBasisCode will be the lowest
          lowestPaxTypeFareForCP = paxTypeFare;
        }
        continue;
      }
    }

    validFareFound = true;
    validPaxTypeFare = paxTypeFare;

    if (UNLIKELY(rexCheckSameFareDate && !pupqItem.pricingUnit()->fareUsage().empty() &&
        !RexBaseTrx::isSameFareDate(pupqItem.pricingUnit()->fareUsage().front()->paxTypeFare(),
                                    paxTypeFare)))
    {
      pupqItem.isValid() = false;
    }
    break;
  } // end of for

  if (!validFareFound)
  {
    // if doing command pricing, use the default one (lowest)
    bool brandedDummyFare = _trx->getRequest()->originBasedRTPricing() && lowestPaxTypeFareForCP &&
                            lowestPaxTypeFareForCP->isDummyFare();

    if (UNLIKELY(lowestPaxTypeFareForCP && !brandedDummyFare))
    {
      if (isValidFareTypeGroup(*lowestPaxTypeFareForCP))
      {
        if (diag.isActive())
        {
          diag << "*** NO FARE VALID   CMD PRICING CHOSE LOWEST ONE ***";
        }
        validPaxTypeFare = lowestPaxTypeFareForCP;

        // set FailedFlag to PricingUnit instead of PaxTypeFare, thinking
        // that if different PU use same PaxTypeFare, it might not
        // fail combinability rule. isFareValid() depends on PU.
        PricingUnit* prU = pupqItem.pricingUnit();
        prU->setCmdPrcFailedFlag(RuleConst::COMBINABILITY_RULE);
        validFareFound = true;
      }
    }
  }

  if (validFareFound)
  {
    pupqItem.fareIndices().push_back(fareIdx); // put fare index for the next expansion

    PaxTypeFare* paxTypeFare = validPaxTypeFare;
    PricingUnit* prU = pupqItem.pricingUnit();
    prU->setTotalPuNucAmount(prU->getTotalPuNucAmount() + paxTypeFare->totalFareAmount());
    prU->mileage() += paxTypeFare->mileage();

    MoneyAmount taxPerMarketBKCode = 0;

    if (TrxUtil::usesPrecalculatedTaxes(*_trx) && _taxPerFareBreak)
    {
      const CxrPrecalculatedTaxes& taxes = paxTypeCortege->cxrPrecalculatedTaxes();
      taxPerMarketBKCode += taxes.getLowerBoundTotalTaxAmount(*paxTypeFare);
    }

    prU->setTotalPuNucAmount(prU->getTotalPuNucAmount() + taxPerMarketBKCode);
    prU->taxAmount() += taxPerMarketBKCode;
    prU->accumulateBaggageLowerBound(paxTypeFare->baggageLowerBound());

    FareUsage* fareUsage = constructFareUsage();
    if (UNLIKELY(fareUsage == nullptr))
    {
      return false;
    }

    prU->fareUsage().push_back(fareUsage);
    fareUsage->mutableForbiddenFop() = paxTypeFare->fare()->forbiddenFop();
    fareUsage->paxTypeFare() = paxTypeFare;
    fareUsage->rec2Cat10() = paxTypeFare->rec2Cat10();
    fareUsage->changePenaltyApply() = paxTypeFare->changePenaltyApply();

    if (puDir == TO)
    {
      // FareUsage is set to INBOUND, OUTBOUND by default
      fareUsage->inbound() = true;
    }

    if (UNLIKELY(isOnlyOwFares()))
    {
      fareUsage->inbound() = false;
      prU->noPUToEOE() = true;
    }

    if ((_pu->puSubType() == PricingUnit::ORIG_OPENJAW ||
         _pu->puSubType() == PricingUnit::DOUBLE_OPENJAW) &&
        mktIdx == 1 && _pu->geoTravelType() == GeoTravelType::International && fareUsage->isOutbound())
    {
      fareUsage->inbound() = true;
      fareUsage->dirChangeFromOutbound() = true;
      fareUsage->ignorePTFCmdPrcFailedFlag() = true;
    }

    if (pupqItem.isValid())
    {
      std::vector<TravelSeg*>& tvlSeg = paxTypeFare->fareMarket()->travelSeg();
      fareUsage->travelSeg() = tvlSeg;

      prU->travelSeg().insert(prU->travelSeg().end(), tvlSeg.begin(), tvlSeg.end());

      if (paxTypeFare->isSpecial())
      {
        prU->puFareType() = PricingUnit::SP;
      }
    }

    bool lowFarePrio = false;
    setPriority(mfm, pupqItem, *paxTypeFare, puDir, cxrFarePhase, lowFarePrio);
    if (lowFarePrio && diag.isActive())
      diag << "NOT CXR FARE - PRIORITY: LOW" << std::endl;

    setUpNewFUforCat31(*fareUsage, *prU, *paxTypeFare);
  }

  return validFareFound;
}

bool
PricingUnitFactory::checkSOJFareRestriction(const PricingUnit& prU) const
{
  return !prU.areAllFaresNormal();
}

}
