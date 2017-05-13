//-------------------------------------------------------------------
// File:    FarePathFactory.cpp
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
#include "Pricing/FarePathFactory.h"

#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/AltPricingUtil.h"
#include "Common/Assert.h"
#include "Common/BookingCodeUtil.h"
#include "Common/ClassOfService.h"
#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/MarriedCabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "Fares/FareByRuleRevalidator.h"
#include "Fares/FareByRuleValidator.h"
#include "Fares/FareTypeMatcher.h"
#include "FreeBagService/BaggageCalculator.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MinFareChecker.h"
#include "Pricing/CombinabilityScoreboard.h"
#include "Pricing/Combinations.h"
#include "Pricing/FPPQItemValidator.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/RuleValidator.h"
#include "Pricing/Shopping/Utils/SetIntersection.h"
#include "Pricing/YFlexiValidator.h"
#include "RexPricing/RefundSolutionValidator.h"
#include "RexPricing/RepriceSolutionValidator.h"
#include "RexPricing/RexPaxTypeFareValidator.h"
#include "Rules/FarePUResultContainer.h"
#include "Rules/PricingUnitLogic.h"
#include "Rules/RuleController.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <sstream>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp)
FALLBACK_DECL(fallbackFlexFareGroupNewXCLogic)
FALLBACK_DECL(fallbackEnableFamilyLogicAvailiablityValidationInBfa)
FALLBACK_DECL(fallbackEnableFamilyLogicAdditionalSolutionsInBfa)
FALLBACK_DECL(fallbackFlexFareGroupNewXOLogic);
FALLBACK_DECL(fallbackCorpIDFareBugFixing);
FALLBACK_DECL(fallbackFFGAcctCodeFareFix);
FALLBACK_DECL(tfpInDiag)
FALLBACK_DECL(fixSpanishLargeFamilyForSRFE);

static Logger
logger("atseintl.Pricing.FarePathFactory");

namespace
{
bool
isCat10AndReValStatusOk(const PUPQItem& pupqItem)
{
  return pupqItem.cat10Status() != PUPQItem::PUValidationStatus::FAILED &&
         pupqItem.ruleReValStatus() != PUPQItem::PUValidationStatus::FAILED;
}
} // unnamed namespace

//----For Debug only methods ------
#if 0
void debugOnlyDisplayPricingUnit(const PricingUnit& pu);
void debugOnlyDisplayMainTripSTLink(const FarePath& fpath);
void debugOnlyDisplayFarePathFiltering(PricingTrx& trx, const FPPQItem& fppqItem, const std::string& fareType, bool filter);
#endif
//-------

FarePathFactory*
FarePathFactoryCreator::create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const
{
  return &trx.dataHandle().safe_create<FarePathFactory>(factoriesConfig);
}

// static
FarePathFactory*
FarePathFactory::createFarePathFactory(const FarePathFactoryCreator& creator,
                                       PUPath* puPath,
                                       PricingUnitFactoryBucket* const puFactoryBucket,
                                       Itin* itin,
                                       PaxFPFBaseData* paxFPFBaseData,
                                       std::vector<SavedFPPQItem>* savedFPPQItems)
{
  Combinations*& combinations = paxFPFBaseData->combinations();
  PricingTrx* pricingTrx = paxFPFBaseData->trx();
  if (!combinations)
  {
    CombinabilityScoreboard* comboScoreboard = nullptr;
    pricingTrx->dataHandle().get(comboScoreboard);
    comboScoreboard->trx() = pricingTrx; // lint -e{413}

    pricingTrx->dataHandle().get(combinations);
    combinations->trx() = pricingTrx; // lint !e413
    combinations->comboScoreboard() = comboScoreboard; // lint !e413
  }

  FarePathFactory* fpFactory = creator.create(paxFPFBaseData->getFactoriesConfig(), *pricingTrx);

  fpFactory->trx() = pricingTrx;
  fpFactory->_isRexTrxAndNewItin = RexPricingTrx::isRexTrxAndNewItin(*pricingTrx);
  fpFactory->itin() = itin;
  fpFactory->puPath() = puPath;
  fpFactory->puFactoryBucket() = puFactoryBucket;
  fpFactory->_paxFPFBaseData = paxFPFBaseData;
  fpFactory->paxType() = paxFPFBaseData->paxType();
  fpFactory->combinations() = combinations;
  fpFactory->_pricingUnitRequester.initialize(*pricingTrx);
  fpFactory->_storage.initialize(*pricingTrx, itin->getMemoryManager(), puPath->allPU().size());
  fpFactory->setSavedFPPQItems(savedFPPQItems);

  fpFactory->_ruleController =
      &pricingTrx->dataHandle().safe_create<RuleControllerWithChancelor<PricingUnitRuleController>>(
          pricingTrx);

  if (UNLIKELY(pricingTrx->isValidatingCxrGsaApplicable() &&
                (pricingTrx->getTrxType() == PricingTrx::MIP_TRX) &&
                (!pricingTrx->getRequest()->isAlternateValidatingCarrierRequest())))
  {
    fpFactory->_ignoreAlternateVCs = true;
  }

  return fpFactory;
}

bool
FarePathFactory::initFarePathFactory(DiagCollector& diag)
{
  LOG4CXX_INFO(logger, "Entered: initFarePathPQ()")

  _failedPricingUnits.initialize(_puPath->allPU().size());

  if (UNLIKELY(_trx->altTrxType() == PricingTrx::WPA ||
                _trx->altTrxType() == PricingTrx::WP_NOMATCH))
  {
    _groupFarePathRequired = false;
  }

  startTimer();
  if (TrxUtil::isTotalPriceEnabled(*_trx))
    _yqyrPreCalc = _itin->yqyrLBFactory()->getYQYRPreCalc(_puPath->fareMarketPath(), _paxType);

  if (TrxUtil::isBaggageInPQEnabled(*_trx))
    _bagCalculator = &_trx->dataHandle().safe_create<BaggageCalculator>(*_trx, *_itin, *_paxType);

  if (LIKELY(_clearingStageForRex != RCS_NEED_PROCESS))
    getAllPricingUnitFactory();

  if (_itin->isSRFEApplicable())
    SRFEUtil::initSpanishResidentFares(*_puPath, *_trx, *_itin);

  if (!buildInitialFarePath(diag))
  {
    resetLowerBound();
    return false;
  }

  setLowerBound();

  _farePathSettings = &_trx->dataHandle().safe_create<FarePathSettings>(
      &_allPUF, &_failedPricingUnits, &_eoeFailedFare, _yqyrPreCalc, _externalLowerBoundAmount);

  return true;
}

//----------------------------------------------------------------------------
int
FarePathFactory::keepValidItems()
{
  std::vector<FPPQItem*> tmpStorage;
  while (!pqEmpty())
  {
    FPPQItem* current(pqTop());
    pqPop();
    if (current->isValid() && current->farePath()->processed())
    {
      tmpStorage.push_back(current);
    }
    else
    {
      releaseFPPQItem(current);
    }
  }
  for (std::vector<FPPQItem*>::const_iterator it(tmpStorage.begin()), itEnd(tmpStorage.end());
       it != itEnd;
       ++it)
  {
    pqPush(*it);
  }
  setLowerBound();
  _fpCount = pqSize();
  _fpCombTried = _fpCount;

  _shutdown = true;

  return _fpCount;
}

FPPQItem*
FarePathFactory::getNextFPPQItemFromQueue()
{
  if (UNLIKELY(pqEmpty()))
  {
    if (!openUpYYFareCombo())
    {
      resetLowerBound();
      return nullptr;
    }
  }
  FPPQItem* fppqItem = pqTop();
  pqPop();
  return fppqItem;
}

FPPQItem*
FarePathFactory::getNextFPPQItem(DiagCollector& diag)
{
  FPPQItem* fppqItem = getNextFPPQItemFromQueue();

  if (UNLIKELY(!fppqItem))
    return nullptr;

  if (_trx->delayXpn() && _fpInitStage)
  {
    _fpInitStage = false;
    for (PricingUnitFactory* puf : _allPUF)
      puf->initStage() = false;
  }

  if (fppqItem->paused())
  {
    fppqItem = buildPausedFarePath(*fppqItem, diag);
    setLowerBound();

    if (!fppqItem)
      return nullptr;

    if (UNLIKELY(fppqItem->paused()))
    {
      LOG4CXX_ERROR(logger,
                    " NEXT IS PAUSED NUC=" << fppqItem->farePath()->getTotalNUCAmount()
                                           << " ext LB = " << _externalLowerBoundAmount)
      pqPush(fppqItem);
      setLowerBound();
      return nullptr;
    }
  }

  if (fppqItem->farePath()->processed())
  {
    decrementCxrFareTypeCount(*fppqItem);

    if (passCxrFarePreference(*fppqItem, diag))
    {
      ++_fpCount;
      setLowerBound();
      return fppqItem;
    }

    return nullptr;
  }

  ++_fpCombTried;
  _paxFPFBaseData->incrementFPCombTried();

  bool valid = fppqItem->isValid();

  if (valid)
  {
    std::list<FPPQItem*> clonedFPPQItems;
    valid = isFarePathValid(*fppqItem, diag, clonedFPPQItems);

    if (_trx->isValidatingCxrGsaApplicable() && valid &&
        !clonedFPPQItems.empty()) // proceed only if we have clones
    {
      fppqItem->farePath()->processed() = true;
      processFarePathClones(fppqItem, clonedFPPQItems);
    }
  }
  else
  {
    // LOG4CXX_INFO(logger, " PREV INVALID FARE-PATH")
    farepathutils::putDiagMsg((*_trx), *fppqItem, diag, _paxFPFBaseData->fpCombTried());
  }

  if (!fallback::tfpInDiag(_trx))
  {
    if (UNLIKELY(_trx->diagnostic().diagnosticType() != DiagnosticNone))
    {
      if (_itin->isThroughFarePrecedence() && _trx->diagnostic().diagParamMapItemPresent("FP"))
      {
        diag.enable(Diagnostic609, Diagnostic610);
        if (DiagnosticUtil::showItinInMipDiag(*_trx, fppqItem->farePath()->itin()->itinNum()))
        {
          diag << "THROUGH FARE PRECEDENCE RANK "
               << _puPath->fareMarketPath()->throughFarePrecedenceRank() << std::endl;
        }
      }
    }
  }

  buildNextFarePathSet(*fppqItem, diag);
  setLowerBound();

  if (valid)
  {
    if (bypassCxrFareCombo(*fppqItem, diag))
    {
      // In case of WPA trx, fail all YY-Fare combination of a FareType
      // once a CXR-Fare combination of that fare type is found.

      return nullptr;
    }

    if (passCxrFarePreference(*fppqItem, diag))
    {
      fppqItem->farePath()->processed() = true;

      if (!fppqItem->farePath()->plusUpFlag() &&
          fppqItem->farePath()->getNUCAdditionalFees() < EPSILON)
      {
        ++_fpCount;
        return fppqItem;
      }
      else
      {
        if ((_lowerBoundFPAmount - fppqItem->farePath()->getNUCAmountScore() > EPSILON) ||
            _lowerBoundFPAmount < 0)
        {
          ++_fpCount;
          return fppqItem;
        }

        if (startMultiPaxShortCkt())
        {
          return nullptr;
        }
        else
        {
          if (!reachedPushBackLimit())
          {
            incrementPushBackCount();
            incrementCxrFareTypeCount(*fppqItem);
            pqPush(fppqItem);
            _paxFPFBaseData->validFPPushedBack() = true;
          }

          if (UNLIKELY(!_trx->isForceNoTimeout() && _paxFPFBaseData->shortCktKeepValidFPsTime() &&
                        (time(nullptr) >= _paxFPFBaseData->shortCktKeepValidFPsTime())))
          {
            _paxFPFBaseData->pricingShortCktHappened() = true;
            keepValidItems();
            LOG4CXX_WARN(logger, "Almost timeout, keeping valid items");
          }
        }
      }
    }
  }

  if (!valid)
  {
    releaseFPPQItem(fppqItem);
  }

  if (pqEmpty())
  {
    if (!openUpYYFareCombo())
    {
      resetLowerBound();
    }
  }

  return nullptr;
}

