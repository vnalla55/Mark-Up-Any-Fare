//-------------------------------------------------------------------
// File:    FarePathCollector.cpp
// Created: January 2006
// Authors: Andrew Ahmad
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

#include "Pricing/FarePathCollector.h"

#include "Common/AltPricingUtil.h"
#include "Common/Config/ConfigMan.h" // sfc
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/NoPNRFareTypeGroup.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/NoPNROptionsSeg.h"
#include "Diagnostic/Diag611Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"

#include <utility>
namespace tse
{
namespace
{
Logger
logger("atseintl.Pricing.FarePathCollector");
}

class FarePathCollectorTask : public TseCallableTrxTask
{
public:
  void performTask() override;

  PaxFarePathFactory*& pfpf() { return _pfpf; }
  DiagCollector*& diag() { return _diag; }
  std::vector<FarePath*>& farePaths() { return _farePaths; }

  void preparePricingProcessForAxess(PricingTrx& trx, FPPQItem& fppqItem, DiagCollector& diag);

private:
  PaxFarePathFactory* _pfpf = nullptr;
  DiagCollector* _diag = nullptr;
  std::vector<FarePath*> _farePaths;
};

void
FarePathCollectorTask::preparePricingProcessForAxess(PricingTrx& trx,
                                                     FPPQItem& fppqItem,
                                                     DiagCollector& diag)

{
  FarePath& farePath = *fppqItem.farePath();

  if (!farePath.selectedNetRemitFareCombo())
    return;

  // 1.Replace PaxTypeFares in MergedFareMarket from the original
  //   Cat35 paxTypeFares (FP->PU->FU->FareMarket form TRX) the AXESS repricing for each requested
  // PaxType.
  uint32_t posPU = 0;

  PUPath* puPath = fppqItem.puPath();
  std::vector<PU*>::iterator it = puPath->allPU().begin();
  std::vector<PU*>::iterator itEnd = puPath->allPU().end();

  for (; it != itEnd; ++it, ++posPU)
  {
    const std::vector<PricingUnit*>& pricingUnitOrig = farePath.pricingUnit();

    PricingUnit* pUnit = pricingUnitOrig[posPU]; // Get the original PU
    uint32_t posFU = 0;

    PU* pu = *it;
    std::vector<MergedFareMarket*>::iterator mfmIt = pu->fareMarket().begin();
    std::vector<MergedFareMarket*>::iterator mfmItEnd = pu->fareMarket().end();
    for (; mfmIt != mfmItEnd; ++mfmIt, ++posFU)
    {
      std::vector<FareUsage*>& fuOrig = pUnit->fareUsage(); // get original FU's

      FareUsage* fu = fuOrig[posFU];

      MergedFareMarket* mfMarket = *mfmIt;
      mfMarket->cxrFarePreferred() = false;
      FareMarket* fm = mfMarket->mergedFareMarket().front(); // 1st FMarket
      PaxTypeBucket* paxTypeCortege = fm->paxTypeCortege(farePath.paxType());
      paxTypeCortege->paxTypeFare().clear();

      paxTypeCortege->paxTypeFare() = fu->selectedPTFs();

      if (mfMarket->mergedFareMarket().size() > 1)
      {
        // clear all other FareMarket's

        std::vector<FareMarket*>::iterator iFM = mfMarket->mergedFareMarket().begin();
        std::vector<FareMarket*>::iterator iFMEnd = mfMarket->mergedFareMarket().end();
        mfMarket->mergedFareMarket().erase(iFM + 1, iFMEnd);
      }
    }
  }

  // clear FarePathFactory
  FarePathFactory* fpf = fppqItem.farePathFactory();
  fpf->pricingAxess() = true;
  fpf->clear();

  // clear & init PricingUnitFactory
  for (const auto pricingUnitFactory : fpf->allPUF())
  {
    pricingUnitFactory->clear();
    pricingUnitFactory->initPricingUnitPQ(diag);
  }

  // clear and init PaxFarePathFactory

  _pfpf->clear();
  _pfpf->farePathFactoryBucket().clear();
  _pfpf->farePathFactoryBucket().push_back(fpf);
  _pfpf->initPaxFarePathFactory(diag);
}

void
FarePathCollectorTask::performTask()
{
  if (logger)
    LOG4CXX_DEBUG(logger, "Starting thread for Pax: " << _pfpf->paxType()->paxType());

  uint32_t fpCount = 24; //_pfpf.fpCount(); fpCount will be zero until
  // after we call getFPPQItem the first time.

  uint16_t topThroughFarePrecedenceRank = 0;
  FPPQItem* fppqItemTop = _pfpf->getFPPQItem(0, *_diag);

  if (fppqItemTop)
    topThroughFarePrecedenceRank =
        fppqItemTop->farePathFactory()->puPath()->fareMarketPath()->throughFarePrecedenceRank();

  for (uint32_t idx = 0; idx < fpCount; ++idx)
  {
    FPPQItem* fppqItem = _pfpf->getFPPQItem(idx, *_diag);

    fpCount = _pfpf->fpCount();

    if (trx()->diagnostic().diagnosticType() == Diagnostic372 && !trx()->diagnostic().isActive())
    {
      trx()->diagnostic().activate();
    }

    if (fppqItem)
    {
      FarePath* fp = fppqItem->farePath();

      const uint16_t fppqItemThroughFarePrecedenceRank =
          fppqItem->farePathFactory()->puPath()->fareMarketPath()->throughFarePrecedenceRank();
      if (topThroughFarePrecedenceRank &&
          fppqItemThroughFarePrecedenceRank > topThroughFarePrecedenceRank)
        break;

      if (fppqItem->isValid())
      {
        if (PricingUtil::finalPricingProcess(*trx(), *fp))
        {
          _farePaths.push_back(fp);

          bool doIt = false; // temporary hold this fix for JalAxess...
          if (doIt && fp && (*trx()).getRequest()->ticketingAgent()->axessUser() &&
              (*trx()).getRequest()->isWpNettRequested())
          {
            if (fp->selectedNetRemitFareCombo())
            {

              preparePricingProcessForAxess(*trx(), *fppqItem, *_diag);
              FPPQItem* fppqItemAxess = nullptr;

              fppqItemAxess = _pfpf->getFPPQItem(0, *_diag);

              if (fppqItemAxess != nullptr)
              {
                bool rc = PricingUtil::finalFPathCreationForAxess(
                    *trx(), *fp, *(fppqItemAxess->farePath()));
                if (!rc)
                {
                  LOG4CXX_INFO(
                      logger,
                      "FINAL AXESS FAREPATH WAS CREATED, PAXTYPE: " << fp->paxType()->paxType());
                }
              }
              else
              {
                if (fp->axessFarePath() == nullptr && fp->collectedNegFareData() != nullptr)
                {
                  fp->collectedNegFareData()->trailerMsg() =
                      "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
                }
              }
            }
            else
            {
              fp->collectedNegFareData()->trailerMsg() =
                  "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
            }
          }
        }
      }
    }
  }

  if (logger)
    LOG4CXX_DEBUG(logger, "Finishing thread for Pax: " << _pfpf->paxType()->paxType());
}

bool
FarePathCollector::process(const std::vector<PaxFarePathFactory*>& paxFarePathFactoryBucket,
                           AltPricingTrx& trx,
                           PUPathMatrix& puMatrix,
                           DiagCollector& diag)
{
  LOG4CXX_INFO(logger, "Entered FarePathCollector::process()");


  bool validFarePathFound = false;

  bool diagActive = false;
  if (trx.diagnostic().diagnosticType() == Diagnostic611)
  {
    diagActive = true;
    diag.enable(&puMatrix, Diagnostic611);
  }

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  DataHandle dataHandle(trx.ticketingDate());
  TseRunnableExecutor pooledExecutor(TseThreadingConst::FAREPATHCOLLECTOR_TASK);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

  uint16_t numTasks = paxFarePathFactoryBucket.size();
  std::vector<FarePathCollectorTask*> tasks;
  tasks.reserve(numTasks);

  std::vector<PaxFarePathFactory*>::const_iterator iter = paxFarePathFactoryBucket.begin();
  std::vector<PaxFarePathFactory*>::const_iterator iterEnd = paxFarePathFactoryBucket.end();
  for (int16_t taskCtr = 1; iter != iterEnd; ++iter, ++taskCtr)
  {
    PaxFarePathFactory* pfpf = *iter;

    FarePathCollectorTask* task = &dataHandle.safe_create<FarePathCollectorTask>();

    task->trx(&trx);
    task->pfpf() = pfpf;
    task->diag() = &diag;

    tasks.push_back(task);

    if (taskCtr < numTasks)
    {
      LOG4CXX_DEBUG(logger, "Using thread pool. Task counter: " << taskCtr);
      pooledExecutor.execute(*task);
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Using synchronous executor. Task counter: " << taskCtr);
      synchronousExecutor.execute(*task);
    }
  }

  LOG4CXX_DEBUG(logger, "Waiting for threads to finish...");

  pooledExecutor.wait();

  LOG4CXX_DEBUG(logger, "All threads finished. Collecting results...");

  std::vector<bool> paxOptions(tasks.size(), false);

  std::vector<FarePathCollectorTask*>::iterator taskIter = tasks.begin();
  std::vector<FarePathCollectorTask*>::iterator taskIterEnd = tasks.end();

  std::vector<Diag611Info> diagInfoVec(tasks.size());
  bool displayIssueWarning = false;

  bool displayDiag = (trx.diagnostic().diagnosticType() == Diagnostic370 ||
                      trx.diagnostic().diagnosticType() == Diagnostic372) &&
                     trx.diagnostic().isActive();

  for (uint16_t index = 0; taskIter != taskIterEnd; ++taskIter, ++index)
  {
    FarePathCollectorTask* task = *taskIter;
    PaxFarePathFactory* pfpf = task->pfpf();

    bool validFarePathFoundForPaxType = false;

    if (!task->farePaths().empty())
    {
      validFarePathFound = true;
      validFarePathFoundForPaxType = true;
      paxOptions[index] = true;
    }

    // check max WPA options for 1S user during
    // WPA Match and WPA No-Match
    if (trx.isLowestFareOverride() && trx.altTrxType() == AltPricingTrx::WPA && !noPNRTrx)
    {
      if (!enoughOptions(trx, task->farePaths(), paxOptions, index))
      {
        validFarePathFoundForPaxType = true;
      }
    }

    bool wqNoMatch = false;

    int uniqueSolutions = 0;
    int solutions = 0;

    if (noPNRTrx)
    {
      const std::vector<FarePath*>& farePaths = task->farePaths();
      solutions = farePaths.size();
      uniqueSolutions =
          solutions - count_if(farePaths.begin(), farePaths.end(), FarePath::isDuplicate());
      noPNRTrx->solutions().found(pfpf->paxType(), uniqueSolutions, true);
      wqNoMatch = noPNRTrx->isNoMatch();
    }

    if (diagActive)
    {
      diagInfoVec[index].validFarePathFoundForPaxType = validFarePathFoundForPaxType;
      diagInfoVec[index].dupResCnt = solutions;
      diagInfoVec[index].noDupResCnt = uniqueSolutions;
      diagInfoVec[index].pfpf = pfpf;
    }

    for (const auto fp : task->farePaths())
    {
      MaximumPenaltyValidator(trx).completeResponse(*fp);

      if (noPNRTrx)
        fp->noMatchOption() = wqNoMatch;

      puMatrix.itin()->farePath().push_back(fp);

      if (diagActive)
        diagInfoVec[index].farePaths.push_back(fp);

      if (trx.getRequest()->ticketingAgent()->axessUser() &&
          trx.getRequest()->isWpNettRequested() && fp->axessFarePath() != nullptr)
      {
        fp->axessFarePath()->itin()->farePath().push_back(fp->axessFarePath());
      }
    }

    if (displayDiag)
    {
      displayIssueWarning = false;

      for (const auto fPath : task->farePaths())
      {
        if (fPath->ignoreSurfaceTPM())
        {
          if (fPath->intlSurfaceTvlLimit())
          {
            displayIssueWarning = true;
          }
          else
          {
            displayIssueWarning = false;
            break;
          }
        }
      }
    }
  }

  if (displayDiag)
  {
    if (!trx.noPNRPricing() && trx.altTrxType() == PricingTrx::WP)
    {
      const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

      if (fcConfig != nullptr)
      {
        if (fcConfig->wpNoMatchPermitted() == 'Y')
          displayIssueWarning = false;
      }
    }

    if (displayIssueWarning)
    {
      DiagManager diagManager(trx, trx.diagnostic().diagnosticType());
      diagManager << "ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED\n";
    }
  }

  LOG4CXX_DEBUG(logger, "Finished collecting results.");

  // WPA nomatch path or WP nomatch path or WPA'XM
  if (noPNRTrx || (trx.isLowestFareOverride() && trx.altTrxType() == AltPricingTrx::WPA))
  {
    bool found = false;
    validFarePathFound = true; // make sure it's true before start checking
    for (uint16_t i = 0; i < paxOptions.size(); ++i)
    {
      if (!paxOptions[i])
      {
        validFarePathFound = false;
      }
      else
      {
        found = true;
      }
    }
    if (!validFarePathFound && found && trx.getRequest()->isLowFareRequested())
    {
      validFarePathFound = true;
    }
  }

  if (true == diagActive)
    build611Diagnostic(trx, diagInfoVec, diag);

  if (!validFarePathFound)
  {
    LOG4CXX_INFO(logger, "Leaving FarePathCollector::process(). No valid FarePath");
    return false;
  }

  LOG4CXX_INFO(logger, "Leaving FarePathCollector::process()");
  return true;
}

void
FarePathCollector::build611Diagnostic(AltPricingTrx& trx,
                                      std::vector<Diag611Info>& diagInfoVec,
                                      DiagCollector& diag)
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  std::vector<Diag611Info>::iterator infoIter = diagInfoVec.begin();
  std::vector<Diag611Info>::iterator infoIterEnd = diagInfoVec.end();

