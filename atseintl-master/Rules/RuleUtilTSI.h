//----------------------------------------------------------------------------
//
//  File:        RuleUtilTSI.h
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
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "DBAccess/TSIInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/RuleConst.h"

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
};

class RuleUtilGeoTest;
class RuleUtilTest;

namespace tse
{
class LocKey;
class PricingTrx;
class FarePath;
class FareUsage;
class PricingUnit;
class FareMarket;
class TravelSeg;
class Itin;
class DataHandle;
class Loc;
class DiagCollector;
class DCFactory;
class TSIGateway;

class RuleUtilTSI
{
  friend class RuleUtilTSIMock;
  friend class RuleUtilTSITest_Base;
  friend class RuleUtilTSITest2;
  friend class ::RuleUtilTest;
  friend class ::RuleUtilGeoTest;
  friend class RuleUtilTest_checkLoopMatch;
  friend class RuleUtilTSITest;

protected:
  RuleUtilTSI() {} // only static class members, hide constructor

public:
  /** @class TravelSegWrapper
  *
  * @brief Wraps travel segments with transient data which
  * is the result of processing a table 995 or TSI.
  *
  * This is an implementation of the Decorator pattern.
  *
  */
  class TravelSegWrapper final
  {
  public:
    TravelSeg* travelSeg() const { return _travelSeg; }
    TravelSeg*& travelSeg() { return _travelSeg; }

    const bool origMatch() const { return _origMatch; }
    bool& origMatch() { return _origMatch; }

    const bool destMatch() const { return _destMatch; }
    bool& destMatch() { return _destMatch; }

  private:
    TravelSeg* _travelSeg = nullptr;
    bool _origMatch = false;
    bool _destMatch = false;
  };
  typedef std::vector<const TravelSegWrapper*> TravelSegWrapperVector;

  typedef TravelSegWrapperVector::iterator TravelSegWrapperVectorI;
  typedef TravelSegWrapperVector::const_iterator TravelSegWrapperVectorCI;

  /// Identify the travel segments in a Journey (FarePath),
  /// Pricing Unit or  Fare Component (FareMarket) that a TSI applies to.

  static bool scopeTSIGeo(const TSICode tsi,
                          const LocKey& locKey1,
                          const LocKey& locKey2,
                          const RuleConst::TSIScopeParamType& defaultScope,
                          const bool allowJourneyScopeOverride,
                          const bool allowPUScopeOverride,
                          const bool allowFCScopeOverride,
                          PricingTrx& trx,
                          const FarePath* farePath,
                          const Itin* itin,
                          const PricingUnit* pricingUnit,
                          const FareMarket* fareMarket,
                          const DateTime& ticketingDate,
                          TravelSegWrapperVector& applTravelSegment,
                          const DiagnosticTypes& callerDiag = DiagnosticNone,
                          const VendorCode& vendorCode = "ATP",
                          const FareUsage* fareUsage = nullptr);

  /// Get the scope of a TSI
  static bool getTSIScopeFromGeoRuleItem(const uint32_t itemNo,
                                         const VendorCode& vendorCode,
                                         PricingTrx& trx,
                                         RuleConst::TSIScopeType& scope);

  static bool getTSIScope(const TSICode tsi, PricingTrx& trx, RuleConst::TSIScopeType& scope);

  /// Get the Origin/Destination check of a TSI from the Table 995
  static bool getTSIOrigDestCheckFromGeoRuleItem(const uint32_t itemNo,
                                                 const VendorCode& vendorCode,
                                                 PricingTrx& trx,
                                                 bool& checkOrig,
                                                 bool& checkDest);

  /// Get the Origin/Destination check (Save Type) of a TSI
  static bool getTSIOrigDestCheck(const TSIInfo* tsiInfo, bool& checkOrig, bool& checkDest);
  static bool
  getTSIOrigDestCheck(const TSICode tsi, PricingTrx& trx, bool& checkOrig, bool& checkDest);

protected:
  // helper functions and data:
  // ---------------------------------------------
  //  Private Data Transfer class for TSI methods
  // ---------------------------------------------

  class TSIData
  {
  public:
    TSIData(const TSIInfo& tsiInfo,
            const RuleConst::TSIScopeType& scope,
            const VendorCode& vendorCode,
            const FarePath* farePath,
            const Itin* itin,
            const PricingUnit* pricingUnit,
            const FareMarket* fareMarket)
      : _tsiInfo(tsiInfo),
        _scope(scope),
        _vendorCode(vendorCode),
        _farePath(farePath),
        _itin(itin),
        _pricingUnit(pricingUnit),
        _fareMarket(fareMarket),
        _locType1(LOCTYPE_NONE),
        _locType2(LOCTYPE_NONE)
    {
    }

