//-------------------------------------------------------------------
//
//  File:        Stopovers.h
//  Created:     July 1, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
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
//-------------------------------------------------------------------

#pragma once

#include "Rules/RuleApplicationBase.h"

#include <list>

namespace tse
{
class DiagCollector;
class FareMarket;
class FarePath;
class FareUsage;
class Itin;
class Loc;
class Logger;
class PaxTypeFare;
class PricingTrx;
class PricingUnit;
class RuleItemInfo;
class StopoversInfo;
class StopoversInfoSeg;
class StopoversInfoWrapper;

class Stopovers : public RuleApplicationBase
{
  friend class StopoversTest;

public:
  static constexpr Indicator UNAVAIL_TAG_NOT_AVAIL = 'X';
  static constexpr Indicator UNAVAIL_TAG_TEXT_ONLY = 'Y';

  static const std::string NUM_STOPS_UNLIMITED;

  static constexpr Indicator OUT_OR_RETURN_EXCLUSIVE = 'X';
  static constexpr Indicator SAME_CARRIER_REQUIRED = 'X';
  static constexpr Indicator OPEN_JAW_REQUIRED = 'X';
  static constexpr Indicator CIRCLE_TRIP_2_REQUIRED = 'X';
  static constexpr Indicator CIRCLE_TRIP_2_PLUS_REQUIRED = 'X';

  static constexpr Indicator SEG_NO_CHARGE = ' ';
  static constexpr Indicator SEG_USE_CHARGE_1 = '1';
  static constexpr Indicator SEG_USE_CHARGE_2 = '2';

  static constexpr Indicator SEG_GEO_APPL_REQUIRED = 'R';
  static constexpr Indicator SEG_GEO_APPL_NOT_PERMITTED = 'N';
  static constexpr Indicator SEG_GEO_APPL_BLANK = ' ';

  static constexpr Indicator SEG_SAME_CARRIER_SAME = 'Y';
  static constexpr Indicator SEG_SAME_CARRIER_NOT_SAME = 'N';
  static constexpr Indicator SEG_SAME_CARRIER_BLANK = ' ';

  static constexpr Indicator SEG_INOUT_IN = 'I';
  static constexpr Indicator SEG_INOUT_OUT = 'O';
  static constexpr Indicator SEG_INOUT_EITHER = 'E';
  static constexpr Indicator SEG_INOUT_BLANK = ' ';

  static constexpr Indicator CXR_PREF_APPLY_LEAST_RESTRICTION = 'Y';

  static constexpr Indicator GTWY_BLANK = ' ';
  static constexpr Indicator GTWY_PERMITTED = 'Y';
  static constexpr Indicator GTWY_NOT_PERMITTED = 'N';
  static constexpr Indicator GTWY_REQUIRED = 'P';

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket,
                              bool performFullFmVal);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override;

  void setRtw(bool rtw) { _isRtw = rtw; }
  bool isRtw() const { return _isRtw; }

  static Record3ReturnTypes applySystemDefaultAssumption(PricingTrx& trx,
                                                         const Itin& itin,
                                                         const PaxTypeFare& fare,
                                                         const FareMarket& fareMarket);

protected:
  bool isGeoSpecified(const StopoversInfoSeg* soInfoSeg);

  enum StopType
  {
    NONE,
    CONNECTION,
    STOPOVER
  };
  enum MatchResult
  {
    NOT_CHECKED,
    MATCH,
    NOT_MATCH,
    DOES_NOT_APPLY
  };

  enum ProcessingResult
  {
    PASS, // The validation result is PASS.
    SOFTPASS, // The validation result is PASS but requires revalidation
    //  during Pricing Unit validation.
    FAIL, // The validation result is FAIL. No more processing
    //  required for this stopover rule.
    SKIP, // The result of this stopover rule is not a PASS or FAIL.
    // No more processing required for this stopover rule.
    // Processing can continue with the next rule item.
    STOP, // Some problem occurred. Stop processing.
    CONTINUE // Continue processing remaining data for this Stopover rule.
  };

