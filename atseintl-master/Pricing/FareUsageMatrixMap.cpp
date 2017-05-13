#include "Pricing/FareUsageMatrixMap.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/JourneyItinWrapper.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/ShoppingPQ.h"
#include "Rules/RuleConst.h"

namespace tse
{

namespace
{
log4cxx::LoggerPtr
logger(log4cxx::Logger::getLogger("atseintl.PricingOrchestrator"));

class ShoppingPQSkippedBitValidator : public tse::PaxTypeFareBitmapValidator::SkippedBitValidator
{
public:
  ShoppingPQSkippedBitValidator() = default;
  ShoppingPQSkippedBitValidator(const ShoppingPQSkippedBitValidator&) = delete;
  void operator=(const ShoppingPQSkippedBitValidator&) = delete;

  ShoppingPQSkippedBitValidator*
  init(ShoppingTrx* trx, Itin* journeyItin, BitmapOpOrderer* bOrder, ShoppingPQ* shoppingPQ)
  {
    _trx = trx;
    _journeyItin = journeyItin;
    _bOrder = bOrder;
    _shoppingPQ = shoppingPQ;

    TSE_ASSERT(!_trx->isSumOfLocalsProcessingEnabled());

    return this;
  }

  virtual void operator()(ItinIndex::ItinIndexIterator& itinIt,
                          PaxTypeFare* ptFare,
                          ShoppingTrx::Leg& leg,
                          uint32_t beginLeg,
                          uint32_t endLeg,
                          uint32_t bit,
                          std::vector<BCETuning>& bceTuning,
                          const DatePair* dates,
                          const ItinIndex::Key carrierKey) override;