    const TSIInfo& tsiInfo() const { return _tsiInfo; }
    const FarePath* farePath() const { return _farePath; }
    const Itin* itin() const { return _itin; }
    const PricingUnit* pricingUnit() const { return _pricingUnit; }
    const FareMarket* fareMarket() const { return _fareMarket; }
    const VendorCode& vendorCode() const { return _vendorCode; }

    const RuleConst::TSIScopeType& scope() const { return _scope; }

    const LocTypeCode& locType1() const { return _locType1; }
    LocTypeCode& locType1() { return _locType1; }

    const LocTypeCode& locType2() const { return _locType2; }
    LocTypeCode& locType2() { return _locType2; }

    const LocCode& locCode1() const { return _locCode1; }
    LocCode& locCode1() { return _locCode1; }

    const LocCode& locCode2() const { return _locCode2; }
    LocCode& locCode2() { return _locCode2; }

  private:
    // Hide the default constructor
    TSIData();

    const TSIInfo& _tsiInfo;
    const RuleConst::TSIScopeType _scope;
    const VendorCode _vendorCode;
    const FarePath* _farePath;
    const Itin* _itin;
    const PricingUnit* _pricingUnit;
    const FareMarket* _fareMarket;

    LocTypeCode _locType1;
    LocTypeCode _locType2;
    LocCode _locCode1;
    LocCode _locCode2;
  };

  // ------------------------------------------------
  //  Private TravelSeg markup class for TSI methods
  // ------------------------------------------------

  class TSITravelSegMarkup
  {
  public:
    TSITravelSegMarkup()
      : _travelSeg(nullptr),
        _nextTravelSeg(nullptr),
        _direction(RuleConst::OUTBOUND),
        _globalDirection(GlobalDirection::ZZ),
        _fareBreakAtOrigin(false),
        _fareBreakAtDestination(false),
        _isStopover(false),
        _isTurnAroundPoint(false),
        _destIsTurnAroundPoint(false),
        _isLastTravelSeg(false),
        _departsOrigGateway(false),
        _arrivesOrigGateway(false),
        _departsDestGateway(false),
        _arrivesDestGateway(false),
        _isOverWater(false),
        _isInternational(false),
        _isFurthest(false),
        _isIntlDomTransfer(false),
        _match(RuleConst::TSI_NOT_MATCH),
        _origMatch(false),
        _destMatch(false),
        _origSave(false),
        _destSave(false)
    {
    }

    TravelSeg* travelSeg() const { return _travelSeg; }
    TravelSeg*& travelSeg() { return _travelSeg; }

    TravelSeg* nextTravelSeg() const { return _nextTravelSeg; }
    TravelSeg*& nextTravelSeg() { return _nextTravelSeg; }

    const RuleConst::TSITravelSegDirection& direction() const { return _direction; }
    RuleConst::TSITravelSegDirection& direction() { return _direction; }

    const GlobalDirection& globalDirection() const { return _globalDirection; }
    GlobalDirection& globalDirection() { return _globalDirection; }

    const bool& fareBreakAtOrigin() const { return _fareBreakAtOrigin; }
    bool& fareBreakAtOrigin() { return _fareBreakAtOrigin; }

    const bool& fareBreakAtDestination() const { return _fareBreakAtDestination; }
    bool& fareBreakAtDestination() { return _fareBreakAtDestination; }

    const bool& isStopover() const { return _isStopover; }
    bool& isStopover() { return _isStopover; }

    const bool& isTurnAroundPoint() const { return _isTurnAroundPoint; }
    bool& isTurnAroundPoint() { return _isTurnAroundPoint; }

    const bool& destIsTurnAroundPoint() const { return _destIsTurnAroundPoint; }
    bool& destIsTurnAroundPoint() { return _destIsTurnAroundPoint; }

    const bool& isLastTravelSeg() const { return _isLastTravelSeg; }
    bool& isLastTravelSeg() { return _isLastTravelSeg; }

    const bool& departsOrigGateway() const { return _departsOrigGateway; }
    bool& departsOrigGateway() { return _departsOrigGateway; }

    const bool& arrivesOrigGateway() const { return _arrivesOrigGateway; }
    bool& arrivesOrigGateway() { return _arrivesOrigGateway; }

    const bool& departsDestGateway() const { return _departsDestGateway; }
    bool& departsDestGateway() { return _departsDestGateway; }

    const bool& arrivesDestGateway() const { return _arrivesDestGateway; }
    bool& arrivesDestGateway() { return _arrivesDestGateway; }

    const bool& isOverWater() const { return _isOverWater; }
    bool& isOverWater() { return _isOverWater; }

    const bool& isInternational() const { return _isInternational; }
    bool& isInternational() { return _isInternational; }

    const bool& isFurthest() const { return _isFurthest; }
    bool& isFurthest() { return _isFurthest; }

