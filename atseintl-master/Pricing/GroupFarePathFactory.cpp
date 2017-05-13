//-------------------------------------------------------------------
// File:    GroupFarePathFactory.cpp
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

#include "Pricing/GroupFarePathFactory.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"
#include "Rules/AccompaniedTravel.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <exception>

// void
// forDebugOnlyDisplayFPIndices(std::vector<uint32_t>& v, std::string& str);

namespace tse
{
Logger
GroupFarePathFactory::_logger("atseintl.Pricing.GroupFarePathFactory");

FALLBACK_DECL(fallbackDividePartySolutionOptimization);

const uint32_t
GroupFarePathFactory::INVALID_FP_INDEX = ~uint32_t(0);

GroupFarePathFactory::~GroupFarePathFactory()
{
  for (PaxFarePathFactoryBase* paxFPF : _paxFarePathFactoryBucket)
    for (FarePathFactory* fpf : paxFPF->farePathFactoryBucket())
      fpf->releaseMemory();
}

bool
GroupFarePathFactory::initGroupFarePathPQ()
{
  TSELatencyData metrics(_trx, "PO INIT GROUPFP PQ");

  if (_paxFarePathFactoryBucket.empty())
  {
    LOG4CXX_ERROR(_logger, "_farePathFactoryBucket Empty");
    return false;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diag = factory->create(_trx);

  _matchCurrency = currencyMatchingNeeded();
  _totalFPFactory = static_cast<uint32_t>(_paxFarePathFactoryBucket.size());
  _isXoRequest = _trx.getOptions()->isXoFares();
  _isXcRequest = _trx.getOptions()->forceCorpFares();
  prepareInfantFactoryIndicators();

  std::vector<uint32_t> fpIndices(_totalFPFactory, 0);

  // expansionPoint (xPoint) is 0
  buildGroupFarePath(true, fpIndices, 0, *diag);

  diag->flushMsg();
  return true;
}

void
GroupFarePathFactory::clearFactories()
{
  for (PaxFarePathFactoryBase* paxFPF : _paxFarePathFactoryBucket)
    for (FarePathFactory* fpf : paxFPF->farePathFactoryBucket())
      fpf->clearAndReleaseMemory();
}

GroupFarePath*
GroupFarePathFactory::getGroupFarePath(DiagCollector& diag, const MoneyAmount lastAmount)
{
  TSELatencyData metrics(_trx, "PO GET GROUP FAREPATH");

  GroupFarePath* gfpathReserved = nullptr;
  if (_groupFarePathPQ.empty())
  {
    // if empty because of reaching 1/2 ShortCkt Timeout
    if (LIKELY(startMultiPaxShortCkt()))
    {
      return getSFBGroupFarePath(gfpathReserved, diag);
    }
  }

  if (_runningMultiPaxShortCktLogic)
  {
    // IS may call multiple time
    return getSFBGroupFarePath(gfpathReserved, diag);
  }

  const bool chkCPStat = PricingUtil::isFullCPTrx(_trx);
  bool isCmdPriced = false;

  while (!_done)
  {
    checkTrxAborted(_trx);

    if (_groupFarePathPQ.empty())
    {
      _done = true;
      break;
    }

    GroupFarePath* gfpath = _groupFarePathPQ.dequeue();
    if (lastAmount > EPSILON)
    {
      const MoneyAmount diff = gfpath->getTotalNUCAmount() - lastAmount;
      if ((diff > EPSILON))
        return nullptr;
    }

    if (gfpathReserved && !isCmdPriced)
    {
      const MoneyAmount diff = gfpath->getTotalNUCAmount() - gfpathReserved->getTotalNUCAmount();
      if ((diff > EPSILON) || (-diff > EPSILON) || (_trx.awardRequest()))
      {
        setAltDateOptionFound(*gfpathReserved);
        if (TrxUtil::cmdPricingTuningEnabled(_trx))
          restoreCmdPrcInfo();

        return gfpathReserved;
      }
    }

    bool shouldReserve = false;

    if (isGroupFarePathValid(*gfpath))
    {
      if (UNLIKELY((_trx.getTrxType() != PricingTrx::MIP_TRX) && isMultiOptionRequest(*gfpath)))
        buildNextGroupFarePathSet(false, *gfpath, diag);

      // Command pricing, try to find a solution that does not fail anything
      if (UNLIKELY(chkCPStat))
      {
        if (!gfpath->cmdPrcWithWarning())
        {
          buildNextGroupFarePathSetMIP(*gfpath, diag);

          if (TrxUtil::cmdPricingTuningEnabled(_trx))
            restoreCmdPrcInfo();

          return gfpath;
        }
        else
        {
          isCmdPriced = true;
          shouldReserve = true;
        }
      }
      else
      {
        if (reserveGFPandContinue(*gfpath, chkCPStat, gfpathReserved) == false)
        {
          buildNextGroupFarePathSetMIP(*gfpath, diag);
          return gfpath;
        }
      }
    }
    else if (startMultiPaxShortCkt())
      return getSFBGroupFarePath(gfpathReserved, diag);

    if (shouldReserve)
      reserveGFPandContinue(*gfpath, chkCPStat, gfpathReserved);

    if (inhibitDivideParty(gfpathReserved))
      gfpathReserved = nullptr;

    buildNextGroupFarePathSet(false, *gfpath, diag);
  }

  if (gfpathReserved)
  {
    buildNextGroupFarePathSetMIP(*gfpathReserved, diag);

    if (TrxUtil::cmdPricingTuningEnabled(_trx))
      restoreCmdPrcInfo();

    return gfpathReserved;
  }

  return nullptr;
}

bool
GroupFarePathFactory::reserveGFPandContinue(GroupFarePath& gfpath,
                                            bool cmdPricing,
                                            GroupFarePath*& gfpathReserved) const
{
  if (UNLIKELY(cmdPricing))
  {
    if (gfpathReserved == nullptr)
    {
      gfpathReserved = &gfpath;

      if (TrxUtil::cmdPricingTuningEnabled(_trx))
      {
        disableFurtherCmdPrcSolution();
      }
    }
    return true;
  }

  if (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.isAltDates() &&
      _trx.getOptions()->inhibitSplitPNR() == false)
  {
    return false;
  }

  gfpath.divideParty() = isDivideParty(gfpath);

  if (!fallback::fallbackDividePartySolutionOptimization(&_trx))
  {
    if ((gfpath.divideParty() == true) && gfpath.valid())
    {
      if (!findNonDividePartySolution(gfpath))
        return false;
    }
  }

  if (gfpath.divideParty() == false && gfpath.valid())
  {
    // Since it is valid and not a Divide-Party
    // no need to Continue processing
    return false;
  }

  // at this point gfpath can be valid but it is a Divide-Party
  // so we need to continue processing

  if (gfpathReserved == nullptr)
  {
    gfpathReserved = &gfpath;
  }
  else if (UNLIKELY(gfpath.valid() && !gfpathReserved->valid()))
  {
    // reserve the valid one, continue for non Divie-Party
    gfpathReserved = &gfpath;
  }
  else if (UNLIKELY(!gfpath.valid() && gfpathReserved->valid()))
  {
    // should not reserve, but continue to search for non-divide-party
    return true;
  }
  else if (UNLIKELY(!gfpath.divideParty() && gfpathReserved->divideParty()))
  {
    gfpathReserved = &gfpath;
  }

  return true;
}

bool
GroupFarePathFactory::buildNextGroupFarePathSetMIP(const GroupFarePath& gfpath, DiagCollector& diag)
{
  if (_trx.getTrxType() != PricingTrx::MIP_TRX)
    return false;

  if (isMultiOptionRequest(gfpath))
  {
    buildNextGroupFarePathSet(false, gfpath, diag);
    return true;
  }

  return false;
}

bool
GroupFarePathFactory::buildNextGroupFarePathSet(bool initStage,
                                                const GroupFarePath& gfpPrev,
                                                DiagCollector& diag)
{
  if (_done)
    return false;

  if (UNLIKELY(_runningMultiPaxShortCktLogic))
    return true;

  if (UNLIKELY(_trx.getTrxType() == PricingTrx::MIP_TRX && (_isXoRequest || _isXcRequest) &&
      gfpPrev.groupFPPQItem().size() != _totalFPFactory))
  {
    return true;
  }

  const std::vector<uint32_t>& fpIndices = gfpPrev.farePathIndices();
  TSE_ASSERT(fpIndices.size() == _totalFPFactory);

  for (uint16_t xPoint = gfpPrev.xPoint(); xPoint < _totalFPFactory; ++xPoint)
  {
    if (fpIndices[xPoint] == INVALID_FP_INDEX)
      continue;

    buildGroupFarePath(initStage, fpIndices, xPoint, diag);
  }

  return true;
}

void
GroupFarePathFactory::buildGroupFarePath(const bool initStage,
                                         const std::vector<uint32_t>& fpIndices,
                                         uint16_t xPoint,
                                         DiagCollector& diagIn)
{
  std::vector<GETFPInput> thrInputVect;
  initThreads(thrInputVect, initStage, xPoint, fpIndices);

  startThreads(thrInputVect);

  GroupFarePath* gfpath = nullptr;
  _trx.dataHandle().get(gfpath);

  if (!collectThreadResult(thrInputVect, *gfpath, true, diagIn))
    return;

  gfpath->xPoint() = xPoint; // next farePath will expand from the next mkt

  if (xPoint == 0 && _totalFPFactory > 1)
    checkZeroAmountInfantFP(*gfpath, diagIn);

  _groupFarePathPQ.enqueue(gfpath);
}

void
GroupFarePathFactory::initThreads(std::vector<GETFPInput>& thrInputVect,
                                  const bool initStage,
                                  const uint16_t xPoint,
                                  const std::vector<uint32_t>& fpIndices)
{
  TSE_ASSERT(fpIndices.size() == _totalFPFactory);
  thrInputVect.clear();
  thrInputVect.reserve(_totalFPFactory);

  for (uint16_t factIdx = 0; factIdx < _totalFPFactory; ++factIdx)
  {
    if (fpIndices[factIdx] == INVALID_FP_INDEX)
      continue;

    thrInputVect.resize(thrInputVect.size() + 1);
    GETFPInput& thrInput = thrInputVect.back();

    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(_trx);

    thrInput.fppqItem = nullptr;
    thrInput.done = false;
    thrInput.primaryFPPQItem = nullptr;
    thrInput.pfpf = _paxFarePathFactoryBucket[factIdx];
    thrInput.gfpf = this;
    thrInput.diag = diag;
    thrInput.idx = fpIndices[factIdx];
    thrInput.factIdx = factIdx;

    if (factIdx == xPoint && !initStage)
    {
      ++thrInput.idx;
    }

    // Some infant's fare paths could be skipped because of CAT13 validation.
    // Therefore, we need to reset the index when primary FP changes.
    if (xPoint == 0 && factIdx > 0 && _infantFactoryInd[factIdx])
    {
      thrInput.idx = 0;
    }

    thrInput.trx(&_trx);
    thrInput.getAlreadyBuiltFPPQItem();
  }
}

void
GroupFarePathFactory::initThreadsSameFareBreak(std::vector<GETFPInput>& thrInputVect,
                                               const FPPQItem* primaryFPPQItem)
{
  thrInputVect.resize(_totalFPFactory - 1u); // primary FP is already done

  for (uint16_t factIdx = 1; factIdx < _totalFPFactory; ++factIdx)
  {
    GETFPInput& thrInput = thrInputVect[factIdx - 1u];

    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(_trx);

    thrInput.fppqItem = nullptr;
    thrInput.done = false;
    thrInput.primaryFPPQItem = primaryFPPQItem;
    thrInput.pfpf = _paxFarePathFactoryBucket[factIdx];
    thrInput.gfpf = this;
    thrInput.diag = diag;
    thrInput.idx = 0;

    thrInput.factIdx = factIdx;

    thrInput.trx(&_trx);
  }
}

void
GroupFarePathFactory::startThreads(std::vector<GETFPInput>& thrInputVect) const
{
  try
  {
    TseRunnableExecutor farePathPQTaskExecutor(TseThreadingConst::FAREPATHPQ_TASK);
    TseRunnableExecutor farePathPQSynchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);

    size_t remainingTasks = thrInputVect.size();

    for (auto& farePathTask : thrInputVect)
    {
      if (farePathTask.done)
      {
        --remainingTasks;
        continue;
      }

      LOG4CXX_INFO(_logger, "Starting Thread: startThreads");

      TseRunnableExecutor& taskExecutor = (_usePooling && (remainingTasks > 1))
                                              ? farePathPQTaskExecutor
                                              : farePathPQSynchronousExecutor;

      taskExecutor.execute(farePathTask);
      --remainingTasks;
    } // end for

    farePathPQTaskExecutor.wait();
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << ex.message() << " - buildGroupFarePath failed");
    throw;
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << e.what() << " - buildGroupFarePath failed");
    throw;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "UnKnown Exception: buildGroupFarePath failed");
    throw;
  }
}

