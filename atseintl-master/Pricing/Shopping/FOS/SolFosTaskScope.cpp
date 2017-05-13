// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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
#include "Pricing/Shopping/FOS/SolFosTaskScope.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/TravelSegAnalysis.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <algorithm>

#include <stdint.h>

namespace tse
{
namespace
{
ConfigurableValue<uint32_t>
flightOnlySolutionsActivationThreshold("SHOPPING_DIVERSITY", "FLIGHT_ONLY_SOLUTIONS_THRESHOLD");
ConfigurableValue<uint32_t>
numFlightOnlySolutionsCfg("SHOPPING_DIVERSITY", "NUM_FLIGHT_ONLY_SOLUTIONS", 10);
ConfigurableValue<uint32_t>
numFlightOnlySolutionsOWCfg("SHOPPING_DIVERSITY", "NUM_FLIGHT_ONLY_SOLUTIONS_OW", 20);
ConfigurableValue<bool>
pqConditionCfg("SHOPPING_DIVERSITY", "PQCONDITION_OVERRIDE_FLIGHT_ONLY_SOLUTIONS", false);
ConfigurableValue<bool>
checkConnectingCitiesCfg("SHOPPING_DIVERSITY",
                         "CHECK_CONNECTING_CITIES_IN_FLIGHT_ONLY_SOLUTIONS",
                         false);
ConfigurableValue<bool>
checkConnectingFlightsCfg("SHOPPING_DIVERSITY",
                          "CHECK_CONNECTING_FLIGHTS_IN_FLIGHT_ONLY_SOLUTIONS",
                          false);
ConfigurableValue<uint32_t>
numSnowmanSolutions("SHOPPING_DIVERSITY", "SNOWMAN_FLIGHT_ONLY_SOLUTIONS");
ConfigurableValue<uint32_t>
numDiamondSolutions("SHOPPING_DIVERSITY", "DIAMOND_FLIGHT_ONLY_SOLUTIONS");
ConfigurableValue<uint32_t>
numTriangleSolutions("SHOPPING_DIVERSITY", "TRIANGLE_FLIGHT_ONLY_SOLUTIONS");
ConfigurableValue<uint64_t>
numDirectFOSPerOnlineCarrier("SHOPPING_DIVERSITY",
                             "NUM_DIRECT_FLIGHT_ONLY_SOLUTIONS_PER_CARRIER",
                             1);
ConfigurableValue<float>
additionalPercentage("SHOPPING_DIVERSITY", "ADDITIONAL_DIRECT_FOS_PERC", 1.0);
}
namespace fos
{
static Logger
logger("atseintl.pricing.FOS.SolFosTaskScope");

SolFosTaskScope::SolFosTaskScope(const ShoppingTrx& trx,
                                 const ItinStatistic& stats,
                                 bool pqConditionOverride)
{
  prepareRegularFosParams(trx, stats, pqConditionOverride);
  prepareDirectFosParams(trx, stats);
}

void
SolFosTaskScope::prepareRegularFosParams(const ShoppingTrx& trx,
                                         const ItinStatistic& stats,
                                         bool pqConditionOverride)
{
  const bool isOneWayRequest = (trx.legs().size() == 1);

  // prepare flags
  prepareFosFlags(pqConditionOverride);

  // setup custom solutions
  if (trx.getNumOfCustomSolutions() > 0)
  {
    FosTaskScope::_numCustomFos = std::max(
        trx.diversity().getMinimalNumCustomSolutions() - stats.getCustomSolutionCount(), 0);
  }

  // setup regular solutions
  uint32_t numPricedSolutions = trx.flightMatrix().size() + trx.estimateMatrix().size();
  uint32_t reqNumOfSolutions =
      static_cast<uint32_t>(trx.getOptions()->getRequestedNumberOfSolutions());

  uint32_t fosActivationLevel = static_cast<uint32_t>(
      ceil((double)reqNumOfSolutions *
           ((double)flightOnlySolutionsActivationThreshold.getValue() / 100.0)));

  bool isBrandedFaresPath = trx.getRequest()->isBrandedFaresRequest();

  bool isRegularSolutionsNeeded =
      (numPricedSolutions < reqNumOfSolutions && (isOneWayRequest || isBrandedFaresPath)) ||
      numPricedSolutions <= fosActivationLevel;

  if (isRegularSolutionsNeeded)
  {
    uint32_t numMissingSolutions = reqNumOfSolutions - numPricedSolutions;
    uint32_t numFOSSolutionsToFind = 0;

    Boundary boundary(
        TravelSegAnalysis::selectTravelBoundary(trx.legs()[0].sop().front().itin()->travelSeg()));

    if (isBrandedFaresPath)
      numFOSSolutionsToFind = numMissingSolutions;
    else if (isOneWayRequest || boundary == Boundary::AREA_21) // for OW or within Europe
      numFOSSolutionsToFind = std::min(numMissingSolutions, numFlightOnlySolutionsOWCfg.getValue());
    else
      numFOSSolutionsToFind = std::min(numMissingSolutions, numFlightOnlySolutionsCfg.getValue());

    calculateOnlineInterlineFosParams(trx, stats, numFOSSolutionsToFind);
  }

  // setup long connection solutions
  if (FosTaskScope::getNumFos() == 0 && FosTaskScope::getNumCustomFos() <= 0)
    return;

  if (trx.diversity().getMaxLongConnectionSolutions())
  {
    FosTaskScope::_numLongConxFos =
        std::max(static_cast<int32_t>(trx.diversity().getMaxLongConnectionSolutions()) -
                     stats.getLongConnectionCount(),
                 0);
  }
}

void
SolFosTaskScope::prepareFosFlags(bool pqConditionOverride)
{
  FosTaskScope::_pqConditionOverride = pqConditionOverride && pqConditionCfg.getValue();
  FosTaskScope::_checkConnectingCities = checkConnectingCitiesCfg.getValue();
  FosTaskScope::_checkConnectingFlights = checkConnectingFlightsCfg.getValue();
}

void
SolFosTaskScope::calculateOnlineInterlineFosParams(const ShoppingTrx& trx,
                                                   const ItinStatistic& stats,
                                                   uint32_t totalNumFOS)
{
  std::set<CarrierCode> onlineCarriers;
  bool isInterlineApplicable = false;

  FosCommonUtil::collectOnlineCarriers(trx, onlineCarriers, isInterlineApplicable);
  isInterlineApplicable = isInterlineApplicable && trx.legs().size() == 2;

  uint32_t numCarriers = onlineCarriers.size() + (isInterlineApplicable ? 1 : 0);
  if (numCarriers == 0)
    return;

  uint32_t numFOSPerCarrier =
      static_cast<uint32_t>(std::ceil(static_cast<double>(totalNumFOS) / numCarriers));

  FosTaskScope::_numOnlineFos = numFOSPerCarrier * onlineCarriers.size();
  for (const CarrierCode& cxr : onlineCarriers)
  {
    FosTaskScope::_numFosPerCarrier[cxr] = numFOSPerCarrier;
  }
  if (isInterlineApplicable)
  {
    double numInterline = static_cast<double>(numFOSPerCarrier);
    FosTaskScope::_numSnowmanFos =
        static_cast<uint32_t>(std::ceil(numInterline * numSnowmanSolutions.getValue() / 100.0));
    FosTaskScope::_numDiamondFos =
        static_cast<uint32_t>(std::ceil(numInterline * numDiamondSolutions.getValue() / 100.0));
    FosTaskScope::_numTriangleFos =
        static_cast<uint32_t>(std::ceil(numInterline * numTriangleSolutions.getValue() / 100.0));
  }
}

void
SolFosTaskScope::prepareDirectFosParams(const ShoppingTrx& trx, const ItinStatistic& stats)
{
  const Diversity& diversity(trx.diversity());
  if (!diversity.isDirectCarriersCapable())
    return;

  size_t numDirectFOS(0);

  typedef std::pair<CarrierCode, size_t> MapElem;
  for (const MapElem& pricedNonStopPerCxr : stats.getNumOfNonStopItinsPerCarrier())
  {
    const CarrierCode cxr = pricedNonStopPerCxr.first;
    const size_t numNSNeeded =
        std::min(diversity.getMaxNonStopCountFor(cxr), numDirectFOSPerOnlineCarrier.getValue());

    if (pricedNonStopPerCxr.second < numNSNeeded)
    {
      size_t reqNumOfFos = (numNSNeeded - pricedNonStopPerCxr.second);
      numDirectFOS += reqNumOfFos;
      FosTaskScope::_numDirectFosPerCarrier[cxr] = static_cast<uint32_t>(reqNumOfFos);
    }
  }
  FosTaskScope::_numDirectFos = static_cast<uint32_t>(numDirectFOS);

  prepareAdditionalDirectFosParams(diversity, stats);
}

void
SolFosTaskScope::prepareAdditionalDirectFosParams(const Diversity& diversity,
                                                  const ItinStatistic& stats)
{
  typedef std::pair<CarrierCode, uint32_t> MapElem;
  typedef std::map<CarrierCode, uint32_t>::const_iterator NumCarrierFosCI;

  int additionalFosRequired =
      int(additionalPercentage.getValue() *
          float((diversity.getNonStopOptionsCount() - stats.getAdditionalNonStopsCount())));

  if (additionalFosRequired <= 0)
    return;

  std::vector<MapElem> additionalNonStopsVec;

  for (const MapElem& additionalNsCarrier : stats.getAdditionalNonStopCountPerCarrier())
  {
    CarrierCode cxr = additionalNsCarrier.first;

    NumCarrierFosCI findIt = FosTaskScope::_numDirectFosPerCarrier.find(cxr);
    size_t numToProcess =
        (findIt != FosTaskScope::_numDirectFosPerCarrier.end()) ? findIt->second : 0;

    uint32_t canHaveMore =
        std::max(diversity.getMaxNonStopCountFor(cxr) - (stats.getNumOfNonStopItinsForCarrier(cxr) +
                                                         numToProcess + additionalNsCarrier.second),
                 size_t(0));

    if (canHaveMore == 0)
      continue;

    additionalNonStopsVec.push_back(MapElem(cxr, canHaveMore));
  }

  FosTaskScope::_numDirectFos += static_cast<uint32_t>(additionalFosRequired);

  FosCommonUtil::diversifyOptionsNumPerCarrier(static_cast<uint32_t>(additionalFosRequired),
                                               additionalNonStopsVec,
                                               FosTaskScope::_numDirectFosPerCarrier);
  return;
}

} // fos
} // tse
