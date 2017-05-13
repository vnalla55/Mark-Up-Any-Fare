#include "Pricing/PaxTypeFareBitmapValidator.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/FareValidatorOrchestrator.h"
#include "Fares/JourneyItinWrapper.h"
#include "Pricing/FareUsageMatrixMap.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/ShoppingPQ.h"
#include "Rules/RuleConst.h"


namespace tse
{

namespace
{
Logger
logger("atseintl.PaxTypeFareBitmapValidator");

ConfigurableValue<uint32_t>
altDateMaxPassedBitCfg("SHOPPING_OPT", "ALT_DATE_MAX_PASSED_BIT", 1000);
ConfigurableValue<uint32_t>
maxFlightsRuleValidation("SHOPPING_OPT", "MAX_FLIGHTS_RULE_VALIDATION", 50);

class SoloSkippedBitValidator : public tse::PaxTypeFareBitmapValidator::SkippedBitValidator
{
public:
  using CxrKeysVec = std::vector<uint32_t>;
  SoloSkippedBitValidator() = default;

  SoloSkippedBitValidator(const SoloSkippedBitValidator&) = delete;
  void operator=(const SoloSkippedBitValidator&) = delete;

  SoloSkippedBitValidator*
  init(ShoppingTrx* trx, BitmapOpOrderer* bOrder, const CxrKeysVec& cxrKeysVec)
  {
    _trx = trx;
    _bOrder = bOrder;
    _cxrKeysVec = cxrKeysVec;
    _altDateMaxPassedBit = altDateMaxPassedBitCfg.getValue();
    _maxFlightsForRuleValidation = maxFlightsRuleValidation.getValue();
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

  virtual bool foundHighestFarePath() const override { return false; }
  virtual uint32_t maxFlightsForRuleValidation() const override { return _maxFlightsForRuleValidation; }
  virtual uint32_t altDateMaxPassedBit() const override { return _altDateMaxPassedBit; }

private:
  ShoppingTrx* _trx = nullptr;
  BitmapOpOrderer* _bOrder = nullptr;
  uint32_t _maxFlightsForRuleValidation = 50;
  uint32_t _altDateMaxPassedBit = 1000;
  CxrKeysVec _cxrKeysVec;
};

void
SoloSkippedBitValidator::
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

  FareMarket* fareMarket = ptFare->fareMarket();
  uint16_t legIndex = fareMarket->legIndex();
  TSE_ASSERT(legIndex < _cxrKeysVec.size());

  Itin* journeyItinAltDate(nullptr);
  if (_trx->isAltDates() && dates)
  {
    // Maybe better to pass ShoppingTrx::AltDatePair or altDatesJourneyItin to
    // SoloSkippedBitValidatorConstructor
    ShoppingTrx::AltDatePairs::const_iterator it = _trx->altDatePairs().find(*dates);
    if (LIKELY(it != _trx->altDatePairs().end()))
      journeyItinAltDate = it->second->journeyItin;
  }
  JourneyItinWrapper journeyItinWrapper(
      *_trx, fareMarket, leg, legIndex, _cxrKeysVec[legIndex], journeyItinAltDate);

  const bool acrossStopOverLeg = endLeg - beginLeg > 1;
  CabinType cab;
  cab.setEconomyClass();

  ShoppingRtgMap rtMap;
  ShoppingRtgMap* rtgMap = &rtMap;
  std::vector<BCETuning>* bceTunningData = &bceTuning;
  ShpBitValidationCollector::FMValidationSharedData* sharedData(nullptr);

  sharedData = _trx->getBitValidationCollector().getFMSharedData(journeyItinWrapper.getCarrierKey(),
                                                                 fareMarket);
  TSE_ASSERT(sharedData);
  sharedData->updateFareMarketData(fareMarket, bit);
  rtgMap = sharedData->getRoutingMapForBit(bit);
  bceTunningData = sharedData->getBCEData(bit);

  journeyItinWrapper.applySegments(itinIt->second, itinIt.bitIndex());