bool
GroupFarePathFactory::collectThreadResult(std::vector<GETFPInput>& thrInputVect,
                                          GroupFarePath& gfpath,
                                          bool noneFound,
                                          DiagCollector& diagIn) const
{
  gfpath.farePathIndices().assign(_totalFPFactory, INVALID_FP_INDEX);
  bool allFound = true;

  for (GETFPInput& thrArg : thrInputVect)
  {
    diagIn << *thrArg.diag;

    if (thrArg.errResponseCode != ErrorResponseException::NO_ERROR)
    {
      throw ErrorResponseException(thrArg.errResponseCode, thrArg.errResponseMsg.c_str());
    }

    gfpath.farePathIndices()[thrArg.factIdx] = thrArg.idx;

    if (thrArg.done)
    {
      noneFound = false;
      FPPQItem* fppqItem = thrArg.fppqItem;
      gfpath.groupFPPQItem().push_back(fppqItem);
      gfpath.increaseTotalNUCAmount(thrArg.pfpf->paxType()->number() *
                                    fppqItem->farePath()->getTotalNUCAmount());

      const FarePath& fp = *fppqItem->farePath();
      MoneyAmount scoreDeviation = fp.yqyrNUCAmount() + fp.bagChargeNUCAmount();

      scoreDeviation -= fp.getDynamicPriceDeviationAmount();

      gfpath.increaseNUCAmountScore(thrArg.pfpf->paxType()->number() * scoreDeviation);

      setPriority(*fppqItem, gfpath);
    }
    else
    {
      allFound = false;

      if ((_isXoRequest || _isXcRequest) && !(_trx.isAltDates() && _trx.awardRequest()))
      {
        gfpath.farePathIndices()[thrArg.factIdx] = INVALID_FP_INDEX;
      }
    }
  }

  if (_isXcRequest || _isXoRequest)
    return !noneFound;
  return allFound;
}

void
GroupFarePathFactory::checkZeroAmountInfantFP(GroupFarePath& gfpath, DiagCollector& diag)
{
  if (gfpath.groupFPPQItem().size() != _totalFPFactory)
    return;

  FPPQItem& primaryFPPQItem = *gfpath.groupFPPQItem().front();
  FarePath& primaryFP = *primaryFPPQItem.farePath();

  for (uint32_t factId = 1; factId < _totalFPFactory; ++factId)
  {
    if (!_infantFactoryInd[factId])
      continue;

    FPPQItem& fppqItem = *gfpath.groupFPPQItem()[factId];
    FarePath& fp = *fppqItem.farePath();
    uint32_t& fpIndex = gfpath.farePathIndices()[factId];

    if (fabs(fp.getTotalNUCAmount()) > EPSILON) // not 0-amount fare path
      continue;

    if (!_accompaniedTravel->validateInfFares(_trx, fp, primaryFP))
    {
      PaxFarePathFactoryBase* pfpf = _paxFarePathFactoryBucket[factId];
      uint32_t newIdx = 0;
      FPPQItem* newFppqItem =
          pfpf->getFirstValidZeroAmountFPPQItem(fpIndex, primaryFPPQItem, diag, &newIdx);
      if (newFppqItem)
      {
        gfpath.groupFPPQItem()[factId] = newFppqItem;
        fpIndex = newIdx;
        recalculatePriorities(gfpath);
      }
    }
  }
}

void
GroupFarePathFactory::setAltDateOptionFound(const GroupFarePath& gfpath) const
{
  if (_trx.isAltDates())
  {
    setAltDateOptionFound(*gfpath.groupFPPQItem().front()->farePath()->itin());
  }
}

