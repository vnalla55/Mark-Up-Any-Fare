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

#include "Common/ShpqTypes.h"
#include "DataModel/FarePath.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <tuple>
#include <vector>

namespace tse
{
class AirSeg;
class DateTime;
class DiagManager;
class FareMarket;
class GroupFarePath;
class InterlineTicketCarrier;
class Itin;
class Money;
class TaxResponse;
class TravelSeg;

class ShoppingUtil
{
public:
  typedef std::tuple<int, int, CarrierCode> InternalSopId;
  typedef std::vector<InternalSopId> InternalSopIdVector;
  typedef InternalSopIdVector::iterator InternalSopIdVectorIterator;
  typedef InternalSopIdVector::const_iterator InternalSopIdVectorConstIterator;

  typedef std::tuple<LegId, SopId> ExternalSopId;
  typedef std::vector<ExternalSopId> ExternalSopIdVector;
  typedef ExternalSopIdVector::iterator ExternalSopIdVectorIterator;
  typedef ExternalSopIdVector::const_iterator ExternalSopIdVectorConstIterator;

  typedef std::map<BrandCode, size_t> BrandQualifiedIndex;
  typedef std::map<uint16_t, BrandQualifiedIndex> LegBrandQualifiedIndex;

  typedef std::map<uint16_t, std::set<const PaxTypeFare*> > FaresPerLegMap;
  typedef std::map<uint16_t, BrandCode> ParityBrandPerLegMap;

  // Position definition enumeration for internal sop id
  enum InternalSopIdType
  { InternalSopIdLegId = 0,
    InternalSopIdBitIndex = 1,
    InternalSopIdGovCxr = 2 };

  enum ExternalSopIdType
  { ExternalSopIdLegId = 0,
    ExternalSopIdExtId = 1 };

private:

  static void singleExternalIdSearch(ShoppingTrx& trx,
                                     const ExternalSopId& externalId,
                                     InternalSopId& internalId);

  // Private constructor to prevent
  // this class from being instantiated
  // as an object
  ShoppingUtil() {};

  // Private destructor
  ~ShoppingUtil() {};

public:
  static size_t getNumLegs(const Itin& itin);

  // Returns leg to brand qualified index relation. Brand qualified index is
  // a relation between brand and index in trx.brandProgramVec()
  static LegBrandQualifiedIndex
  createLegBrandIndexRelation(const PricingTrx& trx, const Itin& itin);

  // Create a key based on a
  // string
  static void createKey(const std::string& value, ItinIndex::Key& key);

  // Create a key based on an
  // integer value
  static void createKey(const uint32_t& value, ItinIndex::Key& indexer);

  // Create cxr key using itin
  static void createCxrKey(const Itin* curItin, ItinIndex::Key& cxrCodeKey);

  // Create cxr key using carrier code
  static void createCxrKey(const CarrierCode& cxrCode, ItinIndex::Key& cxrCodeKey);

  // Create direct schedule key
  static void createScheduleKey(ItinIndex::Key& scheduleKey);

  // Create schedule key using itin
  static void createScheduleKey(const Itin* curItin, ItinIndex::Key& scheduleKey);

  static bool partOfJourney(Itin* itin, const TravelSeg* tvlSeg);

  // Retrieve a direct itinerary for a specific
  // carrier, for a specific leg, using
  // the ShoppingTrx object
  static ItinIndex::ItinCell* retrieveDirectItin(ShoppingTrx& trx,
                                                 const uint32_t& legId,
                                                 const ItinIndex::Key& cxrKey,
                                                 const ItinIndex::ItinCheckType& checkType);

  // Retrieve a direct itinerary for a specific
  // carrier, using the ItinIndex object
  static ItinIndex::ItinCell* retrieveDirectItin(ItinIndex& cxrIdx,
                                                 const ItinIndex::Key& cxrKey,
                                                 const ItinIndex::ItinCheckType& checkType);

  // Retrieve a direct itinerary for a specific
  // carrier, using the const ItinIndex object
  static const ItinIndex::ItinCell* retrieveDirectItin(const ItinIndex& cxrIdx,
                                                       const ItinIndex::Key& cxrKey,
                                                       const ItinIndex::ItinCheckType& checkType);

