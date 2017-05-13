//----------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Pricing/PQDiversifier.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <cmath>
#include <iostream>

using namespace std;
using namespace boost;

namespace tse
{
namespace
{
Logger
logger("atseintl.Pricing.PQDiversifier");

ConfigurableValue<double>
pqDiversityPercentage("SHOPPING_OPT", "PQ_DIVERSITY_PERCENTAGE", 0.10);
ConfigurableValue<uint32_t>
interlineWeightFactor("SHOPPING_OPT", "INTERLINE_WEIGHT_FACTOR", 4);
ConfigurableValue<uint32_t>
interlineLessWeightFactor("SHOPPING_OPT", "INTERLINE_LESS_WEIGHT_FACTOR", 3);
ConfigurableValue<double>
minimumCustomSolutionPercentage("SHOPPING_OPT", "MINIMUM_CUSTOM_SOLUTION_PERCENTAGE", 100.0);
}

PQDiversifier::PQDiversifier()
  : _pqDiversityPercentage(0.10), _interlineWeightFactor(4), _interlineLessWeightFactor(3)
{
}

void
PQDiversifier::init()
{
  _pqDiversityPercentage = pqDiversityPercentage.getValue();
  _interlineWeightFactor = interlineWeightFactor.getValue();
  _interlineLessWeightFactor = interlineLessWeightFactor.getValue();
}

void
PQDiversifier::calculateDiversityNumbers(ShoppingTrx& trx) const
{
  ShoppingTrx::PQDiversifierResult& results = trx.pqDiversifierResult();

  if (trx.getNumOfCustomSolutions())
  {
    results._minNumCustomSolutions = static_cast<uint32_t>(
        ceil(trx.getNumOfCustomSolutions() * minimumCustomSolutionPercentage.getValue() / 100.00));
  }

  const std::vector<CarrierCode>& onlineCarrierList = results.onlineCarrierList;
  const uint16_t numberRequestOptions = trx.getOptions()->getRequestedNumberOfSolutions();
  const uint32_t numberOnlineCarriers = onlineCarrierList.size();
  LOG4CXX_DEBUG(logger, "- Number Requested Options  = " << numberRequestOptions);
  LOG4CXX_DEBUG(logger, "- Number Of Online Carriers = " << numberOnlineCarriers);

  // Perform diversity formula calculations

  if (trx.interlineSolutionsOnly() || trx.noDiversity() ||
      (numberOnlineCarriers == 0 && !trx.onlineSolutionsOnly()))
  {
    results.numberInterlineOptions = numberRequestOptions;
    results._numberOnlineOptionsPerCarrier = 0;
    return;
  }

  if (trx.onlineSolutionsOnly() || isOnlyOnlinesPossible(trx))
  {
    if (numberOnlineCarriers == 0)
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "No online carriers when online-only solution requested");
    }

    results.numberInterlineOptions = 0;
    results._numberOnlineOptionsPerCarrier = numberRequestOptions / numberOnlineCarriers;

    if ((numberRequestOptions % numberOnlineCarriers) > 0)
    {
      ++results._numberOnlineOptionsPerCarrier;
    }

    countOnlineSolutions(trx, numberRequestOptions, 0);
    return;
  }

  const double weightAdjustor =
      (numberRequestOptions > 19) ? _interlineLessWeightFactor : _interlineWeightFactor;
  const bool intellisellWeightPresent = (trx.interlineWeightFactor() > 0);
  double weightFactor = (intellisellWeightPresent) ? trx.interlineWeightFactor() : weightAdjustor;

  if ((!intellisellWeightPresent) && (numberOnlineCarriers < PQ_DIVERSITY_ADJUST_MAX_CXRS))
  {
    if (numberOnlineCarriers == PQ_DIVERSITY_ADJUST_MIN_CXRS)
    {
      weightFactor = 0;
    }
    else
    {
      // Multiplies by 0.5 (by default) to cut in half
      weightFactor = numberOnlineCarriers * PQ_DIVERSITY_ADJUST_MULTIPLIER;
    }
  }

  // Diversity formula 1 (see PQDiversifier.h)
  double onlineResult = (numberRequestOptions * (1.0 + _pqDiversityPercentage)) /
                        (numberOnlineCarriers + weightFactor);
  // Diversity formula 2 (see PQDiversifier.h)
  double interlineResult = 0;

  if (!trx.isAltDates())
  {
    interlineResult = (onlineResult * weightFactor);
  }
  else
  {
    interlineResult = (onlineResult * 1);
  }

  // Round the online result up to a whole number
  results._numberOnlineOptionsPerCarrier =
      std::max<uint32_t>(1, static_cast<uint32_t>(ceil(onlineResult)));
  // Round the interline result up to a whole number
  results.numberInterlineOptions =
      std::max<uint32_t>(1, static_cast<uint32_t>(ceil(interlineResult)));
  LOG4CXX_DEBUG(logger, "Final results:");
  LOG4CXX_DEBUG(logger, "- Number Online Options    = " << results._numberOnlineOptionsPerCarrier);
  LOG4CXX_DEBUG(logger, "- Number Interline Options = " << results.numberInterlineOptions);
  countOnlineSolutions(trx, numberRequestOptions, interlineResult);
}

