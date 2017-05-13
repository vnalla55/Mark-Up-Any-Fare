//------------------------------------------------------------------
// File:    PaxFarePathFactory.cpp
// Created: Feb 2006
// Authors: Mohammad Hossan
//
//  Copyright Sabre 2006
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
#include "Pricing/PaxFarePathFactory.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/AltPricingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag671Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "MinFares/HIPMinimumFare.h"
#include "MinFares/MinFareChecker.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"
#include "Rules/RuleUtil.h"


#include <cassert>

using namespace std;

namespace tse
{

namespace
{
Logger
logger("atseintl.Pricing.PaxFarePathFactory");
}

using FAIL_FARE_PATH_MAP_IS = std::map<FPPQItem*, std::set<DatePair>>;

PaxFarePathFactory*
PaxFarePathFactoryCreator::create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx)
    const
{
  return &trx.dataHandle().safe_create<PaxFarePathFactory>(factoriesConfig);
}

void
PaxFarePathFactory::updateGaussForDelayedExpansion()
{
  for (FareMarketPath* fmp : _allFareMarketPaths)
  {
    size_t puPathSize = fmp->puPaths().size();
    for (size_t i = 0; i < puPathSize; ++i)
      _gauss.include(1);
  }
}

namespace
{
struct FmpSort
{
  FmpSort(PaxType* paxType) : _paxType(paxType) {}

  bool operator()(FareMarketPath* lhs, FareMarketPath* rhs)
  {
    if (lhs->throughFarePrecedenceRank() == rhs->throughFarePrecedenceRank())
      return lhs->firstFareAmt(_paxType) < rhs->firstFareAmt(_paxType);

    return lhs->throughFarePrecedenceRank() < rhs->throughFarePrecedenceRank();
  }

  PaxType* _paxType;
};

} // end namespace

bool
PaxFarePathFactory::processFareMarketPaths()
{
  std::vector<PUPathMatrix*>::iterator i = _puPathMatrixVect.begin();
  const std::vector<PUPathMatrix*>::iterator iEnd = _puPathMatrixVect.end();
  for (; i != iEnd; ++i)
  {
    for (FareMarketPath* fmp : (*i)->itin()->fmpMatrix()->fareMarketPathMatrix())
      if (LIKELY(fmp->firstFareAmt(_paxType) != std::numeric_limits<MoneyAmount>::max()))
        _allFareMarketPaths.push_back(fmp);
  }

  std::sort(_allFareMarketPaths.begin(), _allFareMarketPaths.end(), FmpSort(_paxType));

  return !_allFareMarketPaths.empty();
}

//----------------------------------------------------------------------------
bool
PaxFarePathFactory::initPaxFarePathFactory(DiagCollector& diag)
{
  LOG4CXX_INFO(logger, "Entered: initPaxFarePathFactory.");

  startTimer();
  _gauss.clear();

  if (_trx->delayXpn())
  {
    if (processFareMarketPaths())
    {
      updateGaussForDelayedExpansion();
      expandNextFareMarketPath(&diag);
    }

    return !pqEmpty();
  }

  if (!pricingAxess() && !createFarePathFactories(FarePathFactoryCreator()))
    return false;

  return addFarePathFactoriesToPQ(diag);
}

bool
PaxFarePathFactory::shouldExpandNextFareMarketPath(FareMarketPath* fmp)
{
  if (_throughFarePricing && !fmp->thruPricing(_trx))
    return false;

  if (expandedSameFareBreakFmps.find(fmp) != expandedSameFareBreakFmps.end())
    return false;

  if (_trx->isAltDates())
  {
    if (fmp->puPaths().empty())
      return false;

    DatePair* datePair = fmp->puPaths().front()->itin()->datePair();
    if (_doneDatePairs.find(*datePair) != _doneDatePairs.end())
      return false;

    if ((_trx->altDateCutOffNucThreshold() > 0) &&
        (fmp->firstFareAmt(_paxType) - _trx->altDateCutOffNucThreshold() > EPSILON))
      return false;
  }

  return true;
}

void
PaxFarePathFactory::expandNextFareMarketPath(DiagCollector* diag)
{
  if (!_trx->delayXpn())
    return;

  Diag671Collector* diag671 = nullptr;
  if (_trx->diagnostic().diagnosticType() == Diagnostic671)
  {
    diag671 = dynamic_cast<Diag671Collector*>(diag);
    diag671->enable(Diagnostic671);
  }

  vector<FareMarketPath*> fmpsToExpand;
  bool nextFmpSameAmount = false;
  bool done = false;

  while (!done && (_fmpXPoint < (int16_t)_allFareMarketPaths.size()))
  {
    FareMarketPath* fmp = _allFareMarketPaths[_fmpXPoint];
    FarePathFactory* currentFpf = (pqEmpty() ? nullptr : pqTop());
    bool expandNextFmp = shouldExpandNextFareMarketPath(fmp);

    if (expandNextFmp)
      fmpsToExpand.push_back(fmp);

    if (diag671)
      diag671->displayFmp(*_trx,
                          _fmpXPoint,
                          fmp,
                          expandNextFmp,
                          nextFmpSameAmount,
                          _throughFarePricing,
                          currentFpf,
                          _paxType);

    ++_fmpXPoint;
    setExternalFmpLowerBoundAmount(fmp);

    nextFmpSameAmount = isNextFmpSameAmount(fmp);
    if (nextFmpSameAmount || (currentFpf && sameOrLowerAmountFmpExists(currentFpf)))
      continue;

    if (!fmpsToExpand.empty())
    {
      bool success =
          createFarePathFactoriesForDelayXpn(fmpsToExpand, diag) && addFarePathFactoriesToPQ(*diag);

      if (success && !sameOrLowerAmountFmpExists(pqTop()))
        done = true;

      if (diag671)
        diag671->displayExpansion(fmpsToExpand.size(),
                                  _farePathFactoryBucket.size(),
                                  (pqEmpty() ? nullptr : pqTop()),
                                  _paxType,
                                  pqSize());

      fmpsToExpand.clear();
    }
  }

  if (diag671)
    diag671->flushMsg();
}