  // Retrieves a vector of carrier codes
  // from the carrier index in the specified
  // leg
  static bool retrieveCarrierList(const ShoppingTrx& trx,
                                  const size_t& legId,
                                  std::vector<CarrierCode>& cxrList);

  static void orderSegsByLeg(ShoppingTrx& trx);
  static void orderSegsInItin(Itin* itin);
  static void orderSegsByItin(ShoppingTrx& trx);
  static void sortItinBySegCount(PricingTrx& trx);
  static bool getFareMarketLegIndices(const ShoppingTrx& trx,
                                      const FareMarket* fm,
                                      uint32_t& first,
                                      uint32_t& last);

  // a function which will map each request-sop to a governing carrier
  // and store the encoded results in res. One vector<uint32_t> is stored
  // for each request-leg, and then a uint32_t for each sop within the leg
  static void
  getEncodedGovCxrs(const ShoppingTrx& trx, std::vector<std::vector<ItinIndex::Key>>& res);

  // Strips the specified substring from the end of a money object
  // to string result
  static std::string stripMoneyStr(const class Money& m, const std::string& stripStr);

  // static int getLegTravelSeg(const ShoppingTrx& trx,
  //                           const std::vector<TravelSeg*>& segs);

  static bool getLegTravelSegIndices(const ShoppingTrx& trx,
                                     const std::vector<TravelSeg*>& segs,
                                     uint32_t& startLeg,
                                     uint32_t& endLeg,
                                     uint32_t& adoptedLeg);

  // Convenience function to get the class of service for a set
  // of travel segs. Results are returned in a vector, with the first
  // item in the vector corresponding to the first travel seg, the
  // second item to the second travel seg, and so forth. Throws
  // ErrorResponseException if the class of service cannot be found.
  //
  static const std::vector<ClassOfServiceList>&
  getClassOfService(const PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key);

  static std::vector<ClassOfServiceList>&
  getClassOfService(PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key);

  static std::vector<ClassOfServiceList>*
  getClassOfServiceNoThrow(PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key);

  static ClassOfServiceList&
  getMaxThruClassOfServiceForSeg(PricingTrx& trx, TravelSeg* seg);

  static const ClassOfServiceList&
  getMaxThruClassOfServiceForSeg(const PricingTrx& trx, TravelSeg* seg);

  static ExternalSopId createExternalSopId(const LegId legId, const SopId extId);

  static uint32_t getFlightBitIndex(const ShoppingTrx& trx, const ExternalSopId& externalId);

  static void
  preparePathsFromFlightMatrix(const ShoppingTrx::FlightMatrix& matrix,
                               std::vector<GroupFarePath*>& paths,
                               std::vector<const SopIdVec*>* fMatrixVectors = nullptr);

  static uint32_t getBitmapForASOLeg(ShoppingTrx& trx,
                                     LegId legId,
                                     const CarrierCode& govCxr,
                                     const SopIdVec& sopIds);

  static uint32_t findSopId(const PricingTrx& trx, LegId leg, uint32_t sop);

  static uint32_t findInternalSopId(const PricingTrx& trx, LegId leg, uint32_t externalSopId);

  static std::vector<Itin*> expandEstimatedOptions(const PricingTrx& trx);

  // Get dummy itinerary for specified carrier
  static Itin*
  getDummyItineraryFromCarrierIndex(ItinIndex& curCarrierIndex, ItinIndex::Key& carrierKey);

  static size_t countNonDummyItineraries(const ShoppingTrx::Leg& leg);

  // given SOP Options counts total stop penalty for the itin
  static int totalStopPenalty(const Itin*, const int stopPenalty);

  // given SOP Options counts total travel Duration penalty for the itin
  static int totalTravDurPenalty(const Itin* itin, const int travDurPenalty);

  // given SOP Options and requested Departure Time counts total DepTime Deviation penalty for the
  // itin
  static int totalDepTimeDevPenalty(const Itin* itin,
                                    const int depTimeDevPenalty,
                                    const DateTime& reqDepDateTime);

