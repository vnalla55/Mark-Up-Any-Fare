//-------------------------------------------------------------------
//
//  File:        PaxTypeFare.h
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: Class represents part of fare depending from
//               passanger Type
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

#include "Common/CabinType.h"
#include "Common/DateTime.h"
#include "Common/FareTypeDesignator.h"
#include "Common/SmallBitSet.h"
#include "Common/TrackCollector.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "DataModel/Itin.h"
#include "DataModel/StructuredRuleData.h"
#include "DataModel/TNBrandsTypes.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"

#include <functional>
#include <map>
#include <unordered_set>
#include <vector>
#include <boost/logic/tribool.hpp>
#include <boost/thread/mutex.hpp>
#include <array>
#include <atomic>
#include <memory>

BOOST_TRIBOOL_THIRD_STATE(notValidated)

namespace tse
{
class AdjustedSellingCalcData;
class BrandInfo;
class CombinabilityRuleInfo;
class DataHandle;
class DiscountInfo;
class FareByRuleApp;
class FareByRuleItemInfo;
class FareDisplayInfo;
class FBRPaxTypeFareRuleData;
class NegPaxTypeFareRuleData;
class Logger;
class MiscFareTag;
class PaxTypeFareRuleData;
class QualifyFltAppRuleData;
class SurchargeData;
class VoluntaryChangesInfoW;
class VoluntaryRefundsInfo;

namespace cabinUtils
{
class CabinComparator;
}

class PaxTypeFare
{
  friend class NegotiatedFareController;
  friend class PaxTypeFareTest;
  friend class MockPaxTypeFareWrapper;
  friend class RefundPermutationGeneratorTest;

public:
  // public consts
  // ====== ======

  static const uint16_t MAX_STOPOVERS_UNLIMITED;
  static const uint16_t PAXTYPE_FAIL;
  static const uint16_t PAXTYPE_NO_MATCHED;
  static const uint16_t PAXTYPE_NO_FAREGROUP;
  static const Indicator MOD1_MAX_STAY;
  static const Indicator MOD2_MIN_GROUP;
  static const Indicator MOD3_PERCENT;
  static const MoneyAmount NO_PERCENT;
  // public types
  // ====== =====

  enum PaxTypeFareState
  {
    PTF_Discounted = 0x00000001,
    PTF_FareByRule = 0x00000002,
    PTF_Negotiated = 0x00000004,
    PTF_AccSameFareBreak = 0x00000008,
    PTF_MatchedCorpID = 0x00000010, // fare matched corp id in cat1 (set for all request) or record
    // 8 set when fare goup request only
    PTF_FailedFareGroup = 0x00000020, // fare failed fare group for all pax types
    PTF_NotCat25BaseFare =
        0x00000040, // should not be used as base paxTypeFare for Fare By Rule fares
    PTF_NeedChkCat1R3Psg = 0x00000080, // need check cat1 record3
    // passenger type if fcasPaxType is empty or ADULT
    PTF_FoundCat1R3NoADT = 0x00000100, // found/matched cat1 record3
    // not ADULT passenger type
    PTF_FoundCat1R3ChkAge = 0x00000200, // matched record3 check age
    PTF_SelectedFareNetRemit = 0x00000400, // selected fare for Net Remit cat35
    PTF_AccTvlWarning = 0x00000800, // passed cat19 priced alone
    PTF_Cat35FailIf1OfThen15 = 0x00002000, // cat 35 fare faied THEN 15 IF 1
    PTF_AccSameCabin = 0x00004000, // cat13,19 req same cmpt
    PTF_MatchedCat15QualCorpID =
        0x00008000, // fare matched corp id in qualifying cat1 (then 15 if 1)
    PTF_Invalid_Fare_Currency = 0x00010000, // the currency of the fare is not valid
    PTF_AccTvlNotValidated = 0x00020000, // no control on price Acc Psg in Exchange Transaction
    PTF_SkipCat35ForRex = 0x00040000,
    PTF_KeepForRoutingValidation = 0x00080000, // Keep this fare for routing validation only
    PTF_InvalidForBaggage = 0x00100000,
  };

  using PaxTypeFareStatus = SmallBitSet<uint32_t, PaxTypeFareState>;

  enum FareDisplayState
  {
    FD_Valid = 0,
    FD_WEB_Invalidated          = 0x0000000000000001,
    FD_Inclusion_Code           = 0x0000000000000002, // Fare Not Valid for Inclusion Code Specified
    FD_Outbound_Currency        = 0x0000000000000004, // Fare Not Valid for Outbound Currency
    FD_Fare_Not_Selected_for_RD = 0x0000000000000008, // Fare not selected for RD
    FD_FareClassApp_Unavailable = 0x0000000000000010, // FareClassApp Unavailable Tag says Fare
                                                      // Cannot be Displayed
    FD_Not_Valid_For_Direction  = 0x0000000000000020, // Fare not valid for direction of travel
    FD_FareClass_Match          = 0x0000000000000040, // Fare does not match Fare Class Requested
    FD_TicketDesignator         = 0x0000000000000080, // Fare does not match Ticket Designator
                                                      // Requested
    FD_BookingCode              = 0x0000000000000100, // Fare does not match Booking Code Requested
    FD_PaxType                  = 0x0000000000000200, // Fare not valid for pax type requested
    FD_Eff_Disc_Dates           = 0x0000000000000400, // Fare not valid for date requested based on
                                                      // eff/disc dates of fare
    FD_Private                  = 0x0000000000000800, // Fare is not Private and PV qualifier
                                                      // was used to request private fares only
    FD_Public                   = 0x0000000000001000, // Fare is not public and PL qualifier
                                                      // was used to request public fares only
    FD_Industry_Fare            = 0x0000000000002000, // Industry Fare is excluded
    FD_Base_Fare_Exception      = 0x0000000000004000, // Base Fare Exception thrown
    FD_Cat35_Invalid_Agency     = 0x0000000000008000, // Agency could not be validated
    FD_Corp_ID_AccCode          = 0x0000000000010000, // Corp ID account code could not be qualified
    FD_Pax_Type_Code            = 0x0000000000020000, // Invalid pax Type Code
    FD_OW_RT                    = 0x0000000000040000, // Fare is not OW or RT according to
                                                      // the display type
    FD_Cat35_Incomplete_Data    = 0x0000000000080000, // Fare not filed with complete data
    FD_Cat35_ViewNetIndicator   = 0x0000000000100000, // Attribute that decides the visibility
                                                      // of NET fares
    FD_Cat35_Failed_Rule        = 0x0000000000200000, // Failed Rule validation
    FD_Missing_Footnote_Data    = 0x0000000000400000, // Footnote data not found
    FD_ZZ_Global_Dir            = 0x0000000000800000, // ZZ ony valid if calc cat25
    FD_Dupe_Fare                = 0x0000000001000000, // fare already in list
    FD_Merged_Fare              = 0x0000000002000000, // this fare can be merged with
                                                      // another in list
    FD_Invalid_For_CWT          = 0x0000000004000000, // the fare is not valid for CWT user
    FD_Matched_Fare_Focus       = 0x0000000008000000, // the fare is matched Fare Focus rule
    FD_OriginalAdjusted_Fare    = 0x0000000010000000, // the original Adjusted fare for FRR ASL

    FD_Rule_Tariff              = 0x0000000020000000, // Fare does not match rule tariff
    FD_Rule_Number              = 0x0000000040000000, // Fare does not match rule number
    FD_Fare_Type_Code           = 0x0000000080000000, // Fare does not match fare type code
    FD_Display_Type             = 0x0000000100000000, // Fare does not match display type
    FD_Private_Indicator        = 0x0000000200000000, // Fare does not match private indicator
    FD_Negotiated_Select        = 0x0000000400000000, // Fare does not match 'negotiated'select
    FD_FBR_Select               = 0x0000000800000000, // Fare does not match 'fare by rule' select
    FD_Sales_Restriction_Select = 0x0000001000000000, // Fare does not match 'sales restr.' select
    FD_Retailer_Code            = 0x0000002000000000  // Fare does not match 'Retailer Code' 
 };
  using FareDisplayStatus = SmallBitSet<uint64_t, FareDisplayState>;

  enum BkgCodeSegState
  { BKSS_NOT_YET_PROCESSED = 0x00000001,
    BKSS_SURFACE = 0x00000002,
    BKSS_FAIL = 0x00000004,
    BKSS_PASS = 0x00000008,
    BKSS_NOMATCH = 0x00000010,
    BKSS_FAIL_T999 = 0x00000020,
    BKSS_FAIL_REC1_T999 = 0x00000040,
    BKSS_FAIL_CONV1_T999 = 0x00000080,
    BKSS_FAIL_CONV2_T999 = 0x00000100,
    BKSS_FAIL_MIXEDCLASS = 0x00000200,
    BKSS_FAIL_LOCALMARKET = 0x00000400,
    BKSS_FAIL_PRIME_RBD_DOMESTIC = 0x00000800,
    BKSS_FAIL_PRIME_RBD_INTERNATIONAL = 0x00001000,
    BKSS_REBOOKED = 0x00002000,
    BKSS_DIFFERENTIAL = 0x00004000,
    BKSS_FAIL_CABIN = 0x00008000,
    BKSS_NEED_REVALIDATION = 0x00020000,
    BKSS_NEED_REVALIDATION_REC1 = 0x00040000,
    BKSS_NEED_REVALIDATION_CONV1 = 0x00080000,
    BKSS_NEED_REVALIDATION_CONV2 = 0x00100000,
    BKSS_FAIL_PRIME_RBD_CAT25 = 0x00200000,
    BKSS_AVAIL_BREAK = 0x00400000,
    BKSS_REX_WP_LOCALMARKET_VALIDATION = 0x00800000,
    BKSS_FAIL_TAG_N = 0x01000000,
    BKSS_FAIL_OFFER = 0x02000000,
    BKSS_FAIL_AVAILABILITY = 0x04000000,
    BKSS_DUAL_RBD_PASS = 0x08000000 };

  using BkgCodeSegStatus = SmallBitSet<uint32_t, BkgCodeSegState>;

  struct SegmentStatus
  {
    BookingCode _bkgCodeReBook;
    CabinType _reBookCabin;
    BkgCodeSegStatus _bkgCodeSegStatus;
    BookingCode _dualRbd1;
  };

  using SegmentStatusVec = std::vector<SegmentStatus>;
  using SegmentStatusVecI = std::vector<SegmentStatus>::iterator;
  using SegmentStatusVecCI = std::vector<SegmentStatus>::const_iterator;

  enum BookingCodeState
  { BKS_NOT_YET_PROCESSED = 0x0001,
    BKS_FAIL = 0x0002,
    BKS_PASS_LOCAL_AVAIL = 0x0004,
    BKS_PASS = 0x0008,
    BKS_MIXED = 0x0010,
    BKS_PASS_FLOW_AVAIL = 0x0020,
    BKS_MIXED_FLOW_AVAIL = 0x0040,
    BKS_FAIL_FARE_TYPES = 0x0080,
    BKS_FAIL_TAG_N = 0x0100,
    BKS_FAIL_CAT31_KEEP_FARE = 0x0200,
    BKS_SAME_FAIL = 0x0400,
    BKS_REQ_LOWER_CABIN = 0x0800,
    BKS_FAIL_FLOW_AVAIL = 0x1000,
    BKS_FAIL_NOT_KEEP = 0x2000,
    BKS_NEED_FARE_PATH = 0x4000 };

  using BookingCodeStatus = SmallBitSet<uint16_t, BookingCodeState>;