bool
PaxFarePathFactory::createFarePathFactoriesForDelayXpn(vector<FareMarketPath*>& fmps,
                                                       DiagCollector* diag)
{
  vector<PUPath*> puPaths;

  for (FareMarketPath* fmp : fmps)
    puPaths.insert(puPaths.end(), fmp->puPaths().begin(), fmp->puPaths().end());

  initPricingUnitFactory(puPaths, *diag);

  _farePathFactoryBucket.clear();

  createFarePathFactories(FarePathFactoryCreator(), puPaths);

  return !_farePathFactoryBucket.empty();
}

void
PaxFarePathFactory::initPricingUnitFactory(std::vector<PUPath*>& puPaths, DiagCollector& diag)
{
  PricingUnitFactoryBucket fmpPufb;

  if (getPuFactoryBucketForFmp(puPaths, &fmpPufb))
  {
    std::vector<PricingUnitFactoryBucket*> puFactoryBucketVect;
    puFactoryBucketVect.push_back(&fmpPufb);

    _po->initPricingUnitFactory(*_trx, puFactoryBucketVect, diag);
  }
}

bool
PaxFarePathFactory::getPuFactoryBucketForFmp(std::vector<PUPath*>& puPaths,
                                             PricingUnitFactoryBucket* fmpPufb)
{
  std::map<PU*, PricingUnitFactory*>& allPUs = _puFactoryBucket->puFactoryBucket();

  for (PUPath* puPath : puPaths)
  {
    for (PU* pu : puPath->allPU())
    {
      PricingUnitFactory* puf = allPUs[pu];
      if (puf && !puf->initializedForDelayXpn())
      {
        fmpPufb->puFactoryBucket()[pu] = puf;
        puf->initializedForDelayXpn() = true;
      }
    }
  }

  if (!fmpPufb->puFactoryBucket().empty())
  {
    _puFactoryBucket->cloneExceptBucket(fmpPufb);
    return true;
  }

  return false;
}

bool
PaxFarePathFactory::updateNextLowerBoundFPAmount(MoneyAmount& nextLowerBoundFPAmount)
{
  if (_externalFmpLowerBoundAmount < 0)
    return false;

  bool nextLBAmountFromFMP = false;

  if (nextLowerBoundFPAmount < 0)
    nextLBAmountFromFMP = true;
  else
  {
    if (_externalFmpLowerBoundAmount < nextLowerBoundFPAmount)
      nextLBAmountFromFMP = true;
  }

  if (nextLBAmountFromFMP)
    nextLowerBoundFPAmount = _externalFmpLowerBoundAmount;

  return nextLBAmountFromFMP;
}

bool
PaxFarePathFactory::addFarePathFactoriesToPQ(DiagCollector& diag)
{
  TSELatencyData metrics(*_trx, "PO INIT PAX FP FACTORY");

  bool fpfAdded = false;

  try
  {
    std::vector<FarePathFactory*>::iterator it = _farePathFactoryBucket.begin();
    std::vector<FarePathFactory*>::iterator itEnd = _farePathFactoryBucket.end();
    for (; it != itEnd; ++it)
    {
      FarePathFactory* fpf = (*it);
      fpf->setEoeCombinabilityEnabled(isEoeCombinabilityEnabled());
      if (fpf->initFarePathFactory(diag) && (fpf->lowerBoundFPAmount() >= 0))
      {
        // negative -lowerBoundFPAmount means FPF failed to build any FP
        pqPush(fpf);

        if (UNLIKELY(!_trx->delayXpn()))
          _gauss.include(1);

        fpfAdded = true;
      }
      else
      {
        if (LIKELY(_trx->delayXpn()))
          _gauss.exclude(1);
      }
    }
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "Exception: InitFarePathFactory Failed")
    throw;
  }

  _reqDiagFPCount = fpCountDiagParam();

  if (!fpfAdded)
    return false;

  return true;
}

//----------------------------------------------------------------------------
FPPQItem*
PaxFarePathFactory::getFPPQItem(uint32_t idx, DiagCollector& diag)
{
  if (idx < _fpCount)
    return _validFPPQItemVect[idx];

  if (!buildNextLevelFarePath(diag))
    return nullptr;

  if (idx < _fpCount)
    return _validFPPQItemVect[idx];
  else
    return nullptr;
}
//----------------------------------------------------------------------------
FPPQItem*
PaxFarePathFactory::getFirstValidZeroAmountFPPQItem(uint32_t idx,
                                                    const FPPQItem& primaryFPPQItem,
                                                    DiagCollector& diag,
                                                    uint32_t* newIdx)
{
  TSELatencyData metrics(*_trx, "GET 1ST VALID 0AMT FPPQITEM");

  // check already generated items
  for (; idx < _fpCount; ++idx)
  {
    FPPQItem* fppqItem = _validFPPQItemVect[idx];
    if (fabs(fppqItem->farePath()->getTotalNUCAmount()) < EPSILON &&
        passAccompanidRestriction(primaryFPPQItem, *fppqItem) &&
        checkISICode(primaryFPPQItem, *fppqItem))
    {
      if (newIdx)
        *newIdx = idx;
      return fppqItem;
    }
  }

  const FareMarketPath* primaryFMPath = primaryFPPQItem.puPath()->fareMarketPath();

  if (_trx->delayXpn())
    expandFareMarketPath(const_cast<FareMarketPath*>(primaryFMPath), &diag);

  if (pqEmpty())
  {
    LOG4CXX_INFO(logger, _paxType->paxType() << " getFirstValidZeroAmountFPPQItem:PQ empty");
    return nullptr;
  }

  FarePathFactory* fpf = getMatchingFPF(primaryFPPQItem);
  if (!fpf)
    return nullptr;

  FPPQItem* fppqItem = fpf->getSameFareBasisFPPQItem(primaryFPPQItem, diag);
  if (!fppqItem)
    return nullptr;

  if (fabs(fppqItem->farePath()->getTotalNUCAmount()) < EPSILON &&
      passAccompanidRestriction(primaryFPPQItem, *fppqItem) &&
      checkISICode(primaryFPPQItem, *fppqItem))
  {
    ++_fpCount;
    _validFPPQItemVect.push_back(fppqItem);
    if (newIdx)
      *newIdx = _fpCount - 1;
    return fppqItem;
  }

  return nullptr;
}
//----------------------------------------------------------------------------