  virtual bool foundHighestFarePath() const override { return _shoppingPQ->foundHighestFarePath(); }
  virtual uint32_t maxFlightsForRuleValidation() const override
  {
    return _shoppingPQ->maxFlightsForRuleValidation();
  }
  virtual uint32_t altDateMaxPassedBit() const override { return _shoppingPQ->altDateMaxPassedBit(); }

private:
  ShoppingTrx* _trx = nullptr;
  Itin* _journeyItin = nullptr;
  BitmapOpOrderer* _bOrder = nullptr;
  ShoppingPQ* _shoppingPQ = nullptr;
};

void
ShoppingPQSkippedBitValidator::
operator()(ItinIndex::ItinIndexIterator& itinIt,
           PaxTypeFare* ptFare,
           ShoppingTrx::Leg& leg,
           uint32_t beginLeg,
           uint32_t endLeg,
           uint32_t bit,
           std::vector<BCETuning>& bceTuning,
           const DatePair* dates,
           const ItinIndex::Key carrierKey)
{
  ItinIndex::ItinCell& itinCell = *itinIt;
  uint8_t* bitValue = ptFare->getFlightBit(bit);

  if (bitValue == nullptr || *bitValue != RuleConst::SKIP)
  {
    return;
  }

  const TSELatencyData metrics(*_trx, "SECOND PASS RULE VALIDATION");

  *bitValue = 0;

  ItinIndex::ItinCellInfo& cellInfo = itinCell.first;
  bool bkcProcessed = false;
  bool rtgProcessed = false;
  bool glbProcessed = false;
  if (_trx->isAltDates() && dates != nullptr)
  {
    _journeyItin = _shoppingPQ->getJourneyItinAltDates(*dates);
    if (!_journeyItin)
    {
      ptFare->setFlightInvalid(bit, RuleConst::NO_DATEPAIR_FOUND);
      return;
    }
    if (!ptFare->altDateFltBitStatus().empty() &&
        isAltDateFltBitFailedBKCRTGGLB(ptFare, bit, bkcProcessed, rtgProcessed, glbProcessed))
    {
      return;
    }
  }

  // backup travel segs in journey
  std::vector<TravelSeg*> journeySegs = _journeyItin->travelSeg();

  const bool acrossStopOverLeg = endLeg - beginLeg > 1;

  // call FVO code to validate fares
  FareValidatorOrchestrator::prepareJourneyItin(_journeyItin, nullptr, beginLeg, endLeg);

  CabinType cab;
  cab.setEconomyClass();

  ShoppingRtgMap rtMap;
  ShoppingRtgMap* rtgMap = &rtMap;
  std::vector<BCETuning>* bceTunningData = &bceTuning;
  ShpBitValidationCollector::FMValidationSharedData* sharedData(nullptr);
  FareMarket* fareMarket = ptFare->fareMarket();

  sharedData = _trx->getBitValidationCollector().getFMSharedData(carrierKey, fareMarket);
  TSE_ASSERT(sharedData);
  sharedData->updateFareMarketData(fareMarket, bit);
  rtgMap = sharedData->getRoutingMapForBit(bit);
  bceTunningData = sharedData->getBCEData(bit);

  FareValidatorOrchestrator::FMBackup fmb;
  FareValidatorOrchestrator::prepareShoppingValidation(
      *_trx, _journeyItin, itinCell, fareMarket, fmb, beginLeg, endLeg);

  _bOrder->performBitmapOperations(*_trx,
                                   *ptFare,
                                   *fareMarket,
                                   bit,
                                   *_journeyItin,
                                   cellInfo,
                                   leg.preferredCabinClass() < cab,
                                   acrossStopOverLeg ? _shoppingPQ->asoRuleController()
                                                     : _shoppingPQ->fareMarketRuleController(),
                                   _shoppingPQ->cat4RuleController(),
                                   *rtgMap,
                                   *bceTunningData,
                                   bkcProcessed,
                                   rtgProcessed,
                                   glbProcessed);

  sharedData->collectFareMarketData(fareMarket, bit);

  FareValidatorOrchestrator::cleanupAfterShoppingValidation(
      *_trx, _journeyItin, fareMarket, fmb, beginLeg, endLeg);

  // restore travel segs in journey
  _journeyItin->travelSeg().swap(journeySegs);
  if (!ptFare->altDateFltBitStatus().empty())
  {
    if (bkcProcessed)
      return;

    ShoppingAltDateUtil::setAltDateFltBit(ptFare, bit);
  }
}

} // namespace

FareUsageMatrixMap::FareUsageMatrixMap(ShoppingTrx* trx,
                                       Itin* journeyItin,
                                       FarePath* path,
                                       BitmapOpOrderer* bOrder,
                                       const DatePair* dates,
                                       ShoppingPQ* shoppingPQ)
  : _trx(trx),
    _dates(dates),
    _skippedBitValidator(_trx->dataHandle().create<ShoppingPQSkippedBitValidator>()->init(
        _trx, journeyItin, bOrder, shoppingPQ)),
    _considerNonStopsOnly(shoppingPQ->isNonStopShoppingQueue()),
    _carrier(shoppingPQ->carrier())
{
  validate(*path);
}

void
FareUsageMatrixMap::validate(FarePath& path)
{
  if (UNLIKELY(_trx->isSumOfLocalsProcessingEnabled() && _trx->isAltDates()))
  {
    LOG4CXX_ERROR(logger, "FareUsageMatrixMap - altDates for solo not implemented.");
    return;
  }

  if (_trx->isAltDates() && _dates != nullptr)
  {
    // make sure all fare are good for this date pair
    for (const auto pu : path.pricingUnit())
    {
      for (const auto fu : pu->fareUsage())
      {
        PaxTypeFare& ptFare = *fu->paxTypeFare();
        if (!ptFare.getAltDatePass(*_dates))
        {
          return;
        }
      }
    }
  }
  for (const auto pu : path.pricingUnit())
  {
    for (const auto fu : pu->fareUsage())
    {
      if (UNLIKELY(!validate(*fu)))
      {
        return;
      }
    }
  }
}

bool
FareUsageMatrixMap::validate(FareUsage& fareUsage)
{
  PaxTypeFareBitmapValidator fubv(
      *_trx, *fareUsage.paxTypeFare(), _dates, _skippedBitValidator, this);
  return fubv.validate(_considerNonStopsOnly);
}

const std::vector<int>&
FareUsageMatrixMap::getDimensions() const
{
  if (_dim.empty())
  {
    for (const auto& elem : _map)
    {
      _dim.push_back(elem.sops.size());
    }
  }

  return _dim;
}

void
FareUsageMatrixMap::getSops(const std::vector<int>& pos, std::vector<int>& sops) const
{
  sops.clear();
  size_t index = 0;
  for (std::vector<FareUsageMatrixMapping>::const_iterator i = _map.begin(); i != _map.end();
       ++i, ++index)
  {
    TSE_ASSERT(index < pos.size());
    TSE_ASSERT(size_t(pos[index]) < i->sops.size());
    const SopList& sopList = i->sops[pos[index]];
    if (sops.size() < size_t(i->firstLeg + i->nLegs))
    {
      sops.resize(i->firstLeg + i->nLegs);
    }

    TSE_ASSERT(int(sopList.size()) == i->nLegs);

    std::copy(sopList.begin(), sopList.end(), sops.begin() + i->firstLeg);
  }
}

bool
FareUsageMatrixMap::hasSops(const std::vector<int>& sops, std::vector<int>* pos) const
{
  if (pos != nullptr)
  {
    pos->clear();
  }

  for (std::vector<FareUsageMatrixMapping>::const_iterator i = _map.begin(); i != _map.end(); ++i)
  {
    TSE_ASSERT(size_t(i->firstLeg + i->nLegs) <= sops.size());
    bool found = false;
    for (std::vector<SopList>::const_iterator j = i->sops.begin(); j != i->sops.end(); ++j)
    {
      if (std::equal(sops.begin() + i->firstLeg, sops.begin() + i->firstLeg + i->nLegs, j->begin()))
      {
        found = true;
        if (pos != nullptr)
        {
          pos->push_back(j - i->sops.begin());
        }

        break;
      }
    }

    if (found == false)
    {
      return false;
    }
  }

  return true;
}

void
FareUsageMatrixMap::addMapping(int startIndex, int endIndex)
{
  _map.push_back(FareUsageMatrixMapping(startIndex, endIndex - startIndex));
}

void
FareUsageMatrixMap::addSops(ItinIndex::ItinIndexIterator& itinIt,
                            int startIndex,
                            int endIndex,
                            bool acrossStopOver)
{
  SopList sops;
  if (!acrossStopOver)
  {
    sops.push_back(itinIt->first.sopIndex());
    TSE_ASSERT(sops.front() < int(_trx->legs()[startIndex].sop().size()));
  }
  else
  {
    const SopIdVec& bitSopSet = itinIt.currentSopSet();
    sops.assign(bitSopSet.begin(), bitSopSet.end());
    // remove any arunk segments
    sops.erase(std::remove(sops.begin(), sops.end(), static_cast<int>(ASOLEG_SURFACE_SECTOR_ID)),
               sops.end());
  }

  TSE_ASSERT(int(sops.size()) == _map.back().nLegs);

  _map.back().sops.push_back(std::move(sops));
}

const DatePair*
FareUsageMatrixMap::getDates() const
{
  return _dates;
}
}
