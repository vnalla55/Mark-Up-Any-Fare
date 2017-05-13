/*
 * YQYRCalculator.h
 *
 *  Created on: Feb 25, 2013
 *      Author: SG0892420
 */

#pragma once

#include "Common/TseEnums.h"
#include "Common/TsePoolAllocator.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/YQYRFees.h"
#include "DBAccess/YQYRFeesNonConcur.h"

#include <array>
#include <bitset>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tse
{
class FareMarketPath;
class FarePath;
class FeePercentageCalculator;
class Loc;
class MergedFareMarket;
class PaxType;
class PricingTrx;
class ShoppingTrx;
class TaxCarrierAppl;
class TaxCarrierFlightInfo;
class TravelSeg;

namespace YQYR
{
class Validations
{
public:
  enum Validation : size_t
  {
    POS,
    POT,
    AGENCY,
    TICKETING_DATE,
    JOURNEY,
    PTC,
    RETURNS_TO_ORIGIN,

    TOTAL,
    // A value that means "none of the above"
    NONE = TOTAL
  };

  void skipValidation(Validation validation) { _validations.reset(validation); }
  bool shouldValidate(Validation validation) const { return _validations.test(validation); }

private:
  std::bitset<TOTAL> _validations = (1UL << TOTAL) - 1;
};
}

class YQYRCalculator
{
  struct ValCxrContext;

public:
  enum class Directions : uint8_t
  {
    NONE = 0x00,
    OUTBOUND = 0x01,
    INBOUND = 0x10,
    BOTH = 0x11
  };

  struct FareGeography
  {
    const Loc* furthestLoc = nullptr;
    const TravelSeg* furthestSeg = nullptr;
    Directions direction = Directions::NONE;
    bool returnsToOrigin = false;
    bool isOnline = true;

    void init(const std::vector<TravelSeg*>& segments, bool initFurthestSeg = true);
  };

  using FeeListT = std::list<const YQYRFees*, TsePoolAllocator<const YQYRFees*>>;
  using ValCxrMap = std::map<CarrierCode, ValCxrContext*>;

  YQYRCalculator(PricingTrx& trx, Itin& it, const FareMarketPath* fmp, const PaxType* paxType);
  YQYRCalculator(PricingTrx& trx, Itin& it, const FarePath* fp, bool enableDiag = false);
  YQYRCalculator(PricingTrx& trx, Itin& it);
  YQYRCalculator(ShoppingTrx& trx,
                 FarePath* farePath,
                 const FareGeography& geography,
                 const YQYR::Validations validations,
                 const CurrencyCode calculationCurrency,
                 const std::vector<CarrierCode>& concurringCarriers,
                 bool enableDiagnostics);

  void addToValCxrMap(const ValCxrMap& valCxrMap);
  const ValCxrMap& getValCxrMap() { return _valCxrMap; }

  virtual ~YQYRCalculator() {}

  bool preCalcFailed() const { return _preCalcFailed; }
  virtual void process();
  MoneyAmount lowerBound() const { return _lowerBound; }
  virtual MoneyAmount chargeFarePath(const FarePath& fp, const CarrierCode valCxr) const;
  static bool isSimpleItin(PricingTrx& trx, const Itin& itin);

  struct YQYRApplication
  {
    YQYRApplication(
        const YQYRFees* y, int firstP, int lastP, PricingTrx& trx, const CurrencyCode calcCur);
    int first;
    int last;
    Indicator feeApplInd() const;
    MoneyAmount amount;
    const YQYRFees* yqyr;
    bool operator<(const YQYRApplication& other) const;
  };
  typedef std::vector<YQYRApplication> YQYRFeesApplicationVec;

  void findMatchingPathsShopping(ShoppingTrx& trx,
                                 const CarrierCode valCxr,
                                 const PaxTypeFare& fare,
                                 YQYRFeesApplicationVec& yqyrApplications);
  virtual void findMatchingPaths(const FarePath* fp,
                                 const CarrierCode valCxr,
                                 YQYRFeesApplicationVec& yqyrApplications);
  void findMatchingPaths(const CarrierCode valCxr,
                         const FeePercentageCalculator& feePercentageCalc,
                         std::map<int, std::string>& fbcBySeg,
                         std::map<int, BookingCode>& bookingCodesBySeg,
                         YQYRFeesApplicationVec& yqyrApplications);

  void initializeCarrierContext(const std::vector<CarrierCode>& allValCxrs,
                                const std::set<CarrierCode>& cxrInItin);
  void processCarrier(const CarrierCode, FeeListT& fees);

  static bool validateT190(const TaxCarrierAppl* t190, const CarrierCode cxr);

  class TPMemoryOverused : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  FeeListT allocateFeeList() { return FeeListT(_listMemPoolFactory); }

private:
  typedef std::vector<TravelSeg*>::const_iterator SegmentItType;
  bool initializeYQYRList(const CarrierCode cxr);
  void analyzeItin(const FarePath* fp = nullptr);
  void determineCarriers();

  // For unit tests purposes only
  MoneyAmount calculatePercentageFee(Percent amount, FarePath* farePath) const;

  void processCarrier(const CarrierCode);
  void processDirection(const CarrierCode cxr, Directions dir, FeeListT& fee);

  enum class MatchResult : uint8_t
  { UNKNOWN,
    NEVER,
    S_FAILED,
    P_FAILED_NO_CHANCE,
    P_FAILED,
    S_CONDITIONALLY,
    P_CONDITIONALLY,
    UNCONDITIONALLY };

  enum class FBCMatch : uint8_t
  { NOWHERE,
    NONE,
    SOME,
    ALL };

  struct YQYRPath
  {
    YQYRPath* extend(const YQYRApplication& appl) const;
    void display() const;
    void add(const YQYRApplication& appl);
    MoneyAmount amount = 0;
    std::vector<YQYRApplication> yqyrs;
  };

  struct FeesBySlice
  {
    Directions dir;
    std::vector<const YQYRPath*> yqyrpaths;
  };

  struct CarrierFeeCodeBucket
  {
    CarrierCode cxr;
    MoneyAmount lowerBound() const;
    std::vector<FeesBySlice*> feesBySlice;
  };

  struct ValCxrContext
  {
    ValCxrContext(const CarrierCode cxr) : _cxr(cxr) {}
    std::vector<const CarrierFeeCodeBucket*> _feeBuckets;
    std::vector<CarrierCode> _concurringCarriers;
    MoneyAmount _lowerBound = 0;
    const CarrierCode _cxr;

  private:
    ValCxrContext(const ValCxrContext&);
    ValCxrContext& operator=(const ValCxrContext&);
  };

  MatchResult matchFeeRecord(const YQYRFees* yqyr, int startInd, int endInd);

  const std::vector<const YQYRPath*>* processSlice(int startInd, int endInd, FeeListT& feeList);

  void merge(const YQYRPath* yp,
             const YQYRApplication& appl,
             const std::vector<const YQYRPath*>* ypvIn,
             std::vector<const YQYRPath*>* ypvOut);

  // validators
  bool validateJourney(const YQYRFees& s1Rec);
  bool validateLocs(char dir,
                    const Loc& frontLoc,
                    const LocCode& s1fLoc,
                    LocTypeCode s1fType,
                    const Loc& backLoc,
                    const LocCode& s1bLoc,
                    LocTypeCode s1bType,
                    const VendorCode& vendor,
                    const CarrierCode cxr);
  bool validateWithin(SegmentItType front,
                      SegmentItType back,
                      const LocCode& wiLoc,
                      LocTypeCode wiType,
                      const VendorCode& vendor,
                      const CarrierCode cxr);
  bool validateJourneyVia(SegmentItType front,
                          SegmentItType back,
                          const LocCode& viaLoc,
                          LocTypeCode viaType,
                          const VendorCode& vendor,
                          const CarrierCode cxr);
  bool checkValidatingCxr(const YQYRFees& yqyr);
  bool validateReturnToOrigin(const YQYRFees& yqyr) const;
  MatchResult validateSector(const YQYRFees& yqyr, int segInd);
  MatchResult validatePortion(const YQYRFees& yqyr, int first, int end);
  bool validateIntl(SegmentItType first, SegmentItType end, Indicator intlDomInd) const;
  bool validateEquipment(SegmentItType first, SegmentItType end, const EquipmentType& eqp) const;
  bool validateCarrierTable(SegmentItType first,
                            SegmentItType end,
                            const TaxCarrierFlightInfo* t186) const;
  bool checkLocation(const Loc& loc,
                     const LocCode& lcode,
                     LocTypeCode ltype,
                     const VendorCode& vendor,
                     const CarrierCode cxr) const;
  MatchResult
  validateBookingCode(SegmentItType first, SegmentItType end, const YQYRFees& s1Rec) const;
  FBCMatch validateFbcTktDsg(int segInd, const YQYRFees& s1Rec);
  bool validateVia(SegmentItType front,
                   SegmentItType end,
                   const LocCode& viaLoc,
                   LocTypeCode viaType,
                   const VendorCode& vendor,
                   const CarrierCode cxr,
                   Indicator connInd,
                   int64_t stopoverTime,
                   bool useCalendarDays);
  typedef std::unordered_multimap<int, std::string> FBCbySegMapT;
  friend class YQYRCalculatorTest;

  struct FBCMatchMapKey
  {
    typedef size_t FBCInt;
    static const size_t intBufSize = 32 / sizeof(FBCInt);
    union
    {
      char cbuf[32];
      FBCInt ibuf[intBufSize];
    };
    FBCMatchMapKey(const std::string& s1, const std::string& s2);

    bool operator<(const FBCMatchMapKey& other) const
    {
      return std::lexicographical_compare(
          ibuf, ibuf + intBufSize, other.ibuf, other.ibuf + intBufSize);
    }
    bool operator==(const FBCMatchMapKey& other) const
    {
      return std::equal(ibuf, ibuf + intBufSize, other.ibuf);
    }
  };

  struct FBCMatchMapHash
  {
    size_t operator()(const FBCMatchMapKey& key) const
    {
      size_t h = 14695981039346656037UL;
      const char* c = key.cbuf;
      do
      {
        h ^= *c++;
        h *= 1099511628211UL;
      } while (*c);
      return h;
    }
  };

  typedef std::unordered_map<FBCMatchMapKey, bool, FBCMatchMapHash> FBCMatchMap;
  void initMFMs();
  void initFBCMap(const FarePath* fp = nullptr);
  void initFBCMap(const std::vector<TravelSeg*>::const_iterator travelSegsBegin,
                  const std::vector<TravelSeg*>::const_iterator travelSegsEnd,
                  const PaxTypeFare::SegmentStatusVec& segmentStatus,
                  std::string& fareBasis);
  void initFBCMapForMFM(const MergedFareMarket* mfm);
  void getMFMs(const FareMarketPath* fmp, std::vector<MergedFareMarket*>& mfm);
  bool matchFareBasis(const std::string& tktFareBasis, const std::string& ptfFareBasis);
  bool validatePTC(const YQYRFees& yqyr) const;
  bool validatePOS(const YQYRFees& yqyr) const;
  bool validatePOT(const YQYRFees& yqyr) const;
  bool validateAgency(const YQYRFees& yqyr) const;
  bool validateTravelDate(const YQYRFees& yqyr) const;
  bool validateTicketingDate(const YQYRFees& yqyr) const;
  bool validatePercent(const YQYRFees& yqyr) const;
  void findLowerBound();
  int makeSliceKey(int startInd, int endInd) const;
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator VIA_STOPOVER = 'S';
  static constexpr Indicator VIA_CONNECTION = 'C';

  bool
  isYQYRStopover(TravelSeg* seg, TravelSeg* segNext, int64_t stopoverTime, bool useCalendarDays);
  bool matchPath(const YQYRPath* yp,
                 const std::map<int, std::string>& fbcBySeg,
                 const std::map<int, BookingCode>& bookingCodesBySeg) const;
  void prepareMatchMaps(const FarePath* fp,
                        std::map<int, std::string>& fbcBySeg,
                        std::map<int, BookingCode>& bookingCodesBySeg) const;
  bool shouldSoftpassRBD() const;
  void getFareBreaks(const FarePath* fp, std::set<TravelSeg*>& fareBreaks) const;
  int applCount(const YQYRFees* s1, int first, int last);
  MatchResult prevalidateFbc(int segInd, const YQYRFees& s1Rec) const;
  bool getItinValidatingCarriers(std::vector<CarrierCode>& result) const;

  bool diagParamFilter(const YQYRFees* yqyr);
  void printDiagnostic(const YQYRFees* yqyr, const char* diagMsg);
  void printDiagnostic(const YQYRFees* yqyr, const char* diagMsg, int Seg);
  void printDiagnostic(const YQYRFees* yqyr, const char* diagMsg, int startSeg, int endSeg);
  void printDiagnostic(const YQYRApplication& appl, const char* diagMsg);
  void printDiagnostic(const char* diagMsg,
                       const std::vector<TravelSeg*>& gdSegs,
                       const std::vector<GlobalDirection>& gds,
                       TravelSeg* curSeg);
  void printDiagnostic(const char* diagMsg, const TravelSeg* turnAroundSeg);
  void printDiagnostic(const char* diagMsg, const Loc* turnAroundLoc);
  void addItinDiagnostic(const char* diagMsg, std::ostringstream& stream);
  void sanityCheck(int howMany);

  FBCbySegMapT _fbcBySegNo;
  std::vector<bool> _hasSomeSlashesInFBCs;
  bool _noSlash = false;

protected:
  PricingTrx& _trx; // Used by YQYRCalculatorForREX
  MoneyAmount _lowerBound = 0; // Used by YQYRCalculatorForREX

  DateTime getProcessingDate() const;
private:
  Itin& _itin;
  const CurrencyCode& _itinCalculationCurrency;
  const TravelSeg* _furthestSeg = nullptr;
  const Loc* _furthestLoc = nullptr;
  const YQYR::Validations _validations;
  bool _oneFPMode;
  bool _returnsToOrigin;
  bool _isOnlineItin;
  Directions _direction;

  std::once_flag processOnce;

  PoolFactory _listMemPoolFactory;
  std::vector<FeeListT> _feesByCode;
  FeeListT* _feeList = nullptr;
  std::vector<CarrierCode> _carriers;
  CarrierFeeCodeBucket* _currentBucket = nullptr;
  FeesBySlice* _currentSlice = nullptr;

  ValCxrContext* _valCxrContext = nullptr;
  ValCxrMap _valCxrMap;

  std::map<int, std::vector<const YQYRPath*>*> _subSliceResults;
  FBCMatchMap _fbcMatchMap;
  const PaxType* _paxType = nullptr;
  const FareMarketPath* _fareMarketPath = nullptr;
  std::vector<MergedFareMarket*> _allMFMs;
  std::vector<BookingCode> _rbdBySegNo;
  bool _diagCollection;
  bool _reuseCFCBucket;
  bool _preCalcFailed;
  int _yqyrApplCountLimit;
  int _lastMemGrowthCheck;
  mutable DateTime _travelDate;
};

class YQYRLBFactory
{
public:
  static void init(PricingTrx& trx, Itin& it);
  YQYRCalculator* getYQYRPreCalc(const FareMarketPath* fmp, const PaxType* paxType);

  friend class YQYRCalculatorTest;

private:
  YQYRLBFactory(PricingTrx& trx, Itin& itin) : _trx(trx), _itin(itin) {}

  bool areYQYRsExempted() const;

  PricingTrx& _trx;
  Itin& _itin;
  std::mutex _mutex;
  bool _isSimpleItin = true;
  std::map<std::pair<PaxTypeCode, const FareMarketPath*>, YQYRCalculator*> _preCalcMap;
};

inline Indicator
YQYRCalculator::YQYRApplication::feeApplInd() const
{
  return yqyr->feeApplInd();
}

inline bool
YQYRCalculator::YQYRApplication::
operator<(const YQYRApplication& other) const
{
  return this->amount < other.amount;
}

inline int
YQYRCalculator::makeSliceKey(int startInd, int endInd) const
{
  return (startInd << 8) + endInd;
}

inline bool
YQYRCalculator::validatePercent(const YQYRFees& yqyr) const
{
  if (yqyr.percent() > 0)
    return _isOnlineItin;
  return true;
}

} // tse namespace
