#include "Pricing/ShoppingPaxFarePathFactory.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "Fares/BitmapOpOrderer.h"
#include "Pricing/ShoppingFarePathFactory.h"
#include "Pricing/ShoppingPQ.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackShoppingPQCabinClassValid);

namespace
{
ConfigurableValue<uint32_t>
minCellsToValidate("SHOPPING_OPT", "MIN_FLIGHT_MATRIX_CELLS", 1000);
ConfigurableValue<uint32_t>
maxCellsToValidate("SHOPPING_OPT", "MAX_FLIGHT_MATRIX_CELLS", 10000);

ConfigurableValue<uint32_t>
minCellsToValidateAltDates("SHOPPING_OPT", "MIN_FLIGHT_MATRIX_CELLS_ALTDATE", 100000);
ConfigurableValue<uint32_t>
maxCellsToValidateAltDates("SHOPPING_OPT", "MAX_FLIGHT_MATRIX_CELLS_ALTDATE", 1000000);
}

Logger
ShoppingPaxFarePathFactory::_logger("atseintl.Pricing.ShoppingPaxFarePathFactory");

ShoppingPaxFarePathFactoryCreator::ShoppingPaxFarePathFactoryCreator(ShoppingTrx& trx,
                                                                     ShoppingPQ& pq,
                                                                     BitmapOpOrderer& op)
  : _trx(trx), _pq(pq), _op(op)
{
}

PaxFarePathFactory*
ShoppingPaxFarePathFactoryCreator::create(const FactoriesConfig& factoriesConfig,
                                          const PricingTrx& trx) const
{
  ShoppingPaxFarePathFactory* res =
      &trx.dataHandle().safe_create<ShoppingPaxFarePathFactory>(factoriesConfig);
  res->setShoppingTrx(&_trx);
  res->setShoppingPQ(&_pq);
  res->setBitmapOpOrderer(&_op);
  res->setminMaxCellAltDate(&_trx);
  return res;
}

ShoppingPaxFarePathFactory::ShoppingPaxFarePathFactory(const FactoriesConfig& factoriesConfig)
  : PaxFarePathFactory(factoriesConfig),
    _trx(nullptr),
    _pq(nullptr),
    _op(nullptr),
    _minCellsToValidate(minCellsToValidate.getValue()),
    _maxCellsToValidate(maxCellsToValidate.getValue()),
    _curCellsToValidate(_maxCellsToValidate)
{
}

bool
ShoppingPaxFarePathFactory::initPaxFarePathFactory(DiagCollector& diag)
{
  // Two Steps: create and init ShoppingFarePathFactory
  LOG4CXX_DEBUG(_logger, "Entered: ShoppingPaxFarePathFactory::initPaxFarePathFactory()")

  if (!createFarePathFactories(ShoppingFarePathFactoryCreator(*_trx, *_pq, *_op)))
  {
    return false;
  }

  // init each FarePathFactory  and put it in the PQ
  //
  TSELatencyData metrics(*_trx, "PO INIT PAX FP FACTORY");

  try
  {
    std::vector<FarePathFactory*>::iterator it = _farePathFactoryBucket.begin();
    std::vector<FarePathFactory*>::iterator itEnd = _farePathFactoryBucket.end();

    for (; it != itEnd; ++it)
    {
      FarePathFactory* fpf = (*it);
      const ShoppingFarePathFactory* sfpf = dynamic_cast<ShoppingFarePathFactory*>(fpf);
      TSE_ASSERT(sfpf != nullptr);
      fpf->setEoeCombinabilityEnabled(isEoeCombinabilityEnabled());

      if (fpf->initFarePathFactory(diag))
      {
        if (fpf->lowerBoundFPAmount() >= 0)
        {
          // negative -lowerBoundFPAmount means FPF failed to build any FP
          _farePathFactoryPQ.push(fpf);
        }
      }
    }
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Exception: InitFarePathFactory Failed")
    return false;
  }

  if (_farePathFactoryPQ.empty())
  {
    return false;
  }

  _reqDiagFPCount = fpCountDiagParam();
  return true;
}

bool
ShoppingPaxFarePathFactory::isFarePathValidAfterPlusUps(FPPQItem& fppqItem, DiagCollector& diag)
{
  FarePath& fpath = *fppqItem.farePath();

  if (UNLIKELY(!RuleUtil::isFarePathValidForRetailerCode(*_trx, fpath)))
  {
     _pq->farePathValidationResult(fpath, "FARE RETAILER CODE SOURCE PCC MISMATCH");
    return false;
  }

  if (UNLIKELY(
          (_pq->altDateHighestAmountAllow() > 0) &&
          ((fpath.getTotalNUCAmount() + _pq->taxAmount()) > _pq->altDateHighestAmountAllow())))
  {
    _altDateISCutOffReach = true;
    _pq->farePathValidationResult(fpath, "REACH CUT OFF AMOUNT");
    return false;
  }

  const bool res = checkFarePathHasValidCell(fppqItem);
  _pq->farePathValidationResult(fpath, res ? "PASS" : "FAIL VALID FLIGHTS");
  return res;
}

