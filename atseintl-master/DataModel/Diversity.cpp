/*
 * File:   Diversity.cpp
 * Author: Viktor Ostashevskyi
 *
 * Created on September 26, 2011, 3:56 AM
 */

#include "DataModel/Diversity.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

#include <ext/functional>
#include <tr1/functional>
#include <tr1/tuple>

namespace tse
{
const CarrierCode Diversity::INTERLINE_CARRIER = "";
namespace
{
Logger
logger("atseintl.DataModel.Diversity");
ConfigurableValue<uint64_t>
minOptionsCountForMarketSizeCalculation("SHOPPING_DIVERSITY",
                                        "MIN_OPTIONS_COUNT_FOR_MARKET_SIZE_CALCULATION",
                                        50);
ConfigurableValue<uint64_t>
hundredsOptionsRequestAdjust("SHOPPING_DIVERSITY", "HUNDREDS_OPTIONS_REQUEST_ADJUST", 100u);
ConfigurableValue<bool>
isNonStopOptionPerCarrierEnabled("SHOPPING_DIVERSITY",
                                 "NON_STOP_OPTION_PER_CARRIER_ENABLED",
                                 false);
ConfigurableValue<float>
minimumCustomSolutionPercentage("SHOPPING_OPT", "MINIMUM_CUSTOM_SOLUTION_PERCENTAGE", 100.0);
ConfigurableValue<uint16_t>
fareLevelNumber("SHOPPING_DIVERSITY", "ALT_DATE_DIVERSITY_FARE_LEVELS", 3);
ConfigurableValue<uint64_t>
optionsPerCarrier("SHOPPING_DIVERSITY", "ALT_DATE_DIVERSITY_OPTIONS_PER_CARRIER", 3);
ConfigurableValue<float>
fareCutoffCoef("SHOPPING_OPT", "ALT_DATE_DIVERSITY_CUTOFF_COEF", 4.);
ConfigurableValue<float>
fareCutoffCoefDatePair("SHOPPING_DIVERSITY", "ALT_DATE_DIVERSITY_DATEPAIR_CUTOFF_COEF", 1.5);
}

FIXEDFALLBACK_DECL(fallbackShoppingPQCabinClassValid);
FALLBACK_DECL(fallbackExpandCarrierRemoveMissingCarriers);

namespace
{
// Helper class to count unique cnxn flights
class CompareAirSeg
{
public:
  bool operator()(const AirSeg* lh, const AirSeg* rh)
  {
    return std::tr1::make_tuple(std::tr1::cref(lh->flightNumber()),
                                std::tr1::cref(lh->carrier()),
                                std::tr1::cref(lh->origAirport()),
                                std::tr1::cref(lh->destAirport()),
                                std::tr1::cref(lh->departureDT())) <
           std::tr1::make_tuple(std::tr1::cref(rh->flightNumber()),
                                std::tr1::cref(rh->carrier()),
                                std::tr1::cref(rh->origAirport()),
                                std::tr1::cref(rh->destAirport()),
                                std::tr1::cref(rh->departureDT()));
  }
};

} // anon ns

class Diversity::NonStopIndexImpl : public Diversity::NonStopIndex
{
public:
  NonStopIndexImpl() { _legStat.reserve(2); }

  void addNS(LegId legIdx, CarrierCode carrier, int origSopId, const ShoppingTrx& trx) override
  {
    if (isBlank())
    {
      setupLegStat(trx);
    }

    int sopId = ShoppingUtil::findInternalSopId(trx, legIdx, origSopId);

    bool isBkcValid = trx.legs()[legIdx].sop().at(sopId).cabinClassValid();
    if (isBkcValid)
    {
      _legStat.at(legIdx)[carrier].insert(sopId);
    }
  }

  void addNS(const ShoppingTrx& trx) override
  {
    if (isBlank())
      setupLegStat(trx);

    for (LegId legIdx = 0; legIdx < _legStat.size(); ++legIdx)
    {
      const ShoppingTrx::Leg& leg = trx.legs().at(legIdx);

      for (std::size_t sopIdx = 0; sopIdx < leg.sop().size(); ++sopIdx)
      {
        const ShoppingTrx::SchedulingOption& sop = leg.sop().at(sopIdx);
        if (!fallback::fixed::fallbackShoppingPQCabinClassValid())
        {
          if (!sop.cabinClassValid())
            continue;
        }
        if (sop.itin()->travelSeg().size() != 1)
          continue;
        _legStat[legIdx][sop.governingCarrier()].insert(static_cast<int>(sopIdx));
      }
    }
  }