  enum CmdPricingFailedState
  {
    PTFF_Cat1 = 0x00000001,
    PTFF_Cat2 = 0x00000002,
    PTFF_Cat3 = 0x00000004,
    PTFF_Cat4 = 0x00000008,
    PTFF_Cat5 = 0x00000010,
    PTFF_Cat6 = 0x00000020,
    PTFF_Cat7 = 0x00000040,
    PTFF_Cat8 = 0x00000080,
    PTFF_Cat9 = 0x00000100,
    PTFF_Cat10 = 0x00000200,
    PTFF_Cat11 = 0x00000400,
    PTFF_Cat12 = 0x00000800,
    PTFF_Cat13 = 0x00001000,
    PTFF_Cat14 = 0x00002000,
    PTFF_Cat15 = 0x00004000,
    PTFF_Cat16 = 0x00008000,
    PTFF_Cat17 = 0x00010000,
    PTFF_Cat18 = 0x00020000,
    PTFF_Cat19 = 0x00040000,
    PTFF_Cat20 = 0x00080000,
    PTFF_Cat21 = 0x00100000,
    PTFF_Cat22 = 0x00200000,
    PTFF_Cat23 = 0x00400000,
    PTFF_Cat24 = 0x00800000,
    PTFF_Cat25 = 0x01000000,
    PTFF_Max_Numbered = 25,
    PTFF_Cat35 = 0x02000000,
    PTFF_ALLRUL = 0x04000000 - 1u, // category rule bits
    PTFF_RBD = 0x04000000,
    PTFF_R1UNAVAIL = 0x08000000,
    PTFF_CURR_SEL = 0x10000000,
    PTFF_MISSED_FTNOTE = 0x20000000,
    PTFF_NOT_CP_VALID = 0x80000000, // must not be command priced
    PTFF_ALL = 0xffffffff,
    PTFF_NONE = 0x00000000,
  };

  using PaxTypeFareCPFailedStatus = SmallBitSet<uint32_t, CmdPricingFailedState>;

  enum HitMixClass
  { MX_NOT_APPLICABLE,
    MX_DIFF };

  enum AltDateProcessingState
  {
    BKC_PROCESSED = 0x01, // booking code already processed
    BKC_FAILED = 0x02, // booking code failed
    RTG_PROCESSED = 0x04, // routing already processed
    RTG_FAILED = 0x08, // routing failed
    GLB_PROCESSED = 0x10, // global direction already processed
    GLB_FAILED = 0x20, // global direction failed
  };

  using AltDateFltBitStatus = SmallBitSet<uint8_t, AltDateProcessingState>;

  enum RuleState
  { RS_CatNotSupported = 0,
    RS_Cat01 = 0x00000001,
    RS_Cat13 = 0x00000002,
    RS_Cat18 = 0x00000004,
    RS_Cat19 = 0x00000008,
    RS_Cat20 = 0x00000010,
    RS_Cat21 = 0x00000020,
    RS_Cat22 = 0x00000040,
    RS_Cat25 = 0x00000080,
    RS_Cat31 = 0x00000100,
    RS_Cat33 = 0x00000200,
    RS_Cat35 = 0x00000400,
    RS_AllCat = 0x000007FF,
    RS_DefinedInFare = 0x80000000,
    RS_MaxCat = 35 };

  using RuleStatus = SmallBitSet<uint32_t, RuleState>;

  using CategoryBitSet = std::bitset<PaxTypeFare::RS_MaxCat + 1>;

  //--------------------------------------------------------------------------//
  //-- Start Flight Bitmap data structure declarations
  //--------------------------------------------------------------------------//

  class FlightBit
  {
  public:
    BookingCodeStatus _bookingCodeStatus;
    std::vector<SegmentStatus> _segmentStatus;
    uint16_t _mpmPercentage = 0;
    uint8_t _flightBit = 0;
  };

  using FlightBitmap = std::vector<FlightBit>;

  using FlightBitmapIterator = FlightBitmap::iterator;
  using FlightBitmapConstIterator = FlightBitmap::const_iterator;
  using FlightBitmapPerCarrier = VecMap<uint32_t, FlightBitmap>;

  class FlightBitEqualTo : public std::unary_function<FlightBit, bool>
  {
  private:
    const uint8_t _countValue;

  public:
    explicit FlightBitEqualTo(uint8_t countValue) : _countValue(countValue) {}

    bool operator()(const FlightBit& x) { return (x._flightBit == _countValue); }
  };

  //--------------------------------------------------------------------------//
  //-- End Flight Bitmap data structure declarations
  //--------------------------------------------------------------------------//

  struct PaxTypeFareAllRuleData
  {
    bool chkedRuleData = false;
    bool chkedGfrData = false;
    PaxTypeFareRuleData* fareRuleData = nullptr;
    PaxTypeFareRuleData* gfrRuleData = nullptr;
  };

  /**
   *  @method: csTextMessages
   *  Description: Get/Set methods for Currency Selection Text Messages
   *
   *  @return: const/non-const reference to vector of strings
   */
  std::vector<std::string>& csTextMessages() { return _csTextMessages; }
  const std::vector<std::string>& csTextMessages() const { return _csTextMessages; }

  using PaxTypeFareRuleDataByCatNo = std::array<std::atomic<PaxTypeFareAllRuleData*>, 36>;

  enum BrandStatus
  {
    BS_FAIL = 'F', // FAIL
    BS_HARD_PASS = 'H', // HARD PASS
    BS_SOFT_PASS = 'S' // SOFT PASS - Secondary T189
  };

  using BrandStatusWithDirection = std::pair<BrandStatus, Direction>;

  struct MaxPenaltyInfoRecords
  {
    std::unordered_set<const VoluntaryChangesInfoW*> _voluntaryChangesInfo;
    std::unordered_set<const PenaltyInfo*> _penaltyInfo;
    std::unordered_set<const VoluntaryRefundsInfo*> _voluntaryRefundsInfo;
    bool _penaltyRecordsLoaded = false;
  };

protected:
  // Account Code/ Corporate ID matched in multi AccCode/CorpID processing
  std::string _matchedAccCode;

public:
  PaxTypeFare()
  {
    for(auto& a: *_paxTypeFareRuleDataMap)
#if __GNUC__ < 5    // Bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64658
      a.store(nullptr, std::memory_order_relaxed);
#else // C++ 11 library reference manuals say this is the intended way to initialize values of atomic variables
      std::atomic_init(&a, (PaxTypeFareAllRuleData*)(nullptr));
#endif
  }
  // Placed here so the clone methods must be used
  PaxTypeFare(const PaxTypeFare&) = delete;
  PaxTypeFare& operator=(const PaxTypeFare&) = delete;

  virtual ~PaxTypeFare() = default;

  bool canTreatAsWithSameCabin(const PaxTypeFare& rhs) const;

  WarningMap& warningMap() { return _fare->warningMap(); }
  const WarningMap& warningMap() const { return _fare->warningMap(); }

  // just mark reason for failure
  // leave contents untouched (may have other references)
  // ==============
  void invalidateFare(FareDisplayState fds) { _fareDisplayStatus.set(fds); }

  // initialization
  // ==============
  bool initialize(Fare* fare, PaxType* actualPaxType, FareMarket* fareMarket, PricingTrx& trx);
  bool initialize(Fare* fare, PaxType* actualPaxType, FareMarket* fareMarket);

  // Fare status
  // ==== ======

  bool isDomestic() const { return _fare->isDomestic(); }
  bool isTransborder() const { return _fare->isTransborder(); }

  bool isInternational() const { return _fare->isInternational(); }
  bool isForeignDomestic() const { return _fare->isForeignDomestic(); }

  bool isPublished() const { return _fare->isPublished(); }
  bool isConstructed() const { return _fare->isConstructed(); }
  bool hasConstructedRouting() const;

  bool isReversed() const { return _fare->isReversed(); }

  bool isGlobalDirectionValid() const { return _fare->isGlobalDirectionValid(); }

  bool setGlobalDirectionValid(bool gdIsValid = true)
  {
    return _fare->setGlobalDirectionValid(gdIsValid);
  }

  bool isFareValidForDisplayOnly() const { return _fare->isFareValidForDisplayOnly(); }

  bool isFareNotValidForDisplay() const { return _fare->isFareNotValidForDisplay(); }

  bool setFareNotValidForDisplay(bool fareValidForDisplay = false)
  {
    return _fare->setFareNotValidForDisplay(fareValidForDisplay);
  }

  bool isPsrHipExempt() const { return ((_fare != nullptr) && _fare->isPsrHipExempt()); }

  bool setPsrHipExempt(bool psrHipExempt = true)
  {
    return ((_fare != nullptr) && _fare->setPsrHipExempt(psrHipExempt));
  }
  bool isRoundTrip() const { return _fare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED; }
  virtual bool isWebFare() const { return _fare->isWebFare(); }

  bool isNonRefundableByFareType() const;

  bool setWebFare(bool webFare = true) { return _fare->setWebFare(webFare); }

  void setIsValidForBranding(bool newValue) { _isValidForBranding = newValue; }
  bool isValidForBranding() const { return _isValidForBranding; }

  void setValidForBaggage(bool v) { _status.set(PTF_InvalidForBaggage, !v); }
  bool isValidForBaggage() const { return !_status.isSet(PTF_InvalidForBaggage); }

  Direction getDirection() const;

  bool isFareAndProgramDirectionConsistent(Direction programDirection) const;

  BrandStatus getBestStatusInAnyBrand(bool useDirectionality) const;
  BrandStatus getBrandStatus(const PricingTrx& trx, const BrandCode* brandCode) const;

  bool isValidForBrand(const PricingTrx& trx,
                       const BrandCode* brandCode,
                       bool hardPassedOnly = false) const;

  bool isValidForRequestedBrands(const PricingTrx& trx,
                                 const BrandFilterMap& requestedBrands,
                                 bool hardPassedOnly = false) const;

  bool isValidForCabin(const CabinType& cabin, bool stayInCabin) const;

  BrandStatus isValidForExchangeWithBrands(const PricingTrx& trx, const BrandCode& brandCode) const;

  bool isFromFlownOrNotShoppedFM() const;

  PaxTypeFareStructuredRuleData& getStructuredRuleData() const
  {
    TSE_ASSERT(_structuredRuleData);
    return *_structuredRuleData;
  }

  void createStructuredRuleDataIfNonexistent()
  {
    if (!_structuredRuleData)
      _structuredRuleData =
          std::unique_ptr<PaxTypeFareStructuredRuleData>(new PaxTypeFareStructuredRuleData);
  }

  void setSpanishResidentFareBasisSuffix(const std::string desig)
  {
    _spanishResidentFareBasisSuffix = std::move(desig);
  }

  void setStructuredRuleData(const PaxTypeFareStructuredRuleData& structuredRuleData);

  bool hasStructuredRuleData() const { return _structuredRuleData != nullptr; }

  void clearStructuredRuleData()
  {
      _structuredRuleData.reset();
  }

  virtual int getValidBrandIndex(const PricingTrx& trx, const BrandCode* brandCode,
                                 Direction fareUsageDirection) const;

  virtual void getValidBrands(const PricingTrx& trx,
                              std::vector<int>& brands,
                              bool hardPassedOnly = false) const;

  virtual void getValidBrands(const PricingTrx& trx,
                              std::vector<BrandCode>& brands,
                              bool hardPassedOnly = false) const;

  virtual void getValidBrands(const PricingTrx& trx,
                      std::vector<skipper::BrandCodeDirection>& brands,
                      bool hardPassedOnly = false) const;

  bool hasValidBrands(bool hardPassedOnly = false) const;

  BrandCode getFirstValidBrand(const PricingTrx& trx, Direction fareUsageDirection) const;

  bool isCat23PublishedFail() const { return _fare->isCat23PublishedFail(); }

  bool areFootnotesPrevalidated() const { return _fare->areFootnotesPrevalidated(); }

  void setFootnotesPrevalidated(bool status = true) { _fare->setFootnotesPrevalidated(status); }

