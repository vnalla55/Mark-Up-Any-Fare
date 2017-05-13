//----------------------------------------------------------------------------
//
//  File:        RuleUtil.h
//  Authors:     Devapriya SenGupta
//
//  Updates:
//----------------------------------------------------------------------------
//               5-11-2004: Andrew Ahmad
//                 Added method: scopeTSIGeo
//----------------------------------------------------------------------------
//
//
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

#include "Common/LocUtil.h"
#include "Common/MatchFareClass.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBAForwardDecl.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleUtilTSI.h"

class RuleUtilTest;
class RuleUtilTest2;

namespace tse
{
class PricingTrx;
class Itin;
class FarePath;
class FareMarket;
class PaxTypeFare;
class FareUsage;
class PricingUnit;
class TravelSeg;
class SurchargeData;
class DateTime;

class LocKey;
class GeoRuleItem;
class DiagCollector;
class DiagManager;

class AirSeg;
class PeriodOfStay;
class NoPNRPricingTrx;
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;
class GeneralRuleApp;
class CombinabilityRuleInfo;
class DateOverrideRuleItem;
class FareByRuleApp;
class ExchangePricingTrx;
class CarrierFlight;
class Agent;
class RuleItemInfo;
class CategoryRuleItemInfo;
class FareByRuleItemInfo;
class Logger;
class FareFocusDaytimeApplInfo;
class FareFocusDaytimeApplDetailInfo;
/**
* @class RuleUtil
*
* @brief Defines the Rules Utility class, containing
* all the common methods related to the rules processing
*
*/

class RuleUtil : public RuleUtilTSI
{

private:
  RuleUtil() : RuleUtilTSI() {} // only static class members, hide constructor
public:
  // Declare Cpp Unit test class a friend to allow
  //  unit testing of private methods
  friend class ::RuleUtilTest;
  friend class ::RuleUtilTest2;
  friend class RuleUtilGeoTest;
  friend class RuleUtilMatchTvlSegsTest;
  friend class RuleUtilTest_checkLoopMatch;

  enum EOEAllSegmentIndicator
  { AllSegment,
    CommonPoint,
    Adjacent };

  static constexpr Indicator BLANK_INDICATOR = ' ';
  static const int32_t DEFAULT_TEXT_TBL_ITEM_NO = 0;
  static constexpr Indicator LOGIC_AND = 'A';
  static constexpr Indicator LOGIC_OR = 'O';
  static void getCat27TourCode(const PaxTypeFare* paxTypeFare, std::string& tourCode);

  static int32_t getCat17Table996ItemNo(PricingTrx& trx,
                                        const PaxTypeFare& paxTypeFare,
                                        const PaxTypeFare*& paxFareForCat17);

  static void travelSegWrapperVec2TSVec(const TravelSegWrapperVector& wrapperVec,
                                        std::vector<TravelSeg*>& vector,
                                        bool requireBothOrigDestMatch = false);

  static bool validateTvlDateRange(const DateTime& refDT,
                                   const FareFocusDaytimeApplDetailInfo* ffdtaInfo);

  static bool matchTravelRangeX5(PricingTrx& trx,
                                 const PaxTypeFare& ptf,
                                 DateTime createDate,
                                 const FareFocusDaytimeApplInfo* ffdtInfo);

  /// Table 995 validation

  static bool validateGeoRuleItemCreateDiag(PricingTrx& trx,
                                            const DiagnosticTypes& callerDiag,
                                            DCFactory*& factory,
                                            DiagCollector*& diag,
                                            const uint32_t itemNo,
                                            const VendorCode& vendorCode,
                                            const RuleConst::TSIScopeParamType defaultScope,
                                            const FareMarket* fm);

  static void validateGeoRuleItemWriteGeoRule(GeoRuleItem* geo,
                                              DiagnosticTypes diagType,
                                              DCFactory* factory,
                                              DiagCollector& diag);

  static bool validateGeoRuleItem(const RuleConst::TSIScopeParamType defaultScope,
                                  const FarePath* fp,
                                  const Itin* itin,
                                  const PricingUnit* pu,
                                  const FareMarket* fm,
                                  std::vector<TravelSeg*>::const_iterator& begin,
                                  std::vector<TravelSeg*>::const_iterator& end);

