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
#include "Pricing/Shopping/FOS/FosCommonUtil.h"

#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

#include <algorithm>

namespace tse
{
namespace fos
{

const CarrierCode FosCommonUtil::INTERLINE_CARRIER = "";

CarrierCode
FosCommonUtil::detectCarrier(const ShoppingTrx& trx, const SopIdVec& combination)
{
  CarrierCode cxr = trx.legs()[0].sop()[combination[0]].governingCarrier();
  for (uint32_t legId = 1; legId < combination.size(); ++legId)
    if (cxr != trx.legs()[legId].sop()[combination[legId]].governingCarrier())
      return INTERLINE_CARRIER;
  return cxr;
}

void
FosCommonUtil::collectOnlineCarriers(const ShoppingTrx& trx,
                                     std::set<CarrierCode>& onlineCarriers,
                                     bool& isInterlineApplicable)
{
  std::size_t maxNumCarriers = 0;
  onlineCarriers.clear();

  for (std::size_t legIdx = 0; legIdx < trx.legs().size(); ++legIdx)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIdx];

    std::set<CarrierCode> legCarriers;
    for (const auto& sop : leg.sop())
    {
      if (sop.cabinClassValid())
        legCarriers.insert(sop.governingCarrier());
    }

    if (legCarriers.empty())
    {
      onlineCarriers.clear();
      isInterlineApplicable = false;
      return;
    }

    maxNumCarriers = std::max(maxNumCarriers, legCarriers.size());

    if (legIdx == 0)
    {
      std::swap(onlineCarriers, legCarriers);
    }
    else
    {
      std::set<CarrierCode> oldOnlineCarriers;
      std::swap(oldOnlineCarriers, onlineCarriers);
      std::set_intersection(oldOnlineCarriers.begin(),
                            oldOnlineCarriers.end(),
                            legCarriers.begin(),
                            legCarriers.end(),
                            std::inserter(onlineCarriers, onlineCarriers.begin()));
    }
  }

  isInterlineApplicable = false;
  if (trx.legs().size() > 1)
  {
    isInterlineApplicable = isInterlineApplicable || (maxNumCarriers > 1);
    isInterlineApplicable =
        isInterlineApplicable || (onlineCarriers.empty() && maxNumCarriers == 1);
  }
}

std::size_t
FosCommonUtil::calcNumOfValidSops(const ShoppingTrx& trx)
{
  std::size_t noOfValidSops = 0;
  for (const auto& leg : trx.legs())
  {
    for (const auto& sop : leg.sop())
    {
      if (sop.cabinClassValid())
        ++noOfValidSops;
    }
  }
  return noOfValidSops;
}

bool
FosCommonUtil::checkNumOfTravelSegsPerLeg(const ShoppingTrx& trx,
                                          const SopIdVec& comb,
                                          std::size_t numTS)
{
  for (std::size_t legNo = 0; legNo < comb.size(); ++legNo)
  {
    const int sopNo = comb[legNo];
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legNo].sop()[sopNo];

    if (sop.itin()->travelSeg().size() > numTS)
      return true;
  }
  return false;
}

bool
FosCommonUtil::checkTotalNumOfTravelSegs(const ShoppingTrx& trx,
                                         const SopIdVec& comb,
                                         std::size_t numTS)
{
  std::size_t noOfSegments = 0;
  for (std::size_t legIdx = 0; legIdx < comb.size(); ++legIdx)
    noOfSegments += trx.legs()[legIdx].sop()[comb[legIdx]].itin()->travelSeg().size();
  return noOfSegments > numTS;
}

bool
FosCommonUtil::checkConnectingCitiesRT(const ShoppingTrx& trx, const SopIdVec& comb)
{
  const std::vector<TravelSeg*>& tvlSegsIn = trx.legs()[0].sop()[comb[0]].itin()->travelSeg();
  const std::vector<TravelSeg*>& tvlSegsOut = trx.legs()[1].sop()[comb[1]].itin()->travelSeg();

  if (tvlSegsIn.size() != 2 || tvlSegsOut.size() != 2)
    return true;

  if (tvlSegsIn.front()->origin() == nullptr && tvlSegsOut.front()->origin() == nullptr)
    return false;

  return (tvlSegsIn.front()->origin() == nullptr || tvlSegsOut.front()->origin() == nullptr ||
          tvlSegsIn.front()->destination()->loc() != tvlSegsOut.front()->destination()->loc());
}

namespace
{
class NumPerCarrierLess
{
public:
  bool operator()(const FosCommonUtil::CarrierMapElem& a, const FosCommonUtil::CarrierMapElem& b)
  {
    return (a.second < b.second);
  }
};
} // namespace

void
FosCommonUtil::diversifyOptionsNumPerCarrier(uint32_t totalOptionsRequired,
                                             std::vector<CarrierMapElem>& availableVec,
                                             CarrierMap& requiredMap)
{
  std::sort(availableVec.begin(), availableVec.end(), NumPerCarrierLess());

  uint32_t carriersCount = availableVec.size();

  for (const CarrierMapElem& carrierElem : availableVec)
  {
    CarrierCode cxr = carrierElem.first;

    uint32_t optionsToFind = std::min(carrierElem.second, totalOptionsRequired / carriersCount);

    carriersCount--;

    if (optionsToFind == 0)
      continue;

    requiredMap[cxr] += optionsToFind;
    totalOptionsRequired -= optionsToFind;
  }

  if (totalOptionsRequired > 0) // we cannot collect enough online non-stops
  {
    requiredMap[Diversity::INTERLINE_CARRIER] += totalOptionsRequired;
  }
}

bool
FosCommonUtil::applyNonRestrictionFilter(const ShoppingTrx& trx,
                                         const FosStatistic& stats,
                                         bool isPqOverride,
                                         bool isNoPricedSolutions)
{
  // NOTE: Apply non-restriction filter if:
  //      isPqOverride || 'there are some online solutions lacking' ||
  //      ((!generatedFosSolutions || !isNoPricedSolutions) && isOw)
  const bool isOw = trx.legs().size() == 1;
  bool generatedFosSolutions = false;

  if (isPqOverride || (!isNoPricedSolutions && isOw))
    return true;

  const FosStatistic::CarrierCounterMap& cxrCounterMap = stats.getCarrierCounterMap();
  FosStatistic::CarrierCounterMap::const_iterator it = cxrCounterMap.begin();
  FosStatistic::CarrierCounterMap::const_iterator itEnd = cxrCounterMap.end();
  for (; it != itEnd; ++it)
  {
    if (it->second.value < it->second.limit)
    {
      return true;
    }
    if (it->second.value)
    {
      generatedFosSolutions = true;
    }
  }

  if (!isOw)
    return false;

  if (!generatedFosSolutions)
  {
    const FosStatistic::ValidatorCounters& counters = stats.getValidatorCounters();
    for (uint32_t i = 0; i <= VALIDATOR_LAST; ++i)
    {
      if (counters[i] && i != VALIDATOR_NONSTOP)
      {
        return false;
      }
    }
    return true;
  }

  return false;
}

} // fos
} // tse