  // PaxTypeFare status
  // =========== ======

  bool isValid() const;
  bool isValidForPO() const;
  bool isValidForFareDisplay() const;
  bool isValidNoBookingCode() const;
  virtual bool isValidForPricing() const;
  bool isValidForCombination() const;
  bool isValidForDiscount() const;
  bool isValidForRouting() const;

  bool isDiscounted() const { return _status.isSet(PTF_Discounted); }

  bool isFareByRule() const { return _status.isSet(PTF_FareByRule); }
  bool isNegotiated() const { return _status.isSet(PTF_Negotiated); }

  bool hasCat35Filed() const; // is cat35 base or cat35 clone

  bool needAccSameFareBreak() const { return _status.isSet(PTF_AccSameFareBreak); }
  bool setAccSameFareBreak(const bool isAccFareBreak)
  {
    return _status.set(PTF_AccSameFareBreak, isAccFareBreak);
  }

  bool needAccSameCabin() const { return _status.isSet(PTF_AccSameCabin); }
  bool setAccSameCabin(const bool isSame) { return _status.set(PTF_AccSameCabin, isSame); }

  bool matchedCorpID() const { return _status.isSet(PTF_MatchedCorpID); }
  bool setMatchedCorpID(const bool matchedCorpID = true)
  {
    return _status.set(PTF_MatchedCorpID, matchedCorpID);
  }

  bool matchedCat15QualifyingCorpID() const { return _status.isSet(PTF_MatchedCat15QualCorpID); }
  bool setMatchedCat15QualifyingCorpID(const bool matchedCorpID = true)
  {
    return _status.set(PTF_MatchedCat15QualCorpID, matchedCorpID);
  }

  const std::string& matchedAccCode() const { return _matchedAccCode; }
  std::string& matchedAccCode() { return _matchedAccCode; }

  bool failedFareGroup() const { return _status.isSet(PTF_FailedFareGroup); }
  bool setFailedFareGroup(const bool failedFareGroup = true)
  {
    return _status.set(PTF_FailedFareGroup, failedFareGroup);
  }

  bool canBeFBRBaseFare() const { return !_status.isSet(PTF_NotCat25BaseFare); }
  bool setNotFBRBaseFare(const bool canNot = true)
  {
    return _status.set(PTF_NotCat25BaseFare, canNot);
  }

  bool setNeedChkCat1R3Psg(const bool ifNeed) { return _status.set(PTF_NeedChkCat1R3Psg, ifNeed); }

  bool needChkCat1R3Psg() const { return _status.isSet(PTF_NeedChkCat1R3Psg); }

  bool setFoundCat1R3NoADT(const bool ifFound = true)
  {
    return _status.set(PTF_FoundCat1R3NoADT, ifFound);
  }

  bool foundCat1R3NoADT() const { return _status.isSet(PTF_FoundCat1R3NoADT); }

  bool setFoundCat1R3ChkAge(const bool ifFound = true)
  {
    return _status.set(PTF_FoundCat1R3ChkAge, ifFound);
  }

  bool foundCat1R3ChkAge() const { return _status.isSet(PTF_FoundCat1R3ChkAge); }

  bool selectedFareNetRemit() const { return _status.isSet(PTF_SelectedFareNetRemit); }
  bool setSelectedFareNetRemit(const bool selectedFareNetRemit = true)
  {
    return _status.set(PTF_SelectedFareNetRemit, selectedFareNetRemit);
  }
  bool setAccTvlWarning(const bool ifSet = true) { return _status.set(PTF_AccTvlWarning, ifSet); }

  const bool isAccTvlWarning() const { return _status.isSet(PTF_AccTvlWarning); }

  bool setCat35FailIf1OfThen15(bool cat35FailIf1OfThen15 = true)
  {
    return _status.set(PTF_Cat35FailIf1OfThen15, cat35FailIf1OfThen15);
  }
  const bool cat35FailIf1OfThen15() const { return _status.isSet(PTF_Cat35FailIf1OfThen15); }

  bool invalidFareCurrency() const { return _status.isSet(PTF_Invalid_Fare_Currency); }
  bool setInvalidFareCurrency(const bool invalidCurrency = true)
  {
    return _status.set(PTF_Invalid_Fare_Currency, invalidCurrency);
  }

  bool setAccTvlNotValidated(const bool ifSet = true)
  {
    return _status.set(PTF_AccTvlNotValidated, ifSet);
  }

  const bool isAccTvlNotValidated() const { return _status.isSet(PTF_AccTvlNotValidated); }

  bool setSkipCat35ForRex(const bool ifSet = true)
  {
    return _status.set(PTF_SkipCat35ForRex, ifSet);
  }

  const bool isKeepForRoutingValidation() const
  {
    return _status.isSet(PTF_KeepForRoutingValidation);
  }

  bool setKeepForRoutingValidation(const bool ifSet = true)
  {
    return _status.set(PTF_KeepForRoutingValidation, ifSet);
  }

  void setIbfErrorMessage(IbfErrorMessage newErrorMessage) { _ibfErrorMessage = newErrorMessage; }

  void setNotOfferedOnAllSegments() { _notOfferedOnAllSegments = true; };

  IbfErrorMessage getIbfErrorMessage() const { return _ibfErrorMessage; }

  bool isSoldOut() const;

  bool isNotOfferedOnAllSegments() const { return _notOfferedOnAllSegments; }

  PaxTypeFareStatus& status() { return _status; }
  const PaxTypeFareStatus& status() const { return _status; }

  PaxTypeFare* getAsBookedClone() { return _rebookInfo._asBookedClone; }
  void cloneMeForAsBooked(PricingTrx& trx);
  bool iAmAsBookedClone() const { return _rebookInfo._iAmAsBookedClone; }

  FareDisplayStatus& fareDisplayStatus() { return _fareDisplayStatus; }
  const FareDisplayStatus& fareDisplayStatus() const { return _fareDisplayStatus; }

  void setValidForRepricing(bool value) { _isValidForRepricing = value; }

  // Fare Display  status
  // ============  ======

  bool invalidWEB() const { return _fareDisplayStatus.isSet(FD_WEB_Invalidated); }
  bool notValidForInclusionCode() const { return _fareDisplayStatus.isSet(FD_Inclusion_Code); }
  bool notValidForOutboundCurrency() const
  {
    return _fareDisplayStatus.isSet(FD_Outbound_Currency);
  }
  bool notSelectedForRD() const { return _fareDisplayStatus.isSet(FD_Fare_Not_Selected_for_RD); }
  bool fareClassAppTagUnavailable() const
  {
    return _fareDisplayStatus.isSet(FD_FareClassApp_Unavailable);
  }

  bool notValidForDirection() const { return _fareDisplayStatus.isSet(FD_Not_Valid_For_Direction); }
  bool fareClassMatch() const { return _fareDisplayStatus.isSet(FD_FareClass_Match); }
  bool ticketDesignatorMatch() const { return _fareDisplayStatus.isSet(FD_TicketDesignator); }
  bool bookingCodeMatch() const { return _fareDisplayStatus.isSet(FD_BookingCode); }
  bool paxTypeMatch() const { return _fareDisplayStatus.isSet(FD_PaxType); }
  bool dateCheck() const { return _fareDisplayStatus.isSet(FD_Eff_Disc_Dates); }
  bool privateFareCheck() const { return _fareDisplayStatus.isSet(FD_Private); }
  bool publicFareCheck() const { return _fareDisplayStatus.isSet(FD_Public); }
  bool industryFareCheck() const { return _fareDisplayStatus.isSet(FD_Industry_Fare); }
  bool baseFareException() const { return _fareDisplayStatus.isSet(FD_Base_Fare_Exception); }
  bool cat35InvalidAgency() const { return _fareDisplayStatus.isSet(FD_Cat35_Invalid_Agency); }
  bool notOWorRTfare() const { return _fareDisplayStatus.isSet(FD_OW_RT); }
  bool cat35IncompleteData() const { return _fareDisplayStatus.isSet(FD_Cat35_Incomplete_Data); }
  bool cat35ViewNetIndicator() const { return _fareDisplayStatus.isSet(FD_Cat35_ViewNetIndicator); }
  bool cat35FailedRule() const { return _fareDisplayStatus.isSet(FD_Cat35_Failed_Rule); }
  bool cat25FailedRule() const { return _ruleStatus.isSet(RS_Cat25); }
  bool missingFootnoteData() const { return _fareDisplayStatus.isSet(FD_Missing_Footnote_Data); }
  bool invalidForCWT() const { return _fareDisplayStatus.isSet(FD_Invalid_For_CWT); }
  bool matchedFareFocus() const { return _fareDisplayStatus.isSet(FD_Matched_Fare_Focus); }
  bool notMatchedFareRuleTariff() const { return _fareDisplayStatus.isSet(FD_Rule_Tariff); }
  bool notMatchedFareTypeCode() const { return _fareDisplayStatus.isSet(FD_Fare_Type_Code); }
  bool notMatchedFareRuleNumber() const { return _fareDisplayStatus.isSet(FD_Rule_Number); }
  bool notMatchedFareDisplayType() const { return _fareDisplayStatus.isSet(FD_Display_Type); }
  bool notMatchedFareCat35Select() const { return _fareDisplayStatus.isSet(FD_Negotiated_Select); }
  bool notMatchedFareCat25Select() const { return _fareDisplayStatus.isSet(FD_FBR_Select); }
  bool notMatchedFareCat15Select() const {
                                    return _fareDisplayStatus.isSet(FD_Sales_Restriction_Select); }
  bool notMatchedFarePrivateIndicator() const {
                                    return _fareDisplayStatus.isSet(FD_Private_Indicator); }

  // PaxTypeFare rule command pricing failed flag
  // =========== ==== ======= ======= ====== ====

  PaxTypeFareCPFailedStatus& cpFailedStatus() { return _cpFailedStatus; }
  const PaxTypeFareCPFailedStatus& cpFailedStatus() const { return _cpFailedStatus; }

  bool setCmdPrcFailedFlag(const unsigned int category, const bool ifFailed = true);
  bool setCmdPrcCurrencySelectionFailedFlag(const bool ifFailed = true);
  void setNotValidForCP(bool notValid) { _cpFailedStatus.set(PTFF_NOT_CP_VALID, notValid); }

  bool isFailedCmdPrc(const unsigned int category) const;
  bool isNotValidForCP() const { return _cpFailedStatus.isSet(PTFF_NOT_CP_VALID); }
  bool cmdPricedWFail() const { return _cpFailedStatus.isSet(PTFF_ALL); }

  bool isCmdPricing() const;
  bool validForCmdPricing(bool fxCnException, PricingTrx* trx = nullptr) const;
  bool validForCmdPricing(const FareMarket& fm,
                          bool fxCnException = false,
                          PricingTrx* trx = nullptr) const;

  bool isFilterPricing() const;
  bool validForFilterPricing() const;

  // rule status
  // ==== ======

  bool isCategoryValid(const unsigned int category) const;

  uint16_t getFailedCatNum(std::vector<uint16_t>& catSeq) const;

  bool setCategoryValid(const unsigned int category, const bool catIsValid = true);

  bool areAllCategoryValid() const;

  bool isCategoryProcessed(const unsigned int category) const;

  bool setCategoryProcessed(const unsigned int category, const bool catIsProcessed = true);

  bool isAllDiffCatProcessed() const { return _fare->isAllDiffCatProcessed(); };

  bool isCategorySoftPassed(const unsigned int category) const;

  bool setCategorySoftPassed(const unsigned int category, const bool catSoftPassed = true);

  bool isSoftPassed() const;

  void resetRuleStatus();

  void resetRuleStatusESV()
  {
    _ruleProcessStatus.setNull();
    _ruleStatus.setNull();
    _fare->_ruleProcessStatus.setNull();
    _fare->_ruleStatus.setNull();
  }