  for (uint16_t index = 0; infoIter != infoIterEnd; ++infoIter, ++index)
  {
    Diag611Info& diagInfo = *infoIter;
    PaxFarePathFactoryBase* pfpf = diagInfo.pfpf;


    if (!noPNRTrx || (noPNRTrx && !diagInfo.validFarePathFoundForPaxType))
    {

      diag.printLine();
      diag << "*********  RESULTS FOR PAX TYPE: " << pfpf->paxType()->paxType()
           << "               ************\n";
    }

    bool noDuplicate = false;

    if (noPNRTrx)
    {
      printNoPNRDiag(*noPNRTrx, diag, diagInfoVec[index]);

      // NoPNRPricingTrx::FareTypes::FTGroup fareTypeGroup = NoPNRPricingTrx::FareTypes::FTG_NONE;

      const Diagnostic& diagnostic = trx.diagnostic();
      const std::map<std::string, std::string>& diagParamMap = diagnostic.diagParamMap();
      std::map<std::string, std::string>::const_iterator endIter = diagParamMap.end();
      std::map<std::string, std::string>::const_iterator currIter;

      currIter = diagParamMap.find(Diagnostic::NO_DUPLICATE);

      if (currIter != endIter)
      {
        noDuplicate = true;
      }
    }

    uint32_t fpCounter = 1;

    for (const auto fp : diagInfo.farePaths)
    {
      if (noDuplicate && fp->duplicate())
        continue;

      diag.enable(fp, Diagnostic611);
      diag.printLine();
      const int width = diag.width(6);
      diag << std::setfill('0');
      diag << fpCounter;
      diag << std::setfill(' ');
      diag << " - ";
      diag.width(width);
      diag.setf(std::ios::left, std::ios::adjustfield);
      diag << (*fp);

      if (fp->plusUpFlag())
        diag << "PLUS-UP ADDED \n";

      diag.flushMsg();
      ++fpCounter;
    }

    if (!diagInfoVec[index].validFarePathFoundForPaxType)
    {
      diag.enable(pfpf->puPathMatrixVect().front(), Diagnostic611);
      diag << "****  NO VALID FAREPATH FOUND FOR PAX TYPE: " << pfpf->paxType()->paxType()
           << "  ****\n";

      if (noPNRTrx)
      {
        diag << "               NO COMBINABLE VALID FARE FOUND " << std::endl;
      }
    }
  }
}

