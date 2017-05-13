//-------------------------------------------------------------------
//
//  File:        Transfers1.h
//  Created:     May 23, 2006
//  Authors:     Andrew Ahmad
//
//  Description: Version 2 of Transfers processing code. This
//               second version is required for processing the
//               new Cat-09 record format as mandated by ATPCO.
//
//
//  Copyright Sabre 2004-2006
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
class PricingTrx;
class Itin;
class PaxTypeFare;
class FarePath;
class PricingUnit;
class FareUsage;
class RuleItemInfo;
class TransfersInfo1;
class TransfersInfoSeg1;
class TransfersInfoWrapper;
class DiagCollector;
class Loc;

class Transfers1 : public RuleApplicationBase
{
  friend class Transfers1Test;
  friend class TransfersTest1;

public:
  static constexpr Indicator UNAVAIL_TAG_NOT_AVAIL = 'X';
  static constexpr Indicator UNAVAIL_TAG_TEXT_ONLY = 'Y';

  static const std::string NUM_TRANSFERS_UNLIMITED;

  static constexpr Indicator OUT_OR_RETURN_EXCLUSIVE = 'X';

  static constexpr Indicator ALLOW_BLANK = ' ';
  static constexpr Indicator ALLOW_PRIME_PRIME = 'X';
  static constexpr Indicator ALLOW_PRIME_OTHER = 'X';
  static constexpr Indicator ALLOW_SAME_SAME = 'X';
  static constexpr Indicator ALLOW_OTHER_OTHER = 'X';
  static constexpr Indicator ALLOW_CONNECTION = 'C';
  static constexpr Indicator ALLOW_STOPOVER = 'S';
  static constexpr Indicator ALLOW_CONN_OR_STOP = ' ';

  static constexpr Indicator SEG_NO_CHARGE = ' ';
  static constexpr Indicator SEG_USE_CHARGE_1 = '1';
  static constexpr Indicator SEG_USE_CHARGE_2 = '2';

  static constexpr Indicator SEG_GEO_APPL_REQUIRED = 'R';
  static constexpr Indicator SEG_GEO_APPL_NOT_PERMITTED = 'N';
  static constexpr Indicator SEG_GEO_APPL_BLANK = ' ';

  static constexpr Indicator SEG_GATEWAY_ONLY = 'X';
  static constexpr Indicator SEG_GATEWAY_BLANK = ' ';

  static constexpr Indicator SEG_CARRIER_APPL_BETWEEN = 'X';
  static constexpr Indicator SEG_CARRIER_APPL_BLANK = ' ';

  static constexpr Indicator SEG_CARRIER_TBL_APPL_PERMITTED = ' ';
  static constexpr Indicator SEG_CARRIER_TBL_APPL_NOT_PERMITTED = 'X';

  static constexpr Indicator SEG_BETWEEN_APPL_BETWEEN = 'X';

  static constexpr Indicator SEG_RESTRICTION_DOMESTIC = 'D';
  static constexpr Indicator SEG_RESTRICTION_INTERNATIONAL = 'I';
  static constexpr Indicator SEG_RESTRICTION_DOM_INTL = 'B';
  static constexpr Indicator SEG_RESTRICTION_BLANK = ' ';

  static constexpr Indicator SEG_INOUT_IN = 'I';
  static constexpr Indicator SEG_INOUT_OUT = 'O';
  static constexpr Indicator SEG_INOUT_EITHER = 'E';
  static constexpr Indicator SEG_INOUT_BOTH = 'B';
  static constexpr Indicator SEG_INOUT_BLANK = ' ';

  static constexpr Indicator SURFACE_PERMITTED = 'Y';
  static constexpr Indicator SURFACE_NOT_PERMITTED = 'N';
  static const uint32_t SURFACE_UNLIMITED = 0xFFFFFFFF;

  static constexpr Indicator CXR_PREF_FARE_BRK_SURFACE_PERMITTED = 'N';
  static constexpr Indicator CXR_PREF_FARE_BRK_SURFACE_NOT_PERMITTED = 'Y';