  class StopoverInfoSegMatch final
  {
  public:
    enum CheckResult
    {
      NOT_CHECKED,
      PASS,
      FAIL,
      DOES_NOT_APPLY
    };
    enum ChargeType
    {
      NOCHARGE,
      CHARGE1,
      CHARGE2
    };

    StopoverInfoSegMatch(const StopoversInfoSeg* soInfoSeg) : _soInfoSeg(soInfoSeg) {}

    const StopoversInfoSeg* soInfoSeg() const { return _soInfoSeg; }

    const ChargeType& chargeType() const { return _chargeType; }
    ChargeType& chargeType() { return _chargeType; }

    const CheckResult& segCheckResult() const { return _segCheckResult; }
    CheckResult& segCheckResult() { return _segCheckResult; }

    const MatchResult& geoMatchResult() const { return _geoMatchResult; }
    MatchResult& geoMatchResult() { return _geoMatchResult; }

    const MatchResult& sameCarrierMatchResult() const { return _sameCarrierMatchResult; }
    MatchResult& sameCarrierMatchResult() { return _sameCarrierMatchResult; }

    const MatchResult& inCarrierMatchResult() const { return _inCarrierMatchResult; }
    MatchResult& inCarrierMatchResult() { return _inCarrierMatchResult; }

    const MatchResult& outCarrierMatchResult() const { return _outCarrierMatchResult; }
    MatchResult& outCarrierMatchResult() { return _outCarrierMatchResult; }

  private:
    const StopoversInfoSeg* _soInfoSeg;
    ChargeType _chargeType = ChargeType::NOCHARGE;
    CheckResult _segCheckResult = CheckResult::NOT_CHECKED;
    MatchResult _geoMatchResult = Stopovers::MatchResult::NOT_CHECKED;
    MatchResult _sameCarrierMatchResult = Stopovers::MatchResult::NOT_CHECKED;
    MatchResult _inCarrierMatchResult = Stopovers::MatchResult::NOT_CHECKED;
    MatchResult _outCarrierMatchResult = Stopovers::MatchResult::NOT_CHECKED;
  };

  typedef std::list<StopoverInfoSegMatch> SiSegMatchList;
  typedef SiSegMatchList::const_iterator SiSegMatchListCI;

  /**
   *  @class TravelSegMarkup
   *
   *  @description This class is an adaptation of the Decorator pattern.
   *               It is used to attach additional attributes to a
   *               TravelSeg that are needed to process Stopovers.
   */
  class TravelSegMarkup final
  {
  public:
    enum CheckResult
    {
      NOT_CHECKED,
      PASS,
      SOFTPASS,
      FAIL,
      DOES_NOT_APPLY,
      NEED_REVALIDATION,
      STOP
    };

    enum Direction
    {
      OUTBOUND,
      INBOUND,
      UNKNOWN
    };

    TravelSegMarkup(TravelSeg* ts = nullptr) : _travelSeg(ts) {}

    const TravelSeg* travelSeg() const { return _travelSeg; }
    TravelSeg*& travelSeg() { return _travelSeg; }

    const TravelSeg* nextTravelSeg() const { return _nextTravelSeg; }
    TravelSeg*& nextTravelSeg() { return _nextTravelSeg; }

    const TravelSeg* arunkTravelSeg() const { return _arunkTravelSeg; }
    TravelSeg*& arunkTravelSeg() { return _arunkTravelSeg; }

    const Loc* altOffPointGeo() const { return _altOffPointGeo; }
    const Loc*& altOffPointGeo() { return _altOffPointGeo; }

    const int16_t segmentNumber() const { return _segmentNumber; }
    int16_t& segmentNumber() { return _segmentNumber; }

    const GeoTravelType& geoTravelType() const { return _geoTravelType; }
    GeoTravelType& geoTravelType() { return _geoTravelType; }

    const FareUsage* fareUsage() const { return _fareUsage; }
    FareUsage*& fareUsage() { return _fareUsage; }

