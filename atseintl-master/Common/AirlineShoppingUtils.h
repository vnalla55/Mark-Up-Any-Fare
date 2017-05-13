#pragma once

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "Common/TravelSegAnalysis.h"

#include <log4cxx/helpers/objectptr.h>

#include <functional>

namespace tse
{
class FarePath;
class GroupFarePathFactory;
class GroupFarePath;
class PaxTypeFare;
class PricingUnit;
class PricingTrx;

namespace AirlineShoppingUtils
{

std::ostream& operator<<(std::ostream& os, const TravelSeg& ts);
std::ostream& operator<<(std::ostream& os, const std::vector<TravelSeg*>& schedule);

enum LegId
{ FIRST_LEG = 0,
  SECOND_LEG };

std::string
collectSegmentInfo(const PricingTrx& trx, const FarePath* fp);

class AseItinCombiner
{
public:
  typedef std::map<std::pair<DateTime, DateTime>, std::vector<FarePath*> > DateToSolutionMap;
  typedef DateToSolutionMap::iterator DateToSolutionMapI;
  typedef DateToSolutionMap::const_iterator DateToSolutionMapCI;

  AseItinCombiner(PricingTrx& trx) : _trx(trx) {}
  void combineOneWayItineraries();
  void combineItins(PricingTrx& trx, Itin* outbound, Itin* inbound);

  FarePath* combineFarePaths(PricingTrx& trx, FarePath* outbOption, FarePath* inbOption);

  void setupCombinedItin(PricingTrx& trx, Itin* outbound, Itin* inbound, Itin* resultItin);

  void combineSolutions(std::vector<Itin*>& outboundItins,
                        std::vector<Itin*>& inboundItins);

  void combineItinSolutions(uint16_t brandIndex, FarePath* outboundSol, FarePath* inboundSol);

  bool getSolutions(const std::vector<Itin*>& itins,
                    std::vector<FarePath*>& allSolutions) const;

  void combineOutboundInboundSolutions(std::vector<FarePath*>& outbSolutions,
                                       std::vector<FarePath*>& inbSolutions);

  void setTripCharacteristics(Itin* itin);
  void clearCache() { _outboundInboundToResultCache.clear(); }

private:
  static log4cxx::LoggerPtr _logger;
  PricingTrx& _trx;
  std::map<std::pair<Itin*, Itin*>, Itin*> _outboundInboundToResultCache;
  TravelSegAnalysis _tvlSegAnalysis;
};

void
retrieveBrandInfo(PricingTrx& trx,
                  GroupFarePathFactory& groupFarePathFactory,
                  uint16_t& brandIndex,
                  BrandCode& brandCode);

void
setSolutionBrandInfo(PricingTrx& trx,
                     GroupFarePathFactory& groupFarePathFactory,
                     GroupFarePath& groupFPath);

// convenience methods for diagnostics
std::string
brandsAsString(PricingTrx const& trx, std::set<uint16_t> const& indexes);
std::string
getBrandIndexes(PricingTrx const& trx, const FarePath& fp);
std::string
getBrandIndexes(PricingTrx const& trx, const PricingUnit& pu);
std::string
getBrandIndexes(PricingTrx const& trx, const PaxTypeFare& ptf);

bool
enableItinLevelThreading(const PricingTrx& trx);
}
} // tse