  bool isCat15SecurityValid() const { return _fare->isCat15SecurityValid(); }

  bool setCat15SecurityValid(const bool isValid = true)
  {
    return _fare->setCat15SecurityValid(isValid);
  }

  bool isElectronicTktable() const { return _fare->isElectronicTktable(); }

  bool setElectronicTktable(const bool isEticketable = false)
  {
    return _fare->setElectronicTktable(isEticketable);
  }

  bool isElectronicTktRequired() const { return _fare->isElectronicTktRequired(); }

  bool setElectronicTktRequired(const bool isRequired = false)
  {
    return _fare->setElectronicTktRequired(isRequired);
  }

  bool isPaperTktSurchargeMayApply() const { return _fare->isPaperTktSurchargeMayApply(); }

  bool setPaperTktSurchargeMayApply(const bool mayApply = false)
  {
    return _fare->setPaperTktSurchargeMayApply(mayApply);
  }

  bool isPaperTktSurchargeIncluded() const { return _fare->isPaperTktSurchargeIncluded(); }

  bool setPaperTktSurchargeIncluded(const bool isIncluded = false)
  {
    return _fare->setPaperTktSurchargeIncluded(isIncluded);
  }

  Indicator& penaltyRestInd() { return _penaltyRestInd; }
  const Indicator& penaltyRestInd() const { return _penaltyRestInd; }

  bool& changePenaltyApply() { return _changePenaltyApply; }
  const bool& changePenaltyApply() const { return _changePenaltyApply; }

  bool& reProcessCat05NoMatch() { return _fare->_reProcessCat05NoMatch; }
  const bool& reProcessCat05NoMatch() const { return _fare->_reProcessCat05NoMatch; }

  bool& failedByJalAxessRequest() { return _failedByJalAxessRequest; }
  const bool& failedByJalAxessRequest() const { return _failedByJalAxessRequest; }

  bool& ticketedFareForAxess() { return _ticketedFareForAxess; }
  const bool& ticketedFareForAxess() const { return _ticketedFareForAxess; }

  // booking code & booking code segment status
  // ======= ==== = ======= ==== ======= ======

  bool isBookingCodeProcessed() const
  {
    return !bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED);
  }

  bool isBookingCodePass() const { return _bookingCodeStatus.isSet(BKS_PASS); }

  BookingCodeStatus& bookingCodeStatus() { return _bookingCodeStatus; }

  const BookingCodeStatus& bookingCodeStatus() const { return _bookingCodeStatus; }

  std::vector<SegmentStatus>& segmentStatus() { return _segmentStatus; }

  const std::vector<SegmentStatus>& segmentStatus() const { return _segmentStatus; }

  std::vector<SegmentStatus>& segmentStatusRule2() { return _segmentStatusRule2; }

  const std::vector<SegmentStatus>& segmentStatusRule2() const { return _segmentStatusRule2; }

  int& mixClassStatus() { return _mixClassStatus; }
  const int& mixClassStatus() const { return _mixClassStatus; }