    const FareMarket* fareMarket() const { return _fareMarket; }
    const FareMarket*& fareMarket() { return _fareMarket; }

    const CarrierCode& carrier() const { return _carrier; }
    CarrierCode& carrier() { return _carrier; }

    const CarrierCode& carrierOut() const { return _carrierOut; }
    CarrierCode& carrierOut() { return _carrierOut; }

    const StopType& stopType() const { return _stopType; }
    StopType& stopType() { return _stopType; }

    const Direction& direction() const { return _direction; }
    Direction& direction() { return _direction; }

    const bool& isGateway() const { return _isGateway; }
    bool& isGateway() { return _isGateway; }

    const MatchResult& gatewayGeoTableMatchResult() const { return _gatewayGeoTableMatchResult; }
    MatchResult& gatewayGeoTableMatchResult() { return _gatewayGeoTableMatchResult; }

    const CheckResult& stopoverCheckResult() const { return _stopoverCheckResult; }
    CheckResult& stopoverCheckResult() { return _stopoverCheckResult; }

    const CheckResult& soSegCheckResult() const { return _soSegCheckResult; }
    CheckResult& soSegCheckResult() { return _soSegCheckResult; }

    const bool& isChargeSegmentSpecific() const { return _isChargeSegmentSpecific; }
    bool& isChargeSegmentSpecific() { return _isChargeSegmentSpecific; }

    const bool& useSegmentsToApplyCharges() const { return _useSegmentsToApplyCharges; }
    bool& useSegmentsToApplyCharges() { return _useSegmentsToApplyCharges; }

    const bool& validateEntireRule() const { return _validateEntireRule; }
    bool& validateEntireRule() { return _validateEntireRule; }

    const bool& isTentativeMatch() const { return _isTentativeMatch; }
    bool& isTentativeMatch() { return _isTentativeMatch; }

    const bool& isSurfaceSector() const { return _surfaceSector; }
    bool& isSurfaceSector() { return _surfaceSector; }

    const bool& forceArnkPass() const { return _forceArnkPass; }
    bool& forceArnkPass() { return _forceArnkPass; }

    const std::string& failReason() const { return _failReason; }
    std::string& failReason() { return _failReason; }

    const SiSegMatchList& siSegMatch() const { return _siSegMatch; }
    SiSegMatchList& siSegMatch() { return _siSegMatch; }

    const StopoversInfoSeg* segInfoPass() const { return _recurringSegmentMatched; }
    const StopoversInfoSeg*& segInfoPass() { return _recurringSegmentMatched; }

    const bool& noMoreStopover() const { return _exceedNumberOfStopovers; }
    bool& noMoreStopover() { return _exceedNumberOfStopovers; }

  private:
    TravelSeg* _travelSeg = nullptr;
    TravelSeg* _nextTravelSeg = nullptr;
    TravelSeg* _arunkTravelSeg = nullptr;
    const Loc* _altOffPointGeo = nullptr; // Alternate Geo location to validate
    //  This comes from a following
    //  surface sector.
    int16_t _segmentNumber = 0;
    GeoTravelType _geoTravelType = GeoTravelType::Domestic;
    FareUsage* _fareUsage = nullptr;
    const FareMarket* _fareMarket = nullptr;
    CarrierCode _carrier;
    CarrierCode _carrierOut;
    StopType _stopType = StopType::NONE;
    Direction _direction = Direction::UNKNOWN;
    bool _isGateway = false;
    MatchResult _gatewayGeoTableMatchResult = Stopovers::MatchResult::NOT_CHECKED;
    CheckResult _stopoverCheckResult = CheckResult::NOT_CHECKED;
    CheckResult _soSegCheckResult = CheckResult::NOT_CHECKED;
    bool _isChargeSegmentSpecific = false;
    bool _useSegmentsToApplyCharges = true;
    bool _validateEntireRule = true; // If false, only validate MAX
    bool _isTentativeMatch = false;
    bool _surfaceSector = false;
    bool _forceArnkPass = false;
    std::string _failReason;
    SiSegMatchList _siSegMatch;
    const StopoversInfoSeg* _recurringSegmentMatched = nullptr;
    bool _exceedNumberOfStopovers = false;
  };

