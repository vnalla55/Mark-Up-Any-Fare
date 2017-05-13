//-------------------------------------------------------------------
//  File : RoutingController.h
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ItinIndex.h"
#include "Routing/DRVInfo.h"
#include "Routing/RoutingInfo.h"
#include "Util/FlatSet.h"

#include <boost/thread/mutex.hpp>

#include <map>
#include <set>
#include <vector>

namespace tse
{
class DataHandle;
class Routing;
class RoutingMap;
class PricingTrx;
class Fare;
class FareMarket;
class Loc;
class RtgKey;
class PaxTypeFare;
class RoutingRestriction;
class TravelRoute;
class ShoppingTrx;
class ExchangePricingTrx;

typedef std::map<RtgKey, bool> RtgKeyMap;
typedef std::map<RtgKey, bool>::iterator RtgKeyMapI;
typedef std::map<const RtgKey, RoutingInfo*> RoutingKeyMap;
typedef std::map<std::string, RoutingKeyMap*> TvlRouteMap;
typedef std::map<FareMarket*, TravelRoute*> FareMarketMap;

struct RoutingValue
{
  RoutingValue() : mpmPctg(0), status(false) {}
  uint16_t mpmPctg;
  bool status;
  RoutingInfos rtginfos;
};

typedef std::map<RtgKey, RoutingValue> ShoppingRtgMap;

/**
 * @class RoutingController
 * Manages the routing validation process of a FareMarket.
 * Primary responsibilities are to retrieve routeMap and
 * routeRestriction from database and to instantiate appropriate
 * validation class for routing validation.
 *
 * */
class RoutingController
{
protected:
  using RoutingList = std::vector<const Routing*>;
  using RoutingListI = RoutingList::iterator;
  using RoutingListIC = RoutingList::const_iterator;
  using fbrPaxTypeFareVector = std::vector<PaxTypeFare*>;

  using RoutingComponent = uint16_t;

public:
  friend class RoutingControllerTest;

  explicit RoutingController(PricingTrx&);

  virtual ~RoutingController() = default;

  /**
   * Prepares routing data and process the routing validation.
   * This method is called from routing validator orchestrator.
   * @param trx PricingTrx A reference to the Itin object.
   * @param fareMarket FareMarket A reference to FareMarket object.
   * @returns bool
   *
   * */
  bool buildTvlRouteMapAndProcess(FareMarket&);
  bool validatePaxTypeFare(FareMarket&);
  bool process(FareMarket&, TravelRoute&, RoutingInfos&);
  virtual bool process(PaxTypeFare&, TravelRoute&, RoutingInfos&);
  bool process(ShoppingTrx&,
               FareMarket&,
               const uint32_t& bitIndex,
               PaxTypeFare& curFare,
               ShoppingRtgMap& rtMap);
  bool processShoppingNonSpecialRouting(FareMarket& fareMarket,
                                        PaxTypeFare& curFare,
                                        ShoppingRtgMap& rtMap);
  void processRoutingDiagnostic(const TravelRoute&, const RoutingInfos&, FareMarket&);

  bool isSpecialRouting(const PaxTypeFare& paxTypeFare) const;
  bool needSpecialRouting(const PaxTypeFare& paxTypeFare) const;
  void processSpecialRouting(fbrPaxTypeFareVector&, RoutingInfos&, const TravelRoute& tvlRoute);
  PricingTrx& _trx;
  static void
  groupRestriction12(PricingTrx& trx,
                     const RoutingRestriction* rest,
                     std::map<std::string, std::vector<const RoutingRestriction*>>& nationPairMap);
  static void createCityGroups(const std::vector<const RoutingRestriction*> restrictions12,
                               FlatSet<std::pair<Indicator, LocCode>>& cityGroup1,
                               FlatSet<std::pair<Indicator, LocCode>>& cityGroup2);

protected:
  static const RoutingComponent Base = 1;
  static const RoutingComponent OrigAddOn = 2;
  static const RoutingComponent DestAddOn = 3;
  static const RoutingComponent Whole = 4;

  bool collectSpecialRouting(FareMarket& fareMarket,
                             PaxTypeFare& paxTypeFare,
                             fbrPaxTypeFareVector& fbrFares);