//----------------------------------------------------------------------------------------------
void
PQDiversifier::countOnlineSolutions(ShoppingTrx& trx,
                                    const double numRequestOptions,
                                    const double numberInterlineOptions) const
{
  ShoppingTrx::PQDiversifierResult& results = trx.pqDiversifierResult();
  countCxrSOP(trx);
  calculateMaxCombinationCount(results);
  // LOG4CXX_DEBUG(logger, "Distribution of Online CXR Options:");
  uint16_t extraOptions = 0;
  std::map<CarrierCode, ShoppingTrx::CxrOnlinOptions>::iterator cxrIt =
      results.cxrOnlinOptions.begin();
  std::map<CarrierCode, ShoppingTrx::CxrOnlinOptions>::iterator cxrItEnd =
      results.cxrOnlinOptions.end();

  if (!trx.diversity().hasDCL())
  {
    for (; cxrIt != cxrItEnd; ++cxrIt)
    {
      const uint16_t onlineCombos = cxrIt->second.maxPossibleCombinations;

      if (results._numberOnlineOptionsPerCarrier > onlineCombos)
      {
        // LOG4CXX_DEBUG(logger, "Extra options assigned to CXR:" << cxrIt->first)
        cxrIt->second.numberOnlineOptions = onlineCombos;
        extraOptions +=
            (results._numberOnlineOptionsPerCarrier - cxrIt->second.numberOnlineOptions);
      }
      else
      {
        cxrIt->second.numberOnlineOptions = results._numberOnlineOptionsPerCarrier;
      }

      results.totalOnlineOptions += cxrIt->second.numberOnlineOptions;
      // LOG4CXX_DEBUG(logger, "   - Number of Options for " << cxrIt->first << " = " <<
      // cxrIt->second.numberOnlineOptions);
    }

    // LOG4CXX_DEBUG(logger, "Total Online Options = " << results.totalOnlineOptions)
    // LOG4CXX_DEBUG(logger, "Unallocated Options = " << extraOptions)
  }
  else
  {
    const std::map<CarrierCode, size_t>& DCLMap(trx.diversity().getDCLMap());
    std::map<CarrierCode, size_t>::const_iterator carrieritEnd = DCLMap.end();

    for (; cxrIt != cxrItEnd; ++cxrIt)
    {
      const uint16_t onlineCombos = cxrIt->second.maxPossibleCombinations;

      if (results._numberOnlineOptionsPerCarrier > onlineCombos)
      {
        // LOG4CXX_DEBUG(logger, "Extra options assigned to CXR:" << cxrIt->first)
        cxrIt->second.numberOnlineOptions = onlineCombos;
        extraOptions +=
            (results._numberOnlineOptionsPerCarrier - cxrIt->second.numberOnlineOptions);
      }
      else
      {
        std::map<CarrierCode, size_t>::const_iterator carrierit = DCLMap.find(cxrIt->first);
        if (carrierit != carrieritEnd)
          cxrIt->second.numberOnlineOptions = (uint16_t)(carrierit->second);
        else
          cxrIt->second.numberOnlineOptions = results._numberOnlineOptionsPerCarrier;
      }

      results.totalOnlineOptions += cxrIt->second.numberOnlineOptions;
      // LOG4CXX_DEBUG(logger, "   - Number of Options for " << cxrIt->first << " = " <<
      // cxrIt->second.numberOnlineOptions);
    }
  }

  // LOG4CXX_DEBUG(logger, "Total Online Options = " << results.totalOnlineOptions)
  // LOG4CXX_DEBUG(logger, "Unallocated Options = " << extraOptions)

  if (extraOptions > 0)
  {
    allocateExtraOptions(results, extraOptions);
  }
}

//----------------------------------------------------------------------------------------------