  static bool validateGeoRuleItem(const uint32_t itemNo,
                                  const VendorCode& vendorCode,
                                  const RuleConst::TSIScopeParamType defaultScope,
                                  const bool allowJourneyScopeOverride,
                                  const bool allowPUScopeOverride,
                                  const bool allowFCScopeOverride,
                                  PricingTrx& trx,
                                  const FarePath* fp,
                                  const Itin* itin,
                                  const PricingUnit* pu,
                                  const FareMarket* fm,
                                  const DateTime& ticketingDate,
                                  TravelSegWrapperVector& applTravelSegment,
                                  const bool origCheck,
                                  const bool destCheck,
                                  bool& fltStopCheck,
                                  TSICode& tsi,
                                  LocKey& locKey1,
                                  LocKey& locKey2,
                                  const DiagnosticTypes& callerDiag = DiagnosticNone,
                                  const Indicator chkLogic = LOGIC_AND,
                                  const FareUsage* fareUsage = nullptr);

  static bool getTvlSegBtwAndGeo(PricingTrx& trx,
                                 const uint32_t itemNoBtw,
                                 const uint32_t itemNoAnd,
                                 const VendorCode& vendorCode,
                                 std::vector<TravelSeg*>& tvlSegsRtn,
                                 std::vector<TravelSeg*>* tvlSegsRtn2ndDirection,
                                 const RuleConst::TSIScopeParamType defaultScope,
                                 const FarePath* fp,
                                 const PricingUnit* pu,
                                 const FareMarket* fm,
                                 bool lookForSingleSegments,
                                 const DateTime& ticketingDate,
                                 bool& fltStopCheck,
                                 DiagCollector* callerDiag = nullptr,
                                 bool isCat12Request = false);

  static bool getTvlSegBtwAndGeoForFm(PricingTrx& trx,
                                      const LocKey& locKey1Btw,
                                      const LocKey& locKey1And,
                                      const LocKey& locKey2Btw,
                                      const LocKey& locKey2And,
                                      const VendorCode& vendorCode,
                                      const std::vector<TravelSeg*>& allTvlSegs,
                                      bool fltStopCheck,
                                      bool checkOrigBtw,
                                      bool checkDestBtw,
                                      const GeoTravelType& geoTvlType,
                                      std::vector<TravelSeg*>& tvlSegsRtn,
                                      std::vector<TravelSeg*>* tvlSegsRtn2ndDirection,
                                      bool isCat12Request = false);

  static bool getTvlSegBtwAndGeoDirectConflicted(bool& checkOrig1,
                                                 bool& checkDest1,
                                                 bool& checkOrig2,
                                                 bool& checkDest2);

  static bool findBtwTvlSegs(const RuleUtil::TravelSegWrapperVector& applBtwTvlSegs,
                             const RuleUtil::TravelSegWrapperVector& applAndTvlSegs,
                             const std::vector<TravelSeg*>& allTvlSegs,
                             std::vector<TravelSeg*>& matchedTvlSegs);

  static bool getOrigDestLocFromGeoRuleItem(const uint32_t itemNo,
                                            const VendorCode& vendorCode,
                                            PricingTrx& trx,
                                            bool& checkOrig,
                                            bool& checkDest,
                                            TSICode& tsi,
                                            LocKey& loc1,
                                            LocKey& loc2);

  static bool matchTvlSegsFromTo(PricingTrx& trx,
                                 const LocKey& locKey2Btw,
                                 const LocKey& locKey2And,
                                 const VendorCode& vendorCode,
                                 const std::vector<TravelSeg*>& allTvlSegs,
                                 const bool& fltStopCheck,
                                 std::vector<TravelSeg*>& tvlSegsRtn,
                                 GeoTravelType geoTvlType);

  static bool matchAllTvlSegsFromTo(PricingTrx& trx,
                                    const LocKey& locKey2Btw,
                                    const LocKey& locKey2And,
                                    const VendorCode& vendorCode,
                                    const std::vector<TravelSeg*>& allTvlSegs,
                                    const bool& fltStopCheck,
                                    std::set<TravelSeg*>& tvlSegsRtn,
                                    GeoTravelType geoTvlType);