  inline void processFare(PaxTypeFare& paxTypeFare,
                          FareMarket& fareMarket,
                          TravelRoute& tvlRoute,
                          RoutingInfos& routingInfos,
                          RtgKeyMap& rtMap,
                          bool genericRouting = false);
  void processFares(FareMarket& fareMarket,
                    TravelRoute& tvlRoute,
                    RoutingInfos& routingInfos,
                    fbrPaxTypeFareVector& fbrFares,
                    RtgKeyMap& rtMap,
                    bool genericRouting = false);
  void prepareFareForSpecRtgValid(PaxTypeFare& ptf);

  virtual void getRoutings(PricingTrx& trx,
                           PaxTypeFare& paxTypeFare,
                           const Routing*& routing,
                           const Routing*& origAddOnRouting,
                           const Routing*& destAddOnRouting);

  virtual bool processRoutingValidation(RtgKey& rKey,
                                        PricingTrx& trx,
                                        TravelRoute& tvlRoute,
                                        PaxTypeFare* paxTypeFare,
                                        const Routing* routing,
                                        const Routing* origAddOnRouting,
                                        const Routing* destAddOnRouting,
                                        RoutingInfos&,
                                        const DateTime& travelDate);

  virtual bool validateRoutingMaps(PricingTrx& trx,
                                   PaxTypeFare& paxTypeFare,
                                   TravelRoute& tvlRoute,
                                   MapInfo*,
                                   const Routing* routing,
                                   const DateTime& travelDate,
                                   const Routing* origAddOnRouting = nullptr,
                                   const Routing* destAddOnRouting = nullptr);

  virtual bool processRoutingCheckMissingData(RtgKey& rKey,
                                              RoutingInfo* routingInfo,
                                              const Routing* routing,
                                              const Routing* origAddOnRouting,
                                              const Routing* destAddOnRouting);

  virtual void processRoutingBreakTravel(RtgKey& rKey,
                                         PricingTrx& trx,
                                         TravelRoute& tvlRoute,
                                         RoutingInfo* routingInfo,
                                         TravelRoute& baseTvlRoute,
                                         TravelRoute& origAddOnTvlRoute,
                                         TravelRoute& destAddOnTvlRoute,
                                         const Routing* origAddOnRouting,
                                         const Routing* destAddOnRouting);

  virtual void processRoutingProcessRestrictions(RtgKey& rKey,
                                                 PricingTrx& trx,
                                                 TravelRoute& tvlRoute,
                                                 PaxTypeFare* paxTypeFare,
                                                 RoutingInfo* routingInfo,
                                                 TravelRoute& origAddOnTvlRoute,
                                                 TravelRoute& destAddOnTvlRoute,
                                                 const Routing* routing,
                                                 const Routing* origAddOnRouting,
                                                 const Routing* destAddOnRouting,
                                                 bool& isRoutingValid);

  virtual void processRoutingMileageRouting(RtgKey& rKey,
                                            PricingTrx& trx,
                                            TravelRoute& tvlRoute,
                                            PaxTypeFare* paxTypeFare,
                                            RoutingInfo* routingInfo,
                                            bool& isRoutingValid,
                                            bool& isMileageValid);

  virtual void processRoutingRoutMapValidation(RtgKey& rKey,
                                               PricingTrx& trx,
                                               TravelRoute& tvlRoute,
                                               PaxTypeFare* paxTypeFare,
                                               RoutingInfo* routingInfo,
                                               TravelRoute& baseTvlRoute,
                                               TravelRoute& origAddOnTvlRoute,
                                               TravelRoute& destAddOnTvlRoute,
                                               MapInfo*& mapInfo,
                                               MapInfo*& rtgAddonMapInfo,
                                               const Routing* routing,
                                               const Routing* origAddOnRouting,
                                               const Routing* destAddOnRouting,
                                               const DateTime& travelDate,
                                               bool& isRoutingValid);

  bool domesticRoutingValidationAllowed(PricingTrx& trx,
                                        TravelRoute& tvlRoute,
                                        const Routing* routing,
                                        MapInfo* mapInfo);