  static constexpr Indicator SURFACE_TABLE_RESTR_DOMESTIC = 'D';
  static constexpr Indicator SURFACE_TABLE_RESTR_INTERNATIONAL = 'I';
  static constexpr Indicator SURFACE_TABLE_RESTR_EITHER = ' ';

  static constexpr Indicator SURFACE_TABLE_OD_DEST = 'D';
  static constexpr Indicator SURFACE_TABLE_OD_ORIG = 'O';
  static constexpr Indicator SURFACE_TABLE_OD_EITHER = ' ';

  static const uint32_t TRANSFER_RULE_ITEM_FOR_NOMATCH = 4294967295;

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

  bool& needSurfValidationForFP() { return _needSurfValidationForFP; }

  bool& excludeSurfaceBytes() { return _excludeSurfaceBytes; }
  const bool& excludeSurfaceBytes() const { return _excludeSurfaceBytes; }

  void setRtw(bool rtw) { _isRtw = rtw; }
  bool isRtw() const { return _isRtw; }

  static Record3ReturnTypes
  applySystemDefaultAssumption(const Itin& itin, const FareMarket& fareMarket);

protected:
  enum StopType
  {
    NONE,
    CONNECTION,
    STOPOVER
  };

  enum TransferType
  {
    TFR_NONE,
    PRIMARY_PRIMARY,
    SAME_SAME,
    PRIMARY_OTHER,
    OTHER_OTHER
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

  class TransferInfoSegMatch final
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

    TransferInfoSegMatch(const TransfersInfoSeg1* trInfoSeg) : _trInfoSeg(trInfoSeg) {}

    const TransfersInfoSeg1* trInfoSeg() const { return _trInfoSeg; }

    const ChargeType& chargeType() const { return _chargeType; }
    ChargeType& chargeType() { return _chargeType; }

    const CheckResult& segCheckResult() const { return _segCheckResult; }
    CheckResult& segCheckResult() { return _segCheckResult; }

    const CheckResult& inOutTransfersResult() const { return _inOutTransfersResult; }
    CheckResult& inOutTransfersResult() { return _inOutTransfersResult; }

    const MatchResult& transferTypeMatchResult() const { return _transferTypeMatchResult; }
    MatchResult& transferTypeMatchResult() { return _transferTypeMatchResult; }

    const MatchResult& geoMatchResult() const { return _geoMatchResult; }
    MatchResult& geoMatchResult() { return _geoMatchResult; }

    const MatchResult& gatewayMatchResult() const { return _gatewayMatchResult; }
    MatchResult& gatewayMatchResult() { return _gatewayMatchResult; }

    const MatchResult& stopConnMatchResult() const { return _stopConnMatchResult; }
    MatchResult& stopConnMatchResult() { return _stopConnMatchResult; }

    const MatchResult& inCarrierMatchResult() const { return _inCarrierMatchResult; }
    MatchResult& inCarrierMatchResult() { return _inCarrierMatchResult; }

    const MatchResult& outCarrierMatchResult() const { return _outCarrierMatchResult; }
    MatchResult& outCarrierMatchResult() { return _outCarrierMatchResult; }

    const MatchResult& restrictionMatchResult() const { return _restrictionMatchResult; }
    MatchResult& restrictionMatchResult() { return _restrictionMatchResult; }

  private:
    const TransfersInfoSeg1* _trInfoSeg = nullptr;
    ChargeType _chargeType = ChargeType::NOCHARGE;
    CheckResult _segCheckResult = CheckResult::NOT_CHECKED;
    CheckResult _inOutTransfersResult = CheckResult::NOT_CHECKED;
    MatchResult _transferTypeMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _geoMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _gatewayMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _stopConnMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _inCarrierMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _outCarrierMatchResult = MatchResult::NOT_CHECKED;
    MatchResult _restrictionMatchResult = MatchResult::NOT_CHECKED;
  };

  using TiSegMatchList = std::list<TransferInfoSegMatch>;
  using TiSegMatchListCI = TiSegMatchList::const_iterator;