  // Booking Code will be used, if stored, to change 1st char of the YY Fare Basis Code
  const BookingCode& getChangeFareBasisBkgCode() const { return _changeFareBasisBkgCode; }
  void setChangeFareBasisBkgCode(const BookingCode& bkg) { _changeFareBasisBkgCode = bkg; }
  char getAllowedChangeFareBasisBkgCode() const { return _allowedChangeFareBasisBkgCode; }
  void setAllowedChangeFareBasisBkgCode(char bkg) { _allowedChangeFareBasisBkgCode = bkg; }
  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }
  BookingCode& s8BFBookingCode() { return _s8BFBookingCode; }
  const BookingCode& s8BFBookingCode() const { return _s8BFBookingCode; }

  // routing status
  // ======= ======

  bool isRoutingMapValid() const { return _fare->isRoutingMapValid(); }
  bool setRoutingMapValid(const bool rtMapValid = false) const
  {
    return _fare->setRoutingMapValid(rtMapValid);
  }

  bool isRoutingProcessed() const { return _fare->isRoutingProcessed(); }
  bool setRoutingProcessed(const bool rtProcessed) const
  {
    return _fare->setRoutingProcessed(rtProcessed);
  }

  bool isRouting() const { return _fare->isRouting(); }
  bool setIsRouting(const bool isRouting = true) { return _fare->setIsRouting(isRouting); }

  bool isRoutingValid() const { return _fare->isRoutingValid(); }
  bool setRoutingValid(const bool rtValid = true) const { return _fare->setRoutingValid(rtValid); }

  bool isRoutingInvalidByMPM() const { return _routingInvalidByMPM; }
  void setRoutingInvalidByMPM(const bool rtInvalid = true) { _routingInvalidByMPM = rtInvalid; }

  bool isRequestedFareBasisInvalid() const { return _requestedFareBasisInvalid; }
  void setRequestedFareBasisInvalid(const bool rfbInvalid = true)
  {
    _requestedFareBasisInvalid = rfbInvalid;
  }

  const std::vector<SurchargeData*>& surchargeData() const { return _surchargeData; }
  std::vector<SurchargeData*>& surchargeData() { return _surchargeData; }

  const bool needRecalculateCat12() const { return _needRecalculateCat12; }
  bool& needRecalculateCat12() { return _needRecalculateCat12; }

  const MoneyAmount nucTotalSurchargeFareAmount() const { return _nucTotalSurchargeFareAmount; }
  MoneyAmount& nucTotalSurchargeFareAmount() { return _nucTotalSurchargeFareAmount; }

  const uint16_t mileageSurchargePctg() const { return _fare->mileageSurchargePctg(); }

  uint16_t& mileageSurchargePctg() { return _fare->mileageSurchargePctg(); }

  const MoneyAmount& mileageSurchargeAmt() const;
  MoneyAmount& mileageSurchargeAmt();

  bool surchargeExceptionApplies() const { return _surchargeExceptionApplies; }
  bool& surchargeExceptionApplies() { return _surchargeExceptionApplies; }

  // for discounted fares and its base fare sharing rule validation result
  // === ========== ====
  bool adoptBaseFareRuleStat(DataHandle& dataHandle, uint16_t category);
  bool updateBaseFareRuleStat(DataHandle& dataHandle, uint16_t category);

  // Rule Status check for Cat31 Keep Fare
  bool isValidAsKeepFare(const PricingTrx& trx) const;
  bool isValidAccountCode(const PricingTrx& trx) const;

  // fare parts check
  // ==== ===== =====

  bool isFareInfoMissing() const { return _fare->isFareInfoMissing(); }
  bool isTariffCrossRefMissing() const { return _fare->isTariffCrossRefMissing(); }

  // PaxTypeFare parts check
  // ================ =====

  bool isFareClassAppMissing() const
  {
    return (_fareClassAppInfo == FareClassAppInfo::emptyFareClassApp());
  }

  bool isFareClassAppSegMissing() const
  {
    return (_fareClassAppSegInfo == FareClassAppSegInfo::emptyFareClassAppSeg() ||
            _fareClassAppSegInfo == nullptr);
  }

  bool hasValidFlight() const;

  bool isNoDataMissing() const;

  // access to PaxTypeFare parts
  // ====== == =========== =====

  Fare* fare() { return _fare; }
  const Fare* fare() const { return _fare; }
  void setFare(Fare* fare);

  PaxType*& actualPaxType() { return _actualPaxType; }
  const PaxType* actualPaxType() const { return _actualPaxType; }

  std::vector<uint16_t>& actualPaxTypeItem() { return _actualPaxTypeItem; }
  const std::vector<uint16_t>& actualPaxTypeItem() const { return _actualPaxTypeItem; }

  FareMarket*& fareMarket() { return _fareMarket; }
  const FareMarket* fareMarket() const { return _fareMarket; }

  FareDisplayInfo*& fareDisplayInfo() { return _fareDisplayInfo; }
  const FareDisplayInfo* fareDisplayInfo() const { return _fareDisplayInfo; }

  const FareClassAppInfo*& fareClassAppInfo() { return _fareClassAppInfo; }
  const FareClassAppInfo* fareClassAppInfo() const { return _fareClassAppInfo; }

  const FareClassAppSegInfo*& fareClassAppSegInfo() { return _fareClassAppSegInfo; }
  const FareClassAppSegInfo* fareClassAppSegInfo() const { return _fareClassAppSegInfo; }

  ///@TODO depriciated functions
  PaxType*& paxType() { return _actualPaxType; }
  const PaxType* paxType() const { return _actualPaxType; }

  // Fare amount
  // ==== ======

  const CurrencyNoDec numDecimal() const { return _fare->numDecimal(); }
  const CurrencyCode& currency() const { return _fare->currency(); }

  virtual const MoneyAmount originalFareAmount() const { return _fare->originalFareAmount(); }

  // one way fare amount

  virtual const MoneyAmount fareAmount() const { return _fare->fareAmount(); }

  // one way fare amount converted to NUCs
  // TODO not really nucFareAmount, should be called "calculationCurrencyFareAmount"
  virtual MoneyAmount& nucFareAmount();
  virtual const MoneyAmount nucFareAmount() const;

  MoneyAmount nucAmountWithEstimatedTaxes(const PaxTypeBucket& cortege) const;

  virtual MoneyAmount& rexSecondNucFareAmount() { return _fare->rexSecondNucFareAmount(); }
  virtual const MoneyAmount rexSecondNucFareAmount() const
  {
    return _fare->rexSecondNucFareAmount();
  }

  virtual MoneyAmount& rexSecondMileageSurchargeAmt()
  {
    return _fare->rexSecondMileageSurchargeAmt();
  }
  virtual const MoneyAmount rexSecondMileageSurchargeAmt() const
  {
    return _fare->rexSecondMileageSurchargeAmt();
  }

  // round trip fare amount converted to NUCs

  virtual MoneyAmount& nucOriginalFareAmount() { return _fare->nucOriginalFareAmount(); }
  virtual const MoneyAmount nucOriginalFareAmount() const { return _fare->nucOriginalFareAmount(); }

  // accessors to FareInfo members
  // ========= == ======== =======

  virtual const VendorCode& vendor() const { return _fare->vendor(); }
  const CarrierCode& carrier() const { return _carrier; }

  const LocCode& origin() const { return _fare->origin(); }
  const LocCode& destination() const { return _fare->destination(); }

  const LocCode& market1() const { return _fare->market1(); }
  const LocCode& market2() const { return _fare->market2(); }

  const FareClassCode& fareClass() const { return _fare->fareClass(); }
  const TariffNumber fareTariff() const { return _fare->fareTariff(); }

  const DateTime& effectiveDate() const { return _fare->effectiveDate(); }
  const DateTime& expirationDate() const { return _fare->expirationDate(); }

  const Footnote& footNote1() const { return _fare->footNote1(); }
  const Footnote& footNote2() const { return _fare->footNote2(); }

  const Indicator owrt() const { return _fare->owrt(); }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  const RoutingNumber& routingNumber() const { return _fare->routingNumber(); }

  const Directionality directionality() const { return _fare->directionality(); }

  const GlobalDirection globalDirection() const { return _fare->globalDirection(); }

  const Indicator inhibit() const { return _fare->inhibit(); }

  const SequenceNumberLong sequenceNumber() const { return _fare->sequenceNumber(); }

  const LinkNumber linkNumber() const { return _fare->linkNumber(); }

  const DateTime& createDate() const { return _fare->createDate(); }

  const bool vendorFWS() const { return _fare->vendorFWS(); }

  // accessors to TariffCrossRefInfo members
  // ========= == ================== =======

  const TariffCode& tcrFareTariffCode() const { return _fare->tcrFareTariffCode(); }

  const TariffCategory tcrTariffCat() const { return _tcrTariffCat; }

  void setTcrTariffCatPrivate() { _tcrTariffCat = PRIVATE_TARIFF; }

  const TariffNumber tcrRuleTariff() const { return _tcrRuleTariff; }

  const TariffCode& tcrRuleTariffCode() const { return _fare->tcrRuleTariffCode(); }

  const TariffNumber tcrRoutingTariff1() const { return _fare->tcrRoutingTariff1(); }
  const TariffCode& tcrRoutingTariff1Code() const { return _fare->tcrRoutingTariff1Code(); }

  const TariffNumber tcrRoutingTariff2() const { return _fare->tcrRoutingTariff2(); }
  const TariffCode& tcrRoutingTariff2Code() const { return _fare->tcrRoutingTariff2Code(); }

  const TariffCode& tcrAddonTariff1Code() const { return _fare->tcrAddonTariff1Code(); }

  const TariffCode& tcrAddonTariff2Code() const { return _fare->tcrAddonTariff2Code(); }

  const TariffCode* getRuleTariffCode(FareDisplayTrx& trx);

  // FareClassAppInfo accessors
  // ================ =========

  const Indicator fcaOwrt() const { return _fareClassAppInfo->_owrt; }

  const RoutingNumber& fcaRoutingNumber() const { return _fareClassAppInfo->_routingNumber; }

  const FareType& fcaFareType() const { return _fareClassAppInfo->_fareType; }

  const Indicator fcaSeasonType() const { return _fareClassAppInfo->_seasonType; }
  const Indicator fcaDowType() const { return _fareClassAppInfo->_dowType; }

  const Indicator fcaPricingCatType() const { return _fareClassAppInfo->_pricingCatType; }
  const Indicator fcaDisplayCatType() const { return _fareClassAppInfo->_displayCatType; }

  const Indicator fcaUnavailTag() const { return _fareClassAppInfo->_unavailTag; }

  const std::vector<FareClassAppSegInfo*>& fcasiSegments() const
  {
    return _fareClassAppInfo->_segs;
  }

  // FareClassAppSegInfo accessors
  // =================== =========

  const long fcasBookingCodeTblItemNo() const
  {
    return _fareClassAppSegInfo->_bookingCodeTblItemNo;
  }

  virtual const int fcasMinAge() const { return _fareClassAppSegInfo->_minAge; }
  virtual const int fcasMaxAge() const { return _fareClassAppSegInfo->_maxAge; }

  virtual const TktCode& fcasTktCode() const { return _fareClassAppSegInfo->_tktCode; }
  virtual const TktCodeModifier& fcasTktCodeModifier() const
  {
    return _fareClassAppSegInfo->_tktCodeModifier;
  }

  virtual const TktDesignator& fcasTktDesignator() const
  {
    return _fareClassAppSegInfo->_tktDesignator;
  }
  virtual const TktDesignatorModifier& fcasTktDesignatorModifier() const
  {
    return _fareClassAppSegInfo->_tktDesignatorModifier;
  }

  virtual const PaxTypeCode& fcasPaxType() const { return (_fareClassAppSegInfo->_paxType); }

  virtual bool getPrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const;

  const int fcasCarrierApplTblItemNo() const { return _fareClassAppSegInfo->_carrierApplTblItemNo; }

  // this fare can be associated with multiple FareClassAppSegInfo that are
  // identical except booking code, found in SITA records as solution to
  // bookingCode overflown
  std::vector<const FareClassAppSegInfo*>*& fcasOfOverFlownBkgCds()
  {
    return _fcasOfOverFlownBkgCds;
  }
  std::vector<const FareClassAppSegInfo*>* const& fcasOfOverFlownBkgCds() const
  {
    return _fcasOfOverFlownBkgCds;
  }

  // FareTypeMatrix accessors
  // ============== =========

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  FareTypeDesignator& fareTypeDesignator() { return _fareTypeDesignator; }
  const FareTypeDesignator& fareTypeDesignator() const { return _fareTypeDesignator; }

  Indicator& fareTypeApplication() { return _fareTypeApplication; }
  const Indicator fareTypeApplication() const { return _fareTypeApplication; }

  bool isNormal() const;
  bool isSpecial() const;

  // accessors to SITA extra fields
  // ========= == ==== ===== ======

  const RouteCode& routeCode() const { return _fare->routeCode(); }
  const DBEClass& dbeClass() const { return _fare->dbeClass(); }
  const Indicator fareQualCode() const { return _fare->fareQualCode(); }
  const Indicator tariffFamily() const { return _fare->tariffFamily(); }

  const Indicator cabotageInd() const { return _fare->cabotageInd(); }
  const Indicator govtAppvlInd() const { return _fare->govtAppvlInd(); }
  const Indicator constructionInd() const { return _fare->constructionInd(); }
  const Indicator multiLateralInd() const { return _fare->multiLateralInd(); }

  const LocCode& airport1() const { return _fare->airport1(); }
  const LocCode& airport2() const { return _fare->airport2(); }

  const Indicator viaCityInd() const { return _fare->viaCityInd(); }
  const LocCode& viaCity() const { return _fare->viaCity(); }

  // constructed fare common data accessors
  // =========== ==== ====== ==== =========

  const ConstructedFareInfo::ConstructionType constructionType() const
  {
    return _fare->constructionType();
  }

  const LocCode& gateway1() const { return _fare->gateway1(); }
  const LocCode& gateway2() const { return _fare->gateway2(); }

  const MoneyAmount specifiedFareAmount() const { return _fare->specifiedFareAmount(); }

  const MoneyAmount constructedNucAmount() const { return _fare->constructedNucAmount(); }

  // origin add-on data accessors
  // ====== ====== ==== =========

  const AddonZone origAddonZone() const { return _fare->origAddonZone(); }

  const Footnote& origAddonFootNote1() const { return _fare->origAddonFootNote1(); }
  const Footnote& origAddonFootNote2() const { return _fare->origAddonFootNote2(); }

  const FareClassCode& origAddonFareClass() const { return _fare->origAddonFareClass(); }

  const TariffNumber origAddonTariff() const { return _fare->origAddonTariff(); }

  const RoutingNumber& origAddonRouting() const { return _fare->origAddonRouting(); }

  const MoneyAmount origAddonAmount() const { return _fare->origAddonAmount(); }

  const CurrencyCode& origAddonCurrency() const { return _fare->origAddonCurrency(); }

  const Indicator origAddonOWRT() const { return _fare->origAddonOWRT(); }

  // destination add-on data accessors
  // =========== ====== ==== =========

  const AddonZone destAddonZone() const { return _fare->destAddonZone(); }

  const Footnote& destAddonFootNote1() const { return _fare->destAddonFootNote1(); }
  const Footnote& destAddonFootNote2() const { return _fare->destAddonFootNote2(); }

  const FareClassCode& destAddonFareClass() const { return _fare->destAddonFareClass(); }

  const TariffNumber destAddonTariff() const { return _fare->destAddonTariff(); }

  const RoutingNumber& destAddonRouting() const { return _fare->destAddonRouting(); }

  const MoneyAmount destAddonAmount() const { return _fare->destAddonAmount(); }

  const CurrencyCode& destAddonCurrency() const { return _fare->destAddonCurrency(); }

  const Indicator destAddonOWRT() const { return _fare->destAddonOWRT(); }

  // accessors to constructed SITA fare fields
  // ========= == =========== ==== ==== ======

  const RoutingNumber& throughFareRouting() const { return _fare->throughFareRouting(); }
  const Indicator throughMPMInd() const { return _fare->throughMPMInd(); }
  const RuleNumber& throughRule() const { return _fare->throughRule(); }

  // Accessors to PaxTypeFare extra fields
  // ========= == =========== ===== ======

  // fareAmount converted in DisplayCurrency
  MoneyAmount& convertedFareAmount() { return _convertedFareAmount; }
  const MoneyAmount& convertedFareAmount() const { return _convertedFareAmount; }

  MoneyAmount& convertedFareAmountRT() { return _convertedFareAmountRT; }
  const MoneyAmount& convertedFareAmountRT() const { return _convertedFareAmountRT; }

  const MoneyAmount totalFareAmount() const;

  bool isMiscFareTagValid(void) const;
  MiscFareTag* miscFareTag(void) const;
  MiscFareTag*& miscFareTag(void);

  // return data for a category without allocating when not found
  PaxTypeFareRuleData* paxTypeFareRuleData(uint16_t cat) const;
  PaxTypeFareRuleData* paxTypeFareGfrData(uint16_t cat) const;
  bool isFareRuleDataApplicableForSimilarItin(uint16_t cat) const;

  PaxTypeFareRuleDataByCatNo* paxTypeFareRuleDataMap()
  {
    return _paxTypeFareRuleDataMap.get();
  }
  const PaxTypeFareRuleDataByCatNo* paxTypeFareRuleDataMap() const
  {
    return _paxTypeFareRuleDataMap.get();
  }

  // Price By Cabin Fare Display
  bool isFarePassForInclCode(uint8_t inclusionNumber) const;
  void setFareStatusForInclCode(uint8_t inclusionNumber, bool status)
  {
    _fareStatusPerInclCode[inclusionNumber] = status;
    if(status && _fareDisplayInfo)
      _fareDisplayInfo->setPassedInclusion(inclusionNumber);
  }

  std::map<uint8_t, bool>& fareStatusPerInclCode()
  {
    return _fareStatusPerInclCode;
  }
  const std::map<uint8_t, bool>& fareStatusPerInclCode() const
  {
    return _fareStatusPerInclCode;
  }

  bool alreadyChkedFareRule(uint16_t categoryNumber,
                            DataHandle& dataHandle,
                            PaxTypeFareRuleData*& paxTypeFareRuleData) const;

  bool alreadyChkedGfrRule(uint16_t categoryNumber,
                           DataHandle& dataHandle,
                           PaxTypeFareRuleData*& paxTypeFareRuleData) const;

  // allocates map if not yet set (caller allocs data)
  // rd can be CommonFareRule, General Rule
  void setRuleData(uint16_t cat, DataHandle& dh, PaxTypeFareRuleData* rd, bool isFareRule = true);

  // on exit, ptr to where added; also, allocate if no place to put it
  PaxTypeFareRuleData* setCatRuleInfo(const CategoryRuleInfo& cri,
                                      DataHandle& dh,
                                      bool isLocationSwapped,
                                      bool isFareRule = true);

  // cri can be NULL, means no matched CategoryRuleInfo found
  PaxTypeFareRuleData* setCatRuleInfo(const CategoryRuleInfo* cri,
                                      uint16_t categoryNumber,
                                      DataHandle& dh,
                                      bool isLocationSwapped,
                                      bool isFareRule = true);

  // category specific access
  // if cat not present (or missing data), thows TseException
  const DiscountInfo& discountInfo() const;
  const NegFareRest& negotiatedInfo() const;
  const FareByRuleItemInfo& fareByRuleInfo() const;

  // return 0, if there is no fare rule data or base fare
  PaxTypeFare* getBaseFare(int cat = 25) const;
  // throw exception, if there is no fare rule data or base fare
  PaxTypeFare* baseFare(int cat = 25) const;
  const PaxTypeFare* fareWithoutBase() const;

  virtual FBRPaxTypeFareRuleData* getFbrRuleData() const;
  FBRPaxTypeFareRuleData* getFbrRuleData(uint16_t category) const;

  NegPaxTypeFareRuleData* getNegRuleData() const;

  bool isSpecifiedFare() const;
  bool isSpecifiedCurFare() const;
  const FareByRuleApp& fbrApp() const;

  using Key = uint32_t;
  const VecMap<Key, QualifyFltAppRuleData*>& qualifyFltAppRuleDataMap() const
  {
    return _qualifyFltAppRuleDataMap;
  }

  VecMap<Key, QualifyFltAppRuleData*>& qualifyFltAppRuleDataMap()
  {
    return _qualifyFltAppRuleDataMap;
  }

  const uint16_t maxStopoversPermitted() const { return _maxStopoversPermitted; }
  uint16_t& maxStopoversPermitted() { return _maxStopoversPermitted; }

  // cat14 NVA restriction
  void addNVAData(DataHandle& dataHandle, uint16_t segOrder, const DateTime* NVADate);
  std::map<uint16_t, const DateTime*>* getNVAData() const;
  void useNVAData(std::map<uint16_t, const DateTime*>* NVAData) { _NVAData = NVAData; }

  //-------------------------------------------------------------------------//
  //-- Start shopping methods
  //-------------------------------------------------------------------------//

  const FlightBitmap& flightBitmap() const
  {
    if (_flightBitmapForCarrier)
    {
      return *_flightBitmapForCarrier;
    }

    return (_flightBitmap);
  }

  FlightBitmap& flightBitmap()
  {
    if (_flightBitmapForCarrier)
    {
      return *_flightBitmapForCarrier;
    }

    return (_flightBitmap);
  }

  const VecMap<uint32_t, VecMap<uint64_t, FlightBitmap>>& durationFlightBitmapPerCarrier() const
  {
    return _durationFlightBitmapPerCarrier;
  }

  VecMap<uint32_t, VecMap<uint64_t, FlightBitmap>>& durationFlightBitmapPerCarrier()
  {
    return _durationFlightBitmapPerCarrier;
  }

  const FlightBitmapPerCarrier& flightBitmapPerCarrier() const { return _flightBitmapPerCarrier; }

  FlightBitmapPerCarrier& flightBitmapPerCarrier() { return _flightBitmapPerCarrier; }

  const VecMap<uint32_t, FlightBit>& flightBitmapESV() const { return (_flightBitmapESV); }

  VecMap<uint32_t, FlightBit>& flightBitmapESV() { return (_flightBitmapESV); }

  bool setFlightBitmapSize(const uint32_t& flightBitmapSize);

  // Returns the size of the flight bitmap
  uint32_t getFlightBitmapSize() const
  {
    if (_flightBitmapForCarrier)
      return (uint32_t)_flightBitmapForCarrier->size();

    return (_flightBitmapSize);
  }

  bool& shoppingComponentValidationFailed() { return (_shoppingComponentValidationFailed); }
  const bool& shoppingComponentValidationFailed() const
  {
    return (_shoppingComponentValidationFailed);
  }

  bool& shoppingComponentValidationPerformed() { return (_shoppingComponentValidationPerformed); }

  const bool& shoppingComponentValidationPerformed() const
  {
    return (_shoppingComponentValidationPerformed);
  }

  bool& flightBitmapAllInvalid() { return (_flightBitmapAllInvalid); }
  const bool& flightBitmapAllInvalid() const { return (_flightBitmapAllInvalid); }

  // Returns validity of a certain flight in spiecified durationFlightBitmap
  // false means the flight is not valid
  // true means the flight is valid
  bool isFlightValid(const uint64_t& duration, const uint32_t& i) const;

  // Returns validity of a certain flight
  // false means the flight is not valid
  // true means the flight is valid
  bool isFlightValid(const uint32_t& i) const
  {
    if (_flightBitmapForCarrier)
    {
      return (_flightBitmapForCarrier->size() > i && (*_flightBitmapForCarrier)[i]._flightBit == 0);
    }

    return ((i < _flightBitmapSize) && (_flightBitmap[i]._flightBit == 0));
  }

  bool isFlightSkipped(const uint32_t i) const
  {
    const FlightBitmap& fbmap = _flightBitmapForCarrier ? *_flightBitmapForCarrier : _flightBitmap;
    return i < fbmap.size() && fbmap[i]._flightBit == 'S';
  }

  bool isFlightValidESV(const uint32_t& i) { return (_flightBitmapESV[i]._flightBit == 0); }

  // Returns the invalidity of a certain flight

  // false means the flight is not invalid
  // true means the flight is invalid
  bool isFlightInvalid(const uint32_t& i) const { return (!isFlightValid(i)); }

  // Sets a flight bit to its invalid state
  // false means the operation did not succeed
  // true means the operation succeeded
  bool setFlightInvalid(const uint32_t& i, uint8_t errorNum = '1');

  void setFlightInvalidESV(const uint32_t& i, uint8_t errorNum)
  {
    _flightBitmapESV[i]._flightBit = errorNum;
  }

  // Sets a flight bit in duration to its invalid state
  // false means the operation did not succeed
  // true means the operation succeeded
  bool setFlightInvalid(FlightBitmap& flightBitMap, const uint32_t& i, uint8_t errorNum = '1');

  // Gets the flight bit value
  uint8_t* getFlightBit(const uint32_t& i);

  // Returns ptr to status of a certain flight in spiecified durationFlightBitmap
  const uint8_t* getFlightBit(const uint64_t& duration, const uint32_t& i);

  // Checks whether the entire bitmap is invalid
  // False means the entire bitmap is not invalid
  // True means the entire bitmap is not valid
  bool isFlightBitmapInvalid(bool skippedAsInvalid = false) const;

  static bool
  isFlightBitmapInvalid(const FlightBitmap& flightBitmap, bool skippedAsInvalid = false);

  // Set the flight booking code status into the
  // appropriate flight "bit"
  // Returns false if the operation failed
  // Returns true if the operation succeeded
  bool setFlightBookingCodeStatus(const uint32_t& i, const BookingCodeStatus& bookingCodeStatus);

  // Set the flight MPM percentage into the
  // appropriate flight "bit"
  // Returns false if the operation failed
  // Returns true if the operation succeeded
  bool setFlightMPMPercentage(const uint32_t& i, const uint16_t& mpmPercentage);

  bool swapInFlightSegmentStatus(const uint32_t& i, std::vector<SegmentStatus>& segmentStatus);

  // Get a pointer to the flight segment status
  // Returns NULL if the operation failed
  // Returns a pointer to the data if the operation succeeded
  std::vector<SegmentStatus>* getFlightSegmentStatus(const uint32_t& i);
  const std::vector<SegmentStatus>* getFlightSegmentStatus(const uint32_t& i) const;

  // Only to be set during bitmap validation
  void setIsShoppingFare() { _isShoppingFare = true; }

  void setIsNotShoppingFare() { _isShoppingFare = false; }

  bool isInvalidForNonStops() const { return _isInvalidForNonStops; }
  void setInvalidForNonStops(bool v) { _isInvalidForNonStops = v; }

  MoneyAmount getSoloSurcharges() const { return _soloSurcharges; }
  void setSoloSurcharges(MoneyAmount value) { _soloSurcharges = value; }

  MoneyAmount baggageLowerBound() const { return _baggageLowerBound; }
  void setBaggageLowerBound(MoneyAmount lb) { _baggageLowerBound = lb; }

  // alternate dates
  void initAltDates(const ShoppingTrx& trx);

  uint8_t getAltDateStatus(const DatePair& dates) const;
  uint8_t getAltDateStatus(const DateTime& date, uint8_t leg) const;
  void setAltDateStatus(const DatePair& dates, uint8_t status);

  bool getAltDatePass(const DatePair& dates) const;
  bool getAltDatePass(const DateTime& date, uint8_t leg) const;
  bool isAltDateValid();
  VecMap<DatePair, uint8_t>& altDateStatus() { return _altDateStatus; }
  const VecMap<DatePair, uint8_t>& altDateStatus() const { return _altDateStatus; }

  VecMap<uint64_t, FlightBitmap>& durationFlightBitmap()
  {
    if (_durationFlightBitmapForCarrier)
      return *_durationFlightBitmapForCarrier;
    return _durationFlightBitmap;
  }

  const VecMap<uint64_t, FlightBitmap>& durationFlightBitmap() const
  {
    if (_durationFlightBitmapForCarrier)
      return *_durationFlightBitmapForCarrier;
    return _durationFlightBitmap;
  }

  void setBKSNotApplicapleIfBKSNotPASS();

  std::vector<AltDateFltBitStatus>& altDateFltBitStatus() { return _altDateFltBitStatus; }
  const std::vector<AltDateFltBitStatus>& altDateFltBitStatus() const
  {
    return _altDateFltBitStatus;
  }

  bool setAltDateFltBitStatusSize();

  bool setAltDateFltBitBKCProcessed(const uint32_t& fltBit);
  bool setAltDateFltBitBKCFailed(const uint32_t& fltBit);

  bool isAltDateFltBitBKCProcessed(const uint32_t& fltBit);
  bool isAltDateFltBitBKCPassed(const uint32_t& fltBit);

  bool setAltDateFltBitRTGProcessed(const uint32_t& fltBit);
  bool setAltDateFltBitRTGFailed(const uint32_t& fltBit);

  bool isAltDateFltBitRTGProcessed(const uint32_t& fltBit);
  bool isAltDateFltBitRTGPassed(const uint32_t& fltBit);

  bool setAltDateFltBitGLBProcessed(const uint32_t& fltBit);
  bool setAltDateFltBitGLBFailed(const uint32_t& fltBit);

  bool isAltDateFltBitGLBProcessed(const uint32_t& fltBit);
  bool isAltDateFltBitGLBPassed(const uint32_t& fltBit);

  //-------------------------------------------------------------------------//
  //-- End shopping methods
  //-------------------------------------------------------------------------//

  // Not a DataModel Attribute.This unary predicate is used in FVO (count_if)
  bool isValidFare() const
  {
    return isValid() || (_retrievalInfo && _retrievalInfo->_flag & FareMarket::RetrievKeep &&
                         (!isRoutingProcessed() || isRoutingValid()));
  }

  void copyBasis(std::string& basis,
                 std::string& tktCode,
                 const FareClassCode& fc,
                 const TktDesignator& tc) const;
  void createModifier(std::string& str,
                      const TktDesignatorModifier& mod,
                      const MoneyAmount& percent,
                      const bool asDiscount) const;

  void createModifier(std::string& str,
                      const Indicator& mod,
                      const MoneyAmount& percent,
                      const bool asDiscount) const;

  void ChInDefault(PricingTrx& trx,
                   Indicator fcChildInfantInd,
                   std::string& tktCodeDefault,
                   std::string& tktDesDefault,
                   bool& appendDiscount) const;

  void createTktDesignator(std::string& des) const;

  virtual std::string createFareBasisCodeFD(FareDisplayTrx& trx) const;

  bool isFareBasisEqual(const std::string& fareBasis);
  std::string createFareBasis(PricingTrx* trx, bool ignCat19ChInf = false) const;
  std::string createFareBasis(PricingTrx& trx, bool ignCat19ChInf = false) const;
  std::string createFareBasis(PricingTrx& trx, const std::string& ufbCode) const;
  void createFareBasisRoot(std::string& basis) const;

  void insertBookingCode(std::string& fareBasisCode) const;

  void appendTktDesig(std::string& fareBasisCode,
                      std::string& designator,
                      char tktDesLength,
                      std::string::size_type maxLenAll = 15,
                      std::string::size_type maxLenFB = 10) const;

  /**
   * This methods obtains a new PaxTypeFare pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   *
   * NOTE: see void clone() for details
   */
  PaxTypeFare* clone(DataHandle& dataHandle,
                     bool resetStatus = true,
                     FareMarket* newFm = nullptr,
                     Fare* newFare = nullptr) const;

  /**
   * This methods populates a given PaxTypeFare to be
   * 'equal' to the current object
   *
   * @param PaxTypeFare - object to populate
   *
   * NOTE: Not all fields are copied (e.g. PaxTypeFarePlusUp)
   * Also, it's a shallow copy (pointers refer to orginal data).
   * Caller must allocate/clone
   */
  void clone(DataHandle& dataHandle,
             PaxTypeFare& cloneObj,
             bool resetStatus = true,
             FareMarket* newFm = nullptr,
             Fare* newFare = nullptr) const;

  Indicator& fareDisplayCat35Type() { return _fareDisplayCat35Type; }
  const Indicator& fareDisplayCat35Type() const { return _fareDisplayCat35Type; }

  static bool fareSort(const PaxTypeFare* lptf, const PaxTypeFare* rptf);

  class CabinComparator
  {
  public:
    CabinComparator() = default;
    explicit CabinComparator(bool compareCabins);

    const bool shouldCompareCabins() const { return _compareCabins; }
    bool areCabinsEqual(const CabinType& left, const CabinType& right) const;
    bool isLessCabin(const CabinType& left, const CabinType& right) const;

  private:
    bool _compareCabins = true;
  };

  class FareComparator
  {
  protected:
    const uint16_t _paxTypeNum;
    PaxTypeBucket& _cortege;
    const CabinComparator _cabinComparator;
    const bool _isCmdPricing;

  public:
    FareComparator(PaxTypeBucket& cortege, uint16_t paxTypeNum, bool compareCabins,
                   const bool isCmdPricing = false);
    bool operator()(const PaxTypeFare* lptf, const PaxTypeFare* rptf) const;
  };

  class CxrPrefFareComparator : public FareComparator
  {
  public:
    CxrPrefFareComparator(PaxTypeBucket& cortege, uint16_t paxTypeNum, bool compareCabins)
      : FareComparator(cortege, paxTypeNum, compareCabins)
    {
    }

    bool operator()(const PaxTypeFare* lptf, const PaxTypeFare* rptf) const;
  };

  bool& cat25BasePaxFare() { return _cat25BasePaxFare; }
  bool cat25BasePaxFare() const { return _cat25BasePaxFare; }
  bool isPSRApplied() const;

  bool& puRuleValNeeded() { return _puRuleValNeeded; }
  bool puRuleValNeeded() const { return _puRuleValNeeded; }

  void setNationFRInCat35(const bool isValid = true) { _nationFranceInCat35 = isValid; }
  bool isNationFRInCat35() const { return _nationFranceInCat35; }

  void setSpanishDiscountEnabled() { _spanishDiscountEnabled = true; }
  bool isSpanishDiscountEnabled() const { return _spanishDiscountEnabled; }

  void setIsMipUniqueFare(bool b) { _isMipUniqueFare = b; }
  bool isMipUniqueFare() const { return _isMipUniqueFare; }

  void setLongConnectFare(bool b) { _longConnectFare = b; }
  bool isLongConnectFare() const { return _longConnectFare; }

  void setMatchFareFocusRule(bool b) { _matchFareFocusRule = b; }
  bool isMatchFareFocusRule() const { return _matchFareFocusRule; }

  void setFareCallbackFVO(bool passed = true)
  {
    if (LIKELY(_fareCallbackFVOForCarrier))
      *_fareCallbackFVOForCarrier = passed;
    else
      _fareCallbackFVO = passed;
  }

  const bool isFareCallbackFVO() const
  {
    if (_fareCallbackFVOForCarrier)
      return *_fareCallbackFVOForCarrier;
    return _fareCallbackFVO;
  }

  void setFltIndependentValidationFVO(bool passed = true)
  {
    if (_fltIndependentValidationFVOForCarrier)
      *_fltIndependentValidationFVOForCarrier = passed;
    else
      _fltIndependentValidationFVO = passed;
  }

  const bool isFltIndependentValidationFVO() const
  {
    if (_fltIndependentValidationFVOForCarrier)
      return *_fltIndependentValidationFVOForCarrier;
    return _fltIndependentValidationFVO;
  }

  void setComponentValidationForCarrier(const uint32_t carrierKey,
                                        bool isAltDates,
                                        const uint64_t& duration);

  void setDurationUsedInFVO(const uint64_t durationUsedInFVO)
  {
    _durationUsedInFVO = durationUsedInFVO;
  }
  uint64_t getDurationUsedInFVO() { return _durationUsedInFVO; }
  const uint64_t getDurationUsedInFVO() const { return _durationUsedInFVO; }

  // Cat15 security functions
  void setCat15SecurityFail(bool newValue = true) { _cat15SecurityFail = newValue; };
  void setCat15SoftPass(bool cat15SoftPass);
  bool cat15SecurityFail() const { return _cat15SecurityFail; }
  bool cat15SoftPass() const { return _cat15SoftPass; }
  void setCat15HasT996FT(bool cat15HasT996FT) { _cat15HasT996FT = cat15HasT996FT; }
  void setCat15HasT996FR(bool cat15HasT996FR) { _cat15HasT996FR = cat15HasT996FR; }
  void setCat15HasT996GR(bool cat15HasT996GR) { _cat15HasT996GR = cat15HasT996GR; }

  bool isCat15HasT996FT() const { return _cat15HasT996FT; }
  bool isCat15HasT996FR() const { return _cat15HasT996FR; }
  bool isCat15HasT996GR() const { return _cat15HasT996GR; }
  bool isCat15HasT996() const { return (_cat15HasT996FT || _cat15HasT996FR || _cat15HasT996GR); }

  // _rec2Cat10 can be NULL because PO may not populate it
  const CombinabilityRuleInfo* rec2Cat10() const { return _rec2Cat10; }
  CombinabilityRuleInfo*& rec2Cat10() { return _rec2Cat10; }

  bool& r2Cat10LocSwapped() { return _r2Cat10LocSwapped; }
  const bool r2Cat10LocSwapped() const { return _r2Cat10LocSwapped; }

  const DateTime& retrievalDate() const
  {
    return _retrievalInfo ? _retrievalInfo->_date : DateTime::emptyDate();
  }

  const FareMarket::FareRetrievalFlags retrievalFlag() const
  {
    return _retrievalInfo ? _retrievalInfo->_flag : FareMarket::RetrievNone;
  }

  FareMarket::RetrievalInfo*& retrievalInfo() { return _retrievalInfo; }
  const FareMarket::RetrievalInfo* const retrievalInfo() const { return _retrievalInfo; }

  void setAdjustedSellingCalcData(AdjustedSellingCalcData* adjSellCalcData)
  {
    _adjSellCalcData = adjSellCalcData;
  }
  const AdjustedSellingCalcData* getAdjustedSellingCalcData() const { return _adjSellCalcData; }

  bool isAdjustedSellingBaseFare() const { return _isAdjustedSellingBaseFare; }
  void setAdjustedSellingBaseFare() { _isAdjustedSellingBaseFare = true; }

  bool isDummyFare() const;
  void skipAllCategoryValidation()
  {
    _ruleProcessStatus.set(RS_AllCat);
    _fare->skipAllCategoryValidation();
  }

  void setAllCategoryValidForShopping()
  {
    _ruleStatus.set(RS_AllCat, false);
    _fare->setAllCategoryValidForShopping();
  }

  bool areAnyCategoriesProcessed(const CategoryBitSet& catSet) const
  {
    if (!catSet.any())
      return false;

    for (size_t pos = 0; pos < catSet.size(); ++pos)
    {
      if (catSet.test(pos) && isCategoryProcessed(pos))
        return true;
    }
    return false;
  }

  bool isCWTMarketFare() const;

  uint32_t& bkgCodeTypeForRex() { return _bkgCodeTypeForRex; }
  uint32_t bkgCodeTypeForRex() const { return _bkgCodeTypeForRex; }

  MileageInfo*& mileageInfo();
  const MileageInfo* mileageInfo() const;

  // _cat25Fare can be NULL. It will be temporary populated for some rules validation.
  const PaxTypeFare* cat25Fare() const { return _cat25Fare; }
  PaxTypeFare*& cat25Fare() { return _cat25Fare; }

  uint32_t getRebookedClassesStatus();

  int& mileage() { return _mileage; }
  int mileage() const { return _mileage; }

  bool isOutboundFareForCarnivalInbound() const
  {
    return _fare ? _fare->isOutboundFareForCarnivalInbound() : false;
  }

  bool applyNonIATARounding(const PricingTrx& trx);
  bool isNetRemitTktIndicatorAorB() const;

  bool hasFlexFareValidationStatus() const { return bool(_flexFaresValidationStatus); }
  const flexFares::ValidationStatusPtr getFlexFaresValidationStatus() const;
  flexFares::ValidationStatusPtr getMutableFlexFaresValidationStatus();
  void initializeFlexFareValidationStatus(const PricingTrx& trx);

  void setFlexFaresGroupId(const flexFares::GroupId& id) { _groupIds.insert(id); }

  bool& jumpedDownCabinAllowed(){return jumpedDownAllowed;}
  bool jumpedDownCabinAllowed()const {return jumpedDownAllowed;}

  void initializeMaxPenaltyStructure(DataHandle& dataHandle);