void
GroupFarePathFactory::setAltDateOptionFound(Itin& itin) const
{
  std::pair<DateTime, DateTime>* itinDatePair = itin.datePair();
  if (itinDatePair != nullptr)
  {
    AltDateCellCompare altDateCellCompare(*itinDatePair);
    const PricingTrx::AltDatePairs::iterator itEnd = _trx.altDatePairs().end();
    PricingTrx::AltDatePairs::iterator it =
        std::find_if(_trx.altDatePairs().begin(), itEnd, altDateCellCompare);
    if (it != itEnd)
    {
      it->second->numOfSolutionNeeded = 0;
      itin.errResponseCode() = ErrorResponseException::NO_ERROR;
    }
  }
}

void
GroupFarePathFactory::setPriority(const FPPQItem& fppqItem, GroupFarePath& gfpath) const
{
  const PriorityStatus& sourceStatus = fppqItem.priorityStatus();

  PriorityStatus& status = gfpath.mutablePriorityStatus();

  if (sourceStatus.farePriority() > status.farePriority())
  {
    // regular priority= 1,
    // Cxr wants CxrFare but a FareUsage contains an Industry fare then
    // priority is lower (value is higher, 2)
    //

    status.setFarePriority(sourceStatus.farePriority());
  }

  if (sourceStatus.paxTypeFarePriority() > status.paxTypeFarePriority())
  {
    // Requested PaxType fare was not used,
    // default ADT fare was used instead
    // priority will be lower in PQ and will be in effect
    // for same price level

    status.setPaxTypeFarePriority(sourceStatus.paxTypeFarePriority());
  }

  // Set the FareByRule priority
  //
  status.setFareByRulePriority(PriorityStatus::mergeFbrPriorities(
    status.fareByRulePriority(), sourceStatus.fareByRulePriority()));

  if (sourceStatus.fareCxrTypePriority() > status.fareCxrTypePriority())
  {
    status.setFareCxrTypePriority(sourceStatus.fareCxrTypePriority());
  }

  if (sourceStatus.negotiatedFarePriority() > status.negotiatedFarePriority())
  {
    status.setNegotiatedFarePriority(sourceStatus.negotiatedFarePriority());
  }

  status.setPtfRank(status.ptfRank() + sourceStatus.ptfRank());
}

void
GroupFarePathFactory::recalculatePriorities(GroupFarePath& gfp) const
{
  gfp.resetPriorities();

  for (const auto fppqItem : gfp.groupFPPQItem())
    setPriority(*fppqItem, gfp);
}

bool
GroupFarePathFactory::isGroupFarePathValid(GroupFarePath& gfpath)
{
  if (UNLIKELY(gfpath.groupFPPQItem().empty()))
    return false;

  if (checkShopping(gfpath) == false)
    return false;

  // For XO entry with more than one paxtype, we need to ensure
  // that all pax were priced.
  if (UNLIKELY(_trx.getOptions()->isXoFares() && _trx.paxType().size() > 1 &&
      gfpath.groupFPPQItem().size() < _trx.paxType().size()))
  {
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(_trx);

    if ((_trx.altTrxType() == PricingTrx::WP && fcConfig->wpNoMatchPermitted() == 'N') ||
        _trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      if (_trx.getTrxType() == PricingTrx::PRICING_TRX)
      {
        throw ErrorResponseException(ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS);
      }
      else
      {
        gfpath.valid() = false;
        return false;
      }
    }
  }

  if (gfpath.groupFPPQItem().size() == 1)
    return gfpath.valid() = isGroupFarePathValidForSinglePaxType(gfpath);

  // ----------------- if we have multiple PaxType --------------------
  return isGroupFarePathValidForMultiPaxType(gfpath);
}

bool
GroupFarePathFactory::isGroupFarePathValidForMultiPaxType(GroupFarePath& gfpath)
{
  if (_trx.isAltDates())
    if (!isSameItinInEachFarePath(gfpath))
      return false;

  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(_trx));
  diag.enable(Diagnostic699);
  displayGroupFP(gfpath, diag);

  if ((_trx.isValidatingCxrGsaApplicable()) && (isValidGFPForValidatingCxr(gfpath, diag) == false))
  {
    if (diag.isActive())
    {
      printShutdownMessage(diag);
      diag << "FAILED: NO VALIDATING CARRIER\n";
      diag.printLine();
      diag.flushMsg();
    }
    return false;
  }

  if (checkCurrency(gfpath, diag) == false)
  {
    if (UNLIKELY(diag.isActive()))
    {
      printShutdownMessage(diag);
      diag << "FAILED: CURRENCY MISMATCH\n";
      diag.printLine();
      diag.flushMsg();
    }
    return false;
  }

  std::vector<FarePath*> gfpVect;
  const bool skipThrowing =
      !_trx.diagnostic().isActive() && _trx.diagnostic().diagnosticType() == Diagnostic372;

  if (LIKELY(!skipThrowing))
  {
    for (const auto fppqItem : gfpath.groupFPPQItem())
    {
      if (UNLIKELY(fppqItem->farePath()->intlSurfaceTvlLimit()))
      {
        ErrorResponseException ex(ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR,
                                  "ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED");
        throw ex;
      }
      gfpVect.push_back(fppqItem->farePath());
    }
  }

  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic372 && !_trx.diagnostic().isActive()))
  {
    _trx.diagnostic().activate();
  }

  LOG4CXX_INFO(_logger, "Num of FarePath in the Group = " << gfpVect.size());

  Itin::ISICode firstISICode = Itin::UNKNOWN;
  if (LIKELY(!gfpVect.empty()))
  {
    firstISICode = PricingUtil::calculateFarePathISICode(_trx, *(*gfpVect.begin()));
  }

  bool needFareBreakChk = false;

  for (const auto farePath : gfpVect)
  {
    // Infant can price with no adult now. (Seperately PNR)
    // Check to see 0 amount fares using same fare basis of first accompanying
    // passenger
    //
    if (PaxTypeUtil::isInfant(
            _trx.dataHandle(), farePath->paxType()->paxType(), farePath->paxType()->vendorCode()))
    {
      if (UNLIKELY(!_accompaniedTravel->validateInfFares(_trx, *farePath, gfpVect)))
      {
        LOG4CXX_INFO(_logger, "AccompaniedTravel FAILED, Invalid GroupFarePath");

        if (diag.isActive())
        {
          printShutdownMessage(diag);
          diag << " INVALID GROUP FP: INFANT DID NOT USE ACCOMPANYING PSG FARE\n";
          diag.printLine();
          diag.flushMsg();
        }

        return false;
      }
    }

    if (UNLIKELY(!checkISICode(firstISICode, *farePath, diag)))
    {
      printShutdownMessage(diag);
      diag.printLine();
      diag.flushMsg();
      return false;
    }

    for (const auto pricingUnit : farePath->pricingUnit())
    {
      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        const bool needSameFareBreak = fareUsage->paxTypeFare()->needAccSameFareBreak();

        if (needSameFareBreak || fareUsage->paxTypeFare()->needAccSameCabin())
        {
          needFareBreakChk = true;

          if (UNLIKELY(diag.isActive()))
          {
            if (needSameFareBreak)
              diag << " ACC SAME FARE BREAK ";
            else
              diag << " ACC SAME CABIN ";

            diag << " NEEDED: " << farePath->paxType()->paxType() << " - "
                 << fareUsage->paxTypeFare()->fareClass() << "\n";
          }

          if (!processMultiPax(
                  gfpVect, *fareUsage, needSameFareBreak, pricingUnit->isSideTripPU(), diag))
          {
            LOG4CXX_INFO(_logger, "Building SameBreakFUVect FAILED, Invalid GroupFarePath");
            if (diag.isActive())
            {
              printShutdownMessage(diag);
              diag << " INVALID GROUP FP: FAILED SAME FARE BREAK\n";
              diag.printLine();
              diag.flushMsg();
            }

            return false;
          }
        }
      }
    }

  } // for each farePath in the group

  // There is PASS condition.

  if (!needFareBreakChk)
  {
    LOG4CXX_INFO(_logger, "No Need to check AccompanidTravel, Valid GroupFarePath");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " VALID GROUP FP\n";
      printShutdownMessage(diag);
      diag.printLine();
      diag.flushMsg();
    }
    return gfpath.valid() = true;
  }

  LOG4CXX_INFO(_logger, "AccompanidTravel PASSED, Valid GroupFarePath");

  if (UNLIKELY(diag.isActive()))
  {
    diag << " VALID GROUP FP: PASSED CAT13\n";
    printShutdownMessage(diag);
    diag.printLine();
    diag.flushMsg();
  }
  return gfpath.valid() = true;
}

