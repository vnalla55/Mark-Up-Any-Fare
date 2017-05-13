#pragma once

#include "Common/Logger.h"

#include <utility>
#include <vector>

namespace tse
{
class FareMarket;
class Itin;
class Loc;
class TravelSeg;

/**
 * This class stores original and destination mileage for each leg,
 * thus shall not share one instance between transactions
 */
class SkipFareMarketByMileageSOL
{
  friend class SkipFareMarketByMileageSolMock;

public:
  using Mileage = int32_t;

  SkipFareMarketByMileageSOL();
  virtual ~SkipFareMarketByMileageSOL() = default;

  /**
   * Adaptor for skipByConnectingCityMileage(2 params) above.
   *
   * @param ts is travel segment vec, which is a subset from itin, contains sequence that starts
   *        either with itin->travelSeg().begin(), or ends with itin->travelSeg().end()
   */
  std::pair<bool, std::string> skipByConnectingCityMileage(const FareMarket& fm,
                                                           const std::vector<TravelSeg*>& ts,
                                                           const Itin* itin,
                                                           bool explain = false) const;

  bool isThruMileageInitialized() const { return _thruMileage != 0; }

  bool skipLocalFMByLegOndMileage(int legIdx, const Itin* itin);
  void initializeThruMilage(const FareMarket* fareMarket);

  std::pair<bool, std::string>
  skipByConnectingCityMileage(const FareMarket& fm,
                              const std::vector<TravelSeg*>& fareMarketTravelSegVec,
                              std::vector<TravelSeg*>::const_iterator connXpoint,
                              bool explain) const;

private:
  virtual Mileage getMileage(const Loc& loc1, const Loc& loc2) const;

  float _fareMarketMileageLimit = 0;
  float _fareMarketMileageLimitAbove5000 = 0;
  int32_t _localFMLegOndMileageLimit = 0;
  std::vector<Mileage> _legOndMileage; // original and destination mileage for each leg
  Mileage _thruMileage = 0;
  Mileage _realThruMileage = 0;

  static Logger _logger;
};

} // tse