  bool updateBookingCodeInfo(const ShoppingTrx& trx) override
  {
    if (UNLIKELY(isBlank()))
      return false;

    bool result = false;

    for (LegId legIdx = 0; legIdx < _legStat.size(); ++legIdx)
    {
      NSPerCxrMap& nsPerCxr = _legStat.at(legIdx);

      for (NSPerCxrMap::value_type& cxrItem : nsPerCxr)
      {
        SopSet& cxrSops = cxrItem.second;

        for (SopSet::iterator sopEraseIter = cxrSops.begin(); sopEraseIter != cxrSops.end();)
        {
          bool isBkcValid = trx.legs()[legIdx].sop().at(*sopEraseIter).cabinClassValid();
          if (UNLIKELY(!isBkcValid))
          {
            result = true;
            cxrSops.erase(sopEraseIter++); // and yes, we count to get possibility of empty SopSet
          }
          else
            ++sopEraseIter;
        }
      }
    }

    return result;
  }

  void calcStatPutResultsTo(size_t& outMaxOnlineNSCount,
                            size_t& outMaxInterlineNSCount,
                            std::map<CarrierCode, size_t>& outMaxNSPerCarrier) const override
  {
    outMaxOnlineNSCount = 0;
    outMaxInterlineNSCount = 0;
    outMaxNSPerCarrier.clear();

    if (isBlank())
      return;

    // calculate statistics for online options
    for (const NSPerCxrMap::value_type& obLegStat : _legStat.at(0))
    {
      CarrierCode carrier = obLegStat.first;
      size_t nsPerCxr = accumulateCarrierNonstopCount(carrier);
      if (nsPerCxr > 0)
      {
        outMaxOnlineNSCount += nsPerCxr;
        bool insOk = outMaxNSPerCarrier.insert(std::make_pair(carrier, nsPerCxr)).second;
        TSE_ASSERT(insOk);
      }
    }

    // calculate statistic for interline options
    size_t totalNSCount = 1;
    for (const NSPerCxrMap& legStat : _legStat)
    {
      size_t nsCountForLeg = 0;
      for (const NSPerCxrMap::value_type& legCxrStat : legStat)
        nsCountForLeg += legCxrStat.second.size();

      totalNSCount *= nsCountForLeg;
    }

    outMaxInterlineNSCount = totalNSCount - outMaxOnlineNSCount;
  }

private:
  typedef std::set<int> SopSet;
  typedef std::map<CarrierCode, SopSet> NSPerCxrMap;

  /**
   * No nonstop option has been logged via addNS()
   */
  bool isBlank() const { return _legStat.empty(); }

  void setupLegStat(const ShoppingTrx& trx)
  {
    std::size_t numLegs = 0;
    while (numLegs < trx.legs().size() && !trx.legs().at(numLegs).stopOverLegFlag())
      ++numLegs;
    _legStat.resize(numLegs);
  }

  size_t accumulateCarrierNonstopCount(CarrierCode carrier, LegId legIdx = 0) const
  {
    const NSPerCxrMap& cxrMap = _legStat.at(legIdx);
    const NSPerCxrMap::const_iterator findIt = cxrMap.find(carrier);
    if (findIt == cxrMap.end())
      return 0;

    size_t legNSCount = findIt->second.size();
    if (!legNSCount)
      return 0;

    const bool isFinalLeg = (legIdx + 1 == _legStat.size());
    if (isFinalLeg)
      return legNSCount;
    else
      return legNSCount * accumulateCarrierNonstopCount(carrier, legIdx + 1);
  }