protected:
  std::vector<BrandStatusWithDirection> _brandStatusWithDirection;

  std::vector<CarrierCode> _validatingCarriers;

  bool validForSFRMultiPaxRequest(const FareMarket& fm, bool fxCnException, PricingTrx* trx) const;

  bool validForFilterAndCmdPricingSingleFareBasis(const FareMarket& fareMarket,
                                                  bool fxCnException,
                                                  PricingTrx* trx) const;

  bool validForSpecialFareType(const FareMarket& fareMarket, bool fxCnException) const;

  bool validForFilterAndCmdPricing(const FareMarket& fm, bool fxCnException, PricingTrx* trx) const;

  bool isFMValidForFilterAndCmdPricing(const FareMarket& fareMarket) const;

  bool isChild() const;
  bool isInfant() const;

  bool needSecondRoeDateAmount() const;

  bool isValidInternal(bool needBookingProcessed = false) const;

  bool isMainDataValid() const;

  struct FareBasisCodes
  {
    std::string basis;
    std::string tktCode;
    std::string modCode;
    std::string des;
    std::string tktCodeDefault;
    std::string tktDesDefault;
    std::string ufbCode; // unique fare basis code
  };
  std::string setFareBasis(PricingTrx* trx, FareBasisCodes& fbCodes, bool ignCat19ChInf) const;
  void setCat35Designator(PricingTrx* trx, std::string& des) const;
  void setDiscountedDesignator(PricingTrx* trx,
                               MoneyAmount& percent,
                               const FareCalcConfig* fareCalcConfig,
                               bool ignCat19ChInf,
                               bool& needTktCodeDefault,
                               std::string& modString,
                               FareBasisCodes& fbCodes) const;
  void setFareByRuleDesignator(MoneyAmount& percent,
                               std::string& modString,
                               FareBasisCodes& fbCodes) const;

  // PaxTypeFare status
  // =========== ======
  PaxTypeFareStatus _status;

  int _mixClassStatus = HitMixClass::MX_NOT_APPLICABLE;

  struct RebookInfo
  {
    PaxTypeFare* _asBookedClone = nullptr;
    bool _iAmAsBookedClone = false;
  } _rebookInfo;

  FareDisplayStatus _fareDisplayStatus;
  PaxTypeFareCPFailedStatus _cpFailedStatus; // For 3.5 Command Pricing

  // rule status
  // ==== ======

  // to represent rule status we use negative logic.
  // if for any category status bit == 1, this means
  // that the category was FAILED.

  // rule process uses normal (positive) logic.

  RuleStatus _ruleStatus;
  RuleStatus _ruleProcessStatus;
  RuleStatus _ruleSoftPassStatus;

  static const RuleState _ruleStateMap[RS_MaxCat + 1];

  const RuleState mapRuleStateBit(const unsigned int category) const
  {
    return category > RS_MaxCat ? RS_CatNotSupported : _ruleStateMap[category];
  }

  // booking code status
  // ======= ==== ======

  BookingCodeStatus _bookingCodeStatus{BookingCodeState::BKS_NOT_YET_PROCESSED};
  IbfErrorMessage _ibfErrorMessage = IbfErrorMessage::IBF_EM_NOT_SET;
  bool _notOfferedOnAllSegments = false;
  std::vector<SegmentStatus> _segmentStatus;
  std::vector<SegmentStatus> _segmentStatusRule2;
  char _allowedChangeFareBasisBkgCode = ' ';
  BookingCode _changeFareBasisBkgCode; // to change 1st char of the YY Fare Basis Code
  BookingCode _bookingCode; // for fare display
  BookingCode _s8BFBookingCode;

  // this cache for validation status must be mutable because
  // it is set up in const methond during first getting status
  mutable boost::logic::tribool _keepBrandValidationStatus = notValidated;

  // PaxTypeFare parts
  // =========== =====

  Fare* _fare = const_cast<Fare*>(Fare::emptyFare());
  PaxType* _actualPaxType = nullptr;
  // to keep actual pax type item in the paxtype cortege for each paxtype (fare group process only)
  std::vector<uint16_t> _actualPaxTypeItem;
  FareMarket* _fareMarket = nullptr;
  FareDisplayInfo* _fareDisplayInfo = nullptr;

  const FareClassAppInfo* _fareClassAppInfo = FareClassAppInfo::emptyFareClassApp();
  const FareClassAppSegInfo* _fareClassAppSegInfo = FareClassAppSegInfo::emptyFareClassAppSeg();

  std::vector<const FareClassAppSegInfo*>* _fcasOfOverFlownBkgCds = nullptr;

  static constexpr Indicator PRICING_CATTYPE_NORMAL = 'N';
  static constexpr Indicator PRICING_CATTYPE_SPECIAL = 'S';

  // PaxTypeFare extra fields
  // =========== ===== ======

  MoneyAmount _convertedFareAmount = 0.0;
  MoneyAmount _convertedFareAmountRT = 0.0;
  MoneyAmount _soloSurcharges = 0.0;
  MoneyAmount _baggageLowerBound = 0.0;

  std::unique_ptr<PaxTypeFareRuleDataByCatNo> _paxTypeFareRuleDataMap{new PaxTypeFareRuleDataByCatNo()};
  mutable boost::mutex _maxPenaltyMutex;

  std::map<uint8_t, bool> _fareStatusPerInclCode;

  // FareTypeMatrix
  // ==============

  FareTypeDesignator _fareTypeDesignator;
  Indicator _fareTypeApplication = ' ';
  CabinType _cabin;

  // Max Stopovers
  uint16_t _maxStopoversPermitted = 0;

  // Shopping Fields
  // ======== ======
  FlightBitmap* _flightBitmapForCarrier = nullptr;
  FlightBitmap _flightBitmap;
  FlightBitmapPerCarrier _flightBitmapPerCarrier;
  // CarrierKey, Duration, Bitmap
  VecMap<uint32_t, VecMap<uint64_t, FlightBitmap>> _durationFlightBitmapPerCarrier;
  VecMap<uint32_t, FlightBit> _flightBitmapESV;
  VecMap<DatePair, uint8_t> _altDateStatus;
  VecMap<uint64_t, FlightBitmap> _durationFlightBitmap;
  VecMap<uint64_t, FlightBitmap>* _durationFlightBitmapForCarrier = nullptr;
  std::vector<AltDateFltBitStatus> _altDateFltBitStatus;
  std::unique_ptr<PaxTypeFareStructuredRuleData> _structuredRuleData;

  VecMap<Key, QualifyFltAppRuleData*> _qualifyFltAppRuleDataMap;

  // Misc Fare Tags (Cat 23) pointer:
  MiscFareTag* _miscFareTag = nullptr;

  uint32_t _flightBitmapSize = 0;
  bool _shoppingComponentValidationFailed = false;
  bool _shoppingComponentValidationPerformed = false;
  bool _flightBitmapAllInvalid = false;
  bool _isShoppingFare = false;
  bool _isInvalidForNonStops = false;

  static const char negFareIsSellingLFare; // NegFare is a selling fare
  static const char negFareIsBundledBFare; // NegFare is a bundled fare
  static const char negFareIsNetTFare; // NegFare is a Net fare
  static const char negFareIsNetCFare; // NegFare is a Net fare
  static const char NonRefundable;
  static const std::string MatchNR;

  static const PaxTypeCode matchJNNPaxType;
  static const PaxTypeCode matchJNFPaxType;
  static const PaxTypeCode matchUNNPaxType;

  bool _puRuleValNeeded = false;

  // routing mileage status
  // ======= ==== ======
  bool _surchargeExceptionApplies = false;

  // cached fields
  TariffCategory _tcrTariffCat = PUBLIC_TARIFF;
  TariffNumber _tcrRuleTariff = 0;
  CarrierCode _carrier;
  RuleNumber _ruleNumber;

  void resetCat35ViewNetIndicator() // Used only by NegFareController as Cat35s are built.
  {
    // resets this bit.
    _fareDisplayStatus.set(FD_Cat35_ViewNetIndicator, false);
  }