  // -----------------------------------------------------
  //  Private container class.
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
  class TravelSegMarkupContainer
  {
  public:
    TravelSegMarkupContainer() {}
    ~TravelSegMarkupContainer() {}

    typedef std::deque<TravelSegMarkup*>::size_type size_type;
    typedef std::deque<TravelSegMarkup*>::reference reference;
    typedef std::deque<TravelSegMarkup*>::iterator iterator;
    typedef std::deque<TravelSegMarkup*>::reverse_iterator reverse_iterator;
    typedef std::deque<TravelSegMarkup*>::const_iterator const_iterator;

    size_type size() const { return _tsmDeque.size(); }
    bool empty() const { return _tsmDeque.empty(); }

    reference front() { return _tsmDeque.front(); }
    reference back() { return _tsmDeque.back(); }

    iterator begin() { return _tsmDeque.begin(); }
    iterator end() { return _tsmDeque.end(); }

    const_iterator begin() const { return _tsmDeque.begin(); }
    const_iterator end() const { return _tsmDeque.end(); }

    reverse_iterator rbegin() { return _tsmDeque.rbegin(); }
    reverse_iterator rend() { return _tsmDeque.rend(); }

    // The travelSeg member of tsm must be set
    //  prior to calling this method!!
    void push_back(TravelSegMarkup* tsm)
    {
      _tsmDeque.push_back(tsm);

      std::map<const TravelSeg*, int16_t>::value_type value(tsm->travelSeg(), _tsmDeque.size() - 1);
      _tsMap.insert(value);
    }

    // The travelSeg member of tsm must be set
    //  prior to calling this method!!
    void push_front(TravelSegMarkup* tsm)
    {
      _tsmDeque.push_front(tsm);

      std::map<const TravelSeg*, int16_t>::value_type value(tsm->travelSeg(), 0);
      _tsMap.insert(value);

      // Since we pushed to the front of the deque we have
      //  to update the map with the new indexes.
      if (size() > 1)
      {
        for (size_type ctr = 1; ctr != size(); ++ctr)
        {
          _tsMap[_tsmDeque[ctr]->travelSeg()] = ctr;
        }
      }
    }

    iterator find(const TravelSeg* ts)
    {
      if (empty())
      {
        return end();
      }

      std::map<const TravelSeg*, size_type>::iterator i =
          _tsMap.find(ts); // O(log N) time complexity
      if (i == _tsMap.end())
      {
        return end();
      }
      size_type index = (*i).second;
      iterator iter = _tsmDeque.begin();
      iter += index; // constant time complexity
      return iter;
    }

    const_iterator find(const TravelSeg* ts) const
    {
      if (empty())
      {
        return end();
      }

      std::map<const TravelSeg*, size_type>::const_iterator i =
          _tsMap.find(ts); // O(log N) time complexity
      if (i == _tsMap.end())
      {
        return end();
      }
      size_type index = (*i).second;
      const_iterator iter = _tsmDeque.begin();
      iter += index; // constant time complexity
      return iter;
    }

  private:
    // This deque contains the TravelSegMarkup objects and
    //  preserves the original order of the travel segments
    std::deque<TravelSegMarkup*> _tsmDeque;

    // This map is used to find a specific object in the tsmDeque
    //  using a TravelSeg pointer as the key.
    std::map<const TravelSeg*, size_type> _tsMap;
  };

  typedef TravelSegMarkupContainer::iterator TravelSegMarkupI;
  typedef TravelSegMarkupContainer::const_iterator TravelSegMarkupCI;
  typedef TravelSegMarkupContainer::reverse_iterator TravelSegMarkupRI;