    const bool& isIntlDomTransfer() const { return _isIntlDomTransfer; }
    bool& isIntlDomTransfer() { return _isIntlDomTransfer; }

    const RuleConst::TSIMatch& match() const { return _match; }
    RuleConst::TSIMatch& match() { return _match; }

    const bool origMatch() const { return _origMatch; }
    bool& origMatch() { return _origMatch; }

    const bool destMatch() const { return _destMatch; }
    bool& destMatch() { return _destMatch; }

    const std::string& noMatchReason() const { return _noMatchReason; }
    std::string& noMatchReason() { return _noMatchReason; }

    const bool origSave() const { return _origSave; }
    bool& origSave() { return _origSave; }

    const bool destSave() const { return _destSave; }
    bool& destSave() { return _destSave; }

  private:
    TravelSeg* _travelSeg;
    TravelSeg* _nextTravelSeg;
    RuleConst::TSITravelSegDirection _direction;
    GlobalDirection _globalDirection;
    bool _fareBreakAtOrigin;
    bool _fareBreakAtDestination;
    bool _isStopover;
    bool _isTurnAroundPoint;
    bool _destIsTurnAroundPoint;
    bool _isLastTravelSeg;
    bool _departsOrigGateway;
    bool _arrivesOrigGateway;
    bool _departsDestGateway;
    bool _arrivesDestGateway;
    bool _isOverWater;
    bool _isInternational;
    bool _isFurthest;
    bool _isIntlDomTransfer;
    RuleConst::TSIMatch _match;
    bool _origMatch;
    bool _destMatch;
    bool _origSave;
    bool _destSave;
    std::string _noMatchReason;
  };

  // -----------------------------------------------------
  //  Private container class for TSI methods.
  //  This class aggregates a deque and a map in order to
  //   combine features of both standard STL containers.
  //  This class provides a convenient way to store a
  //   set of TravelSegMarkup objects preserving the
  //   original order and also providing an efficient
  //   method of finding a specific object using a
  //   TravelSeg pointer as the key.
  //  This is essentially a Deque with a secondary key.
  //
  //  NOTE: It is NOT safe to share an instance of this
  //         container between multiple threads.
  // -----------------------------------------------------
  class TSITravelSegMarkupContainer
  {
  public:
    TSITravelSegMarkupContainer() {}
    ~TSITravelSegMarkupContainer() {}

    typedef std::deque<TSITravelSegMarkup>::size_type size_type;
    typedef std::deque<TSITravelSegMarkup>::reference reference;
    typedef std::deque<TSITravelSegMarkup>::iterator iterator;
    typedef std::deque<TSITravelSegMarkup>::reverse_iterator reverse_iterator;

    size_type size() const { return _tsmDeque.size(); }
    bool empty() const { return _tsmDeque.empty(); }

    reference front() { return _tsmDeque.front(); }
    reference back() { return _tsmDeque.back(); }

    iterator begin() { return _tsmDeque.begin(); }
    iterator end() { return _tsmDeque.end(); }

    reverse_iterator rbegin() { return _tsmDeque.rbegin(); }
    reverse_iterator rend() { return _tsmDeque.rend(); }

    // The travelSeg member of tsm must be set
    //  prior to calling this method!!
    void push_back(const TSITravelSegMarkup& tsm)
    {
      _tsmDeque.push_back(tsm);

      VecMap<const TravelSeg*, int16_t>::value_type value(
          tsm.travelSeg(), static_cast<int16_t>(_tsmDeque.size() - 1));
      _tsMap.insert(value);
    }

    // The travelSeg member of tsm must be set
    //  prior to calling this method!!
    void push_front(const TSITravelSegMarkup& tsm)
    {
      _tsmDeque.push_front(tsm);

      VecMap<const TravelSeg*, int16_t>::value_type value(tsm.travelSeg(), 0);
      _tsMap.insert(value);

      // Since we pushed to the front of the deque we have
      //  to update the map with the new indexes.
      if (size() > 1)
      {
        for (size_type ctr = 1; ctr != size(); ++ctr)
        {
          _tsMap[_tsmDeque[ctr].travelSeg()] = ctr;
        }
      }
    }

    iterator find(const TravelSeg* ts)
    {
      if (empty())
      {
        return end();
      }

      VecMap<const TravelSeg*, size_type>::iterator i = _tsMap.find(ts); // O(log N) time complexity
      if (i == _tsMap.end())
      {
        return end();
      }
      size_type index = (*i).second;
      iterator iter = _tsmDeque.begin();
      iter += static_cast<int32_t>(index); // constant time complexity
      return iter;
    }

  private:
    // This deque contains the TravelSegMarkup objects and
    //  preserves the original order of the travel segments
    std::deque<TSITravelSegMarkup> _tsmDeque;