  static bool isFareValidForRetailerCodeQualiferMatch(PricingTrx& trx, const PaxTypeFare& ptf);
  static bool isCat35FareUsingFRCode(const PaxTypeFare& paxTypeFare, PseudoCityCode& sourcePCC);
  static bool isAslFareUsingFRCode(const PaxTypeFare& paxTypeFare);
  static bool isPricingUnitValidForRetailerCode(PricingUnit& pu);
  static bool isFarePathValidForRetailerCode(PricingTrx& trx, const FarePath& fPath);

  /// Table 994 validation

  static bool validateDateOverrideRuleItem(PricingTrx& trx,
                                           uint32_t itemNo,
                                           const VendorCode& vendor,
                                           const DateTime& travelDate,
                                           const DateTime& ticketingDate,
                                           const DateTime& reservationDate,
                                           DiagCollector* diag = nullptr,
                                           const DiagnosticTypes& callerDiag = DiagnosticNone);

  static bool validateDateOverrideRuleItem(PricingTrx& trx,
                                           bool& isInbound,
                                           uint32_t itemNo,
                                           const VendorCode& vendor,
                                           const DateTime& ticketingDate,
                                           const DateTime& reservationDate,
                                           DiagCollector* diag = nullptr,
                                           const DiagnosticTypes& callerDiag = DiagnosticNone);

  static bool
  matchJointCarrier(PricingTrx& trx, const PaxTypeFare& paxTypeFare, const int itemNumber);
  static bool matchJointCarrier(PricingTrx& trx,
                                const FareMarket& fareMarket,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const int itemNumber);
  /// Table 993 validation

  static bool validateSamePoint(PricingTrx& trx,
                                const VendorCode& vendor,
                                const uint16_t itemNumber,
                                const LocCode& mkt1,
                                const LocCode& mkt2,
                                const DateTime& travelDate,
                                DiagCollector* diag = nullptr,
                                const DiagnosticTypes& callerDiag = DiagnosticNone);

  /// Match up the Fare types between Fare and Rule

  static bool matchFareType(const FareType& ftFromRule, const FareType& ftFromFareC);

  static bool matchGenericFareType(const FareType& ftFromRule, const FareType& ftFromFareC);

  /// Match up the Season Indicators from Fare and Rule

  static bool matchSeasons(Indicator sFromRule, Indicator sFromFareC);

  static bool matchDayOfWeek(Indicator dowFromRule, Indicator dowFromFareC);

  static bool matchFareClass(const char* fareClassRuleCStr, const char* fareClassFareCStr)
  {
    return matchFareClassN(fareClassRuleCStr, fareClassFareCStr);
  }
  static bool matchFootNoteCtrlInfoNew(PricingTrx& trx,
                                       const FootNoteCtrlInfo& fnci,
                                       const PaxTypeFare& paxTypeFare,
                                       bool& isLocationSwapped,
                                       const Footnote& footnote,
                                       const TariffNumber& tariff);

  static bool matchFootNoteCtrlInfo(PricingTrx& trx,
                                    const FootNoteCtrlInfo& fnci,
                                    const PaxTypeFare& paxTypeFare,
                                    bool& isLocationSwapped,
                                    const Footnote& footnote,
                                    const TariffNumber& tariff);

  static bool isDiscount(const uint16_t categoryNumber);

  // two ways to return for same method
  static GeneralFareRuleInfo*
  getGeneralFareRuleInfo(PricingTrx& trx,
                         const PaxTypeFare& paxTypeFare,
                         const uint16_t categoryNumber,
                         bool& isLocationSwapped,
                         const TariffNumber* overrideTcrRuleTariff = nullptr,
                         const RuleNumber* overrideRuleNumber = nullptr);

  // Save record 2 pointer and isLocationSwapped in the vector
  static bool getGeneralFareRuleInfo(PricingTrx& trx,
                                     const PaxTypeFare& paxTypeFare,
                                     const uint16_t categoryNumber,
                                     GeneralFareRuleInfoVec& gfrInfoVec,
                                     const TariffNumber* overrideTcrRuleTariff = nullptr,
                                     const RuleNumber* overrideRuleNumber = nullptr);

  // two ways to return for same method
  static GeneralRuleApp*
  getGeneralRuleApp(PricingTrx& trx, const PaxTypeFare& paxTypeFare, const uint16_t categoryNumber);

  static bool getGeneralRuleApp(PricingTrx& trx,
                                const PaxTypeFare& paxTypeFare,
                                const uint16_t categoryNumber,
                                std::vector<GeneralRuleApp*>& vecGenRuleApp);

