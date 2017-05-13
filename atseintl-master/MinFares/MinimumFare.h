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

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "MinFares/MinFareFareSelection.h"

#include <iosfwd>
#include <vector>

namespace tse
{
class DiagCollector;
class FarePath;
class FareUsage;
class Itin;
class Loc;
class LocKey;
class MinFareAppl;
class MinFareDefaultLogic;
class MinFarePlusUpItem;
class MinFareRuleLevelExcl;
class PaxType;
class PricingTrx;
class PricingUnit;
class TravelSeg;

/**
 * @class MinimumFare
 *
 * This is the base class of all Minimum Fare modules.
 *
 * This class provides the common functions used by all Minimum Fare modules.
 *
 */

class MinimumFare
{
public:
  enum Appl
  {
    YES = 'Y',
    NO = 'N',
    PERMITTED = 'P'
  };

  enum PointType
  {
    TICKET_POINT = 'T',
    STOPOVER_POINT = 'S',
    CONNECTION_POINT = 'C',
    ANY_POINT = 'Z'
  };

  enum ServiceRestrition
  {
    ONLINE_SERVICE = 'O',
    INLINE_SERVICE = 'I'
  };

  enum TariffCategory
  {
    BLANK_TRF_CAT = -1,
    PUBLIC_TRF_CAT = 0,
    PRIVATE_TRF_CAT = 1
  };

  enum HipProcess
  {
    DEFAULT_HIP_PROCESSING, // Standard (default) Hip Processing
    NORMAL_HIP_PROCESSING // Normal Fare Hip Processing
  };

  static constexpr char BLANK = ' ';
  static constexpr Indicator INCLUDE = 'I';
  static constexpr Indicator EXCLUDE = 'E';
  static const TariffNumber ANY_TARIFF = -1;
  static const int32_t DEFAULT_TEXT_TBL_ITEM_NO = 0;
  static const std::string MISSING_DATA_ERROR_MSG;
  static const std::string ROUTING_EXCLUSION_IND;
  static const std::string MILEAGE_EXCLUSION_IND;
  static const std::string RULE_EXCLUSION_IND[];

  static const Loc* originOfPricingUnit(const PricingUnit& pu);

  static const Loc* origin(const std::vector<TravelSeg*>& travelSegs);
  static const Loc* destination(const std::vector<TravelSeg*>& travelSegs);

  static bool isBetweenLocs(PricingTrx& trx,
                            const Loc& loc1,
                            const Loc& loc2,
                            const LocTypeCode& loc1Type,
                            const LocCode& loc1Code,
                            const LocTypeCode& loc2Type,
                            const LocCode& loc2Code);

  static bool isWithinLoc(PricingTrx& trx,
                          const std::vector<TravelSeg*>& travelSegs,
                          const LocTypeCode& locType,
                          const LocCode& locCode,
                          bool checkHiddenStop = true);

  static bool matchGeo(PricingTrx& trx,
                       const Directionality directionality,
                       const LocKey& loc1,
                       const LocKey& loc2,
                       const std::vector<TravelSeg*>& tvlSegs,
                       const Itin& itin,
                       bool reverseOrigin = false);

  static bool isStopOver(const Itin& itin,
                         const FareUsage* fareUsage,
                         const TravelSeg& travelSeg,
                         const TravelSeg& nextTravelSeg,
                         bool isNextTravelSegSurfaceAtFareBreak = false);

  static bool
  checkIntermediateExclusion(PricingTrx& trx,
                             const MinimumFareModule module,
                             const MinFareAppl& matchedApplItem,
                             const MinFareDefaultLogic* matchedDefaultItem,
                             const bool& selectNormalFare,
                             const std::vector<TravelSeg*>& travelSegs,
                             const Itin& itin,
                             const PricingUnit* pu,
                             const FareUsage* fareUsage,
                             const MinFareFareSelection::FareDirectionChoice fareDirection,
                             std::vector<TravelSeg*>::const_iterator travelBoardIter,
                             std::vector<TravelSeg*>::const_iterator travelOffIter,
                             bool isBoardSegAfterSurfaceAtFareBreak = false,
                             bool isOffSegFollowedBySurfaceAtFareBreak = false,
                             bool reverseFcOrigin = false);

  static bool checkAdditionalPoint(const MinimumFareModule module,
                                   const MinFareAppl& matchedApplItem,
                                   const MinFareDefaultLogic* matchedDefaultItem,
                                   const std::vector<TravelSeg*>& travelSegs,
                                   std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                   std::vector<TravelSeg*>::const_iterator travelOffIter,
                                   bool stopOver);

  static bool checkAdditionalNation(const MinFareAppl& matchedApplItem,
                                    const MinFareDefaultLogic* matchedDefaultItem,
                                    const bool& selectNormalFare,
                                    const std::vector<TravelSeg*>& travelSegs,
                                    const Itin& itin,
                                    const PricingUnit* pu,
                                    const FareUsage* fareUsage,
                                    const MinFareFareSelection::FareDirectionChoice fareDirection,
                                    std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                    std::vector<TravelSeg*>::const_iterator travelOffIter,
                                    bool isBoardSegAfterSurfaceAtFareBreak,
                                    bool isOffSegFollowedBySurfaceAtFareBreak);

