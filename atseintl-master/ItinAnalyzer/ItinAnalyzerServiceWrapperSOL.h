#pragma once

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "ItinAnalyzer/Diag922Data.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "ItinAnalyzer/SkipFareMarketByMileageSOL.h"
#include "Routing/RoutingConsts.h"

#include <map>
#include <vector>

namespace tse
{

class FareMarket;
class TravelSeg;
class Itin;
class ItinAnalyzerService;
class GovCxrGroupParameters;

class ItinAnalyzerServiceWrapperSOL
{
public:
  ItinAnalyzerServiceWrapperSOL(ShoppingTrx& trx, ItinAnalyzerService& itinAnalyzer);
  void doSumOfLocals();
private:
  typedef std::pair<const uint32_t, bool> SopInfo;
  typedef std::map<const Itin*, SopInfo> Itin2SopInfoMap;

  struct SolRestriction
  {
    bool skipByArea = false;
    const ConfigSet<CarrierCode>* unskippableCarriers = &empty;
    const ConfigSet<CarrierCode> empty;
  };

  bool filterGovCxrOverride(const Itin* itin);
  bool isPairValidForGovCxrOverride(const std::vector<TravelSeg*>& fm1ts,
                                    const std::vector<TravelSeg*>& fm2ts);
  bool isOverrideCxrExistInTrvSegs(const std::vector<TravelSeg*>& trvSegs);

  void getGovCxrAndAllFareMkt(Itin* itin,
                              SOLFareMarketMap& uniqueFareMarkets,
                              int legIdx,
                              SopInfo origSopInfo,
                              int idx,
                              GovCxrGroupParameters& params,
                              bool isFullyOpenAndOnline,
                              bool isSkipByAreaValidation);

  std::vector<FareMarket*> getGovCxrAndAllFareMktold(Itin* itin,
                                                     int startSeg,
                                                     const std::vector<TravelSeg*>& fmts,
                                                     SOLFareMarketMap& uniqueFareMarkets,
                                                     int legIdx,
                                                     int origSopId,
                                                     int idx,
                                                     GovCxrGroupParameters& params);

  /**
     * Execute those thru & local FM checks,
     * that that require fare market geo travel type has been already set up
     * (DTC check, mileage+DTC check)
     *
     * @return true if should keep
     */
  bool shouldKeepFareMarket(const FareMarket* fareMarket, const Itin* itin);

  /**
     * @return true if should skip
     */
  bool skipDomesticTransborderThroughFareMarket(const FareMarket* fareMarket, const Itin* itin);

  /**
     * Execute those local FM checks, that do not request fare market to be set up
     *
     * @param ctxPairTS1 fare market is also checked in travel segments pair context
     * @param ctxPairTS2
     * @param legIdx is "1" for IB leg, "2" for OB leg
     */
  bool shouldKeepLocalFareMarket(const std::vector<TravelSeg*>& fmToCheck,
                                 const std::vector<TravelSeg*>& ctxPairTS1,
                                 const std::vector<TravelSeg*>& ctxPairTS2,
                                 const Itin* itin,
                                 int legIdx,
                                 bool isFullyOpenAndOnline,
                                 bool isSkipByAreaValidation,
                                 const GovCxrGroupParameters& params);

  void initializeFareMarket(FareMarket*& fm,
                            Itin* itin,
                            SOLFareMarketMap& uniqueFareMarkets,
                            const std::vector<TravelSeg*>& fmts,
                            int startSeg,
                            int origSopId,
                            int bitIndex,
                            GovCxrGroupParameters& params);

  bool isOnlineThroughFareMarket(const Itin* itin) const;
  bool isFullyOpenThroughAvl(const Itin* itin) const;
  bool isFullyOpenAndOnline(ItinIndex& carrierIndex,
                            const CarrierCode govCxr,
                            const uint32_t carrierKey) const;
  void setUpSolRestriction(const Itin* itin, SolRestriction& context) const;
  void fillCxrFareMarkets(Itin& itin, ItinIndex::ItinRow& row) const;

  FareMarket* checkDualGoverningCarrier(FareMarket* fareMarket,
                                        const std::vector<TravelSeg*>& fmts,
                                        int legIdx,
                                        int origSopId,
                                        bool isLocal = false);

  FareMarket*
  checkDualGoverningCarrierForIS(FareMarket* fareMarket,
                                 const std::vector<TravelSeg*>& fmts,
                                 int legIdx,
                                 int origSopId);

  bool hasFares(const Itin& itin, const FareMarket& fareMarket);
  bool isForeignDomesticJourneyItin(const Itin* journeyItin) const;

  ShoppingTrx& _trx;
  ItinAnalyzerService& _itinAnalyzer;
  SkipFareMarketByMileageSOL _solTuning;
  CarrierCode _cxrOverride{BLANK};
  Diag922Data _diag922Data;
  bool _isSkipForeignDomestic = false;
  bool _foreignDomesticSkipConfigured;
  bool _area12SOLSkipConfigured;
  bool _area13SOLSkipConfigured;
  bool _nationUSCAArea1SOLSkipConfigured;
  bool _nationUSCAChinaSOLSkipConfigured;
  bool _subarea21Area3SOLSkipConfigured;
  bool _subarea21SubArea22Or23SOLSkipConfigured;
  bool _subarea21InternationalSOLSkipConfigured;
  bool _isThisCarrierUnskippable = false;
};
} // tse