  class StopoversInfoSegMarkup final
  {
  public:
    struct MatchedCounts
    {
      uint16_t _matchCount = 0;
      uint16_t _matchCountIn = 0;
      uint16_t _matchCountOut = 0;
      uint16_t _matchCountCharge = 0;
      uint16_t _matchCountInCharge = 0;
      uint16_t _matchCountOutCharge = 0;
    };

    void initialize(const StopoversInfoSeg* soInfoSeg);

    const StopoversInfoSeg* soInfoSeg() const { return _soInfoSeg; }
    const uint16_t numStopovers() const { return _numStopovers; }

    bool numStopoversUnlimited() const { return _numStopoversUnlimited; }
    bool checkInOut() { return _checkInOut; }
    bool inOutOut() { return _inOutOut; }
    bool inOutIn() { return _inOutIn; }
    bool inOutEither() { return _inOutEither; }

    uint16_t& matchCount() { return _currentCounts->_matchCount; }
    const uint16_t matchCount() const { return _currentCounts->_matchCount; }

    uint16_t& matchCountIn() { return _currentCounts->_matchCountIn; }
    const uint16_t matchCountIn() const { return _currentCounts->_matchCountIn; }

    uint16_t& matchCountOut() { return _currentCounts->_matchCountOut; }
    const uint16_t matchCountOut() const { return _currentCounts->_matchCountOut; }

    uint16_t& matchCountCharge() { return _currentCounts->_matchCountCharge; }
    const uint16_t matchCountCharge() const { return _currentCounts->_matchCountCharge; }

    uint16_t& matchCountInCharge() { return _currentCounts->_matchCountInCharge; }
    const uint16_t matchCountInCharge() const { return _currentCounts->_matchCountInCharge; }

    uint16_t& matchCountOutCharge() { return _currentCounts->_matchCountOutCharge; }
    const uint16_t matchCountOutCharge() const { return _currentCounts->_matchCountOutCharge; }

    void setCurrentCounts(MatchedCounts* currentCounts) { _currentCounts = currentCounts; }

    std::map<std::string, MatchedCounts>& stoSameLocCnts() { return _stoSameLocCnts; }
    const std::map<std::string, MatchedCounts>& stoSameLocCnts() const { return _stoSameLocCnts; }

    void increaseMatchCount(bool countInbound, bool countOutbound);

  private:
    const StopoversInfoSeg* _soInfoSeg = nullptr;
    uint16_t _numStopovers = 0;
    bool _numStopoversUnlimited = false;

    bool _checkInOut = false;
    bool _inOutOut = false;
    bool _inOutIn = false;
    bool _inOutEither = false;

    MatchedCounts _defaultCounts;
    MatchedCounts* _currentCounts = &_defaultCounts;

    std::map<std::string, MatchedCounts> _stoSameLocCnts;
  };

  bool validateStopoversInfo(const StopoversInfo& soInfo);

  bool validateStopoversInfo_Mandate2010(const StopoversInfo& soInfo);

  bool initializeTravelSegMarkupContainer(PricingTrx& trx, TravelSegMarkupContainer& tsmCon, const FareMarket& fm);

  bool initializeTravelSegMarkupContainer(PricingTrx& trx,
                                          TravelSegMarkupContainer& tsmProcOrder,
                                          TravelSegMarkupContainer& tsmPUOrder,
                                          const PricingUnit& pu,
                                          const FareUsage& fareUsage,
                                          const StopoversInfoWrapper& soInfoWrapper,
                                          const Itin* itin = nullptr);

  bool ignoreRecurringSegForRtw(const StopoversInfoSeg& seg) const;

  void initializeStopoversInfoSegMarkup(DataHandle& localDataHandle,
                                        std::vector<StopoversInfoSegMarkup*>& soInfoSegMarkup,
                                        const std::vector<StopoversInfoSeg*>& segs,
                                        bool& allSegsGeoApplOK,
                                        bool& allSegsGeoApplNot,
                                        bool& geoRestrictionsExist);