  /**
   *  @class TravelSegMarkup
   *
   *  @description This class is an adaptation of the Decorator pattern.
   *               It is used to attach additional attributes to a
   *               TravelSeg that are needed to process Transfers.
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
      STOP,
      NOT_ACCEPTED
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

    const CarrierCode& governingCarrier() const { return _governingCarrier; }
    CarrierCode& governingCarrier() { return _governingCarrier; }

    const FlightNumber& flightNumber() const { return _flightNumber; }
    FlightNumber& flightNumber() { return _flightNumber; }

    const FlightNumber& flightNumberOut() const { return _flightNumberOut; }
    FlightNumber& flightNumberOut() { return _flightNumberOut; }

    const StopType& stopType() const { return _stopType; }
    StopType& stopType() { return _stopType; }

    const TransferType& transferType() const { return _transferType; }
    TransferType& transferType() { return _transferType; }

    const Direction& direction() const { return _direction; }
    Direction& direction() { return _direction; }

    const bool& isGateway() const { return _isGateway; }
    bool& isGateway() { return _isGateway; }

    const MatchResult& transferTypeMatchResult() const { return _transferTypeMatchResult; }
    MatchResult& transferTypeMatchResult() { return _transferTypeMatchResult; }

    const CheckResult& transferCheckResult() const { return _transferCheckResult; }
    CheckResult& transferCheckResult() { return _transferCheckResult; }

    const CheckResult& trSegCheckResult() const { return _trSegCheckResult; }
    CheckResult& trSegCheckResult() { return _trSegCheckResult; }

    const bool& isChargeSegmentSpecific() const { return _isChargeSegmentSpecific; }
    bool& isChargeSegmentSpecific() { return _isChargeSegmentSpecific; }

    const bool& useSegmentsToApplyCharges() const { return _useSegmentsToApplyCharges; }
    bool& useSegmentsToApplyCharges() { return _useSegmentsToApplyCharges; }

    const bool& validateEntireRule() const { return _validateEntireRule; }
    bool& validateEntireRule() { return _validateEntireRule; }

    const bool& isTentativeMatch() const { return _isTentativeMatch; }
    bool& isTentativeMatch() { return _isTentativeMatch; }

    const std::string& failReason() const { return _failReason; }
    std::string& failReason() { return _failReason; }

    const TiSegMatchList& tiSegMatch() const { return _tiSegMatch; }
    TiSegMatchList& tiSegMatch() { return _tiSegMatch; }

  private:
    TravelSeg* _travelSeg = nullptr;
    TravelSeg* _nextTravelSeg = nullptr;
    const Loc* _altOffPointGeo = nullptr; // Alternate Geo location to
    // validate
    //  This comes from a following
    //  surface sector.
    int16_t _segmentNumber = 0;
    GeoTravelType _geoTravelType = GeoTravelType::Domestic;
    FareUsage* _fareUsage = nullptr;
    const FareMarket* _fareMarket = nullptr;
    CarrierCode _carrier;
    CarrierCode _carrierOut;
    CarrierCode _governingCarrier;
    FlightNumber _flightNumber = 0;
    FlightNumber _flightNumberOut = 0;
    StopType _stopType = StopType::NONE;
    TransferType _transferType = TransferType::TFR_NONE;
    Direction _direction = Direction::UNKNOWN;
    bool _isGateway = false;
    MatchResult _transferTypeMatchResult = Transfers1::MatchResult::NOT_CHECKED;
    CheckResult _transferCheckResult = CheckResult::NOT_CHECKED;
    CheckResult _trSegCheckResult = CheckResult::NOT_CHECKED;
    bool _isChargeSegmentSpecific = false;
    bool _useSegmentsToApplyCharges = true;
    bool _validateEntireRule = true; // If false, only validate MAX
    bool _isTentativeMatch = false;
    std::string _failReason;
    TiSegMatchList _tiSegMatch;
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
  class TravelSegMarkupContainer final
  {
  public:
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

  class TransfersInfoSegMarkup final
  {
  public:
    void initialize(const TransfersInfoSeg1* trInfoSeg);

    const TransfersInfoSeg1* trInfoSeg() { return _trInfoSeg; }
    const int16_t numTransfers() { return _numTransfers; }

    bool numTransfersUnlimited() { return _numTransfersUnlimited; }
    bool checkTransferType() { return _checkTransferType; }
    bool checkInOut() { return _checkInOut; }
    bool inOutOut() { return _inOutOut; }
    bool inOutIn() { return _inOutIn; }
    bool inOutEither() { return _inOutEither; }
    bool inOutBoth() { return _inOutBoth; }

    uint16_t& matchCount() { return _currentCounts->_matchCount; }
    const uint16_t matchCount() const { return _currentCounts->_matchCount; }

    uint16_t& matchCountIn() { return _currentCounts->_matchCountIn; }
    const uint16_t matchCountIn() const { return _currentCounts->_matchCountIn; }

    uint16_t& matchCountOut() { return _currentCounts->_matchCountOut; }
    const uint16_t matchCountOut() const { return _currentCounts->_matchCountOut; }

    void setLocCounts(const std::string& loc) { _currentCounts = &_locCounts[loc]; }
    void setDefaultCounts() { _currentCounts = &_defaultCounts; }

    TravelSegMarkupCI& tsmBtwnOutFirst() { return _tsmBtwnOutFirst; }
    const TravelSegMarkupCI tsmBtwnOutFirst() const { return _tsmBtwnOutFirst; }

    TravelSegMarkupCI& tsmBtwnOutLast() { return _tsmBtwnOutLast; }
    const TravelSegMarkupCI tsmBtwnOutLast() const { return _tsmBtwnOutLast; }

    TravelSegMarkupCI& tsmBtwnInFirst() { return _tsmBtwnInFirst; }
    const TravelSegMarkupCI tsmBtwnInFirst() const { return _tsmBtwnInFirst; }

    TravelSegMarkupCI& tsmBtwnInLast() { return _tsmBtwnInLast; }
    const TravelSegMarkupCI tsmBtwnInLast() const { return _tsmBtwnInLast; }

  private:

    struct MatchedCounts
    {
      uint16_t _matchCount = 0;
      uint16_t _matchCountIn = 0;
      uint16_t _matchCountOut = 0;
    };

    const TransfersInfoSeg1* _trInfoSeg = nullptr;
    int16_t _numTransfers = 0;
    bool _numTransfersUnlimited = false;

    bool _checkTransferType = false;
    bool _checkInOut = false;
    bool _inOutOut = false;
    bool _inOutIn = false;
    bool _inOutEither = false;
    bool _inOutBoth = false;

    MatchedCounts _defaultCounts;
    MatchedCounts* _currentCounts = &_defaultCounts;
    std::map<std::string, MatchedCounts> _locCounts;

    TravelSegMarkupCI _tsmBtwnOutFirst;
    TravelSegMarkupCI _tsmBtwnOutLast;
    TravelSegMarkupCI _tsmBtwnInFirst;
    TravelSegMarkupCI _tsmBtwnInLast;
  };

  typedef std::map<const TravelSeg*, const RuleUtil::TravelSegWrapper*> TSITravelSegMap;

  bool checkPreconditions(const TransfersInfo1& trInfo, Record3ReturnTypes& ret, DiagCollector* dc);

  bool initializeTravelSegMarkupContainer(PricingTrx& trx,
                                          TravelSegMarkupContainer& tsmCon,
                                          const FareMarket& fm);
  bool checkIOIndTransferInfoSegs(const TransfersInfoWrapper& trInfoWrapper,
                                  const FareUsage& fareUsage,
                                  bool samePU);

  bool initializeTravelSegMarkupContainer(PricingTrx& trx,
                                          TravelSegMarkupContainer& tsmProcOrder,
                                          TravelSegMarkupContainer& tsmPUOrder,
                                          const PricingUnit& pu,
                                          const FareUsage& fareUsage,
                                          const TransfersInfoWrapper& tsInfoWrapper,
                                          const Itin* itin = nullptr);

  void initializeTransfersInfoSegMarkup(PricingTrx& trx,
                                        DataHandle& localDataHandle,
                                        std::vector<TransfersInfoSegMarkup*>& trInfoSegMarkup,
                                        const TransfersInfo1& trInfo,
                                        const std::vector<TransfersInfoSeg1*>& segs,
                                        const TravelSegMarkupContainer& tsmCon,
                                        bool& allSegsGeoApplOK,
                                        bool& allSegsGeoApplNot,
                                        bool& geoRestrictionsExist);

  ProcessingResult processTransfers(PricingTrx& trx,
                                    TravelSegMarkupContainer& tsmProcOrder,
                                    TravelSegMarkupContainer& tsmPUOrder,
                                    const TransfersInfoWrapper& trInfoWrapper,
                                    const FarePath* fp,
                                    const PricingUnit* pu,
                                    const FareMarket* fm,
                                    const PaxTypeFare* ptf,
                                    std::string& failReason,
                                    const FareUsage* fu);

  ProcessingResult processFareBreakSurfaceRestrictions(PricingTrx& trx,
                                                       const TransfersInfo1& trInfo,
                                                       const FareMarket& fm,
                                                       std::string* failReason);

  ProcessingResult processEmbeddedSurfaceRestrictions(PricingTrx& trx,
                                                      const TransfersInfo1& trInfo,
                                                      const FareMarket& fm,
                                                      std::string* failReason);

  ProcessingResult processSurfaceSectorRestrictions(PricingTrx& trx,
                                                    const TransfersInfo1& trInfo,
                                                    const FareMarket& fm,
                                                    std::string* failReason);

  ProcessingResult processSurcharges(PricingTrx& trx,
                                     TravelSegMarkupContainer& tsmCon,
                                     const TransfersInfoWrapper& trInfoWrapper,
                                     const TransfersInfo1& trInfo,
                                     const FareUsage& fareUsage,
                                     const FarePath& farePath);

  ProcessingResult identifyTransfers(PricingTrx& trx,
                                     TravelSegMarkupContainer& tsmCon,
                                     const TransfersInfo1& trInfo,
                                     const TransfersInfoWrapper& trInfoWrapper,
                                     const FareUsage* const fareUsage);

  void identifyGateways(PricingTrx& trx,
                        TravelSegMarkupContainer& tsmCon,
                        const TransfersInfo1& trInfo,
                        const PricingUnit* pu,
                        const FareMarket* fm);

  void identifyGateways(PricingTrx& trx,
                        TravelSegMarkupContainer& tsmCon,
                        const PricingUnit* pu,
                        const FareMarket* fm);

  void markGateways(PricingTrx& trx, TravelSegMarkupContainer& tsmCon, const FareMarket& fm);

  void setStopType(TravelSegMarkup& tsm, const TransfersInfo1& trInfo);

  void setTransferType(TravelSegMarkup& tsm, const TransfersInfo1& trInfo);

  ProcessingResult processTransferTypeRestrictions(TravelSegMarkupContainer& tsmCon,
                                                   const TransfersInfoWrapper& trInfoWrapper,
                                                   const PaxTypeFare* ptf);

  ProcessingResult processTransferInfoSegs(PricingTrx& trx,
                                           TravelSegMarkupContainer& tsmProcOrder,
                                           TravelSegMarkupContainer& tsmPUOrder,
                                           const TransfersInfoWrapper& trInfoWrapper,
                                           const FarePath* fp,
                                           const PricingUnit* pu,
                                           const FareMarket* fm,
                                           const PaxTypeFare* ptf,
                                           std::string& failReason,
                                           const FareUsage* fu);

  bool isLocTypeWithNoLocData(const tse::TransfersInfoSeg1& tis) const;
  void setRecurringSegCounters(const tse::PricingTrx& trx,
                               const FareMarket* fm,
                               const VendorCode& vendorOfFare,
                               const Loc& transferLoc,
                               TransfersInfoSegMarkup& trISM);

  bool checkImplicitInfoSegMatch(TravelSegMarkup& tsm, bool allSegsGeoApplNot);

  MatchResult checkInfoSegTransferType(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg);

  MatchResult checkInfoSegStopConn(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg);

  bool isDomestic(const TravelSeg& ts) const;
  MatchResult checkInfoSegRestriction(TravelSegMarkup& tsm, const TransfersInfoSeg1& trInfoSeg);

  bool processInfoSegCarrier(PricingTrx& trx,
                             TravelSegMarkup& tsm,
                             TransferInfoSegMatch& tiSegMatch,
                             const TransfersInfo1& trInfo,
                             const TransfersInfoSeg1& trInfoSeg,
                             const VendorCode& vendorOfFare);

  MatchResult checkInfoSegCarrier(PricingTrx& trx,
                                  TravelSegMarkup& tsm,
                                  const TransfersInfo1& trInfo,
                                  const TransfersInfoSeg1& trInfoSeg,
                                  MatchResult& inCarrierMatch,
                                  MatchResult& outCarrierMatch,
                                  const VendorCode& vendorOfFare);

  MatchResult checkInfoSegCarrierIn(PricingTrx& trx,
                                    const CarrierCode& carrier,
                                    const TransfersInfo1& trInfo,
                                    const TransfersInfoSeg1& trInfoSeg,
                                    const VendorCode& vendorOfFare);

  MatchResult checkInfoSegCarrierOut(PricingTrx& trx,
                                     const CarrierCode& carrier,
                                     const TransfersInfo1& trInfo,
                                     const TransfersInfoSeg1& trInfoSeg,
                                     const VendorCode& vendorOfFare);

  MatchResult checkCarrierApplicationTable(PricingTrx& trx,
                                           const CarrierCode& carrier,
                                           const uint32_t carrierAppTblNo,
                                           const TransfersInfo1& trInfo,
                                           const VendorCode& vendorOfFare);

  bool checkInfoSegDirectionality(TravelSegMarkup& tsm,
                                  TransfersInfoSegMarkup& trISM,
                                  bool& processNextInfoSeg,
                                  bool& failed,
                                  std::string& shortFailReason,
                                  std::string& failReason);

  bool checkInfoSegBetweenGeo(PricingTrx& trx,
                              TravelSegMarkup& tsm,
                              TransfersInfoSegMarkup& trISM,
                              const TransfersInfo1& trInfo,
                              const TravelSegMarkupContainer& tsmCon,
                              MatchResult& geoMatch,
                              bool& processNextInfoSeg,
                              bool& failed);

  bool checkInfoSegGeo(PricingTrx& trx,
                       TravelSegMarkup& tsm,
                       TransfersInfoSegMarkup& trISM,
                       const TransfersInfo1& trInfo,
                       std::vector<TransfersInfoSegMarkup*>& trISMVector,
                       const FarePath* fp,
                       const PricingUnit* pu,
                       const FareMarket* fm,
                       const PaxTypeFare* ptf,
                       bool processChargeCalcOnly,
                       MatchResult& geoMatch,
                       bool& processNextInfoSeg,
                       bool& failed,
                       std::string& failReason,
                       const VendorCode& vendorOfFare);

  bool checkInfoSegMainGeo(PricingTrx& trx,
                           TravelSegMarkup& tsm,
                           TransfersInfoSegMarkup& trISM,
                           const TransfersInfo1& trInfo,
                           const FarePath* fp,
                           const PricingUnit* pu,
                           const FareMarket* fm,
                           const PaxTypeFare* ptf,
                           MatchResult& geoMatch,
                           bool& processNextInfoSeg,
                           bool& failed,
                           std::string& failReason,
                           const VendorCode& vendorOfFare);

  bool checkInfoSegAltGeo(PricingTrx& trx,
                          TravelSegMarkup& tsm,
                          TransfersInfoSegMarkup& trISM,
                          const TransfersInfo1& trInfo,
                          std::vector<TransfersInfoSegMarkup*>& trISMVector,
                          const FarePath* fp,
                          const PricingUnit* pu,
                          const FareMarket* fm,
                          const PaxTypeFare* ptf,
                          bool processChargeCalcOnly,
                          MatchResult& geoMatch,
                          bool& processNextInfoSeg,
                          bool& failed,
                          std::string& failReason,
                          const TransfersInfoSeg1*& matchedInfoSeg,
                          const VendorCode& vendorOfFare);

  bool processTSI(PricingTrx& trx,
                  TSITravelSegMap& tsiTravelSegMap,
                  const TransfersInfoSeg1* trInfoSeg,
                  const FarePath* fp,
                  const PricingUnit* pu,
                  const FareMarket* fm,
                  const PaxTypeFare* ptf,
                  std::string& failReason);

  ProcessingResult checkMinTransfers(PricingTrx& trx,
                                     const TravelSegMarkupContainer& tsmCon,
                                     const TransfersInfo1& trInfo,
                                     const PaxTypeFare* ptf,
                                     std::string& failReason);

  ProcessingResult checkMaxTransfers(PricingTrx& trx,
                                     const TransfersInfoWrapper& trInfoWrapper,
                                     const TravelSegMarkupContainer& tsmCon,
                                     std::string& failReason);

  ProcessingResult checkMaxTransfersInOut(PricingTrx& trx,
                                          TravelSegMarkupContainer& tsmCon,
                                          const TransfersInfoWrapper& trInfoWrapper,
                                          const PaxTypeFare* ptf,
                                          std::string& failReason);

  MatchResult matchGeo(PricingTrx& trx,
                       const Loc* loc,
                       const TransfersInfo1& trInfo,
                       const TransfersInfoSeg1& trInfoSeg,
                       const VendorCode& vendorOfFare) const;

  bool checkIsInLoc(PricingTrx& trx,
                    const Loc& locToCheck,
                    const LocKey& locToCheckWithin,
                    const VendorCode& vendor) const;

  bool isFareBreakSurfacePermitted(PricingTrx& trx,
                                   const TransfersInfo1& trInfo,
                                   const FareMarket& fm) const;

  uint32_t getMaxNumEmbeddedSurfaces(PricingTrx& trx, const TransfersInfo1& trInfo) const;

  bool isSurfaceSector(const FareMarket& fm, const TravelSeg& ts) const;

  bool matchSurfaceIntlDom(const TravelSeg& ts, const SurfaceTransfersInfo& sti) const;

  bool matchSurfaceOrigDest(const TravelSeg& ts,
                            const FareMarket& fm,
                            const SurfaceTransfersInfo& sti) const;

  bool matchSurfaceGeo(PricingTrx& trx, const TravelSeg& ts, const SurfaceTransfersInfo& sti) const;

  bool matchSurfaceGeoFareBreak(PricingTrx& trx,
                                const TravelSeg& ts,
                                const SurfaceTransfersInfo& sti,
                                bool matchFirst) const;

  bool hasGeoLocationsSpecified(const SurfaceTransfersInfo& sti) const;

  int16_t strToInt(const std::string& str, bool& unlimited) const;

  void printDiagnostics(DiagCollector& diag,
                        const TransfersInfo1& trInfo,
                        const TravelSegMarkupContainer& tsmCon,
                        const FareMarket& fm) const;

  void printDiagnostics(DiagCollector& diag,
                        const TransfersInfo1& trInfo,
                        const TravelSegMarkupContainer& tsmCon,
                        const PricingUnit& pu) const;

  void printDiagHeader(PricingTrx& trx,
                       DiagCollector& diag,
                       const TransfersInfoWrapper& trInfoWrapper,
                       const TransfersInfo1& trInfo,
                       const FareMarket* fm) const;

  void printDiagTsmDetail(DiagCollector& diag,
                          const TransfersInfo1& trInfo,
                          const TravelSegMarkup& tsm,
                          const TravelSeg* ts) const;

  void
  printDiagTravelSeg(DiagCollector& diag, const TravelSeg* ts, const int16_t segmentNumber) const;

  void printSurfaceValidationResult(const std::string& message,
                                    const TravelSeg* ts,
                                    std::string& out) const;

  std::string getMatchResultText(const MatchResult& matchResult) const;

  virtual bool isSameVCTRSequence(const FareUsage* fu,
                                  const FareUsage* ruleFu,
                                  const TransfersInfoWrapper& trInfoWrapper);

  virtual bool isFCvsPU(const TransfersInfo1& trInfo);

  bool isRecurringSegForPU(const TransfersInfo1& trInfo, const TransfersInfoSeg1& seg);

  bool ignoreRecurringSegForRtw(const TransfersInfoSeg1& seg) const;
  bool isNumTransfersSpecified(const std::string& str) const;

private:
  DataHandle _dataHandle;
  bool _needSurfValidationForFP = false;
  bool _excludeSurfaceBytes = false; // Use only for Dynamic phase from HIP/Net Remit fare selection
  bool _isRtw = false;
};

} // namespace tse