  static void setTimeout(PricingTrx& trx, int timeout);

  static bool isShoppingTrx(const Trx& trx)
  {
    const PricingTrx* prcTrx = dynamic_cast<const PricingTrx*>(&trx);
    return (prcTrx ? isShoppingTrx(*prcTrx) : false);
  }

  static bool isShoppingTrx(const PricingTrx& prcTrx)
  {
    return (prcTrx.getTrxType() == PricingTrx::MIP_TRX) ||
           (dynamic_cast<const ShoppingTrx*>(&prcTrx) != nullptr);
  }

  static bool setupFareMarketTravelSegESV(FareMarket* fareMarket, Itin* itin);

  static bool isCabinClassValid(const ShoppingTrx& trx, const SopIdVec& sops);
  static bool areSopConnectionPointsDifferent(const ShoppingTrx& trx,
                                              const SopIdVec& lhSops,
                                              const SopIdVec& rhSops);

  static bool isSimilarOption(const ShoppingTrx& trx,
                              ShoppingTrx::FlightMatrix const& solutions,
                              const SopIdVec& sops);

  static bool isSameClassOfService(const ShoppingTrx& trx,
                                   const TravelSeg* basedTravelSeg,
                                   const TravelSeg* travelSeg);

  static bool isSameCxrAndCnxPointAndClassOfService(const ShoppingTrx& trx,
                                                    const ShoppingTrx::SchedulingOption& basedSop,
                                                    const ShoppingTrx::SchedulingOption& sop);

  // Get cheapest ESV values // OW, RT, CT, OJ
  static void getCheapestESVValues(const ShoppingTrx& trx,
                                   const Itin* itin,
                                   std::vector<MoneyAmount>& esvValues);

  /**
   * Compare travel segment by orig/dest airport and departure date.
   * Reports two travel segments are not the same if both are not AirSeg.
   *
   * Tweak: in Carnival SOL trx reports two travel segs as the same even in the case, when
   *        both are not AirSeg.
   * Tweak: in Carnival SOL trx and IS exchange trx also compare marketing carrier code
   *        of travel segments
   */
  static bool travelSegsSimilar(ShoppingTrx& trx, const TravelSeg* seg1, const TravelSeg* seg2);

  /**
   * Compare scheduling options if both share the same connecting city
   *  by checking travel segments in the way travelSegsSimilar() does
   *
   * Tweak: in Carnival SOL trx also compares SOPs governing carrier
   */
  static bool schedulingOptionsSimilar(ShoppingTrx& trx,
                                       const ShoppingTrx::SchedulingOption& sop1,
                                       const ShoppingTrx::SchedulingOption& sop2);

  static uint64_t buildAvlKey(const std::vector<TravelSeg*>& seg);

  static void getIdsVecForKey(const uint64_t key, std::vector<uint16_t>& idsVec);

  static void
  getIdsVecForKey(const PricingTrx::ClassOfServiceKey& key, std::vector<uint16_t>& idsVec);

  static std::vector<ClassOfServiceList>&
  getThruAvailability(const PricingTrx& trx, std::vector<TravelSeg*>& travelSeg);

  static std::vector<ClassOfServiceList>&
  getLocalAvailability(const PricingTrx& trx, std::vector<TravelSeg*>& travelSeg);

  static ClassOfServiceList&
  getAvailability(const PricingTrx& trx, TravelSeg* travelSeg);

  static void prepareFFClassOfService(ShoppingTrx::SchedulingOption& sop);

  static void buildCarriersSet(const Itin* itin, std::set<CarrierCode>& crxSet);

  static bool checkCarriersSet(ShoppingTrx& trx,
                               CarrierCode& validatingCarrier,
                               InterlineTicketCarrier* itc,
                               std::set<CarrierCode>& crxSet);

  static FarePath* getFarePathForKey(const PricingTrx& trx,
                                     const Itin* const itin,
                                     const FarePath::FarePathKey& farePathKey);