  virtual ProcessingResult processStopovers(PricingTrx& trx,
                                            TravelSegMarkupContainer& tsmProcOrder,
                                            TravelSegMarkupContainer& tsmPUOrder,
                                            const StopoversInfoWrapper& soInfoWrapper,
                                            const FarePath* fp,
                                            const PricingUnit* pu,
                                            const FareMarket* fm,
                                            std::string& failReason,
                                            const VendorCode& vendorOfFare,
                                            const FareUsage* fUsage = nullptr);

  ProcessingResult processSurcharges(PricingTrx& trx,
                                     TravelSegMarkupContainer& tsmCon,
                                     const StopoversInfoWrapper& soInfoWrapper,
                                     const StopoversInfo& soInfo,
                                     const FareUsage& fareUsage,
                                     const FarePath& farePath);

  void identifyStopovers(TravelSegMarkupContainer& tsmCon, const StopoversInfo& soInfo,
                        PricingTrx& trx);

  void identifyGateways(PricingTrx& trx,
                        TravelSegMarkupContainer& tsmCon,
                        const StopoversInfo& soInfo,
                        const PricingUnit* pu,
                        const FareMarket* fm);

  void identifyGatewayGeoTableMatches(PricingTrx& trx,
                                      TravelSegMarkupContainer& tsmCon,
                                      const StopoversInfo& soInfo,
                                      const FarePath* fp,
                                      const PricingUnit* pu,
                                      const FareMarket* fm,
                                      const VendorCode& vendorOfFare);

  void markGateways(PricingTrx& trx, TravelSegMarkupContainer& tsmCon, const FareMarket& fm);

  void setStopType(TravelSegMarkup& tsm, const StopoversInfo& soInfo,
                   PricingTrx& trx);

  ProcessingResult checkMaxStopTime(PricingTrx& trx,
                                    TravelSegMarkupContainer& tsmCon,
                                    const StopoversInfo& soInfo,
                                    std::string& failReason,
                                    const FareUsage *fUsage = nullptr);

  // Remove with APO29538_StopoverMinStay
  bool
  isStayTimeWithinDays(const DateTime& arriveDT, const DateTime& departDT, const int16_t& stayDays);

  ProcessingResult processStopoverInfoSegs(PricingTrx& trx,
                                           const FareMarket* fm,
                                           const VendorCode& vendorOfFare,
                                           TravelSegMarkupContainer& tsmCon,
                                           const StopoversInfoWrapper& soInfoWrapper,
                                           std::string& failReason);

  bool checkRequiredSegmentsSatisfied(const StopoversInfoWrapper& soWrapper,
                                      const std::vector<StopoversInfoSegMarkup*>& segs,
                                      std::string& failReason) const;

  bool hasMoreSoInfoSegWithSameCharge(const std::vector<StopoversInfoSegMarkup*>::iterator& curSeg,
                                      const std::vector<StopoversInfoSegMarkup*>::iterator& endSeg,
                                      const Indicator& curSegChargeInd);

  bool checkImplicitInfoSegMatch(TravelSegMarkup& tsm, bool allSegsGeoApplNot);

  bool processInfoSegCarrier(TravelSegMarkup& tsm,
                             StopoverInfoSegMatch& siSegMatch,
                             const StopoversInfoSeg& soInfoSeg);

  MatchResult checkInfoSegSameCarrier(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg);

  MatchResult checkInfoSegCarrierIn(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg);

  MatchResult checkInfoSegCarrierOut(TravelSegMarkup& tsm, const StopoversInfoSeg& soInfoSeg);

  bool checkInfoSegDirectionality(TravelSegMarkup& tsm,
                                  StopoversInfoSegMarkup& soISM,
                                  bool& processNextInfoSeg,
                                  bool& failed,
                                  std::string& shortFailReason,
                                  std::string& failReason);

