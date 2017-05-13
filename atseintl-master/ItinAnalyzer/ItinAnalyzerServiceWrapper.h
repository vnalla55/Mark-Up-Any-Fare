//----------------------------------------------------------------------------
//  Copyright Sabre 2009
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

#include <unordered_set>
#include <utility>
#include <vector>


namespace tse
{
class DataHandle;
class ExchangePricingTrx;
class FareMarket;
class Itin;
class ItinAnalyzerService;
class PricingTrx;
class TravelSeg;

class ItinAnalyzerServiceWrapper
{
  friend class ItinAnalyzerServiceTest;
  friend class ItinAnalyzerServiceWrapperTest;
  friend class ItinAnalyzerService;

  template <typename TrxType, typename ItinAnalyzerServiceWrapperType>
  friend void itinanalyzerutils::initializeAndStoreFareMarkets(
      FareMarketsWithCodes& fareMarketsWithCodes,
      ItinAnalyzerServiceWrapperType* baseItinAnalyzerWrapper,
      TrxType& trx);

protected:
  typedef std::vector<SegmentAttributes> SegmentAttribs;
  typedef std::vector<std::vector<TravelSeg*> > SideTrips;
  typedef std::vector<std::vector<TravelSeg*>*> PendingSideTrips;
  typedef std::vector<bool> PendingSideTripsForced;
  typedef std::pair<const uint32_t, bool> SopInfo;
  typedef std::map<const Itin*, SopInfo> Itin2SopInfoMap;


  struct SideTripsInfo
  {
    SideTripsInfo() : _fmStForced(false) {}

    PendingSideTrips _pendingSideTrips;
    PendingSideTripsForced _pendingSideTripsForced;
    SideTrips _resultSideTrips;
    bool _fmStForced;
  };

  ItinAnalyzerService& _itinAnalyzer;
  std::vector<Itin2SopInfoMap> _itin2SopInfoMapVec;

public:
  ItinAnalyzerServiceWrapper(ItinAnalyzerService& itinAnalyzer);
  virtual ~ItinAnalyzerServiceWrapper() {}

protected:
  virtual void initialize(PricingTrx& trx);
  virtual void processExchangeItin(ExchangePricingTrx& trx);
  virtual void determineChanges(ExchangePricingTrx& trx);
  virtual bool selectTicketingCarrier(PricingTrx& trx);
  virtual void setRetransits(PricingTrx& trx);
  virtual void setOpenSegFlag(PricingTrx& trx);
  virtual void setTripCharacteristics(PricingTrx& trx);
  virtual void checkJourneyActivation(PricingTrx& trx);
  virtual void checkSoloActivation(PricingTrx& trx);
  virtual void setATAEContent(PricingTrx& trx);
  virtual void setATAEAvailContent(PricingTrx& trx);
  virtual void setATAESchedContent(PricingTrx& trx);
  virtual void setItinRounding(PricingTrx& trx);
  virtual void setInfoForSimilarItins(PricingTrx& trx);
  virtual bool buildFareMarket(PricingTrx& trx, Itin& itin);
  virtual bool setupAndStoreFareMarket(PricingTrx& trx, Itin& itin, FareMarket* newFareMarket,
                                       uint32_t legIndex=0xFFFFFFFF) const;
  bool simplifiedBuildFareMarket(PricingTrx& trx, Itin& itin);

private:
  void createFareMarketForRtw(PricingTrx& trx, Itin& itin) const;
  bool collectSegmentsForRtw(PricingTrx& trx, Itin& itin, std::vector<TravelSeg*>& travelSegs) const;
  bool setupAndStoreFareMarketForRtw(PricingTrx& trx, Itin& itin, FareMarket& newFm) const;

  void getGovCxrOverrides(PricingTrx& trx,
                          const FareMarket& newFm,
                          std::vector<CarrierCode>& govCxrOverrides,
                          std::set<CarrierCode>& participatingCxrs) const;

  void createFareMarket(PricingTrx& trx,
                        Itin& itin,
                        SegmentAttribs::iterator endSegIt,
                        std::vector<TravelSeg*>& travelSegs,
                        bool breakIndicator,
                        SideTripsInfo* const sideTripsInfo) const;

  bool collectSegments(PricingTrx& trx,
                       Itin& itin,
                       const SegmentAttributes& segAttr,
                       std::vector<TravelSeg*>& travelSegs,
                       SideTripsInfo& sideTripsInfo) const;

  bool skipForShoppingIS(PricingTrx& trx, const FareMarket* fm) const;

  void selectGovCxrs(PricingTrx& trx,
                     Itin& itin,
                     FareMarket* newFareMarket,
                     bool& checkIfMarketAlreadyAdded,
                     bool& fareMarketAdded,
                     bool& fmBreakSet,
                     const std::vector<CarrierCode>& govCxrOverrides,
                     const std::set<CarrierCode>& participatingCarriers,
                     uint32_t legIndex) const;