    // This map is used to find a specific object in the tsmDeque
    //  using a TravelSeg pointer as the key.
    VecMap<const TravelSeg*, size_type> _tsMap;
  };

  typedef TSITravelSegMarkupContainer::iterator TSITravelSegMarkupI;
  typedef TSITravelSegMarkupContainer::reverse_iterator TSITravelSegMarkupRI;

  static bool getReversedTravelSegs(DataHandle& dataHandle,
                                    std::vector<TravelSeg*>& newTvlSegs,
                                    const std::vector<TravelSeg*>& oldTvlSegs);

  static TravelSeg* reverseTravel(DataHandle& dataHandle,
                                  const std::vector<TravelSeg*>& oldTvlSegVec,
                                  uint16_t orderNum);

  // Gets the GeoType(s) and GeoLocale(s) for processing the TSI
  static bool
  getGeoData(const PricingTrx& trx, TSIData& tsiData, const LocKey& locKey1, const LocKey& locKey2);
  static bool getGeoDataLogErrMsg(const PricingTrx& trx, TSIData& tsiData, const char* msg);
  static void logErrMsg(const PricingTrx& trx, const TSIInfo& tsiInfo, const char* msg);
  static void logErrMsg(const PricingTrx& trx, TSICode tsi, const char* msg);
  static void logDbgMsg(const PricingTrx& trx, const TSIInfo& tsiInfo, const char* msg);
  static void logDbgMsg(const PricingTrx& trx, TSICode tsi, const char* msg);
  static const LocCode getGeoDataGetLoc(const Loc* loc, const LocTypeCode& locType);

  static bool setupTravelSegMarkup(PricingTrx& trx,
                                   const TSIData& tsiData,
                                   const FareUsage* fareUsage,
                                   TSITravelSegMarkupContainer& tsMarkup);

  static bool setupTravelSegMarkupSetFurthest(const RuleConst::TSIScopeType& scope,
                                              const TSIData& tsiData,
                                              bool stopoverExists,
                                              TSITravelSegMarkupContainer& tsMarkup);

  static bool setupTravelSegMarkupSetStopover(const RuleConst::TSIScopeType& scope,
                                              const TSIData& tsiData,
                                              const FareUsage* fareUsage,
                                              bool stopoverExists,
                                              TSITravelSegMarkupContainer& tsMarkup,
                                              const PricingTrx& trx);
  static void setupTravelSegMarkupRtwPostprocess(const tse::PricingTrx& trx,
                                                 const TSIData& tsiData,
                                                 TSITravelSegMarkupContainer& tsiMarkups);

  static TSITravelSegMarkup& setupTravelSegMarkupCreateSegFM(TSITravelSegMarkupContainer& tsMarkup,
                                                             TravelSeg* ts,
                                                             TravelSeg* nextTs,
                                                             GeoTravelType geoTvlType,
                                                             bool loopForward,
                                                             const FareMarket* fm,
                                                             const FareUsage* fareUsage);

  static TSITravelSegMarkup& setupTravelSegMarkupCreateSegPTF(TSITravelSegMarkupContainer& tsMarkup,
                                                              TravelSeg* ts,
                                                              TravelSeg* nextTs,
                                                              GeoTravelType geoTvlType,
                                                              bool loopForward,
                                                              const PaxTypeFare* ptf,
                                                              const FareUsage* fareUsage);

  template <class T>
  static int32_t setupTravelSegMarkupGetStart(bool loopForward, const T& container);
  template <class T>
  static int32_t setupTravelSegMarkupGetEnd(bool loopForward, const T& container);
  static void setupTravelSegMarkupSetDirection(TSITravelSegMarkup& tsm, bool isInbound);

