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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/Diag372Collector.h"

#include <vector>

namespace tse
{
class PricingTrx;
class Itin;
class TravelSeg;
class FarePath;
class FareMarket;
class FareUsage;
class PricingUnit;
class LimitationCmn;
class LimitationJrny;
class LimitationFare;
class Loc;
class DiagCollector;
class PaxTypeFare;
class PaxType;
class LocKey;
class DataHandle;
class SurfaceSectorExemptionInfo;

/**
 * @class LimitationOnIndirectTravel
 *
 * This class implements Limitations on Indirect Travel logic.
 *
 */

class LimitationOnIndirectTravel final
{
  friend class LimitationOnIndirectTravelTest;

public:
  static const char BLANK;
  static constexpr int MAX_DEPART = 9;
  static constexpr int MAX_ARRIVAL = 9;
  static constexpr int MAX_RETRANSIT = 9;
  static constexpr int MAX_STOPOVER = 9;
  static constexpr int MAX_DOMESTIC = 9;
  static const std::string REPLACE_TOKEN;
  static const std::string CONFIRMED_STATUS;

  enum LimitationAppl
  {
    JOURNEY = 'J',
    PU = 'P',
    FC = 'F'
  };

  enum GeneralRetransitLocType
  {
    COUNTRY_OF_J_ORIGIN = '1',
    COUNTRY_OF_FC_ORIGIN = '2',
    COUNTRY_OF_FC_DESTIN = '3',
    COUNTRY_OF_PAYMENT = '4',
    INTERMEDIATE_POINT_WITHIN_FC = '5'
  };

  enum Application
  {
    YES = 'Y',
    NO = 'N'
  };

  enum WhollyWithinAppl
  {
    MUST_BE_WHOLLY_WITHIN = '1',
    MUST_NOT_BE_WHOLLY_WITHIN = '2'
  };

  enum DirectionInd
  {
    BETWEEN = 'B',
    FROM = 'F',
    WITHIN = 'W'
  };

  enum FareComponentAppl
  {
    OUTBOUND = 'O',
    INBOUND = 'I',
    BOTH = 'B'
  };

  LimitationOnIndirectTravel(PricingTrx& trx, const Itin& itin) : _trx(trx), _itin(itin) {}

  bool retransitFareComponentBoardOffPoint(const FareMarket& fareMarket);
  void validateJourney();
  bool validateIntlSurfaceTravel(FarePath& farePath);
  bool validatePricingUnit(const PricingUnit& pu, DiagCollector& diag682);
  bool validateFareComponent(FareMarket& fareMarket);
  bool validateFareComponentAfterHip(const FareUsage& fareUsage);
  bool validateFarePath(FarePath& farePath, DiagCollector& diag);

private:
  bool validateJourneyItem(const LimitationJrny& lim, DiagCollector* diag);
  bool
  validatePricingUnitItem(const PricingUnit& pu, const LimitationFare& lim, DiagCollector* diag);
  bool validateFareComponentItem(FareMarket& fareMarket,
                                 const LimitationFare& lim,
                                 DiagCollector* diag,
                                 const FareUsage* fareUsage = nullptr);
  bool
  validateNotViaHipItem(const FareUsage& fareUsage, const LimitationFare& lim, DiagCollector* diag);

  bool isExemptedByTicketingCarrier(const LimitationCmn& lim);
  bool isMustWhollyWithinOrNot(const LimitationCmn& lim);

  bool isWithinLoc(const std::vector<TravelSeg*>& travelSegs, const LocKey& loc);
  bool isViaLoc(const std::vector<TravelSeg*>& travelSegs, const LocKey& loc);
  bool isToFromLoc(const Loc& loc, const LimitationCmn& lim);
  bool
  isToFromLoc(const TravelSeg& travelSeg, const TravelSeg& nextTravelSeg, const LimitationCmn& lim);
  bool
  isBetweenLocs(const std::vector<TravelSeg*>& travelSegs, const LocKey& loc1, const LocKey& loc2);
  bool isFromLoc1ToLoc2(const std::vector<TravelSeg*>& travelSegs,
                        const LocKey& loc1,
                        const LocKey& loc2);
  bool isFromLoc1ToLoc2(const Loc& orig,
                        const Loc& dest,
                        const LocKey& loc1,
                        const LocKey& loc2,
                        const DateTime& ticketingDT);

  bool isJourneyOriginAppl(const LimitationCmn& lim);
  bool isPricingUnitOriginAppl(const PricingUnit& pu, const LimitationCmn& lim);

  bool isFareType(const PaxTypeFare& paxTypeFare, const LimitationCmn& lim);
  bool isSameFareTypeDesig(const FareType& fareType1, const FareType& fareType2);

  bool isSalesLoc(const LimitationCmn& lim);
  bool isTicketLoc(const LimitationCmn& lim);
  bool isSameNation(const Loc& loc1, const Loc& loc2);