void
PaxFarePathFactory::setExternalFmpLowerBoundAmount(FareMarketPath* currentFmp)
{
  if (_fmpXPoint >= (int16_t)_allFareMarketPaths.size())
    _externalFmpLowerBoundAmount = -1;
  else
  {
    FareMarketPath* nextFmp = _allFareMarketPaths[_fmpXPoint];

    if (currentFmp->throughFarePrecedenceRank() != nextFmp->throughFarePrecedenceRank())
      _externalFmpLowerBoundAmount = -1;
    else
      _externalFmpLowerBoundAmount = nextFmp->firstFareAmt(_paxType);
  }
}

bool
PaxFarePathFactory::isNextFmpSameAmount(FareMarketPath* currentFmp)
{
  if (_fmpXPoint >= (int16_t)_allFareMarketPaths.size())
    return false;

  FareMarketPath* nextFmp = _allFareMarketPaths[_fmpXPoint];

  if (currentFmp->throughFarePrecedenceRank() != nextFmp->throughFarePrecedenceRank())
    return false;

  MoneyAmount delta = nextFmp->firstFareAmt(_paxType) - currentFmp->firstFareAmt(_paxType);

  if (fabs(delta) <= EPSILON)
    return true;

  return false;
}

bool
PaxFarePathFactory::sameOrLowerAmountFmpExists(const FarePathFactory* fpf)
{
  if (_externalFmpLowerBoundAmount < 0)
    return false;

  MoneyAmount delta = _externalFmpLowerBoundAmount - fpf->lowerBoundFPAmount();

  bool isLowerAmount = (delta < 0);
  if (isLowerAmount || (fabs(delta) <= EPSILON))
    return true;

  return false;
}

namespace
{
struct isFareSame : std::unary_function<const PaxTypeFare*, bool>
{
  isFareSame(const PaxTypeFare* paxTypeFare) : _paxTypeFare(paxTypeFare) {}

  bool operator()(const PaxTypeFare* paxTypeFare) const
  {
    return (paxTypeFare->vendor() == _paxTypeFare->vendor() &&
            paxTypeFare->carrier() == _paxTypeFare->carrier() &&
            paxTypeFare->fareTariff() == _paxTypeFare->fareTariff() &&
            paxTypeFare->ruleNumber() == _paxTypeFare->ruleNumber() &&
            paxTypeFare->fareClass() == _paxTypeFare->fareClass());
  }

private:
  const PaxTypeFare* _paxTypeFare;
};

struct isMatchFarePathCollected : std::unary_function<const FarePath*, bool>
{
  isMatchFarePathCollected(const FarePath* farePath, const bool isAxess)
    : _farePath(farePath), _isAxess(isAxess)
  {
  }

  bool operator()(const FarePath* farePath) const
  {
    if (_farePath->paxType() != farePath->paxType())
      return false;

    const std::vector<PricingUnit*>& units1 = _farePath->pricingUnit();
    const std::vector<PricingUnit*>& units2 = farePath->pricingUnit();

    if (units1.size() != units2.size())
      return false;

    std::vector<PricingUnit*>::const_iterator iterPU1 = units1.begin();
    std::vector<PricingUnit*>::const_iterator iterPU2 = units2.begin();

    while (iterPU1 != units1.end())
    {
      const std::vector<FareUsage*>& usages1 = (*iterPU1)->fareUsage();
      const std::vector<FareUsage*>& usages2 = (*iterPU2)->fareUsage();

      if (usages1.size() != usages2.size())
        return false;

      std::vector<FareUsage*>::const_iterator iterFU1 = usages1.begin();
      std::vector<FareUsage*>::const_iterator iterFU2 = usages2.begin();

      while (iterFU1 != usages1.end())
      {
        PaxTypeFare* noMatchFare = (*iterFU1)->paxTypeFare();
        PaxTypeFare* matchFare = (*iterFU2)->paxTypeFare();

        if (!isFareSame(noMatchFare)(matchFare))
          return false;

        // booking codes comparison - only for users other than Axess
        if (!_isAxess)
        {
          if (!isSameBookingCode(_farePath, farePath, (*iterFU1), (*iterFU2)))
            return false;
        }

        ++iterFU1;
        ++iterFU2;
      }
      ++iterPU1;
      ++iterPU2;
    }
    return true;
  }

private:
  bool isSameBookingCode(const FarePath* farePathMatch,
                         const FarePath* farePathNoMatch,
                         const FareUsage* fareUsageMatch,
                         const FareUsage* fareUsageNoMatch) const
  {
    const std::vector<TravelSeg*>& segsMatch = fareUsageMatch->travelSeg();
    const std::vector<TravelSeg*>& segsNoMatch = fareUsageNoMatch->travelSeg();

    if (segsMatch.size() != segsNoMatch.size())
      return false;

    std::vector<TravelSeg*>::const_iterator iterMatchTS = segsMatch.begin();
    std::vector<TravelSeg*>::const_iterator iterNoMatchTS = segsNoMatch.begin();
    std::vector<TravelSeg*>::const_iterator endIterTS = segsMatch.end();

    while (iterMatchTS != endIterTS)
    {
      AirSeg* segMatch = dynamic_cast<AirSeg*>(*iterMatchTS);
      AirSeg* segNoMatch = dynamic_cast<AirSeg*>(*iterNoMatchTS);

      if (segMatch && segNoMatch)
      {
        std::string bkgCodeMatch = "";
        std::string bkgCodeNoMatch = "";

        int segMatchIdx = segMatch->segmentOrder() - 1;
        int segNoMatchIdx = segNoMatch->segmentOrder() - 1;

        if (static_cast<int>(farePathMatch->bookingCodeRebook.size()) > segMatchIdx)
          bkgCodeMatch = farePathMatch->bookingCodeRebook[segMatchIdx];

        if (static_cast<int>(farePathNoMatch->bookingCodeRebook.size()) > segNoMatchIdx)
          bkgCodeNoMatch = farePathNoMatch->bookingCodeRebook[segNoMatchIdx];

        if (bkgCodeMatch.empty())
        {
          bkgCodeMatch = segMatch->getBookingCode();
        }

        if (bkgCodeNoMatch.empty())
        {
          bkgCodeNoMatch = segNoMatch->getBookingCode();
        }
        if (bkgCodeMatch != bkgCodeNoMatch)
          return false;
      }
      else if ((segMatch && !segNoMatch) || (!segMatch && segNoMatch))
      {
        return false;
      }

      ++iterMatchTS;
      ++iterNoMatchTS;
    }

    return true;
  }