bool
GroupFarePathFactory::isGroupFarePathValidForSinglePaxType(const GroupFarePath& gfpath) const
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(_trx));
  diag.enable(Diagnostic699);
  displayGroupFP(gfpath, diag);

  const bool skipThrowing =
      !_trx.diagnostic().isActive() && _trx.diagnostic().diagnosticType() == Diagnostic372;

  if (gfpath.groupFPPQItem().front()->farePath()->intlSurfaceTvlLimit() && !skipThrowing)
  {
    ErrorResponseException ex(ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR);
    throw ex;
  }

  if (_trx.diagnostic().diagnosticType() == Diagnostic372 && !_trx.diagnostic().isActive())
  {
    _trx.diagnostic().activate();
  }

  // Unless doing command pricing, warn that infant can not travel alone
  if ((!_trx.getOptions() || !_trx.getOptions()->fbcSelected()) &&
      PaxTypeUtil::isInfant(_trx.dataHandle(),
                            gfpath.groupFPPQItem()[0]->farePath()->paxType()->paxType(),
                            gfpath.groupFPPQItem()[0]->farePath()->paxType()->vendorCode()))
  {
    if (diag.isActive())
    {
      diag << " WARNING GROUP FP: INFANT DID NOT USE ACCOMPANIED ADT FARE\n";
    }
  }

  if (diag.isActive())
  {
    diag << " VALID GROUP FP\n";
    printShutdownMessage(diag);
    diag.printLine();
    diag.flushMsg();
  }

  return true;
}

bool
GroupFarePathFactory::isValidGFPForValidatingCxr(GroupFarePath& gfpath, DiagCollector& diag) const
{
  std::vector<CarrierCode> gfpCxrVec;
  bool firstTime = true;

  for (const auto fppqItem : gfpath.groupFPPQItem())
  {
    FarePath* farePath = fppqItem->farePath();

    if (firstTime)
    {
      gfpCxrVec = farePath->validatingCarriers();
      firstTime = false;
      if (gfpCxrVec.empty())
      {
        // If a valid FarePath doesn't have a validating Cxr List,
        // We don't need to process GSA validation and just let this GFP to pass
        gfpath.validatingCarriers().clear();
        return true;
      }
    }
    else
    {
      //Intersect with already being processed list of Validating Cxr
      PricingUtil::intersectCarrierList(gfpCxrVec, farePath->validatingCarriers());

      if (gfpCxrVec.empty())
      {
        // Empty intersection list means, this FarePath is not good for further processing
        gfpath.validatingCarriers().clear();
        return false;
      }
    }
  }

  // We are done with processing all FarePath of this GFP. If we are here, that means the list is not empty.
  // Let's update the GFP level Validating Cxr list
  if (UNLIKELY(diag.isActive()))
  {
    diag << " VALIDATING CXR: ";
    for (auto& elem : gfpCxrVec)
      diag << elem << "  ";
    diag << std::endl;
  }

  gfpath.validatingCarriers() = gfpCxrVec;

  // Set all farePath within GroupFarePath to common validating carriers
  for (const auto fppqItem : gfpath.groupFPPQItem())
  {
    if (LIKELY(fppqItem->farePath()))
      fppqItem->farePath()->validatingCarriers() = gfpCxrVec;
  }

  return true;
}

void
GroupFarePathFactory::printShutdownMessage(DiagCollector& diag) const
{
  for (const auto paxFarePathFactory : _paxFarePathFactoryBucket)
  {
    if (paxFarePathFactory->pricingShortCktHappened())
    {
      diag << " WARNING: ALL FARE COMBINATIONS NOT VALIDATED DUE TO TIMEOUT \n";
      return;
    }
  }
}

//---------------------------------------------------------------------------------------
// When it fails Cat-13 a lot and it has passed some factor of ShorCkt
// Time out, we stick with the Primary/ADT FarePath and  search FP of
// same FareBreak for other PaxType also
//---------------------------------------------------------------------------------------
GroupFarePath*
GroupFarePathFactory::getSFBGroupFarePath(GroupFarePath*& gfpathReserved, DiagCollector& diag)
{
  const bool chkCPStat = PricingUtil::isFullCPTrx(_trx);
  bool isCmdPriced = false;

  GroupFarePath* gfpath = nullptr;
  _trx.dataHandle().get(gfpath);

  std::vector<GETFPInput> thrInputVect;

  //------- Create/start Threads ---------
  uint32_t idx = 0;
  while (true)
  {
    checkTrxAborted(_trx);
    PaxFarePathFactoryBase* pfpf = _paxFarePathFactoryBucket.front();
    FPPQItem* primaryFPPQItem = pfpf->getFPPQItem(idx, diag);
    if (primaryFPPQItem == nullptr)
    {
      break;
    }
    const FareMarketPath* fareMarketPath = primaryFPPQItem->puPath()->fareMarketPath();
    gfpath->groupFPPQItem().push_back(primaryFPPQItem);

    const FarePath& primaryFp = *primaryFPPQItem->farePath();
    gfpath->increaseTotalNUCAmount(pfpf->paxType()->number() * primaryFp.getTotalNUCAmount());

    MoneyAmount scoreDeviation = primaryFp.yqyrNUCAmount() + primaryFp.bagChargeNUCAmount();
    scoreDeviation -= primaryFp.getDynamicPriceDeviationAmount();
    gfpath->increaseNUCAmountScore(pfpf->paxType()->number() * scoreDeviation);

    setPriority(*primaryFPPQItem, *gfpath);

    initThreadsSameFareBreak(thrInputVect, primaryFPPQItem);

    startThreads(thrInputVect);

    if (UNLIKELY(!collectThreadResult(thrInputVect, *gfpath, false, diag)))
    {
      ++idx; // Next Primary/ADT FarePath index
      gfpath->reset();
      pfpf->removeFPF(&diag, fareMarketPath);
      continue;
    }

    if (gfpathReserved && !isCmdPriced)
    {
      const MoneyAmount diff = gfpath->getTotalNUCAmount() - gfpathReserved->getTotalNUCAmount();
      if (UNLIKELY((diff > EPSILON) || (-diff > EPSILON)))
      {
        if (TrxUtil::cmdPricingTuningEnabled(_trx))
          restoreCmdPrcInfo();
        return gfpathReserved;
      }
    }

    bool shouldReserve = false;

    if (isGroupFarePathValid(*gfpath))
    {
      setAltDateOptionFound(*gfpath);

      if (UNLIKELY(chkCPStat))
      {
        if (!gfpath->cmdPrcWithWarning())
        {
          if (TrxUtil::cmdPricingTuningEnabled(_trx))
            restoreCmdPrcInfo();
          return gfpath;
        }
        else
        {
          isCmdPriced = true;
          shouldReserve = true;
        }
      }
      else
      {
        if (reserveGFPandContinue(*gfpath, chkCPStat, gfpathReserved) == false)
        {
          if (LIKELY(TrxUtil::cmdPricingTuningEnabled(_trx)))
            restoreCmdPrcInfo();
          return gfpath;
        }
      }
    }

    if (UNLIKELY(shouldReserve))
      reserveGFPandContinue(*gfpath, chkCPStat, gfpathReserved);

    if (UNLIKELY(inhibitDivideParty(gfpathReserved)))
      gfpathReserved = nullptr;

    gfpath->reset();
    ++idx; // Next Primary/ADT FarePath index
  }
  if (gfpathReserved)
  {
    setAltDateOptionFound(*gfpathReserved);
    if (TrxUtil::cmdPricingTuningEnabled(_trx))
      restoreCmdPrcInfo();
    return gfpathReserved;
  }

  return nullptr;
}

void
GroupFarePathFactory::setPaxFarePathFactoryBucket(const std::vector<PaxFarePathFactory*>& bucket)
{
  _paxFarePathFactoryBucket.assign(bucket.begin(), bucket.end());
}

bool
GroupFarePathFactory::isSameItinInEachFarePath(const GroupFarePath& gfpath) const
{
  std::vector<FPPQItem*>::const_iterator it = gfpath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::const_iterator itEnd = gfpath.groupFPPQItem().end();
  Itin* itin = (*it)->farePath()->itin();
  ++it;
  for (; it != itEnd; ++it)
    if ((*it)->farePath()->itin()->travelSeg() != itin->travelSeg())
      return false;

  return true;
}

bool
GroupFarePathFactory::checkShopping(const GroupFarePath& gfpath) const
{
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::PRICING_TRX))
  {
    return true;
  }
  else if (_trx.getTrxType() == PricingTrx::IS_TRX)
  {
    return checkIS(gfpath);
  }
  else if ((_trx.getTrxType() == PricingTrx::MIP_TRX) && _trx.isAltDates())
  {
    return checkMIPAltDateCell(gfpath);
  }

  return true;
}

