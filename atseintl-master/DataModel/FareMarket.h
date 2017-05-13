//-------------------------------------------------------------------
//
//  File:        FareMarket.h
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: One Fare Market
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/18/04 - VN - PaxTypeCortege added.
//        05/17/04 - DEV - Added side trip travel seg
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

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/SmallBitSet.h"
#include "Common/Thread/TSEFastMutex.h"
#include "Common/Thread/TSELockGuards.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "DataModel/CopyablePtr.h"
#include "DataModel/FareMarketCurrencyKey.h"
#include "DataModel/FnRecord2Key.h"
#include "DataModel/GfrRecord2Key.h"
#include "DataModel/IbfAvailabilityEnumPolicies.h"
#include "DataModel/IbfAvailabilityStatusAccumulators.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/NegPaxTypeFareDataComparator.h"
#include "DataModel/NegPTFBucketContainer.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeBucket.h"
#include "DataModel/PaxTypeFareResultKey.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DBAForwardDecl.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>

#include <bitset>
#include <map>
#include <memory>
#include <limits>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace tse
{
class CarrierPreference;
class Cat31FareBookingCodeValidator;
class ClassOfService;
class DataHandle;
class Fare;
class FareCompInfo;
class FareMarketCurrencyKey;
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;
class Loc;
class MileageInfo;
class PaxType;
class PaxTypeFare;
class PricingTrx;
class RexBaseTrx;
class TravelSeg;

typedef std::pair<std::vector<PaxTypeFare*>::const_iterator,
                  std::vector<PaxTypeFare*>::const_iterator> PTFRange;

typedef std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys>::iterator
CurrencyCacheI;
// typedef boost::mutex FMMutex;
typedef TSEFastMutex FMMutex;
// typedef boost::mutex::scoped_lock FMScopedLock;
typedef TSEGuard FMScopedLock;
struct SOPUsage
{
  bool applicable_ = false;
  Itin* itin_ = nullptr;
  int origSopId_ = -1;
  int startSegment_ = -1;
  int endSegment_ = -1;
  int flightsFirstProcessedIn_ = -1;
  std::vector<std::vector<ClassOfService*>*> cos_;
};
typedef std::vector<SOPUsage> SOPUsages;
typedef std::map<ItinIndex::Key, SOPUsages> ApplicableSOP;

struct OWRTFilter
{
  OWRTFilter(Indicator selectedOWRT) : _selectedOWRT(selectedOWRT) {}
  bool operator()(PaxTypeFare* pxFare);

private:
  Indicator _selectedOWRT;
};

class FareMarket
{
public:
  typedef std::shared_ptr<NegPTFBucketContainer<NegPaxTypeFareDataComparator>> NegFaresBuckets;
  typedef std::vector<NegFaresBuckets> AllNegFaresBucketsVec;
  using MultiPaxUniqueFareBasisCodes = std::unordered_set<std::string>;
  // for backwards compatibility

  enum FareTypeGroupState
  { Business = 0x00000001,
    Economy = 0x00000002,
    EconomyExcursion = 0x00000004,
    EconomyAdvancePurchase = 0x00000008,
    EconomyInstantPurchase = 0x00000010,
    First = 0x00000020,
    PremiumEconomy = 0x00000040,
    PremiumFirst = 0x00000080,
    Promotional = 0x00000100,
    Special = 0x00000200 };

  typedef SmallBitSet<uint16_t, FareTypeGroupState> FareTypeGroupStatus;
  typedef std::map<Itin*, bool> FMItinMap;
  typedef std::map<Itin*, bool>::const_iterator FMItinMapCI;
  typedef std::map<BrandCode, IbfErrorMessageFmAcc> BrandAvailability;
  typedef skipper::BrandCodeDirection BrandCodeDirection;
  typedef std::map<BrandCodeDirection, IbfErrorMessageFmAcc> BrandDirectionAvailability;

  enum SOL_FM_TYPE : int8_t
  { SOL_FM_NOTAPPLICABLE = -1,
    SOL_FM_LOCAL,
    SOL_FM_THRU };

  enum SOL_COMPONENT_DIRECTION
  { SOL_COMPONENT_ORIGIN = 0,
    SOL_COMPONENT_DESTINATION,
    SOL_COMPONENT_UNDEFINED };

  // These enums are for a ServiceStatus bit for SinglePass
  // shopping.  They enable us to mark which FareMarkets have
  // already been processed by a service so they are skipped
  // in the 'second call' phase
  enum ServiceStatus
  { ItinAnalyzer = 0x01,
    FareCollector = 0x02,
    FareValidator = 0x04,
    RexFareSelector = 0x08,
    RexFareRulePreparor = 0x10 };

  enum SavedResultMatchFlags
  { ChkNothing = 0x00,
    ChkDirectionality = 0x01,
    ChkPaxTypeCode = 0x02,
    ChkPsgAge = 0x04 };

  boost::timed_mutex _esvMutex;

  class FareMarketSavedGfrResult
  {
  public:
    struct Result
    {
      Record3ReturnTypes _ret = Record3ReturnTypes::NOTPROCESSED;
      union
      {
        bool _needCat13AccFareBreak;
        struct
        {
          bool _matchedCorpId;
          bool _isWebFare;
          bool _foundCat1R3NoADT;
          bool _foundCat1R3ChkAge;
        }; // eligibility result
        bool _penaltyRestInd;
        struct
        {
          bool _webFare;
          bool _cat15SoftPass;
          bool _cat15SecurityFail;
          bool _cat15FOPExist;
          bool _cat15NationFRForCWT;
          bool _cat15HasSecurity;
          bool _cat15HasEtktWarning;
        };
      };
      bool _checkFareRuleAltGenRule;
      bool _generalRuleEnabled;
      bool _readRecord0;
      TariffNumber _genTariffNumber;
      RuleNumber _genRuleNumber;
      const PaxTypeFare* _ptfResultFrom;
      DateTime _latestTktDT;
      DateTime _earliestTktDT;
      bool _reProcessCat05NoMatch;
      Result()
        : _webFare(false),
          _cat15SoftPass(false),
          _cat15SecurityFail(false),
          _cat15FOPExist(false),
          _cat15NationFRForCWT(false),
          _cat15HasSecurity(false),
          _cat15HasEtktWarning(false),
          _checkFareRuleAltGenRule(false),
          _generalRuleEnabled(false),
          _readRecord0(false),
          _genTariffNumber(0),
          _ptfResultFrom(nullptr),
          _reProcessCat05NoMatch(false)
      {
      }
    };

    typedef std::map<PaxTypeFareResultKey, const Result*> ResultMap;

    class Results
    {
    public:
      Results() : _matchFlags(ChkNothing) {}

      // copy semantics -- needed because the mutex is
      // not copyable
      Results(const Results& r) : _matchFlags(r._matchFlags), _resultMap(r._resultMap) {}

      Results& operator=(const Results& r)
      {
        _matchFlags = r._matchFlags;
        _resultMap = r._resultMap;
        return *this;
      }

      bool directional() const { return _matchFlags.isSet(ChkDirectionality); }
      void setDirectional(const bool isDirectional)
      {
        _matchFlags.set(ChkDirectionality, isDirectional);
      }
      bool chkPsgType() const { return _matchFlags.isSet(ChkPaxTypeCode); }
      void setChkPsgType(const bool doesChk) { _matchFlags.set(ChkPaxTypeCode, doesChk); }
      bool chkPsgAge() const { return _matchFlags.isSet(ChkPsgAge); }
      void setChkPsgAge(const bool doesChk) { _matchFlags.set(ChkPsgAge, doesChk); }
      ResultMap& resultMap() { return _resultMap; }
      FMMutex& resultMapMutex() { return _resultMapMutex; }

    private:
      SmallBitSet<uint8_t, SavedResultMatchFlags> _matchFlags;
      ResultMap _resultMap;
      FMMutex _resultMapMutex;
    };

    GeneralFareRuleInfoVec*& gfrList() { return _gfrList; }
    std::vector<FareMarket::FareMarketSavedGfrResult::Results>& resultVec() { return _resultVec; }
    const std::vector<FareMarket::FareMarketSavedGfrResult::Results>& resultVec() const
    {
      return _resultVec;
    }

  private:
    GeneralFareRuleInfoVec* _gfrList;
    std::vector<Results> _resultVec;
  };

  class FareMarketSavedFnResult
  {
  public:
    struct Result
    {
      Record3ReturnTypes _ret;
      union
      {
        bool _needCat13AccFareBreak;
        bool _penaltyRestInd;
        struct
        {
          bool _cat15SoftPass;
          bool _cat15SecurityFail;
          bool _cat15FOPExist;
          bool _cat15NationFRForCWT;
          bool _cat15HasSecurity;
          bool _cat15HasEtktWarning;
          bool _cat15HasT996FT;
        };

        struct
        {
          std::map<uint16_t, const DateTime*>* _cat14NVAData;
        };
      };
      const PaxTypeFare* _ptfResultFrom;
      const Fare* _fare;
      DateTime _latestTktDT;
      DateTime _earliestTktDT;

      Result()
        : _ret(NOTPROCESSED),
          _cat15SoftPass(false),
          _cat15SecurityFail(false),
          _cat15FOPExist(false),
          _cat15NationFRForCWT(false),
          _cat15HasSecurity(false),
          _cat15HasEtktWarning(false),
          _cat15HasT996FT(false),
          _ptfResultFrom(nullptr),
          _fare(nullptr)
      {
      }
    };

    typedef std::map<PaxTypeFareResultKey, const Result*> ResultMap;

    class Results
    {
    public:
      Results() : _matchFlags(ChkNothing) {}

      // copy semantics -- needed because the mutex is
      // not copyable
      Results(const Results& r) : _matchFlags(r._matchFlags), _resultMap(r._resultMap) {}

      Results& operator=(const Results& r)
      {
        _matchFlags = r._matchFlags;
        _resultMap = r._resultMap;
        return *this;
      }

      bool directional() const { return _matchFlags.isSet(ChkDirectionality); }
      void setDirectional(const bool isDirectional)
      {
        _matchFlags.set(ChkDirectionality, isDirectional);
      }
      bool chkPsgType() const { return _matchFlags.isSet(ChkPaxTypeCode); }
      void setChkPsgType(const bool doesChk) { _matchFlags.set(ChkPaxTypeCode, doesChk); }
      bool chkPsgAge() const { return _matchFlags.isSet(ChkPsgAge); }
      void setChkPsgAge(const bool doesChk) { _matchFlags.set(ChkPsgAge, doesChk); }
      ResultMap& resultMap() { return _resultMap; }
      FMMutex& resultMapMutex() { return _resultMapMutex; }

    private:
      SmallBitSet<uint8_t, SavedResultMatchFlags> _matchFlags;
      ResultMap _resultMap;
      FMMutex _resultMapMutex;
    };

    FootNoteCtrlInfoVec*& fnCtrlList() { return _fnCtrlList; }
    std::vector<FareMarket::FareMarketSavedFnResult::Results>& resultVec() { return _resultVec; }
    const std::vector<FareMarket::FareMarketSavedFnResult::Results>& resultVec() const
    {
      return _resultVec;
    }

  private:
    FootNoteCtrlInfoVec* _fnCtrlList;
    std::vector<FareMarket::FareMarketSavedFnResult::Results> _resultVec;
  };

  enum FareRetrievalFlags
  { RetrievNone = 0x00,
    RetrievHistorical = 0x01,
    RetrievKeep = 0x02,
    RetrievTvlCommence = 0x04,
    RetrievCurrent = 0x08,
    RetrievLastReissue = 0x10,
    MergeHistorical = 0x20,
    RetrievExpndKeep = 0x40 };

  struct RetrievalInfo
  {
    FareRetrievalFlags _flag;
    DateTime _date;

    bool keep() const { return (_flag & RetrievKeep) || (_flag & RetrievExpndKeep); }

    static RetrievalInfo* construct(const PricingTrx& trx,
                                    const DateTime& date,
                                    const FareMarket::FareRetrievalFlags& flag);
  };

  struct MultiAirportInfo
  {
    typedef std::vector<LocCode> Airports;

    const Airports& origin() const { return _origin; }
    Airports& origin() { return _origin; }

    const Airports& destination() const { return _destination; }
    Airports& destination() { return _destination; }

  private:
    Airports _origin;
    Airports _destination;
  };

  static std::string fareRetrievalFlagToStr(FareRetrievalFlags flag);

  typedef boost::unordered_map<GfrRecord2Key, FareMarketSavedGfrResult*, GfrRecord2Key::Hash>
  FMSavedGfrUMap;
  FMMutex& fmSavedGfrMapMutex() { return _fmSavedGfrMapMutex; }

  typedef boost::unordered_map<FnRecord2Key, FareMarketSavedFnResult*, FnRecord2Key::Hash>
  FMSavedFnUMap;
  FMMutex& fmSavedFnMapMutex() { return _fmSavedFnMapMutex; }

  FMMutex& fmReversedFMMutex() { return _fmReversedFMMutex; }

  FareMarket() = default;
  FareMarket(const std::vector<TravelSeg*>& travelSegs,
             const DateTime& travelDate,
             const Loc* origin,
             const Loc* destination,
             const LocCode& boardMultiCity,
             const LocCode& offMultiCity,
             uint16_t legIndex)
    : _origin(origin),
      _destination(destination),
      _boardMultiCity(boardMultiCity),
      _offMultiCity(offMultiCity),
      _travelSeg(travelSegs),
      _legIndex(legIndex),
      _travelDate(travelDate)
  {
  }
  FareMarket(const FareMarket&) = delete;
  FareMarket& operator=(const FareMarket&) = delete;

  virtual ~FareMarket() = default;
  bool operator==(const FareMarket& fareMarket) const;

  // initialization

  bool initialize(PricingTrx& trx);

  // access to members

  const Loc*& origin() { return _origin; }
  const Loc* origin() const { return _origin; }

  const Loc*& destination() { return _destination; }
  const Loc* destination() const { return _destination; }

  LocCode& boardMultiCity() { return _boardMultiCity; }
  const LocCode& boardMultiCity() const { return _boardMultiCity; }

  LocCode& offMultiCity() { return _offMultiCity; }
  const LocCode& offMultiCity() const { return _offMultiCity; }

  void setGlobalDirection(GlobalDirection dir) { _globalDirection = dir; }
  const GlobalDirection getGlobalDirection() const { return _globalDirection; }

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType& geoTravelType() const { return _geoTravelType; }

  void setFltTrkIndicator(bool value) { _statusBits.set(FLT_TRX_INDICATOR, value); }
  bool fltTrkIndicator() const { return _statusBits[FLT_TRX_INDICATOR]; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  const CarrierPreference*& governingCarrierPref() { return _governingCarrierPref; }

  const CarrierPreference* governingCarrierPref() const { return _governingCarrierPref; }

  TravelSeg*& primarySector() { return _primarySector; }
  const TravelSeg* primarySector() const { return _primarySector; }

  void setFlowMarket(bool value) { _statusBits.set(FLOW_MARKET, value); }
  bool flowMarket() const { return _statusBits[FLOW_MARKET]; }

  void setHasAllMarriedSegs(bool value) { _statusBits.set(HAS_ALL_MARRIED_SEGMENTS, value); }
  bool hasAllMarriedSegs() const { return _statusBits[HAS_ALL_MARRIED_SEGMENTS]; }

  std::vector<bool>& availBreaks() { return _availBreaks; }
  const std::vector<bool>& availBreaks() const { return _availBreaks; }

  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }
  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }

  FareMarket*& reversedFareMarket() { return _reversedFareMarket; }
  FareMarket* const& reversedFareMarket() const { return _reversedFareMarket; }

  std::vector<std::vector<TravelSeg*>>& sideTripTravelSeg() { return _sideTripTravelSeg; }
  const std::vector<std::vector<TravelSeg*>>& sideTripTravelSeg() const
  {
    return _sideTripTravelSeg;
  }

  std::set<TravelSeg*>& stopOverTravelSeg() { return _stopOverTravelSeg; }
  const std::set<TravelSeg*>& stopOverTravelSeg() const { return _stopOverTravelSeg; }

  std::vector<PaxTypeBucket>& paxTypeCortege() { return _paxTypeCortege; }
  const std::vector<PaxTypeBucket>& paxTypeCortege() const { return _paxTypeCortege; }

  PaxTypeBucket* paxTypeCortege(const PaxType* requestedPaxType);
  const PaxTypeBucket* paxTypeCortege(const PaxType* requestedPaxType) const;

  bool hasAnyFareValid() const;

  //    std::vector<PaxTypeBucket>& paxTypeRec8Cortege() { return _paxTypeRec8Cortege; }
  //    const std::vector<PaxTypeBucket>& paxTypeRec8Cortege() const { return _paxTypeRec8Cortege;
  // }

  void setCat19PaxFlags(PaxType* pt);

  bool isChildNeeded() const { return _statusBits[IS_CHILD_NEEDED]; }
  void setChildNeeded(bool value) { _statusBits.set(IS_CHILD_NEEDED, value); }
  bool isInfantNeeded() const { return _statusBits[IS_INFANT_NEEDED]; }
  void setInfantNeeded(bool value) { _statusBits.set(IS_INFANT_NEEDED, value); }
  bool isInfNeeded() const { return _statusBits[IS_INF_NEEDED]; }
  void setInfNeeded(bool value) { _statusBits.set(IS_INF_NEEDED, value); }
  bool bypassCat19FlagsSet() const { return _statusBits[BYPASS_CAT19_FLAG_SET]; }
  void setBypassCat19FlagsSet(bool value) { _statusBits.set(BYPASS_CAT19_FLAG_SET, value); }
  bool isJcb() const { return _statusBits[IS_JCB]; }
  void setJcb(bool value) { _statusBits.set(IS_JCB, value); }

  int getValidForPOFaresCount() const;

  void invalidateAllPaxTypeFaresForRetailer(PricingTrx& trx);

  std::vector<PaxTypeFare*>& allPaxTypeFare() { return _allPaxTypeFare; }
  const std::vector<PaxTypeFare*>& allPaxTypeFare() const { return _allPaxTypeFare; }

  std::vector<std::pair<Fare*, bool> >& footNoteFailedFares() { return _footNoteFailedFares; }
  const std::vector<std::pair<Fare*, bool> >& footNoteFailedFares() const { return _footNoteFailedFares; }

  std::vector<PaxTypeFare*>& footNoteFailedPaxTypeFares() { return _footNoteFailedPaxTypeFares; }
  const std::vector<PaxTypeFare*>& footNoteFailedPaxTypeFares() const { return _footNoteFailedPaxTypeFares; }

  PTFRange getPTFRange(TariffNumber tariff, DataHandle& dataHandle);
  void resetPTFRangeImpl();

  SmallBitSet<uint8_t, FMTravelBoundary>& travelBoundary() { return _travelBoundary; }
  const SmallBitSet<uint8_t, FMTravelBoundary>& travelBoundary() const { return _travelBoundary; }

  ErrorResponseException::ErrorResponseCode& failCode() { return _failCode; }
  const ErrorResponseException::ErrorResponseCode failCode() const { return _failCode; }

  std::vector<std::vector<ClassOfService*>*>& classOfServiceVec() { return _classOfServiceVec; }
  const std::vector<std::vector<ClassOfService*>*>& classOfServiceVec() const
  {
    return _classOfServiceVec;
  }

  std::vector<CurrencyCode>& inBoundAseanCurrencies() { return _inBoundAseanCurrencies; }
  const std::vector<CurrencyCode>& inBoundAseanCurrencies() const
  {
    return _inBoundAseanCurrencies;
  }

  std::vector<CurrencyCode>& outBoundAseanCurrencies() { return _outBoundAseanCurrencies; }
  const std::vector<CurrencyCode>& outBoundAseanCurrencies() const
  {
    return _outBoundAseanCurrencies;
  }

  uint16_t getStartSegNum() const;
  uint16_t getEndSegNum() const;

  FMDirection& direction() { return _direction; }
  const FMDirection& direction() const { return _direction; }

  void setBreakIndicator(bool value) { _statusBits.set(BREAK_INDICATOR, value); }
  bool breakIndicator() const { return _statusBits[BREAK_INDICATOR]; }

  uint16_t& legIndex() { return _legIndex; }
  const uint16_t& legIndex() const { return _legIndex; }

  void setCombineSameCxr(bool value) { _statusBits.set(COMBINE_SAME_CARRIER, value); }
  bool combineSameCxr() const { return _statusBits[COMBINE_SAME_CARRIER]; }

  void setValidateFareGroup(bool value) { _statusBits.set(VALIDATE_FARE_GROUP, value); }
  bool validateFareGroup() const { return _statusBits[VALIDATE_FARE_GROUP]; }

  DateTime& travelDate() { return _travelDate; }
  const DateTime& travelDate() const { return _travelDate; }

  DateTime& ruleApplicationDate() { return _ruleApplicationDate; }
  const DateTime& ruleApplicationDate() const { return _ruleApplicationDate; }

  const std::string getDirectionAsString() const
  {
    if (_direction == FMDirection::INBOUND)
      return ".IN.";

    if (_direction == FMDirection::OUTBOUND)
      return ".OUT.";

    return ".UNKNWN.";
  };

  MileageInfo*& mileageInfo(bool isOutbound);
  const MileageInfo* mileageInfo(bool isOutbound) const;

  std::string& fareBasisCode() { return _fareBasisCode; }
  const std::string& fareBasisCode() const { return _fareBasisCode; }

  MultiPaxUniqueFareBasisCodes& getMultiPaxUniqueFareBasisCodes() const
  {
    TSE_ASSERT(_multiPaxUniqueFareBasisCodes);
    return *_multiPaxUniqueFareBasisCodes;
  }

  bool isMultiPaxUniqueFareBasisCodes() const { return _multiPaxUniqueFareBasisCodes != nullptr; }

  void createMultiPaxUniqueFareBasisCodes()
  {
    _multiPaxUniqueFareBasisCodes.reset(new MultiPaxUniqueFareBasisCodes);
  }

  char& fbcUsage() { return _fbcUsage; }
  const char& fbcUsage() const { return _fbcUsage; }

  std::string& fareCalcFareAmt() { return _fareCalcFareAmt; }
  const std::string& fareCalcFareAmt() const { return _fareCalcFareAmt; }

  bool useDummyFare() const { return !_fareBasisCode.empty() && !_fareCalcFareAmt.empty(); }

  std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys>& currencies()
  {
    return _currencies;
  }
  const std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys>& currencies() const
  {
    return _currencies;
  }

  SmallBitSet<uint8_t, ServiceStatus>& serviceStatus() { return _serviceStatus; }
  const SmallBitSet<uint8_t, ServiceStatus>& serviceStatus() const { return _serviceStatus; }

  bool getMergedAvailability() const { return _statusBits[MERGED_AVAILABILITY]; }
  void setMergedAvailability(bool mergedAvailability)
  {
    _statusBits.set(MERGED_AVAILABILITY, mergedAvailability);
  }

  bool isEsvThruMarket() const { return _statusBits[IS_ESV_THRU_MARKET]; }
  void setEsvThtuMarket(bool isEsvThruMarket)
  {
    _statusBits.set(IS_ESV_THRU_MARKET, isEsvThruMarket);
  }

  bool isThroughFarePrecedenceNGS() const { return _statusBits[THROUGH_FARE_PRECEDENCE_NGS]; }
  void setThroughFarePrecedenceNGS(bool f) { _statusBits.set(THROUGH_FARE_PRECEDENCE_NGS, f); }

  bool isShopped() const
  {
    for (const TravelSeg* seg : _travelSeg)
    {
      if (seg->isShopped())
        return true;
    }
    return false;
  }

  bool isNotShopped() const
  {
    for (const TravelSeg* seg : _travelSeg)
    {
      if (!seg->isShopped())
        return true;
    }
    return false;
  }

  bool isRequestedFareBasisInfoUseful() const
  {
    return std::any_of(_travelSeg.begin(),
                       _travelSeg.end(),
                       [](const TravelSeg* seg)
                       { return seg->isRequestedFareBasisInfoUseful(); });
  }

  bool isRequestedFareBasisValid(const TravelSeg::RequestedFareBasis& rfb,
                                 const PaxTypeFare& ptf,
                                 PricingTrx& trx) const
  {
    for (TravelSeg* seg : _travelSeg)
    {
      if (!seg->isRequestedFareBasisInfoUseful())
        continue;

      if (!seg->isRequestedFareBasisValid(rfb, ptf, trx))
        return false;
    }
    return true;
  }

  bool existValidFares() const;
  bool existRequestedFareBasisValidFares() const;
  bool isRoutingNotProcessed() const;
  bool isCategoryNotProcessed() const;
  bool isBookingCodeNotProcessed() const;

  bool areAllFaresFailingOnRules() const;
  bool areAllFaresFailingOnRouting() const;
  bool areAllFaresFailingOnBookingCodes() const;
  bool isTravelSegPresent(const TravelSeg* seg) const;

  const FareMarket::FareMarketSavedFnResult::Result*
  savedFnResult(const uint32_t categoryNumber,
                const PaxTypeFare& paxTypeFare,
                FareMarketSavedFnResult::Results& results);

  bool saveFnResult(DataHandle& dataHandle,
                    const uint32_t categoryNumber,
                    const PaxTypeFare& paxTypeFare,
                    const FareMarketSavedFnResult::Result* result,
                    FareMarketSavedFnResult::Results& results);

  const FareMarket::FareMarketSavedGfrResult::Result*
  savedGfrResult(const uint32_t categoryNumber,
                 const PaxTypeFare& paxTypeFare,
                 FareMarketSavedGfrResult::Results& results);

  bool saveGfrResult(DataHandle& dataHandle,
                     const uint32_t categoryNumber,
                     const PaxTypeFare& paxTypeFare,
                     const FareMarketSavedGfrResult::Result* result,
                     FareMarketSavedGfrResult::Results& results);

  void resetResultMap(const uint32_t categoryNumber);

  std::vector<FareMarketSavedGfrResult::Results>*
  saveGfrUMList(DataHandle& dataHandle,
                const VendorCode& vendor,
                const CarrierCode& carrier,
                const TariffNumber& tcrRuleTariff,
                const RuleNumber& ruleNumber,
                uint32_t category,
                const DateTime& travelDate,
                const DateTime& returnDate,
                const DateTime& ticketDate,
                const GeneralFareRuleInfoVec* gfrList);

  GeneralFareRuleInfoVec* getGfrList(const VendorCode& vendor,
                                     const CarrierCode& carrier,
                                     const TariffNumber& tcrRuleTariff,
                                     const RuleNumber& ruleNumber,
                                     uint32_t category,
                                     const DateTime& travelDate,
                                     const DateTime& returnDate,
                                     const DateTime& ticketDate,
                                     std::vector<FareMarketSavedGfrResult::Results>*& resultVec);
  void resetGfrResultUMap(uint32_t categor);
  std::vector<FareMarketSavedFnResult::Results>*
  saveFnCtlUMLst(DataHandle& dataHandle,
                 const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const TariffNumber& tcrRuleTariff,
                 const Footnote& footnote,
                 uint32_t category,
                 const DateTime& travelDate,
                 const DateTime& returnDate,
                 const DateTime& ticketDate,
                 const FootNoteCtrlInfoVec* fnCtrlList);
  FootNoteCtrlInfoVec* getFnCtrlList(const VendorCode& vendor,
                                     const CarrierCode& carrier,
                                     const TariffNumber& tcrRuleTariff,
                                     const Footnote& footnote,
                                     uint32_t category,
                                     const DateTime& travelDate,
                                     const DateTime& returnDate,
                                     const DateTime& ticketDate,
                                     std::vector<FareMarketSavedFnResult::Results>*& resultVec);
  void resetFnResultUMap(uint32_t category);

  CurrencyCode& indirectEquivAmtCurrencyCode() { return _indirectEquivAmtCurrencyCode; }
  const CurrencyCode& indirectEquivAmtCurrencyCode() const { return _indirectEquivAmtCurrencyCode; }

  const DateTime& retrievalDate() const
  {
    return _retrievalInfo ? _retrievalInfo->_date : DateTime::emptyDate();
  }
  const FareRetrievalFlags retrievalFlag() const
  {
    return _retrievalInfo ? _retrievalInfo->_flag : FareMarket::RetrievNone;
  }

  RetrievalInfo*& retrievalInfo() { return _retrievalInfo; }
  const RetrievalInfo* const retrievalInfo() const { return _retrievalInfo; }

  bool isChanged() const;
  bool isFlown() const;

  bool isLocalJourneyFare() const;
  bool isFlowJourneyFare() const;

  int setFCChangeStatus(const int16_t& pointOfChgSegOrder);
  int setFCChangeStatus(const int16_t& pointOfChgFirst, const int16_t& pointOfChgSecond);

  std::vector<FCChangeStatus>& asBookChangeStatusVec() { return _asBookChangeStatus; }
  const std::vector<FCChangeStatus>& asBookChangeStatusVec() const { return _asBookChangeStatus; }

  FCChangeStatus& changeStatus();
  const FCChangeStatus& changeStatus() const;

  FCChangeStatus& asBookChangeStatus();
  const FCChangeStatus& asBookChangeStatus() const;

  std::vector<FCChangeStatus>& changeStatusVec() { return _changeStatus; }
  const std::vector<FCChangeStatus>& changeStatusVec() const { return _changeStatus; }

  FareCompInfo*& fareCompInfo() { return _fareCompInfo; }
  const FareCompInfo* fareCompInfo() const { return _fareCompInfo; }

  RexBaseTrx*& rexBaseTrx() { return _rexBaseTrx; }
  const RexBaseTrx* rexBaseTrx() const { return _rexBaseTrx; }

  void setComponentValidationForCarrier(const uint32_t carrierKey)
  {
    _foundPTFForCarrier = &_foundPTFPerCarrier[carrierKey];
    _lastFareProcessedForCarrier = &_lastFareProcessedPerCarrier[carrierKey];
    _firstFareProcessedForCarrier = &_firstFareProcessedPerCarrier[carrierKey];
    _allPaxTypeFareProcessedForCarrier = &_allPaxTypeFareProcessedPerCarrier[carrierKey];
  }

  void setLastFareProcessed(int index)
  {
    if (_lastFareProcessedForCarrier)
      *_lastFareProcessedForCarrier = index;
    else
      _lastFareProcessed = index;
  }

  const int lastFareProcessed() const
  {
    if (_lastFareProcessedForCarrier)
      return *_lastFareProcessedForCarrier;
    return _lastFareProcessed;
  }

  void setFirstFareProcessed(int index)
  {
    if (_firstFareProcessedForCarrier)
      *_firstFareProcessedForCarrier = index;
    else
      _firstFareProcessed = index;
  }

  const int firstFareProcessed() const
  {
    if (_firstFareProcessedForCarrier)
      return *_firstFareProcessedForCarrier;
    return _firstFareProcessed;
  }

  void setFoundPTF(bool passed = true)
  {
    if (_foundPTFForCarrier)
      *_foundPTFForCarrier = passed;
    else
      _foundPTF = passed;
  }

  bool foundPTF() const
  {
    if (LIKELY(_foundPTFForCarrier))
      return *_foundPTFForCarrier;
    return _foundPTF;
  }

  std::vector<PaxTypeFare*>& allPaxTypeFareProcessed()
  {
    if (LIKELY(_allPaxTypeFareProcessedForCarrier))
      return *_allPaxTypeFareProcessedForCarrier;
    return _allPaxTypeFareProcessed;
  }

  void clearAllPaxTypeFareProcessed()
  {
    if (_allPaxTypeFareProcessedForCarrier)
      (*_allPaxTypeFareProcessedForCarrier).clear();
    else
      _allPaxTypeFareProcessed.clear();
  }

  void addProcessedPTF(PaxTypeFare* ptfPtr);

  bool specialRtgFound() const { return _statusBits[SPECIAL_RTG_FOUND]; }
  void setSpecialRtgFound(bool specialRtgFound)
  {
    _statusBits.set(SPECIAL_RTG_FOUND, specialRtgFound);
  }

  void setComparePaxTypeCode(bool value) { _statusBits.set(COMPARE_PAX_TYPE_CODE, value); }
  bool comparePaxTypeCode() const { return _statusBits[COMPARE_PAX_TYPE_CODE]; }

  std::map<std::string, bool> _webFareMatchSet; // key is vendor and cat-15 item no    to keep track
  // if web fare
  std::map<std::string, bool>& getWebFareMatchSet() { return _webFareMatchSet; }

  void setFareTypeGroupValid(NoPNRPricingTrx::FareTypes::FTGroup ftg);
  bool isFareTypeGroupValid(NoPNRPricingTrx::FareTypes::FTGroup ftg) const;

  void setExcItinIndex(uint16_t excItinIndex) { _excItinIndex = excItinIndex; }

  void clone(FareMarket& cloneObj) const;

  bool findFMCxrInTbl(PricingTrx::TrxType trxType,
                      const std::vector<CarrierApplicationInfo*>& carrierApplList) const;

  void recreateTravelSegments(DataHandle& dh);
  void setCarrierInAirSegments(const CarrierCode& carrier);


  void setOrigDestByTvlSegs();
  ApplicableSOP*& getApplicableSOPs() { return _applicableSOPs; }
  const ApplicableSOP* getApplicableSOPs() const { return _applicableSOPs; }
  void setFmTypeSol(SOL_FM_TYPE type);
  SOL_FM_TYPE getFmTypeSol() const;
  void setSolComponentDirection(SOL_COMPONENT_DIRECTION direction);
  SOL_COMPONENT_DIRECTION getSolComponentDirection() const;

  typedef boost::filter_iterator<OWRTFilter, std::vector<PaxTypeFare*>::iterator> owrt_iterator;
  owrt_iterator ow_begin()
  {
    return owrt_iterator(
        OWRTFilter(ONE_WAY_MAYNOT_BE_DOUBLED), _allPaxTypeFare.begin(), _allPaxTypeFare.end());
  }
  owrt_iterator ow_end()
  {
    return owrt_iterator(
        OWRTFilter(ONE_WAY_MAYNOT_BE_DOUBLED), _allPaxTypeFare.end(), _allPaxTypeFare.end());
  }
  owrt_iterator hrt_begin()
  {
    return owrt_iterator(
        OWRTFilter(ROUND_TRIP_MAYNOT_BE_HALVED), _allPaxTypeFare.begin(), _allPaxTypeFare.end());
  }
  owrt_iterator hrt_end()
  {
    return owrt_iterator(
        OWRTFilter(ROUND_TRIP_MAYNOT_BE_HALVED), _allPaxTypeFare.end(), _allPaxTypeFare.end());
  }

  PaxTypeFare* getCheapestOW()
  {
    if (_cheapestOW == nullptr)
    {
      if (ow_begin() != ow_end())
        _cheapestOW = *ow_begin();
    }
    return _cheapestOW;
  }
  PaxTypeFare* getCheapestHRT()
  {
    if (_cheapestHRT == nullptr)
    {
      if (hrt_begin() != hrt_end())
        _cheapestHRT = *hrt_begin();
    }
    return _cheapestHRT;
  }

  bool isDualGoverning() const { return _statusBits[DUAL_GOVERNING_FLAG]; }
  void setDualGoverningFlag(bool dualGovFlag) { _statusBits.set(DUAL_GOVERNING_FLAG, dualGovFlag); }

  void setRemoveOutboundFares(bool value) { _statusBits.set(REMOVE_OUTBOUND_FARES, value); }
  bool removeOutboundFares() const { return _statusBits[REMOVE_OUTBOUND_FARES]; }

  std::map<const TravelSeg*, LocCode>& boardMultiCities() { return _boardMultiCities; }
  const std::map<const TravelSeg*, LocCode>& boardMultiCities() const { return _boardMultiCities; }

  std::map<const TravelSeg*, LocCode>& offMultiCities() { return _offMultiCities; }
  const std::map<const TravelSeg*, LocCode>& offMultiCities() const { return _offMultiCities; }

  void setHasCat35Fare(bool hasCat35Fare) { _statusBits.set(HAS_CAT35_FARE, hasCat35Fare); }
  bool hasCat35Fare() const { return _statusBits[HAS_CAT35_FARE]; }
  void setMarketCurrencyPresent(PricingTrx& trx);

  void setThru(bool thru) { _statusBits.set(IS_THRU, thru); }
  bool isThru() const { return _statusBits[IS_THRU]; }

  bool isSimpleTrip() const { return _statusBits[SIMPLE_TRIP]; }
  void setSimpleTrip(bool simpleTrip) { _statusBits.set(SIMPLE_TRIP, simpleTrip); }

  bool hasDuplicates() const { return _statusBits[HAS_DUPLICATES]; }
  void setHasDuplicates(bool hasDup) { _statusBits.set(HAS_DUPLICATES, hasDup); }

  bool isKeepSoloFM() const { return _statusBits[KEEP_SOLO_FM]; }
  void setKeepSoloFM(bool keep) { _statusBits.set(KEEP_SOLO_FM, keep); }

  const MultiAirportInfo* getMultiAirportInfo() const { return _multiAirportInfo; }
  MultiAirportInfo* getMultiAirportInfo() { return _multiAirportInfo; }
  void setMultiAirportInfo(MultiAirportInfo* info) { _multiAirportInfo = info; }

  bool isHighTPMGoverningCxr() const { return _statusBits[HIGHEST_TPM_GOV_CXR]; }
  void setHighTPMGoverningCxr(bool highTPM) { _statusBits.set(HIGHEST_TPM_GOV_CXR, highTPM); }

  bool isFirstCrossingAndHighTpm() const { return _statusBits[FIRST_CROSSING_AND_HIGH_TPM]; }
  void setFirstCrossingAndHighTpm(bool both) { _statusBits.set(FIRST_CROSSING_AND_HIGH_TPM, both); }

  std::vector<int>& marketIDVec() { return _marketIDVec; }
  const std::vector<int>& marketIDVec() const { return _marketIDVec; }

  std::vector<int>& brandProgramIndexVec() { return _brandProgramIndexVec; }
  const std::vector<int>& brandProgramIndexVec() const { return _brandProgramIndexVec; }

  void addOriginRequestedForOriginalDirection(const LocCode& origin) {
    _originsRequested.insert(origin);
  }
  void addDestinationRequestedForReversedDirection(const LocCode& destination) {
    _destinationsRequested.insert(destination);
  }
  const std::set<LocCode>& getOriginsRequested() const { return _originsRequested; }
  const std::set<LocCode>& getDestinationsRequested() const { return _destinationsRequested; }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers; }
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers; }

  FMItinMap& fMItinMap() { return _fmItinMap; }
  const FMItinMap& fMItinMap() const { return _fmItinMap; }
  void addFMItinMap(Itin* itin, bool spanishFamilyDiscount);

  IbfErrorMessage getStatusForBrandWithoutDirection(const BrandCode& brandCode) const;
  IbfErrorMessage getStatusForBrand(const BrandCode& brandCode, Direction direction) const;

  void
  updateStatusForBrand(const BrandCode& brandCode, Direction direction, IbfErrorMessage status,
    bool isHardPass = false);

  //TODO(andrzej.fediuk) DIR: after merging _brandAvailability vectors backwardCompabilityOnly
  //should be removed. Right now it modifies behavior when direction BOTHWAYS is used.
  void forceStatusForBrand(const BrandCode& brandCode, Direction direction, IbfErrorMessage status,
    bool backwardCompabilityOnly = false);

  bool useHardPassBrandAvailabilityOnly(BrandCodeDirection brandInDirection) const
  {
    const BrandCodeDirection brandInBothDirection =
         BrandCodeDirection(brandInDirection.brandCode, Direction::BOTHWAYS);

    if (_hardPassAvailabilityOnly.find(brandInBothDirection) != _hardPassAvailabilityOnly.end())
      return true;

    if (_hardPassAvailabilityOnly.find(brandInDirection) != _hardPassAvailabilityOnly.end())
      return true;

    return false;
  }
  void setUseHardPassBrandAvailabilityOnly(const BrandCodeDirection& brandInDirection)
  {
    _hardPassAvailabilityOnly.insert(brandInDirection);
  }

  std::string toString() const;

  const AllNegFaresBucketsVec& getAllNegFaresBuckets() const { return _allNegFaresBuckets; }
  void addNegFaresBuckets(NegFaresBuckets& buckets) { _allNegFaresBuckets.push_back(buckets); }

  // Returns true if fare market contains at least one segment and all segments have the same
  // brand code or none of them has
  // Returns false if fare market contains at least one segment and segments have different
  // brand codes or some of them don't have brand code at all
  // Throws if Fare Market doesn't contain any segments
  bool isApplicableForPbb() const;

  // Returns true if all segments have the same not empty brand code
  // Returns false if all segments have empty brand code or segments have different
  // brand codes or some of them don't have brand code at all
  // Throws if Fare Market doesn't contain any segments
  bool hasBrandCode() const;

  // Returns BrandCode which is shared by all segments of fare market or an empty BrandCode
  // when brand codes of segments are not equal or all segments have empty brand code
  // Throws if Fare Market doesn't contain any segments or
  BrandCode getBrandCode() const;

  // Return true if both origin and destination of the fare market are within the same country
  // and the country is US or CA.
  bool isDomestic() const { return LocUtil::isDomestic(*origin(), *destination()); }

  // Return true if both origin and destination of the fare market are within the same country
  // and the country is not US or CA.
  bool isForeignDomestic() const { return LocUtil::isForeignDomestic(*origin(), *destination()); }

  // Return true if one of the origin or destination of the fare market is within US
  // and the other is within CA.
  bool isTransBorder() const { return LocUtil::isTransBorder(*origin(), *destination()); }

  // Return true if origin and destination of the fare market are within different countries and
  // the fare marker is not trans-border (ie. between US and CA)
  bool isInternational() const { return LocUtil::isInternational(*origin(), *destination()); }

  // Return true if the origin and destination of the fare market are in russian group (RU and XU).
  bool isWithinRussianGroup() const
  {
    return LocUtil::isRussianGroup(*origin()) && LocUtil::isRussianGroup(*destination());
  }

  std::string getHashKey() const;

  void setCat31RbdValidator(const Cat31FareBookingCodeValidator* v) { _cat31RbdValidator = v; }
  const Cat31FareBookingCodeValidator* getCat31RbdValidator() const { return _cat31RbdValidator; }

  bool isFullyFlownFareMarket() const;

  bool isSimilarItinMarket() const { return _statusBits[SIMIALR_ITIN_MARKET]; }
  void setSimilarItinMarket(const bool value = true)
  {
    _statusBits.set(SIMIALR_ITIN_MARKET, value);
  }

  bool doesSpanMoreThanOneLeg() const;

  bool hasTravelSeg(const TravelSeg* const tvlSeg) const;

  bool isFMInvalidByRbdByCabin() const { return _fmInvalidByRbdByCabin; }
  void setInvalidByRbdByCabin() { _fmInvalidByRbdByCabin = true; }

  void setBKSNotApplicapleIfBKSNotPASS();
  bool canValidateFBRBaseFares() const { return _canValidateFBRBaseFares; }
  void setCanValidateFBRBaseFares(bool value = true) { _canValidateFBRBaseFares = value; }

  bool isCmdPricing() const;