  void buildRtgKey(PaxTypeFare& paxTypeFare,
                   const FareMarket& fareMarket,
                   const Routing* routing,
                   const Routing*& origAddOnRouting,
                   const Routing*& destAddOnRouting,
                   RtgKey& rKey);

  bool buildAddOnTravelRoute(PricingTrx& trx,
                             TravelRoute& addOnTvlRoute,
                             TravelRoute& tvlRoute,
                             bool orig);

  bool buildBaseTravelRouteForRestrictions(TravelRoute& newTvlRoute,
                                           TravelRoute& tvlRoute,
                                           RtgKey& rKey);

  bool validateRestriction12(
      DataHandle& dataHandle,
      const TravelRoute& tvlRoute,
      std::map<std::string, std::vector<const RoutingRestriction*>>& nationPairMap,
      RoutingInfo* routingInfo);

  bool validateRestriction17(const TravelRoute& tvlRoute,
                             PricingTrx& trx,
                             RoutingInfo* routingInfo,
                             std::set<CarrierCode>&,
                             std::vector<const RoutingRestriction*>&);

  bool buildBaseTravelRoute(const Routing* origAddOnRouting,
                            const Routing* destAddOnRouting,
                            TravelRoute& newTvlRoute,
                            TravelRoute& tvlRoute,
                            RtgKey& rKey);

  bool buildTvlSegs(TravelRoute& tvlRoute,
                    TravelRoute& newTvlRoute,
                    std::vector<TravelSeg*>& newTvlSegs,
                    RtgKey& rKey);

  bool buildTravelSegs(TravelRoute& tvlRoute, const Loc* loc, std::vector<TravelSeg*>& newTvlSegs);

  bool buildTravelSegs(const Routing* origAddOnRouting,
                       const Routing* destAddOnRouting,
                       TravelRoute& tvlRoute,
                       TravelRoute& newTvlRoute,
                       std::vector<TravelSeg*>& newTvlSegs,
                       RtgKey& rKey);

  void updateFareSurcharge(PaxTypeFare&, const MileageInfo&);
  void updatePaxTypeFare(RtgKey&, PaxTypeFare&, RoutingInfos&);
  /**
   * Validates the restriction requirement for the FareMarket.
   * */

  bool processRestrictions(PaxTypeFare& paxTypeFare,
                           PricingTrx& trx,
                           const TravelRoute& tvlRoute,
                           RoutingInfo* routingInfo,
                           const Routing* routing,
                           const std::vector<RoutingRestriction*>& allRouteRestrictions);
  bool isRestSeqUseAndLogic(const std::vector<RoutingRestriction*>& allRouteRestrictions);
  bool isPositiveRestriction(const RoutingRestriction* restr);

  virtual bool validateRestriction(PricingTrx& trx,
                                   PaxTypeFare& paxTypeFare,
                                   const RoutingRestriction& rests,
                                   const TravelRoute& tvlRoute,
                                   RoutingInfo* routingInfo);

  bool processAddOnRestrictions(const Routing* routing,
                                PaxTypeFare& paxTypeFare,
                                PricingTrx& trx,
                                const TravelRoute& tvlRoute,
                                RoutingInfo* routingInfo,
                                RoutingComponent);

  static const NationCode& getDomesticNationGroup(const NationCode& nation);
  virtual bool validateMileage(PricingTrx& trx,
                               PaxTypeFare& paxtypeFare,
                               const TravelRoute& tvlRoute,
                               MileageInfo& mileageInfo);

  void
  updateCityCarrierVec(TravelRoute& tvlRoute, RoutingInfo& drvInfo, DRVCityCarrier& cityCarrierVec);

  void processSurchargeException(PricingTrx&,
                                 std::vector<PaxTypeFare*>&,
                                 RoutingInfos&,
                                 const TravelRoute& tvlRoute);

  bool mileageValidated(PaxTypeFare& paxTypeFare);
  bool validateMileageSurchargeException(PricingTrx& trx,
                                         PaxTypeFare& curFare,
                                         TravelRoute& tvlRoute,
                                         const RoutingInfo& rtgInfo);