bool
GroupFarePathFactory::checkIS(const GroupFarePath& gfpath) const
{
  std::vector<PaxFarePathFactoryBase*>::const_iterator i = _paxFarePathFactoryBucket.begin();
  std::vector<PaxFarePathFactoryBase*>::const_iterator iEnd = _paxFarePathFactoryBucket.end();

  std::vector<FPPQItem*>::const_iterator j = gfpath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::const_iterator jEnd = gfpath.groupFPPQItem().end();

  std::set<DatePair> datePairSetIntersection;

  bool firstFP = true;
  for (; i != iEnd && j != jEnd; ++i, ++j)
  {
    if (UNLIKELY((*i)->checkFailedFPIS(*j, datePairSetIntersection, firstFP) == false))
    {
      return false;
    }
    firstFP = false;
  }
  return true;
}

bool
GroupFarePathFactory::checkMIPAltDateCell(const GroupFarePath& gfpath) const
{
  if (_trx.altDatePairs().size() <= 1)
  {
    // Single Date Alt-Date Trx
    return true;
  }

  std::pair<DateTime, DateTime>* itinDatePair =
      gfpath.groupFPPQItem().front()->farePath()->itin()->datePair();
  if (itinDatePair == nullptr)
  {
    LOG4CXX_ERROR(_logger, "AltDate Trx, But why Itin DatePair is NULL ???")
    return true;
  }

  AltDateCellCompare altDateCellCompare(*itinDatePair);
  const PricingTrx::AltDatePairs::iterator itEnd = _trx.altDatePairs().end();
  PricingTrx::AltDatePairs::iterator it =
      std::find_if(_trx.altDatePairs().begin(), itEnd, altDateCellCompare);

  if (it != itEnd)
  {
    if (it->second->numOfSolutionNeeded > 0)
    {
      // Itin Not priced
      return true;
    }
    else
    {
      // This Date is Already priced, Price next one...
      return false;
    }
  }
  else
  {
    LOG4CXX_ERROR(_logger, " Alt-Date Trx, But why Itin DatePair is Not in Trx???")
  }

  return true;
}

bool
GroupFarePathFactory::checkISICode(const Itin::ISICode& firstISICode,
                                   FarePath& fp,
                                   DiagCollector& diag) const
{
  Itin::ISICode fpISICode = PricingUtil::calculateFarePathISICode(_trx, fp);
  if (LIKELY(firstISICode == fpISICode))
  {
    return true;
  }

  if (diag.isActive())
  {
    diag << " INVALID GROUP FP: FAILED ISI CHECK: ";
    switch (firstISICode)
    {
    case Itin::SITI:
      diag << "SITI";
      break;
    case Itin::SITO:
      diag << "SITO";
      break;
    case Itin::SOTI:
      diag << "SOTI";
      break;
    case Itin::SOTO:
      diag << "SOTO";
      break;
    default:
      diag << "UNKNOWN";
      break;
    }

    diag << "-";

    switch (fpISICode)
    {
    case Itin::SITI:
      diag << "SITI";
      break;
    case Itin::SITO:
      diag << "SITO";
      break;
    case Itin::SOTI:
      diag << "SOTI";
      break;
    case Itin::SOTO:
      diag << "SOTO";
      break;
    default:
      diag << "UNKNOWN";
      break;
    }
    diag << "\n";
    diag.printLine();
    diag.flushMsg();
  }

  LOG4CXX_INFO(_logger, "ISI Check FAILED, Invalid GroupFarePath");
  return false;
}

bool
GroupFarePathFactory::processMultiPax(const std::vector<FarePath*>& gfpVect,
                                      const FareUsage& currentFU,
                                      const bool needSameFareBreak,
                                      const bool isInSideTripPU,
                                      DiagCollector& diag) const
{
  if (_trx.isShopping())
  {
    return processMultiPaxShopping(gfpVect, currentFU, diag);
  }
  std::vector<FarePath*> fpVect;
  std::vector<FareUsage*> fuVect;

  fuVect.push_back(const_cast<FareUsage*>(&currentFU));

  const Itin& itin = *gfpVect.front()->itin();

  int16_t startSegOrderFU = itin.segmentOrder(currentFU.travelSeg().front());
  int16_t endSegOrderFU = itin.segmentOrder(currentFU.travelSeg().back());

  for (const auto farePath : gfpVect)
  {
    bool doneThisFP = false;
    FareUsage* matchedAccFU = nullptr;

    for (const auto pricingUnit : farePath->pricingUnit())
    {
      if (UNLIKELY(pricingUnit->isSideTripPU() != isInSideTripPU))
        continue;

      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        if (fareUsage == &currentFU)
        {
          doneThisFP = true;
          break;
        }

        if (needSameFareBreak)
        {
          if (startSegOrderFU == itin.segmentOrder(fareUsage->travelSeg().front()) &&
              endSegOrderFU == itin.segmentOrder(fareUsage->travelSeg().back()))
          {
            matchedAccFU = fareUsage;
            doneThisFP = true;
            break;
          }
        }

        else
        {
          // only req same compartment, extended travel needed
          int16_t startSegOrderAccFU = itin.segmentOrder(fareUsage->travelSeg().front());
          if (startSegOrderAccFU > endSegOrderFU)
            break; // no more FU in this PU will have part of this travel

          int16_t endSegOrderAccFU = itin.segmentOrder(fareUsage->travelSeg().back());
          if (endSegOrderAccFU < startSegOrderFU)
            continue;

          if (fareUsage->paxTypeFare()->cabin() != currentFU.paxTypeFare()->cabin())
          {
            matchedAccFU = nullptr;
            doneThisFP = true;
            break;
          }

          matchedAccFU = fareUsage;
          // to count number of passenger accompanying, using any
          // matched FU in this FarePath gives the same result

          if (startSegOrderAccFU == startSegOrderFU && endSegOrderAccFU == endSegOrderFU)
          {
            // coverred all travel segments
            doneThisFP = true;
            break;
          }

          if (endSegOrderAccFU >= endSegOrderFU)
            break; // done with this PU, continue check FU of other PUs
        }
      }

      if (doneThisFP)
        break;
    } // PU loop

    if (matchedAccFU)
      fuVect.push_back(matchedAccFU);
  } // FarePaths loop

  if (fuVect.size() < 2)
  {
    if (diag.isActive())
    {
      diag << " INVALID GROUP FP: REQ ACC FARE BREAK NOT FOUND\n";
      diag.printLine();
      diag.flushMsg();
    }
    return false;
  }

  if (!_accompaniedTravel->validate(_trx, fuVect))
  {
    LOG4CXX_INFO(_logger, "AccompaniedTravel FAILED, Invalid GroupFarePath");
    if (diag.isActive())
    {
      diag << " INVALID GROUP FP: FAILED CAT13\n";
      diag.printLine();
      diag.flushMsg();
    }
    return false;
  }

  return true;
}

bool
GroupFarePathFactory::processMultiPaxShopping(const std::vector<FarePath*>& gfpVect,
                                              const FareUsage& currentFU,
                                              DiagCollector& diag) const
{
  std::vector<FareUsage*> fuVect;

  fuVect.push_back(const_cast<FareUsage*>(&currentFU));

  bool foundFB = false;

  uint32_t startCurrentLeg = 0, endCurrentLeg = 0, adoptedIndex = 0;
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
  const std::vector<TravelSeg*>& travelSegs = currentFU.travelSeg();

  bool res = ShoppingUtil::getLegTravelSegIndices(
      shoppingTrx, travelSegs, startCurrentLeg, endCurrentLeg, adoptedIndex);
  TSE_ASSERT(res);

  for (const auto farePath : gfpVect)
  {
    for (const auto pricingUnit : farePath->pricingUnit())
    {
      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        if (fareUsage == &currentFU)
          break;

        // same fareBreak?
        const std::vector<TravelSeg*>& fuTravelSegs = fareUsage->travelSeg();
        uint32_t startFULeg = 0, endFULeg = 0;

        res = ShoppingUtil::getLegTravelSegIndices(
            shoppingTrx, fuTravelSegs, startFULeg, endFULeg, adoptedIndex);
        TSE_ASSERT(res);
        if ((startCurrentLeg == startFULeg) && (endCurrentLeg == endFULeg))
        {
          foundFB = true;
          fuVect.push_back(fareUsage);
        }
      }
    }
  }

  if (!foundFB || fuVect.size() < 2)
  {
    if (diag.isActive())
    {
      diag << " SAME FARE BREAK NOT FOUND\n";
    }
    return false;
  }

  if (!_accompaniedTravel->validate(_trx, fuVect))
  {
    LOG4CXX_INFO(_logger, "AccompaniedTravel FAILED, Invalid GroupFarePath");
    if (diag.isActive())
    {
      diag << " INVALID GROUP FP: FAILED CAT13\n";
    }
    return false;
  }

  return true;
}