  std::vector<NSPerCxrMap> _legStat;
};

void
Diversity::setBucketDistribution(BucketType type, float value)
{
  if (type >= 0 && type < BUCKET_COUNT)
    _bucketDistribution[type] = value;
}

float
Diversity::getBucketDistribution(BucketType type) const
{
  if (LIKELY(type >= 0 && type < BUCKET_COUNT))
    return _bucketDistribution[type];
  return 0.0;
}

void
Diversity::initialize(DiagCollector* dc,
                      ShoppingTrx& trx,
                      const std::map<ItinIndex::Key, CarrierCode>& carrierMap)
{
  _nonstopIndex = createNonStopIndex(trx);

  bool isBrandedFaresPath = trx.getRequest()->isBrandedFaresRequest();

  if (!isBrandedFaresPath || trx.getRequest()->isAllFlightsRepresented())
  {
    _shouldUseInterlineNonStops = false;
    _shouldGenerateAllNonStopFlightOnlySolutions = false;
  }
  else
  {
    _shouldUseInterlineNonStops = true;
    _shouldGenerateAllNonStopFlightOnlySolutions = true;
  }

  _minOptionsCountForMarketSizeCalculation = minOptionsCountForMarketSizeCalculation.getValue();

  if (dc)
  {
    dc->printHeader();
    *dc << "Common diversity parameters setup:\n";
  }
  setCarrierMap(carrierMap);
  collectScheduleMarketData(trx);
  ScheduleCarrierDataStorage scdStorage;
  collectScheduleCarrierData(trx, scdStorage);
  printSopsInfo(dc, trx);

  std::set<CarrierCode> onlineCarriers;
  collectOnlineCarriers(dc, trx, scdStorage, onlineCarriers);
  setupDirectCarriers(dc, onlineCarriers, scdStorage);
  calculateMarketSize(dc, trx, scdStorage, onlineCarriers);
  setupNumberOfOptionsToGenerate(dc, trx);
  setupNumberOfOptionsPerCarrierDefault(dc);
  setupCarriersRequirements(onlineCarriers, trx);
  setupHDNSMarket(dc, trx, scdStorage);
  setupNonStopOptionsPerCarrier(dc, trx);
  setupNumOfCustomSolutions(dc, trx);
  setupCustomSolutionsRatio(dc, trx);
  setupMaxLongConnectionSolutions(dc, trx);
  setupNonStopOptionsCount(dc, trx);
  if (!fallback::fallbackExpandCarrierRemoveMissingCarriers(&trx) && !_diversityCarrierList.empty())
    cleanupCarriersOnDCLMap(dc);
  printDCL(dc);
  calculateMaxRCOnlineOptions(dc, trx, scdStorage);

  if (dc)
    dc->printLine();
}

void
Diversity::initializeV2(ShoppingTrx& trx)
{
  if (_model != DiversityModelType::V2)
    return;

  _nonstopIndex = createNonStopIndex(trx);
  _nonstopIndex->addNS(trx);
  _nonstopIndex->calcStatPutResultsTo(
      _maxOnlineNonStopCount, _maxInterlineNonStopCount, _maxNonStopCountPerCarrier);
}

size_t
Diversity::getMaxNonStopCountFor(CarrierCode cxr) const
{
  if (UNLIKELY(!shouldConsiderCarrierForNonStop(cxr)))
  {
    return 0;
  }
  if (cxr == Diversity::INTERLINE_CARRIER)
  {
    return _maxInterlineNonStopCount;
  }
  const std::map<CarrierCode, size_t>::const_iterator findIt = _maxNonStopCountPerCarrier.find(cxr);
  return (findIt == _maxNonStopCountPerCarrier.end()) ? 0 : findIt->second;
}

bool
Diversity::shouldConsiderCarrierForNonStop(CarrierCode cxr) const
{
  if (cxr != Diversity::INTERLINE_CARRIER)
  {
    return true;
  }
  return _shouldUseInterlineNonStops || isAdditionalNonStopsEnabled();
}

CarrierCode
Diversity::detectCarrier(ItinIndex::Key cxrKey) const
{
  if (cxrKey == ItinIndex::Key(0))
  {
    return Diversity::INTERLINE_CARRIER;
  }

  std::map<ItinIndex::Key, CarrierCode>::const_iterator cxrIt = _carrierMap.find(cxrKey);
  if (cxrIt != _carrierMap.end())
    return cxrIt->second;

  TSE_ASSERT(!"Bad carrier key");
  return "";
}

bool
Diversity::updateNSBookingCodeInfo(const ShoppingTrx& trx)
{
  if (UNLIKELY(_nonstopIndex->updateBookingCodeInfo(trx)))
  {
    _nonstopIndex->calcStatPutResultsTo(
        _maxOnlineNonStopCount, _maxInterlineNonStopCount, _maxNonStopCountPerCarrier);
    return true;
  }
  else
    return false;
}

Diversity::NonStopIndex*
Diversity::createNonStopIndex(const ShoppingTrx& trx)
{
  return &trx.dataHandle().safe_create<NonStopIndexImpl>();
}

void
Diversity::collectScheduleMarketData(const ShoppingTrx& trx)
{
  TSE_ASSERT(_nonstopIndex != nullptr);

  for (LegId legIdx = 0; legIdx < trx.legs().size(); ++legIdx)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIdx];
    if (leg.stopOverLegFlag())
      continue;

