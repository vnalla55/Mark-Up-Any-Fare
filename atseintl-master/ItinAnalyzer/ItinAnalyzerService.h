//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TravelSegAnalysis.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "ItinAnalyzer/ExcItinAnalyzerServiceWrapper.h"
#include "ItinAnalyzer/ItinAnalyzerServiceWrapper.h"
#include "Service/Service.h"

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace tse
{
class AirSeg;
class AltPricingDetailObFeesTrx;
class AltPricingTrx;
class AncillaryPricingTrx;
class BaggageTrx;
class BaseExchangeTrx;
class BrandingTrx;
class CarrierPreference;
class ContentServices;
class Diag185Collector;
class Diag892Collector;
class DiagCollector;
class ExchangePricingTrx;
class ExcItin;
class FareCompInfo;
class FareMarket;
class FlightFinderTrx;
class InterlineTicketCarrier;
class Itin;
class ItinAnalyzerService;
class ItinIndex;
class Logger;
class MetricsTrx;
class NoPNRPricingTrx;
class PricingDetailTrx;
class PricingTrx;
class RefundPricingTrx;
class RepricingTrx;
class RexBaseTrx;
class RexExchangeTrx;
class RexPricingTrx;
class RexShoppingTrx;
class StructuredRuleTrx;
class TaxTrx;
class TktFeesPricingTrx;
class TravelSeg;
class TseServer;

class ItinAnalyzerService : public Service
{
  friend class ItinAnalyzerServiceTest;
  friend class ItinAnalyzerServiceMock;
  friend class BffgenerateDummySOPTest;
  friend class ItinAnalyzerServiceWrapper;
  friend class ReissueRefundItinAnalyzer;
  friend class ItinAnalyzerServiceWrapperSOL;

  typedef std::vector<LocCode> Airports;

private:
  static Logger _logger;

  // Shopping constants
  const static uint8_t MIN_NUMBER_LEGS_FOR_STOPOVER = 2;
  const static uint32_t ITININDEX_CELL_FAKE_SOP_ID = 9999;

  // Cost for domestic travel that is USCA
  const static uint32_t INTL_TRAVCOST_USCA = 1;
  // Cost for domestic travel that is not USCA
  const static uint32_t INTL_TRAVCOST_NONUSCA = 1;
  // Cost for travel crossing one major IATA
  const static uint32_t INTL_TRAVCOST_ONEIATA = 4;
  // Cost for travel crossing two major IATAs
  const static uint32_t INTL_TRAVCOST_TWOIATA = 8;
  // Cost for travel crossing all three major IATAs
  const static uint32_t INTL_TRAVCOST_ALLIATA = 16;
  // Cost for travel within sub IATA 11
  const static uint32_t INTL_TRAVCOST_SUBIATA11 = 2;
  // Cost for travel within sub IATA 21
  const static uint32_t INTL_TRAVCOST_SUBIATA21 = 2;
  // Cost for travel within a sub IATA that is not 11 or 21
  const static uint32_t INTL_TRAVCOST_SUBIATA = 1;

  // Cost for travel that crosses any IATA
  const static uint32_t INTL_DETAILTRAVCOST_IATA = 8;
  // Cost for travel that crosses any sub IATA
  const static uint32_t INTL_DETAILTRAVCOST_SUBIATA = 4;
  // Cost for travel that crosses a national boundary
  const static uint32_t INTL_DETAILTRAVCOST_NATION = 1;

  const static uint16_t MAX_SIDETRIP_IN_FAREMARKET = 2;

  const static uint16_t DELETED_ITIN_INDEX = 55555;

  static ContentServices _contentSvcs;
  static bool _enableASOLegs;

  int _useASOThreshold = 0;

  TravelSegAnalysis _tvlSegAnalysis;
  bool _thruFMOnly = false;
  bool _splitItinsForDomesticOnly = false;
  uint16_t _requestedNumberOfSeats = 0;

  // Placed here so they wont be called

  ItinAnalyzerService(const ItinAnalyzerService& rhs);
  ItinAnalyzerService& operator=(const ItinAnalyzerService& rhs);

  void checkIfPenaltiesAndFCsEqualToSumFromFareCalc(RefundPricingTrx& trx) const;

  bool buildFareMarket(PricingTrx& trx, bool checkLimitations = true);
  std::string printNoPreferredClass(CabinType& preferredCabinClass, ShoppingTrx::Leg& curLeg) const;
  virtual void buildFareMarket(RexBaseTrx& trx, ExcItin& itin);
  void completeExcRtwItin(RexBaseTrx& trx, ExcItin& itin, FareMarket& fareMarket);
  void addRegularFareMarketsForPlusUpCalculation(RexBaseTrx& trx, ExcItin& itin);
  void removeDuplicatedFMs(std::vector<FareMarket*>& fareMarkets, size_t regularPart);

  void cloneFareMarket(FareMarket& fareMarket, FareMarket& anotherFareMarket, CarrierCode govCxr);

  void buildInboundFareMarket(const FareMarket& fareMarket, FareMarket& inbFareMarket);

  FareMarket* getExistingFareMarket(ShoppingTrx& trx,
                                    FareMarket& fareMarket,
                                    std::vector<FareMarket*>& fareMarkets);

  bool validateRestrictedCurrencyNation(PricingTrx& trx);

  virtual void setValidatingCarrier(PricingTrx& trx);
  virtual void setValidatingCarrier(TaxTrx& trx);
  virtual void setFlightFinderValidatingCarrier(PricingTrx& trx);
  virtual void setGeoTravelTypeAndValidatingCarrier(PricingTrx& trx);
  void fillSimilarItinFMCOS(PricingTrx& trx);
  virtual void removeUnwantedFareMarkets(PricingTrx& trx);
  virtual void processFMsForValidatingCarriers(PricingTrx& trx);

  virtual void processItinFMsForValidatingCarriers(
      PricingTrx& trx,
      Itin& motherItin,
      std::map<FareMarket*, std::set<CarrierCode>>& fmValidatingCxrMap);

  virtual void setItinCurrencies(PricingTrx& trx);
  virtual void setItinCurrencies(RexBaseTrx& trx);
  virtual void setItinCurrencies(RexShoppingTrx& trx);

  virtual void setAgentCommissions(PricingTrx& trx);

  // Adds an itin and its associated info into the itin index associative structure
  bool
  addScheduleGroupMapEntry(ItinIndex& curIndex, Itin*& itin, ItinIndex::ItinCellInfo& itinCellInfo);

  // Groups the schedules by governing carrier and connection point at the leg level
  bool groupSchedulesByGovCxrAndConxnPoint(ShoppingTrx::Leg& curLeg, ShoppingTrx& trx);

  // Main function call for grouping the schedules in the itin index structure
  bool groupSchedules(ShoppingTrx& trx);

  void validateAllItinsPreferredCabinTypes(std::vector<Itin*>& itinVec,
                                           PricingTrx& trx,
                                           DiagCollector* collector);
  void
  validateSimilarItinsPreferredCabinTypes(Itin& itin, PricingTrx& trx, DiagCollector* collector);
  bool validateSingleItinPreferredCabinTypes(Itin* itin, PricingTrx& trx, DiagCollector* collector);
  bool
  isJumpedDownCabinOffered(const CabinType& preferredCabinForLeg, const CabinType& travelSegCabin);
  int getJumpedDownDistance(const CabinType& preferredCabin, const CabinType& jumpedDownCabin);

  // Clears seats for cabins above preferred
  void removeHigherCabins(PricingTrx& trx);
  void removeHigherCabins(PricingTrx& trx,
                          std::vector<ClassOfService*>& classOfService,
                          TravelSeg* travelSeg);
  void removeHigherCabins(std::vector<ClassOfService*>& classOfService, CabinType preferredCabin);

  void setAirSegsWithRequestedCabin(PricingTrx& trx);
  virtual bool selectGcmFareMarkets(PricingTrx& trx,
                                    std::vector<FareMarket*>& gcmFareMarkets,
                                    Diag185Collector* diag185 = 0);
  void selectGcmFareMarkets(PricingTrx& trx, std::vector<FareMarket*>& gcmFareMarkets,
                            std::vector<TravelSeg*>& tempTvlSegs, Diag185Collector* diag185 = 0);

  virtual int16_t getGcm(std::vector<TravelSeg*>& itinTvlSegs, Diag185Collector* diag185);

  virtual bool isDomesticFM(const PricingTrx& trx, const FareMarket& fm);

  virtual bool isInternationalItin(const PricingTrx& trx);

  virtual bool isItinWhollyWithInSameNation(const Itin& itin);

  bool atLeastOneSegmentInRequestedCabin(const Itin& itin, CabinType reqCabin);

  ClassOfService * findOfferedAndAvailable(PricingTrx& trx,
                                           std::vector<ClassOfService*>& classOfService,
                                           CabinType reqCabin,
                                           CabinType bookedCabin,
                                           bool allCabin) const;

  bool isAllFareMarketsValid(const PricingTrx& trx,
                             std::vector<FareMarket*>&gcmFareMarkets,
                             CabinType reqCabin,
                             Diag185Collector* diag185);
  bool isCurrentFmSameAsPrevFM(const FareMarket* currentFm, const FareMarket* prevFM);

  bool isItinOutsideEurope(const Itin& itin);

  bool isItinOutsideNetherlandAntilles(const Itin& itin);

  // functions and structures for SOP scoring used for 100+ options requests
  class SOPScoreComparator
  {
  private:
    double _point;
    int _numberOfClassesToAdd;
    int _requestedNumberOfSeats;
    std::map<const Itin*, double>* _sopScoreMap;
    const ShoppingTrx::Leg& _curLeg;
    const double _halfSops;
    const double _qualityCoeff;

  public:
    SOPScoreComparator(double point,
                       int noToAdd,
                       int reqNoOfSeats,
                       std::map<const Itin*, double>* sopScoreMap,
                       const ShoppingTrx::Leg& curLeg,
                       double qualityCoeff);

    bool
    operator()(const ItinIndex::ItinCell& firstCell, const ItinIndex::ItinCell& secondCell) const;
  };

  void sortItinsBasedOnSOPScore(ItinIndex& curCarrierIndex, SOPScoreComparator& comparator);

  void removeDeletedItins(std::vector<Itin*>& itins);
  void validateTicketingAgreement(PricingTrx& trx);

  bool validateFlightCabin(ShoppingTrx& trx);
  bool validateFlightCabinESV(ShoppingTrx& trx);

  void validateSOPPremiumFlightCabin(ShoppingTrx& trx,
                                     CabinType preferredCabin,
                                     int totalSeat,
                                     bool lastLeg,
                                     ShoppingTrx::SchedulingOption& sop,
                                     std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                     std::vector<ShoppingTrx::SchedulingOption*>& sopMixedVec,
                                     bool& jumpDownAllowed,
                                     std::map<CabinType, bool>& cabinsOffered,
                                     std::map<CabinType, bool>& cabinsAvailable);

  void validateSOPPremiumTravSegVector(ShoppingTrx& trx,
                                       ShoppingTrx::SchedulingOption& sop,
                                       CabinType preferredCabin,
                                       int totalSeat,
                                       bool lastLeg,
                                       bool& requestedCabinSopFound,
                                       bool& nonRequestedCabinSopFound,
                                       std::map<CabinType, bool>& cabinsOffered,
                                       std::map<CabinType, bool>& cabinsAvailable);

  void validateSOPPremiumCOSVector(ShoppingTrx& trx,
                                   int totalSeat,
                                   AirSeg* curAirSeg,
                                   std::vector<ClassOfService*>& classOfService,
                                   std::map<CabinType, bool>& cabinsOffered,
                                   std::map<CabinType, bool>& cabinsAvailable,
                                   bool& requestedCabinFound,
                                   CabinType preferredCabin);

  void setupDefaultBookingCodeAndCabin(ShoppingTrx::SchedulingOption& sop,
                                       int totalSeat,
                                       AirSeg* curAirSeg,
                                       std::vector<ClassOfService*>& classOfService);

  void validateJumpDown(ShoppingTrx& trx,
                        CabinType preferredCabinClass,
                        std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                        std::map<CabinType, bool>& legCabinOffered,
                        std::map<CabinType, bool>& legCabinAvailable,
                        ShoppingTrx::Leg& curLeg);

  CabinType getSopCabin(ShoppingTrx::SchedulingOption& sop);

  // Shopping customized logic for determining governing carrier and
  // for generating the fare market structures from the DataHandle
  bool getGovCxrAndFareMkt(ShoppingTrx& trx);

  // Convenience method for creating a vector of direct itineraries
  // for the specified leg
  void buildLegDirectItins(ShoppingTrx& trx, const uint32_t& legId, std::vector<Itin*>& dItins);

  // Primary governing carrier and fare market structure generation method
  void getGovCxrAndFareMkt(PricingTrx& trx,
                           Itin& itin,
                           bool isPricingOp,
                           bool isCustomSop = false,
                           uint32_t legIndex = 0);

  virtual void setSortTaxByOrigCity(PricingTrx& trx);

  // Primary method used to generate stop over legs
  void generateStopOverLegs(ShoppingTrx& trx);

  // Generates the initial set of combinations for stop over legs
  void generateStopOverLegCombinationPairs(const ShoppingTrx& trx,
                                           IndexPairVector& combinations,
                                           const bool& isRoundTrip);

  // Prunes combinations of stop overs that are invalid
  void pruneStopOverLegCombinationPairs(const ShoppingTrx& trx,
                                        IndexPairVector& combinations,
                                        const bool& isRoundTrip);

  // Prepare the invalid across stop over leg pruning list
  void preparePruningRemovalList(const ShoppingTrx& trx,
                                 IndexPairVector& combinations,
                                 IndexPairVector& removeList);

  // Generates the stop over leges from the combinations of leg indices
  // provided
  void
  generateStopOverLegsFromCombinationPairs(ShoppingTrx& trx, const IndexPairVector& combinations);

  // Estimates the governing carrier for the stop over legs
  // generated
  void estimateGovCxrForStopOverLegs(ShoppingTrx& trx);

  // Gather travel boundary maps needed for governing carrier estimation
  void genTravelBoundMapForGovCxrEst(ShoppingTrx& trx,
                                     const IndexVector& jumpedLegIndices,
                                     std::map<uint32_t, FareMarket*>& travelBoundaryMap);

  // Calculate travel boundary summations for effcient determination
  // of carrier providing highest level of intl service
  void genTravelBoundSumsForGovCxrEst(ShoppingTrx& trx,
                                      const std::map<uint32_t, FareMarket*>& travelBoundaryMap,
                                      IndexVector& travelBoundSummations);

  // Determine if a match/tie was found in the summation calculation
  // and generate the matches
  void genTravelBoundMaxCountsForGovCxrEst(ShoppingTrx& trx,
                                           const IndexVector& travelBoundSummations,
                                           IndexPair& maxCountIndex,
                                           IndexVector& repeatedMaxCountIndices);

  // Break the tie in the summations by using more detailed
  // checks for highest internation service provider
  void estimateGovCxrDetailedCheck(ShoppingTrx& trx,
                                   const std::map<uint32_t, FareMarket*>& travelBoundaryMap,
                                   const IndexPair& maxCountIndex,
                                   const IndexVector& repeatedMaxCountIndices,
                                   uint32_t& govCxrIdxChosen);

  void addAcrossStopOverLegArunkSpans(ShoppingTrx& trx);
  void generateAcrossStopOverLegCxrMaps(ShoppingTrx& trx);

  // Copies class of service data from src to dest
  void copyClassOfServiceData(ShoppingTrx& trx, const AirSeg& src, AirSeg& dest);

  // Copies necessary data in air segment generation
  AirSeg* copyDefaultAirSegData(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest);

  // Generates an airsegment that encompasses the starting
  // point of the orig AirSeg and the ending point of the
  // dest AirSeg
  AirSeg* generateNewAirSegMin(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest);

  // Generates an airsegment that encompasses the starting
  // point of the orig AirSeg and the ending point of the
  // dest AirSeg
  AirSeg* generateNewAirSeg(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest);

  // Generates an airsegment that encompasses a surface sector
  // between the orig and dest, it spans the gap
  AirSeg* generateNewAirSegArunkSpan(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest);

  AirSeg* generateNewAirSeg(ShoppingTrx& trx,
                            const AirSeg& orig,
                            const AirSeg& dest,
                            const uint32_t& selectIdxOne,
                            const uint32_t& selectIdxTwo);

  // Generates an airsegment that encompasses the starting
  // point of the orig AirSeg and the ending point of the
  // dest AirSeg
  AirSeg* generateNewAirSeg(ShoppingTrx& trx,
                            const AirSeg& orig,
                            const AirSeg& dest,
                            const CarrierCode& cxr,
                            const FlightNumber& flightNum,
                            const CarrierCode& opCxr,
                            const FlightNumber& opFlightNum);

  // Generates a new itinerary based on an air segment
  // and a leg
  Itin* generateNewItinInLeg(ShoppingTrx& trx, ShoppingTrx::Leg& leg, AirSeg*& airSeg);

  // Generates a new itinerary based on air segments and
  // a leg
  Itin*
  generateNewItinInLeg(ShoppingTrx& trx, ShoppingTrx::Leg& leg, std::vector<AirSeg*>& airSegs);

  // Generates a new itinerary based on air segments
  Itin* generateNewItin(ShoppingTrx& trx, std::vector<AirSeg*>& airSegs, FareMarket* newFM);

  // Ensures that every governing carrier group per leg has
  // a direct itin to submit for fare collection
  bool ensureGroupHasDirectFlight(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg);

  void addOtherGoverningCarriersToDirectFlight(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg);
  void getOtherGovCxrFMs(ShoppingTrx& trx,
                         ItinIndex::ItinRow& itinRow,
                         ItinIndex::ItinCell* directItinCell);

  // Generate a "journey" itinerary that covers the journey
  // defined by all of the legs in the transaction, minus
  // the across stopover legs
  bool generateJourneyItin(ShoppingTrx& trx);

  // Log all leg data to the logger
  void logLegData(const ShoppingTrx& trx);

  // Log a specific leg to the logger
  void logLegData(const ShoppingTrx& trx, uint32_t& id, const ShoppingTrx::Leg& leg);

  // Log a stop over combination index to the logger
  void logStopOverIndex(const ShoppingTrx& trx, const IndexPairVector& index);

  // Log commonly output segment data to the logger
  void logSegmentData(const AirSeg& aSeg);

  // Classify legs based on their surface sector situation
  void classifyLegSurfaceSectorTypes(ShoppingTrx& trx);

  // Looks for existing legs in the original leg set for
  // removal from the potential set of across stop over
  // legs
  void addExistingLegCombinationsToRemovalList(const std::vector<ShoppingTrx::Leg>& legs,
                                               const LocCode& itinOneOrigin,
                                               const LocCode& itinTwoDestin,
                                               IndexPair& curPair,
                                               IndexPairVector& removeList);

  // Generates a standard across stop over leg
  void generateAcrossStopOverLeg(ShoppingTrx& trx, const IndexPair& curPair);

  // Generates an across stop over leg that deals with a surface sector
  void generateAcrossStopOverLegSurface(ShoppingTrx& trx, const IndexPair& curPair);

  // Checks if the transaction contains a simple one-way or round-trip
  void checkSimpleTrip(PricingTrx& trx);

  std::string getTravelSegOrderStr(const Itin& itin,
                                   const std::vector<TravelSeg*>& travelSegs,
                                   const FareMarket* fm = nullptr);
  std::string
  getTravelSegOrderStr(const Itin& itin, const std::vector<std::vector<TravelSeg*>>& sideTrips);

  virtual void checkJourneyActivation(PricingTrx& trx);

  virtual void checkSoloActivation(PricingTrx& trx);

  virtual void prepareForJourney(PricingTrx& trx);

  void setShoppingFareMarketInfo(ShoppingTrx& trx);

  void generateAltDatesMap(ShoppingTrx& trx) const;
  void checkValidateAltDays(ShoppingTrx& trx) const;

  bool checkCallDCAPerItn(std::vector<Itin*>& itins);

  virtual void setATAEContent(PricingTrx& trx);
  virtual void setATAEAvailContent(PricingTrx& trx);
  virtual void setATAESchedContent(PricingTrx& trx);

  void setATAEContent(AncillaryPricingTrx& trx);

  virtual void checkFirstDatedSegBeforeDT(PricingTrx& trx);
  virtual void setTrxDataHandleTicketDate(PricingTrx& trx, const DateTime& date);
  virtual void checkRestrictedCurrencyNation(PricingTrx& trx);
  virtual const Loc* getTicketLoc(PricingTrx& trx);
  virtual const Loc* getSalesLoc(PricingTrx& trx);
  virtual void setFurthestPoint(PricingTrx& trx, Itin* itin);

  void copyExcFareCompInfoToNewItinFm(ExchangePricingTrx& excTrx);
  std::vector<TravelSeg*>::const_iterator
  fcInfoMatch(ExchangePricingTrx& excTrx, const FareCompInfo& fcInfo);
  bool segMatch(const TravelSeg* tvlSegNew, const TravelSeg* tvlSegExc);
  void removeOldSurOverrides(ExchangePricingTrx& excTrx, const FareCompInfo& fcInfo);
  void removeNewSurOverrides(ExchangePricingTrx& excTrx, const TravelSeg* tvlSeg);
  void copyExcStopoverOverride(ExchangePricingTrx& excTrx,
                               const FareCompInfo& fcInfo,
                               const TravelSeg& tvlSegExc,
                               TravelSeg* tvlSegNew);

  void exchangePricingTrxException(ExchangePricingTrx& trx);
  void processExchangeItin(ExchangePricingTrx& trx);

  // Build local markets for shopping trx
  void buildLocalMarkets(ShoppingTrx& trx);

  // Build local markets for itinerary
  void buildMarketsForItinerary(ShoppingTrx& trx, Itin* curItin, Itin* dummyItin, uint32_t legId);

  CarrierCode getFareMarketCarrier(ShoppingTrx& trx,
                                   std::vector<TravelSeg*>::iterator segIterOrig,
                                   std::vector<TravelSeg*>::iterator segIterDest);

  // Add fare market with checking if it already exist
  void addFareMarket(ShoppingTrx& trx,
                     FareMarket* fareMarket,
                     std::vector<FareMarket*>& trxFareMarkets,
                     std::vector<FareMarket*>& carrierFareMarkets,
                     std::vector<FareMarket*>& itineraryFareMarkets);

  void findCommenceDate(ExcItin& itin);

  void validateSpecialProcessingStatus(ShoppingTrx& trx, const ShoppingTrx::Leg& curLeg);
  void setIndicesToSchedulingOption(PricingTrx& trx);

  void checkNoMatchItin(NoPNRPricingTrx& trx);

  void recalculateFareCalcFareAmt(RexBaseTrx& trx);

  void generateDummySOP(FlightFinderTrx& trx);

  ShoppingTrx::Leg* addDummyLeg(FlightFinderTrx& trx, bool addInFront);

  void addDummySop(FlightFinderTrx& trx, ShoppingTrx::Leg* curLeg, AirSeg* journeySeg);

  void addDummySop(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg, const AirSeg& journeySeg);

  bool processESV(ShoppingTrx& trx);

  void addScheduleGroupMapEntryESV(ShoppingTrx::Leg& curLeg,
                                   ShoppingTrx::SchedulingOption& curSop,
                                   ItinIndex::ItinCellInfo& itinCellInfo);

  bool groupSchedulesESV(ShoppingTrx& trx);

  bool createDummyItinForEveryGovCxr(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg);

  void determineChanges(ExchangePricingTrx& trx);
  void applyReissueExchange(RexPricingTrx& trx);
  void determineExchangeReissueStatus(BaseExchangeTrx& trx);
  bool isSegmentChanged(const TravelSeg* seg) const;
  bool setCalendarOutboundCurrencyBFRT(PricingTrx& trx) const;

  virtual const CarrierPreference*
  getCarrierPref(PricingTrx& trx, const CarrierCode& carrier, const DateTime& date);

  void discoverThroughFarePrecedence(const Trx& trx, Itin& itin);
  void failIfThroughFarePrecedenceImpossible(Itin& itin);

  void validateInterlineTicketCarrierESV(ShoppingTrx& trx);

  void collectMultiAirport(ShoppingTrx& trx);
  void collectMultiAirportForFareMarket(ShoppingTrx& trx,
                                        unsigned legIndex,
                                        const ItinIndex::Key& carrierKey,
                                        const FareMarket& fareMarket,
                                        Airports& originAirports,
                                        Airports& destinationAirports);
  void printMultiAirport(ShoppingTrx& trx);

  bool processIbfBrandRetrievalAndParity(PricingTrx& trx);
  bool processBfaBrandRetrieval(PricingTrx& trx);
  void removeFMsWithErrors(PricingTrx& trx, Diag892Collector* diag892);
  void removeFareMarketsFromTrxAndItins(PricingTrx& trx, std::set<FareMarket*>& FmsToBeRemoved);

  template <typename D>
  D* createDiag(PricingTrx& trx);

  template <typename D>
  void flushDiag(D* diag);

  Diag185Collector* createDiag185(PricingTrx& trx);

  void initializeTNBrandsData(PricingTrx& trx);
  void setThruFareMarketsForBfa(PricingTrx& trx);

  void processOldBrandedFareSecondaryRBD(PricingTrx& trx);
  void updateSeatsForFlightsWithSecondaryRBD(Itin* itin, std::set<TravelSeg*> flights, std::vector<BookingCode>& sbkc, bool isFlightsWithPBKC);
  void zeroOutSeatsForSecondaryRBD(std::vector<ClassOfService*>& cosVec, std::vector<BookingCode>& sbkc);
  bool isAnyFlightWithPrimaryBookingCode(Itin* itin, std::set<TravelSeg*>& flights, std::vector<BookingCode>& pbkc, bool isDomesticFlight);
  void processFaremarketsForBadItin(PricingTrx& trx);

  void initializeFareMarketsFromItin(PricingTrx& trx,
                                     ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper);

protected:

  virtual ItinAnalyzerServiceWrapper* selectProcesing(PricingTrx& trx);
  ItinAnalyzerServiceWrapper _itinAnalyzerWrapper{*this};
  ExcItinAnalyzerServiceWrapper _excItinAnalyzerWrapper{*this};
  bool isRexTrxRedirected2ExcTrx(PricingTrx& trx) const;
  bool processRexCommon(RexBaseTrx& trx);

  void diag982(PricingTrx& trx,
               const std::vector<Itin*>::const_iterator begin,
               const std::vector<Itin*>::const_iterator end,
               const std::string& msg);

  bool addFakeTravelSeg(PricingTrx& trx) const;
  bool addFakeTravelSegValidateDates(const PricingTrx& trx) const;
  void addFakeTravelSegCreateFakeTS(const PricingTrx& trx,
                                    const Itin* const itin,
                                    AirSeg*& fakeTS,
                                    bool& addOutbound) const;
  void addFakeTravelSegUpdateAvlMap(PricingTrx& trx, AirSeg* const fakeTS) const;
  virtual void
  getCabins(PricingTrx& trx, AirSeg* const fakeTS, std::vector<ClassOfService*>& cosList) const;

  void removeIncorrectFareMarkets(PricingTrx& trx) const;
  void cloneFakeFareMarkets(PricingTrx& trx);
  void updateFakeFM(PricingTrx& trx) const;
  FareMarket* findFakeFM(PricingTrx& trx, Itin& itin) const;
  void updateFareMarket(FareMarket& fm,
                        CarrierCode carrier,
                        DataHandle& dataHandle,
                        const DateTime& tvlDate) const;

  // This function processes each thru-fare only itin in trx
  // For each such itin, Solo (sum of local) fare markets
  // (i.e. not covering exactly one leg) are marked for removal.
  // If 'keep solo fm' flag is set for a fare market,
  // it is kept.
  // At the end, all fare markets marked for removal
  // are deleted from all itins and from trx.
  void removeSoloFMs(PricingTrx& trx);

  // Iterate over legs (a leg is a subsequence of travel sements
  // inside the itin having the same leg id)
  // For each leg, check if the first or last segment is arunk
  // (non-air transportation)
  // If first such leg found, stop iterating and mark itin as non thru-fare
  // Otherwise, do nothing with the itin.
  void checkThruFMOnlyforNonAirSegs(Itin& itin);

  bool processTaxShoppingTrx(TaxTrx& taxTrx);
  void markFlownSegments(PricingTrx& trx);

public:
  ItinAnalyzerService(const std::string& name, TseServer& srv);

  /**
    * Initialize from config
    *
    * @param argc   command line argument count
    * @param argv   command line arguments
    *
    * @return true if successful, false otherwise
    */
  virtual bool initialize(int argc, char* argv[]) override;

  /**
    * Process a Metrics transaction
    *
    * @param trx    Transaction object to process
    *
    * @return true if successful, false otherwise
    */
  virtual bool process(MetricsTrx& trx) override;

  /**
    * Process a Pricing transaction
    *
    * @param trx    Transaction object to process
    *
    * @return true if successful, false otherwise
    */
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(RepricingTrx& trx) override;
  virtual bool process(FareDisplayTrx& trx) override;
  virtual bool process(TaxTrx& taxTrx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(FlightFinderTrx& trx) override;
  virtual bool process(RexShoppingTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(AncillaryPricingTrx& trx) override;
  virtual bool process(BaggageTrx& trx) override;
  virtual bool process(TktFeesPricingTrx& trx) override;
  virtual bool process(BrandingTrx& trx) override;
  virtual bool process(PricingDetailTrx& trx) override;
  virtual bool process(AltPricingDetailObFeesTrx& trx);
  bool process(StructuredRuleTrx& trx) override;

  void retrieveBrands(PricingTrx& trx);
  void populateGoverningCarrierOverrides(PricingTrx& trx);
  void removeItinsWithoutGoverningCarrier(PricingTrx& trx);
  void setPnrSegmentCollocation(PricingTrx* pricingTrx, std::vector<Itin*>& itins);
  void processSpanishDiscount(PricingTrx& trx);

  bool FDisSpecifiedCarrier(const CarrierCode& carrier);

  /**
    * Sets inbound or outbound for a FareMarket
    */
  bool hasRetransit(const std::vector<TravelSeg*>& segs) const;
  void setInboundOutbound(PricingTrx& trx, const Itin& itin, FareMarket& fareMarket) const;
  FMDirection
  getInboundOutbound(const Itin& itin, const FareMarket& fareMarket, DataHandle& dataHandle) const;

  void deprecated_setInboundOutbound(const Itin& itin,
                                     FareMarket& fareMarket,
                                     DataHandle& dataHandle) const;
  void setInboundOutbound(ShoppingTrx& trx);
  void setInboundOutbound(FlightFinderTrx& trx);

  /**
    * selects the ticketing carrier for each itin in Trx.
    */
  bool selectTicketingCarrier(PricingTrx& trx);

  virtual bool setGeoTravelTypeAndTktCxr(Itin& itin);

  /**
    * Sets a bitmask of trip characteristics
    */
  void setTripCharacteristics(PricingTrx& trx);
  virtual void setTripCharacteristics(Itin* itin);
  /**
    * Sets a bitmask of trip characteristics
    */
  virtual void setIntlSalesIndicator(PricingTrx& trx);
  virtual void setIntlSalesIndicator(Itin& itin, const Loc& ticketingLoc, const Loc& saleLoc);
  /**
    * Sets retransited bool
    */
  void setRetransits(PricingTrx& trx);
  virtual void setRetransits(Itin& itin);

  void setOpenSegFlag(PricingTrx& trx);
  virtual void setOpenSegFlag(Itin& itin);

  void setFareMarketCOS(PricingTrx& trx);

  void checkJourneyAndGetCOS(FlightFinderTrx& trx);

  void
  checkFFInterIntraLineAvail(FlightFinderTrx& trx, Itin* itin, ShoppingTrx::SchedulingOption& sop);

  void getThruAvailForFF(FlightFinderTrx& trx,
                         std::vector<TravelSeg*>& travelSegVec,
                         ShoppingTrx::SchedulingOption& sop);

  void getLocalAvailForFF(FlightFinderTrx& trx,
                          std::vector<TravelSeg*>& travelSegVec,
                          ShoppingTrx::SchedulingOption& sop);

  void updateSopAvailability(ShoppingTrx::SchedulingOption& sop,
                             std::vector<ClassOfServiceList>& cosListVec);

  void saveCarrierPref(bool& differentCarrier,
                       bool& consecutiveSameCx,
                       Itin& curItin,
                       FlightFinderTrx& trx);

  // ******************************** WN SNAP  ******************************** //
  void removeMultiLegFareMarkets(PricingTrx& trx);
  // ******************************** WN SNAP  ******************************** //

  /**
    * setCurrencyOverride()
    */
  virtual void setCurrencyOverride(PricingTrx& trx);

  /**
    * Sets itin rounding
    */
  void setItinRounding(PricingTrx& trx);
  void setItinRounding(Itin& itin);

  void setUpInfoForAltDateJourneyItin(Itin& journeyItin, ShoppingTrx::AltDatePairs& altDatesPair);

  void calculateFlightTimeMinutes(ShoppingTrx& trx);

  void calculateMileage(ShoppingTrx& trx);

  void processFDShopperRequest(FareDisplayTrx& trx,
                               Itin& itin,
                               std::set<CarrierCode>& carrierList,
                               FareMarket& fm);

  void markSimilarItinMarkets(const Itin& itin) const;
  bool needToPopulateFareMarkets(PricingTrx& trx, bool isRepricingFromMIP) const;
  bool itinHasValidatingCxrData(PricingTrx& trx, Itin& itin) const;

  void buildFareMarketsForSimilarItin(PricingTrx& trx, Itin& itin);

  struct IsPartButNotWholeDummyFare : public std::unary_function<const FareMarket*, bool>
  {
    IsPartButNotWholeDummyFare(const std::map<const TravelSeg*, uint16_t>& dummyFCSegs)
      : _dummyFCSegs(dummyFCSegs)
    {
    }

    bool operator()(const FareMarket* fm) const;

  private:
    const std::map<const TravelSeg*, uint16_t>& _dummyFCSegs;
  };

  struct ItinWithoutGoverningCarrier : public std::unary_function<const Itin*, bool>
  {
    ItinWithoutGoverningCarrier(const CarrierCode& cxrOverride) : _cxrOverride(cxrOverride) {}

    bool operator()(const Itin* itin) const;

  private:
    const CarrierCode& _cxrOverride;
  };
}; // End class ItinAnalyzerService

template <typename D>
D*
ItinAnalyzerService::createDiag(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  D* diag = dynamic_cast<D*>(factory->create(trx));
  if (LIKELY(diag == nullptr))
    return nullptr;

  diag->enable(diag->diagnosticType());
  diag->printHeader();
  return diag;
}

template <typename D>
void
ItinAnalyzerService::flushDiag(D* diag)
{
  diag->flushMsg();
}
} // End namespace tse