  static bool processTravelSegs(PricingTrx& trx,
                                const TSIData& tsiData,
                                TravelSegWrapperVector& applTravelSeg,
                                TSITravelSegMarkupContainer& tsMarkup,
                                bool locSpecified);
  static bool processTravelSegsSetupMatchCriteria(const PricingTrx& trx,
                                                  const TSIInfo& tsiInfo,
                                                  std::vector<TSIInfo::TSIMatchCriteria>& mc1,
                                                  std::vector<TSIInfo::TSIMatchCriteria>& mc2);
  static void processTravelSegsSetupTSIMatch(const PricingTrx& trx,
                                             const TSIData& tsiData,
                                             const std::vector<TSIInfo::TSIMatchCriteria>& mc1,
                                             const std::vector<TSIInfo::TSIMatchCriteria>& mc2,
                                             TSITravelSegMarkup& tsm,
                                             bool& origMatch,
                                             bool& destMatch,
                                             RuleConst::TSIMatch& currMatch,
                                             RuleConst::TSIMatch& prevMatch);
  static TravelSegWrapper* processTravelSegsAddPrev(PricingTrx& trx,
                                                    TSITravelSegMarkup& prevTsm,
                                                    bool origMatch,
                                                    bool destSave);
  static TravelSegWrapper* processTravelSegsAddCurr(PricingTrx& trx,
                                                    TSITravelSegMarkup& tsm,
                                                    bool origMatch,
                                                    bool destMatch,
                                                    bool origSave,
                                                    bool destSave);
  static TravelSegWrapper* processTravelSegsAddNext(PricingTrx& trx,
                                                    TSITravelSegMarkup& nextTsm,
                                                    bool destMatch,
                                                    bool origSave);
  static bool processTravelSegsForward(PricingTrx& trx,
                                       const TSIData& tsiData,
                                       TravelSegWrapperVector& applTravelSeg,
                                       TSITravelSegMarkupContainer& tsMarkup,
                                       bool locSpecified,
                                       bool checkOrig,
                                       bool checkDest,
                                       bool origSave,
                                       bool destSave,
                                       bool checkFareBreaksOnly);
  static bool processTravelSegsBackward(PricingTrx& trx,
                                        const TSIData& tsiData,
                                        TravelSegWrapperVector& applTravelSeg,
                                        TSITravelSegMarkupContainer& tsMarkup,
                                        bool locSpecified,
                                        bool checkOrig,
                                        bool checkDest,
                                        bool origSave,
                                        bool destSave,
                                        bool checkFareBreaksOnly);

  static bool processTravelSegsForward(PricingTrx& trx,
                                       const TSIData& tsiData,
                                       TravelSegWrapperVector& applTravelSeg,
                                       TSITravelSegMarkupContainer& tsMarkup,
                                       bool locSpecified,
                                       bool checkFareBreaksOnly);
  static bool processTravelSegsBackward(PricingTrx& trx,
                                        const TSIData& tsiData,
                                        TravelSegWrapperVector& applTravelSeg,
                                        TSITravelSegMarkupContainer& tsMarkup,
                                        bool locSpecified,
                                        bool checkFareBreaksOnly);
  static bool getDirectionsFromLoop(PricingTrx& trx,
                                    const TSIInfo& tsiInfo,
                                    bool& loopForward,
                                    bool& checkFareBreaksOnly);

  static RuleConst::TSIScopeType
  scopeTSIGeoScopeSetup(const TSIInfo* tsiInfo,
                        const RuleConst::TSIScopeParamType& defaultScope,
                        const bool allowJourneyScopeOverride,
                        const bool allowPUScopeOverride,
                        const bool allowFCScopeOverride);
  static bool scopeTSIGeoScopeValidate(TSICode tsi,
                                       RuleConst::TSIScopeType scope,
                                       PricingTrx& trx,
                                       const FarePath* farePath,
                                       const Itin* itin,
                                       const PricingUnit* pu,
                                       const FareMarket* fm);
  static bool scopeTSIGeoCheckDMC(RuleConst::TSIScopeType scope,
                                  PricingTrx& trx,
                                  const PricingUnit* pu,
                                  const FareMarket* fm,
                                  const FareMarket*& retFM);

  static bool scopeTSIGeoCreateDiag(PricingTrx& trx,
                                    DCFactory*& factory,
                                    DiagCollector*& diag,
                                    const DiagnosticTypes& callerDiag,
                                    const TSIData& tsiData,
                                    const RuleConst::TSIScopeParamType& defaultScope,
                                    const RuleConst::TSIScopeType& processingScope,
                                    const bool allowJourneyScopeOverride,
                                    const bool allowPUScopeOverride,
                                    const bool allowFCScopeOverride,
                                    const LocKey& locKey1,
                                    const LocKey& locKey2);
  static void scopeTSIGeoWriteMatch(DiagCollector* diag,
                                    const TSIInfo* tsiInfo,
                                    RuleConst::TSIScopeType scope,
                                    bool matchFound);

  static void
  identifyIntlDomTransfers(const TSIData& tsiData, TSITravelSegMarkupContainer& tsMarkup);

  //
  // Checks a TravelSeg against the TSI GeoData.
  //
  static bool checkGeoData(const PricingTrx& trx,
                           const TSIData& tsiData,
                           const TravelSeg* seg,
                           bool& origMatch,
                           bool& destMatch);

  //
  // check segment locations matching
  //
  static void getLocationMatchings(const PricingTrx& trx,
                                   const TSIData& tsiData,
                                   const TravelSeg* seg,
                                   bool& loc1OrigMatch,
                                   bool& loc1DestMatch,
                                   bool& loc2OrigMatch,
                                   bool& loc2DestMatch);
  //
  // check if locations passed macth to segment origin and desination
  static bool getLocationMatchings(const PricingTrx& trx,
                                   const Loc& loc,
                                   const LocTypeCode& locType,
                                   const LocCode& locCode,
                                   const VendorCode& vendorCode,
                                   GeoTravelType geoTvlType);