void
PQDiversifier::allocateExtraOptions(ShoppingTrx::PQDiversifierResult& results,
                                    uint16_t extraOptions) const
{
  typedef std::map<CarrierCode, ShoppingTrx::CxrOnlinOptions>::value_type CxrOnlinOptionsItem;

  uint32_t totalPossibleOptions = 0u;
  for (const CxrOnlinOptionsItem& item : results.cxrOnlinOptions)
  {
    const ShoppingTrx::CxrOnlinOptions& cxrOnlinOptions = item.second;

    if (cxrOnlinOptions.maxPossibleCombinations > results._numberOnlineOptionsPerCarrier)
    {
      totalPossibleOptions += cxrOnlinOptions.maxPossibleCombinations;
    }
  }

  for (CxrOnlinOptionsItem& item : results.cxrOnlinOptions)
  {
    ShoppingTrx::CxrOnlinOptions& cxrOnlinOptions = item.second;

    if (cxrOnlinOptions.maxPossibleCombinations > results._numberOnlineOptionsPerCarrier)
    {
      const uint32_t extra = static_cast<uint32_t>(
          round(static_cast<double>(extraOptions * cxrOnlinOptions.maxPossibleCombinations) /
                static_cast<double>(totalPossibleOptions)));
      TSE_ASSERT(extra <= 0xFFFF);

      cxrOnlinOptions.numberOnlineOptions += static_cast<uint16_t>(extra);
      results.totalOnlineOptions += static_cast<uint16_t>(extra);
    }
  }
}

//----------------------------------------------------------------------------------------------
void
PQDiversifier::countCxrSOP(ShoppingTrx& trx) const
{
  ShoppingTrx::PQDiversifierResult& results = trx.pqDiversifierResult();

  for (const ShoppingTrx::Leg& leg : trx.legs())
  {
    const ItinIndex::ItinMatrix& im = leg.carrierIndex().root();

    for (const CarrierCode& cxr : results.onlineCarrierList)
    {
      ShoppingTrx::CxrOnlinOptions& cxrOnlinOptions = results.cxrOnlinOptions[cxr];
      cxrOnlinOptions.carrier = cxr;

      ItinIndex::Key govCxrKey;
      ShoppingUtil::createKey(cxr, govCxrKey);

      const auto rowIt = im.find(govCxrKey);

      if (rowIt == im.end())
      {
        cxrOnlinOptions.cxrSOPCount.push_back(0); // no sop for this cxr in this leg
        continue;
      }

      size_t sopCount = 0;
      const ItinIndex::ItinRow& itinRow = rowIt->second;

      for (const auto& columnIt : itinRow)
      {
        const ItinIndex::ItinColumn& col = columnIt.second;
        sopCount += col.size();
      }

      cxrOnlinOptions.cxrSOPCount.push_back(sopCount);
    }
  }
}

//----------------------------------------------------------------------------------------------

void
PQDiversifier::calculateMaxCombinationCount(ShoppingTrx::PQDiversifierResult& results) const
{
  std::map<CarrierCode, ShoppingTrx::CxrOnlinOptions>::iterator cxrIt =
      results.cxrOnlinOptions.begin();
  std::map<CarrierCode, ShoppingTrx::CxrOnlinOptions>::iterator cxrItEnd =
      results.cxrOnlinOptions.end();

  for (; cxrIt != cxrItEnd; ++cxrIt)
  {
    ShoppingTrx::CxrOnlinOptions& cxrOnlinOptions = cxrIt->second;
    const uint64_t cxrMaxDefaultOptions = 65000;
    uint64_t cxrMaxOptions = 1;
    std::vector<uint16_t>::iterator it = cxrOnlinOptions.cxrSOPCount.begin();
    std::vector<uint16_t>::iterator itEnd = cxrOnlinOptions.cxrSOPCount.end();

    for (; it != itEnd; ++it)
    {
      cxrMaxOptions *= *it;
    }

    if (cxrMaxOptions > cxrMaxDefaultOptions)
    {
      cxrOnlinOptions.maxPossibleCombinations = cxrMaxDefaultOptions;
    }
    else
    {
      cxrOnlinOptions.maxPossibleCombinations = static_cast<uint16_t>(cxrMaxOptions);
    }

    // LOG4CXX_DEBUG(logger, "- Possible Combo for Cxr: " << cxrOnlinOptions.carrier << " = " <<
    // cxrMaxOptions)
  }
}

//----------------------------------------------------------------------------------------------