//----------------------------------------------------------------------------
FPPQItem*
FarePathFactory::buildPausedFarePath(FPPQItem& item, DiagCollector& diag)
{
  FPPQItem* nextItem = &item;
  FPPQItem::Greater<FPPQItem::GreaterFare> gtr;
  do
  {
    FPPQItem& fppqItem = *nextItem;
    fppqItem.paused() = false;
    const uint16_t xPoint = fppqItem.xPoint();
    const std::vector<uint32_t>& puIndices = fppqItem.puIndices();
    const uint32_t puIdx = puIndices[xPoint];

    const MoneyAmount delta = getPUDelta(fppqItem);

    PricingUnitFactory* puf = _allPUF[xPoint];
    puf->delta() = delta;
    PUPQItem* pupqItem = puf->getPUPQItem(puIdx, diag);
    if (pupqItem == nullptr || !isCat10AndReValStatusOk(*pupqItem))
    {
      releaseFPPQItem(&fppqItem);
      return nullptr;
    }

    FarePath& fpath = *fppqItem.farePath();
    PricingUnit* newPU = pupqItem->pricingUnit();
    updateFarePathAmounts(fpath, *newPU, &fppqItem, xPoint);

    fppqItem.xPointPuNUCAmount() = newPU->getTotalPuNucAmount();
    fppqItem.pupqItemVect()[xPoint] = pupqItem;
    fpath.pricingUnit()[xPoint] = newPU;
    fpath.mileage() += newPU->mileage();

    if (pupqItem->paused())
    {
      fppqItem.paused() = true;
    }
    else
    {
      // reset for the new pupqItem
      resetPriority(*pupqItem, fppqItem);
    }

    nextItem = &fppqItem;
    bool isGreaterFPonPQ = false;
    if (!pqEmpty())
    {
      if (gtr(&fppqItem, pqTop()))
        isGreaterFPonPQ = true;

      if (isGreaterFPonPQ)
      {
        nextItem = pqTop();
        pqPop();
        pqPush(&fppqItem);
      }
    }
    if (nextItem->farePath()->getTotalNUCAmount() - _externalLowerBoundAmount > EPSILON)
    {
      pqPush(nextItem);
      return nullptr;
    }
  } while (nextItem->paused());

  return nextItem;
}

//----------------------------------------------------------------------------
void
FarePathFactory::buildNextFarePathSet(FPPQItem& fppqItem, DiagCollector& diagnostic)
{
  if (reachedPushBackLimit())
  {
    return;
  }

  const std::vector<uint32_t>& puIndices = fppqItem.puIndices();
  const MoneyAmount puDelta = getPUDelta(fppqItem);
  const unsigned totalPUFactory = _puPath->totalPU();

  requestForNewPricingUnits(fppqItem.xPoint(), puIndices, puDelta, 1, diagnostic);

  _pricingUnitRequester.processRequests(diagnostic);

  for (unsigned xPoint = fppqItem.xPoint(); xPoint < totalPUFactory; ++xPoint)
  {
    if (!buildFarePath(false, puIndices, xPoint, diagnostic))
    {
      LOG4CXX_DEBUG(logger, "buildFarePath failed, NO more PU at xPoint=" << xPoint);
    }
  }

  _pricingUnitRequester.clearProcessing();
}

void
FarePathFactory::requestForNewPricingUnits(uint16_t xPoint,
                                           const std::vector<uint32_t>& puIndices,
                                           const MoneyAmount puDelta,
                                           unsigned expandStep,
                                           DiagCollector& diagnostic)
{
  const unsigned totalPUFactory = _puPath->totalPU();
  for (; xPoint < totalPUFactory; ++xPoint)
  {
    PricingUnitFactory& factory = *_allPUF[xPoint];
    _pricingUnitRequester.registerRequest(factory, puIndices[xPoint] + expandStep, puDelta);
  }
}

bool
FarePathFactory::buildInitialFarePath(DiagCollector& diagnostic)
{
  if (UNLIKELY(_clearingStageForRex == RCS_NEED_PROCESS))
  {
    _clearingStageForRex = RCS_ALREADY_PROCESSED;
    _pricingUnitRequester.clearFactoryForRex(_allPUF, diagnostic);
  }

  const std::vector<uint32_t> puIndices(_puPath->totalPU(), 0);
  requestForNewPricingUnits(0, puIndices, -1.0, 0, diagnostic);
  _pricingUnitRequester.processRequests(diagnostic);

  bool result = buildFarePath(true, puIndices, 0, diagnostic);
  _pricingUnitRequester.clearProcessing();

  return result;
}

//----------------------------------------------------------------------------
bool
FarePathFactory::buildFarePath(bool initStage,
                               const std::vector<uint32_t>& puIndices,
                               const unsigned xPoint,
                               DiagCollector& diagnostic)
{
  TSE_ASSERT(xPoint < puIndices.size());

  FPPQItem* fppqItem = constructFPPQItem();
  FarePath* farePath = constructFarePath();

  fppqItem->farePath() = farePath;
  farePath->itin() = _itin;
  fppqItem->puIndices() = puIndices;
  fppqItem->puIndices()[xPoint] += initStage ? 0 : 1;
  fppqItem->xPoint() = xPoint;
  farePath->paxType() = _paxType;
  fppqItem->puPath() = _puPath;
  farePath->setFlexFaresGroupId(_puPath->getFlexFaresGroupId());

  if (_yqyrPreCalc)
    farePath->setYqyrNUCAmount(_yqyrPreCalc->lowerBound());

  bool found = true;
  const unsigned totalPUFactory = _puPath->totalPU();
  for (unsigned puIndex = 0; puIndex < totalPUFactory; ++puIndex)
  {
    PricingUnitFactory& factory = *_allPUF[puIndex];
    const uint32_t pricingUnitIndex = fppqItem->puIndices()[puIndex];
    PUPQItem* pupqItem =
        _pricingUnitRequester.getRequestedPUPQItem(factory, pricingUnitIndex, diagnostic);
    if (!pupqItem)
    {
      LOG4CXX_INFO(logger, "buildFarePath: get PricingUnit failed")
      found = false;
      break;
    }

    fppqItem->pupqItemVect().push_back(pupqItem);
    PricingUnit* pricingUnit = pupqItem->pricingUnit();
    farePath->pricingUnit().push_back(pricingUnit);
    farePath->mileage() += pricingUnit->mileage();
    updateFarePathAmounts(*farePath, *pricingUnit);

    if (xPoint == puIndex)
    {
      fppqItem->xPointPuNUCAmount() = pricingUnit->getTotalPuNucAmount();
    }

    if (!isCat10AndReValStatusOk(*pupqItem))
    {
      fppqItem->isValid() = false;
    }

    if (pupqItem->paused())
    {
      fppqItem->paused() = true;
    }

    if (UNLIKELY(_isRexTrxAndNewItin))
    {
      farePath->rebookClassesExists() =
          farePath->rebookClassesExists() || pupqItem->rebookClassesExists();
    }

    farepathutils::setPriority(*_trx, *pupqItem, *fppqItem, itin());
  }

  if (_puPath->isSRFApplicable())
    SRFEUtil::applyDiscountUpperBound(*_trx, *_trx->getOptions(), *farePath, *_puPath);
  else
  {
    if (!fallback::fixSpanishLargeFamilyForSRFE(_trx))
      SRFEUtil::applySpanishLargeFamilyDiscount(*_trx, *farePath);
  }

  if (!found)
  {
    releaseFPPQItem(fppqItem);
    return false;
  }

  processSameFareDate(*fppqItem);

  if (!checkEarlyEOECombinability(initStage, *fppqItem, xPoint, diagnostic))
  {
    releaseFPPQItem(fppqItem);
    return false;
  }

  pqPush(fppqItem);

  return true;
}