void
ShoppingPaxFarePathFactory::setShoppingTrx(ShoppingTrx* trx)
{
  _trx = trx;
}

void
ShoppingPaxFarePathFactory::setShoppingPQ(ShoppingPQ* pq)
{
  _pq = pq;
}

void
ShoppingPaxFarePathFactory::setBitmapOpOrderer(BitmapOpOrderer* op)
{
  _op = op;
}

void
ShoppingPaxFarePathFactory::setminMaxCellAltDate(ShoppingTrx* trx)
{
  if (!trx->isAltDates())
    return;
  _minCellsToValidate = minCellsToValidateAltDates.getValue();
  _maxCellsToValidate = maxCellsToValidateAltDates.getValue();
  _curCellsToValidate = _maxCellsToValidate;
}

namespace
{
void
extractFaresFromFarePath(const FarePath& path, std::vector<PaxTypeFare*>& key)
{
  key.reserve(8);
  for (const auto pu : path.pricingUnit())
  {
    for (const auto fu : pu->fareUsage())
    {
      key.push_back(fu->paxTypeFare());
    }
  }
}
}

FarePathFlightsInfo&
ShoppingPaxFarePathFactory::getFarePathFlightsInfo(FPPQItem& fppqItem, const DatePair* dates)
{
  // keep count how many fare paths tried and failed. It will be reset when good option is found.
  _pq->incrementFarePathTried();
  FarePathFlightsInfoMapKey key;
  extractFaresFromFarePath(*fppqItem.farePath(), key.first);
  key.second = dates;
  FarePathFlightsInfoMap::iterator i = _info.find(key);

  if (i == _info.end())
  {
    i = _info.insert(FarePathFlightsInfoMap::value_type(
                         key,
                         FarePathFlightsInfo(
                             *_trx, *_pq, fppqItem, *this, *_op, _curCellsToValidate, dates)))
            .first;
  }
  return i->second;
}

bool
ShoppingPaxFarePathFactory::checkFarePathHasValidCell(FPPQItem& fppqItem)
{
  const TSELatencyData metrics(*_trx, "SHOPPING FPF FIND CELL");
  std::vector<FarePathFlightsInfo*> info;

  if (_trx == nullptr || !_trx->isAltDates())
  {
    info.push_back(&getFarePathFlightsInfo(fppqItem, nullptr));
  }
  else
  {
    for (ShoppingTrx::AltDatePairs::const_iterator i = _pq->altDatePairsPQ().begin();
         i != _pq->altDatePairsPQ().end();
         ++i)
    {
      bool foundOptionDate = false;
      // This block of code will be guarded for read/write by multi-thread
      {
        boost::lock_guard<boost::mutex> guard(_trx->altDateLowestAmountMutex());
        ShoppingTrx::AltDateLowestAmount::iterator itr = _trx->altDateLowestAmount().find(i->first);

        if (LIKELY(itr != _trx->altDateLowestAmount().end()))
        {
          if ((itr->second->lowestOptionAmountForSnowman <
               (_pq->taxAmount()) + (*fppqItem.farePath()).getTotalNUCAmount()) &&
              (itr->second->lowestOptionAmount <
               (_pq->taxAmount()) + (*fppqItem.farePath()).getTotalNUCAmount()))
          {
            foundOptionDate = true;
          }
        }
      }

      if ((i->second->numOfSolutionNeeded > 0) && (!foundOptionDate))
      {
        info.push_back(&getFarePathFlightsInfo(fppqItem, &i->first));
      }
    }
  }

  bool res = false;

  if (info.empty())
  {
    _altDateISCutOffReach = true;
    FarePath& fpath = *fppqItem.farePath();
    _pq->farePathValidationResult(fpath, "FOUND OPTION FOR ALL DATES");
    return res;
  }

  for (const auto elem : info)
  {
    if (elem->getPassedSops().empty() == false || elem->findNewPassedSops(_trx))
    {
      res = true;
      break;
    }
  }

  if (res)
  {
    _curCellsToValidate = _maxCellsToValidate;
  }
  else
  {
    _curCellsToValidate /= 2;

    if (_curCellsToValidate < _minCellsToValidate)
    {
      _curCellsToValidate = _minCellsToValidate;
    }
  }

  return res;
}

//----------------------------------------------------------------------------------

int
ConnectionPointsCounter::
operator()(const std::vector<int>& pos) const
{
  std::vector<int> sops;
  _map.getSops(pos, sops);
  int res = 0;

  for (uint32_t n = 0; n != sops.size(); ++n)
  {
    TSE_ASSERT(n < _trx->legs().size()); // lint !e666 !e574
    TSE_ASSERT(sops[n] < int(_trx->legs()[n].sop().size())); // lint !e666 !e574
    res += int(_trx->legs()[n].sop()[sops[n]].itin()->travelSeg().size());
  }

  return res;
}