private:
  static Logger _logger;

  virtual bool applyNonIATAR(const PricingTrx& trx,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const RuleNumber& ruleNumber);

  void updateCache(const Fare* fare);

  std::vector<std::string> _csTextMessages;
  std::vector<SurchargeData*> _surchargeData;
  MoneyAmount _nucTotalSurchargeFareAmount = 0.0;

  std::string _spanishResidentFareBasisSuffix;

  std::map<uint16_t, const DateTime*>* _NVAData = nullptr;

  // _rec2Cat10 can be NULL because PO may not populate it
  //
  CombinabilityRuleInfo* _rec2Cat10 = nullptr;

  uint32_t _bkgCodeTypeForRex = BkgCodeSegState::BKSS_NOT_YET_PROCESSED;

  Indicator _penaltyRestInd = 'N'; // Values are N - no penalties or Y - penalties apply
  bool _changePenaltyApply = false; // Set and used during NVA/NVB process

  bool _cat25BasePaxFare = false;
  Indicator _fareDisplayCat35Type = ' ';

  bool _cat15SecurityFail = false;
  bool _cat15SoftPass = false;
  bool _cat15HasT996FT = false;
  bool _cat15HasT996FR = false;
  bool _cat15HasT996GR = false;

  bool _needRecalculateCat12 = true;

  bool _r2Cat10LocSwapped = false;

  bool _failedByJalAxessRequest = false;

  bool _ticketedFareForAxess = false;

  bool _nationFranceInCat35 = false;

  bool _spanishDiscountEnabled = false;
  bool _isMipUniqueFare = true;
  bool _nonRefundable = false;
  // indicator if the fare is only valid to use for long-connect SOPs
  bool _longConnectFare = false;
  bool _matchFareFocusRule = false;
  bool _routingInvalidByMPM = false;
  bool _requestedFareBasisInvalid = false;
  bool _isValidForBranding = true; // Valid for Interline Branded Fares and PBB
  bool _fareCallbackFVO = false;
  bool _fltIndependentValidationFVO = false;
  Indicator _applyNonIATARounding = BLANK;

  bool* _fareCallbackFVOForCarrier = nullptr;
  bool* _fltIndependentValidationFVOForCarrier = nullptr;
  VecMap<uint32_t, bool> _fareCallbackFVOPerCarrier;
  VecMap<uint32_t, VecMap<uint64_t, bool>> _durationFltIndependentValidationCallbackPerCarrier;
  VecMap<uint32_t, bool> _fltIndependentValidationFVOPerCarrier;
  VecMap<uint32_t, VecMap<uint64_t, bool>> _durationFltIndependentValidationPerCarrier;

  FareMarket::RetrievalInfo* _retrievalInfo = nullptr;

  AdjustedSellingCalcData* _adjSellCalcData = nullptr;
  bool _isAdjustedSellingBaseFare = false;

  // _cat25Fare can be NULL. It will be temporary populated for some rules validation.
  PaxTypeFare* _cat25Fare = nullptr;

  uint64_t _durationUsedInFVO = 0;

  flexFares::ValidationStatusPtr _flexFaresValidationStatus;
  flexFares::GroupsIds _groupIds;
  bool jumpedDownAllowed = true;

  // Specify maximum penalty
  // please remove _penaltyRecordsOld when removing smpShoppingISCoreFix fallback
  MaxPenaltyInfoRecords* _penaltyRecordsOld = nullptr;
  bool _smpShoppingISCoreFixFallback = true;
  MaxPenaltyInfoRecords _penaltyRecords;

  int _mileage = 0;
  uint16_t _numOfValidatingCxrFailedFareCatRule = 0;

  bool _isValidForRepricing = true;