//----------------------------------------------------------------------------
void
FarePathFactory::processSameFareDate(FPPQItem& fppqItem)
{
  const bool checkSameFareDate = ((_trx->excTrxType() == PricingTrx::AR_EXC_TRX ||
                                   _trx->excTrxType() == PricingTrx::AF_EXC_TRX) &&
                                  static_cast<RexBaseTrx*>(_trx)->repriceWithSameFareDate());

  FarePath& farePath = *fppqItem.farePath();
  if (UNLIKELY(fppqItem.isValid() && checkSameFareDate && farePath.pricingUnit().size() > 1))
  {
    const std::vector<PricingUnit*>& pricingUnits = farePath.pricingUnit();
    FareUsage& firstFareUsage = *pricingUnits.front()->fareUsage().front();
    for (std::vector<PricingUnit*>::const_iterator i = pricingUnits.begin() + 1;
         i != pricingUnits.end();
         ++i)
    {
      PricingUnit& pricingUnit = **i;
      FareUsage& currentFareUsage = *pricingUnit.fareUsage().front();
      if (!RexBaseTrx::isSameFareDate(firstFareUsage.paxTypeFare(), currentFareUsage.paxTypeFare()))
      {
        fppqItem.isValid() = false;
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
bool
FarePathFactory::reachedPushBackLimit()
{
  if ((_plusUpPushBackCount > _factoriesConfig.plusUpPushBackMax()) &&
      (_factoriesConfig.plusUpPushBackMax() > 0))
  {
    // per FarePathFactory limit reached
    return true;
  }

  // check if global limit reached
  return _paxFPFBaseData->reachedGlobaPushBackLimit();
}

//----------------------------------------------------------------------------
void
FarePathFactory::incrementPushBackCount()
{
  ++_plusUpPushBackCount;
  _paxFPFBaseData->incermentGlobalPushBackCount();
}

//----------------------------------------------------------------------------
bool
FarePathFactory::checkEarlyEOECombinability(const bool initStage,
                                            FPPQItem& fppqItem,
                                            const uint16_t xPoint,
                                            DiagCollector& diagIn)
{
  if (initStage || _allPUF.size() <= 2 || xPoint < 1)
  {
    return true;
  }

  if (UNLIKELY(_trx->getOptions() && _trx->getOptions()->fareX()))
  {
    return true;
  }

  if (fppqItem.paused())
  {
    return true;
  }

  int32_t puFactIdx1 = -1;
  int32_t puFactIdx2 = -1;

  FareUsage* failedSourceFareUsage = nullptr;
  FareUsage* failedEOETargetFareUsage = nullptr;
  bool valid =
      checkEOEFailedFare(*fppqItem.farePath(), failedSourceFareUsage, failedEOETargetFareUsage);

  const bool showItin = DiagnosticUtil::showItinInMipDiag(*_trx,
                                                          fppqItem.farePath()->itin()->itinNum());

  if (valid)
  {
    farepathutils::addMainTripSideTripLink(fppqItem);
  }

  diagIn.enable(fppqItem.puPath(), Diagnostic609, Diagnostic610);
  if (UNLIKELY(diagIn.isActive()))
  {
    if (!valid)
      farepathutils::addMainTripSideTripLink(fppqItem);
    if (showItin)
    {
      diagIn.printLine();
      farepathutils::printFarePathToDiag(*_trx, diagIn, *fppqItem.farePath(), _puPath);
      if (!valid && failedSourceFareUsage && failedEOETargetFareUsage)
      {
        diagIn << " FAILED EARLY EOE CHECK: " << failedSourceFareUsage->paxTypeFare()->fareClass()
               << " - " << failedEOETargetFareUsage->paxTypeFare()->fareClass() << "\n";
      }
    }
  }

  if (valid &&
      checkEOECombinability(fppqItem, failedSourceFareUsage, failedEOETargetFareUsage, diagIn))
  {
    diagIn.enable(fppqItem.puPath(), Diagnostic610);
    if (UNLIKELY(diagIn.isActive()))
    {
      if (showItin)
      {
        diagIn << " PASSED EARLY EOE CHECK\n";

        if (!fallback::tfpInDiag(_trx))
          if (_itin->isThroughFarePrecedence() && _trx->diagnostic().diagParamMapItemPresent("FP"))
            diagIn << "THROUGH FARE PRECEDENCE RANK "
                   << _puPath->fareMarketPath()->throughFarePrecedenceRank() << std::endl;
      }
    }

    fppqItem.eoeValidationStatus() = FPPQItem::EOEValidationStatus::EOE_PASSED;
    return true;
  }
  diagIn.enable(fppqItem.puPath(), Diagnostic610);

  if (failedSourceFareUsage && failedEOETargetFareUsage)
  {
    farepathutils::findPUFactoryIdx(*fppqItem.farePath(),
                                    failedSourceFareUsage,
                                    failedEOETargetFareUsage,
                                    puFactIdx1,
                                    puFactIdx2);
  }
  else
  {
    if (UNLIKELY(diagIn.isActive()))
    {
      if (showItin)
      {
        diagIn << " FAILED EARLY EOE CHECK\n";
      }
    }

    fppqItem.eoeValidationStatus() = FPPQItem::EOEValidationStatus::EOE_FAILED;
    return true; // need to keep in PQ for expansion
  }

  if (puFactIdx1 < 0 || puFactIdx2 < 0)
  {
    if (diagIn.isActive())
    {
      if (showItin)
      {
        diagIn << " FAILED EARLY EOE CHECK\n";
      }
    }
    // need to keep in PQ for expansion or for command pricing
    fppqItem.eoeValidationStatus() = FPPQItem::EOEValidationStatus::EOE_FAILED;
    return true;
  }

  if ((puFactIdx1 < fppqItem.xPoint()) && (puFactIdx2 < fppqItem.xPoint()))
  {
    if (UNLIKELY(diagIn.isActive()))
    {
      if (showItin)
      {
        diagIn << " FAILED EARLY EOE CHECK\n";
      }
    }

    return false;
  }

  if (puFactIdx1 >= fppqItem.xPoint())
  {
    if (UNLIKELY(diagIn.isActive()))
    {
      if (showItin)
      {
        diagIn << " FAILED EARLY EOE CHECK\n";
      }
    }

    // don't want to tune it
    fppqItem.eoeValidationStatus() = FPPQItem::EOEValidationStatus::EOE_FAILED;
    return true; // need to keep in PQ for expansion
  }

  FarePath* tmpFPath = constructFarePath();
  tmpFPath->itin() = _itin;
  tmpFPath->paxType() = _paxType;

  tmpFPath->pricingUnit().insert(tmpFPath->pricingUnit().end(),
                                 fppqItem.farePath()->pricingUnit().begin(),
                                 fppqItem.farePath()->pricingUnit().end());
  tmpFPath->mileage() = fppqItem.farePath()->mileage();
  updateFarePath(*tmpFPath, *fppqItem.farePath());

  FPPQItem::EOEValidationStatus eoeValStatus = FPPQItem::EOEValidationStatus::EOE_UNKNOWN;
  PricingUnitFactory* puf2 = _allPUF[puFactIdx2];
  uint32_t idx2 = fppqItem.puIndices()[puFactIdx2];

  // Find replacement of the 2nd PU of the failed PU-Pair
  PUPQItem* pupqItem2 = findEOEValidPUPair(
      tmpFPath, puFactIdx1, puFactIdx2, idx2, puf2, failedEOETargetFareUsage, eoeValStatus, diagIn);

  bool ret = true;

  if (pupqItem2 == nullptr)
  {
    if (UNLIKELY(diagIn.isActive()))
    {
      if (showItin)
      {
        diagIn << " FAILED EARLY EOE CHECK\n";
      }
    }

    ret = false;
  }
  else
  {
    fppqItem.farePath()->mileage() = tmpFPath->mileage();
    updateFarePath(*fppqItem.farePath(), *tmpFPath);
    replaceWithNewPU(fppqItem, pupqItem2, puFactIdx2, idx2);
    fppqItem.eoeValidationStatus() = eoeValStatus;
  }

  _storage.releaseFarePath(*tmpFPath);

  return ret;
}

//----------------------------------------------------------------------------
PUPQItem*
FarePathFactory::findEOEValidPUPair(FarePath* tmpFPath,
                                    const int32_t puFactIdx1,
                                    const int32_t puFactIdx2,
                                    uint32_t& idx,
                                    PricingUnitFactory* puf,
                                    const FareUsage* const failedEOETargetFareUsage,
                                    FPPQItem::EOEValidationStatus& eoeValStatus,
                                    DiagCollector& diagIn)
{
  puf->delta() = -1;

  uint32_t loopCount = 0;
  while (1)
  {
    ++loopCount;
    PUPQItem* pupqItem = nullptr;
    while (1)
    {
      PricingUtil::checkTrxAborted(
          *_trx, _factoriesConfig.maxNbrCombMsgThreshold(), _fpCombTried, _maxNbrCombMsgSet);
      ++idx;
      pupqItem = puf->getPUPQItem(idx, diagIn);
      if (pupqItem == nullptr)
      {
        return nullptr;
      }

      if ((pupqItem->cat10Status() != PUPQItem::PUValidationStatus::FAILED &&
           pupqItem->ruleReValStatus() != PUPQItem::PUValidationStatus::FAILED) ||
          pupqItem->pricingUnit()->isCmdPricing())
      {
        break;
      }
    }

    if (failedEOETargetFareUsage && loopCount <= _eoeValidPUPairSearchLoopMax &&
        puf->pu()->puType() != PricingUnit::Type::CIRCLETRIP &&
        farepathutils::failedFareExistsInPU(failedEOETargetFareUsage->paxTypeFare(),
                                            *pupqItem->pricingUnit()))
    {
      if (LIKELY(!pupqItem->pricingUnit()->isCmdPricing()))
      {
        continue;
      }
    }

    tmpFPath->decreaseTotalNUCAmount(tmpFPath->pricingUnit()[puFactIdx2]->getTotalPuNucAmount());
    tmpFPath->increaseTotalNUCAmount(pupqItem->pricingUnit()->getTotalPuNucAmount());

    tmpFPath->pricingUnit()[puFactIdx2] = pupqItem->pricingUnit();

    FareUsage* failedSourceFareUsage = nullptr;
    FareUsage* failedTargetFareUsage = nullptr;

    bool saveFailPair = true;
    bool valid = checkEOEFailedFare(*tmpFPath, failedSourceFareUsage, failedTargetFareUsage);
    if (!valid)
    {
      saveFailPair = false;
    }

    if (valid)
    {
      farepathutils::clearMainTripSideTripLink(*tmpFPath);
      farepathutils::addMainTripSideTripLink(*tmpFPath, puPath());
      farepathutils::copyPUPathEOEInfo(*tmpFPath, _puPath);

      diagIn.enable(Diagnostic634);
      if (UNLIKELY(diagIn.isActive()))
      {
        diagIn.printLine();
        diagIn << *tmpFPath;
      }
    }
    else
    {
      diagIn.enable(tmpFPath, Diagnostic609, Diagnostic610);
      if (UNLIKELY(diagIn.isActive()))
      {
        farepathutils::clearMainTripSideTripLink(*tmpFPath);
        farepathutils::addMainTripSideTripLink(*tmpFPath, puPath());
        if (DiagnosticUtil::showItinInMipDiag(*_trx, tmpFPath->itin()->itinNum()))
        {
          diagIn.printLine();
          farepathutils::printFarePathToDiag(*_trx, diagIn, *tmpFPath, _puPath);
          diagIn << " FAILED EARLY EOE CHECK: " << failedSourceFareUsage->paxTypeFare()->fareClass()
                 << " - " << failedTargetFareUsage->paxTypeFare()->fareClass() << "\n";
        }
      }
    }

    if (valid &&
        _combinations->process(
            *tmpFPath, 0, failedSourceFareUsage, failedTargetFareUsage, diagIn) == CVR_PASSED)
    {
      eoeValStatus = FPPQItem::EOEValidationStatus::EOE_PASSED;
      return pupqItem;
    }
    else
    {
      int32_t curIdx1 = -1;
      int32_t curIdx2 = -1;
      if (LIKELY(failedSourceFareUsage && failedTargetFareUsage))
      {
        farepathutils::findPUFactoryIdx(
            *tmpFPath, failedSourceFareUsage, failedTargetFareUsage, curIdx1, curIdx2);

        if (saveFailPair)
        {
          saveEOEFailedFare(failedSourceFareUsage->paxTypeFare(),
                            failedTargetFareUsage->paxTypeFare());
        }
      }
      if (curIdx1 != puFactIdx1 || curIdx2 != puFactIdx2)
      {
        // failing diff PU pair
        // we still need it in PQ for expanding combination
        // or for CMD pricing

        eoeValStatus = FPPQItem::EOEValidationStatus::EOE_FAILED;
        return pupqItem;
      }

      if (_externalLowerBoundAmount > 0 &&
          tmpFPath->getTotalNUCAmount() - _externalLowerBoundAmount > EPSILON &&
          (puf->pu()->puType() == PricingUnit::Type::CIRCLETRIP ||
           loopCount > _eoeValidPUPairSearchLoopMax))
      {
        // building CT-PU is more expensive, therefore not continuing the loop,
        // also continuing this loop too long when this FarePath is not cheaper
        // is not good for performance either

        eoeValStatus = FPPQItem::EOEValidationStatus::EOE_FAILED;
        return pupqItem;
      }
    }

  } // end while

  return nullptr;
}

//----------------------------------------------------------------------------
void
FarePathFactory::replaceWithNewPU(FPPQItem& fppqItem,
                                  PUPQItem* pupqItem,
                                  const int32_t puFactIdx,
                                  const uint32_t idx)
{
  if (puFactIdx == fppqItem.xPoint())
  {
    fppqItem.xPointPuNUCAmount() = pupqItem->pricingUnit()->getTotalPuNucAmount();
  }

  fppqItem.farePath()->pricingUnit()[puFactIdx] = pupqItem->pricingUnit();
  fppqItem.puIndices()[puFactIdx] = idx;
  fppqItem.pupqItemVect()[puFactIdx] = pupqItem;

  recalculatePriority(fppqItem);
}

//----------------------------------------------------------------------------
void
FarePathFactory::recalculatePriority(FPPQItem& fppqItem)
{
  fppqItem.clearPriority();
  std::vector<PUPQItem*>::iterator it = fppqItem.pupqItemVect().begin();
  std::vector<PUPQItem*>::iterator itEnd = fppqItem.pupqItemVect().end();
  for (; it != itEnd; ++it)
  {
    farepathutils::setPriority(*_trx, **it, fppqItem, itin());
  }
}

//----------------------------------------------------------------------------
FPPQItem*
FarePathFactory::getSameFareBasisFPPQItem(const FPPQItem& primaryFPPQItem, DiagCollector& diag)
{
  const FarePath& primaryFP = *primaryFPPQItem.farePath();

  std::ostringstream primaryFareBasis;
  for (const PricingUnit* pu : primaryFP.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      primaryFareBasis << fu->paxTypeFare()->fareClass();

  if (!_searchedPrimaryFareBasis.insert(primaryFareBasis.str()).second)
  {
    // already tried
    return nullptr;
  }

  FPPQItem* fppqItem = constructFPPQItem();
  FarePath* fpath = constructFarePath();
  fppqItem->farePath() = fpath;

  TSE_ASSERT(primaryFP.pricingUnit().size() == _allPUF.size());

  for (size_t puNo = 0; puNo < _allPUF.size(); ++puNo)
  {
    diag.enable(puPath(), Diagnostic603, Diagnostic601, Diagnostic605);

    PricingUnit& primaryPu = *primaryFP.pricingUnit()[puNo];
    uint16_t puIdx = 0;
    PUPQItem* pupqItem = _allPUF[puNo]->getSameFareBasisPUPQItem(primaryPu, puIdx, diag);

    if (pupqItem == nullptr)
    {
      releaseFPPQItem(fppqItem);
      return nullptr;
    }

    fppqItem->puIndices().push_back(puIdx);
    fppqItem->pupqItemVect().push_back(pupqItem);
    PricingUnit* prU = pupqItem->pricingUnit();
    fpath->pricingUnit().push_back(prU);
    fpath->mileage() += prU->mileage();
    updateFarePathAmounts(*fpath, *prU);
  }

  fpath->itin() = _itin;
  fpath->paxType() = _paxType;
  fppqItem->puPath() = _puPath;
  fppqItem->ignorePUIndices() = true;

  std::list<FPPQItem*> clonedFPPQItems;
  if (isFarePathValid(*fppqItem, diag, clonedFPPQItems))
  {
    fppqItem->farePath()->processed() = true;
    if (_trx->isValidatingCxrGsaApplicable() && clonedFPPQItems.size())
      processFarePathClones(fppqItem, clonedFPPQItems);
    return fppqItem;
  }
  else
    return nullptr;
}

//----------------------------------------------------------------------------
void
FarePathFactory::resetPriority(const PUPQItem& pupqItem, FPPQItem& fppqItem)
{
  const PriorityStatus& prevStatus = fppqItem.pausedPriorityStatus();
  PriorityStatus& status = fppqItem.mutablePriorityStatus();

  // TODO what about negotiated?
  status.setFarePriority(prevStatus.farePriority());
  status.setPaxTypeFarePriority(prevStatus.paxTypeFarePriority());
  status.setFareByRulePriority(prevStatus.fareByRulePriority());
  status.setFareCxrTypePriority(prevStatus.fareCxrTypePriority());
  status.setPtfRank(prevStatus.ptfRank());

  farepathutils::setPriority(*_trx, pupqItem, fppqItem, itin());
}

//----------------------------------------------------------------------------

MoneyAmount
FarePathFactory::getPUDelta(const FPPQItem& prevFPpqItem)
{
  if (_externalLowerBoundAmount <= 0)
  {
    // _externalLowerBoundAmount can be 0, e.g. for INF-PaxType,
    // or -1, means NO other option available, continue with this one
    //
    return _externalLowerBoundAmount;
  }

  // LOG4CXX_ERROR(logger, "_externalLowerBoundAmount=" << _externalLowerBoundAmount
  //                       << " Current FP Amount = " <<  prevFPpqItem.farePath()->totalNUCAmount())
  // LOG4CXX_ERROR(logger, "PU count = " << prevFPpqItem.farePath()->pricingUnit().size()
  //                       << " Delta = " <<
  //                     _externalLowerBoundAmount -
  //                      (prevFPpqItem.farePath()->totalNUCAmount() -
  //                       prevFPpqItem.xPointPuNUCAmount()))

  return _externalLowerBoundAmount -
         (prevFPpqItem.farePath()->getTotalNUCAmount() - prevFPpqItem.xPointPuNUCAmount());
}

//----------------------------------------------------------------------------
void
FarePathFactory::getAllPricingUnitFactory()
{
  std::vector<PU*>::iterator puIt = _puPath->allPU().begin();
  std::vector<PU*>::iterator puItEnd = _puPath->allPU().end();

  for (; puIt != puItEnd; ++puIt)
  {
    std::map<PU*, PricingUnitFactory*>& pufBucket = _puFactoryBucket->puFactoryBucket();

    const std::map<PU*, PricingUnitFactory*>::iterator it = pufBucket.find(*puIt);
    if (UNLIKELY(it == pufBucket.end()))
    {
      LOG4CXX_ERROR(logger, "COULDN'T INITIALIZE FARE PATH FACTORY")
      return;
    }
    _allPUF.push_back(it->second);
  }
  // LOG4CXX_INFO(logger, "Number of PUF in FPF=" << _allPUF.size())
}

//----------------------------------------------------------------------------
void
FarePathFactory::GETCXRPUThrArg::performTask()
{
  try
  {
    if (getNextCxr)
    {
      pupqItem = puf->getNextCxrFarePUPQItem(*prevPUPQItem, isXPoint, valCxr, *(diag));
    }
    else
    {
      pupqItem = puf->getPUPQItem(idx, *(diag));
    }

    if (pupqItem != nullptr)
    {
      done = true;
      if (UNLIKELY(!isCat10AndReValStatusOk(*pupqItem)))
      {
        isValid = false;
      }
    }
    else
    {
      // no more valid pu
      done = false;
    }
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger, "Exception:" << ex.message() << " - GetPU failed")
    errResponseCode = ex.code();
    errResponseMsg = ex.message();
    done = false;
    return;
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(logger, "Exception:" << e.what() << " - GetPU failed")
    errResponseCode = ErrorResponseException::SYSTEM_EXCEPTION;
    errResponseMsg = std::string("ATSEI SYSTEM EXCEPTION");
    done = false;
    return;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "UNKNOWN Exception Caught: GetPU failed")
    errResponseCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    errResponseMsg = std::string("UNKNOWN EXCEPTION");
    done = false;
    return;
  }
}

//----------------------------------------------------------------------------

bool
FarePathFactory::GETCXRPUThrArg::getAlreadyBuiltPUPQItem()
{
  if (getNextCxr)
  {
    return false;
  }

  pupqItem = puf->getAlreadyBuiltPUPQItem(idx);

  if (pupqItem != nullptr)
  {
    done = true;
    if (UNLIKELY(!isCat10AndReValStatusOk(*pupqItem)))
    {
      isValid = false;
    }
    return true;
  }

  return false;
}

void
FarePathFactory::resetLowerBound()
{
  _lowerBoundFPAmount = -1;
}

// FarePath Revalidation (Rule, plus up etc)
bool
FarePathFactory::isFarePathValid(FPPQItem& fppqItem,
                                 DiagCollector& diag,
                                 std::list<FPPQItem*>& clonedFPPQItems)
{
  if (fppqItem.puPath()->isSRFApplicable())
  {
    FarePath& farePath = *fppqItem.farePath();
    if (SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(farePath.pricingUnit()))
      return false;
    SRFEUtil::restoreFarePathForSpanishResident(farePath);
  }

  fppqItem.setFarePathSettings(_farePathSettings);

  FarePathValidator farePathValidator(*_paxFPFBaseData);

  farePathValidator.setSettings(*_farePathSettings);

  farePathValidator.setEoeCombinabilityEnabled(_eoeCombinabilityEnabled);

  bool valid = true;
  valid = farePathValidator.validate(fppqItem, diag, pricingAxess());

  if(valid && (!fallback::fallbackFlexFareGroupNewXCLogic(_trx) ||
    !fallback::fallbackFlexFareGroupNewXOLogic(_trx)))
    valid = isFarePathValidForFlexFareGroup(fppqItem, farePathValidator);

  bool shouldPassCmdPricing = farePathValidator.shouldPassCmdPricing();
  if (valid || shouldPassCmdPricing)
  {
    farepathutils::copyReusablePUToMutablePU(_storage, fppqItem);
  }
  if (valid)
  {
    valid = farePathValidator.penaltyValidation(*fppqItem.farePath());
  }
  std::string res = farePathValidator.getResults();

  const bool isMip = _trx->getTrxType() == PricingTrx::MIP_TRX;

  if (valid && isMip)
  {
    if (!checkFBR(fppqItem))
    {
      if (!fppqItem.farePath()->itin()->getSimilarItins().empty())
        saveFPPQItem(fppqItem, RuleConst::FARE_BY_RULE);
      valid = false;
    }
    res += valid ? "FBR-RECHECK:P " : "FBR-RECHECK:F ";
  }

  std::vector<FPPQItem*> clonedFpPQ;
  if (_trx->isValidatingCxrGsaApplicable())
    ValidatingCxrUtil::cloneFpPQItemForValCxrs(*_trx, fppqItem, clonedFpPQ, _storage);
  else
    clonedFpPQ.push_back(&fppqItem);

  bool isPrimaryValid = valid;
  FPPQItemValidator fppqItemValidator(*_paxFPFBaseData, allPUF(), _failedPricingUnits, diag);
  fppqItemValidator.setYqyrCalc(_yqyrPreCalc);
  fppqItemValidator.setBagCalculator(_bagCalculator);

  for (FPPQItem* fpPQI : clonedFpPQ)
  {
    bool localValid = valid;

    std::string localRes = res;

    if (localValid || shouldPassCmdPricing)
      BookingCodeUtil::copyBkgStatusToFu(*fpPQI->farePath());

    fppqItemValidator.validate(
        fpPQI, localRes, localValid, shouldPassCmdPricing, pricingAxess(), lowerBoundFPPQItem());

    fpPQI->isValid() = localValid;
    if (fpPQI != &fppqItem)
    {
      // only for clones
      if (localValid)
      {
        fpPQI->farePath()->processed() = true;
        clonedFPPQItems.push_back(fpPQI); // keep only if valid
      }
      else
        releaseFPPQItem(fpPQI);
    }
    else
    {
      bool isMotherValid = true;
      isPrimaryValid = localValid;
      if (!fallback::fallbackEnableFamilyLogicAvailiablityValidationInBfa(_trx) && _trx->isBRAll())
      {
        isMotherValid = farepathutils::checkMotherAvailability(fppqItem.farePath(),
                                                               PaxTypeUtil::totalNumSeats(*_trx));
        fpPQI->isValid() = localValid && isMotherValid;
        isPrimaryValid = localValid && isMotherValid;
      }

      bool saveFPPQ =
        !localValid && isPrimaryValid && fppqItemValidator.failedCategoryRevalidatedForSimilarItins();
      uint16_t category = fppqItemValidator.getProcessedCategory();

      if (!fallback::fallbackEnableFamilyLogicAdditionalSolutionsInBfa(_trx) && _trx->isBRAll() &&
          !saveFPPQ)
      {
        saveFPPQ = !isMotherValid;
        if (saveFPPQ)
          category = RuleConst::DUMMY_RULE; //TODO(andrzej.fediuk) add reason
      }

      if (saveFPPQ)
      {
        if (!fppqItem.farePath()->itin()->getSimilarItins().empty())
          saveFPPQItem(fppqItem, category);
      }
    }

    if (UNLIKELY(localValid && _ignoreAlternateVCs))
      return true;
  }

  if (_trx->isValidatingCxrGsaApplicable())
    return (isPrimaryValid || !clonedFPPQItems.empty());
  else
    return isPrimaryValid;
}

//Validate whether farePath is valid for FlexFareGroup
bool
FarePathFactory::isFarePathValidForFlexFareGroup(FPPQItem& fppqItem,
                                                 FarePathValidator& farePathValidator)
{
  bool valid = true;
  if(!fallback::fallbackFlexFareGroupNewXCLogic(_trx))
  { //Validate Farepath for Flex Fare Group XC tag
    const flexFares::GroupId groupID = fppqItem.farePath()->getFlexFaresGroupId();
    if (_trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupID) &&
       (!_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(groupID).empty() ||
       (!fallback::fallbackFFGAcctCodeFareFix(_trx) &&
        !_trx->getRequest()->getMutableFlexFaresGroupsData().getAccCodes(groupID).empty())))
    {
      valid = isFarePathValidForCorpIDFare(fppqItem, groupID);
      if (!valid)
      {
        if (!fallback::fallbackFFGAcctCodeFareFix(_trx))
          farePathValidator.updateResults("CORP_ID/AC_CODE-FARE: F ");
        else
          farePathValidator.updateResults("CORPID-FARE: F ");
      }
    }
  }

  if(!fallback::fallbackFlexFareGroupNewXOLogic(_trx) &&
    (!fallback::fallbackCorpIDFareBugFixing(_trx) && valid))
  { //Validate Farepath for Flex Fare Group XC tag
    const flexFares::GroupId groupID = fppqItem.farePath()->getFlexFaresGroupId();
    if (_trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupID))
    {
      valid = isFarePathValidForXOFare(fppqItem, groupID);
      if (!valid)
        farePathValidator.updateResults("XO-FARE: F ");
    }
  }
  return valid;
}

//Validate whether farePath is valid for FlexFareGroup
bool
FarePathFactory::isFarePathValidForCorpIDFare(FPPQItem& fppqItem, flexFares::GroupId groupID)
{
  //1. Check if the FlexFareGroup has XC indicator
  //2. If XC=OFF, return true if atleast one leg in FarePath has valid CorpID Fare else return false
  //3. If XC=ON, return true if all the legs in FarePath has valid CorpID Fare else return false

  bool xcIndicator = _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(groupID);
  for (const PricingUnit* pu : fppqItem.farePath()->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->matchedCorpID())
      {
        if (!xcIndicator)
          return true; //Found CorpID Fare for atleast one FareUsage
      }
      else //CorpID fare is not available
      {
        if (xcIndicator)
          return false;
      }
    }
  }
  return xcIndicator;
}

