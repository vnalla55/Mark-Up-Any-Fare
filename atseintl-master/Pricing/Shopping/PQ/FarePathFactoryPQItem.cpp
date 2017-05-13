// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         22-11-2011
//! \file         FarePathFactoryPQItem.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/Shopping/PQ/FarePathFactoryPQItem.h"

#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/Shopping/PQ/SoloFarePathFactory.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

namespace tse
{

namespace shpq
{

namespace
{

void
completePUFactoryInit(const PUPath& puPath,
                      std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  for (PricingUnitFactoryBucket* puFactoryBucket : puFactoryBucketVect)
  {
    for (PU* const pu : puPath.puPath())
    {
      if (LIKELY(puFactoryBucket->puFactoryBucket()[pu]))
      {
        puFactoryBucket->puFactoryBucket()[pu]->initStage() = false;
      }
    }
  }
}

} // anon

Logger
FarePathFactoryPQItem::_logger("atseintl.ShoppingPQ.FarePathFactoryPQItem");

FarePathFactoryPQItem::FarePathFactoryPQItem(const ConxRouteCxrPQItem& crcPQItem,
                                             SoloTrxData& soloTrxData,
                                             PUStruct& puStruct)
  : _solutionPattern(*crcPQItem.getSolPattern()),
    _farePathFactory(nullptr),
    _farePath(nullptr),
    _score(UNKNOWN_MONEY_AMOUNT),
    _applCxrs(crcPQItem.getApplicableCxrs()),
    _prevalidateDirectFlights(false),
    _performNSPreValidation(false),
    _nsVisitor(nullptr)
{
  if (LIKELY(puStruct._itin && puStruct._puPath && puStruct._puFactoryBucketVec))
    _farePathFactory = SoloFarePathFactory::createFarePathFactory(
        soloTrxData, *puStruct._puFactoryBucketVec, puStruct);

  // negative lowerBoundFPAmount means FPF failed to build any FP,
  // see PaxFarePathFactory::initPaxFarePathFactory()
  if (_farePathFactory && _farePathFactory->initFarePathFactory(soloTrxData.getDiagCollector()) &&
      _farePathFactory->lowerBoundFPAmount() >= 0)
  {
    completePUFactoryInit(*puStruct._puPath, *puStruct._puFactoryBucketVec);
    _score = _farePathFactory->lowerBoundFPAmount();
    // NOTE: _farePath could be set here by getLBItemFarePath() (defined below
    //       in anonymous namespace), which would be an initial possibly invalid
    //       fare path, but I leave it NULL to prevent coding mistakes like
    //       calling getFarePath() if expand() hasn't been called yet.
  }
  else
  {
    // no fare paths at all can be build from this _farePathFactory.
    _farePathFactory = nullptr; // TODO: release memory somehow (?)
  }
}

FarePathFactoryPQItem::FarePathFactoryPQItem(const FarePathFactoryPQItem& other,
                                             DiagCollector& diag)
  : _solutionPattern(other._solutionPattern),
    _farePathFactory(other._farePathFactory),
    _farePath(nullptr),
    _score(UNKNOWN_MONEY_AMOUNT),
    _applCxrs(other._applCxrs),
    _prevalidateDirectFlights(false),
    _performNSPreValidation(false),
    _nsVisitor(other._nsVisitor)
{
  if (LIKELY(_farePathFactory && _farePathFactory->lowerBoundFPAmount() >= 0))
  {
    _score = _farePathFactory->lowerBoundFPAmount();
    // _farePath not set here - see comment in previous constructor.
  }
  else
  {
    // no more fare paths can be build from this _farePathFactory.
    _farePathFactory = nullptr;
  }
}

/**
 * Our goal is to find any non-stop combination which does not exists in flight matrix
 * This is similar to visitor in SoloFarePathFactory, but this one doesn't care about ptf
 */
class FarePathFactoryPQItem::CheckNSVisitor
{
public:
  explicit CheckNSVisitor(const ShoppingTrx& trx)
    : _nsSopsCollected(false), _nsCapableSopVec(trx.legs().size()), _lastValidSopComb(0), _trx(trx)
  {
  }

