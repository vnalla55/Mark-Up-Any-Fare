// vim:ts=2:sts=2:sw=2:cin:et
// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/PQ/SoloPQ.h"

#include "Common/Assert.h"
#include "Common/BookingCodeUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/FarePathFactoryPQItem.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);

namespace
{
ConfigurableValue<int32_t>
supressLocalThresholdTimeoutPercentage("SHOPPING_DIVERSITY",
                                       "SUPPRESS_LOCAL_THRESHOLD_TIMEOUT_PERCENTAGE");
ConfigurableValue<uint64_t>
maxNotUsedFPsCfg("SHOPPING_DIVERSITY", "MAX_NOT_USED_FAREPATH", 5000);
ConfigurableValue<uint64_t>
maxFailedFPsCfg("SHOPPING_DIVERSITY", "MAX_FAILED_FAREPATH", 10000);
ConfigurableValue<uint64_t>
uniqueOutboundSchedulesCfg("SHOPPING_DIVERSITY", "SUPPRESS_LOCAL_THRESHOLD_OUTBOUND_SCHEDULE");
}

namespace shpq
{
namespace
{
Logger
logger("atseintl.ShoppingPQ.SoloPQ");

time_t
getHurryOutTime(const ShoppingTrx& trx)
{
  time_t suppressLocalTime = 0;
  int hurryPercentTreshold = 0;
  bool hurryUpLogicEnabled = true;
  hurryPercentTreshold = supressLocalThresholdTimeoutPercentage.getValue();
  if (supressLocalThresholdTimeoutPercentage.isDefault())
  {
    hurryUpLogicEnabled = false;
  }

  const TrxAborter* aborter = trx.aborter();
  if (hurryUpLogicEnabled && (aborter != nullptr) && (aborter->timeout() > 0))
  {
    suppressLocalTime =
        aborter->getTimeOutAt() - ((aborter->timeout() * (100 - hurryPercentTreshold)) / 100);
  }
  return suppressLocalTime;
}

size_t
getUniqueOBSchedules(const ShoppingTrx& trx)
{
  // Skip this config value for one way requests.
  return trx.journeyItin()->travelSeg().size() != 1 ? uniqueOutboundSchedulesCfg.getValue() : 0;
}
}

SoloPQ::SoloPQ(ShoppingTrx& trx, const ItinStatistic& stats, DiagCollector* diag942)
  : _trx(trx),
    _diagCollector(trx, diag942),
    _hurryOutTime(getHurryOutTime(trx)),
    _uniqueOBSchedules(getUniqueOBSchedules(trx)),
    _maxNotUsedFPs(maxNotUsedFPsCfg.getValue()),
    _maxFailedFPs(maxFailedFPsCfg.getValue()),
    _stats(stats)
{
  if (trx.isThroughFarePrecedencePossible() &&
      _trx.diagnostic().diagnosticType() == Diagnostic910)
  {
    _diag910 = static_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
    _diag910->activate();
  }
}

SoloPQ::~SoloPQ()
{
  LOG4CXX_TRACE(logger, "SoloPQ processing stopped, current PQ size:" << size());
  _diagCollector.displayNoOfExpansions(size());
}

void
SoloPQ::enqueue(const SoloPQItemPtr& item, const SoloPQItem* const expandedFrom)
{
  _pq.push(item);
  _diagCollector.onPQEnqueue(item, expandedFrom);
  LOG4CXX_TRACE(logger, "SoloPQ::enqueue() (" << size() << ") " << item << ":" << item->str());
}

SoloPQItemPtr
SoloPQ::dequeue()
{
  SoloPQItemPtr result = _pq.top();
  _pq.pop();
  _diagCollector.onPQDequeue(result);
  LOG4CXX_TRACE(logger, "SoloPQ::dequeue() (" << size() << ") " << result << ":" << result->str());
  return result;
}

SoloPQItemPtr
SoloPQ::peek() const
{
  SoloPQItemPtr result = _pq.top();
  // _diagCollector.onPQPeek(result)
  LOG4CXX_TRACE(logger, "SoloPQ::peek() (" << size() << ") " << result << ":" << result->str());
  return result;
}

size_t
SoloPQ::size() const
{
  return _pq.size();
}

bool
SoloPQ::empty() const
{
  return _pq.empty();
}

//---------------------------------------------------------------------------
/// Suppress the SOL Solution Pattern in PO once a configurable CPU time
/// has been reached or a configurable number of OB flight has been generated.
//---------------------------------------------------------------------------
bool
SoloPQ::skipLocalPattern(const SoloPQItemPtr& item)
{
  _onlyThruFM = _onlyThruFM || checkFailedFPs() || checkOBSchedules() || checkHurryOut();
  bool skip = (_onlyThruFM && !item->getSolPattern()->isThruPattern());
  if (skip)
    _diagCollector.onSkipLocalPattern(item);
  return skip;
}

bool
SoloPQ::checkHurryOut()
{
  const bool result = (_hurryOutTime && (time(nullptr) >= _hurryOutTime));
  if (result)
  {
    // Return ShoppingResponse/@PHL="T" in IS response if hurry out condition met
    TrxAborter* aborter = _trx.aborter();
    if (aborter)
      aborter->setHurryLogicActivatedFlag();
    _diagCollector.onSOLHurryOutCondition();
  }
  return result;
}

bool
SoloPQ::checkOBSchedules()
{
  const bool result = (_uniqueOBSchedules && (_stats.getNumOfUniqueSops(0) >= _uniqueOBSchedules));
  if (UNLIKELY(result))
    _diagCollector.onOBSchedulesCondition();
  return result;
}

bool
SoloPQ::checkNotUsedCondition()
{
  bool continueProcessing = true;
  if (_lastOptionsCount == _stats.getProcessedOptionsCount())
  {
    ++_notUsedFPs;
  }
  else
  {
    _notUsedFPs = 0;
  }

  if (UNLIKELY(_notUsedFPs >= _maxNotUsedFPs && !_onlyThruFM))
  {
    _onlyThruFM = true;
    _notUsedFPs = 0;
  }

  if (_notUsedFPs >= _maxNotUsedFPs)
  {
    continueProcessing = false;
    _dm->handlePQStop();
    _diagCollector.onNotUsedFPCondtion(_notUsedFPs);
  }

  _lastOptionsCount = _stats.getTotalOptionsCount();
  return continueProcessing;
}

bool
SoloPQ::checkFailedFPs()
{
  const bool result = _failedFPs >= _maxFailedFPs;
  if (UNLIKELY(result))
    _diagCollector.onFailedFPCondition(_failedFPs);
  return result;
}

SoloPQItemPtr
SoloPQ::getNextFarepathCapableItem(SoloTrxData& soloTrxData)
{
  TSELatencyData metrics(soloTrxData.getTrx(), "SOLOPQ GETNEXTFAREPATHCAPABLEITEM");
  LOG4CXX_TRACE(logger, "SoloPQ::getNextFarepathCapableItem()");

  if (!checkNotUsedCondition())
    return FarePathFactoryPQItemPtr();

  const bool tfp = _trx.isThroughFarePrecedencePossible();

  while (!empty() && (fallback::reworkTrxAborter(&_trx) ? !checkTrxMustHurry(soloTrxData.getTrx())
                                                        : !_trx.checkTrxMustHurry()))
  {
    if (fallback::reworkTrxAborter(&soloTrxData.getTrx()))
      checkTrxAborted(soloTrxData.getTrx());
    else
      soloTrxData.getTrx().checkTrxAborted();

    SoloPQItemPtr item = dequeue();

    if (skipLocalPattern(item))
      continue;

    DiversityModel::PQItemAction action = _dm->getPQItemAction(item.get());
    switch (action)
    {
    case DiversityModel::SKIP:
      continue;
    case DiversityModel::STOP:
      return FarePathFactoryPQItemPtr();
    case DiversityModel::USE:
      ;
      /* no break */
    }

    checkDirectFlightsTuning(item);
    if (soloTrxData.getShoppingTrx().diversity().hasDCL() &&
       (item->getLevel() == shpq::SoloPQItem::CRC_LEVEL))
    {
      shpq::ConxRouteCxrPQItem* crcPqItem = static_cast<shpq::ConxRouteCxrPQItem*>(item.get());

      ShoppingTrx& shpTrx = soloTrxData.getShoppingTrx();
      if (shpTrx.diversity().getFareCutoffAmount() > EPSILON &&
          crcPqItem->getScore() > shpTrx.diversity().getFareCutoffAmount())
      {
        if (crcPqItem->shouldSkip(soloTrxData, _stats, *this) == true)
          continue;
      }
    }
    item->expand(soloTrxData, *this);

    // NOTE: item->getFarePath() should be called after item->expand().
    //       Before expand() is called, getFarePath() is NULL.
    //       After expand() is called, getFarePath() is not NULL if:
    //         - item is FarePathFactoryPQItem,
    //         - and, obviously, a fare path has got produced.
    if (item->getFarePath())
    {
      if (UNLIKELY(tfp))
      {
        if (isThroughFarePrecedenceCompatible(*item))
          continue;
      }

      return item;
    }
  }

  if (empty())
    _diagCollector.onPQEmpty();
  else
  {
    _diagCollector.onHurryOutCondition();
    _dm->handlePQHurryOutCondition();
  }

  LOG4CXX_TRACE(logger, "SoloPQ::getNextFarepathCapableItem(): no farepath found");
  return FarePathFactoryPQItemPtr();
}

void
SoloPQ::checkDirectFlightsTuning(const SoloPQItemPtr& item)
{
  FarePathFactoryPQItem* fpfItem = dynamic_cast<FarePathFactoryPQItem*>(item.get());
  if (!fpfItem)
    return;

  bool prevalidateDirectFlights;

  prevalidateDirectFlights = _dm->isNonStopNeededOnlyFrom(item.get());

  fpfItem->setPrevalidateDirectFlights(prevalidateDirectFlights);
  fpfItem->setPerformNSPreValidation(prevalidateDirectFlights &&
                                     _dm->shouldPerformNSPreValidation(fpfItem));
}

bool
SoloPQ::isThroughFarePrecedenceCompatible(const SoloPQItem& item)
{
  if (item.getSolPattern()->isThruPattern())
    return true;

  FarePath& farePath = *item.getFarePath();

  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (!fareUsage->paxTypeFare()->fareMarket()->isThroughFarePrecedenceNGS())
      {
        // One of the fare markets does not need TFP; it means that the whole fare path doesn't
        // need to be checked.
        return true;
      }

      const size_t segSize = fareUsage->travelSeg().size();
      TSE_ASSERT(segSize > 0);
      const BookingCode bookingCode = BookingCodeUtil::getFuSegmentBookingCode(*fareUsage, 0);

      for (size_t index = 1; index < segSize; ++index)
      {
        if (BookingCodeUtil::getFuSegmentBookingCode(*fareUsage, index) != bookingCode)
        {
          // Fare markets with mixed booking codes do not need TFP.
          return true;
        }
      }
    }
  }

  // We are here means, all of the Fare Market are TFP compatible and have same booking code.
  // Log the diag 910 with details about invalid Fare Path.
  if (UNLIKELY(_diag910))
  {
    if (!_trx.isThroughFarePrecedenceFoundInvalidFarePath())
    {
      *_diag910 << "VALIDATION FOR THROUGH FARE PRECEDENCE: FOLLOWING INVALID FARE PATHS NEED "
                   "THROUGH FARE PRECEDENCE BUT ARE USING LOCAL MARKETS:" << std::endl;
      *_diag910 << "--------------------------------------------------------------------------"
                   "----------------------------------------------------" << std::endl;
      *_diag910 << "   LEG PU CX/FARE BASIS           /TOTAL AMOUNT/ AMT CUR/GI" << std::endl;
      *_diag910 << "-----------------------------------------------------------" << std::endl;
      _diag910->flushMsg();
    }

    _diag910->displayFarePath(_trx, farePath);
    _diag910->flushMsg();
  }

  _trx.setThroughFarePrecedenceFoundInvalidFarePath(true);
  return false;
}
}
} // namespace tse::shpq