  static bool checkGeoNotType(const TSIData& tsiData);

  //
  // Gets the GeoLocale from the Itinerary
  //
  static bool getGeoLocaleFromItin(const TSIData& tsiData, const Loc*& loc);
  static bool getGeoLocaleFromItinTurnAround(const TSIData& tsiData, const Loc*& loc);

  //
  // Checks a TravelSeg against the TSI GeoData and Match Criteria.
  //
  static RuleConst::TSIMatch
  checkTSIMatchTravelSeg(const PricingTrx& trx,
                         const TSIData& tsiData,
                         const TSITravelSegMarkup& tsm,
                         const std::vector<TSIInfo::TSIMatchCriteria>& mc,
                         bool& origMatch,
                         bool& destMatch,
                         std::string& noMatchReason);

  //
  // Checks a TravelSeg against the TSI Match Criteria
  //
  static bool checkMatchCriteria(const TSIData& tsiData,
                                 const TSITravelSegMarkup& tsm,
                                 const std::vector<TSIInfo::TSIMatchCriteria>& mc,
                                 std::string& noMatchReason,
                                 const PricingTrx& trx);

  class TSIMatchCriteria
  {
    friend class RuleUtilTSITest_GetTSIOrigDestCheck;
    friend class RuleUtilTSITest_CheckIntercontinental;

  public:
    TSIMatchCriteria(const PricingTrx& trx,
                     const TSIData& tsiData,
                     const TSITravelSegMarkup& tsm,
                     std::string& noMatchReason);
    bool process(TSIInfo::TSIMatchCriteria matchCriteria);

  protected:
    bool checkStopOver();
    bool checkInbound();
    bool checkOutbound();
    bool checkFurthest();
    bool checkDomestic();
    bool checkOneCountry();
    bool checkInternational();
    bool checkGateway();
    bool checkOrigGateway();
    bool checkDestGateway();
    bool checkTransAtlantic();
    bool checkTransPacific();
    bool checkTransOceanic();
    bool checkIntercontinental();
    bool checkIntercontinentalFD();
    bool checkIntercontinentalRtw();
    bool checkIsOverWater();
    bool checkIsIntlDomTransfer();

    void getCheckOrigDest(bool& origCheck, bool& destCheck);

    const PricingTrx& _trx;
    const TSIData& _tsiData;
    const TSITravelSegMarkup& _tsm;
    std::string& _noMatchReason;
    LocUtil::LocList _rtwContinents;
  };

  static void checkLoopMatch(const TSIData& tsiData,
                             const RuleConst::TSIMatch& prevMatch,
                             const RuleConst::TSIMatch& currMatch,
                             bool& addPrevTravSeg,
                             bool& addCurrTravSeg,
                             bool& addNextTravSeg,
                             bool& continueLooping,
                             bool locSpecified);

  static void checkLoopMatchAll(const TSIData& tsiData,
                                const RuleConst::TSIMatch& currMatch,
                                bool& addPrevTravSeg,
                                bool& addCurrTravSeg,
                                bool& addNextTravSeg,
                                bool& continueLooping);

  static void checkLoopMatchFirst(const TSIData& tsiData,
                                  const RuleConst::TSIMatch& currMatch,
                                  bool& addPrevTravSeg,
                                  bool& addCurrTravSeg,
                                  bool& addNextTravSeg,
                                  bool& continueLooping);

  static void checkLoopMatchOnce(const TSIData& tsiData,
                                 const RuleConst::TSIMatch& currMatch,
                                 bool& addPrevTravSeg,
                                 bool& addCurrTravSeg,
                                 bool& addNextTravSeg,
                                 bool& continueLooping,
                                 bool locSpecified);

  static void checkLoopMatchLast(const TSIData& tsiData);

  static void checkLoopMatchFirstAll(const TSIData& tsiData,
                                     const RuleConst::TSIMatch& currMatch,
                                     bool& addPrevTravSeg,
                                     bool& addCurrTravSeg,
                                     bool& addNextTravSeg,
                                     bool& continueLooping);

  static void checkLoopMatchSoft(const TSIData& tsiData,
                                 const RuleConst::TSIMatch& currMatch,
                                 bool& addPrevTravSeg,
                                 bool& addCurrTravSeg,
                                 bool& addNextTravSeg,
                                 bool& continueLooping);

  static void checkLoopMatchSecondFirst(const TSIData& tsiData,
                                        const RuleConst::TSIMatch& prevMatch,
                                        const RuleConst::TSIMatch& currMatch,
                                        bool& addPrevTravSeg,
                                        bool& addCurrTravSeg,
                                        bool& addNextTravSeg,
                                        bool& continueLooping);