  const FarePath* _farePath;
  const bool _isAxess;
};
}

bool
PaxFarePathFactory::isValidForIntegrated(const FarePath* farePath)
{
  // check ticketing agent
  bool isAxess = _trx->getRequest()->ticketingAgent()->axessUser();

  // check whether same solution has already been found in match scenario
  const std::vector<FarePath*>& farePaths = _trx->itin().front()->farePath();
  return (std::find_if(farePaths.begin(),
                       farePaths.end(),
                       isMatchFarePathCollected(farePath, isAxess)) == farePaths.end());
}

void
PaxFarePathFactory::buildFarePathsAmounts(CurrencyConversionFacade& ccFacade)
{
  uint32_t addForDups = 0;
  const std::vector<FarePath*>& farePaths = _trx->itin().front()->farePath();
  std::vector<FarePath*>::const_iterator iterEnd = farePaths.end();

  std::vector<FarePath*>::const_iterator iter = farePaths.begin();
  while (iter != iterEnd)
  {
    FarePath* farePath = *iter;
    if (farePath->paxType() == _paxType)
      checkUniqueFarePathTotals(farePath, ccFacade, addForDups, false);
    ++iter;
  }
}

//----------------------------------------------------------------------------
bool
PaxFarePathFactory::buildNextLevelFarePath(DiagCollector& diag)
{
  removeObsoleteFpfs(diag);

  if (pqEmpty())
  {
    LOG4CXX_INFO(logger, _paxType->paxType() << " buildNextLevelFarePath:PQ empty");
    return false;
  }

  int32_t genCount = 0;
  bool validFound = false;

  FarePathFactory* fpf = pqTop();
  bool tryNextFPF = true;

  CurrencyConversionFacade ccFacade;
  uint32_t addForDups = 0; // Additional FarePaths created for duplicate
  // totals elimination after taxes.
  bool isNewItinForRex =
      RexPricingTrx::isRexTrxAndNewItin(*_trx) &&
      !(_trx->getTrxType() == PricingTrx::MIP_TRX && _trx->billing()->actionCode() == "WFR") &&
      !(_trx->getTrxType() == PricingTrx::MIP_TRX && _trx->billing()->actionCode() == "WFR.C");

  bool isProcessingForAsBooked = isNewItinForRex && _fpCount;

  /*  When _trx->getRequest()->lowFareRequested() == 'Y' we process for first winner,
      as Booked or as Rebooked.
      When _trx->getRequest()->lowFareRequested() == 'N' we process for as Booked winner only
  */
  if (isProcessingForAsBooked)
    _trx->getRequest()->lowFareRequested() = 'N';

  bool isIntegrated, markDuplicates;
  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(_trx);
  if (noPNRPricingTrx)
  {
    isIntegrated = noPNRPricingTrx->integrated();
    markDuplicates = true;

    if (!_allowDuplicateTotals)
      buildFarePathsAmounts(ccFacade);
  }
  else
  {
    isIntegrated = false;
    markDuplicates = false;
  }

  try
  {
    do
    {
      // The as Rebooked solution is generated and timeout during searching the as Booked one
      if (UNLIKELY(isNewItinForRex && _fpCount && !_maxNbrCombMsgSet &&
          _fpCombTried > _factoriesConfig.maxNbrCombMsgThreshold()))
      {
        if (_trx->aborter())
          throw 0;
      }

      PricingUtil::checkTrxAborted(
          *_trx, _factoriesConfig.maxNbrCombMsgThreshold(), _fpCombTried, _maxNbrCombMsgSet);

      if ((_maxSearchNextLevelFarePath >= 0) && (genCount > _maxSearchNextLevelFarePath))
        return false;

      if (tryNextFPF)
      {
        pqPop();

        if (UNLIKELY(!checkAltDates(fpf)))
          return false;
      }

      if (UNLIKELY(isProcessingForAsBooked))
        fpf->clearFactoryForRex(diag);

      MoneyAmount nextLowerBoundFPAmount = -1;
      const FPPQItem* nextItem = nullptr;

      uint16_t nextItemThroughFarePrecedenceRank = 0;
      uint16_t currentItemThroughFarePrecedenceRank = 0;

      if (UNLIKELY(sameOrLowerAmountFmpExists(fpf)))
      {
        expandNextFareMarketPath(&diag);
        tryNextFPF = true;
        pqPush(fpf);
        fpf = pqTop();
        continue;
      }

      if (!pqEmpty())
      {
        nextLowerBoundFPAmount = pqTop()->lowerBoundFPAmount();
        if (LIKELY(nextLowerBoundFPAmount >= 0))
        {
          if (LIKELY(fpf->lowerBoundFPAmount() >= 0))
          {
            nextItemThroughFarePrecedenceRank =
                pqTop()->puPath()->fareMarketPath()->throughFarePrecedenceRank();
            currentItemThroughFarePrecedenceRank =
                fpf->puPath()->fareMarketPath()->throughFarePrecedenceRank();
          }

          if (LIKELY(nextItemThroughFarePrecedenceRank <= currentItemThroughFarePrecedenceRank))
            nextItem = pqTop()->lowerBoundFPPQItem();
          else
            nextLowerBoundFPAmount = -1;
        }
      }

      bool nextLBAmountFromFMP =
          updateNextLowerBoundFPAmount(nextLowerBoundFPAmount);
      if (nextLBAmountFromFMP)
        nextItem = nullptr;
      fpf->externalLowerBoundAmount() = nextLowerBoundFPAmount;

      FPPQItem* fppqItem = fpf->getNextFPPQItem(diag);

      ++genCount;

      if (fppqItem != nullptr)
      {
        bool isFarePathGreater = false;

        FPPQItem::GreaterLowToHigh<FPPQItem::GreaterFare> greater;
        if (nextItem && greater(fppqItem, nextItem))
          isFarePathGreater = true;

        if (!isFarePathGreater && nextLBAmountFromFMP)
          isFarePathGreater =
              fppqItem->farePath()->getTotalNUCAmount() - nextLowerBoundFPAmount > EPSILON;

        if (isFarePathGreater)
          fpf->pushBack(fppqItem);
        else
        {
          if (isFarePathValidAfterPlusUps(*fppqItem, diag))
          {
            FarePath* farePath = fppqItem->farePath();
            if (isIntegrated && !isValidForIntegrated(farePath))
              releaseFPPQItem(*fpf, fppqItem);
            else
            {
              if (!_allowDuplicateTotals)
                checkUniqueFarePathTotals(
                    fppqItem->farePath(), ccFacade, addForDups, markDuplicates);

              ++_fpCount;
              _validFPPQItemVect.push_back(fppqItem);
              validFound = true;
            }
          }
          else
          {
            releaseFPPQItem(*fpf, fppqItem);
            if (_altDateISCutOffReach)
              return false;
          }
        }
      }

      tryNextFPF = true;
      if (fpf->lowerBoundFPAmount() >= 0) // current FPF has more in its PQ
      {
        bool bcontinue(true);
        if (UNLIKELY(!_trx->isForceNoTimeout() && shutdownFPFactory(*fpf)))
        {
          LOG4CXX_INFO(logger, __FUNCTION__ << ":shortcircuit logic");
          // remove unfinished items
          bcontinue = fpf->keepValidItems() > 0;
        }

        if (LIKELY(bcontinue))
        {
          if (validFound && genCount >= _reqDiagFPCount &&
              _fpCount >= _reqValidFPCount + addForDups)
          {
            // done
            pqPush(fpf);
            return true;
          }
          else
          {
            if (nextItemThroughFarePrecedenceRank > currentItemThroughFarePrecedenceRank ||
                (nextItemThroughFarePrecedenceRank == currentItemThroughFarePrecedenceRank &&
                 nextLowerBoundFPAmount - fpf->lowerBoundFPAmount() > EPSILON))
            {
              // we want to continue with the same FPFactory
              tryNextFPF = false;
            }
            else
            {
              // we need to pop the next FPF
              pqPush(fpf);
            }
          }
        }
      }

      if (tryNextFPF)
      {
        if (_trx->delayXpn() && (nextLBAmountFromFMP || pqEmpty()))
          expandNextFareMarketPath(&diag);

        if (pqEmpty())
        {
          LOG4CXX_INFO(logger, _paxType->paxType() << " buildNextLevelFarePath: PQ empty");
          return true;
        }

        fpf = pqTop();
      }

      if (UNLIKELY(startMultiPaxShortCkt()))
      {
        if (tryNextFPF == false)
          pqPush(fpf);

        return true;
      }
    } while (!validFound || genCount < _reqDiagFPCount || _fpCount < _reqValidFPCount + addForDups);
  }
  catch (ErrorResponseException& ere)
  {
    // too many failed combos from shopping queue.
    if (ere.code() == ErrorResponseException::TOO_MANY_COMBOS)
      LOG4CXX_DEBUG(logger, "REACH MAXIMUM FAREPATH/FLIGHT FAILED COUNT FROM A SHOPPING QUEUE");

    if (isNewItinForRex && _fpCount)
    {
      // Error for as Booked
      RexPricingTrx* rexTrx = static_cast<RexPricingTrx*>(_trx);
      rexTrx->reissuePricingErrorMsg() = ere.message();
      rexTrx->reissuePricingErrorCode() = ere.code();
      return false;
    }

    throw;
  }

  catch (...)
  {
    LOG4CXX_ERROR(logger, "Exception: Build Pax-FarePath Failed or Timed-Out")
    if (isNewItinForRex && _fpCount)
    {
      // Error for as Booked
      RexPricingTrx* rexTrx = static_cast<RexPricingTrx*>(_trx);
      rexTrx->reissuePricingErrorMsg() = "MAX NBR COMBINATIONS EXCEEDED/USE SEGMENT SELECT";
      rexTrx->reissuePricingErrorCode() = ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED;
      return false;
    }

    throw;
  }

  return true;
}