  bool isNSCapable()
  {
    for (const NSSopVec& legNS : _nsCapableSopVec)
    {
      if (legNS.empty())
        return false;
    }

    if (hasNSSopsCollected() &&
        _trx.flightMatrix().find(_lastValidSopComb) == _trx.flightMatrix().end())
      return true;

    if (!hasNSSopsCollected())
    {
      // transform into internal sops
      for (size_t leg = 0; leg < _nsCapableSopVec.size(); ++leg)
      {
        NSSopVec& legNS = _nsCapableSopVec[leg];
        std::transform(legNS.begin(),
                       legNS.end(),
                       legNS.begin(),
                       std::bind(&ShoppingUtil::findInternalSopId,
                                 std::cref(_trx),
                                 leg,
                                 std::placeholders::_1));
      }

      markNSSopsCollected();
    }

    std::vector<int> sopIdxVec(_trx.legs().size());
    bool result = recursivelyFindNotExistingComb(0, sopIdxVec);

    _lastValidSopComb.swap(sopIdxVec);
    return result;
  }

  void visit(uint16_t legIndex, const SOPUsage& flbSopUsage)
  {
    if (!flbSopUsage.applicable_)
      return;

    bool isNonStopSOP = (ShoppingUtil::getTravelSegCountForExternalSopID(
                             _trx, legIndex, flbSopUsage.origSopId_) == 1);

    if (!isNonStopSOP)
      return;

    NSSopVec& legNS = _nsCapableSopVec[legIndex];
    legNS.push_back(flbSopUsage.origSopId_);
  }

  void markNSSopsCollected() { _nsSopsCollected = true; }
  bool hasNSSopsCollected() const { return _nsSopsCollected; }

private:
  typedef std::vector<int> NSSopVec;

  bool recursivelyFindNotExistingComb(const size_t legIdx, std::vector<int>& sopVec) const
  {
    TSE_ASSERT(legIdx < _nsCapableSopVec.size());
    const bool isFinalLegIdx = (legIdx + 1 == _nsCapableSopVec.size());

    if (isFinalLegIdx)
    {
      for (int nextSop : _nsCapableSopVec[legIdx])
      {
        sopVec[legIdx] = nextSop;

        if (_trx.flightMatrix().find(sopVec) == _trx.flightMatrix().end())
          return true;
      }
    }
    else
    {
      for (int nextSop : _nsCapableSopVec[legIdx])
      {
        sopVec[legIdx] = nextSop;

        if (recursivelyFindNotExistingComb(legIdx + 1, sopVec))
          return true;
      }
    }

    return false;
  }

  bool _nsSopsCollected;
  std::vector<NSSopVec> _nsCapableSopVec; // per leg
  NSSopVec _lastValidSopComb; // per leg
  const ShoppingTrx& _trx;
};

bool
FarePathFactoryPQItem::canProduceNonStops(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "FPF PQITEM PRE VALIDATION");

  if (UNLIKELY(!_farePathFactory))
    return false;

  const Itin* itin = _farePathFactory->puPath()->itin();
  if (UNLIKELY(itin == nullptr))
    return false;

  if (_nsVisitor == nullptr)
    _nsVisitor = &trx.dataHandle().safe_create<CheckNSVisitor>(trx);

  if (_nsVisitor->hasNSSopsCollected())
    return _nsVisitor->isNSCapable();

  for (const FareMarket* fm : itin->fareMarket())
  {
    if (fm->getApplicableSOPs() == nullptr)
      return false;

    const uint16_t legIdx = fm->legIndex();
    for (const ApplicableSOP::value_type& cxrSops : *fm->getApplicableSOPs())
    {
      for (const SOPUsage& su : cxrSops.second)
      {
        _nsVisitor->visit(legIdx, su);
      }
    }
  }