//Validate whether farePath is valid for FlexFareGroup with XOFare
bool
FarePathFactory::isFarePathValidForXOFare(FPPQItem& fppqItem, const flexFares::GroupId groupID)
{
  //1. Check if the FlexFareGroup has XO indicator
  //2. If XO=OFF, return true if atleast one Fare component in FarePath has valid XO Fare else return false
  //3. If XO=ON, return true if all the fare components in FarePath has valid XO Fare else return false

  bool xoIndicator = TypeConvert::pssCharToBool(
       _trx->getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(groupID));
  for (const PricingUnit* pu : fppqItem.farePath()->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fppqItem.farePath()->paxType()->paxType() == fu->paxTypeFare()->actualPaxType()->paxType())
      {
        if (!xoIndicator)
          return true; //Found XO Fare for atleast one FareUsage
      }
      else //XO Fare is not available
      {
        if (xoIndicator)
          return false;
      }
    }
  }
  return xoIndicator;
}

bool
FarePathFactory::checkFBR(const FPPQItem& fppqItem) const
{
  return checkFBR<FareByRuleRevalidator>(fppqItem, PaxTypeUtil::totalNumSeats(*_trx));
}

void
FarePathFactory::saveFPPQItem(const FPPQItem& fppqItem, const uint16_t category)
{
  // Tthis logic should be called only for mother itineraries with saving items enabled.
  // Such state is represented by _savedFPPQItems pointer being not nullptr
  TSE_ASSERT(_savedFPPQItems && "Saving item for not mother itinerary!");
  _savedFPPQItems->emplace_back(_savedFPPQItems->size(),
                                category,
                                fppqItem.createDuplicate(*_trx),
                                *_farePathSettings,
                                SavedFPPQItem::Stage::FP_LEVEL);
}

bool
FarePathFactory::checkEOECombinability(FPPQItem& fppqItem,
                                       FareUsage*& failedSourceFareUsage,
                                       FareUsage*& failedTargetFareUsage,
                                       DiagCollector& diag)
{
  if (UNLIKELY(!isEoeCombinabilityEnabled()))
    return true;

  FarePath& fpath = *fppqItem.farePath();
  if (fpath.pricingUnit().size() == 1)
    return true;


  farepathutils::copyPUPathEOEInfo(fpath, fppqItem.puPath());

  if (_combinations->process(fpath,
                             _paxFPFBaseData->fpCombTried(),
                             failedSourceFareUsage,
                             failedTargetFareUsage,
                             diag) != CVR_PASSED)
  {
    if (failedSourceFareUsage != nullptr && failedTargetFareUsage != nullptr)
    {
      saveEOEFailedFare(failedSourceFareUsage->paxTypeFare(), failedTargetFareUsage->paxTypeFare());
    }
    return false;
  }

  return true;
}

void
FarePathFactory::saveEOEFailedFare(const PaxTypeFare* paxTypeFare1, const PaxTypeFare* paxTypeFare2)
{
  typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>> EOEFailedMMap;

  if (UNLIKELY(_factoriesConfig.eoeTuningEnabled() == false))
  {
    return;
  }

  if (_puPath->totalPU() == 1 || _puPath->totalFC() <= 2)
  {
    // No EOE needed
    return;
  }

  // TSELatencyData metrics( *_trx, "SAVE EOE COMB RES" );

  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>>::iterator it =
      _eoeFailedFare.find(paxTypeFare1);
  if (it != _eoeFailedFare.end())
  {
    it->second.insert(paxTypeFare2);
  }
  else
  {
    std::set<const PaxTypeFare*> newSet;
    newSet.insert(paxTypeFare2);
    _eoeFailedFare.insert(EOEFailedMMap::value_type(paxTypeFare1, newSet));
  }

  return;
}