  // Save FareByRule record 2 pointer and isLocationSwapped in vector
  static bool getFareByRuleCtrlInfo(PricingTrx& trx,
                                    FareByRuleApp& fbrApp,
                                    FareMarket& fareMarket,
                                    FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                                    DiagManager& diagManager);

  static CombinabilityRuleInfo*
  getCombinabilityRuleInfo(PricingTrx& trx, PaxTypeFare& paxTypeFare, bool& isLocationSwapped);

  static CombinabilityRuleInfo* getCombinabilityRuleInfo(PricingTrx& trx, PaxTypeFare& paxTypeFare);

  static bool matchGeneralFareRuleNew(PricingTrx& trx,
                                      const PaxTypeFare& paxTypeFare,
                                      const GeneralFareRuleInfo& gfri,
                                      bool& isLocationSwapped);

  static bool matchGeneralFareRule(PricingTrx& trx,
                                   const PaxTypeFare& paxTypeFare,
                                   const GeneralFareRuleInfo& gfri,
                                   bool& isLocationSwapped);

 static bool matchOneWayRoundTrip(const Indicator owrtFromRule, const Indicator owrtFromFare);
 static bool matchOWRT(const Indicator owrtFromFareFocusRule, const Indicator owrtFromFare);
 static bool matchFareRouteNumber(Indicator routingAppl,
                                   const RoutingNumber& rnFromRule,
                                   const RoutingNumber& rnFromFare);

  // For footnote validation
  static bool matchLocation(PricingTrx& trx,
                            const LocKey& loc1FromRule,
                            const LocKey& loc2FromRule,
                            const PaxTypeFare& paxTypeFare,
                            bool& isLocationSwapped,
                            const Footnote& footnote,
                            const TariffNumber& tariff);

  // Uses LocCode instead of Loc but functions the same
  static bool matchGeo(PricingTrx& trx,
                       const LocKey& loc1FromRule,
                       const LocKey& loc2FromRule,
                       const LocCode& origin,
                       const LocCode& destination,
                       const VendorCode& vendorCode,
                       GeoTravelType geoTvlType,
                       const bool isRecord1_2_6);

  static bool matchLoc_R1_2_6(PricingTrx& trx,
                              const LocKey& loc1FromRule,
                              const LocKey& loc2FromRule,
                              const PaxTypeFare& paxTypeFare,
                              bool& isLocationSwapped);

  static bool matchLoc(PricingTrx& trx,
                       const LocKey& loc1FromRule,
                       const LocKey& loc2FromRule,
                       const PaxTypeFare& paxTypeFare,
                       bool& isLocationSwapped);

  static bool matchGeo(PricingTrx& trx,
                       const LocKey& loc1FromRule,
                       const LocKey& loc2FromRule,
                       const Loc& origin,
                       const Loc& destination,
                       const VendorCode& vendorCode,
                       GeoTravelType geoTvlType);

  static bool matchGeo(PricingTrx& trx,
                       const LocKey& loc1FromRule,
                       const LocKey& loc2FromRule,
                       const LocCode& origin,
                       const LocCode& destination,
                       const Loc& originLoc,
                       const Loc& destinationLoc,
                       const VendorCode& vendorCode,
                       GeoTravelType geoTvlType);

  static bool matchLocation(PricingTrx& trx,
                            const LocKey& loc1FromRule,
                            const LocCode& loc1ZoneFromRule,
                            const LocKey& loc2FromRule,
                            const LocCode& loc2ZoneFromRule,
                            const VendorCode& vendorCode,
                            const FareMarket& fareMarket,
                            bool& isLocationSwapped,
                            const CarrierCode& carrierCode);

  static bool matchGeo(PricingTrx& trx,
                       const LocKey& loc1FromRule,
                       const LocCode& loc1ZoneFromRule,
                       const LocKey& loc2FromRule,
                       const LocCode& loc2ZoneFromRule,
                       const VendorCode& vendorCode,
                       const Loc& origin,
                       const Loc& destination,
                       GeoTravelType geoTvlType,
                       const CarrierCode& carrierCode);

  static bool table995WhollyWithin(PricingTrx& trx,
                                   const uint32_t itemNo1,
                                   const uint32_t itemNo2,
                                   const VendorCode& vendorCode);