  Itin& journeyItin = journeyItinWrapper.getJourneyItin();
  _bOrder->performBitmapOperations(*_trx,
                                   *ptFare,
                                   *fareMarket,
                                   bit,
                                   journeyItin,
                                   cellInfo,
                                   leg.preferredCabinClass() < cab,
                                   acrossStopOverLeg ? _trx->ASOfareMarketRuleController()
                                                     : _trx->fareMarketRuleController(),
                                   _trx->cat4RuleController(),
                                   *rtgMap,
                                   *bceTunningData,
                                   bkcProcessed,
                                   rtgProcessed,
                                   glbProcessed);

  sharedData->collectFareMarketData(fareMarket, bit);

  journeyItinWrapper.eraseSegments();
  if (UNLIKELY(!ptFare->altDateFltBitStatus().empty()))
  {
    if (bkcProcessed)
      return;

    ShoppingAltDateUtil::setAltDateFltBit(ptFare, bit);
  }
}
}

PaxTypeFareBitmapValidator::PaxTypeFareBitmapValidator(ShoppingTrx& trx,
                                                       PaxTypeFare& paxTypeFare,
                                                       const DatePair* dates,
                                                       SkippedBitValidator* skippedBitValidator,
                                                       FareUsageMatrixMap* fareUsageMatrixMap)
  : _trx(trx),
    _ptFare(paxTypeFare),
    _legIndex(_ptFare.fareMarket()->legIndex()),
    _leg(trx.legs()[_legIndex]),
    _acrossStopOver(_leg.stopOverLegFlag()),
    _startIndex(_acrossStopOver ? _leg.jumpedLegIndices().front() : _legIndex),
    _endIndex(1 + (_acrossStopOver ? _leg.jumpedLegIndices().back() : _legIndex)),
    _fmrmIT(trx.fareMarketRuleMap().find(_ptFare.fareMarket())),
    _shoppingBCETuningData(_fmrmIT == trx.fareMarketRuleMap().end()
                               ? _defaultSBCE
                               : _fmrmIT->second._shoppingBCETuningData),
    _dates(dates),
    _variabledDuration(_trx.durationAltDatePairs().size() > 1),
    _duration(dates ? dates->second.get64BitRepDateOnly() - dates->first.get64BitRepDateOnly() : 0),
    _skippedBitValidator(skippedBitValidator),
    _fareUsageMatrixMap(fareUsageMatrixMap)
{
  if (UNLIKELY(_legIndex < 0))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID SCHEDULE");
  }

  TSE_ASSERT(_ptFare.fareMarket()->travelSeg().empty() == false);
}

PaxTypeFareBitmapValidator::SkippedBitValidator*
PaxTypeFareBitmapValidator::createSoloSkippedBitValidator(
    ShoppingTrx* trx, BitmapOpOrderer* bOrder, const std::vector<ItinIndex::Key>& cxrKeysVec)
{
  TSE_ASSERT(trx->isSumOfLocalsProcessingEnabled());
  return trx->dataHandle().create<SoloSkippedBitValidator>()->init(trx, bOrder, cxrKeysVec);
}

bool
PaxTypeFareBitmapValidator::validate(bool considerNonStopsOnly)
{
  if (LIKELY(!_trx.isIataFareSelectionApplicable()))
    return validate(_ptFare.fareMarket()->governingCarrier(), considerNonStopsOnly);

  const FareMarket& fareMarket = *_ptFare.fareMarket();
  const ApplicableSOP* applicableSOP = fareMarket.getApplicableSOPs();
  if (!applicableSOP) // stopover leg
    return validate(_ptFare.fareMarket()->governingCarrier(), considerNonStopsOnly);

  for (const auto& sopUsage : *applicableSOP)
  {
    if (_fareUsageMatrixMap->carrier())
    {
      // online queue. Only process carrier's SOPusage
      ItinIndex::Key key;
      ShoppingUtil::createKey(*_fareUsageMatrixMap->carrier(), key);
      if (key != sopUsage.first)
        continue;
    }

    _ptFare.fareMarket()->setComponentValidationForCarrier(sopUsage.first);
    _ptFare.setComponentValidationForCarrier(sopUsage.first, _trx.isAltDates(), _duration);

    if (!validate(sopUsage.first, considerNonStopsOnly))
      return false;
  }

  return true;
}