  bool checkInfoSegGeo(PricingTrx& trx,
                       const FareMarket* fm,
                       const VendorCode& vendorOfFare,
                       TravelSegMarkup& tsm,
                       StopoversInfoSegMarkup& soISM,
                       std::vector<StopoversInfoSegMarkup*>& soISMVector,
                       MatchResult& geoMatch,
                       bool& processNextInfoSeg,
                       const StopoversInfoSeg*& arunkMatchedInfoSeg);

  bool checkInfoSegMainGeo(PricingTrx& trx,
                           const FareMarket* fm,
                           const VendorCode& vendorOfFare,
                           TravelSegMarkup& tsm,
                           StopoversInfoSegMarkup& soISM,
                           MatchResult& geoMatch,
                           bool& processNextInfoSeg);

  bool checkInfoSegAltGeo(PricingTrx& trx,
                          const FareMarket* fm,
                          const VendorCode& vendorOfFare,
                          TravelSegMarkup& tsm,
                          StopoversInfoSegMarkup& soISM,
                          std::vector<StopoversInfoSegMarkup*>& soISMVector,
                          MatchResult& geoMatch,
                          bool& processNextInfoSeg,
                          const StopoversInfoSeg*& matchedInfoSeg);

  ProcessingResult processSamePointRestrictions(PricingTrx& trx,
                                                const FareMarket* fm,
                                                const TravelSegMarkupContainer& tsmCon,
                                                const StopoversInfo& soInfo,
                                                std::string& failReason);

  ProcessingResult checkMinStops(PricingTrx& trx,
                                 const TravelSegMarkupContainer& tsmCon,
                                 const StopoversInfo& soInfo,
                                 std::string& failReason);

  ProcessingResult checkMaxStopsInOut(PricingTrx& trx,
                                      const FarePath* fp,
                                      TravelSegMarkupContainer& tsmCon,
                                      const StopoversInfoWrapper& soInfoWrapper,
                                      std::string& failReason);

  ProcessingResult processGatewayRestrictions(PricingTrx& trx,
                                              TravelSegMarkupContainer& tsmCon,
                                              const StopoversInfo& soInfo,
                                              std::string& failReason);

  MatchResult matchGeo(PricingTrx& trx,
                       const FareMarket* fm,
                       const VendorCode& vendorOfFare,
                       const Loc* loc,
                       StopoversInfoSegMarkup& soISM);

  bool checkTripType(PricingTrx& trx,
                     const StopoversInfo& soInfo,
                     const PricingUnit& pu,
                     const FarePath& fp);

  bool checkPaxType(PricingTrx& trx,
                    const FarePath& fp,
                    FareUsage& fareUsage,
                    const StopoversInfoWrapper& soInfoWrapper);

  virtual FMDirection fareUsageDirToValidate(const StopoversInfo& soInfo);
  virtual bool
  isFareUsageDirPass(const FMDirection& fuDirToValidate, const TravelSegMarkup& tsm);

  void printDiagnostics(DiagCollector& diag,
                        const StopoversInfo& soInfo,
                        const TravelSegMarkupContainer& tsmCon,
                        const FareMarket& fm);

  void printDiagnostics(DiagCollector& diag,
                        const StopoversInfo& soInfo,
                        const TravelSegMarkupContainer& tsmCon,
                        const PricingUnit& pu);

  void printDiagHeader(DiagCollector& diag,
                       const StopoversInfoWrapper& soInfoWrapper,
                       const PricingUnit* pu);

  void printDiagTsmDetail(DiagCollector& diag,
                          const StopoversInfo& soInfo,
                          const TravelSegMarkup& tsm,
                          const TravelSeg* ts);

  void printDiagTravelSeg(DiagCollector& diag, const TravelSeg* ts, const int16_t segmentNumber);

  void printDiagStopoverInfoCharge2(DiagCollector& diag, const StopoversInfo& soInfo);

  bool atLeastOneRecurringSegPass(TravelSegMarkup& tsm);

  void chkApplScope(const StopoversInfo& soInfo);