  static bool matchFareFootnote(const Footnote& ft1FromRule,
                                const Footnote& ft2FromRule,
                                const Footnote& ft1FromFare,
                                const Footnote& ft2FromFare);

  /// get all types of surcharges for the current Journey (FarePath),
  /// accumulate plusUp surcharges for the all Fare usages.

  static bool getSurcharges(PricingTrx& trx, FarePath& farePath);

  static void getFootnotes(const PaxTypeFare& paxTypeFare,
                           std::vector<Footnote>& footnotes,
                           std::vector<TariffNumber>& fareTariffs);

  static void getFirstValidTvlDT(DateTime& rtnDT,
                                 const TravelSegWrapperVector& tvlSegVector,
                                 const bool orig,
                                 NoPNRPricingTrx* noPNRTrx = nullptr);

  static void getFirstValidTvlDT(DateTime& startDT,
                                 DateTime& endDT,
                                 const TravelSegWrapperVector& tvlSegVector,
                                 const bool orig,
                                 NoPNRPricingTrx* noPNRTrx = nullptr);

  static void getLastValidTvlDT(DateTime& rtnDT,
                                const TravelSegWrapperVector& tvlSegVector,
                                const bool orig,
                                NoPNRPricingTrx* noPNRTrx = nullptr);

  static void getFirstValidTvlDT(DateTime& rtnDT,
                                 const std::vector<TravelSeg*>& travelSegs,
                                 const bool orig,
                                 NoPNRPricingTrx* noPNRTrx = nullptr);

  static void getFirstValidTvlDT(DateTime& startDT,
                                 DateTime& endDT,
                                 const std::vector<TravelSeg*>& travelSegs,
                                 const bool orig,
                                 NoPNRPricingTrx* noPNRTrx = nullptr);

  static void getLastValidTvlDT(DateTime& rtnDT,
                                const std::vector<TravelSeg*>& travelSegs,
                                const bool orig,
                                NoPNRPricingTrx* noPNRTrx = nullptr);

  static bool getTvlDateForTbl994Validation(DateTime& travelDate,
                                            const uint16_t category,
                                            const PaxTypeFare& paxTypeFare,
                                            const RuleItemInfo* ruleItemInfo,
                                            const PricingUnit* pricingUnit,
                                            NoPNRPricingTrx* noPNRTrx = nullptr);

  static bool usingRebookedSeg(const PaxTypeFare& paxTypeFare);

  static void
  getLatestBookingDate(PricingTrx& trx, DateTime& rtnDate, const PaxTypeFare& paxTypeFare);

  static void getLatestBookingDate(DateTime& rtnDate, const std::vector<TravelSeg*>& tvlSegs);

  static void diagGeoTblItem(const uint32_t itemNo,
                             const tse::VendorCode& vendorCode,
                             tse::PricingTrx& trx,
                             DiagCollector& diag);

  static void diagLocKey(PricingTrx& trx, const LocKey& lockKey, DiagCollector& diag);

  static bool isInDirection(PricingTrx& trx,
                            const TravelSeg& tvlSeg,
                            const std::vector<TravelSeg*>& tvlSegs,
                            const LocKey& locFrom,
                            const LocKey& locTo);

  static bool isWithinLoc(PricingTrx& trx, const TravelSeg& tvlSeg, const LocKey& loc1);

  static void
  displayCPWarning(std::ostringstream& output, const uint32_t failedStat, bool asCatNum = false);

  static void processCurrencyAdjustment(PricingTrx& trx, FarePath& farePath);

  static bool isSalePointAppl(PricingTrx& trx);

  static void validateCurrAdjPU(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit);

  static bool isTktCarrierAppl(PricingTrx& trx, FarePath& farePath);

  static bool isNigeriaIntl(FareUsage& fu);

  static const PaxTypeFare* selectAdjustedFare(PricingTrx& trx,
                                               FarePath& farePath,
                                               PricingUnit& pricingUnit,
                                               FareUsage& fu,
                                               DateTime travelDate = DateTime::emptyDate());

  static const RuleItemInfo* getRuleItemInfo(PricingTrx& trx,
                                             const CategoryRuleInfo* const rule,
                                             const CategoryRuleItemInfo* const item,
                                             const DateTime& applDate = DateTime::emptyDate());