    for (const auto& elem : leg.carrierIndex().root())
    {
      const Itin* firstItin = itinanalyzerutils::getFirstItin(elem.second);
      if (!firstItin)
        continue;
      for (const auto fm : firstItin->fareMarket())
      {
        if (UNLIKELY(!fm))
          continue;
        const ApplicableSOP* sops(fm->getApplicableSOPs());
        if (UNLIKELY(!sops))
          continue;
        for (const auto& sop : *sops)
        {
          std::map<ItinIndex::Key, CarrierCode>::const_iterator govCxrIt(
              _carrierMap.find(sop.first));
          if (UNLIKELY(govCxrIt == _carrierMap.end()))
            continue;
          CarrierCode govCxr(govCxrIt->second);
          if (_smdStorage.find(fm) != _smdStorage.end() &&
              _smdStorage[fm].find(govCxr) != _smdStorage[fm].end())
            continue;
          ScheduleMarketData& smd =
              _smdStorage[fm]
                  .insert(std::make_pair(govCxr, ScheduleMarketData(_todRanges.size())))
                  .first->second;
          for (size_t sopIdx = 0; sopIdx < sop.second.size(); ++sopIdx)
          {
            const SOPUsage& sopUsage = sop.second[sopIdx];
            if (!sopUsage.applicable_ || sopUsage.itin_->isDummy())
              continue;
            const Itin* itin(sopUsage.itin_);
            // If SOP has a single flight that cover THRU FM, then increase non-stop counter
            if (itin->travelSeg().size() == 1u + sopUsage.endSegment_ - sopUsage.startSegment_ &&
                itin->travelSeg().size() == 1)
            {
              smd.nonStopSOPs++;
              _nonstopIndex->addNS(legIdx, govCxr, sopUsage.origSopId_, trx);
            }
            // If start segment is not 0, than this FM is not starting one.
            // We want TOD counters only for first FM in leg
            if (sopUsage.startSegment_ == 0)
              smd.sopPerTOD[itin->getTODBucket(_todRanges)]++;
            // Find min/max flight durations
            if (itin->getFlightTimeMinutes() > smd.maxFlightDuration)
              smd.maxFlightDuration = itin->getFlightTimeMinutes();
            if (itin->getFlightTimeMinutes() < smd.minFlightDuration)
              smd.minFlightDuration = itin->getFlightTimeMinutes();
          }
        }
      }
    }
  }

  _nonstopIndex->calcStatPutResultsTo(
      _maxOnlineNonStopCount, _maxInterlineNonStopCount, _maxNonStopCountPerCarrier);
}

void
Diversity::collectScheduleCarrierData(const ShoppingTrx& trx,
                                      ScheduleCarrierDataStorage& scdStorage)
{
  scdStorage.resize(trx.legs().size());

  // Iterate through legs
  for (LegId legIdx = 0; legIdx < trx.legs().size(); legIdx++)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIdx];

    if (leg.stopOverLegFlag())
      continue;

    // Iterate through carriers
    for (const auto& elem : leg.carrierIndex().root())
    {
      const ItinIndex::ItinRow& row = elem.second;

      uint16_t nonStopFlights = 0;
      uint16_t allFlights = 0;
      uint16_t onlineFlights = 0;

      std::set<const AirSeg*, CompareAirSeg> uniqueCnxnGoverningFlights;

      // Iterate through number of connections
      for (const auto& row_colIt : row)
      {
        uint32_t cnxnNum = row_colIt.first;
        const ItinIndex::ItinColumn& column = row_colIt.second;

        // Iterate through itins
        for (const auto& cell : column)
        {
          const Itin* itin = cell.second;

          if (!itin || itin->isDummy())
            continue;

          ++allFlights;

          if (ShoppingUtil::isOnlineFlight(*itin))
            ++onlineFlights;

          if (cnxnNum == 0)
            nonStopFlights++;
          else if (cell.first.getPrimarySector())
            uniqueCnxnGoverningFlights.insert(cell.first.getPrimarySector()->toAirSeg());
        }
      }

      if (LIKELY(nonStopFlights != 0 || uniqueCnxnGoverningFlights.size() != 0 ||
                 onlineFlights != 0))
      {
        const CarrierCode cxr = _carrierMap.find(elem.first)->second;
        ScheduleCarrierData& scd = scdStorage[legIdx][cxr];
        scd.nonStopFlights = nonStopFlights;
        scd.uniqueCnxnGoverningFlights = static_cast<uint16_t>(uniqueCnxnGoverningFlights.size());
        scd.allGoverningFlights = allFlights;
        scd.onlineFlights = onlineFlights;
      }
    }
  }
}