  bool initializeTravelSegMarkupContainerSingleFU(PricingTrx& trx,
                                                  TravelSegMarkupContainer& tsmProcOrder,
                                                  TravelSegMarkupContainer& tsmPUOrder,
                                                  const PricingUnit& pu,
                                                  FareUsage* fu,
                                                  const StopoversInfoWrapper& soInfoWrapper,
                                                  const Itin* itin,
                                                  bool notValidateEntireRule,
                                                  bool swapFareUsageOrder);

  bool isNumStopsMaxUnlimited(const StopoversInfo& soInfo, int16_t& intValue) const;
  bool isNumStopsUnlimited(const std::string& str, int16_t& intValue) const;

  int16_t getNumStopMax(const StopoversInfo& soInfo) const;
  void setNextTravelSeg(TravelSegMarkupContainer& cont, TravelSeg* ts);

private:
  static Logger _logger;
  DataHandle _dataHandle;

  typedef std::map<const StopoversInfoSeg*, int16_t> SoMapSegmentMaxNumber;
  typedef SoMapSegmentMaxNumber::iterator SoMapSegmentMaxNumberI;

  bool _isRtw = false;

  enum ApplScope
  {
    UNKOWN_SCOPE,
    FC_SCOPE,
    PU_SCOPE
  };

  ApplScope _applScope = ApplScope::UNKOWN_SCOPE;

  bool locateStopoversInfo(const RuleItemInfo* rule,
                           const StopoversInfoWrapper*& soInfoWrapper,
                           const StopoversInfo*& soInfo);

  bool checkPreconditions(const StopoversInfo& soInfo,
                          Record3ReturnTypes& ret,
                          DiagCollector* diagPtr);

  bool isPuScope() const { return ( _applScope == PU_SCOPE ); }

  bool checkStopoversInSameLoc(const PricingTrx& trx,
                               const FareMarket* fm,
                               const VendorCode& vendor,
                               const LocTypeCode& locType,
                               const Loc& loc,
                               StopoversInfoSegMarkup& soISM);
  bool checkFirstAddNo(Indicator chargeInd,
                       int16_t matchCnt,
                       bool chargeFirstUnlimited,
                       bool charge1AddUnlimited,
                       int16_t charge1FirstNo,
                       int16_t charge1AddNo);
  bool checkGTWYGeoMatchResult(TravelSegMarkup& tsm,
                               StopoverInfoSegMatch& siSegMatch,
                               MatchResult gtwyGeoMatch,
                               MatchResult geoMatch,
                               Indicator geoAppl,
                               Indicator chargeInd,
                               bool& savePrevSegMatch);

  bool isGeoWithinSameLoc(const StopoversInfoSeg* soInfoSeg);

  // Save minimum stopover time, and unit, for every FareUsage in TravelSegMarkupContainer.
  void saveStopoverMinTime(TravelSegMarkupContainer& tsmCon, const StopoversInfo& soInfo);

  bool checkMaxExceed(TravelSeg* ts,
                      FareUsage* fu,
                      const StopoversInfoWrapper& soInfoWrapper,
                      bool isOutOrReturnExclusive,
                      bool isExceedNumStopovers,
                      bool isOutOrInExceeded,
                      bool isTotalMaxExceeded,
                      bool outMaxExceeded,
                      bool isInMaxExceeded) const;
  void setPassedValidation(FareUsage* fu,
                           const StopoversInfoWrapper& soInfoWrapper,
                           TravelSegMarkup* tsm,
                           TravelSeg* ts,
                           bool passedByLeastRestrictive) const;
  bool isStopoverNotPermitted(const StopoversInfoWrapper& soInfoWrapper,
                              bool numStopsMaxUnlimited,
                              int16_t numStopsMax,
                              int16_t tempTotalStopsCtr) const;

  void clearSurchargesForCXSegments(const StopoversInfoWrapper* soInfoWrapper,
                                    const StopoversInfo* stopInfo,
                                    TravelSegMarkupContainer& tsmCon,
                                    const FareUsage* fu) const;
};
} // namespace tse