public:
  void addVoluntaryChangesInfo(const VoluntaryChangesInfoW* r3)
  {
    MaxPenaltyInfoRecords& recordsReference =
        (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

    boost::mutex::scoped_lock g(_maxPenaltyMutex);
    recordsReference._voluntaryChangesInfo.insert(r3);
  }

  void addPenaltyInfo(const PenaltyInfo* r3)
  {
    MaxPenaltyInfoRecords& recordsReference =
        (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

    boost::mutex::scoped_lock g(_maxPenaltyMutex);
    recordsReference._penaltyInfo.insert(r3);
  }

  void addVoluntaryRefundsInfo(const VoluntaryRefundsInfo* r3)
  {
    MaxPenaltyInfoRecords& recordsReference =
        (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

    boost::mutex::scoped_lock g(_maxPenaltyMutex);
    recordsReference._voluntaryRefundsInfo.insert(r3);
  }

  const std::unordered_set<const PenaltyInfo*>& getPenaltyInfo() const;
  const std::unordered_set<const VoluntaryChangesInfoW*>& getVoluntaryChangesInfo() const;
  const std::unordered_set<const VoluntaryRefundsInfo*>& getVoluntaryRefundsInfo() const;

  void setPenaltyLoadedFlag()
  {
    MaxPenaltyInfoRecords& recordsReference =
        (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

    recordsReference._penaltyRecordsLoaded = true;
  }

  bool arePenaltyRecordsLoaded() const
  {
    const MaxPenaltyInfoRecords& recordsReference =
        (!_smpShoppingISCoreFixFallback ? _penaltyRecords : *_penaltyRecordsOld);

    return recordsReference._penaltyRecordsLoaded;
  }

  const std::vector<BrandStatusWithDirection>& getBrandStatusVec() const
  {
    return _brandStatusWithDirection;
  }

  std::vector<BrandStatusWithDirection>& getMutableBrandStatusVec()
  {
    return _brandStatusWithDirection;
  }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers; }
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers; }

  RuleStatus& ruleStatus() { return _ruleStatus; }

  void setValidatingCxrInvalid(const CarrierCode& cxr, uint16_t failedCatNum);
  void setValidatingCxrInvalid(const std::vector<CarrierCode>& cxrs, uint16_t failedCatNum);
  void removeFromValidatingCarrierList(const CarrierCode& cxr);
  void consolidValidatingCxrList();

#ifdef TRACKING
  void track(const std::string& label, const std::string& additionalInfo = "") const
  {
    _trackCollector.collect(label, additionalInfo);
  }

  void track(bool condition, const std::string& label, const std::string& additionalInfo = "") const
  {
    if (condition)
      _trackCollector.collect(label, additionalInfo);
  }

  const TrackCollector& tracks() const { return _trackCollector; }
#else
  void track(const std::string&, const std::string& = "") const {}

  void track(bool, const std::string&, const std::string& = "") const {}
#endif

protected:
#ifdef TRACKING
  mutable TrackCollector _trackCollector;
#endif
};

using PaxTypeFarePtrVec = std::vector<PaxTypeFare*>;
using PaxTypeFarePtrVecI = std::vector<PaxTypeFare*>::iterator;
using PaxTypeFarePtrVecCI = std::vector<PaxTypeFare*>::const_iterator;
} // tse namespace