void
Diversity::collectOnlineCarriers(DiagCollector* dc,
                                 const ShoppingTrx& trx,
                                 const ScheduleCarrierDataStorage& scdStorage,
                                 std::set<CarrierCode>& onlineCarriers)
{
  for (LegId legIdx = 0; legIdx < scdStorage.size(); legIdx++)
  {
    if (trx.legs()[legIdx].stopOverLegFlag())
      continue;

    // Get carriers available on this leg
    std::set<CarrierCode> tmpCxrSet;
    std::transform(scdStorage[legIdx].begin(),
                   scdStorage[legIdx].end(),
                   std::inserter(tmpCxrSet, tmpCxrSet.begin()),
                   [](const ScheduleCarrierDataStorage::value_type::value_type& p)
                   { return p.first; });
    if (onlineCarriers.empty())
      onlineCarriers.swap(tmpCxrSet);
    else
    {
      std::set<CarrierCode> tmpCxrSet2;
      tmpCxrSet2.swap(onlineCarriers);
      std::set_intersection(tmpCxrSet.begin(),
                            tmpCxrSet.end(),
                            tmpCxrSet2.begin(),
                            tmpCxrSet2.end(),
                            std::inserter(onlineCarriers, onlineCarriers.end()));
    }
  }

  if (dc)
  {
    *dc << "\tOnline carriers: " << onlineCarriers.size() << " ( ";
    std::copy(
        onlineCarriers.begin(), onlineCarriers.end(), std::ostream_iterator<CarrierCode>(*dc, " "));
    *dc << ")\n";
  }
}

void
Diversity::calculateMaxRCOnlineOptions(DiagCollector* dc,
                                       const ShoppingTrx& trx,
                                       const ScheduleCarrierDataStorage& scdStorage)
{
  const tse::Billing* billing = trx.billing();

  if (!billing)
  {
    _maxRCOnlineOptionsCount = 0;
    return;
  }

  int governingCarrierOnlineOptions = 1;

  const CarrierCode requestingCarrier = billing->partitionID();

  for (const auto& elem : scdStorage)
  {
    if (elem.find(requestingCarrier) != elem.end())
    {
      const ScheduleCarrierData& scd = elem.find(requestingCarrier)->second;
      governingCarrierOnlineOptions *= scd.onlineFlights;
    }
    else
    {
      governingCarrierOnlineOptions = 0;
      break;
    }
  }

  _maxRCOnlineOptionsCount = governingCarrierOnlineOptions;

  if (dc)
    *dc << "\tMax.Requesting Carrier Online Combinations: " << _maxRCOnlineOptionsCount << "\n";
}

void
Diversity::calculateMarketSize(DiagCollector* dc,
                               const ShoppingTrx& trx,
                               const ScheduleCarrierDataStorage& scdStorage,
                               const std::set<CarrierCode>& onlineCarriers)
{
  size_t optionsCount = 1;
  for (const ShoppingTrx::Leg& leg : trx.legs())
    optionsCount *= ShoppingUtil::countNonDummyItineraries(leg);

  if (optionsCount < _minOptionsCountForMarketSizeCalculation)
    _marketSize = optionsCount;
  else
    calculateMarketSizeFromOnlineCarriers(dc, trx, scdStorage, onlineCarriers);

  if (dc)
    *dc << "\tMarket size: " << _marketSize << "\n";
}

void
Diversity::calculateMarketSizeFromOnlineCarriers(DiagCollector* dc,
                                                 const ShoppingTrx& trx,
                                                 const ScheduleCarrierDataStorage& scdStorage,
                                                 const std::set<CarrierCode>& onlineCarriers)
{
  if (dc)
    *dc << "\tNumber of unique governing flights per carrier:\n";

  for (const CarrierCode& carrier : onlineCarriers)
  {
    if (dc)
      *dc << "\t\t" << carrier << ":";

    size_t uniqueGoverningFligtsProduct = 1;
    for (LegId legIdx = 0; legIdx < scdStorage.size(); ++legIdx)
    {
      if (trx.legs()[legIdx].stopOverLegFlag())
        continue;

      const ScheduleCarrierData& scd = scdStorage[legIdx].find(carrier)->second;
      uniqueGoverningFligtsProduct *= (scd.nonStopFlights + scd.uniqueCnxnGoverningFlights);

      if (dc)
        *dc << " " << std::setw(3) << (scd.nonStopFlights + scd.uniqueCnxnGoverningFlights);
    }

    if (dc)
      *dc << "\n";

    _marketSize += uniqueGoverningFligtsProduct;
  }
}