  return _nsVisitor->isNSCapable();
}

// virtual
void
FarePathFactoryPQItem::expand(SoloTrxData& soloTrxData, SoloPQ& pq)
{
  if (UNLIKELY(!_farePathFactory))
    return;

  const MoneyAmount nextLowerBound = (pq.empty() ? -1.0 : pq.peek()->getScore());
  _farePathFactory->externalLowerBoundAmount() = nextLowerBound;

  _farePathFactory->setPrevalidateDirectFlights(_prevalidateDirectFlights);

  if (_performNSPreValidation)
  {
    if (!canProduceNonStops(soloTrxData.getShoppingTrx()))
    {
      _farePathFactory = nullptr;
      return;
    }
  }

  // search for a farePath. The FarePathFactory::getNextFPPQItem() seems to
  // return either NULL or the FarePathFactory::lowerBoundFPPPQItem() (i.e.
  // the item we have set _score for). Additionally the FarePathFactory::getNextFPPQItem()
  // sets the FarePathFactory::lowerBoundFPPPQItem() and FarePathFactory::lowerBoundFPAmount()
  // to the next (potentially invalid) fare path.
  FPPQItem* const fppqItem = _farePathFactory->getNextFPPQItem(soloTrxData.getDiagCollector());

  if (fppqItem)
  {
    _farePath = fppqItem->farePath();
  }
  else
  {
    pq.diagCollector().informFarePathInvalid(this, _farePathFactory->getValidationMsg());
    pq.incrementFailedFPs();
  }

  if (_farePathFactory->lowerBoundFPAmount() >= 0)
  {
    // enqueue new item to get further fare paths.
    SoloPQItemManager& pqItemMgr = soloTrxData.getPQItemManager();
    FarePathFactoryPQItemPtr fppqItem(
        pqItemMgr.constructFPFPQItem(*this, soloTrxData.getDiagCollector()));

    pq.enqueue(fppqItem, this);
  }

  // No more fare paths should be build from this _farePathFactory by *this* object.
  // The _farePathFactory has already been passed to the newly created object above
  // or no new fare paths can be build any way (lowerBoundFPAmount() is less then 0).
  //
  // Thus, the next call of expand() for this object will have no effect.
  _farePathFactory = nullptr;
}

namespace
{
inline const FarePath*
getLBItemFarePath(FarePathFactory* const farePathFactory)
{
  return (farePathFactory && farePathFactory->lowerBoundFPAmount() >= 0)
             ? farePathFactory->lowerBoundFPPQItem()->farePath()
             : nullptr;
}

inline bool
isNull(std::ostream& stream, const void* const var, const char* const varId)
{
  if (!var)
  {
    stream << " (no" << varId << ") ";
    return true;
  }
  return false;
}

void
displayFarePath(std::ostream& stream, const FarePath* const farePath)
{
  if (isNull(stream, farePath, "FP"))
    return;

  std::string separator = " FP: ";

  for (const PricingUnit* const pricingUnit : farePath->pricingUnit())
  {

    if (isNull(stream, pricingUnit, "PU"))
      continue;

    for (const FareUsage* const fareUsage : pricingUnit->fareUsage())
    {
      if (isNull(stream, fareUsage, "FU"))
        continue;

      const PaxTypeFare* const paxTypeFare(fareUsage->paxTypeFare());
      if (isNull(stream, paxTypeFare, "PTF"))
        continue;

      const FareMarket* const fareMarket(paxTypeFare->fareMarket());
      if (isNull(stream, fareMarket, "FM"))
        continue;

      stream << separator << fareMarket->boardMultiCity() << "-" << fareMarket->governingCarrier()
             << "-" << paxTypeFare->fareClass() << "-" << fareMarket->offMultiCity();
      separator = " ";
    }
    separator = " : ";
  }
}

void
displayDelta(std::ostream& stream, const FarePathFactory* const fpFactory)
{
  if (isNull(stream, fpFactory, "FPF"))
    return;
  if (0 <= fpFactory->externalLowerBoundAmount())
    stream << " [FPF delta: " << std::setw(7) << std::setprecision(2)
           << fpFactory->externalLowerBoundAmount() << "] ";
}
}

// virtual
std::string
FarePathFactoryPQItem::str(const StrVerbosityLevel strVerbosity /* = SVL_BARE*/) const
{
  std::ostringstream stream;
  stream.setf(std::ios::fixed, std::ios::floatfield);
  stream << "FPF " << _solutionPattern.getSPIdStr() << " ";

  if (isFailed())
    stream << " UNKNW ";
  else
    stream << std::setw(7) << std::setprecision(2) << getScore();

  if (strVerbosity >= SVL_BARE)
  {
    displayDelta(stream, _farePathFactory);
    displayFarePath(stream, _farePath ? _farePath : getLBItemFarePath(_farePathFactory));
  }

  return stream.str();
}

// virtual
void
FarePathFactoryPQItem::visitFareMarkets(FareMarketVisitor& visitor) const
{
  if (!_farePath)
    return;

  for (PricingUnit* pricingUnit : _farePath->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();
      visitor.visit(fareMarket);
    }
  }
}

} /* namespace shpq */
} /* namespace tse */