FarePathFlightsInfo::FarePathFlightsInfo(ShoppingTrx& trx,
                                         ShoppingPQ& pq,
                                         FPPQItem& fppqItem,
                                         ShoppingPaxFarePathFactory& spfpf,
                                         BitmapOpOrderer& op,
                                         int maxCells,
                                         const DatePair* dates)
  : _trx(trx),
    _pq(pq),
    _fppqItem(fppqItem),
    _spfpf(spfpf),
    _mapping(&trx, pq.journeyItin(), fppqItem.farePath(), &op, dates, &pq),
    _itor(_mapping.getDimensions(), ConnectionPointsCounter(trx, _mapping)),
    iter_(trx, _mapping),
    _maxCells(maxCells)
{
}

const std::set<std::vector<int> >&
FarePathFlightsInfo::getPassedSops() const
{
  return _passedSet;
}

bool
FarePathFlightsInfo::findNewPassedSops(const ShoppingTrx* trx, std::vector<int>* sops)
{
  std::vector<int>* defaultSops(nullptr);

  if (sops == nullptr)
  {
    trx->dataHandle().get(defaultSops);
    sops = defaultSops;
  }

  // allocate upto 6 legs
  uint failedCount[6] = { 0, 0, 0, 0, 0, 0 };
  uint failIndex = 0;

  while (_maxCells != 0 && iter_.next(*sops))
  {
    // check condition when called with sops is specified
    if ((sops != defaultSops) && (_pq.checkHurryCond()))
    {
      _pq.setProcessDoneWithCond();
      break;
    }

    if (!fallback::fixed::fallbackShoppingPQCabinClassValid())
    {
      if (!ShoppingUtil::isCabinClassValid(_trx, *sops))
        continue;
    }

    if (_passedSet.count(*sops) == 0 && sopsPass(*sops) &&
        _pq.getEstimateMatrix().count(*sops) == 0)
    {
      return true;
    }

    bool foundFail = false;
    const DatePair* dates = _mapping.getDates();
    std::vector<PricingUnit*>::const_iterator puIt = _fppqItem.farePath()->pricingUnit().begin();
    std::vector<PricingUnit*>::const_iterator puItEnd = _fppqItem.farePath()->pricingUnit().end();
    failIndex = 0;
    uint puIdx = 0;

    for (; puIt != puItEnd; ++puIt, ++puIdx)
    {
      std::vector<FareUsage*>::const_iterator it = (*puIt)->fareUsage().begin();
      std::vector<FareUsage*>::const_iterator itEnd = (*puIt)->fareUsage().end();

      for (; it != itEnd; ++it, failIndex++)
      {
        if ((*it)->isFailedFound())
        {
          ++failedCount[failIndex];
          (*it)->resetFailedFound();

          if (_pq.maxFailedCellsToValidate() < failedCount[failIndex])
          {
            PaxTypeFare* ptf = (*it)->paxTypeFare();
            saveFailedFare(puIdx, ptf, dates);
            foundFail = true;
          }
        }
      }
    }

    if (foundFail)
    {
      break;
    }
  }

  return false;
}

void
FarePathFlightsInfo::saveFailedFare(const uint puIdx, PaxTypeFare* ptf, const DatePair* datePair)
{
  if (datePair != nullptr)
  {
    _spfpf.saveFailedFPIS(&_fppqItem, *datePair);
    _fppqItem.farePathFactory()->saveISFailedFare(puIdx, ptf, *datePair);
  }
  else
  {
    _spfpf.saveFailedFPIS(&_fppqItem);
    _fppqItem.farePathFactory()->saveISFailedFare(puIdx, ptf);
  }
}

bool
FarePathFlightsInfo::sopsPass(const std::vector<int>& sops)
{
  std::vector<int> pos;

  if (_mapping.hasSops(sops, &pos) == false)
  {
    return false;
  }

  if (_trx.isLngCnxProcessingEnabled())
  {
    if (ShoppingUtil::isLongConnection(_trx, sops) && !_pq.checkNumOfLngCnx())
      return false;
  }

  const std::vector<int>& dim = _mapping.getDimensions();
  size_t index = 0;

  for (size_t n = 0; n != pos.size(); ++n)
  {
    TSE_ASSERT(n < dim.size());
    index = index * dim[n] + pos[n];
  }

  if (index < _validated.size() && _validated[index])
  {
    return index < _passed.size() && _passed[index];
  }

  if (index >= _validated.size())
  {
    _validated.resize(index + 1);
  }

  _validated[index] = true;

  if (_pq.isValidMatrixCell(sops) == false)
  {
    return false;
  }

  if (_pq.isFarePathValid(*_fppqItem.farePath(), sops))
  {
    _passedSet.insert(sops);

    if (index >= _passed.size())
    {
      _passed.resize(index + 1);
    }

    _passed[index] = true;
    // since we've found that at least once cell passes,
    // never give up on trying to find passing sops from this fare path
    _maxCells = -1;
    return true;
  }
  else if (_maxCells > 0)
  {
    --_maxCells;
  }

  return false;
}
}