void
FarePathCollector::printNoPNRDiag(NoPNRPricingTrx& trx, DiagCollector& diag, Diag611Info& diagInfo)
{
  NoPNRPricingTrx::FareTypes::FTGroup fareTypeGroup = NoPNRPricingTrx::FareTypes::FTG_NONE;

  const Diagnostic& diagnostic = trx.diagnostic();
  const std::map<std::string, std::string>& diagParamMap = diagnostic.diagParamMap();
  auto endIter = diagParamMap.end();
  auto currIter = diagParamMap.find(Diagnostic::FARE_TYPE_DESIGNATOR);

  if (currIter != endIter && !currIter->second.empty())
    fareTypeGroup = trx.mapFTtoFTG(currIter->second);
  else
    fareTypeGroup = NoPNRPricingTrx::FareTypes::FTG_NONE;

  if (fareTypeGroup != NoPNRPricingTrx::FareTypes::FTG_NONE)
  {
    diag.enable(Diagnostic611);
    diag.printLine();

    diag << "FARE TYPE DESIGNATION - " << NoPNRPricingTrx::FareTypes::FTGC[fareTypeGroup] << "\n";
    diag << "FARE TYPE DESIGNATION INCLUDES THE FOLLOWING FARE TYPES:\n";

    const NoPNRFareTypeGroup* newFareTypeGroup =
        trx.dataHandle().getNoPNRFareTypeGroup(fareTypeGroup);

    int cnt = 1;
    const unsigned int LINE_LENGTH = 63;
    const unsigned int MARGIN = 2;
    int seqNo = 0;
    int noDisplayOptions = 0;

    if (newFareTypeGroup != nullptr)
    {
      diag << " ";
      const std::vector<FareTypeMatrix*>& segs = (*newFareTypeGroup).segs();

      for (const auto fareTypeMatrix : segs)
      {
        const FareType& fareType = fareTypeMatrix->fareType();
        if (fareType.size() + cnt + 1 < LINE_LENGTH)
        {
          diag << " " << fareType;
          cnt = fareType.size() + cnt + 1;
        }
        else
        {
          diag << "\n"
               << "  " << fareType;
          cnt = fareType.size() + MARGIN;
        }
      }

      diag << "\n";

      const NoPNROptions* wqcc = trx.noPNROptions();

      if (!wqcc)
        throw std::runtime_error("ERROR NO NOPNROPTIONS FOR AGENT");

      const std::vector<NoPNROptionsSeg*>& segments = wqcc->segs();

      for (const auto segment : segments)
      {
        if (segment->fareTypeGroup() == fareTypeGroup)
        {
          seqNo = segment->seqNo();
          noDisplayOptions = segment->noDisplayOptions();
        }
      }
    }

    diag.setf(std::ios::left, std::ios::adjustfield);
    diag << std::setw(59) << "FARE TYPE DESIGNATION MATRIX PRIORITY ORDER";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag << "- " << std::setw(2) << seqNo << "\n";

    diag.setf(std::ios::left, std::ios::adjustfield);
    diag << std::setw(59) << "NBR OF FARE TYPE COMBINATIONS REQUESTED";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag << "- " << std::setw(2) << noDisplayOptions << "\n";

    diag.setf(std::ios::left, std::ios::adjustfield);
    diag << std::setw(59) << "NBR OF FARE TYPE COMBINATIONS FOUND";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag << "- " << std::setw(2) << diagInfo.dupResCnt << "\n";

    diag.setf(std::ios::left, std::ios::adjustfield);
    diag << std::setw(59) << "NBR OF FARE TYPE COMBINATIONS THAT ARE NOT DUPLICATED";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag << "- " << std::setw(2) << diagInfo.noDupResCnt << "\n";
  }

  diag.flushMsg();
}