bool
FarePathFactory::checkEOEFailedFare(FarePath& fpath,
                                    FareUsage*& failedSourceFareUsage,
                                    FareUsage*& failedEOETargetFareUsage)
{
  if (UNLIKELY(_factoriesConfig.eoeTuningEnabled() == false))
  {
    return true;
  }

  if (UNLIKELY(_puPath->totalPU() == 1 || _puPath->totalFC() <= 2))
  {
    // No EOE needed
    return true;
  }

  // TSELatencyData metrics( *_trx, "CHECK EOE COMB RES" );

  if (_eoeFailedFare.empty())
  {
    return true;
  }

  std::vector<PricingUnit*>& puVect = fpath.pricingUnit();

  std::vector<PricingUnit*>::iterator puIt = puVect.begin();
  const std::vector<PricingUnit*>::iterator puItEnd = puVect.end();
  const std::vector<PricingUnit*>::iterator puItLast = puVect.end() - 1;
  for (; puIt != puItLast; ++puIt)
  {
    std::vector<FareUsage*>::iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::iterator fuItEnd = (*puIt)->fareUsage().end();
    for (; fuIt != fuItEnd; ++fuIt)
    {
      const PaxTypeFare* sourcePaxTypeFare = (*fuIt)->paxTypeFare();
      typedef std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>> EOEFailedMMap;
      const EOEFailedMMap::iterator fSetIt = _eoeFailedFare.find(sourcePaxTypeFare);
      if (fSetIt == _eoeFailedFare.end())
      {
        continue;
      }

      std::set<const PaxTypeFare*>& failedSet = fSetIt->second;

      auto puTarget = puIt + 1;
      for (; puTarget != puItEnd; ++puTarget)
      {
        for (FareUsage* fareUsage : (*puTarget)->fareUsage())
        {
          PaxTypeFare* targetPaxTypeFare = fareUsage->paxTypeFare();
          if (failedSet.count(targetPaxTypeFare) != 0)
          {
            // TSELatencyData metrics( *_trx, "EOE RES MATCH FOUND" );
            failedSourceFareUsage = *fuIt;
            failedEOETargetFareUsage = fareUsage;
            return false; // EOE pair found
          }
        }
      }
    }
  }

  return true;
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool
FarePathFactory::checkCarrierPreference(const FarePath& fpath, DiagCollector& diag)
{
  if (UNLIKELY(!isEoeCombinabilityEnabled()))
    return true;

  if (!_puPath->abaTripWithOWPU())
  {
    return true;
  }
  PricingUnit& prU1 = *fpath.pricingUnit().front();
  PricingUnit& prU2 = *fpath.pricingUnit().back();

  if (prU1.puFareType() == prU2.puFareType())
  {
    // not NL and SP combination
    return true;
  }

  if (prU1.fareUsage().front()->paxTypeFare()->owrt() ==
      prU2.fareUsage().front()->paxTypeFare()->owrt())
  {
    // not a combination of Tag-1 and Tag-3 in these two OW PUs
    return true;
  }

  const CarrierPreference* cxrPref1 =
      prU1.fareUsage().front()->paxTypeFare()->fareMarket()->governingCarrierPref();

  if (cxrPref1 &&
      (cxrPref1->noApplycombtag1and3() == 'Y' || cxrPref1->noApplycombtag1and3() == 'y'))
  {
    return false;
  }

  const CarrierPreference* cxrPref2 =
      prU2.fareUsage().front()->paxTypeFare()->fareMarket()->governingCarrierPref();

  if (cxrPref2 &&
      (cxrPref2->noApplycombtag1and3() == 'Y' || cxrPref2->noApplycombtag1and3() == 'y'))
  {
    return false;
  }

  return true;
}

void
FarePathFactory::releaseFPPQItem(FPPQItem* fppqItem)
{
  if (LIKELY(fppqItem))
  {
    _storage.releaseFPPQItemDeep(*fppqItem);
  }
}

//----------------------------------------------------------------------------
bool
FarePathFactory::bypassCxrFareCombo(FPPQItem& fppqItem, DiagCollector& diag)
{
  if (UNLIKELY(!_factoriesConfig.searchAlwaysLowToHigh() && _groupFarePathRequired))
  {
    return false;
  }

  if (fppqItem.priorityStatus().farePriority() == DEFAULT_PRIORITY)
  {
    if (UNLIKELY(!_groupFarePathRequired))
    {
      // WPA trx: Save CXR FareType to be able to stop future YY-Fare Combo
      incrementCxrFareTypeCount(fppqItem);

      //===>>>> next lines are for debug only
      // std::string fpFareType;
      // getCxrFareType(fppqItem, fpFareType);
      // debugOnlyDisplayFarePathFiltering(*_trx, fppqItem, fpFareType, false);
      //<<<<< ====
    }
    return false;
  }

  // It is not a complete CXR-Fare-Combination

  return checkCxrFareComboCount(fppqItem);
}

//----------------------------------------------------------------------------
bool
FarePathFactory::checkCxrFareComboCount(FPPQItem& fppqItem)
{
  std::string fpFareType;
  getCxrFareType(fppqItem, fpFareType);

  std::map<std::string, std::map<CarrierCode, uint16_t>>::iterator it =
      _fpFareTypeCountByValCxrMap.find(fpFareType);

  std::vector<CarrierCode> bypassValCxrVect;
  if (it != _fpFareTypeCountByValCxrMap.end())
  {
    int count = 0;
    if (fppqItem.farePath()->validatingCarriers().empty())
    {
      // Non GSA Path or Single Val-Cxr Itin,
      std::map<CarrierCode, uint16_t>::iterator jt = it->second.find(CarrierCode());
      if (jt != it->second.end())
      {
        count = jt->second;
      }
    }
    else
    {
      // multiple val-cxr path
      for (CarrierCode cxr : fppqItem.farePath()->validatingCarriers())
      {
        std::map<CarrierCode, uint16_t>::iterator jt = it->second.find(cxr);
        if (jt != it->second.end())
        {
          if (jt->second > 0)
          {
            // bypassing cxr-fare combo for this cxr
            count = jt->second;
            bypassValCxrVect.push_back(cxr);
          }
        }
      }
    }
    if (count == 0)
    {
      // Not bypassing Cxr-Fare combination
      return false;
    }

    // A CXR-FarePath of same FareType is in the PQ, This YY-FarePath is
    // bypassing the CXR-FarePath

    if (_groupFarePathRequired)
    {
      // Since a valid CXR-Fare combo has already been found and pushed back in to the
      // PQ because of PlusUP, Surchages etc.  we can not use this one yet.
      // Save it in _processedYYFarePaths. We can use it when CXR-FarePath
      // fails in GroupFarePath level (cat-13)

      if (bypassValCxrVect.empty() ||
          bypassValCxrVect.size() == fppqItem.farePath()->validatingCarriers().size())
      {
        fppqItem.farePath()->processed() = true;
        _processedYYFarePaths.insert(std::make_pair(fpFareType, &fppqItem));
        return true;
      }
      else
      {
        FPPQItem* newItem = cloneFPPQItem(fppqItem, bypassValCxrVect);
        newItem->farePath()->processed() = true;
        _processedYYFarePaths.insert(std::make_pair(fpFareType, newItem));

        removeValidatingCarriers(fppqItem, bypassValCxrVect);
        return false; // main fppqItem is not bypassing Cxr-Fare
      }
    }
    else
    {
      if (bypassValCxrVect.size() != fppqItem.farePath()->validatingCarriers().size())
      {
        // some carrier bypasses but some may not
        removeValidatingCarriers(fppqItem, bypassValCxrVect);
        return false; // this fppqItem not bypassing for the remaining val-cxr
      }

      // WPA trx, Once a CXR-Fare combo is found YY-Fare Combo needs to be filtered out
      incrementCxrFareTypeCount(fppqItem);
      releaseFPPQItem(&fppqItem);
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
FPPQItem*
FarePathFactory::cloneFPPQItem(FPPQItem& fppqItem, std::vector<CarrierCode>& valCxrVect)
{
  if (fppqItem.farePath()->gsaClonedFarePaths().empty())
  {
    FPPQItem* newItem = fppqItem.clone(_storage);
    newItem->farePath()->validatingCarriers() = valCxrVect;
    return newItem;
  }

  FPPQItem* newItem = constructFPPQItem();
  *newItem = fppqItem;

  // find the tagged FP for this ValCxr list:
  //
  std::vector<FarePath*> oldTaggedFP;
  std::vector<CarrierCode> vcFoundInTaggedFP;
  for (CarrierCode cxr : valCxrVect)
  {
    FarePath* tfp = ValidatingCxrUtil::getTaggedFarePathWithValCxr(cxr, *fppqItem.farePath());
    if (tfp)
    {
      vcFoundInTaggedFP.push_back(cxr);
      if (std::find(oldTaggedFP.begin(), oldTaggedFP.end(), tfp) == oldTaggedFP.end())
        oldTaggedFP.push_back(tfp);
    }
  }

  // check if we have a new primary FP coming out of TaggedFPs for this list of ValCxr:
  CarrierCode defVcxr = "";
  CarrierCode marketVcxr = "";
  FarePath* newPrimaryFP = nullptr;
  bool retVal = false;
  if (!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
  {
    DefaultValidatingCarrierFinder defValCxrFinder(*_trx, *fppqItem.farePath()->itin());
    retVal = defValCxrFinder.determineDefaultValidatingCarrier(valCxrVect, defVcxr, marketVcxr);
  }
  else
  {
    ValidatingCarrierUpdater valCxrUpdater(*_trx);
    retVal = valCxrUpdater.determineDefaultValidatingCarrier(
        *fppqItem.farePath()->itin(), valCxrVect, defVcxr, marketVcxr);
  }

  if (retVal)
    newPrimaryFP = ValidatingCxrUtil::getTaggedFarePathWithValCxr(defVcxr, *fppqItem.farePath());

  if (!oldTaggedFP.empty())
  {
    std::vector<FarePath*> newTaggedFP;
    if (newPrimaryFP && vcFoundInTaggedFP.size() != valCxrVect.size())
    {
      // old primary FP will become a tagged FP with a smaller VC list
      //
      FarePath* newFP = fppqItem.farePath()->clone(_puPath, _storage);
      newFP->gsaClonedFarePaths().clear();
      newFP->validatingCarriers() = valCxrVect;
      ValidatingCxrUtil::getValCxrSetDifference(
          newFP->validatingCarriers(), vcFoundInTaggedFP, _trx);
      newTaggedFP.push_back(newFP);
    }
    else if (!newPrimaryFP)
    {
      // vc list of old primary FP is changing
      //
      FarePath* newFP = fppqItem.farePath()->clone(_puPath, _storage);
      newFP->gsaClonedFarePaths().clear();
      newFP->validatingCarriers() = valCxrVect;
      ValidatingCxrUtil::getValCxrSetDifference(
          newFP->validatingCarriers(), vcFoundInTaggedFP, _trx);
      newItem->farePath() = newFP;
    }
    else
    {
      // old primary FP is not needed
    }

    // clone tagged FP needed:
    //
    std::vector<FarePath*>::iterator it = oldTaggedFP.begin();
    std::vector<FarePath*>::iterator itEnd = oldTaggedFP.end();
    for (; it != itEnd; ++it)
    {
      FarePath* newFP = (*it)->clone(_puPath, _storage);
      PricingUtil::intersectCarrierList(newFP->validatingCarriers(), valCxrVect);
      if (newPrimaryFP && *it == newPrimaryFP)
      {
        // this one is new primary FP
        newItem->farePath() = newFP;
      }
      else
      {
        newTaggedFP.push_back(newFP);
      }
    }

    // add the cloned tagged FP to the primary
    //
    it = newTaggedFP.begin();
    itEnd = newTaggedFP.end();
    for (; it != itEnd; ++it)
    {
      newItem->farePath()->gsaClonedFarePaths().push_back(*it);
      newItem->farePath()->validatingCarriers().insert(
          newItem->farePath()->validatingCarriers().end(),
          (*it)->validatingCarriers().begin(),
          (*it)->validatingCarriers().end());
    }
  }
  else
  {
    // main FP has the all the vc
    FarePath* newFP = fppqItem.farePath()->clone(_puPath, _storage);
    newFP->gsaClonedFarePaths().clear();
    newFP->validatingCarriers() = valCxrVect;
    newItem->farePath() = newFP;
  }
  return newItem;
}

//----------------------------------------------------------------------------
void
FarePathFactory::removeValidatingCarriers(FPPQItem& fppqItem,
                                          std::vector<CarrierCode>& valCxrsToRemove)
{
  if (fppqItem.farePath()->gsaClonedFarePaths().empty())
  {
    ValidatingCxrUtil::getValCxrSetDifference(
        fppqItem.farePath()->validatingCarriers(), valCxrsToRemove, _trx);
    return;
  }

  std::vector<CarrierCode> newVCList = fppqItem.farePath()->validatingCarriers();
  ValidatingCxrUtil::getValCxrSetDifference(newVCList, valCxrsToRemove, _trx);

  // check if we have a new primary FP coming out of TaggedFPs for this list of vc:
  //
  CarrierCode defVcxr = "";
  CarrierCode marketVcxr = "";
  FarePath* newPrimaryFP = nullptr;
  bool retVal = false;

  if (!fallback::fallbackValidatingCxrMultiSp(_trx) || _trx->overrideFallbackValidationCXRMultiSP())
  {
    DefaultValidatingCarrierFinder defValCxrFinder(*_trx, *fppqItem.farePath()->itin());
    retVal = defValCxrFinder.determineDefaultValidatingCarrier(newVCList, defVcxr, marketVcxr);
  }
  else
  {
    ValidatingCarrierUpdater valCxrUpdater(*_trx);
    retVal = valCxrUpdater.determineDefaultValidatingCarrier(
        *fppqItem.farePath()->itin(), newVCList, defVcxr, marketVcxr);
  }

  if (retVal)
    newPrimaryFP = ValidatingCxrUtil::getTaggedFarePathWithValCxr(defVcxr, *fppqItem.farePath());

  std::vector<FarePath*> newTaggedFP;
  std::vector<CarrierCode> vcFoundInTaggedFP;
  for (FarePath* fp : fppqItem.farePath()->gsaClonedFarePaths())
  {
    PricingUtil::intersectCarrierList(fp->validatingCarriers(), newVCList);
    if (!fp->validatingCarriers().empty())
    {
      if (fp != newPrimaryFP)
        newTaggedFP.push_back(fp);

      vcFoundInTaggedFP.insert(vcFoundInTaggedFP.end(),
                               fp->validatingCarriers().begin(),
                               fp->validatingCarriers().end());
    }
  }

  fppqItem.farePath()->gsaClonedFarePaths().clear();

  if (newPrimaryFP)
  {
    // finding the vc of original PrimaryFP, and check if any remaining:
    //
    fppqItem.farePath()->validatingCarriers() = newVCList;
    ValidatingCxrUtil::getValCxrSetDifference(
        fppqItem.farePath()->validatingCarriers(), vcFoundInTaggedFP, _trx);
    if (!fppqItem.farePath()->validatingCarriers().empty())
    {
      // old primary FP is becoming a tagged FP with shorter vc list
      //
      newPrimaryFP->gsaClonedFarePaths().push_back(fppqItem.farePath());
      newPrimaryFP->validatingCarriers().insert(newPrimaryFP->validatingCarriers().end(),
                                                fppqItem.farePath()->validatingCarriers().begin(),
                                                fppqItem.farePath()->validatingCarriers().end());
    }
    fppqItem.farePath() = newPrimaryFP;
  }
  else
  {
    // finding the vc of original primaryFP:
    fppqItem.farePath()->validatingCarriers() = newVCList;
    ValidatingCxrUtil::getValCxrSetDifference(
        fppqItem.farePath()->validatingCarriers(), vcFoundInTaggedFP, _trx);
  }

  // now add the tagged FP to the (new) primary FP, and append the vc of the taggedFP to the
  // primaryFP
  //
  std::vector<FarePath*>::iterator it = newTaggedFP.begin();
  std::vector<FarePath*>::iterator itEnd = newTaggedFP.end();
  for (; it != itEnd; ++it)
  {
    fppqItem.farePath()->gsaClonedFarePaths().push_back(*it);
    fppqItem.farePath()->validatingCarriers().insert(
        fppqItem.farePath()->validatingCarriers().end(),
        (*it)->validatingCarriers().begin(),
        (*it)->validatingCarriers().end());
  }
}

//----------------------------------------------------------------------------
void
FarePathFactory::incrementCxrFareTypeCount(FPPQItem& fppqItem)
{
  if (UNLIKELY(!_factoriesConfig.searchAlwaysLowToHigh() && _groupFarePathRequired))
  {
    return;
  }

  if (!_puPath->cxrFarePreferred())
  {
    return;
  }

  if (fppqItem.priorityStatus().farePriority() != DEFAULT_PRIORITY)
  {
    return;
  }

  std::string fpFareType;
  getCxrFareType(fppqItem, fpFareType);

  // compare all the FareType as a set. May need to compare only those FT for which
  // CXR fare could not be selected.
  //
  std::map<std::string, std::map<CarrierCode, uint16_t>>::iterator it =
      _fpFareTypeCountByValCxrMap.find(fpFareType);

  if (it != _fpFareTypeCountByValCxrMap.end())
  {
    if (fppqItem.farePath()->validatingCarriers().empty())
    {
      it->second[CarrierCode()] += 1;
    }
    else
    {
      for (CarrierCode cxr : fppqItem.farePath()->validatingCarriers())
      {
        std::map<CarrierCode, uint16_t>::iterator jt = it->second.find(cxr);
        if (jt != it->second.end())
        {
          jt->second++;
        }
        else
        {
          it->second.insert(std::make_pair(cxr, 1));
        }
      }
    }
  }
  else
  {
    std::map<CarrierCode, uint16_t> perCxrMap;
    if (fppqItem.farePath()->validatingCarriers().empty())
    {
      perCxrMap[CarrierCode()] = 1;
    }
    else
    {
      for (CarrierCode cxr : fppqItem.farePath()->validatingCarriers())
      {
        perCxrMap.insert(std::make_pair(cxr, 1));
      }
    }
    _fpFareTypeCountByValCxrMap.insert(std::make_pair(fpFareType, perCxrMap));
  }
}

//----------------------------------------------------------------------------
void
FarePathFactory::decrementCxrFareTypeCount(FPPQItem& fppqItem)
{
  if (!_factoriesConfig.searchAlwaysLowToHigh())
  {
    return;
  }

  if (!_puPath->cxrFarePreferred())
  {
    return;
  }

  if (fppqItem.priorityStatus().farePriority() != DEFAULT_PRIORITY)
  {
    return;
  }

  if (!_groupFarePathRequired)
  {
    return;
  }

  std::string fpFareType;
  getCxrFareType(fppqItem, fpFareType);

  // Comparing all the FareTypes as a set. May need to compare only those FT for which
  // CXR fare could not be selected. May need to go to liner search ...
  //

  std::map<std::string, std::map<CarrierCode, uint16_t>>::iterator it =
      _fpFareTypeCountByValCxrMap.find(fpFareType);

  bool addYYFarePathsToPQ = false;
  if (it != _fpFareTypeCountByValCxrMap.end())
  {
    if (fppqItem.farePath()->validatingCarriers().empty())
    {
      // Non-GSA request or Itin with single Val-Cxr
      std::map<CarrierCode, uint16_t>::iterator jt = it->second.find(CarrierCode());
      if (jt != it->second.end())
      {
        if (jt->second == 1)
        {
          addYYFarePathsToPQ = true;
        }
        if (jt->second >= 1)
          jt->second--;
      }
    }
    else
    {
      // trx with multiple val-cxr
      for (CarrierCode cxr : fppqItem.farePath()->validatingCarriers())
      {
        std::map<CarrierCode, uint16_t>::iterator jt = it->second.find(cxr);
        if (jt != it->second.end())
        {
          if (jt->second == 1)
            addYYFarePathsToPQ = true;

          if (jt->second >= 1)
            jt->second--;
        }
      }
    }

    if (addYYFarePathsToPQ)
    {
      // If the poped one is CXR-FarePath we decrement count, count=0 means
      // previously built YY-FarePath can be used next
      //
      std::pair<std::multimap<std::string, FPPQItem*>::iterator,
                std::multimap<std::string, FPPQItem*>::iterator> p =
          _processedYYFarePaths.equal_range(fpFareType);

      if (p.first == p.second)
      {
        return;
      }
      std::multimap<std::string, FPPQItem*>::iterator i = p.first;
      for (; i != p.second; ++i)
      {
        // CXR-FarePaths are gone, saved YY-FarePath can be used now
        pqPush(i->second);
      }
      _processedYYFarePaths.erase(fpFareType);
    }
  }
}

//----------------------------------------------------------------------------
void
FarePathFactory::getCxrFareType(const FPPQItem& fppqItem, std::string& fpFareType)
{
  const FarePath& fpath = *fppqItem.farePath();
  size_t puCount = fpath.pricingUnit().size();
  for (uint16_t puIdx = 0; puIdx < puCount; ++puIdx)
  {
    const PU& puTemplate = *_allPUF[puIdx]->pu();
    if (!puTemplate.cxrFarePreferred())
    {
      continue;
    }

    const PricingUnit& pu = *fpath.pricingUnit()[puIdx];
    size_t fuCount = pu.fareUsage().size();
    for (uint16_t fuIdx = 0; fuIdx < fuCount; ++fuIdx)
    {
      const PaxTypeFare& ptf = *pu.fareUsage()[fuIdx]->paxTypeFare();
      const FareType& ft = ptf.fcaFareType();
      if (!puTemplate.fareMarket()[fuIdx]->cxrFarePreferred())
      {
        // LOG4CXX_ERROR(logger, "NOT cxrFarePreferred FC="
        //                         << pu.fareUsage()[fuIdx]->paxTypeFare()->fareClass()
        //                         << " FT= " << ft
        //                         << " CXR=" << pu.fareUsage()[fuIdx]->paxTypeFare()->carrier())
        continue;
      }

      // LOG4CXX_ERROR(logger, " cxrFarePreferred FC="
      //                         << pu.fareUsage()[fuIdx]->paxTypeFare()->fareClass()
      //                         << " FT= " << ft
      //                         << " CXR=" << pu.fareUsage()[fuIdx]->paxTypeFare()->carrier())

      fpFareType += ft.c_str();
    }
  }
}

//----------------------------------------------------------------------------
//
// When _factoriesConfig.searchAlwaysLowToHigh() is true, we need to make sure that there is no
// valid
// corresponding CXR-Fare combo available which has higher preference set by Carrier
// in IndustryPricingAppl table.
//
// I am keeping it configureable until we are sure that this procedure does not drag
// down the performance. Until then _factoriesConfig.searchAlwaysLowToHigh() will be FALSE,
// meaning that Higher-CXR fare may come before coresponding lower YY fare.
//
//----------------------------------------------------------------------------

bool
FarePathFactory::passCxrFarePreference(FPPQItem& fppqItem, DiagCollector& diag)
{
  if (UNLIKELY(!_factoriesConfig.searchAlwaysLowToHigh()))
  {
    return true;
  }

  if (_openUpYYFarePhase)
  {
    return true;
  }

  if (fppqItem.priorityStatus().farePriority() == DEFAULT_PRIORITY)
  {
    return true;
  }

  std::map<CarrierCode, std::deque<bool>> cxrFareRest;
  if (!getCxrFareRestriction(fppqItem, cxrFareRest))
  {
    return true;
  }
  std::string fpFareType;
  if (_factoriesConfig.enableCxrFareSearchTuning())
  {
    // check previously found cxr-fare combination
    if (checkCxrFPPQItemFoundBefore(fppqItem, fpFareType))
      return false;
  }
  std::vector<CarrierCode> passingValCxrList;
  getFPValCxrFromPTF(fppqItem, passingValCxrList);
  if (checkCxrFPPQItemExists(fpFareType, fppqItem, passingValCxrList, diag))
  {
    // all vc has corresponding cxr-fare combination,
    // passingValCxrList is also empty in this case but we don't need to check

    // LOG4CXX_ERROR(logger, " CXR FARE COMBO EXISTS ... DO NOT USE YY COMBO")

    if (_groupFarePathRequired)
    {
      // insert to multimap
      fppqItem.farePath()->processed() = true;
      _processedYYFarePaths.insert(std::make_pair(fpFareType, &fppqItem));
    }
    return false;
  }
  else if (!passingValCxrList.empty())
  {
    // Cxr-fare combo may not exists for all the carriers.
    // for these cxr we need to save this YY combo for later possible use,
    // passingValCxrList contains the list of VC for which Cxr-FP was not found
    // For these passing cxr in passingValCxrList, we need to continue with this YY combination

    if (passingValCxrList.size() < fppqItem.farePath()->validatingCarriers().size())
    {
      std::vector<CarrierCode> notPassingValCxrList = fppqItem.farePath()->validatingCarriers();
      ValidatingCxrUtil::getValCxrSetDifference(notPassingValCxrList, passingValCxrList, _trx);

      if (_groupFarePathRequired)
      {
        // save it for possible later use
        FPPQItem* newItem = cloneFPPQItem(fppqItem, notPassingValCxrList);
        newItem->farePath()->processed() = true;
        _processedYYFarePaths.insert(std::make_pair(fpFareType, newItem));
      }

      removeValidatingCarriers(fppqItem, notPassingValCxrList);
    }
    fppqItem.mutablePriorityStatus().setFarePriority(DEFAULT_PRIORITY);
    return true;
  }

  // Cxr-Fare Combination NOT found for ALL Val-Cxr

  // LOG4CXX_ERROR(logger, " CONTINUE With YY COMBO")

  fppqItem.mutablePriorityStatus().setFarePriority(DEFAULT_PRIORITY);
  return true;
}

bool
FarePathFactory::checkCxrFPPQItemFoundBefore(FPPQItem& fppqItem, std::string& fpFareType)
{
  // check previously found cxr-fare combination
  getCxrFareType(fppqItem, fpFareType);
  std::map<std::string, std::vector<CarrierCode>>::iterator it =
      _valCxrCxrFarePathExistanceMap.find(fpFareType);
  if (it != _valCxrCxrFarePathExistanceMap.end())
  {
    // cxr fare combination found before, but check if validating carrier matches
    if (it->second.empty()) // single carrier itinerary
    {
      if (_groupFarePathRequired)
      {
        // insert to multimap
        fppqItem.farePath()->processed() = true;
        _processedYYFarePaths.insert(std::make_pair(fpFareType, &fppqItem));
      }
      return true;
    }
    else
    {
      std::vector<CarrierCode>& cxrFPvalCxrList = it->second;
      std::vector<CarrierCode> passingVC = fppqItem.farePath()->validatingCarriers();

      ValidatingCxrUtil::getValCxrSetDifference(passingVC, cxrFPvalCxrList, _trx);
      if (!passingVC.empty())
      {
        std::vector<CarrierCode> failingVC = fppqItem.farePath()->validatingCarriers();
        ValidatingCxrUtil::getValCxrSetDifference(failingVC, passingVC, _trx);

        if (!failingVC.empty())
        {
          removeValidatingCarriers(fppqItem, failingVC);
          if (_groupFarePathRequired)
          {
            FPPQItem* newItem = cloneFPPQItem(fppqItem, failingVC);
            newItem->farePath()->processed() = true;
            _processedYYFarePaths.insert(std::make_pair(fpFareType, newItem));
          }
        }
        return false; // means this fppqItem has no coresponding cxr-fare-combo
      }
      else
      {
        // all vc has corresponding cxr-fare
        if (_groupFarePathRequired)
        {
          fppqItem.farePath()->processed() = true;
          _processedYYFarePaths.insert(std::make_pair(fpFareType, &fppqItem));
        }
        return true; // this fppqItem has coresponding cxr-fare-combo, do not use this
      }
    }
  }

  return false;
}

/*----------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/

bool
FarePathFactory::openUpYYFareCombo()
{
  if (_processedYYFarePaths.empty())
  {
    return false;
  }

  std::multimap<std::string, FPPQItem*>::iterator it = _processedYYFarePaths.begin();
  std::multimap<std::string, FPPQItem*>::iterator itEnd = _processedYYFarePaths.end();
  for (; it != itEnd; ++it)
  {
    pqPush(it->second);
  }
  _openUpYYFarePhase = true;
  setLowerBound();
  _processedYYFarePaths.clear();

  return true;
}

/*----------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/

bool
FarePathFactory::getCxrFareRestriction(const FPPQItem& fppqItem,
                                       std::map<CarrierCode, std::deque<bool>>& cxrFareRest,
                                       std::vector<CarrierCode>& alreadyPassingValCxrList)
{
  bool anyRestExists = false;

  std::vector<CarrierCode> fpValCxr;
  if (!getFPValCxrFromPTF(fppqItem, fpValCxr))
  {
    // LOG4CXX_DEBUG(logger, ":" << __LINE__ << ":" << __FUNCTION__ <<" Single VC ?")
  }

  for (CarrierCode cxr : fpValCxr)
  {
    std::deque<bool> rest;
    if (getCxrFareRestriction(fppqItem, cxr, rest, alreadyPassingValCxrList))
      anyRestExists = true;
    cxrFareRest.insert(std::map<CarrierCode, std::deque<bool>>::value_type(cxr, rest));
  }

  return anyRestExists;
}

//----------------------------------------------------------------------------

bool
FarePathFactory::getCxrFareRestriction(const FPPQItem& fppqItem,
                                       const CarrierCode& valCxr,
                                       std::deque<bool>& cxrFareRest,
                                       std::vector<CarrierCode>& alreadyPassingValCxrList)
{
  bool anyRestExists = false;
  const uint16_t totalPUFactory = _puPath->totalPU();
  for (uint16_t puFactIdx = 0; puFactIdx < totalPUFactory; ++puFactIdx)
  {
    PUPQItem& pupqItem = *fppqItem.pupqItemVect()[puFactIdx];

    const PU& pu = *_allPUF[puFactIdx]->pu();

    bool cxrRestExists = false;
    const uint16_t totalMkt = pu.fareMarket().size();
    for (uint16_t mktIdx = 0; mktIdx < totalMkt; ++mktIdx)
    {
      const MergedFareMarket& mfm = *pu.fareMarket()[mktIdx];
      const PaxTypeFare& paxTypeFare = *pupqItem.pricingUnit()->fareUsage()[mktIdx]->paxTypeFare();

      if ((paxTypeFare.carrier() == INDUSTRY_CARRIER) && mfm.cxrFarePreferred() &&
          PricingUtil::cxrFareTypeExists(
              mfm, paxTypeFare, pu.puType(), pu.fareDirectionality()[mktIdx], _paxType, valCxr))
      {
        anyRestExists = true;
        cxrRestExists = true;
        break;
      }
    }
    cxrFareRest.push_back(cxrRestExists);
  }

  if (!anyRestExists)
  {
    alreadyPassingValCxrList.push_back(valCxr);
    return false;
  }
  return true;
}

bool
FarePathFactory::getFPValCxrFromPTF(const FPPQItem& fppqItem, std::vector<CarrierCode>& fpValCxr)
{
  if (!fppqItem.farePath()->validatingCarriers().empty())
  {
    // if already determined then not need to get from PaxTypeFare
    fpValCxr = fppqItem.farePath()->validatingCarriers();
    return true;
  }

  for (const PricingUnit* pu : fppqItem.farePath()->pricingUnit())
  {
    if (!pu->validatingCarriers().empty())
    {
      if (fpValCxr.empty())
      {
        // start with the vc list from itin
        _itin->getValidatingCarriers(*_trx, fpValCxr);
      }

      PricingUtil::intersectCarrierList(fpValCxr, pu->validatingCarriers());
      if (fpValCxr.empty())
      {
        // no common val-cxr, no need to continue
        break;
      }

      continue;
    }

    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->validatingCarriers().empty())
      {
        // Non GSA or single VC Path
        CarrierCode blankCC;
        fpValCxr.push_back(blankCC);
        return false;
      }

      if (fpValCxr.empty())
      {
        // start with the vc list from itin
        _itin->getValidatingCarriers(*_trx, fpValCxr);
      }

      PricingUtil::intersectCarrierList(fpValCxr, fu->paxTypeFare()->validatingCarriers());
      if (fpValCxr.empty())
      {
        // no common val-cxr, no need to continue
        break;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool
FarePathFactory::buildCxrFarePath(const bool initStage,
                                  const FPPQItem& prevFPPQItem,
                                  const std::vector<uint32_t>& puIndices,
                                  const CarrierCode& valCxr,
                                  const std::deque<bool>& cxrFareRest,
                                  const uint16_t xPoint,
                                  DiagCollector& diagIn)
{
  FPPQItem* fppqItem = constructFPPQItem();

  FarePath* fpath = constructFarePath();

  fppqItem->farePath() = fpath;
  fpath->itin() = _itin;

  const uint16_t totalPU = _puPath->allPU().size();

  std::vector<GETCXRPUThrArg> thrInputVect;

  //-- Get PricingUnit using Multiple Threads ----
  //
  uint16_t factIdx = 0;
  try
  {
    TseRunnableExecutor pricingUnitPQExecutor(TseThreadingConst::PRICINGUNITPQ_TASK);
    TseRunnableExecutor pricingUnitPQSynchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

    // Create the objects needed to communicate with each thread.
    unsigned int numTasks = _allPUF.size();
    if (UNLIKELY(puIndices.size() < numTasks))
      numTasks = puIndices.size();
    thrInputVect.resize(numTasks);

    //------------ Create/Start Threads --------------

    uint16_t remainingPU = totalPU;
    for (uint16_t factIdx = 0; factIdx < totalPU; ++factIdx, --remainingPU)
    {
      DCFactory* factory = DCFactory::instance();
      DiagCollector* diag = factory->create(*_trx);
      diag->enable(puPath(), Diagnostic603, Diagnostic601, Diagnostic605);

      GETCXRPUThrArg& thrInput = thrInputVect[factIdx];

      // lint --e{413}
      thrInput.idx = puIndices[factIdx];
      thrInput.getNextCxr = cxrFareRest[factIdx];
      thrInput.prevPUPQItem = prevFPPQItem.pupqItemVect()[factIdx];
      thrInput.puf = _allPUF[factIdx];
      thrInput.diag = diag;
      thrInput.trx(_trx);
      thrInput.valCxr = valCxr;

      // if this is the expansionPoint, start from the next pu
      if (!initStage)
      {
        if (factIdx == xPoint)
        {
          ++(thrInput.idx);
          thrInput.isXPoint = true;
        }
      }
      else
        thrInput.isXPoint = true;

      thrInput.puf->delta() = -1;

      if (thrInput.getAlreadyBuiltPUPQItem())
      {
        // no need to start thread for this
        continue;
      }

      TseRunnableExecutor& taskExecutor =
          (remainingPU > 1) ? pricingUnitPQExecutor : pricingUnitPQSynchronousExecutor;

      taskExecutor.execute(thrInput);
    }

    pricingUnitPQExecutor.wait();
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "Exception: BuildFarePath Failed or Timed-Out")
    throw;
  }

  //--------- Collect output from all the Threads -----------
  //
  factIdx = 0;
  bool found = true;
  std::vector<GETCXRPUThrArg>::iterator it = thrInputVect.begin();
  for (; it != thrInputVect.end(); ++it)
  {
    GETCXRPUThrArg& thrArg = (*it);
    diagIn << (DiagCollector&)*(thrArg.diag);

    if (UNLIKELY(thrArg.errResponseCode != ErrorResponseException::NO_ERROR))
    {
      throw ErrorResponseException(thrArg.errResponseCode, thrArg.errResponseMsg.c_str());
    }

    if (found && thrArg.done)
    {
      // lint --e{413}
      fppqItem->pupqItemVect().push_back(thrArg.pupqItem);
      PUPQItem& pupqItem = *thrArg.pupqItem;
      PricingUnit* prU = pupqItem.pricingUnit();
      fpath->pricingUnit().push_back(prU);
      fpath->mileage() += prU->mileage();
      updateFarePathAmounts(*fpath, *prU);

      if (xPoint == factIdx)
      {
        fppqItem->xPointPuNUCAmount() = prU->getTotalPuNucAmount();
      }

      fppqItem->puIndices().push_back(thrArg.idx); // put pu index for the next expansion
      if (UNLIKELY(!thrArg.isValid))
      {
        fppqItem->isValid() = false; // PUScope cat10, ruleReVal status
      }
      if (UNLIKELY(pupqItem.paused()))
      {
        fppqItem->paused() = true;
      }

      //-------------------- Set Priority -------------------------
      //
      farepathutils::setPriority(*_trx, pupqItem, *fppqItem, itin());

      // debugOnlyDisplayPricingUnit(*prU);
    }
    else
    {
      LOG4CXX_INFO(logger, "buildFarePath: get PricingUnit failed")
      found = false;
    }
    ++factIdx;

  } // end for

  if (!found)
  {
    releaseFPPQItem(fppqItem);
    return false;
  }

  // lint --e{413}
  fpath->itin() = _itin;
  fpath->paxType() = _paxType;
  fppqItem->xPoint() = xPoint; // next farePath will expand from the next mkt
  fppqItem->puPath() = _puPath;
  fppqItem->ignorePUIndices() = true;
  fppqItem->farePath()->validatingCarriers().clear();
  fppqItem->farePath()->validatingCarriers().push_back(valCxr);
  _cxrFarePathPQ.push(fppqItem);

  return true;
}

//----------------------------------------------------------------------------
void
FarePathFactory::buildNextCxrFarePathSet(const bool initStage,
                                         const FPPQItem& prevFPPQItem,
                                         const CarrierCode& valCxr,
                                         const std::deque<bool>& cxrFareRest,
                                         DiagCollector& diag)
{
  const uint16_t totalPUFactory = _puPath->totalPU();
  const std::vector<uint32_t> puIndices(prevFPPQItem.puIndices());

  uint16_t xPoint = prevFPPQItem.xPoint();
  for (; xPoint < totalPUFactory; ++xPoint)
  {
    if (!buildCxrFarePath(false, prevFPPQItem, puIndices, valCxr, cxrFareRest, xPoint, diag))
    {
      LOG4CXX_DEBUG(logger, "buildFarePath failed, NO more PU at xPoint=" << xPoint)
    }

    PricingUtil::checkTrxAborted(
        *_trx, _factoriesConfig.maxNbrCombMsgThreshold(), _fpCombTried, _maxNbrCombMsgSet);
  }
}

//----------------------------------------------------------------------------
bool
FarePathFactory::checkCxrFPPQItemExists(const std::string& fpFareType,
                                        const FPPQItem& fppqItem,
                                        std::vector<CarrierCode>& passingValCxrList,
                                        DiagCollector& diag)
{
  std::map<CarrierCode, std::deque<bool>> cxrFareRest;
  std::vector<CarrierCode> alreadyPassingValCxrList; // cxr with no restriction, current FP is ok to
  // use
  if (!getCxrFareRestriction(fppqItem, cxrFareRest, alreadyPassingValCxrList))
  {
    return false;
  }

  ValidatingCxrUtil::getValCxrSetDifference(passingValCxrList, alreadyPassingValCxrList, _trx);
  if (passingValCxrList.empty())
  {
    passingValCxrList = alreadyPassingValCxrList;
    return false;
  }

  bool cxrFPExistsForAllValCxr = true;
  std::vector<CarrierCode> tmpVCList = passingValCxrList;
  for (CarrierCode cxr : tmpVCList)
  {
    auto it = cxrFareRest.find(cxr);
    if (it == cxrFareRest.end())
    {
      cxrFPExistsForAllValCxr = false;
      continue;
    }

    if (!checkCxrFPPQItemExists(fpFareType, fppqItem, cxr, it->second, passingValCxrList, diag))
    {
      cxrFPExistsForAllValCxr = false;
    }
  }
  if (cxrFPExistsForAllValCxr && !alreadyPassingValCxrList.empty())
    cxrFPExistsForAllValCxr = false;

  passingValCxrList.insert(
      passingValCxrList.end(), alreadyPassingValCxrList.begin(), alreadyPassingValCxrList.end());

  return cxrFPExistsForAllValCxr;
}

//----------------------------------------------------------------------------
bool
FarePathFactory::checkCxrFPPQItemExists(const std::string& fpFareType,
                                        const FPPQItem& fppqItem,
                                        const CarrierCode& valCxr,
                                        const std::deque<bool>& cxrFareRest,
                                        std::vector<CarrierCode>& passingValCxrList,
                                        DiagCollector& diag)
{
  while (!_cxrFarePathPQ.empty())
  {
    // clean up previous check
    FPPQItem* item = _cxrFarePathPQ.top();
    _cxrFarePathPQ.pop();
    releaseFPPQItem(item);
  }

  const std::vector<uint32_t> puIndices(_puPath->totalPU(), 0);
  if (!buildCxrFarePath(true, fppqItem, puIndices, valCxr, cxrFareRest, 0, diag))
  {
    // no CXR-Fare combination could be built to init the Q
    return false;
  }

  if (_cxrFarePathPQ.empty())
  {
    return false;
  }

  while (true)
  {
    FPPQItem* fppqItem = _cxrFarePathPQ.top();
    _cxrFarePathPQ.pop();

    FarePath& fpath = *(fppqItem->farePath());
    if (UNLIKELY(fpath.processed()))
    {
      return true;
    }

    ++_fpCombTried;
    _paxFPFBaseData->incrementFPCombTried();

    bool valid = fppqItem->isValid();
    if (LIKELY(valid))
    {
      std::list<FPPQItem*> fppqItems;
      valid = isFarePathValid(*fppqItem, diag, fppqItems);

      if (valid)
      {
        std::vector<CarrierCode> cxrFPvalCxr;
        cxrFPvalCxr.push_back(valCxr);
        recordCxrFPPQItemExistance(fpFareType, cxrFPvalCxr);
        ValidatingCxrUtil::getValCxrSetDifference(passingValCxrList, cxrFPvalCxr, _trx);
        return true; // cxr fare exists
      }
    }
    else
    {
      farepathutils::putDiagMsg((*_trx), *fppqItem, diag, _paxFPFBaseData->fpCombTried());
    }

    buildNextCxrFarePathSet(false, *fppqItem, valCxr, cxrFareRest, diag);

    releaseFPPQItem(fppqItem);

    if (_cxrFarePathPQ.empty())
    {
      return false; // no cxr fare combination found
    }
  }

  return false;
}

//----------------------------------------------------------------------------
void
FarePathFactory::recordCxrFPPQItemExistance(const std::string& fpFareType,
                                            std::vector<CarrierCode>& cxrFPvalCxr)
{
  if (!_factoriesConfig.enableCxrFareSearchTuning())
  {
    return;
  }

  if (cxrFPvalCxr.empty() || !_trx->isValidatingCxrGsaApplicable())
  {
    std::map<std::string, std::vector<CarrierCode>>::iterator it =
        _valCxrCxrFarePathExistanceMap.find(fpFareType);
    if (it == _valCxrCxrFarePathExistanceMap.end())
    {
      _valCxrCxrFarePathExistanceMap.insert(std::make_pair(fpFareType, std::vector<CarrierCode>()));
    }
  }
  else
  {
    std::map<std::string, std::vector<CarrierCode>>::iterator it =
        _valCxrCxrFarePathExistanceMap.find(fpFareType);
    if (it == _valCxrCxrFarePathExistanceMap.end())
    {
      _valCxrCxrFarePathExistanceMap.insert(std::make_pair(fpFareType, cxrFPvalCxr));
    }
    else
    {
      std::vector<CarrierCode>& recordedValCxr = it->second;
      if (recordedValCxr != cxrFPvalCxr)
      {
        std::vector<CarrierCode>::iterator jt = cxrFPvalCxr.begin();
        std::vector<CarrierCode>::iterator jtEnd = cxrFPvalCxr.end();
        for (; jt != jtEnd; ++jt)
        {
          std::vector<CarrierCode>::iterator kt =
              std::find(recordedValCxr.begin(), recordedValCxr.end(), *jt);
          if (kt == recordedValCxr.end())
          {
            recordedValCxr.push_back(*jt);
          }
        }
        cxrFPvalCxr = recordedValCxr;
      }
    }
  }
}

//---------------------------------------------------------------
bool
FarePathFactory::startMultiPaxShortCkt()
{
  if (_trx->getTrxType() == PricingTrx::IS_TRX || _groupFarePathRequired == false ||
      _trx->paxType().size() <= 1 || _factoriesConfig.plusUpPushBackThreshold() < 0)
  {
    // Not for: Shopping IS or WPA or Single-Pax
    return false;
  }

  if (_paxType->paxType() == ADULT)
    return false;

  if (_multiPaxShortCktStarted)
    return false;

  if (_paxType->paxType() == INFANT || _paxType->paxType() == CHILD || _paxType->paxType() == INS ||
      _paxType->paxType() == CNE || _paxType->paxType() == INE || _paxType->paxType() == JNF ||
      _paxType->paxType() == JNN)
  {
    if ((_plusUpPushBackCount >= _factoriesConfig.plusUpPushBackThreshold()) &&
        ((time(nullptr) - _startTime) > _multiPaxShortCktTimeOut))
    {
      _multiPaxShortCktStarted = true;
      return true;
    }
  }

  return false;
}

void
FarePathFactory::clearFactoryForRex(DiagCollector& diag)
{
  if (_clearingStageForRex == RCS_NOT_PROCESSED)
  {
    _clearingStageForRex = RCS_NEED_PROCESS;
    clear();
    initFarePathFactory(diag);
  }
}

//-------------------------------------------------------------------
// Check whether there is at least one brand common for all pricing units in a fare path
bool
FarePathFactory::hasBrandParity(const FarePath& farePath, DiagCollector& diag) const
{
  if (_trx->diagnostic().diagnosticType() == Diagnostic901 &&
      _trx->diagnostic().diagParamMapItem("IBF") == "FAREPATHS")
  {
    diag.enable(Diagnostic901);
    diag << BrandedDiagnosticUtil::displayBrandedFaresFarePathValidation(_trx, farePath);
  }

  BrandParityResult parityValidationResult = hasBrandParity_impl(farePath);

  if (diag.isActive())
  {
    switch (parityValidationResult)
    {
    case PASSED:
      diag << " PASSED BRAND PARITY VALIDATION " << std::endl;
      break;
    case FAILED_COMMON_BRANDS:
      diag << " FAILED BRAND PARITY - NO COMMON BRANDS " << std::endl;
      break;
    case FAILED_STATUS_CHECK:
      diag << " FAILED BRAND PARITY - STATUS CHECK FAILED " << std::endl;
      break;
    default:
      diag << " UNEXPECTED VALIDATION RESULT :  " << parityValidationResult << std::endl;
    }
  }

  return parityValidationResult == PASSED;
}

//-------------------------------------------------------------------
FarePathFactory::BrandParityResult
FarePathFactory::hasBrandParity_impl(const FarePath& farePath) const
{
  utils::SetIntersection<BrandCode> setIntersection;

  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    BrandCodeSet brandsInCurrPU = PricingUtil::getCommonBrandsFromPU(*_trx, pricingUnit);
    setIntersection.addSet(brandsInCurrPU);
    if (setIntersection.get().empty())
    {
      return FAILED_COMMON_BRANDS;
    }
  }

  // In IS we need to make sure that there is at least one brand (having parity) with a HARD_PASS
  // status
  if ((_trx->getTrxType() == PricingTrx::IS_TRX) &&
      !hasHardPassBrandOnEachLeg(farePath, setIntersection.get()))
  {
    return FAILED_STATUS_CHECK;
  }

  return PASSED;
}

//-------------------------------------------------------------------
bool
FarePathFactory::hasHardPassBrandOnEachLeg(const FarePath& farePath,
                                           const std::set<BrandCode>& parityBrands) const
{
  // Should be called only in IS as in MIP there's no trx.legs() that is used here
  ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(_trx);
  TSE_ASSERT(shoppingTrx != nullptr);

  // In regular IS (where there are only thru fares) condition that this function checkc is always
  // fulfilled ( cross cabin logic invalidates soft_passed brands in FCO for thru markets )
  if (!shoppingTrx->isSumOfLocalsProcessingEnabled())
    return true;

  uint16_t legCount = shoppingTrx->legs().size();

  std::set<uint16_t> legsWithHardPassedBrands;

  for (PricingUnit* pricingUnit : farePath.pricingUnit())
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      uint16_t currentLegId = fareUsage->paxTypeFare()->fareMarket()->legIndex();
      if (legsWithHardPassedBrands.find(currentLegId) != legsWithHardPassedBrands.end())
        continue;

      std::vector<BrandCode> hardPassedBrandsInThisFareUsage;
      // getting only hard passed brands (true)
      fareUsage->paxTypeFare()->getValidBrands(*_trx, hardPassedBrandsInThisFareUsage, true);
      // Check that hard_passed brand found is among brands that matter ( those that have parity for
      // the whole fare path )
      for (const BrandCode& hardPassedBrandCodeInThisFareUsage : hardPassedBrandsInThisFareUsage)
      {
        if (parityBrands.find(hardPassedBrandCodeInThisFareUsage) != parityBrands.end())
        {
          legsWithHardPassedBrands.insert(currentLegId);
          break;
        }
      }
      if (legsWithHardPassedBrands.size() == legCount)
        return true;
    }

  return false;
}

//-------------------------------------------------------------------
void
FarePathFactory::updateFarePathAmounts(FarePath& fpath,
                                       const PricingUnit& prU,
                                       FPPQItem* fppqItem,
                                       int16_t xPoint)
{
  if (fppqItem)
    fpath.decreaseTotalNUCAmount(fppqItem->xPointPuNUCAmount());

  fpath.increaseTotalNUCAmount(prU.getTotalPuNucAmount());
}

//-------------------------------------------------------------------
void
FarePathFactory::updateFarePath(FarePath& fpath, const FarePath& inFpath)
{
  fpath.setTotalNUCAmount(inFpath.getTotalNUCAmount());
  fpath.setFlexFaresGroupId(inFpath.getFlexFaresGroupId());
}

void
FarePathFactory::clear()
{
  _fpCount = 0;
  _fpCombTried = 0;
  _lowerBoundFPAmount = -1;
  resetLowerBound();
  _failedPricingUnits.clear();
  _eoeFailedFare.clear();
  _externalLowerBoundAmount = -1;
  _fpFareTypeCountMap.clear();
  _processedYYFarePaths.clear();
  _openUpYYFarePhase = false;
  while (!pqEmpty())
  {
    FPPQItem* fppqItem = pqTop();
    pqPop();
    releaseFPPQItem(fppqItem);
  }
}

void
FarePathFactory::releaseMemory()
{
  _storage.trimMemory();
}

void
FarePathFactory::clearAndReleaseMemory()
{
  clear();
  releaseMemory();
}

void
FarePathFactory::processFarePathClones(FPPQItem*& fppqItem, std::list<FPPQItem*>& clonedFPPQItems)
{
  ValidatingCxrUtil::processFarePathClones(fppqItem, clonedFPPQItems, _storage, _trx);
  for (FPPQItem* item : clonedFPPQItems)
  {
    if (item == fppqItem)
      continue;

    // Do not push back for WPA and WQ
    if (_groupFarePathRequired && !_trx->noPNRPricing())
      pqPush(item);
    else
      releaseFPPQItem(item);
  }
}
}
