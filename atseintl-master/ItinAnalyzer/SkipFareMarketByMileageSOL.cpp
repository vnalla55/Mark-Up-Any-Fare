#include "ItinAnalyzer/SkipFareMarketByMileageSOL.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <algorithm>
#include <iostream>

namespace tse
{
Logger
SkipFareMarketByMileageSOL::_logger("atseintl.ItinAnalyzer.SkipFareMarketByMileageSOL");

namespace
{
ConfigurableValue<float>
fareMarketMileageLimit("SHOPPING_DIVERSITY", "FAREMARKET_MILEAGE_LIMIT_PERCENTAGE", 0);
ConfigurableValue<float>
fareMarketMileageLimitAbove5000("SHOPPING_DIVERSITY",
                                "FAREMARKET_MILEAGE_LIMIT_PERCENTAGE_ABOVE_5000",
                                0);
ConfigurableValue<int32_t>
localFMLegOndMileageLimit("SHOPPING_DIVERSITY", "LOCAL_FM_LEG_OND_MILEAGE_LIMIT", 0);

struct SkipByMileageExplanation
{
  SkipByMileageExplanation(int32_t calculatedThruFareMarketMileage,
                           int32_t connPointMileage,
                           int32_t realThruFareMarketMileage)
    : _calculatedThruFareMarketMileage(calculatedThruFareMarketMileage),
      _connPointMileage(connPointMileage),
      _realThruFareMarketMileage(realThruFareMarketMileage)
  {
  }

  /**
     * A         B
     *  \       /
     *   \     /
     *    C x D
     */
  LocCode _locA, _locB;
  LocCode _locC, _locD;

  /**
     * @implement
     */
  void printTo(std::ostream& os)
  {
    os << "mileage of " << _locA << "-" << _locC << "+" << _locD << "-" << _locB << " ("
       << _connPointMileage << " miles)"
       << " is higher than calculated thru fare market mileage ("
       << _calculatedThruFareMarketMileage
       << " miles), real thru mileage: " << _realThruFareMarketMileage;
  }

private:
  int32_t _calculatedThruFareMarketMileage;
  int32_t _connPointMileage;
  int32_t _realThruFareMarketMileage;
};

} // namespace

SkipFareMarketByMileageSOL::SkipFareMarketByMileageSOL()
  : _fareMarketMileageLimit(fareMarketMileageLimit.getValue() / 100),
    _fareMarketMileageLimitAbove5000(fareMarketMileageLimitAbove5000.getValue() / 100),
    _localFMLegOndMileageLimit(localFMLegOndMileageLimit.getValue())
{
}

std::pair<bool, std::string>
SkipFareMarketByMileageSOL::skipByConnectingCityMileage(
    const FareMarket& fm,
    const std::vector<TravelSeg*>& fareMarketTravelSegmentVec,
    std::vector<TravelSeg*>::const_iterator connXpoint,
    bool explain) const
{
  const std::pair<bool, std::string> PASS(false, std::string());

  if (connXpoint == fareMarketTravelSegmentVec.begin() ||
      connXpoint == fareMarketTravelSegmentVec.end())
  {
    // not applicable for thru fare markets,
    // thus filtering is not in effect
    return PASS;
  }

  if (fm.geoTravelType() == GeoTravelType::ForeignDomestic)
  {
    // not applicable as far produces poor results for foreign-domestic request
    return PASS;
  }

  if (UNLIKELY(!_fareMarketMileageLimit))
  {
    // not set in ACMS
    return PASS;
  }

  const Loc& origin = *fareMarketTravelSegmentVec.front()->origin();
  const Loc& destination = *fareMarketTravelSegmentVec.back()->destination();
  const Loc& connPointLoc1 = *(**(connXpoint - 1)).destination();
  const Loc& connPointLoc2 = *(**connXpoint).origin();

  Mileage connPointMileage =
      getMileage(origin, connPointLoc1) + getMileage(connPointLoc2, destination);

  if (_thruMileage < connPointMileage)
  {
    std::string skipReason;
    if (UNLIKELY(explain))
    {
      SkipByMileageExplanation explanation(_thruMileage, connPointMileage, _realThruMileage);
      explanation._locA = origin.loc();
      explanation._locB = destination.loc();
      explanation._locC = connPointLoc1.loc();
      explanation._locD = connPointLoc2.loc();

      std::stringstream os;
      explanation.printTo(os);
      skipReason = os.str();
    }

    return std::make_pair(true, skipReason);
  }

  return PASS;
}

void
SkipFareMarketByMileageSOL::initializeThruMilage(const FareMarket* fareMarket)
{
  if (!fareMarket)
    return;

  _realThruMileage = getMileage(*fareMarket->origin(), *fareMarket->destination());
  if (_realThruMileage > 5000.0f)
    _thruMileage = _localFMLegOndMileageLimit * _fareMarketMileageLimitAbove5000;
  else
    _thruMileage = (int32_t)(
        (float)_realThruMileage *
        (_fareMarketMileageLimit + _fareMarketMileageLimit * sinf(log10f(_realThruMileage))));
}

std::pair<bool, std::string>
SkipFareMarketByMileageSOL::skipByConnectingCityMileage(const FareMarket& fm,
                                                        const std::vector<TravelSeg*>& ts,
                                                        const Itin* itin,
                                                        bool explain) const
{
  if (UNLIKELY(!_fareMarketMileageLimit))
  {
    // not set in ACMS
    return std::make_pair(false, std::string());
  }

  const std::vector<TravelSeg*>& itinTs = itin->travelSeg();
  std::vector<TravelSeg*>::const_iterator connXpoint;

  if (ts.front() == itinTs.front())
  {
    connXpoint = itinTs.begin() + ts.size();
  }
  else if (LIKELY(ts.back() == itinTs.back()))
  {
    connXpoint = itinTs.end() - ts.size();
    TSE_ASSERT(*connXpoint == ts.front());
  }
  else
  {
    TSE_ASSERT(!"invalid arguments");
  }

  return skipByConnectingCityMileage(fm, itinTs, connXpoint, explain);
}

bool
SkipFareMarketByMileageSOL::skipLocalFMByLegOndMileage(int legIdx, const Itin* itin)
{
  if (UNLIKELY(!_localFMLegOndMileageLimit))
  {
    // not set in ACMS
    return false;
  }

  const bool firstCallForThisLeg =
      (static_cast<int>(_legOndMileage.size()) <= legIdx || !_legOndMileage[legIdx]);
  if (firstCallForThisLeg)
  {
    // calc leg original and destination mileage for this leg

    const Loc* origin = itin->travelSeg().front()->origin();
    const Loc* destination = itin->travelSeg().back()->destination();
    const Mileage mileage = getMileage(*origin, *destination);

    // store for the next call
    if (static_cast<int>(_legOndMileage.size()) <= legIdx)
      _legOndMileage.resize(legIdx + 1, /*not initialized by default*/ 0);
    _legOndMileage[legIdx] = mileage;
  }

  bool skip = (_legOndMileage[legIdx] > _localFMLegOndMileageLimit);
  return skip;
}

SkipFareMarketByMileageSOL::Mileage
SkipFareMarketByMileageSOL::getMileage(const Loc& loc1, const Loc& loc2) const
{
  return TseUtil::greatCircleMiles(loc1, loc2);
}
}