//---------------------------------------------------------------
void
PaxFarePathFactory::checkUniqueFarePathTotals(FarePath* farePath,
                                              CurrencyConversionFacade& ccFacade,
                                              uint32_t& addForDups,
                                              bool markDups)
{
  std::set<MoneyAmount>::const_reverse_iterator fpTotalsIter = _uniqueFarePathTotals.rbegin();
  std::set<MoneyAmount>::const_reverse_iterator fpTotalsIterEnd = _uniqueFarePathTotals.rend();

  bool dupFound = false;

  const Itin* itin = farePath->itin();

  MoneyAmount fpTotalAmt = farePath->getTotalNUCAmount();
  CurrencyNoDec fpTotalNoDec = 2;

  if (itin->originationCurrency() != NUC)
  {
    Money fpTotal(itin->originationCurrency());
    Money fpTotalNUC(farePath->getTotalNUCAmount(), NUC);

    if (ccFacade.convert(fpTotal, fpTotalNUC, *_trx, itin->useInternationalRounding()))
    {
      fpTotalAmt = fpTotal.value();
      fpTotalNoDec = fpTotal.noDec(_trx->ticketingDate());
    }
    else
    {
      LOG4CXX_ERROR(logger, "Currency conversion error");
    }
  }

  MoneyAmount diffAmount = 1.0 / pow(10.0, fpTotalNoDec);

  for (; fpTotalsIter != fpTotalsIterEnd; ++fpTotalsIter)
  {
    if (fabs(fpTotalAmt - *fpTotalsIter) <= diffAmount)
    {
      dupFound = true;
      break;
    }
  }

  if (dupFound || (_trx->getRequest()->isWpas() && _trx->getRequest()->wpaXm() &&
                   AltPricingUtil::isRBDasBooked(*_trx, farePath)))
  {
    ++addForDups;
    if (markDups)
      farePath->duplicate() = true;
  }
  else
  {
    _uniqueFarePathTotals.insert(fpTotalAmt);
  }
}