void
GroupFarePathFactory::GETFPInput::performTask()
{
  try
  {
    if (primaryFPPQItem == nullptr)
    {
      fppqItem = pfpf->getFPPQItem(idx, *(diag));
    }
    else
    {
      // search Same FareBreak
      fppqItem = pfpf->getSameFareBreakFPPQItem(primaryFPPQItem, *(diag));
    }
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_INFO(_logger, "Exception:" << ex.message() << "  GetFPPQItem failed");
    errResponseCode = ex.code();
    errResponseMsg = ex.message();
    done = false;
    return;
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << e.what() << " - GetFPPQItem failed");
    errResponseCode = ErrorResponseException::SYSTEM_EXCEPTION;
    errResponseMsg = std::string("ATSEI SYSTEM EXCEPTION");
    done = false;
    return;
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "UNKNOWN Exception: GetFPPQItem failed");
    errResponseCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    errResponseMsg = std::string("UNKNOWN EXCEPTION");
    done = false;
    return;
  }

  if (fppqItem != nullptr)
  {
    done = true;
  }
  else
  {
    LOG4CXX_INFO(_logger, "GetFPPQItem failed");
  }
}

bool
GroupFarePathFactory::GETFPInput::getAlreadyBuiltFPPQItem()
{
  fppqItem = pfpf->getAlreadyBuiltFPPQItem(idx);

  if (fppqItem != nullptr)
  {
    done = true;
    return true;
  }
  return false;
}

void
GroupFarePathFactory::displayGroupFP(const GroupFarePath& gfpath, DiagCollector& diag) const
{
  if (LIKELY(!diag.isActive()))
    return;

  diag << " GROUP TOTAL AMOUNT: " << gfpath.getTotalNUCAmount() << "\n";
  for (const auto fppqItem : gfpath.groupFPPQItem())
  {
    diag << *fppqItem->farePath();
    diag << " ---------\n";
  }
}

bool
GroupFarePathFactory::isMultiOptionRequest(const GroupFarePath& gfpath) const
{
  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(_trx)))
  {
    if (!gfpath.groupFPPQItem().front()->farePath()->rebookClassesExists())
    {
      return false;
    }

    return true;
  }

  if (_trx.getTrxType() != PricingTrx::MIP_TRX)
  {
    return false;
  }

  if (_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing())
    return true;

  FarePath* farePath = gfpath.groupFPPQItem().front()->farePath();
  Itin& curItin = *(farePath->itin());

  if (!_trx.isAltDates())
    return !curItin.getSimilarItins().empty();

  std::pair<DateTime, DateTime>* itinDatePair = curItin.datePair();
  if (itinDatePair == nullptr)
  {
    return false;
  }

  AltDateCellCompare altDateCellCompare(*itinDatePair);
  const PricingTrx::AltDatePairs::iterator itEnd = _trx.altDatePairs().end();
  PricingTrx::AltDatePairs::iterator it =
      std::find_if(_trx.altDatePairs().begin(), itEnd, altDateCellCompare);

  if (it != itEnd)
  {
    if (it->second->numOfSolutionNeeded > 0)
    {
      checkAltDateSolution(curItin, *farePath, *it->second);
    }
    else
    {
      // This Date is Already priced, Should not be here ...
      LOG4CXX_DEBUG(_logger, " This Date is Already priced, Should not be here ...??? ")
    }
  }

  if (!(_trx.getRequest()->owPricingRTTaxProcess() ||
        AirlineShoppingUtils::enableItinLevelThreading(_trx)))
    it = _trx.altDatePairs().begin();

  for (; it != itEnd; ++it)
  {
    if (it->second->numOfSolutionNeeded > 0)
    {
      // Not all Dates are priced yet, need to continue more
      LOG4CXX_DEBUG(_logger,
                    "Date Not priced:"
                        << " DTP:" << it->first.first << " " << it->first.second)
      return true;
    }
    else
    {
      // This Date is Already priced. Is there any date not priced yet?
      LOG4CXX_DEBUG(_logger,
                    " Date priced:"
                        << " DTP:" << it->first.first << " " << it->first.second)
      if ((_trx.getRequest()->owPricingRTTaxProcess() ||
           AirlineShoppingUtils::enableItinLevelThreading(_trx)))
        return false;
    }
  }

  return false;
}

void
GroupFarePathFactory::checkAltDateSolution(Itin& curItin,
                                           FarePath& farePath,
                                           PricingTrx::AltDateInfo& altDateInfo) const
{
  if (_numTaxCallForMipAltDate > 0)
  {
    const bool sameCarrier = (altDateInfo.taxCxr == curItin.validatingCarrier());

    if (!sameCarrier)
    {
      if (altDateInfo.taxCxr.empty())
      {
        altDateInfo.taxCxr = curItin.validatingCarrier();
        altDateInfo.taxItin.push_back(&curItin);
      }
    }

    MoneyAmount taxAmt = getTax(curItin, &farePath);

    if ((taxAmt != 0) && (taxAmt + farePath.getTotalNUCAmount() < altDateInfo.totalPlusTax))
    {
      altDateInfo.numOfSolutionNeeded = 0;
      if (!altDateInfo.taxItin.empty())
      {
        std::vector<Itin*>::const_iterator taxIt = altDateInfo.taxItin.begin();
        const std::vector<Itin*>::const_iterator taxItEnd = altDateInfo.taxItin.end();
        for (; taxIt != taxItEnd; ++taxIt)
        {
          (*taxIt)->farePath().clear();
        }
        altDateInfo.taxItin.clear();
      }
    }
    else
    {
      if (sameCarrier)
      {
        ++altDateInfo.sameCxrCnt;
      }
      else
      {
        ++altDateInfo.differentCxrCnt;
      }

      if (altDateInfo.totalPlusTax == 0)
      {
        altDateInfo.totalPlusTax = taxAmt + farePath.getTotalNUCAmount();
      }
      else
      {
        curItin.errResponseCode() = ErrorResponseException::INVALID_NUMBER_FOR_ALT_DATES;
      }

      if (sameCarrier)
      {
        if (altDateInfo.sameCxrCnt > _numTaxCallForMipAltDate)
        {
          altDateInfo.numOfSolutionNeeded = 0;
        }
      }
      else
      {
        if (altDateInfo.differentCxrCnt > _numTaxCallForMipAltDate)
        {
          altDateInfo.numOfSolutionNeeded = 0;
        }
      }
    }
  }
  else
  {
    altDateInfo.numOfSolutionNeeded = 0;
    altDateInfo.pendingSolution = true;
  }
}

void
GroupFarePathFactory::setThroughFarePricing()
{
  if (_throughFarePricing)
    return;

  for (const auto paxFarePathFactory : _paxFarePathFactoryBucket)
    paxFarePathFactory->setThroughFarePricing();

  _throughFarePricing = true;
}

bool
GroupFarePathFactory::startMultiPaxShortCkt()
{
  if (_trx.getTrxType() == PricingTrx::IS_TRX)
  {
    // MultiPaxShortCkt logic is Not for Shopping IS
    return false;
  }

  if (UNLIKELY(_trx.paxType().size() <= 1 || _trx.isBrandsForTnShopping()))
    return false;

  if (LIKELY(_runningMultiPaxShortCktLogic))
    return true;

  if ((time(nullptr) - _startTime) > _multiPaxShortCktTimeOut)
  {
    _runningMultiPaxShortCktLogic = true;
    return true;
  }

  return false;
}

bool
GroupFarePathFactory::isDivideParty(const GroupFarePath& gfpath) const
{
  if (_trx.getTrxType() != PricingTrx::PRICING_TRX && _trx.getTrxType() != PricingTrx::MIP_TRX)
    return false;

  if (_trx.paxType().size() <= 1)
    return false;

  if (UNLIKELY(!_trx.getRequest()->isLowFareRequested() && !_trx.getRequest()->isLowFareNoAvailability() &&
      _trx.getOptions()->returnAllData() != NCB))
    return false;

  std::vector<FarePath*> gfp;
  for (const auto fppqItem : gfpath.groupFPPQItem())
    gfp.push_back(fppqItem->farePath());

  return PricingUtil::isDivideParty(_trx, gfp);
}