bool
FarePathCollector::enoughOptions(AltPricingTrx& trx,
                                 std::vector<FarePath*>& fPaths,
                                 std::vector<bool>& paxOptions,
                                 uint16_t j)
{
  LOG4CXX_INFO(logger, "Entered FarePathCollector::enoughOptions()");

  // Match & no FP found -> nothing change
  if (fPaths.empty() && !trx.getRequest()->isLowFareRequested())
    return true;

  // No-Match & no FP found; Match options stored
  if (fPaths.empty() && trx.getRequest()->isLowFareRequested() && !trx.itin().empty())
  { // -> nothing change
    const Itin* itin = trx.itin().front();
    if (!itin->farePath().empty())
      paxOptions[j] = true;

    return true;
  }

  // 1. No-Match & FP found -> find number of stored Match
  // 2. Match & FP found

  uint32_t fareOptionMaxDefault = AltPricingUtil::getConfigFareOptionMaxDefault();
  uint32_t fareOptionMaxLimit = AltPricingUtil::getConfigFareOptionMaxLimit();

  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  if (noPNRPricingTrx)
  {
    const NoPNROptions* noPNROptions = noPNRPricingTrx->noPNROptions();
    _allowDuplicateTotals = noPNROptions->wqDuplicateAmounts() == YES ? true : false;
    _farePathMaxCount = noPNROptions->maxNoOptions();
  }
  else
  {
    if (!_alreadyReadFCC)
    {
      if (trx.getRequest()->isWpa50())
      {
        _farePathMaxCount = AltPricingTrx::WPA_50_OPTIONS;
      }

      const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

      if (fcConfig)
      {
        if (trx.isRfbListOfSolutionEnabled())
          _allowDuplicateTotals = true;
        else
          _allowDuplicateTotals = fcConfig->wpaShowDupAmounts() == YES;

        _alreadyReadFCC = true;

        if (!_farePathMaxCount)
        {
          _farePathMaxCount = fcConfig->wpaFareOptionMaxNo();
        }
      }
      else
      {
        LOG4CXX_WARN(logger,
                     "Error getting Fare Calc Config for PCC: "
                         << trx.getRequest()->ticketingAgent()->tvlAgencyPCC()
                         << " Using default values.");
      }
    }
  }
  uint32_t farePathCount = _farePathMaxCount;

  if (farePathCount <= 0)
  {
    farePathCount = fareOptionMaxDefault;
  }
  else if (farePathCount > fareOptionMaxLimit)
  {
    farePathCount = fareOptionMaxLimit;
  }

  // No-Match & FP found -> find number of stored Match
  if (!fPaths.empty() && trx.getRequest()->isLowFareRequested() && !trx.itin().empty())
  {
    const Itin* itin = trx.itin().front();

    if (itin->farePath().empty())
      return true;

    PaxType currPaxType = *(fPaths.front()->paxType());
    const size_t fpMatchCount = itin->countPaxTypeInFarePaths(currPaxType);

    if (farePathCount <= fpMatchCount)
    {
      //        fpIter = fpIterEnd;
      fPaths.clear();
      return true;
    }
    if (!fpMatchCount)
      return true;

    if (fpMatchCount < farePathCount)
    {
      farePathCount = (farePathCount - fpMatchCount);
    }
    if (!_allowDuplicateTotals)
    {
      AltPricingUtil::intermediateProcessing(trx, fPaths, farePathCount);
    }
    else // duplicates allows
    {
      std::vector<FarePath*> leftFarePaths;
      leftFarePaths.reserve(fPaths.size());

      for (const auto farePath : fPaths)
      {
        if (!farePath)
          continue;

        leftFarePaths.push_back(farePath);

        if (leftFarePaths.size() == farePathCount)
          break;
      }

      fPaths = std::move(leftFarePaths);
    }
    return true;
  }

  // Match & FP Found
  if (!_allowDuplicateTotals)
  {
    return (AltPricingUtil::intermediateProcessing(trx, fPaths, farePathCount)
                ? true
                : (paxOptions[j] = false));
  }

  if (fPaths.size() < farePathCount)
  {
    paxOptions[j] = false;
    return false;
  }

  return true;
}
} //TSE