protected:
  const uint16_t& getExcItinIndex() const { return _excItinIndex; }

  // members
  const Loc* _origin = nullptr;
  const Loc* _destination = nullptr;

  LocCode _boardMultiCity;
  LocCode _offMultiCity;
  std::map<const TravelSeg*, LocCode> _boardMultiCities;
  std::map<const TravelSeg*, LocCode> _offMultiCities;

  GlobalDirection _globalDirection = GlobalDirection::XX;
  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;

  CarrierCode _governingCarrier;
  const CarrierPreference* _governingCarrierPref = nullptr;
  TravelSeg* _primarySector = nullptr;

  std::vector<bool> _availBreaks;

  FareTypeGroupStatus _fareTypeGroupStatus;

  // all travel seg included to the FareMarket

  std::vector<TravelSeg*> _travelSeg;

  std::vector<std::vector<TravelSeg*>> _sideTripTravelSeg;

  // StopOver segments by considering forced stop/conn, surface and fare break
  std::set<TravelSeg*> _stopOverTravelSeg;

  FareMarket* _reversedFareMarket = nullptr;
  // data assotiated with requested PaxType

  std::vector<PaxTypeBucket> _paxTypeCortege;

  // all PaxTypeFares regardless of PaxType
  std::vector<PaxTypeFare*> _allPaxTypeFare;
  std::vector<std::pair<Fare*, bool> > _footNoteFailedFares;
  std::vector<PaxTypeFare*> _footNoteFailedPaxTypeFares;

  SmallBitSet<uint8_t, FMTravelBoundary> _travelBoundary;

  ErrorResponseException::ErrorResponseCode _failCode = ErrorResponseException::NO_ERROR;

  std::vector<std::vector<ClassOfService*>*> _classOfServiceVec;

  FMDirection _direction = FMDirection::UNKNOWN;

  std::vector<CurrencyCode> _inBoundAseanCurrencies;
  std::vector<CurrencyCode> _outBoundAseanCurrencies;

  uint16_t _legIndex = 0;
  uint16_t _excItinIndex = 0;
  char _fbcUsage = COMMAND_PRICE_FBC; // command pricing/filter pricing/brand pricing

  MileageInfo* _mInfo = nullptr;
  MileageInfo* _mInfoOut = nullptr;

  std::string _fareBasisCode; // For Q<FareBase>
  CopyablePtr<MultiPaxUniqueFareBasisCodes> _multiPaxUniqueFareBasisCodes;
  // For Exchange pricing with specified FareAmount
  std::string _fareCalcFareAmt;

  std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys> _currencies;

  SmallBitSet<uint8_t, ServiceStatus> _serviceStatus;

  RetrievalInfo* _retrievalInfo = nullptr;

  DateTime _travelDate;
  DateTime _ruleApplicationDate;

  std::vector<FCChangeStatus> _changeStatus{FCChangeStatus::UK};
  std::vector<FCChangeStatus> _asBookChangeStatus{FCChangeStatus::UK};

  bool _foundPTF = true;
  bool* _foundPTFForCarrier = nullptr;
  VecMap<uint32_t, bool> _foundPTFPerCarrier;

  int _lastFareProcessed = -1;
  int* _lastFareProcessedForCarrier = nullptr;
  VecMap<uint32_t, int> _lastFareProcessedPerCarrier;

  int _firstFareProcessed = -1;
  int* _firstFareProcessedForCarrier = nullptr;
  VecMap<uint32_t, int> _firstFareProcessedPerCarrier;

  std::vector<PaxTypeFare*> _allPaxTypeFareProcessed;
  std::vector<PaxTypeFare*>* _allPaxTypeFareProcessedForCarrier = nullptr;
  VecMap<uint32_t, std::vector<PaxTypeFare*>> _allPaxTypeFareProcessedPerCarrier;