  static bool checkDomesticExclusion(const Itin& itin,
                                     const PaxTypeFare& paxTypeFare,
                                     const MinFareAppl* matchedApplItem,
                                     const MinFareDefaultLogic* matchedDefaultItem,
                                     std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                     std::vector<TravelSeg*>::const_iterator travelOffIter);

  static bool
  checkIntermediateCityPair(const MinimumFareModule module,
                            const MinFareAppl& matchedApplItem,
                            const MinFareDefaultLogic* matchedDefaultItem,
                            const bool& selectNormalFare,
                            const std::vector<TravelSeg*>& travelSegs,
                            const Itin& itin,
                            std::vector<TravelSeg*>::const_iterator travelBoardIter,
                            std::vector<TravelSeg*>::const_iterator travelOffIter,
                            const MinFareFareSelection::FareDirectionChoice fareDirection =
                                MinFareFareSelection::OUTBOUND);

  static bool checkIntermediateStop(const Itin& itin,
                                    const FareUsage* fareUsage,
                                    const std::vector<TravelSeg*>& thruFareTravelSegs,
                                    std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                    std::vector<TravelSeg*>::const_iterator travelOffIter,
                                    bool isBoardSegAfterSurfaceAtFareBreak,
                                    bool isOffSegFollowedBySurfaceAtFareBreak);

  /**
   * This function is called to print fare component information
   *
   * @param fare A reference to the fare component.
   */
  static void printFareInfo(const PaxTypeFare& paxTypeFare,
                            DiagCollector& diag,
                            const MinimumFareModule module,
                            const char* prefix = nullptr,
                            bool asRoundTripFare = false,
                            const MoneyAmount& plusUpAndSurcharge = 0,
                            const bool isNetFare = false);

  static void printFareInfo(const PtfPair& ptfPair,
                            MinFareFareSelection::FareDirectionChoice fareDirection,
                            DiagCollector& diag,
                            const MinimumFareModule module,
                            const char* prefix = nullptr,
                            bool asRoundTripFare = false,
                            const MoneyAmount& plusUpAndSurcharge = 0);

  void printPaxTypeFare(const PaxTypeFare& paxTypeFare,
                        std::ostream& os,
                        MinimumFareModule module,
                        const char* prefix = nullptr,
                        bool isRtFare = false,
                        const MoneyAmount& plusUpAndSurcharge = 0);

  MinimumFare(const DateTime& travelDate = DateTime::emptyDate()) : _travelDate(travelDate) {}
  virtual ~MinimumFare() = default;

  /**
    * This function is to scan all intermediate markets to get highest fare amount.
    *
    * @param paxTypeFare a reference to PaxTypeFare
    * @param requestedPaxType a reference to PaxType which is from the input entry
    * @param diag a reference to diagcollector.
  */
  void processIntermediate(const MinimumFareModule module,
                           const MinFareFareSelection::EligibleFare eligibleFare,
                           PricingTrx& trx,
                           const Itin& itin,
                           const PaxTypeFare& paxTypeFare,
                           const PaxType& requestedPaxType,
                           DiagCollector* diag,
                           bool selectNormalFare,
                           const MinFareFareSelection::FareDirectionChoice selectOutBound,
                           std::set<uint32_t>& normalExemptSet,
                           MinFarePlusUpItem& curPlusUp,
                           const FarePath* farePath,
                           const PricingUnit* pu = nullptr,
                           FareUsage* fu = nullptr,
                           const PaxTypeCode& actualPaxTypeCode = "",
                           bool reverseFcOrigin = false);

  bool checkNormalExempt(const MinimumFareModule module,
                         const MinFareFareSelection::EligibleFare eligibleFare,
                         PricingTrx& trx,
                         const Itin& itin,
                         const PaxTypeFare& paxTypeFare,
                         const PaxTypeFare*& paxTypeFareBase,
                         const PaxTypeFare*& paxTypeFareInterm,
                         const PaxType& requestedPaxType,
                         std::vector<TravelSeg*>::const_iterator travelBoardIter,
                         std::vector<TravelSeg*>::const_iterator travelOffIter,
                         const FarePath* farePath = nullptr,
                         const PricingUnit* pu = nullptr,
                         PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN);

  bool matchSpecialGeo(PricingTrx& trx,
                       std::vector<TravelSeg*>::const_iterator travelBoardIter,
                       std::vector<TravelSeg*>::const_iterator travelOffIter);

  virtual bool compareAndSaveFare(const PaxTypeFare& paxTypeFareThru,
                                  const PaxTypeFare& paxTypeFareIntermediate,
                                  MinFarePlusUpItem& curPlusUp,
                                  bool useInternationalRounding = false,
                                  bool outbound = true)
  {
    return false;
  }