  void selectGovCxrsOld(PricingTrx& trx,
                        Itin& itin,
                        FareMarket* newFareMarket,
                        bool& checkIfMarketAlreadyAdded,
                        bool& fareMarketAdded,
                        bool& fmBreakSet,
                        const std::vector<CarrierCode>& govCxrOverrides,
                        const std::set<CarrierCode>& participatingCarriers) const;

  void cloneAndStoreFareMarket(PricingTrx& trx,
                               FareMarket* const srcFareMarket,
                               Itin& itin,
                               TravelSeg* const primarySector,
                               CarrierCode& govCxr,
                               const std::vector<CarrierCode>& govCxrOverrides,
                               const std::set<CarrierCode>& participatingCarriers,
                               bool yyOverride,
                               bool fmBreakSet,
                               bool removeOutboundFares,
                               bool highTPMGovCxr=false) const;

  void addFareMarketDiffGovCXR(PricingTrx& trx, Itin& itin, FareMarket& fareMarket) const;

  // Add newFareMarket to itin and to trx
  // If checkIfExist is given, check if this fare market already
  // exists in trx. If fare market already exists:
  // a) dont add it to trx (to itin only)
  // b) if the same fare market exists in a different itin
  //    with different "thru fare only" flag, mark it as
  //    "keep solo"
  FareMarket*
  storeFareMarket(PricingTrx& trx, FareMarket* newFareMarket, Itin& itin, bool checkIfExist) const;

  void getSideTripEnd(Itin& itin,
                      const SegmentAttributes& attribs,
                      SideTripsInfo* const sideTripsInfo,
                      std::vector<TravelSeg*>& travelSegs) const;

  void getSideTripBegin(DataHandle& dataHandle,
                        const SegmentAttributes& attribs,
                        SideTripsInfo* const sideTripsInfo) const;

  void
  getSideTripElement(const SegmentAttributes& attribs, SideTripsInfo* const sideTripsInfo) const;

  void createARUNKFareMarket(PricingTrx& trx,
                             Itin& itin,
                             SegmentAttribs::iterator beginSegIt,
                             SegmentAttribs::iterator endSegIt) const;

  void addStToFareMarket(const Itin& itin,
                         FareMarket& newFareMarket,
                         std::vector<std::vector<TravelSeg*> >& sideTrips,
                         std::set<int16_t>& stSegOrders) const;

  void addPendingStToFareMarket(Itin& itin,
                                FareMarket& newFareMarket,
                                std::vector<std::vector<TravelSeg*>*>& pendingSideTrips,
                                std::set<int16_t>& stSegOrders) const;

  void createSideTripCombination(PricingTrx& trx,
                                 Itin& itin,
                                 FareMarket*& fareMarket,
                                 SideTrips::iterator& currentSideTripIt) const;

  void createThruFareMarket(PricingTrx& trx, Itin& itin, const FareMarket* const fareMarket) const;

  bool isJointUSCAFareMarket(const FareMarket* const fareMarket) const;

  bool setBoardAndOffPoints(PricingTrx& trx, const Itin& itin, FareMarket& fareMarket) const;

  // Checks if fareMarket exists in itin
  // Fare markets are compared using rules arbitrary for this method
  // If found, return a pointer to an 'identical' fare market from itin
  // Otherwise, return NULL
  FareMarket*
  getExistingFareMarket(const FareMarket& fareMarket, Itin* const itin, PricingTrx& trx) const;

  // Checks if fareMarket exists in trx
  // Fare markets are compared using rules arbitrary for this method
  // If found, return a pointer to an 'identical' fare market from trx
  // Otherwise, return NULL
  FareMarket* getExistingFareMarket(const FareMarket& fareMarket,
                                    PricingTrx& trx,
                                    bool checkExistingForFareSelection = false) const;

  bool isVctrCarrier(const FareMarket* fareMarket) const;

  static bool isValidSideTripCombination(Itin& itin,
                                         std::vector<std::vector<TravelSeg*> >& sideTrips,
                                         std::vector<TravelSeg*>& st);

  static void copyFromInvalidSideTrip(std::vector<std::vector<TravelSeg*> >& sideTrips,
                                      std::vector<TravelSeg*>& newSt,
                                      std::vector<TravelSeg*>& travelSegs);

  bool isAcrossStopoverDomesticUSFareMarket(const FareMarket& fareMarket) const;

  void parseAcrossStopoverExcludedCarriersCities(const std::string& acrossStopoverEnabledCarriers);

  bool isFMNotAllowed(const std::vector<SegmentAttributes>& segAttributes,
                      std::vector<SegmentAttributes>::const_iterator begin,
                      std::vector<SegmentAttributes>::const_iterator end) const;

  std::map<std::string, std::set<std::string> > _acrossStopoverEnabledCarriersCities;

}; // End class ItinAnalyzerServiceWrapper

} // End namespace tse