  static FarePath* getFarePathForKey(const PricingTrx& trx,
                                     const Itin* const itin,
                                     const PaxType* paxType,
                                     const uint16_t outBrandIndex = INVALID_BRAND_INDEX,
                                     const uint16_t inBrandIndex = INVALID_BRAND_INDEX,
                                     const Itin* outItin = nullptr,
                                     const Itin* inItin = nullptr);

  static TaxResponse* getTaxResponseForKey(const PricingTrx& trx,
                                           const Itin* const itin,
                                           const FarePath::FarePathKey& farePathKey);

  static TaxResponse* getTaxResponseForKey(const PricingTrx& trx,
                                           const Itin* const itin,
                                           const PaxType* paxType,
                                           const uint16_t outBrandIndex = INVALID_BRAND_INDEX,
                                           const uint16_t inBrandIndex = INVALID_BRAND_INDEX,
                                           const Itin* outItin = nullptr,
                                           const Itin* inItin = nullptr);

  static bool isBrandValid(const PricingTrx& trx, const uint16_t brandIndex);
  static bool isBrandValid(PricingTrx& trx, const uint16_t brandIndex, const DatePair& datePair);

  static std::vector<std::vector<ClassOfService*>*>
  getFMCOSBasedOnAvailBreakFromFU(PricingTrx& trx,
                                  FarePath* farePath,
                                  const std::vector<int32_t>& nextFareMarkets,
                                  const uint32_t fareMarketIndex);

  static void getFMCOSEmpty(PricingTrx& trx, FareMarket* fareMarket);
  static void getFMCOSBasedOnAvailBreak(PricingTrx& trx,
                                        Itin* itin,
                                        FareMarket* fareMarket,
                                        bool processPartOfJourneyWithMergedAvl = true);
  static void getFMCOSBasedOnAvailBreakImpl(PricingTrx& trx,
                                            Itin* itin,
                                            FareMarket* fareMarket,
                                            bool processPartOfJourneyWithMergedAvl = true);
  static void mergeFMCOSBasedOnAvailBreak(PricingTrx& trx,
                                          Itin* itin,
                                          FareMarket* fareMarket,
                                          bool processPartOfJourneyWithMergedAvl = true);
  static void mergeFMCOSBasedOnAvailBreakImpl(PricingTrx& trx,
                                              Itin* itin,
                                              FareMarket* fareMarket,
                                              bool processPartOfJourneyWithMergedAvl = true);
  static bool mergeClassOfService(PricingTrx& trx,
                                  ClassOfServiceList** orig,
                                  ClassOfServiceList& tbm);

  static const ClassOfService*
  getCOS(const std::vector<ClassOfService*>& cosVec, const BookingCode& bc);

  static ClassOfServiceList*
  selectFareMarketCOS(PricingTrx& trx,
                      Itin* itin,
                      TravelSeg* travelSeg,
                      ClassOfServiceList* cos,
                      bool processPartOfJourneyWithMergedAvl = true);
  static ClassOfServiceList*
  getFlowJourneyAvailability(PricingTrx& trx, Itin* itin, TravelSeg* travelSegment);

  static Itin* getItinForFareMarketMIP(const PricingTrx& trx, const FareMarket* fareMarket);

  static bool checkMinConnectionTime(const TravelSeg* firstTravelSeg,
                                     const TravelSeg* secondTravelSeg,
                                     int64_t minimumConnectTime /* In seconds */);

  static bool checkMinConnectionTime(const TravelSeg* firstTravelSeg,
                                     const TravelSeg* secondTravelSeg,
                                     const PricingOptions* pricingOptions);

  static bool checkMinConnectionTime(const PricingOptions* pricingOptions,
                                     const SopIdVec& sops,
                                     const LegVec& legs);

  static bool checkPositiveConnectionTime(const SopIdVec& sops, const LegVec& legs);