bool
PaxTypeFareBitmapValidator::validate(CarrierCode governingCarrier, bool considerNonStopsOnly)
{
  ItinIndex::Key key;
  ShoppingUtil::createKey(governingCarrier, key);

  return validate(key, considerNonStopsOnly);
}

bool
PaxTypeFareBitmapValidator::validate(ItinIndex::Key key, bool considerNonStopsOnly)
{
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator iter;
  bool swapFlightBitmap = false;

  if (LIKELY(_fareUsageMatrixMap != nullptr))
  {
    _fareUsageMatrixMap->addMapping(_startIndex, _endIndex);
  }

  ItinIndex& index = _leg.carrierIndex();

  ItinIndex::ItinIndexIterator i1 =
      _acrossStopOver ? index.beginAcrossStopOverRow(_trx, _legIndex, key) : index.beginRow(key);
  const ItinIndex::ItinIndexIterator i2 =
      _acrossStopOver ? index.endAcrossStopOverRow() : index.endRow();

  if (_trx.isAltDates() && _variabledDuration && _dates != nullptr &&
      !_trx.isIataFareSelectionApplicable())
  {
    if (_duration != _ptFare.getDurationUsedInFVO() /*trx.mainDuration()*/)
    {
      iter = _ptFare.durationFlightBitmap().find(_duration);
      if (iter == _ptFare.durationFlightBitmap().end())
      {
        LOG4CXX_ERROR(
            logger,
            "PaxTypeFareMatrixMap - Invalid PaxTypeFare passed to PO. Needs to be investigated.");
        return false;
      }

      _ptFare.flightBitmap().swap(iter->second);
      swapFlightBitmap = true;
    }
  }

  uint32_t loopCount = 0;
  for (; i1 != i2; ++i1)
  {
    if (considerNonStopsOnly && !isNonStopItin(i1))
      continue;

    ++loopCount;
    if (!validate(i1, key))
      continue;

    if ((_acrossStopOver) && (loopCount > _skippedBitValidator->maxFlightsForRuleValidation()))
      break;

    if (_trx.isAltDates() && _bitCount >= _skippedBitValidator->altDateMaxPassedBit())
      break;
  }

  if (_trx.isAltDates() && _dates != nullptr)
  {
    if (swapFlightBitmap)
    {
      _ptFare.flightBitmap().swap(iter->second);
      swapFlightBitmap = false;
    }
    if (!_atLeastOneSopValid)
    {
      _ptFare.setAltDateStatus(*_dates, RuleConst::ALL_BITS_FAIL);
    }
  }

  if (considerNonStopsOnly && !_atLeastOneSopValid)
  {
    _ptFare.setInvalidForNonStops(true);
  }

  return true;
}

bool
PaxTypeFareBitmapValidator::validate(const uint32_t bit, const ItinIndex::Key& key)
{
  ItinIndex& index = _leg.carrierIndex();

  const ItinIndex::ItinIndexIterator itinIndexBegin =
      _acrossStopOver ? index.beginAcrossStopOverRow(_trx, _legIndex, key) : index.beginRow(key);
  const ItinIndex::ItinIndexIterator itinIndexEnd =
      _acrossStopOver ? index.endAcrossStopOverRow() : index.endRow();

  for (ItinIndex::ItinIndexIterator itinIndex = itinIndexBegin; itinIndex != itinIndexEnd;
       ++itinIndex)
  {
    if (itinIndex.bitIndex() == bit)
    {
      return validate(itinIndex, key);
    }
  }

  return false;
}