  static bool useSameCarrier(const std::vector<TravelSeg*>& tvlSegs);
  static bool useCxrInCxrFltTbl(const std::vector<TravelSeg*>& tvlSegs,
                                const VendorCode& vendor,
                                const int carrierFltTblItemNo,
                                const DateTime& ticketDate);

  static bool useSameTariffRule(const std::vector<const PaxTypeFare*>& paxTypeFares,
                                const bool needSameRule = true);

  static bool matchCreateOrExpireDate(const DateTime& tktDate,
                                      const DateTime& createDate,
                                      const DateTime& expireDate);

  static DateTime addPeriodToDate(const DateTime& travelDate, const PeriodOfStay& stayPeriod);

  static bool
  validateDutyFunctionCode(const Agent* agent, const AgencyDutyCode& dbDutyFunctionCode);

  static void getAllPTFs(std::vector<const PaxTypeFare*>& ptfv, const FarePath& farePath);

  static bool checkDutyFunctionCode(const Agent* agent, const AgencyDutyCode& dbDutyFunctionCode);

  static bool determineRuleChecks(const uint16_t& categorySequence,
                                  const FareByRuleItemInfo& fbrItemInfo,
                                  bool& checkFare,
                                  bool& checkBaseFare);

  static bool isVendorPCC(const VendorCode& vendor, const PricingTrx& trx);

  static bool
  compareRoutings(const RoutingNumber& routingFromRule, const RoutingNumber& routingFromFare);

  static bool validateMatchExpression(const FareClassCodeC& fareClassMatchRule);

  static bool matchFareClassExpression(const char* fareClassRule, const char* fareBasis);

  static void getPrimeRBDrec1(const PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);
  static void getPrimeRBD(const PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);
  static bool matchPrimeRBD(const FareFocusBookingCodeInfo* ffbci,
                            const std::vector<BookingCode>& bkgCodes);
  static bool matchCat35Type(const Indicator displayTypeRule,
                             const Indicator displayTypeFare);
  static bool matchBookingCode(PricingTrx& trx,
                               uint64_t bookingCodeItemNo,
                               const PaxTypeFare& paxTypeFare,
                               DateTime adjustedTicketDate);

  static bool isInLoc(PricingTrx& trx,
                      const LocCode& validatingLoc,
                      LocTypeCode ruleLocType,
                      const LocCode& ruleLoc,
                      const PaxTypeFare& paxTypeFare);

  static bool isInFFUserGroup(PricingTrx& trx,
                              const LocCode& validatingLoc,
                              LocTypeCode ruleLocType,
                              const LocCode& ruleLoc,
                              const PaxTypeFare& paxTypeFare,
                              GeoTravelType geoType);

  static std::string getFareBasis(PricingTrx& trx, const PaxTypeFare* ptf);

  static void setAdjustedTicketDateTime(PricingTrx& trx, DateTime& adjustedTicketDate);

  static short getTimeDiffAgentPccHDQ(PricingTrx& trx);

  static MoneyAmount convertCurrency(PricingTrx& trx,
                                     const FarePath& fp,
                                     MoneyAmount sourceAmount,
                                     const CurrencyCode& sourceCurrency,
                                     const CurrencyCode& targetCurrency,
                                     bool doNonIataRounding);

  static bool isFrrNetCustomer(PricingTrx& trx);

private:

  static void recreateRefundPlusUps(PricingTrx& trx, FarePath& fPath);

  static MoneyAmount addNonTktSurcharges(FareUsage& fU, bool& isTktSurchargeFU);

  static void exchangeSurchargeOverride(ExchangePricingTrx& exchangeTrx, FareUsage& fu);

  static void
  getExcSTOSurchargeOverride(ExchangePricingTrx& excTrx, FarePath& farePath, FareUsage& fu);

  static void getFPPlusUpOverride(ExchangePricingTrx& excTrx, FarePath& farePath);

  static void getFUPlusUpOverride(ExchangePricingTrx& excTrx, FareUsage& fu, FarePath& farePath);