  static void mergeCxrKeys(const shpq::CxrKeys& newCxrKeys, shpq::CxrKeys& carriers);
  static void mergeCxrKeys(const FareMarket& fm, shpq::CxrKeys& carriers);
  static void
  collectFPCxrKeys(const FarePath& farePath, const size_t noOfLegs, shpq::CxrKeysPerLeg& result);
  static bool collectFPCxrKeysNew(ShoppingTrx& trx,
                                  const FarePath& farePath,
                                  const size_t noOfLegs,
                                  shpq::CxrKeysPerLeg& result);
  static void
  collectSopsCxrKeys(const ShoppingTrx& trx, const SopIdVec& sops, shpq::CxrKeyPerLeg& result);
  static void collectOnlineCxrKeys(const shpq::CxrKeysPerLeg& cxrKeysPerLeg,
                                   shpq::CxrKeys& onlineCxrKeys,
                                   bool& isInterlineApplicable);

  static bool isOnlineConnectionFlight(const int& sop,
                                       const int& legIndex,
                                       const CarrierCode* carrier,
                                       const ShoppingTrx* trx,
                                       const bool needDirectFlt = false,
                                       const bool flightOnlySolution = false);

  template <typename T>
  static bool isOnlineFlight(const T& itin, CarrierCode carrier = "")
  {
    return isOnlineFlightRef(itin, carrier);
  }

  template <typename T>
  static bool isOnlineFlightRef(const T& itin, CarrierCode& carrier)
  {
    for (unsigned int i = 0; i < itin.travelSeg().size(); ++i)
    {
      const AirSeg* const airSeg = itin.travelSeg()[i]->toAirSeg();
      if (UNLIKELY(airSeg == nullptr))
        continue;
      if (carrier.empty())
        carrier = airSeg->marketingCarrierCode();
      else if (carrier != airSeg->marketingCarrierCode())
        return false;
    }
    return true;
  }

  static bool isOnlineOptionForCarrier(const ShoppingTrx::SchedulingOption* outbound,
                                       const ShoppingTrx::SchedulingOption* inbound,
                                       const CarrierCode& carrier);

  template <typename V>
  static bool
  isOnlineOptionForCarrier(const ShoppingTrx& trx, const V& sopVec, const CarrierCode& carrier);

  template <typename V>
  static bool isOnlineFlightSolution(const ShoppingTrx& trx, const V& sopVec);

  static bool checkForcedConnection(const SopIdVec& sops, const ShoppingTrx& trx);
  static bool checkOverridedSegment(const SopIdVec& sops, const ShoppingTrx& trx);
  static bool isCustomSolutionGfp(const ShoppingTrx& trx, GroupFarePath* gfp);
  static bool isCustomLegFM(const ShoppingTrx& trx, const FareMarket* fm);
  static bool isSpanishDiscountApplicableForShopping(const PricingTrx& trx, const Itin* itin);

  static int
  getRequestedBrandIndex(PricingTrx& trx, const ProgramID& programId, const BrandCode& brandCode);

  static BrandCode& getBrandCode(const PricingTrx& trx, int index);
  static ProgramCode& getProgramCode(const PricingTrx& trx, int index);
  static ProgramID& getProgramId(const PricingTrx& trx, int index);

  static std::string getBrandPrograms(const PricingTrx& trx, const FarePath* fPath);
  static std::string getBrandCodeString(const PricingTrx& trx, int brandIndex);
  static std::string getFakeBrandString(const PricingTrx& trx, int brandIndex);
  static bool isThisBrandReal(const BrandCode& brand);

  template <typename BrandCodeContainerT>
  static bool isAnyBrandReal(const BrandCodeContainerT& brandCodes) {
    for (const BrandCode& brand : brandCodes)
    {
      if (isThisBrandReal(brand))
        return true;
    }
    return false;
  }

  static std::string getFarePathBrandCode(const FarePath* farePath);
  static std::string
  getFareBrandProgram(const PricingTrx& trx, const BrandCode& brandCode, const PaxTypeFare* ptf,
                      Direction fareUsageDirection);

  static char getIbfErrorCode(IbfErrorMessage ibfErrorMessage);

  static bool isValidItinBrand(const Itin* itin, BrandCode brandCode);

  static std::set<BrandCode> getHardPassedBrandsFromFares(const PricingTrx& trx,
                                                          const std::set<const PaxTypeFare*>& fares);
  static FaresPerLegMap buildFaresPerLegMap(const FarePath& farePath);
  static void removeBrandsNotPresentInFilter(std::set<BrandCode>& brands,
                                               const BrandFilterMap& filter);

