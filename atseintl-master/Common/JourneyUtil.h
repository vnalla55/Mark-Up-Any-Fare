#pragma once


#include <map>
#include <set>
#include <vector>

namespace tse
{

class TravelSeg;
class FareMarket;
class PricingTrx;
class ClassOfService;
class FarePath;
class Itin;
class OAndDMarket;

class JourneyUtil
{

public:
  typedef std::map<const TravelSeg*, const OAndDMarket*> SegOAndDMap;
  static constexpr int MAX_MARRIED_SEGS = 3;

  static void addOAndDMarket(PricingTrx& trx,
                             Itin& itin,
                             FarePath* fpath,
                             FareMarket* fm,
                             const bool isLocalJourneyCarrier = false,
                             const std::map<const TravelSeg*, bool>* marriedSegs = nullptr);

  static std::vector<ClassOfService*>* availability(const TravelSeg* tvlSeg, Itin* itin);

  static void initOAndDCOS(Itin* itin);

  static OAndDMarket* getOAndDMarketFromFM(const Itin& itin, const FareMarket* fm);

  static void
  getThruFMs(const Itin* itin, const FarePath* fpath, std::vector<FareMarket*>& thruFMs);

  static const OAndDMarket*
  getOAndDMarketFromSegment(const TravelSeg* seg, const Itin* itin, const FarePath* fpath = nullptr);

  static const FareMarket*
  getFlowMarketFromSegment(const TravelSeg* seg, const Itin* itin, const FarePath* fpath = nullptr);

  static bool checkIfFmSegInFlowOd(const FareMarket* fm, const SegOAndDMap& oAndDMap);

  static bool checkIfSegInFlowOd(const TravelSeg* seg, const SegOAndDMap& oAndDMap);

  static bool getMarriedSegments(const PricingTrx& trx,
                                 const FareMarket& fm,
                                 std::map<const TravelSeg*, bool>& marriedSegments,
                                 bool& journeyByMarriage,
                                 bool& outOfSeqMarriage);

  static bool journeyActivated(PricingTrx& trx);
  static bool useJourneyLogic(PricingTrx& trx);
  static bool usePricingJourneyLogic(PricingTrx& trx);
  static bool useShoppingJourneyLogic(PricingTrx& trx);

private:
  static void addToSegmentOAndDMarketMap(const OAndDMarket* od, SegOAndDMap& oAndDMap);

  static const OAndDMarket*
  getOAndDMarketFromSegMap(const TravelSeg* tvlSeg, const SegOAndDMap& oAndDMap);

  static void addThruFMs(const std::vector<OAndDMarket*>& oAndDs, std::set<FareMarket*>& thruFMs);
};

} // namespace tse