private:
  FMMutex _fmSavedGfrMapMutex;
  FMSavedGfrUMap _fmSavedGfrUMap;
  FMSavedFnUMap _fmSavedFnUMap;
  FMMutex _fmSavedFnMapMutex;
  FMMutex _fmReversedFMMutex;
  CurrencyCode _indirectEquivAmtCurrencyCode;

  FareCompInfo* _fareCompInfo = nullptr;
  RexBaseTrx* _rexBaseTrx = nullptr;

  PaxTypeFare* _cheapestOW = nullptr; // cheapest One Way fare
  PaxTypeFare* _cheapestHRT = nullptr; // cheapest Half Round Trip fare

  // for IS local fare markets
  ApplicableSOP* _applicableSOPs = nullptr;
  SOL_FM_TYPE _solFmType = SOL_FM_TYPE::SOL_FM_NOTAPPLICABLE;
  SOL_COMPONENT_DIRECTION _solComponentDirection{SOL_COMPONENT_DIRECTION::SOL_COMPONENT_UNDEFINED};

  class PTFRangeImpl* _ptfRangeImpl = nullptr;
  MultiAirportInfo* _multiAirportInfo = nullptr;

  std::vector<int> _marketIDVec;
  std::vector<int> _brandProgramIndexVec;
  std::set<LocCode> _originsRequested; //for ORIGINAL direction
  std::set<LocCode> _destinationsRequested; //for REVERSED direction

  std::vector<CarrierCode> _validatingCarriers;

  friend class FareMarketTest;

  boost::mutex _mutexFMItinMap;
  FMItinMap _fmItinMap;

  mutable boost::shared_mutex _mutexBrandAvailability;
  //TODO(andrzej.fediuk) DIR: refactor
  //Only BFA requests support multiple brands on one fare market (as a possible
  //solution). For this case we have to have availability for each direction
  //as each brand can be from different program. For IBF (and similar) requests
  //we only consider one brand per market and "general" availability is enough.
  //To make code easier to maintain for now we keep both availability information
  //with, and without direction, and use specific one depending on request type.
  BrandAvailability _brandAvailability;
  BrandAvailability _brandAvailabilityHardPassesOnly;
  BrandDirectionAvailability _brandDirectionAvailability;
  BrandDirectionAvailability _brandDirectionAvailabilityHardPassesOnly;
  // Brands for which hard passed fares exist on this market.
  // For these brands we remove soft passed fares from that fm.
  // Consequently we can only use hard passsed fares availability statuses.
  std::set<BrandCodeDirection> _hardPassAvailabilityOnly;

  AllNegFaresBucketsVec _allNegFaresBuckets;

  const Cat31FareBookingCodeValidator* _cat31RbdValidator = nullptr;

  bool _fmInvalidByRbdByCabin = false;
  bool _canValidateFBRBaseFares = false;

  enum StatusBit : uint8_t
  { HAS_DUPLICATES = 0,
    IS_THRU,
    SIMPLE_TRIP, // was built from simple-trip itin
    KEEP_SOLO_FM,
    HIGHEST_TPM_GOV_CXR,
    FIRST_CROSSING_AND_HIGH_TPM,
    HAS_CAT35_FARE,
    DUAL_GOVERNING_FLAG,
    FLT_TRX_INDICATOR,
    HAS_ALL_MARRIED_SEGMENTS,
    FLOW_MARKET,
    IS_CHILD_NEEDED,
    IS_INFANT_NEEDED, // is any infant needed
    IS_INF_NEEDED, // just INF
    BYPASS_CAT19_FLAG_SET,
    IS_JCB,
    BREAK_INDICATOR,
    COMBINE_SAME_CARRIER,
    VALIDATE_FARE_GROUP,
    SPECIAL_RTG_FOUND,
    COMPARE_PAX_TYPE_CODE,
    REMOVE_OUTBOUND_FARES,
    MERGED_AVAILABILITY,
    IS_ESV_THRU_MARKET,
    THROUGH_FARE_PRECEDENCE_NGS,
    SIMIALR_ITIN_MARKET, // if this FM belong to mother itinerary
    STATUS_SIZE };
  std::bitset<STATUS_SIZE> _statusBits;
};

typedef std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys>::iterator CurrencyIter;
typedef std::set<FareMarketCurrencyKey, FareMarketCurrencyKey::CompareKeys>::const_iterator
ConstCurrencyIter;

typedef std::vector<PaxTypeBucket> PaxTypeBucketVec;
typedef PaxTypeBucketVec::iterator PaxTypeBucketVecI;
} // tse namespace