  static void getLoopItemToSet(const TSIInfo& tsiInfo,
                               bool& addPrevTravSeg,
                               bool& addCurrTravSeg,
                               bool& addNextTravSeg);

  static void writeTSIDiagLine(DiagCollector& diag);

  static void writeTSIDiagHeader(DiagCollector& diag,
                                 const TSIData& tsiData,
                                 const RuleConst::TSIScopeParamType& defaultScope,
                                 const RuleConst::TSIScopeType& processingScope,
                                 const bool allowJourneyScopeOverride,
                                 const bool allowPUScopeOverride,
                                 const bool allowFCScopeOverride,
                                 const LocKey& locKey1,
                                 const LocKey& locKey2);
  static std::string writeTSIDiagHeaderGeoReqired(const char& geoReq);
  static std::string writeTSIDiagHeaderGeoOut(const char& geoReq);
  static std::string writeTSIDiagHeaderGeoItinPart(const char& geoItinPart);
  static std::string writeTSIDiagHeaderGeoGeoCheck(const char& geoCheck);
  static std::string writeTSIDiagHeaderLoopDirection(const char& loopDirection);
  static std::string writeTSIDiagHeaderLoopToSet(const int16_t& loopToSet);
  static std::string writeTSIDiagHeaderLoopMatch(const char& loopMatch);
  static std::string writeTSIDiagHeaderScopeParam(const RuleConst::TSIScopeParamType& defaultScope);
  static std::string writeTSIDiagHeaderScope(const char& scope);
  static std::string writeTSIDiagHeaderProcScope(const RuleConst::TSIScopeType& scope);
  static std::string writeTSIDiagHeaderType(const char& type);
  static std::string writeTSIDiagHeaderMatchCriteria(const TSIInfo::TSIMatchCriteria& mc);

  static void writeTSIDiagDetails(DiagCollector& diag,
                                  const TSIData& tsiData,
                                  TSITravelSegMarkupContainer& tsMarkup);
  static void writeTSIDiagTravelSeg(DiagCollector& diag,
                                    const int32_t puNumber,
                                    const int32_t fcNumber,
                                    const bool displayPuFc,
                                    const TSITravelSegMarkup& tsm);

public: // used by FareController etc.
  static log4cxx::LoggerPtr _dataErrLogger;

private:
  static log4cxx::LoggerPtr _tsiLogger;

protected:
  class TSISetup
  {
    friend class RuleUtilTSITest_TSISetupChain;

  protected:
    // for unit test purpose
    enum TSISETUPTYPE
    {
      TSI_NONE = 0,
      TSI_TURNAROUND,
      TSI_GATEWAY,
      TSI_OVERWATER,
      TSI_INTERNATIONAL
    };

  public:
    // ctors
    virtual ~TSISetup() {}
    // derived classes shold impelement this metods
    virtual bool build(const RuleConst::TSIScopeType& scope,
                       std::string& errMsg,
                       const TSIData& tsiData,
                       bool loopForward,
                       PricingTrx& trx) = 0;
    virtual void process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup) = 0;
    virtual void initialize(PricingTrx&) {};
    const std::vector<TravelSeg*>&
    travelSeg(const RuleConst::TSIScopeType& scope, const TSIData& tsiData);
    virtual int32_t tsiType() { return TSI_NONE; }
  };

  class TSISetupTurnAround : public TSISetup
  {
    friend class RuleUtilTSITest_TSISetupTurnAround;

  public:
    TSISetupTurnAround() : _turnAroundPoint(nullptr), _turnAroundPointAtDest(nullptr) {}
    bool build(const RuleConst::TSIScopeType& scope,
               std::string& errMsg,
               const TSIData& tsiData,
               bool loopForward,
               PricingTrx& trx) override;
    void process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup) override;
    int32_t tsiType() override { return TSI_TURNAROUND; }