//---------------------------------------------------------------
bool
PaxFarePathFactory::shutdownFPFactory(FarePathFactory& fpf)
{
  if (fpf.shutdown())
  {
    return false; // already shutdown, do not re-call keepValidItems again
  }

  if (_validFPPushedBack && _shortCktShutdownFPFsTime && (time(nullptr) >= _shortCktShutdownFPFsTime))
  {
    LOG4CXX_WARN(logger, "Almost timeout: Shutting down all fare path factories");
    _pricingShortCktHappened = true;
    return true;
  }

  const uint32_t curCount = fpf.fpCombTried();

  _gauss.exclude(curCount - 1); // remove
  _gauss.include(curCount);

  if ((curCount > SHORT_CKT_COMB_COUNT) && (curCount - _gauss.min() > 3 * _gauss.stddev()))
  {
    if (time(nullptr) - _startTime > _shortCktTimeOut)
    {
      LOG4CXX_WARN(logger, "Short-Ckt Timeout Passed With No Success: Will Try Different FPF")
      return true;
    }
  }

  return false;
}
//-------------------------------------------------------------------------
bool
PaxFarePathFactory::createFarePathFactories(const FarePathFactoryCreator& creator)
{
  std::vector<PUPathMatrix*>::iterator i = _puPathMatrixVect.begin();
  const std::vector<PUPathMatrix*>::iterator iEnd = _puPathMatrixVect.end();
  for (; i != iEnd; ++i)
  {
    PUPathMatrix& matrix = *(*i);

    createFarePathFactories(creator, matrix.puPathMatrix());
  }

  if (_farePathFactoryBucket.empty())
  {
    LOG4CXX_INFO(logger, "farePathFactoryBucket Empty");
    return false;
  }

  return true;
}

void
PaxFarePathFactory::createFarePathFactories(const FarePathFactoryCreator& creator,
                                            std::vector<PUPath*>& puPaths)
{
  std::vector<PUPath*>::iterator it = puPaths.begin();
  const std::vector<PUPath*>::iterator itEnd = puPaths.end();
  for (; it != itEnd; ++it)
  {
    PUPath* puPath = *it;

    if (UNLIKELY(_trx->getRequest()->isChangeSoldoutBrand()))
    {
      BrandCode brandCode = puPath->getBrandCode();
      if (ShoppingUtil::isThisBrandReal(brandCode))
      {
        Itin* itin = puPath->itin();
        // Don't return itineraries that are soldout on every leg (total soldout).
        // At least one leg must have the desired brand.
        if (!itin->getIbfAvailabilityTracker().atLeastOneLegValidForBrand(brandCode))
          continue;
      }
    }

    std::vector<SavedFPPQItem>* savedFPPQItemsPtr =
        !puPath->itin()->getSimilarItins().empty() ? &_savedFPPQItems : nullptr;

    FarePathFactory* const fpFactory = FarePathFactory::createFarePathFactory(creator,
                                                                              puPath,
                                                                              _puFactoryBucket,
                                                                              puPath->itin(),
                                                                              this,
                                                                              savedFPPQItemsPtr);

    _farePathFactoryBucket.push_back(fpFactory);
  }
}

//---------------------------------------------------------------------------
uint32_t
PaxFarePathFactory::fpCountDiagParam()
{
  if (LIKELY(_trx->diagnostic().diagnosticType() == DiagnosticNone))
    return 0;

  const DiagnosticTypes diagType = _trx->diagnostic().diagnosticType();
  if ((diagType >= Diagnostic601 && diagType <= Diagnostic690 && diagType != Diagnostic666) ||
      diagType == Diagnostic910 ||
      diagType == Diagnostic413 ||
      diagType == Diagnostic420)
  {
    const std::map<std::string, std::string>& dgParamMap = _trx->diagnostic().diagParamMap();
    const auto it = dgParamMap.find("FP");

    if (it != dgParamMap.end())
      return atoi(it->second.c_str());
  }

  return 0;
}

//---------------------------------------------------------------
void
PaxFarePathFactory::releaseFPPQItem(FarePathFactory& fpf, FPPQItem* fppqItem)
{
  TSE_ASSERT(fppqItem->farePathFactory() == &fpf);
  if (UNLIKELY(!_trx->isShopping()))
  {
    // do not release FPPQItem for IS because it uses pointers to FPPQItem,
    // see PaxFarePathFactory::_failedFarePathIS
    fpf.releaseFPPQItem(fppqItem);
  }
}

//---------------------------------------------------------------
bool
PaxFarePathFactory::checkAltDates(FarePathFactory*& fpFactory)
{
  if (!_trx->isAltDates())
    return true;

  if (_trx->getTrxType() != PricingTrx::MIP_TRX)
    return true;

  if (_trx->altDatePairs().size() <= 1) // Single Date Alt-Date Trx
    return true;

  // Multi date MIP
  if (UNLIKELY(_trx->altDateCutOffNucThreshold() > 0 &&
                (fpFactory->lowerBoundFPAmount() - _trx->altDateCutOffNucThreshold() > EPSILON)))
  {
    LOG4CXX_DEBUG(logger, " Too expensive:")

    // CutOff reached
    _trx->setCutOffReached();
    return false;
  }

  return true;
}

void
PaxFarePathFactory::expandFareMarketPath(FareMarketPath* fareMarketPath, DiagCollector* diag)
{
  if (_fmpXPoint >= (int16_t)_allFareMarketPaths.size())
    return; // no FMP to expand

  for (uint16_t i = 0; i < _fmpXPoint; ++i)
    if (_allFareMarketPaths[i] == fareMarketPath)
      return; // already expanded

  vector<FareMarketPath*> fmpsToExpand;
  fmpsToExpand.push_back(fareMarketPath);

  if (createFarePathFactoriesForDelayXpn(fmpsToExpand, diag))
    addFarePathFactoriesToPQ(*diag);

  expandedSameFareBreakFmps.insert(fareMarketPath);
}