  static ParityBrandPerLegMap createParityBrandPerLegMap(const PricingTrx& trx,
                                                         const FarePath& farePath);

  static BrandCode getFirstBrandValidForAllFares(const PricingTrx& trx,
                                           const std::set<BrandCode>& brands,
                                           const std::set<const PaxTypeFare*>& fares);

  static void saveParityBrandsToFareUsages(FarePath& farePath, const PricingTrx& trx,
                                  const ShoppingUtil::ParityBrandPerLegMap& parityBrandPerLegMap);

  static std::string getFlexFaresValidationStatus(const Record3ReturnTypes& result);

  template <class T>
  static void BrandIndicesToCodes(PricingTrx& trx, const T& s, BrandCodeSet& result);

  template <class V>
  static std::size_t getTravelSegCount(const ShoppingTrx& trx, const V& sops);

  static std::size_t
  getTravelSegCountForExternalSopID(const ShoppingTrx& trx, LegId leg, int origSopId);

  template <class V>
  static bool isCustomSolution(const ShoppingTrx& trx, const V& sops);

  template <class V>
  static bool isLongConnection(const ShoppingTrx& trx, const V& sops);

  template <class V>
  static bool isDirectFlightSolution(const ShoppingTrx& trx, const V& sops);

  template <class V>
  static bool isOnlineSolution(const ShoppingTrx& trx, const V& sops);

  /**
   *
   *   @method isIndustryFareUsed
   *
   *   Description: Checks to see if fare path uses industry fare(s)
   *   @param  FarePath      - farePath
   *
   *   @return bool          - true if fare path uses industry fare(s), false otherwise.
   */
  static bool isIndustryFareUsed(const FarePath& farePath);

  // currently it supports two-leg requests only
  static bool isOpenJaw(const ShoppingTrx& shoppingTrx);
  static bool isBrazilDomestic(const ShoppingTrx& shoppingTrx);

  static std::vector<FareMarket*> getUniqueFareMarketsFromItinMatrix(ShoppingTrx& trx);

  static bool isDuplicatedFare(const PaxTypeFare* ptf1, const PaxTypeFare* ptf2);
  static void displayRemovedFares(const FareMarket& fm,
                                  const uint16_t& uniqueFareCount,
                                  const std::vector<PaxTypeFare*>& removedFaresVec,
                                  DiagManager& diag);

  // Call it with valid (i.e. not-shared) FareMarket::classOfServiceVec().
  static void updateFinalBooking(FarePath& farePath);
  static void updateFinalBookingBasedOnAvailBreaks(PricingTrx& trx, FarePath& farePath, Itin& itin);
  static void updateFinalBooking(FarePath& farePath, FareUsage& fu);

  static void updateFinalBooking(FarePath& farePath,
                                 FareUsage& fu,
                                 const std::vector<std::vector<ClassOfService*>*>& avlVec);
  static ClassOfService* findCOS(const std::vector<ClassOfService*>& avl, BookingCode bc);

  static bool isJCBMatched(const PricingTrx& trx);

  static std::vector<CarrierCode> getGoverningCarriersPerLeg(const Itin& itin);

  static void removeSoftPassesFromFareMarket(FareMarket* fm,
                                             unsigned int brandIndex,
                                             DiagCollector* diag,
                                             Direction direction = Direction::BOTHWAYS);

  static bool validHardPassExists(const FareMarket* fm,
                                  unsigned int brandIndex,
                                  std::set<Direction>& hardPassedDirections,
                                  DiagCollector* diag,
                                  bool useDirectionality,
                                  bool printAllFares);

  static bool hardPassForBothDirectionsFound(const std::set<Direction>& hardPassedDirections);

  static bool isMinConnectionTimeMet(const std::vector<ShoppingTrx::Leg>& legs,
                                     const PricingOptions* options,
                                     bool allowIllogicalFlights,
                                     std::vector<uint16_t>& problematicLegs);