  private:
    const TravelSeg* _turnAroundPoint;
    const TravelSeg* _turnAroundPointAtDest;
  };

  class TSISetupGateway : public TSISetup
  {
    friend class RuleUtilTSITest_TSISetupGateway;

  public:
    TSISetupGateway()
      : _foundGateways(false),
        _tsiGW(nullptr)
    {
    }
    bool build(const RuleConst::TSIScopeType& scope,
               std::string& errMsg,
               const TSIData& tsiData,
               bool loopForward,
               PricingTrx& trx) override;
    void process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup) override;
    void initialize(PricingTrx& trx) override;
    int32_t tsiType() override { return TSI_GATEWAY; }

    void collectOutInTvl(const PricingUnit& pu,
                         std::vector<TravelSeg*>& outSegs,
                         std::vector<TravelSeg*>& inSegs);

    bool _foundGateways;

    TSIGateway* _tsiGW;
  };

  class TSISetupOverWater : public TSISetup
  {
    friend class RuleUtilTSITest_TSISetupOverWater;

  public:
    bool build(const RuleConst::TSIScopeType& scope,
               std::string& errMsg,
               const TSIData& tsiData,
               bool loopForward,
               PricingTrx& trx) override;
    void process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup) override;
    int32_t tsiType() override { return TSI_OVERWATER; }

  private:
    std::set<const TravelSeg*> _overWaterSegments;
  };

  class TSISetupInternational : public TSISetup
  {
    friend class RuleUtilTSITest_TSISetupInternational;

  public:
    bool build(const RuleConst::TSIScopeType& scope,
               std::string& errMsg,
               const TSIData& tsiData,
               bool loopForward,
               PricingTrx& trx) override;
    void process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup) override;
    int32_t tsiType() override { return TSI_INTERNATIONAL; }
    //  private:
    void getInternationalSegmentsForStopoverTSI(const Loc& referenceLoc,
                                                const std::vector<TravelSeg*>& inSeg,
                                                std::vector<TravelSeg*>& outSeg);
    void getInternationalSegmentsForTSI18(const std::vector<TravelSeg*>& inSeg,
                                          std::vector<TravelSeg*>& outSeg);

    std::set<const TravelSeg*> _intlSegments;
  };

  class TSISetupChain
  {
    friend class RuleUtilTSITest_TSISetupChain;

  public:
    TSISetupChain() : _checkOrig(false), _checkDest(false), _checkFareBreaksOnly(false) {}

    bool& checkOrig() { return _checkOrig; }
    bool& checkDest() { return _checkDest; }
    bool& checkFareBreaksOnly() { return _checkFareBreaksOnly; }
    std::string& errMsg() { return _errMsg; }
    bool
    buildTSIChain(PricingTrx& trx, const TSIData& tsi, bool loopForward, bool checkFareBreaksOnly);
    void process(const TravelSeg* seg, TSITravelSegMarkup& tsm);
    bool passCheckFareBreakOnly(TSITravelSegMarkup& tsm);

  private:
    struct TSISetupProcess
    {
      TSISetupProcess(const TravelSeg* seg, TSITravelSegMarkup& tsm)
        : _TravelSeg(seg), _segMarkup(tsm)
      {
      }
      void operator()(TSISetup* tsiSetup) { tsiSetup->process(_TravelSeg, _segMarkup); }
      const TravelSeg* _TravelSeg;
      TSITravelSegMarkup& _segMarkup;
    };
    std::vector<TSISetup*> _setupChain;
    bool _checkOrig;
    bool _checkDest;
    bool _checkFareBreaksOnly;
    std::string _errMsg;

  public:
  };

  static bool setupTravelSegMarkupJour(PricingTrx& trx,
                                       const FareUsage* fareUsage,
                                       const TSIData& tsiData,
                                       TSITravelSegMarkupContainer& tsMarkup,
                                       bool loopForward,
                                       TSISetupChain& tsiChain,
                                       bool& stopoverExists);
  static bool setupTravelSegMarkupSubJour(PricingTrx& trx,
                                          const FareUsage* fareUsage,
                                          const TSIData& tsiData,
                                          TSITravelSegMarkupContainer& tsMarkup,
                                          bool loopForward,
                                          TSISetupChain& tsiChain,
                                          bool& stopoverExists);
  static bool setupTravelSegMarkupFare(PricingTrx& trx,
                                       const FareUsage* fareUsage,
                                       const TSIData& tsiData,
                                       TSITravelSegMarkupContainer& tsMarkup,
                                       bool loopForward,
                                       TSISetupChain& tsiChain,
                                       bool& stopoverExists);
};

// ----------------------------------------------------------------------------
//
// @function int32_t RuleUtilTSI::setupTravelSegMarkupGetStart
//
// Description:
//
// function gets integer index of first element depending on the loop direction
//
// @param  loopForward            - loop direction
// @param  container              - reference to container
//
// @return integer index of first element
//
// ----------------------------------------------------------------------------
template <class T>
inline int32_t
RuleUtilTSI::setupTravelSegMarkupGetStart(bool loopForward, const T& container)
{
  return loopForward ? 0 : int32_t(container.size()) - 1;
}

// ----------------------------------------------------------------------------
//
// @function int32_t RuleUtilTSI::setupTravelSegMarkupGetEnd
//
// Description:
//
// function gets integer index of end element depending on the loop direction
//
// @param  loopForward            - loop direction
// @param  container              - reference to container
//
// @return integer index of end element
//
// ----------------------------------------------------------------------------
template <class T>
inline int32_t
RuleUtilTSI::setupTravelSegMarkupGetEnd(bool loopForward, const T& container)
{
  return loopForward ? int32_t(container.size()) : -1;
}

} // tse