namespace
{
CarrierCode
getCarrierItin(const Itin& itin)
{
  CarrierCode cxr;

  for (const auto ts : itin.travelSeg())
  {
    const AirSeg* const airSeg = ts->toAirSeg();

    if (!airSeg)
      continue;

    if (cxr.empty())
    {
      cxr = airSeg->marketingCarrierCode();
    }
    else if (cxr != airSeg->marketingCarrierCode())
    {
      return "";
    }
  }

  return cxr;
}
}

bool
PQDiversifier::isOnlyOnlinesPossible(ShoppingTrx& trx) const
{
  const Itin& itin = *trx.legs()[0].sop()[0].itin();
  const CarrierCode firstCxr = getCarrierItin(itin);

  for (std::vector<ShoppingTrx::Leg>::const_iterator leg = trx.legs().begin();
       leg != trx.legs().end();
       ++leg)
  {
    if (leg->stopOverLegFlag())
      continue;

    for (size_t n = 0; n != leg->requestSops(); ++n)
    {
      const Itin& itin = *leg->sop()[n].itin();
      const CarrierCode cxr = getCarrierItin(itin);

      if ((cxr.empty()) || (cxr != firstCxr))
      {
        return false;
      }
    }
  }

  return true;
}

void
PQDiversifier::process(ShoppingTrx& trx) const
{
  ShoppingTrx::PQDiversifierResult& results = trx.pqDiversifierResult();
  results.onlineCarrierList = collectOnlineCarriers(trx);
  calculateDiversityNumbers(trx);
  results.validResults = true;

  if (trx.isAltDates())
  {
    uint32_t numOfSolutionNeeded =
        calculateNumOfSolutionNeededForDatePair(trx.getOptions(), results);

    for (ShoppingTrx::AltDatePairs::iterator i = trx.altDatePairs().begin();
         i != trx.altDatePairs().end();
         ++i)
    {
      i->second->numOfSolutionNeeded = numOfSolutionNeeded;
      ShoppingTrx::FoundOptionInfo* foundOptionInfo = nullptr;
      trx.dataHandle().get(foundOptionInfo);
      std::pair<DatePair, ShoppingTrx::FoundOptionInfo*> mapItem(i->first, foundOptionInfo);
      // The line below is single thread process so there is no need to guard the map
      trx.altDateLowestAmount().insert(mapItem);
    }
  }
}

std::vector<CarrierCode>
PQDiversifier::collectOnlineCarriers(ShoppingTrx& trx) const
{
  // Calculate intersection of governing carries over all legs.
  // The invariant here is:
  //    1. set carriers[0] is a set of governing carriers on first leg
  //    2. set carriers[i] is an intersection of:
  //      * set carriers[i-1] and
  //      * set of direct carriers on i-th leg
  std::vector<std::set<CarrierCode>> carriers;

  for (size_t legId = 0; legId < trx.legs().size(); ++legId)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legId];
    if (leg.stopOverLegFlag())
      continue;

    carriers.push_back(std::set<CarrierCode>());
    const ItinIndex& itinIndex = leg.carrierIndex();

    // itin matrix iteration loop
    for (const auto& rowIt : itinIndex.root())
    {
      const ItinIndex::ItinCell* curCell =
          ShoppingUtil::retrieveDirectItin(trx, legId, rowIt.first, ItinIndex::CHECK_NOTHING);

      if (!curCell)
        continue;

      // Get the direct itinerary for this carrier
      const Itin* itin = curCell->second;

      if (!itin)
        continue;

      // get the thru-fare market
      const FareMarket* fM = itin->fareMarket().front();

      if (!fM)
        continue;

      const CarrierCode cxr = fM->governingCarrier();

      if (!cxr.empty() && (carriers.size() == 1 || carriers[carriers.size() - 2].count(cxr)))
      {
        carriers.back().insert(cxr);
      }
    }
  }

  return std::vector<CarrierCode>(carriers.back().begin(), carriers.back().end());
}

uint32_t
PQDiversifier::calculateNumOfSolutionNeededForDatePair(
    const PricingOptions* options, ShoppingTrx::PQDiversifierResult& diversityResults) const
{
  uint32_t result = 0;

  if (options->isEnableCalendarForInterlines())
    result = options->getRequestedNumberOfSolutions();
  else
    result = (diversityResults._numberOnlineOptionsPerCarrier > 0)
                 ? diversityResults._numberOnlineOptionsPerCarrier
                 : diversityResults.numberInterlineOptions;

  return result;
}
}