  static bool mctIsMetForFirstSopOnEachLeg(const std::vector<ShoppingTrx::Leg>& legs,
                                           const PricingOptions* options,
                                           bool allowIllogicalFlights);

  static bool mctIsMetForAnySopCombination(const std::vector<ShoppingTrx::Leg>& legs,
                                           const PricingOptions* options,
                                           bool allowIllogicalFlights,
                                           std::vector<uint16_t>& problematicLegs);

private:
  static std::vector<ClassOfServiceList>&
  getClassOfServiceInternal(const PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key);

}; // End of ShoppinhUtil class

template <typename V>
bool
ShoppingUtil::isOnlineOptionForCarrier(const ShoppingTrx& trx,
                                       const V& sopVec,
                                       const CarrierCode& carrier)
{
  if (sopVec.size() == 0)
    return false;

  for (unsigned int legId = 0; legId < sopVec.size(); ++legId)
  {
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legId].sop()[sopVec[legId]];
    const Itin& itin = *sop.itin();
    if (!isOnlineFlight(itin, carrier))
      return false;
  }
  return true;
}

template <typename V>
bool
ShoppingUtil::isOnlineFlightSolution(const ShoppingTrx& trx, const V& sopVec)
{
  if (sopVec.empty())
    return false;

  CarrierCode cxr = "";
  for (unsigned int legId = 0; legId < sopVec.size(); ++legId)
  {
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legId].sop()[sopVec[legId]];
    const Itin& itin = *sop.itin();
    if (!isOnlineFlightRef(itin, cxr))
      return false;
  }
  return true;
}

template <class V>
std::size_t
ShoppingUtil::getTravelSegCount(const ShoppingTrx& trx, const V& sops)
{
  std::size_t result = 0;

  std::size_t leg = 0;
  typename V::const_iterator it, end;
  for (it = sops.begin(), end = sops.end(); it != end; ++it, ++leg)
  {
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[leg].sop()[*it];
    const Itin& itin = *sop.itin();
    result += itin.travelSeg().size();
  }

  return result;
}

template <class V>
bool
ShoppingUtil::isCustomSolution(const ShoppingTrx& trx, const V& sops)
{
  bool isCustom = false;

  for (std::size_t leg = 0; leg < sops.size(); ++leg)
  {
    if (!trx.legs()[leg].isCustomLeg())
      continue; /*shadow leg for custom solution detection*/

    if (trx.legs()[leg].sop()[sops[leg]].isCustomSop())
    {
      isCustom = true;
    }
    else
    {
      isCustom = false;
      break;
    }
  }

  return isCustom;
}

template <class V>
bool
ShoppingUtil::isLongConnection(const ShoppingTrx& trx, const V& sops)
{
  for (std::size_t leg = 0; leg < sops.size(); ++leg)
  {
    if (trx.legs()[leg].sop()[sops[leg]].isLngCnxSop())
      return true;
  }

  return false;
}

template <class V>
bool
ShoppingUtil::isDirectFlightSolution(const ShoppingTrx& trx, const V& sops)
{
  for (std::size_t legIdx = 0; legIdx < sops.size(); ++legIdx)
  {
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legIdx].sop()[sops[legIdx]];
    if (sop.itin()->travelSeg().size() > 1)
      return false;
  }
  return true;
}

template <class V>
bool
ShoppingUtil::isOnlineSolution(const ShoppingTrx& trx, const V& sops)
{
  for (std::size_t legIdx = 1; legIdx < sops.size(); ++legIdx)
  {
    const ShoppingTrx::SchedulingOption& prevSop = trx.legs()[legIdx - 1].sop()[sops[legIdx - 1]];
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legIdx].sop()[sops[legIdx]];
    if (prevSop.governingCarrier() != sop.governingCarrier())
      return false;
  }
  return true;
}

template <class T>
void
ShoppingUtil::BrandIndicesToCodes(PricingTrx& trx, const T& s, BrandCodeSet& result)
{
  typename T::const_iterator i = s.begin();
  typename T::const_iterator iEnd = s.end();

  for (; i != iEnd; ++i)
    result.insert(getBrandCode(trx, *i));
}

} // End namespace tse