  bool checkGeneralRetransit(LimitationAppl limAppl,
                             const std::vector<TravelSeg*>& travelSegs,
                             bool reverseDirection,
                             const LimitationCmn& lim,
                             DiagCollector* diag,
                             const FareMarket* fareMarket = nullptr);
  bool checkSpecificRetransit(const std::vector<TravelSeg*>& travelSegs,
                              const LimitationCmn& lim,
                              DiagCollector* diag);
  bool checkMaxIntlDepartArrivalAtLoc(const std::vector<TravelSeg*>& travelSegs,
                                      const LocTypeCode& locType,
                                      const LocCode& locCode,
                                      const LimitationCmn& lim,
                                      DiagCollector* diag);
  bool checkMaxRetransitAtLoc(const std::vector<TravelSeg*>& travelSegs,
                              const LocTypeCode& locType,
                              const LocCode& locCode,
                              const LimitationCmn& lim,
                              DiagCollector* diag);
  bool checkMaxRetransitAtIntermLoc(const std::vector<TravelSeg*>& travelSegs,
                                    const CarrierCode& govCxr,
                                    const LimitationCmn& lim,
                                    DiagCollector* diag);

  bool checkGeneralStopOver(LimitationAppl limAppl,
                            const std::vector<TravelSeg*>& travelSegs,
                            bool reverseDirection,
                            const LimitationCmn& lim,
                            DiagCollector* diag,
                            bool noSideTrip,
                            const FareMarket* fareMarket = nullptr);
  bool checkSpecificStopOver(const std::vector<TravelSeg*>& travelSegs,
                             const LimitationCmn& lim,
                             DiagCollector* diag,
                             bool noSideTrip);
  bool checkMaxStopOverAtLoc(const std::vector<TravelSeg*>& travelSegs,
                             const LocTypeCode& locType,
                             const LocCode& locCode,
                             const LimitationCmn& lim,
                             DiagCollector* diag,
                             bool noSideTrip);
  bool checkMaxStopOver(const std::vector<TravelSeg*>& travelSegs,
                        const LimitationCmn& lim,
                        DiagCollector* diag,
                        bool noSideTrip);
  bool checkMaxStopOverAtIntermLoc(const std::vector<TravelSeg*>& travelSegs,
                                   const CarrierCode& govCxr,
                                   const LimitationCmn& lim,
                                   DiagCollector* diag,
                                   bool noSideTrip);

  bool isFareComponentInternational(const FareMarket& fareMarket);

  const Loc* origin(const std::vector<TravelSeg*>& travelSegs);
  const Loc* destination(const std::vector<TravelSeg*>& travelSegs);

  bool matchGovCxrAppl(const FareMarket& fareMarket, const LimitationFare& lim);
  bool matchFareComponentDirection(const FareMarket& fareMarket,
                                   const LimitationFare& lim,
                                   const FareUsage* fareUsage = nullptr);
  bool matchGlobalDirection(const FareMarket& fareMarket, const LimitationFare& lim);
  bool checkConfirmedStatus(const std::vector<TravelSeg*>& travelSegs,
                            const LimitationFare& lim,
                            DiagCollector* diag);
  bool checkNotViaHip(const FareUsage& fareUsage, const LimitationFare& lim, DiagCollector* diag);
  bool notViaRouting(const PaxTypeFare& paxTypeFare, const LimitationFare& lim);
  bool checkMaxNumDomSeg(const std::vector<TravelSeg*>& travelSegs,
                         const LimitationFare& lim,
                         DiagCollector* diag,
                         FareMarket* fm = nullptr);
  bool notViaIntermediateLoc(const std::vector<TravelSeg*>& travelSegs, const LimitationFare& lim);

  bool matchAllViaCxr(const FareMarket& fareMarket, const LimitationFare& lim, DiagCollector* diag);
  bool allViaGovCxr(const std::vector<TravelSeg*>& travelSegs, const CarrierCode& govCxr);
  bool matchRetransitViaGovCxr(const FareMarket& fareMarket,
                               const LimitationFare& lim,
                               DiagCollector* diag);
  void getRetransits(const std::vector<TravelSeg*>& travelSegs,
                     const CarrierCode& govCxr,
                     std::map<LocCode, int>& retransits);
  bool retransitViaGovCxr(const std::vector<TravelSeg*>& travelSegs,
                          const CarrierCode& govCxr,
                          const LocCode& retransitCity,
                          int numRetransit);

  void retrieveFCLimitation();
  void retrievePULimitation();

  void displayLimitation(const LimitationCmn* lim, DiagCollector& diag);

  void displayFare(const FareUsage& fareUsage, DiagCollector& diag);
  void
  displayRetransitErrMsg(const NationCode& nation, const LimitationCmn& lim, DiagCollector* diag);