  bool collectRoutingData(FareMarket& fareMarket,
                          PaxTypeFare& curFare,
                          const Routing*& routing,
                          const Routing*& origAddOnRouting,
                          const Routing*& destAddOnRouting,
                          TravelRoute& tvlRoute,
                          RtgKey& rtKey);
  bool collectRoutingDataNotFullMapRouting(FareMarket& fareMarket,
                                           PaxTypeFare& curFare,
                                           const Routing*& routing,
                                           const Routing*& origAddOnRouting,
                                           const Routing*& destAddOnRouting,
                                           TravelRoute& tvlRoute,
                                           RtgKey& rtKey);

  void processShoppingSpecialRouting(PaxTypeFare& curFare,
                                     const uint32_t& bitIndex,
                                     TravelRoute& tvlRoute,
                                     ShoppingRtgMap& rtMap);
  void processShoppingNonSpecialRouting(FareMarket& fareMarket,
                                        PaxTypeFare& curFare,
                                        TravelRoute& tvlRoute,
                                        const Routing* routing,
                                        const Routing* origAddOnRouting,
                                        const Routing* destAddOnRouting,
                                        RtgKey& rKey,
                                        ShoppingRtgMap& rtMap,
                                        RoutingValue& rtgValue);

  void updateFlightBitStatus(PaxTypeFare& curFare,
                             const uint32_t& bitIndex,
                             const RoutingValue& rtgValue);

  bool processShoppingRouting(FareMarket& fareMarket,
                              const uint32_t& bitIndex,
                              PaxTypeFare& curFare,
                              ShoppingRtgMap& rtMap);
  class RestrictionKey
  {
  public:
    RestrictionKey() : _checkFlip(false) {}

    RestrictionKey(const RestrictionNumber& no,
                   const LocCode& m1,
                   const LocCode& m2,
                   bool checkFlip)
      : _restriction(no), _market1(m1), _market2(m2), _checkFlip(checkFlip)
    {
    }

    const RestrictionNumber& restriction() const { return _restriction; }
    RestrictionNumber& restriction() { return _restriction; }

    const LocCode& market1() const { return _market1; }
    LocCode& market1() { return _market1; }

    const LocCode& market2() const { return _market2; }
    LocCode& market2() { return _market2; }

    bool operator<(const RestrictionKey& key) const;

  private:
    RestrictionNumber _restriction;
    LocCode _market1;
    LocCode _market2;
    bool _checkFlip;
  };

  using RestrictionValidity = std::map<RestrictionKey, bool>;

  /* Predicate for finding invalid restrictions by restriction key */
  struct RestrictionInvalid : public std::unary_function<RestrictionValidity::value_type, bool>
  {
    bool operator()(const RestrictionValidity::value_type& d) const { return !d.second; }
  };

  TravelRoute add(const TravelRoute&, const TravelRoute&, const Indicator& unticketedPointInd);
  void dummyFareMileage(PricingTrx& trx, PaxTypeFare& paxTypeFare, FareMarket& fm);
  void dummyFareMilePercentage(const ExchangePricingTrx& excTrx,
                               PaxTypeFare& paxTypeFare,
                               FareMarket& fm);
  void
  dummyFareMileCity(const ExchangePricingTrx& excTrx, PaxTypeFare& paxTypeFare, FareMarket& fm);

  virtual const Routing* getRoutingData(PricingTrx& trx, PaxTypeFare& paxTypeFare) const;

private:
  TvlRouteMap* _tvlRouteMap;
  boost::mutex* _tvlRouteMapMutex;

  RoutingKeyMap* _routingKeyMap;
  boost::mutex* _routingKeyMapMutex;

  FareMarketMap* _fareMarketMap;
  boost::mutex* _fareMarketMapMutex;

  void processPaxTypeFare(PaxTypeFare& paxTypeFare,
                          TravelRoute& tvlRoute,
                          FareMarket& fareMarket,
                          RoutingInfos& routingInfos,
                          RtgKeyMap& rtMap,
                          const Routing* routing,
                          const Routing* origAddOnRouting,
                          const Routing* destAddOnRouting);
};
}