  static void getGroupMaxAmtForFU(const PricingTrx& trx,
                                  std::vector<SurchargeData*>::iterator sI,
                                  std::vector<SurchargeData*>::iterator sE,
                                  MoneyAmount& amt,
                                  const Indicator sType,
                                  const LocKey& lc1,
                                  const LocKey& lc2,
                                  const LocKey& lc3,
                                  const LocKey& lc4,
                                  const LocKey& lc5,
                                  const LocKey& lc6,
                                  const TSICode& tsi,
                                  const TSICode& tsiBtw,
                                  const TSICode& tsiAnd,
                                  const Indicator tPortion);

  //       static SurchargeData* getGroupMaxAmtForPU(std::vector<FareUsage*>::iterator fUI,
  static FareUsage* getGroupMaxAmtForPU(const PricingTrx& trx,
                                        std::vector<FareUsage*>::iterator fUI,
                                        std::vector<FareUsage*>::iterator fUEnd,
                                        MoneyAmount& amt,
                                        const CarrierCode& cc,
                                        const Indicator sType,
                                        const LocKey& lc1,
                                        const LocKey& lc2,
                                        const LocKey& lc3,
                                        const LocKey& lc4,
                                        const LocKey& lc5,
                                        const LocKey& lc6,
                                        const TSICode& tsi,
                                        const TSICode& tsiBtw,
                                        const TSICode& tsiAnd,
                                        const bool inBound,
                                        const Indicator tPortion);

  //        static SurchargeData* getGroupMaxAmtForFP(std::vector<PricingUnit*>::iterator puIt,
  static FareUsage* getGroupMaxAmtForFP(const PricingTrx& trx,
                                        std::vector<PricingUnit*>::iterator puIt,
                                        std::vector<PricingUnit*>::iterator puIEnd,
                                        MoneyAmount& amt,
                                        const CarrierCode& cc,
                                        const Indicator sType,
                                        const LocKey& lc1,
                                        const LocKey& lc2,
                                        const LocKey& lc3,
                                        const LocKey& lc4,
                                        const LocKey& lc5,
                                        const LocKey& lc6,
                                        const TSICode& tsi,
                                        const TSICode& tsiBtw,
                                        const TSICode& tsiAnd,
                                        const bool inBound,
                                        const Indicator tPortion);

  static void setupSelectedGroupForFu(FareUsage& fUsg,
                                      const CarrierCode& cc,
                                      const Indicator sType,
                                      const LocKey& lc1,
                                      const LocKey& lc2,
                                      const LocKey& lc3,
                                      const LocKey& lc4,
                                      const LocKey& lc5,
                                      const LocKey& lc6,
                                      const TSICode& tsi,
                                      const TSICode& tsiBtw,
                                      const TSICode& tsiAnd,
                                      const Indicator tPortion);

  static bool checkT994Dates(const DateOverrideRuleItem& dorItem,
                             const DateTime& earliestDate,
                             const DateTime& latestDate,
                             const DateTime& ticketingDate,
                             const DateTime& reservationDate);

  static void printT994Dates(const DateOverrideRuleItem& dorItem, DiagCollector* diag);
  static void printT994Dates(const DateOverrideRuleItem& dorItem,
                             DiagCollector* diag,
                             const DiagnosticTypes& callerDiag);

  static bool checkTravelSegLoc(PricingTrx& trx,
                                const GeoRuleItem* geoRuleItem,
                                const Loc& loc,
                                const VendorCode& vendorCode,
                                GeoTravelType geoTvlType,
                                LocUtil::ApplicationType locAppl = LocUtil::FLIGHT_RULE);

  static bool checkTravelSeg(PricingTrx& trx,
                             const GeoRuleItem* geoRuleItem,
                             const TravelSeg* travelSeg,
                             bool& origCheck,
                             bool& destCheck,
                             bool& fltStopCheck,
                             const Indicator chkLogic,
                             const VendorCode& vendorCode,
                             GeoTravelType geoTvlType);

  static bool isFromCat25BaseFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare);

  static bool useCxrInCxrFltTbl(const AirSeg& airSeg, const CarrierFlight& table986);

  static void collectAddonFootnote(const Footnote& footnote,
                                   const TariffNumber& tariff,
                                   bool isVendorATPCO,
                                   std::vector<Footnote>& footnotes,
                                   std::vector<TariffNumber>& fareTariffs);

  static std::vector<TravelSeg*> getFilteredSegsInOrder(const std::vector<TravelSeg*>& allTvlSegs,
                                                        const std::set<TravelSeg*>& filteredSegSet);

  static Logger _logger;
};
}