bool
PaxTypeFareBitmapValidator::validate(ItinIndex::ItinIndexIterator& itinIt,
                                     const ItinIndex::Key& carrierKey)
{
  if (_dates != nullptr)
  {
    const int sop = itinIt->first.sopIndex();
    const DateTime& date = ShoppingAltDateUtil::getDateSop(_trx, _legIndex, sop);

    if ((_legIndex == 0 && date != _dates->first) || (_legIndex == 1 && date != _dates->second))
    {
      return false;
    }
  }

  const uint32_t bit = itinIt.bitIndex();

  if (_shoppingBCETuningData.size() <= (unsigned int)bit)
  {
    _shoppingBCETuningData.resize(bit + 1);
  }

  if (LIKELY(!_skippedBitValidator->foundHighestFarePath()))
  {
    // this function call validates the bit if it was previously
    // marked as 'skipped' because we weren't sure if we needed
    // to validate it previously
    (*_skippedBitValidator)(
        itinIt, &_ptFare, _leg, _startIndex, _endIndex,
        bit, _shoppingBCETuningData[bit], _dates, carrierKey);
  }

  if (_skippedBitValidator->foundHighestFarePath() || _ptFare.isFlightValid(bit))
  {
    _atLeastOneSopValid = true;
    ++_bitCount;

    if (_fareUsageMatrixMap != nullptr)
    {
      _fareUsageMatrixMap->addSops(itinIt, _startIndex, _endIndex, _acrossStopOver);
    }
  }

  return true;
}

bool
PaxTypeFareBitmapValidator::isNonStopItin(const ItinIndex::ItinIndexIterator& itinIt) const
{
  if (LIKELY(!_acrossStopOver))
  {
    uint32_t sopId = itinIt->first.sopIndex();
    const ShoppingTrx::Leg& leg = _trx.legs()[_startIndex];
    const ShoppingTrx::SchedulingOption& sop = leg.sop()[sopId];
    return sop.itin()->travelSeg().size() <= 1;
  }
  else
  {
    const IndexVector& legIds = _leg.jumpedLegIndices();
    const SopIdVec& sopIds = itinIt.currentSopSet();
    TSE_ASSERT(legIds.size() == sopIds.size());

    for (std::size_t i = 0, numLegs = legIds.size(); i < numLegs; ++i)
    {
      uint32_t legId = legIds[i];
      if (legId == ASOLEG_SURFACE_SECTOR_ID)
        continue;
      const ShoppingTrx::Leg& leg = _trx.legs()[legId];
      const ShoppingTrx::SchedulingOption& sop = leg.sop()[sopIds[i]];
      if (sop.itin()->travelSeg().size() > 1)
        return false;
    }

    return true;
  }
}

bool
PaxTypeFareBitmapValidator::SkippedBitValidator::isAltDateFltBitFailedBKCRTGGLB(
    PaxTypeFare* ptFare, const int bit, bool& bkcProcessed, bool& rtgProcessed, bool& glbProcessed)
{
  if (ptFare->isAltDateFltBitBKCProcessed(bit))
  {
    if (!ptFare->isAltDateFltBitBKCPassed(bit))
    {
      ptFare->setFlightInvalid(bit, RuleConst::BOOKINGCODE_FAIL);
      return true;
    }
    bkcProcessed = true;
    rtgProcessed = true;
    glbProcessed = true;
  }
  else if (ptFare->isAltDateFltBitRTGProcessed(bit))
  {
    if (!ptFare->isAltDateFltBitRTGPassed(bit))
    {
      ptFare->setFlightInvalid(bit, RuleConst::ROUTING_FAIL);
      return true;
    }
    rtgProcessed = true;
    glbProcessed = true;
  }
  else if (ptFare->isAltDateFltBitGLBProcessed(bit))
  {
    if (!ptFare->isAltDateFltBitGLBPassed(bit))
    {
      ptFare->setFlightInvalid(bit, RuleConst::GLOBALDIR_FAIL);
      return true;
    }
    glbProcessed = true;
  }
  return false;
}

}