  bool find1stIntlSurface(const Itin& itin,
                          const std::vector<PricingUnit*>& pus,
                          LocCode& surfBoardPoint,
                          LocCode& surfOffPoint,
                          int& endSegOrder,
                          DiagCollector* diag);
  bool hasTpmSurfExempt(const LocCode& city1, const LocCode& city2);
  uint32_t getMileage(TravelSeg* surfaceSeg, DiagCollector* diag);
  bool isStopOver(const TravelSeg* travelSeg, const TravelSeg* nextTravelSeg = nullptr);

  void displayRetransitFareComponentBoardOffPointFailed(const FareMarket& fareMarket,
                                                        bool retransitBoard);

  void displayTravelSegs(const std::vector<TravelSeg*>& travelSegVec,
                         DiagCollector& diag,
                         bool noSideTrip);

  void originPos(const std::vector<TravelSeg*>& travelSegs,
                 std::vector<TravelSeg*>::const_iterator& origPos);
  void destinationPos(const std::vector<TravelSeg*>& travelSegs,
                      std::vector<TravelSeg*>::const_iterator& destPos);

  const LocCode boardMultiCity(DataHandle& dataHandle,
                               const TravelSeg& travelSeg,
                               const GeoTravelType& itinTravelType,
                               const CarrierCode& govCxr);

  const LocCode offMultiCity(DataHandle& dataHandle,
                             const TravelSeg& travelSeg,
                             const GeoTravelType& itinTravelType,
                             const CarrierCode& govCxr);

  void fareMarketOriginPos(const FareMarket& fareMarket,
                           std::vector<TravelSeg*>::const_iterator& origPos);
  void fareMarketDestinationPos(const FareMarket& fareMarket,
                                std::vector<TravelSeg*>::const_iterator& destPos);

  virtual const std::vector<SurfaceSectorExemptionInfo*>& getSurfaceSectorExemptionInfo();
  virtual const std::vector<SurfaceSectorExemptionInfo*>&
                                         getSurfaceSectorExemptionInfo(const CarrierCode& cxr);

  int ignoreIntlSurfaceTravelRestrictionVC(FarePath& farePath,
                                           const LocCode locCode1,
                                           const LocCode locCode2);

  int ignoreIntlSurfaceTravelRestriction(FarePath& farePath,
                                         const LocCode locCode1,
                                         const LocCode locCode2,
                                         const CarrierCode& cxr);

private:
  bool validateCRSOnIndirectTravel(const SurfaceSectorExemptionInfo* info, const std::string& crs);
  bool validateLocOnIndirectTravel(Indicator locException,
                                   LocTypeCode locTypeCode,
                                   const LocCode& locCode,
                                   const LocCode& city);
  bool validatePOSOnIndirectTravel(const SurfaceSectorExemptionInfo* info, const LocCode& loc);
  bool validatePTCOnIndirectTravel(const SurfaceSectorExemptionInfo* info);
  bool
  validateOperCxrOnIndirectTravel(const std::set<CarrierCode>& carriers, const Indicator exception);
  bool
  validateMktgCxrOnIndirectTravel(const std::set<CarrierCode>& carriers, const Indicator exception);
  DiagCollector* get370() const;
  void end370(DiagCollector& diag) const;

  std::string getCrs();
  void setDiag372InRequest();
  bool defineDiagCxrRequested(CarrierCode& cxr, bool &diagCxrFound);
  bool defineDiagSQinRequest(const CarrierCode& cxr,
                             Diag372Collector::DiagStream& diagStr,
                             const int& exemptionsSize, bool seqFound);
  void validateSurfaceSectorExemption(const SurfaceSectorExemptionInfo* info,
                                      const LocCode locCode1,
                                      const LocCode locCode2,
                                      const std::string& crs,
                                      bool& validationResult,
                                      std::string& failedOn);
  void displayValitionResult(Diag372Collector::DiagStream& diagStr,
                             const bool& validationResult,
                             const std::string& failedOn);
  void prepareDiag372OutPut(FarePath& fp,
                            std::string& crs,
                            Diag372Collector::DiagStream& diagStr);

  bool checkDiagReqCxr(bool a);
  bool anyCxrPass(FarePath& fp, const std::vector<CarrierCode>& carriers);
  bool displayFarePath(bool a) { return _displayFarePath = a;}
  const bool displayFarePath() const { return _displayFarePath;}
  void inline  initializeAttrib() {
                _diagSeqNumber = -1;
                _valCxrResult  = -1;
                _diagSQFail = false;  }

  PricingTrx& _trx;
  const Itin& _itin;
  DateTime _travelCommenceDT = _itin.travelDate();
  bool _diagEnabled = _trx.diagnostic().isActive();
  bool _displayFarePath = false;
  bool _diagSQFail = false;
  bool _diag = false;
  int _diagSeqNumber = -1;
  int _valCxrResult = -1;
  boost::mutex _mutex; // remove _mutex together with overusedMutexInLimitationOnIndirectTravel
  std::vector<const LimitationFare*> _fcApplyItems;
  std::vector<const LimitationFare*> _puApplyItems;
};
}