/*-----------------------------------------------------------------------------*
 *                                                                             *
 * Fail the combination when asked to inhibit divide party even if this is     *
 * the cheapest combination.                                                   *
 *                                                                             *
 *-----------------------------------------------------------------------------*/
bool
GroupFarePathFactory::inhibitDivideParty(const GroupFarePath* gfpath) const
{
  if (gfpath == nullptr)
    return false;

  if (UNLIKELY(_trx.getTrxType() != PricingTrx::MIP_TRX))
    return false;

  if (gfpath->divideParty() && _trx.getOptions()->inhibitSplitPNR())
    return true;

  return false;
}

/*-----------------------------------------------------------------------------*
 * check currency only for ADT/CNN/INF PaxType Combination                     *
 *-----------------------------------------------------------------------------*/
bool
GroupFarePathFactory::currencyMatchingNeeded() const
{
  if (_trx.paxType().size() <= 1)
    return false;

  for (const auto paxType : _trx.paxType())
  {
    const PaxTypeCode& reqPaxType = paxType->paxType();
    if (reqPaxType != ADULT && reqPaxType != MIL && reqPaxType != SRC && reqPaxType != CHILD &&
        reqPaxType != INFANT && reqPaxType != INS && reqPaxType != SNN &&
        (!isalpha(reqPaxType[0]) || !isDigit(reqPaxType[1]) || !isDigit(reqPaxType[2])))
    {
      // pax type is not ADT/MIL/SRC/CNN/INF/INS/SNN, C10 etc
      return false;
    }
  }

  return true;
}

/*-----------------------------------------------------------------------------*
 * check currency only for ADT/CNN/INF PaxType Combination                     *
 *-----------------------------------------------------------------------------*/