  virtual bool compareAndSaveFare(const PaxTypeFare& paxTypeFareThru,
                                  const PtfPair& constFares,
                                  MinFarePlusUpItem& curPlusUp,
                                  bool useInternationalRounding = false,
                                  bool outbound = true)
  {
    return false;
  }

  /**
   * This function is called to compare the latest higher fare amount against
   *  the current unpublish fare amount.
   *
   * @param paxTypeFareThru A reference to current thru fare component.
   * @param paxTypeFareIntermediate A reference to current intermediate fare component.
   *
   * @return true, indicates that the comparison results in a fare plus up
   *         false, no plus up.
   */
  virtual bool compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                  MoneyAmount unPubAmount,
                                  const LocCode& boardPoint,
                                  const LocCode& constPoint,
                                  const LocCode& offPoint,
                                  MinFarePlusUpItem& curPlusUp,
                                  bool useInternationalRounding = false,
                                  bool outbound = true)
  {
    return false;
  }

  bool savePlusUpInfo(const PaxTypeFare& paxTypeFareThru,
                      const PtfPair& ptfPair,
                      MinFarePlusUpItem& curPlusUp,
                      bool useInternationalRounding = false,
                      bool outbound = true);

  /**
   * This function is called to print exception information if minimum fare module is excluded
   *
   * @param paxTypeFare A reference to PaxTypeFare
   * @param diag a reference to diagcollector.
   */
  static void printExceptionInfo(const PaxTypeFare& paxTypeFare, DiagCollector& diag);

  void printException(const PaxTypeFare& paxTypeFare, std::ostream& os);

  /**
   * This function is called to print exception information if minimum fare module
   *  is excluded in case of no fares found and print information from travel segments
   *
   * @param paxTypeFare A reference to PaxTypeFare
   * @param diag a reference to diagcollector.
   */
  static void printExceptionInfo(DiagCollector& diag,
                                 const LocCode& boardCity,
                                 const LocCode& offCity,
                                 const std::string& exceptionMsg = "");

  static void printExceptionInfo(DiagCollector& diag,
                                 const std::vector<TravelSeg*>& travelSegs,
                                 const MinFareFareSelection::FareDirectionChoice direction,
                                 const std::string& exceptionMsg = "");

  void printException(const TravelSeg& tvlBoard, const TravelSeg& tvlOff, std::ostream& os);

  const MinFareRuleLevelExcl* _matchedRuleItem = nullptr;
  const MinFareAppl* _matchedApplItem = nullptr;
  const MinFareDefaultLogic* _matchedDefaultItem = nullptr;

  bool isSpecialHipSpecialOnly() { return _spclHipSpclOnly; }

  const DateTime& travelDate() const { return _travelDate; }

  void adjustRexPricingDates(PricingTrx& trx, const DateTime& retrievalDate);

  void adjustPortExcDates(PricingTrx& trx);

  void restorePricingDates(PricingTrx& trx, const DateTime& ticketDate);

  static void printRexDatesAdjustment(DiagCollector& diag, const DateTime& retrievalDate);

protected:
  void checkNigeriaCurrencyAdjustment(DiagCollector* diag,
                                      MinimumFareModule module,
                                      MinFarePlusUpItem& curPlusUp,
                                      PricingTrx& trx,
                                      const Itin& itin,
                                      const FarePath* farePath,
                                      const PricingUnit* pu,
                                      const FareUsage* thruFareUsage,
                                      const PaxTypeFare& interPaxTypeFare,
                                      const PaxType& requestedPaxType,
                                      const CabinType& cabin,
                                      bool selectNormalFare,
                                      MinFareFareSelection::FareDirectionChoice fareDirection,
                                      MinFareFareSelection::EligibleFare eligibleFare,
                                      const std::vector<TravelSeg*>& travelSeg,
                                      const MinFareAppl* matchedApplItem,
                                      const MinFareDefaultLogic* matchedDefaultItem,
                                      const PaxTypeFare* nmlThruFare = nullptr);

  static bool matchHIPOrigin(std::vector<TravelSeg*>::const_iterator travelBoardIter,
                             std::vector<TravelSeg*>::const_iterator travelOffIter,
                             std::vector<TravelSeg*>::const_iterator tvlBrdThruIter,
                             std::vector<TravelSeg*>::const_iterator tvlOffThruIter,
                             bool reverseFcOrigin,
                             Indicator hipExemptInterToInter,
                             Indicator hipOrigInd,
                             Indicator hipOrigNationInd,
                             Indicator hipFromInterInd);

  static bool matchHIPDestination(std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                  std::vector<TravelSeg*>::const_iterator travelOffIter,
                                  std::vector<TravelSeg*>::const_iterator tvlBrdThruIter,
                                  std::vector<TravelSeg*>::const_iterator tvlOffThruIter,
                                  bool reverseFcOrigin,
                                  Indicator hipDestInd,
                                  Indicator hipDestNationInd,
                                  Indicator hipToInterInd);

protected:
  HipProcess _hipProcess = HipProcess::DEFAULT_HIP_PROCESSING;
  bool _spclHipSpclOnly = false;
  DateTime _travelDate;
};
}