//----------------------------------------------------------------------------
FPPQItem*
PaxFarePathFactory::getSameFareBreakFPPQItem(const FPPQItem* primaryFPPQItem, DiagCollector& diag)
{
  const FareMarketPath* fareMarketPath = primaryFPPQItem->puPath()->fareMarketPath();
  std::vector<FPPQItem*>::iterator it = _validFPPQItemVect.begin();
  std::vector<FPPQItem*>::iterator itEnd = _validFPPQItemVect.end();
  for (; it != itEnd; ++it)
  {
    if (passAccompanidRestriction(*primaryFPPQItem, **it) && checkISICode(*primaryFPPQItem, **it))
      return *it;
  }

  if (_trx->delayXpn())
    expandFareMarketPath(const_cast<FareMarketPath*>(fareMarketPath), &diag);

  if (pqEmpty())
    return nullptr;

  if (_paxType->paxType() == INFANT && primaryFPPQItem)
  {
    FarePathFactory* fpf = getMatchingFPF(*primaryFPPQItem);
    if (fpf)
    {
      FPPQItem* fppqItem = fpf->getSameFareBasisFPPQItem(*primaryFPPQItem, diag);
      if (fppqItem)
      {
        if (passAccompanidRestriction(*primaryFPPQItem, *fppqItem) &&
            checkISICode(*primaryFPPQItem, *fppqItem))
        {
          return fppqItem;
        }
      }
    }
  }

  std::vector<FarePathFactory*> diffFareBreakFPF;
  separateDiffFareBreakFPF(fareMarketPath, diffFareBreakFPF);

  if (pqEmpty())
    return nullptr;

  int32_t genCount = 0;
  bool validFound = false;

  FarePathFactory* fpf = pqTop();
  bool tryNextFPF = true;

  try
  {
    do
    {
      PricingUtil::checkTrxAborted(
          *_trx, _factoriesConfig.maxNbrCombMsgThreshold(), _fpCombTried, _maxNbrCombMsgSet);

      if (tryNextFPF)
      {
        pqPop();

        if (UNLIKELY(!checkAltDates(fpf)))
          return nullptr;
      }

      MoneyAmount nextLowerBoundFPAmount = -1;
      if (!pqEmpty())
        nextLowerBoundFPAmount = pqTop()->lowerBoundFPAmount();

      fpf->externalLowerBoundAmount() = nextLowerBoundFPAmount;

      FPPQItem* fppqItem = fpf->getNextFPPQItem(diag);

      ++genCount;

      if (fppqItem != nullptr)
      {
        if ((fppqItem->farePath()->getTotalNUCAmount() - nextLowerBoundFPAmount > EPSILON) &&
            nextLowerBoundFPAmount >= 0)
        {
          fpf->pushBack(fppqItem);
        }
        else
        {
          if (isFarePathValidAfterPlusUps(*fppqItem, diag))
          {
            ++_fpCount;
            _validFPPQItemVect.push_back(fppqItem);
            if (passAccompanidRestriction(*primaryFPPQItem, *fppqItem) &&
                checkISICode(*primaryFPPQItem, *fppqItem))
            {
              if (!_trx->delayXpn())
                removeFPF(&diag, fareMarketPath);
              addFPF(diffFareBreakFPF);

              if (_trx->delayXpn() && pqEmpty())
                expandNextFareMarketPath(&diag);

              return fppqItem;
            }
          }
          else
          {
            releaseFPPQItem(*fpf, fppqItem);
          }
        }
      }

      tryNextFPF = true;
      if (fpf->lowerBoundFPAmount() >= 0) // current FPF has more in its PQ
      {
        bool bcontinue(true);
        if (UNLIKELY(!_trx->isForceNoTimeout() && shutdownFPFactory(*fpf)))
        {
          LOG4CXX_INFO(logger, __FUNCTION__ << ":shortcircuit logic");
          // remove unfinished items
          bcontinue = fpf->keepValidItems() > 0;
        }

        if (LIKELY(bcontinue))
        {
          if (nextLowerBoundFPAmount - fpf->lowerBoundFPAmount() > EPSILON)
          {
            // we want to continue with the same FPFactory
            tryNextFPF = false;
          }
          else
          {
            // we need to pop the next FPF
            pqPush(fpf);
          }
        }
      }

      if (tryNextFPF)
      {
        if (UNLIKELY(pqEmpty()))
        {
          LOG4CXX_INFO(logger, _paxType->paxType() << " buildNextLevelFarePath: PQ empty");
          addFPF(diffFareBreakFPF);
          return nullptr;
        }

        fpf = pqTop();
      }
    } while (!validFound || genCount < _reqDiagFPCount);
  }
  catch (ErrorResponseException& ere)
  {
    // too many failed combos from shopping queue.
    if (ere.code() == ErrorResponseException::TOO_MANY_COMBOS)
      LOG4CXX_DEBUG(logger, "REACH MAXIMUM FAREPATH/FLIGHT FAILED COUNT FROM A SHOPPING QUEUE");

    throw;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "Exception: Build Pax-FarePath Failed or Timed-Out")
    throw;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------------------------------

FarePathFactory*
PaxFarePathFactory::getMatchingFPF(const FPPQItem& primaryFPPQItem)
{
  FarePathFactory* fpf = nullptr;
  std::vector<FarePathFactory*> tmpVect;

  while (!pqEmpty())
  {
    if (primaryFPPQItem.farePathFactory()->puPath() == pqTop()->puPath())
    {
      fpf = pqTop();
      break;
    }

    FarePathFactory* topFPF = pqTop();
    pqPop();
    tmpVect.push_back(topFPF);
  }

  std::vector<FarePathFactory*>::iterator it = tmpVect.begin();
  std::vector<FarePathFactory*>::iterator itEnd = tmpVect.end();
  for (; it != itEnd; ++it)
    pqPush(*it);

  return fpf;
}

//-----------------------------------------------------------------------------------------------------
void
PaxFarePathFactory::separateDiffFareBreakFPF(const FareMarketPath* fareMarketPath,
                                             std::vector<FarePathFactory*>& diffFareBreakFPF)
{
  if (fareMarketPath == nullptr)
    return;

  std::vector<FarePathFactory*> tmpVect;

  while (!pqEmpty())
  {
    FarePathFactory* fpf = pqTop();
    pqPop();

    if (fareMarketPath == fpf->puPath()->fareMarketPath())
      tmpVect.push_back(fpf);
    else
      diffFareBreakFPF.push_back(fpf);
  }

  std::vector<FarePathFactory*>::iterator it = tmpVect.begin();
  std::vector<FarePathFactory*>::iterator itEnd = tmpVect.end();
  for (; it != itEnd; ++it)
  {
    (*it)->multiPaxShortCktStarted() = true;
    pqPush(*it);
  }
}

void
PaxFarePathFactory::removeObsoleteFpfs(DiagCollector& diag)
{
  if (!_trx->delayXpn())
    return;

  if (_throughFarePricing)
    removeFPF(&diag, nullptr, nullptr, true);
  else if (_trx->isAltDates())
  {
    PricingTrx::AltDatePairs::const_iterator i = _trx->altDatePairs().begin();
    PricingTrx::AltDatePairs::const_iterator iEnd = _trx->altDatePairs().end();
    for (; i != iEnd; ++i)
    {
      if (i->second->numOfSolutionNeeded == 0)
      {
        std::pair<std::set<DatePair>::iterator, bool> p = _doneDatePairs.insert(i->first);
        if (p.second)
          removeFPF(&diag, nullptr, &(i->first));
      }
    }
  }

  if (pqEmpty())
    expandNextFareMarketPath(&diag);
}

//-----------------------------------------------------------------------------------------------------
void
PaxFarePathFactory::removeFPF(DiagCollector* diag,
                              const FareMarketPath* fareMarketPath,
                              const DatePair* datePair,
                              bool nonThruPricing)
{
  if (!fareMarketPath && !datePair && !nonThruPricing)
    return;

  Diag671Collector* diag671 = nullptr;
  if (_trx->diagnostic().diagnosticType() == Diagnostic671)
  {
    diag671 = dynamic_cast<Diag671Collector*>(diag);
    diag671->enable(Diagnostic671);
  }

  std::vector<FarePathFactory*> tmpVect;

  while (!pqEmpty())
  {
    FarePathFactory* fpf = pqTop();
    pqPop();

    bool match = (fareMarketPath && (fareMarketPath == fpf->puPath()->fareMarketPath())) ||
                 (datePair && (*datePair == *(fpf->itin()->datePair()))) ||
                 (nonThruPricing && !(fpf->puPath()->fareMarketPath()->thruPricing(_trx)));

    if (!match)
      tmpVect.push_back(fpf);
    else
    {
      if (diag671 && _trx->delayXpn())
        diag671->removeFpfDiag(datePair, nonThruPricing, fpf);

      fpf->clearAndReleaseMemory();
    }
  }

  std::vector<FarePathFactory*>::iterator it = tmpVect.begin();
  std::vector<FarePathFactory*>::iterator itEnd = tmpVect.end();
  for (; it != itEnd; ++it)
    pqPush(*it);
}

//-----------------------------------------------------------------------------------------------------
void
PaxFarePathFactory::addFPF(std::vector<FarePathFactory*>& diffFareBreakFPF)
{
  std::vector<FarePathFactory*>::iterator it = diffFareBreakFPF.begin();
  std::vector<FarePathFactory*>::iterator itEnd = diffFareBreakFPF.end();
  for (; it != itEnd; ++it)
  {
    pqPush(*it);
  }
}
//-----------------------------------------------------------------------------------------------------
bool
PaxFarePathFactory::passAccompanidRestriction(const FPPQItem& primaryFPPQItem,
                                              const FPPQItem& fppqItem)
{

  if (primaryFPPQItem.puPath()->fareMarketPath() == fppqItem.puPath()->fareMarketPath())
  {
    const FarePath& farePath = *fppqItem.farePath();
    if (PaxTypeUtil::isInfant(
            _trx->dataHandle(), farePath.paxType()->paxType(), farePath.paxType()->vendorCode()))
    {
      if (_accompaniedTravel.validateInfFares(*_trx, farePath, *primaryFPPQItem.farePath()))
      {
        return true;
      }
    }
    else
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------------------------------
bool
PaxFarePathFactory::checkISICode(const FPPQItem& primaryFPPQItem, const FPPQItem& fppqItem)
{

  if (LIKELY(primaryFPPQItem.farePath()->intlSaleIndicator() == fppqItem.farePath()->intlSaleIndicator()))
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------------------------------
bool
PaxFarePathFactory::startMultiPaxShortCkt()
{
  if (_trx->getTrxType() == PricingTrx::IS_TRX || _trx->altTrxType() == PricingTrx::WPA ||
      _trx->altTrxType() == PricingTrx::WP_NOMATCH || _trx->isBrandsForTnShopping())
  {
    // MultiPaxShortCkt logic is Not for Shopping IS or
    // for pricing-trx where GroupFarePath is not built
    return false;
  }

  if (_primaryPaxType)
    return false;

  if (_trx->paxType().size() <= 1)
    return false;

  if ((time(nullptr) - _startTime) > _factoriesConfig.multiPaxShortCktTimeOut())
    return true;

  return false;
}

//-----------------------------------------------------------------------------------------------------
void
PaxFarePathFactory::saveFailedFPIS(FPPQItem* fppqItem)
{
  if (_trx->getTrxType() != PricingTrx::IS_TRX)
  {
    return;
  }

  FAIL_FARE_PATH_MAP_IS::iterator it = _failedFarePathIS.find(fppqItem);
  if (it == _failedFarePathIS.end())
  {
    std::set<DatePair> newSet; // empty set for regular IS
    _failedFarePathIS.insert(FAIL_FARE_PATH_MAP_IS::value_type(fppqItem, newSet));
  }
}
//-----------------------------------------------------------------------------------------------------
void
PaxFarePathFactory::saveFailedFPIS(FPPQItem* fppqItem, const DatePair& datePair)
{
  if (_trx->getTrxType() != PricingTrx::IS_TRX)
  {
    return;
  }

  FAIL_FARE_PATH_MAP_IS::iterator it = _failedFarePathIS.find(fppqItem);
  if (it != _failedFarePathIS.end())
  {
    it->second.insert(datePair);
  }
  else
  {
    std::set<DatePair> newSet;
    newSet.insert(datePair);
    _failedFarePathIS.insert(FAIL_FARE_PATH_MAP_IS::value_type(fppqItem, newSet));
  }
}
//-----------------------------------------------------------------------------------------------------
bool
PaxFarePathFactory::checkFailedFPIS(FPPQItem* fppqItem,
                                    std::set<DatePair>& datePairSet,
                                    bool firstFP)
{
  FAIL_FARE_PATH_MAP_IS::iterator it = _failedFarePathIS.find(fppqItem);
  if (LIKELY(it == _failedFarePathIS.end()))
  {
    return true;
  }

  if (!_trx->isAltDates())
  {
    return false;
  }

  bool valid = false;
  PricingTrx::AltDatePairs::const_iterator aIt = _trx->altDatePairs().begin();
  PricingTrx::AltDatePairs::const_iterator aItEnd = _trx->altDatePairs().end();
  for (; aIt != aItEnd; ++aIt)
  {
    std::set<DatePair>::const_iterator j = it->second.find(aIt->first);
    if (j != it->second.end())
    {
      continue;
    }

    if (aIt->second->numOfSolutionNeeded > 0)
    {
      if (firstFP)
      {
        datePairSet.insert(aIt->first);
      }
      else if (datePairSet.count(aIt->first) != 0)
      {
        valid = true;
        break;
      }
    }
  }

  return valid;
}

} // namespace tse