bool
GroupFarePathFactory::checkCurrency(const GroupFarePath& gfpath, DiagCollector& diag) const
{
  if (_matchCurrency == false)
  {
    // no need to match currency
    return true;
  }

  bool sortingNeeded = false;
  std::vector<FPPQItem*>::const_iterator it = gfpath.groupFPPQItem().begin();
  const std::vector<FPPQItem*>::const_iterator itEnd = gfpath.groupFPPQItem().end();
  const FPPQItem& item = **it;
  const PUPath* puPath = item.puPath();
  ++it;
  for (; it != itEnd; ++it)
  {
    if (puPath->fareMarketPath() != (*it)->puPath()->fareMarketPath())
    {
      // When diff fare-breaks are used, no need to match currency
      return true;
    }
    if (UNLIKELY(puPath != (*it)->puPath()))
    {
      sortingNeeded = true;
    }
  }
  std::map<uint16_t, const PaxTypeFare*> sortedFCPrimary;
  if (UNLIKELY(sortingNeeded))
  {
    getSortedFC(*item.farePath(), sortedFCPrimary);
  }

  it = gfpath.groupFPPQItem().begin();
  ++it;
  for (; it != itEnd; ++it)
  {
    if (UNLIKELY(sortingNeeded))
    {
      std::map<uint16_t, const PaxTypeFare*> sortedFCNextPaxType;
      getSortedFC(*(*it)->farePath(), sortedFCNextPaxType);

      std::map<uint16_t, const PaxTypeFare*>::const_iterator i = sortedFCPrimary.begin();
      std::map<uint16_t, const PaxTypeFare*>::const_iterator iE = sortedFCPrimary.end();
      std::map<uint16_t, const PaxTypeFare*>::const_iterator j = sortedFCNextPaxType.begin();
      std::map<uint16_t, const PaxTypeFare*>::const_iterator jE = sortedFCNextPaxType.end();

      for (; i != iE && j != jE; ++i, ++j)
      {
        if ((*i).second->currency() != (*j).second->currency())
        {
          return false;
        }
      }
    }
    else
    {
      std::vector<PricingUnit*>::const_iterator i = item.farePath()->pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator iEnd = item.farePath()->pricingUnit().end();
      std::vector<PricingUnit*>::const_iterator j = (*it)->farePath()->pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator jEnd = (*it)->farePath()->pricingUnit().end();
      for (; i != iEnd && j != jEnd; ++i, ++j)
      {
        std::vector<FareUsage*>::const_iterator m = (*i)->fareUsage().begin();
        std::vector<FareUsage*>::const_iterator mEnd = (*i)->fareUsage().end();
        std::vector<FareUsage*>::const_iterator n = (*j)->fareUsage().begin();
        std::vector<FareUsage*>::const_iterator nEnd = (*j)->fareUsage().end();
        for (; m != mEnd && n != nEnd; ++m, ++n)
        {
          if ((*m)->paxTypeFare()->currency() != (*n)->paxTypeFare()->currency())
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void
GroupFarePathFactory::getSortedFC(const FarePath& fpath,
                                  std::map<uint16_t, const PaxTypeFare*>& sortedFC) const
{
  const Itin& itin = *fpath.itin();
  for (const auto pricingUnit : fpath.pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      const uint16_t pos = itin.segmentOrder(fareUsage->travelSeg().front());
      sortedFC.insert(
          std::map<uint16_t, const PaxTypeFare*>::value_type(pos, fareUsage->paxTypeFare()));
    }
  }
}

bool
GroupFarePathFactory::isEqualToTopOrLastGroupFarePath(const MoneyAmount lastAmount) const
{
  const GroupFarePath* gfpath = nullptr;
  if (_groupFarePathPQ.empty())
  {
    gfpath = _groupFarePathPQ.lastDequeued();
  }
  else
  {
    gfpath = _groupFarePathPQ.top();
  }

  if (!gfpath)
  {
    return false;
  }

  const MoneyAmount diff = gfpath->getTotalNUCAmount() - lastAmount;
  if ((diff > EPSILON) || (-diff > EPSILON))
  {
    return false;
  }
  return true;
}

MoneyAmount
GroupFarePathFactory::getTax(Itin& curItin, FarePath* farePath) const
{
  MoneyAmount taxAmt = 0;
  if (!curItin.farePath().empty())
    return taxAmt;

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(_trx.dataHandle(), taxFactoryMap);

  TaxItinerary taxItinerary;
  curItin.farePath().push_back(farePath);
  taxItinerary.initialize(_trx, curItin, taxFactoryMap);
  taxItinerary.accumulator();
  curItin.farePath().clear();

  if (curItin.getTaxResponses().empty())
    return taxAmt;

  const TaxResponse* taxResponse = curItin.getTaxResponses().front();
  if (taxResponse == nullptr)
    return taxAmt;

  for (const auto taxItem : taxResponse->taxItemVector())
    taxAmt += taxItem->taxAmount();

  return taxAmt;
}

void
GroupFarePathFactory::disableFurtherCmdPrcSolution() const
{
  for (const auto fareMarket : _trx.itin().front()->fareMarket())
  {
    if (fareMarket->fbcUsage() != COMMAND_PRICE_FBC)
      continue;

    if (!fareMarket->fareCalcFareAmt().empty())
      continue; // do nothing for dummy fares

    fareMarket->fareBasisCode() = "";
    if (UNLIKELY(fareMarket->isMultiPaxUniqueFareBasisCodes()))
      fareMarket->getMultiPaxUniqueFareBasisCodes().clear();
  }
}

void
GroupFarePathFactory::restoreCmdPrcInfo() const
{
  if (_trx.isMultiPassengerSFRRequestType())
    return;

  for (const auto fareMarket : _trx.itin().front()->fareMarket())
  {
    if (UNLIKELY(fareMarket->fbcUsage() != COMMAND_PRICE_FBC || !fareMarket->fareBasisCode().empty() ||
        fareMarket->breakIndicator()))
      continue;

    if (UNLIKELY(!fareMarket->fareCalcFareAmt().empty()))
      continue; // do nothing for dummy fares

    for (const auto travelSegment : fareMarket->travelSeg())
    {
      if (UNLIKELY(!travelSegment->fareBasisCode().empty()))
      {
        fareMarket->fareBasisCode() = travelSegment->fareBasisCode();
        break;
      }
    }
  }
}

void
GroupFarePathFactory::prepareInfantFactoryIndicators()
{
  _infantFactoryInd.resize(_totalFPFactory);

  for (uint16_t factId = 0; factId < _totalFPFactory; ++factId)
  {
    PaxFarePathFactoryBase* pfpf = _paxFarePathFactoryBucket[factId];

    bool isInfant = PaxTypeUtil::isInfant(
        _trx.dataHandle(), pfpf->paxType()->paxType(), pfpf->paxType()->vendorCode());
    _infantFactoryInd[factId] = static_cast<char>(isInfant);
  }
}

bool
GroupFarePathFactory::findNonDividePartySolution(const GroupFarePath& gfp) const
{
  std::vector<PaxType*> paxTypeVec;
  std::vector<MoneyAmount> paxTotalAmountVec;
  std::vector<FarePath*> farePathVec;
  std::vector<std::vector<BookingCode> > allPaxTypesBookingCodeVec;

  // Loop thru each pax fare path factory in the _paxFarePathFactoryBucket
  // to find the primary pax type which can be ADT or something else.
  // currently _paxFarePathFactoryBucket[0] should have the
  // primary pax type
  PaxType* primaryPaxType = nullptr;
  for (const auto paxFarePathFactory : _paxFarePathFactoryBucket)
  {
    if (paxFarePathFactory->primaryPaxType())
    {
      primaryPaxType = paxFarePathFactory->paxType();
      break;
    }
  }

  if (!primaryPaxType)
    return false;

  // Obtain each passenger type,  each passenger's total nuc amount,
  // and each fare path information from GroupFarePath, and store them
  // into paxTypeVec, paxTotalAmountVec, and farePathVec separately.
  //
  // Also, paxTypeVec[0], paxTotalAmountVec[0], and farePathVec[0]
  // will contain the information for the primary pax type.
  bool rc = getAllDataFromGFP(gfp, *primaryPaxType, paxTypeVec, paxTotalAmountVec, farePathVec);
  if (!rc)
    return false;

  // For each passenger type, store its booking codes
  // into the allPaxTypesBookingCodeVec. allPaxTypesBookingCodeVec[0]
  // will have the booking codes for primary pax type
  allPaxTypesBookingCodeVec.resize(farePathVec.size());
  getAllFPBookingCodes(farePathVec, allPaxTypesBookingCodeVec);

  for (const auto paxFPFactory : _paxFarePathFactoryBucket)
  {
    // We only need to search the non primary passenger type's
    // booking codes to see if they match with primary pax type's
    // booking code.
    if (!paxFPFactory->primaryPaxType())
    {
      bool searchNextPaxFPFactory = false;
      uint16_t j = 1; // starting from paxTypeVec[1] because no need
      // for paxTypeVec[0] which is primary type

      while ((j < paxTypeVec.size()) && (!searchNextPaxFPFactory))
      {
        if (paxTypeVec[j]->paxType() == paxFPFactory->paxType()->paxType())
        {
          for (FarePathFactory* fpf : paxFPFactory->farePathFactoryBucket())
          {
            if (findBookingCodesinFPF(fpf, paxTotalAmountVec[j], allPaxTypesBookingCodeVec[0]))
            {
              // For this pax type, we found the booking codes that match with
              // the primary pax type's booking codes and with the same price.
              // So go check another pax type to see if it has the same booking
              // code and the same price
              searchNextPaxFPFactory = true;
              break;
            }
          }

          if (searchNextPaxFPFactory == false)
          {
            // If we get here, none of the fare path factories has the booking codes
            return false;
          }
        }
        ++j;
      } // end of while loop
    } // end of if
  } // end of for loop

  return true;
}

bool
GroupFarePathFactory::getAllDataFromGFP(const GroupFarePath& gfp,
                                        const PaxType& primaryPaxType,
                                        std::vector<PaxType*>& ptVec,
                                        std::vector<MoneyAmount>& paxTotalAmountVec,
                                        std::vector<FarePath*>& farePathVec) const
{
  bool foundPrimaryPaxType = false;

  // store the primary data into the ptVec[0],  paxTotalAmountVec[0],
  // and farePathVec[0] first.
  for (const auto fppqItem : gfp.groupFPPQItem())
  {
    if (fppqItem->farePath()->paxType()->paxType() == primaryPaxType.paxType())
    {
      ptVec.push_back(fppqItem->farePath()->paxType());
      paxTotalAmountVec.push_back(fppqItem->farePath()->getTotalNUCAmount());
      farePathVec.push_back(fppqItem->farePath());
      foundPrimaryPaxType = true;
      break;
    }
  }

  if (foundPrimaryPaxType)
  {
    // store the rest into ptVec[1..N], paxTotalAmountVec[1..N], and
    // farePathVec[1..N] next
    for (const auto fppqItem : gfp.groupFPPQItem())
    {
      if (fppqItem->farePath()->paxType()->paxType() != primaryPaxType.paxType())
      {
        ptVec.push_back(fppqItem->farePath()->paxType());
        paxTotalAmountVec.push_back(fppqItem->farePath()->getTotalNUCAmount());
        farePathVec.push_back(fppqItem->farePath());
      }
    }
  }

  return foundPrimaryPaxType;
}

void
GroupFarePathFactory::getAllFPBookingCodes(
    const std::vector<FarePath*>& farePathVec,
    std::vector<std::vector<BookingCode>>& allPaxTypesBookingCodeVec) const
{
  for (uint16_t i = 0; i < farePathVec.size(); ++i)
    for (const PricingUnit* pu : farePathVec[i]->pricingUnit())
      for (const FareUsage* fu : pu->fareUsage())
        for (const PaxTypeFare::SegmentStatus& ss : fu->segmentStatus())
          allPaxTypesBookingCodeVec[i].push_back(ss._bkgCodeReBook);
}

bool
GroupFarePathFactory::findBookingCodesinFPF(FarePathFactory* fpf,
                                            const MoneyAmount& topAmount,
                                            const std::vector<BookingCode>& desiredBookingCode)
    const
{
  const FPPQItem* fppqItem = fpf->lowerBoundFPPQItem();
  if (!fppqItem)
    return false;

  const FarePath* fp = fppqItem->farePath();
  if (!fp)
    return false;

  if (fp->getTotalNUCAmount() > topAmount)
    return false;

  std::vector<BookingCode> fmBookingCodes(desiredBookingCode.size());

  for (const PricingUnit* pricingUnit : fp->pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      findBookingCodesInFM(
          fareUsage->paxTypeFare()->fareMarket(), fp->itin(), fmBookingCodes, desiredBookingCode);

      if (fmBookingCodes == desiredBookingCode)
        return true;
    }
  }

  return false;
}

void
GroupFarePathFactory::findBookingCodesInFM(const FareMarket* fareMarket,
                                           const Itin* itin,
                                           std::vector<BookingCode>& fmBookingCodes,
                                           const std::vector<BookingCode>& desiredBookingCode) const
{
  std::vector<int> segmentOrders;

  for (const TravelSeg* ts : fareMarket->travelSeg())
  {
    if (static_cast<uint16_t>(itin->segmentOrder(ts)) > desiredBookingCode.size())
      return;
    segmentOrders.push_back(itin->segmentOrder(ts));
  }

  for (const auto paxTypeFare : fareMarket->allPaxTypeFare())
  {
    if ((!paxTypeFare) || (!paxTypeFare->isValid()) ||
        (segmentOrders.size() != paxTypeFare->segmentStatus().size()))
      continue;

    bool checkNextPtf = false;
    uint16_t i = 0;

    while ((checkNextPtf == false) && (i < paxTypeFare->segmentStatus().size()))
    {
      int segmentOrder = segmentOrders[i];
      if (paxTypeFare->segmentStatus()[i]._bkgCodeReBook != desiredBookingCode[segmentOrder - 1])
      {
        checkNextPtf = true;
        break;
      }
      ++i;
    }

    if (checkNextPtf == false)
    {
      // this pax type fare has all the matched booking codes
      // so copy the booking codes into fmBookingCodes and return
      for (i = 0; i < paxTypeFare->segmentStatus().size(); ++i)
      {
        int segmentOrder = segmentOrders[i];
        fmBookingCodes[segmentOrder - 1] = paxTypeFare->segmentStatus()[i]._bkgCodeReBook;
        return;
      }
    }
  }
}

#if 0

/***********************************************************************
 For Debug only
 ***********************************************************************/

void forDebugOnlyDisplayFPIndices(std::vector<uint32_t>& v, std::string& str)
{
        std::vector<uint32_t>::iterator it = v.begin();
        const std::vector<uint32_t>::iterator itEnd = v.end();
        char idxStr[8];
        for(; it != itEnd; ++it)
        {
          sprintf(idxStr, "%d ", *it);
          str += std::string(idxStr);
        }
}

#endif
}