void
Diversity::setupNumberOfOptionsToGenerate(DiagCollector* dc, const ShoppingTrx& trx)
{
  uint32_t result = trx.getOptions()->getRequestedNumberOfSolutions();

  if (trx.isAltDates())
  {
    uint32_t numberOfOptionsToFind(result);
    result = 0;
    ShoppingTrx::AltDatePairs::const_iterator it = trx.altDatePairs().begin();
    for (; it != trx.altDatePairs().end(); ++it)
    {
      it->second->numOfSolutionNeeded = numberOfOptionsToFind;
      result += numberOfOptionsToFind;
    }
  }
  else
  {
    uint32_t adjustPercentage = 100u;

    adjustPercentage = hundredsOptionsRequestAdjust.getValue();

    (result *= adjustPercentage) /= 100;
  }
  _numberOfOptionsToGenerate = result;

  if (dc)
    *dc << "\tNumber of options to generate: " << _numberOfOptionsToGenerate << "\n";
}

void
Diversity::setupNumberOfOptionsPerCarrierDefault(DiagCollector* dc)
{
  if (_optionsPerCarrierDefault == 0.0f)
    return;

  if (_optionsPerCarrierDefault < 0.0f)
    _optionsPerCarrierDefault =
        -_optionsPerCarrierDefault * static_cast<float>(_numberOfOptionsToGenerate);
  else
    _optionsPerCarrierDefault =
        std::min(_optionsPerCarrierDefault, static_cast<float>(_numberOfOptionsToGenerate));

  if (dc)
    *dc << "\tOptions per carrier by default: " << _optionsPerCarrierDefault << "\n";
}

void
Diversity::cleanupCarriersOnDCLMap(DiagCollector* dc)
{
  // Verify all elements in DCL map exist in OPC map
  std::vector<CarrierCode> dclNotOnOpt;
  for (auto& dclElem : _diversityCarrierList)
  {
    if (_optionsPerCarrier.find(dclElem.first) == _optionsPerCarrier.end())
      dclNotOnOpt.push_back(dclElem.first);
  }

  for (auto& elem : dclNotOnOpt)
  {
    _diversityCarrierList.erase(elem);
    if (dc)
      *dc << "Carrier: " << elem << " removed of diversity list"
          << "\n";
  }
}

void
Diversity::setupCarriersRequirements(const std::set<CarrierCode>& onlineCarriers,
                                     const ShoppingTrx& trx)
{
  std::set<CarrierCode>::const_iterator carrierIt(onlineCarriers.begin());
  std::map<CarrierCode, size_t>::const_iterator dclitEnd = _diversityCarrierList.end();
  for (; carrierIt != onlineCarriers.end(); ++carrierIt)
  {
    std::map<CarrierCode, float>::iterator cxrMapIt = _optionsPerCarrier.find(*carrierIt);
    std::map<CarrierCode, size_t>::const_iterator dclit = _diversityCarrierList.find(*carrierIt);
    if (cxrMapIt == _optionsPerCarrier.end())
    {
      if (dclit != dclitEnd)
        _optionsPerCarrier.insert(std::make_pair(*carrierIt, (uint16_t)(dclit->second)));
      else
         _optionsPerCarrier.insert(std::make_pair(*carrierIt, _optionsPerCarrierDefault));
    }
    else
    {
      if (dclit != dclitEnd)
        _optionsPerCarrier[*carrierIt] = (uint16_t)(dclit->second);
      else
      {
        if (cxrMapIt->second < 0.0)
          cxrMapIt->second = -cxrMapIt->second * static_cast<float>(_numberOfOptionsToGenerate);
        else
          cxrMapIt->second =
              std::min(cxrMapIt->second, static_cast<float>(_numberOfOptionsToGenerate));
      }
    }
  }
}

void
Diversity::setupHDNSMarket(DiagCollector* dc,
                           const ShoppingTrx& trx,
                           const ScheduleCarrierDataStorage& scdStorage)
{
  if (!boost::indeterminate(_highDensityMarket))
    return;

  if (isAdditionalNonStopsEnabled()) // More non-stop logic is enabled, don't use hdnsm
    return;

  uint32_t nonStopCarriers = 0;
  uint32_t nonStopFlights = 0;
  uint32_t cnxnGovCarriers = 0;
  uint32_t cnxnGovFlights = 0;

  for (LegId legIdx = 0; legIdx < scdStorage.size(); legIdx++)
  {
    if (trx.legs()[legIdx].stopOverLegFlag())
      continue;

    std::map<CarrierCode, ScheduleCarrierData>::const_iterator scdIt = scdStorage[legIdx].begin();
    for (; scdIt != scdStorage[legIdx].end(); ++scdIt)
    {
      if (scdIt->second.nonStopFlights != 0)
      {
        nonStopCarriers++;
        nonStopFlights += scdIt->second.nonStopFlights;
      }
      if (scdIt->second.uniqueCnxnGoverningFlights != 0)
      {
        cnxnGovCarriers++;
        cnxnGovFlights += scdIt->second.uniqueCnxnGoverningFlights;
      }
    }
  }

  double nonStopDensity =
      (nonStopCarriers == 0 ? 0.0 : static_cast<double>(nonStopFlights) / nonStopCarriers);

  double cnxnDensity =
      (cnxnGovCarriers == 0 ? 0.0 : static_cast<double>(cnxnGovFlights) / cnxnGovCarriers);

  _highDensityMarket = (nonStopDensity > cnxnDensity);

  if (dc)
  {
    *dc << "High density non-stop market detection:\n";
    *dc << "\tNumber of non-stop flights: " << nonStopFlights << "\n";
    *dc << "\tNumber of non-stop carriers: " << nonStopCarriers << "\n";
    *dc << "\tNumber of unique connection only governing flights: " << cnxnGovFlights << "\n";
    *dc << "\tNumber of unique connection only governing carriers: " << cnxnGovCarriers << "\n";
    *dc << "\tHigh density non-stop market: " << ((nonStopDensity > cnxnDensity) ? "T" : "F")
        << "\n";
  }
}

void
Diversity::setupDirectCarriers(DiagCollector* dc,
                               const std::set<CarrierCode>& onlineCarriers,
                               const ScheduleCarrierDataStorage& scdStorage)
{
  const size_t noOfLegs(scdStorage.size());
  std::set<CarrierCode>::const_iterator carrierIt = onlineCarriers.begin();
  for (; carrierIt != onlineCarriers.end(); ++carrierIt)
  {
    const CarrierCode& carrier = (*carrierIt);
    const ScheduleCarrierData& scdOutbound = scdStorage[0].find(carrier)->second;
    if (noOfLegs == 1)
    {
      if (scdOutbound.nonStopFlights != 0)
        _directOptionsCarriers.insert(carrier);
    }
    else
    {
      const ScheduleCarrierData& scdInbound = scdStorage[1].find(carrier)->second;
      if (scdOutbound.nonStopFlights != 0 && scdInbound.nonStopFlights != 0)
        _directOptionsCarriers.insert(carrier);
    }
  }

  if (dc)
  {
    const bool isDirectCarriersCapable = !_directOptionsCarriers.empty();

    *dc << "\tDirect online carriers: " << _directOptionsCarriers.size();
    if (isDirectCarriersCapable)
    {
      *dc << " ( ";
      std::copy(_directOptionsCarriers.begin(),
                _directOptionsCarriers.end(),
                std::ostream_iterator<CarrierCode>(*dc, " "));
      *dc << ")";
    }
    *dc << "\n";

    *dc << "\tMax number of online non-stop options: " << getMaxOnlineNonStopCount() << "\n";

    *dc << "\tMax number of online non-stop options per cxr: ";
    typedef std::map<CarrierCode, size_t>::value_type CxrNSCount;
    for (const CxrNSCount& cxrNS : _maxNonStopCountPerCarrier)
    {
      *dc << cxrNS.first << "[" << cxrNS.second << "] ";
    }
    *dc << "\n";

    *dc << "\tMax number of interline non-stop options: " << getMaxInterlineNonStopCount() << "\n";
  }
}

void
Diversity::setupNonStopOptionsPerCarrier(DiagCollector* dc, const ShoppingTrx& trx)
{
  _isNonStopOptionPerCarrierEnabled = isNonStopOptionPerCarrierEnabled.getValue();
  if (dc)
  {
    *dc << "\tNon-stop option per carrier is enabled: "
        << (_isNonStopOptionPerCarrierEnabled ? "Yes" : "No") << "\n";
  }
}

void
Diversity::setupNumOfCustomSolutions(DiagCollector* dc, const ShoppingTrx& trx)
{
  if (trx.getNumOfCustomSolutions())
  {
    _numOfCustomSolutions = trx.getNumOfCustomSolutions();
    if (dc)
    {
      *dc << "Custom Solution generation:\n";
      *dc << "\tNumber of custom solutions: " << _numOfCustomSolutions << "\n";
    }
  }
}

void
Diversity::setupCustomSolutionsRatio(DiagCollector* dc, const ShoppingTrx& trx)
{
  if (trx.getNumOfCustomSolutions())
  {
    float customSolutionPrecentage = 100.0;
    customSolutionPrecentage = minimumCustomSolutionPercentage.getValue();
    _minimalNumCustomSolutions = static_cast<int16_t>(
        ceil(trx.getNumOfCustomSolutions() * customSolutionPrecentage / HUNDRED));
    if (dc)
    {
      *dc << "\tPercentage of custom solutions: " << customSolutionPrecentage << "\n";
      *dc << "\tMinimal number of custom solutions: " << _minimalNumCustomSolutions << "\n";
    }
  }
}

void
Diversity::setupMaxLongConnectionSolutions(DiagCollector* dc, const ShoppingTrx& trx)
{
  if (trx.maxNumOfLngCnxSolutions())
  {
    _maxLongConnectionSolutions = trx.maxNumOfLngCnxSolutions();
    if (dc)
    {
      *dc << "Long connection solutions:\n";
      *dc << "\tMax number of long connection solutions: " << _maxLongConnectionSolutions << "\n";
    }
  }
}

void
Diversity::setupNonStopOptionsCount(DiagCollector* dc, const ShoppingTrx& trx)
{
  // If number of non-stops has been already set
  // it shouldn't be derived from percentage of non-stops.
  if (_nonStopOptionsCount == 0)
  {
    float nsoCount = _nonStopOptionsPercentage * trx.getOptions()->getRequestedNumberOfSolutions();
    TSE_ASSERT(nsoCount >= 0.0f);
    _nonStopOptionsCount = static_cast<size_t>(ceilf(nsoCount));
  }

  if (dc)
  {
    *dc << "\tNumber of additional non-stop options to generate: " << _nonStopOptionsCount << "\n";
  }
}

void
Diversity::printScheduleInfo(DiagCollector* dc, const FareMarket* fm, const CarrierCode& govCxr)
    const
{
  ScheduleMarketDataStorage::const_iterator smdStorageIt = _smdStorage.find(fm);
  if (smdStorageIt == _smdStorage.end())
    return;
  ScheduleMarketDataStorage::mapped_type::const_iterator smdIt = smdStorageIt->second.find(govCxr);
  if (smdIt == smdStorageIt->second.end())
    return;
  const ScheduleMarketData& smd = smdIt->second;
  *dc << "Schedules info:" << '\n';
  *dc << "                Non-stop options: " << smd.nonStopSOPs << '\n';
  *dc << "                Minimal flight duration: " << smd.minFlightDuration << '\n';
  *dc << "                Maximal flight duration: " << smd.maxFlightDuration << '\n';
  *dc << "                SOPs per TOD:";
  for (size_t i = 0; i < smd.sopPerTOD.size(); ++i)
  {
    *dc << " " << i << ":" << smd.sopPerTOD[i];
  }
  *dc << '\n';
}

void
Diversity::printSopsInfo(DiagCollector* dc, const ShoppingTrx& trx) const
{
  if (!dc || !trx.getRequest()->isBrandedFaresRequest())
    return;

  *dc << "\tNumber of outbound sops: " << trx.legs()[0].requestSops() << "\n";
  if (trx.legs().size() > 1)
    *dc << "\tNumber of inbound sops: " << trx.legs()[1].requestSops() << "\n";
}

void
Diversity::printDCL(DiagCollector* dc) const
{
  if (dc)
  {
    if (_diversityCarrierList.empty())
    {
      return;
    }
    std::map<CarrierCode, size_t>::const_iterator dit = _diversityCarrierList.begin();
    std::map<CarrierCode, size_t>::const_iterator ditEnd = _diversityCarrierList.end();
    *dc << "Carriers: ";
    for (; dit != ditEnd; ++dit)
    {
      size_t required = (size_t)(dit->second);
      *dc << " " << dit->first << "[" << required << "]";
    }
    *dc << "\n";
  }
}

Diversity::AltDatesParam::AltDatesParam():
  _fareLevelNumber(fareLevelNumber.getValue()),
  _optionsPerCarrier(optionsPerCarrier.getValue()),
  _fareCutoffCoef(fareCutoffCoef.getValue()),
  _fareCutoffCoefDatePair(fareCutoffCoefDatePair.getValue())
{}

} // ns tse
