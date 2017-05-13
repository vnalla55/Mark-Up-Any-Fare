//----------------------------------------------------------------------------
//  File:         Diagnostic.h
//  Description:  Root Diagnostic object, Container of all the diagnostic messages
//
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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

#include "Common/Global.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Util/BranchPrediction.h"

#include <boost/thread/mutex.hpp>

#include <deque>
#include <map>
#include <string>
#include <tr1/tuple>

namespace tse
{
class PaxTypeFare;
class FareMarket;

enum DiagnosticTypes
{ DiagnosticNone = 0, // default one
  LegacyDiagnostic8 = 8, // currently 610
  LegacyDiagnostic10 = 10, // currently 631
  LegacyDiagnostic11 = 11, // currently 634
  LegacyDiagnostic12 = 12, // currently 608
  LegacyDiagnostic74 = 74, // currently 660
  Diagnostic24 = 24,
  Diagnostic85 = 85,
  SimilarItinIADiagnostic = 149,
  Diagnostic150 = 150, // Status
  Diagnostic185 = 185, // Price by Cabin
  Diagnostic187 = 187, // RBD by Cabin - ANSWER Table
  Diagnostic151 = 151, // Metrics
  Diagnostic188 = 188, // TPM (Ticket Point Mileage)
  Diagnostic191 = 191, // Ticketing Cxr Service and Validating Cxr
  Diagnostic192 = 192, // Itin Analyzer
  Diagnostic193 = 193, // Customer Configuration Diagnostic
  Diagnostic195 = 195, // ATAE call
  Diagnostic194 = 194, // Change status of itin segments for Rex Pricing
  Diagnostic198 = 198,
  Diagnostic199 = 199,
  AllFareDiagnostic = 200,
  Diagnostic201 = 201, // Fare with Tafiff Inhibited
  Diagnostic202 = 202, // R2 matching
  Diagnostic203 = 203, // Fare type pricing table
  Diagnostic204 = 204, // Record1 unmatched
  Diagnostic207 = 207, // Inclusion code diagnostic
  Diagnostic208 = 208, // Fare By Rule - Record 8
  Diagnostic209 = 209, // User preference
  Diagnostic212 = 212, // Currency/Suppression Diagnostic
  Diagnostic219 = 219,
  Diagnostic220 = 220, // True Pax Type Display
  Diagnostic223 = 223, // Miscellaneous Fare Tags
  Diagnostic225 = 225, // Fare By Rule - Record 2 Cat 25
  Diagnostic231 = 231, // Voluntary changes - retrieve VCTR from selected fare
  Diagnostic233 = 233, // Voluntary refund - retrieve VCTR from selected fare
  Diagnostic240 = 240, // Fare Focus Rules
  Diagnostic251 = 251, // add-on construction
  Diagnostic252 = 252, // add-on construction
  Diagnostic253 = 253, // add-on construction
  Diagnostic254 = 254, // add-on construction
  Diagnostic255 = 255, // add-on construction
  Diagnostic256 = 256, //
  Diagnostic257 = 257, // Duplicate Display of Add Ons Diagnostic
  Diagnostic258 = 258, // Constructed vs Published dup. elimination
  Diagnostic259 = 259, // Constructed Fares Cache
  Diagnostic270 = 270,
  Diagnostic271 = 271, // Industry Fares - Multilateral
  Diagnostic272 = 272, // Industry Fares - YY
  Diagnostic273 = 273, // Industry Fares - Pass/Fail
  Diagnostic277 = 277, // Industry Fares - Fare vector
  Diagnostic291 = 291, // Fares processed by RD/WPRD
  Diagnostic299 = 299, // Release (Unused/Failed) Fares
  Diagnostic300 = 300,
  Diagnostic301 = 301,
  Diagnostic302 = 302,
  Diagnostic303 = 303,
  Diagnostic304 = 304, // Flight Application
  Diagnostic305 = 305, // Advance Reservation/Ticketing
  Diagnostic306 = 306, // Minimum Stay Restriction Rule
  Diagnostic307 = 307, // Maximum Stay Restriction Rule
  Diagnostic308 = 308, // Stopovers
  Diagnostic309 = 309, // Transfers
  Diagnostic311 = 311,
  Diagnostic312 = 312, // Surcharges Rule
  Diagnostic313 = 313, // Accompanying Travel
  Diagnostic314 = 314, // Travel Restriction
  Diagnostic315 = 315,
  Diagnostic316 = 316, // Penalties
  Diagnostic319 = 319,
  Diagnostic323 = 323, // Miscellaneous Fare Tags
  Diagnostic325 = 325, // Cat 25 - Fare By Rule
  Diagnostic331 = 331, // Cat 31 - Voluntory Exchange
  Diagnostic333 = 333, // Cat 33 - Voluntory Refunds
  Diagnostic327 = 327, // Cat 27 - Tours
  Diagnostic335 = 335, // Cat 35 - Negotiated
  Diagnostic350 = 350,
  Diagnostic355 = 355,
  Diagnostic370 = 370, // Limitation on Indirect Travel
  Diagnostic371 = 371, // Sales Restriction by Nation
  Diagnostic372 = 372, // ...
  Diagnostic399 = 399, // Reserved
  Diagnostic400 = 400, // Result of Booking Code Validation
  Diagnostic404 = 404, // BookingCodeException for Primary Bkg Validation
  Diagnostic405 = 405, // BookingCodeException
  Diagnostic411 = 411, // Booking Code for the one PaxTypeFare
  Diagnostic413 = 413, // Differential Validation
  Diagnostic416 = 416, // Sector Surcharge
  Diagnostic419 = 419, // Final BookingCode validation display
  Diagnostic420 = 420, // MixedClass for PU in the FarePath
  Diagnostic430 = 430, // PU Revalidation Table 999
  Diagnostic450 = 450, // Routing Validation Display
  Diagnostic451 = 451, // Routing Keys for all the faremarkets
  Diagnostic452 = 452, // Routing TPM Exclusion
  Diagnostic455 = 455, // routing map display
  Diagnostic457 = 457, // Route Map and List Debugging Display
  Diagnostic460 = 460, // PaxTypeFare Display Routing Results
  Diagnostic469 = 469, // Release Fares Unused by Cat 25/Routing
  Diagnostic499 = 499, // PaxTypeFare Rule Validation Status
  Diagnostic500 = 500, // Rule Category
  Diagnostic502 = 502, // Rule Record2 Matching
  Diagnostic505 = 505, // Latest/earlist date for tkting
  Diagnostic511 = 511, // Record2 result reuse on FM
  Diagnostic512 = 512, // Surcharge collection
  Diagnostic527 = 527, // TourCodes Fare Path processing
  Diagnostic535 = 535, // Negotiated Fare Rule for IT/BT ticketing
  Diagnostic550 = 550, // Rule pre-validation
  Diagnostic555 = 555, // Rule re-validation for PU scope
  Diagnostic556 = 556, // Journey - only for programmer
  Diagnostic599 = 599,
  Diagnostic600 = 600,
  Diagnostic601 = 601,
  Diagnostic602 = 602, // Cat 31 Tbl988 fare bytes pre-validation
  Diagnostic603 = 603,
  Diagnostic605 = 605,
  Diagnostic606 = 606,
  Diagnostic607 = 607, // FareTypePricing FP/PU match logic
  Diagnostic608 = 608,
  Diagnostic609 = 609,
  Diagnostic610 = 610,
  Diagnostic611 = 611,
  Diagnostic612 = 612,
  Diagnostic614 = 614,
  Diagnostic615 = 615,
  Diagnostic620 = 620,
  Diagnostic625 = 625,
  Diagnostic631 = 631,
  Diagnostic632 = 632,
  Diagnostic633 = 633,
  Diagnostic634 = 634,
  Diagnostic635 = 635,
  Diagnostic636 = 636,
  Diagnostic637 = 637,
  Diagnostic638 = 638,
  Diagnostic639 = 639,
  Diagnostic640 = 640,
  Diagnostic653 = 653,
  Diagnostic654 = 654,
  Diagnostic655 = 655,
  Diagnostic660 = 660,
  Diagnostic661 = 661,
  Diagnostic662 = 662,
  Diagnostic663 = 663,
  Diagnostic666 = 666,
  Diagnostic671 = 671,
  Diagnostic676 = 676, // Multi ticket request
  Diagnostic681 = 681,
  Diagnostic682 = 682,
  Diagnostic688 = 688, // Cat 31 Process Tag Permutations
  Diagnostic689 = 689, // Cat 31 Validation of Repricing Solutions
  Diagnostic690 = 690,
  Diagnostic691 = 691, // Net Remit Fare Path Display
  Diagnostic692 = 692, // Net Remit Fare Selection
  Diagnostic693 = 693, // Markup Any Fare Fare Path Display
  Diagnostic699 = 699,
  Diagnostic700 = 700, // Minimum Fare Checker Driver
  Diagnostic701 = 701,
  Diagnostic702 = 702,
  Diagnostic709 = 709, // Minimum Fare table matching
  Diagnostic710 = 710, // Minimum Fare Fare Selection Process
  Diagnostic711 = 711, // Minimum Fare EPQ-Exception Provision Qualification
  Diagnostic718 = 718, // Minimum Fare HIP/BHC Higher Intermediate/OneWay Backh
  Diagnostic719 = 719, // Minimum Fare CTM
  Diagnostic760 = 760, // Minimum Fare COM
  Diagnostic761 = 761, // Minimum Fare CPM
  Diagnostic762 = 762, // Minimum Fare DMC
  Diagnostic763 = 763, // Construct Market Display
  Diagnostic765 = 765, // Minimum Fare COP
  Diagnostic766 = 766, // Minimum Fare OSC
  Diagnostic767 = 767, // Minimum Fare OJM
  Diagnostic768 = 768, // Minimum Fare RTC (Highest RT check in RT type)
  Diagnostic775 = 775, // Minimum Fare RSC
  Diagnostic776 = 776, // Nigeria Currency Adjustment - Fare Selection
  FailTaxCodeDiagnostic = 800, // TaxCodeData Fail one
  AllPassTaxDiagnostic281 = 801, // TaxOutVector only qualified pass
  TaxRecSummaryDiagnostic = 802, // TaxRecVector
  PFCRecSummaryDiagnostic = 803, // PFCOutVector
  LegacyTaxDiagnostic24 = 804, // Legacy Tax Q/*24 Display
  TaxOrchestratorDiagnostic = 805,
  Diagnostic806 = 806, // Tax Reissue info for cat31 or non-cat31
  Diagnostic807 = 807,
  Diagnostic808 = 808, // TaxDriver
  Diagnostic809 = 809, // TaxApply
  Diagnostic810 = 810, // Tax
  Diagnostic811 = 811, // TaxCodeValidator
  Diagnostic812 = 812, // TaxRound
  Diagnostic813 = 813,
  Diagnostic814 = 814, // AdjustTax
  Diagnostic815 = 815, // FareClassValidator
  Diagnostic816 = 816, // reserve for tax
  Diagnostic817 = 817, // reserve for tax
  Diagnostic818 = 818, // reserve for tax
  Diagnostic819 = 819, // reserve for tax
  Diagnostic820 = 820, // reserve for tax
  Diagnostic821 = 821, // reserve for tax
  Diagnostic822 = 822, // reserve for tax
  Diagnostic823 = 823, // reserve for tax
  Diagnostic824 = 824, // reserve for tax
  Diagnostic825 = 825, // reserve for tax, TaxReissue details
  Diagnostic826 = 826, // reserve for tax
  Diagnostic827 = 827, // reserve for tax
  Diagnostic828 = 828, // reserve for tax
  Diagnostic829 = 829, // reserve for tax
  Diagnostic830 = 830, // ATPCO Taxes - InputRequestDiagnostic, RequestDiagnostic
  Diagnostic831 = 831, // ATPCO Taxes - DBDiagnostic
  Diagnostic832 = 832, // ATPCO Taxes - PositiveDiagnostic
  Diagnostic833 = 833, // ATPCO Taxes - NegativeDiagnostic
  Diagnostic834 = 834, // reserve for tax
  Diagnostic835 = 835, // reserve for tax
  Diagnostic836 = 836, // reserve for tax
  Diagnostic837 = 837, // reserve for tax
  Diagnostic838 = 838, // reserve for tax
  Diagnostic839 = 839, // reserve for tax
  Diagnostic840 = 840, // reserve for tax
  Diagnostic841 = 841, // reserve for tax
  Diagnostic842 = 842, // reserve for tax
  Diagnostic843 = 843, // reserve for tax
  Diagnostic844 = 844, // reserve for tax
  Diagnostic845 = 845, // reserve for tax
  Diagnostic846 = 846, // PFC Pprocess
  Diagnostic847 = 847, // PFC Absorption
  Diagnostic848 = 848, // PFC CollectionMethod
  Diagnostic849 = 849, // PFC Exemption
  Diagnostic850 = 850, // reserved for FareCalc
  Diagnostic851 = 851, // Interline Eletronic Ticket
  Diagnostic852 = 852, // reserved for FareCalc
  Diagnostic853 = 853, // FareCalc Diag
  Diagnostic854 = 854, // FareCalcConfig
  Diagnostic855 = 855, // reserved for FareCalc
  Diagnostic856 = 856, // WQCC Diag
  Diagnostic858 = 858, // Endorsement collection
  // WPA Fare Calc Diagnostics
  Diagnostic860 = 860, // reserved for FareCalc
  Diagnostic861 = 861, // NVB/NVA processing
  Diagnostic863 = 863, // WPA Accompanied Travel
  Diagnostic864 = 864, // Consolidator Plus Up
  Diagnostic865 = 865, // Cat35 commissions
  Diagnostic866 = 866, // Tkt Commission (commission cap)
  Diagnostic867 = 867, // Commission Management
  Diagnostic868 = 868, // Fare Retail Rule
  Diagnostic870 = 870, // Service Fee - OB Fees
  Diagnostic871 = 871, // Service Fee - OB Fees
  Diagnostic872 = 872, // Service Fee - Type R and T OB Fees
  Diagnostic875 = 875, // Service Fee - OC Fees S5
  Diagnostic876 = 876, // Service Fee - OC Fees S6
  Diagnostic877 = 877, // Service Fee - OC Fees S7
  Diagnostic878 = 878, // Service Fee: OC Fees sorting
  Diagnostic879 = 879, // Service Fee: Currency conversion
  Diagnostic880 = 880, // Service Fee: Slice and Dice
  Diagnostic881 = 881, // Service Fee: Pseudo Objects building
  // Branded Fares diagnostics
  Diagnostic888 = 888, // Branded Fares: S8 - Branding programs,fares,services
  Diagnostic889 = 889, // Branded Fares: T189 - Fare identification
  Diagnostic890 = 890, // Branded Fares: XML request to MMGR service
  Diagnostic891 = 891, // Branded Fares: XML response from MMGR service
  Diagnostic892 = 892, // Branded Fares: Display brand info
  Diagnostic893 = 893, // Branded Fares: Display reviewed and parsed data from Branded service
  Diagnostic894 = 894, // Branded Fares: Directionality
  Diagnostic896 = 896, // Branded Fares: T166 - Optional Services
  Diagnostic898 = 898, // Branded Fares: MMGR S8-Branding programs,fares,services
  Diagnostic900 = 900, // Shopping: gov carrier
  Diagnostic901 = 901, // Shopping: gov carrier/thru fare
  Diagnostic902 = 902, // Shopping: Schedule grouping
  Diagnostic903 = 903, // Shopping: Schedule grouped thru fares
  Diagnostic904 = 904, // Shopping: Rule-valided thru fare
  Diagnostic905 = 905, // Shopping: parsed XML
  Diagnostic906 = 906, // Shopping: flight bitmap
  Diagnostic907 = 907, // Shopping: flight bitmap bit info
  Diagnostic908 = 908, // Shopping: cabin class valid
  Diagnostic909 = 909, // Shopping: Routing check against bitmap
  Diagnostic910 = 910, // Shopping: Fare paths
  Diagnostic911 = 911, // Shopping: all validations
  Diagnostic912 = 912, // Shopping: save qualified cat 4
  Diagnostic913 = 913,
  Diagnostic914 = 914, // Shopping: OutputAltDates
  Diagnostic915 = 915, // Shopping: Pricing Unit Rule Validation diagnostics
  Diagnostic916 = 916, // Shopping: Cat-12 Surcharge Rule
  Diagnostic917 = 917, // Shopping: Cat-12 Final Surcharge Collection
  Diagnostic918 = 918, // Shopping: HIP minimum fare check
  Diagnostic919 = 919, // Shopping: CTM minimum fare check
  Diagnostic920 = 920, // Shopping: Fare Market Path Build
  Diagnostic921 = 921, // Shopping: Fare Selection for PU
  Diagnostic922 = 922, // Shopping: Local Fare Markets (SOL)
  Diagnostic923 = 923, // Shopping: Cheapest OW / HRT fare amount (SOL)
  Diagnostic924 = 924, // Shopping: Cheapest amount per solution pattern (SOL)
  Diagnostic925 = 925, // Shopping: Cat-10 Scoreboard and PU validation
  Diagnostic926 = 926, // Shopping: Cat-10 Fare Path Validation
  Diagnostic929 = 929, // Shopping: SOL PQ expansion.
  Diagnostic930 = 930, // Shopping: IS Response
  Diagnostic931 = 931, // Shopping: Group Itin
  Diagnostic941 = 941, // Shopping: Itin generator (SOL)
  Diagnostic942 = 942, // Shopping: Diversity
  Diagnostic952 = 952, // Shopping: ESV/VIS - Fares and Routings cache
  Diagnostic953 = 953, // Shopping: ESV/VIS - Flight related validation
  Diagnostic954 = 954, // Shopping: ESV/VIS - Valid fares for each SOP
  Diagnostic956 = 956, // Shopping: ESV/VIS - PQ
  Diagnostic957 = 957, // Shopping: ESV - Diversity results
  Diagnostic958 = 958, // Shopping: VIS - Diversity results
  Diagnostic959 = 959, // Shopping: ESV/VIS Result itineraries details
  Diagnostic965 = 965, // Shopping: FF Rule Validation
  Diagnostic966 = 966, // Shopping: FF Rule Validation/ PU status
  Diagnostic969 = 969, // Shopping: FF Response
  Diagnostic970 = 970, // Shopping: MIP taxes splitting logic
  Diagnostic975 = 975, // Shopping: Benchmarks for IS and MIP
  Diagnostic980 = 980, // MIPS: Journey Diagnostic
  Diagnostic981 = 981, // MIPS: AltDates Diagnostic
  Diagnostic982 = 982, // MIPS: AltDates Diagnostic Itineraries
  Diagnostic983 = 983, // MIPS: WN SNAP Diagnostic Itineraries
  Diagnostic984 = 984,
  Diagnostic985 = 985,
  Diagnostic986 = 986,
  Diagnostic987 = 987, // Exchange Shopping: Leg Determination & Constrains Consolidation
  Diagnostic988 = 988, // MIP: Validate Interline Ticket Carrier Agreement Diagnostic
  Diagnostic989 = 989, // Exchange Shopping: Response
  Diagnostic990 = 990, // Exchange Shopping: Populate similar Items
  Diagnostic991 = 991, // MIPS: Family Logic saved FPPQItems validation
  // Diagnostic992 = 992, // AirlineShopping dss
  Diagnostic993 = 993, // AirlineShopping merged fare market
  Diagnostic994 = 994, // AirlineShopping combined OW itins into RT
  Diagnostic995 = 995, // AirlineShopping highest/lowest fare paths selection
  Diagnostic996 = 996, // AirlineShopping display soldout condition
  Diagnostic997 = 997, // Used for displaying Carnival SOL subitineraries.
  Diagnostic998 = 998, // Used with /DD213 to get diag 213 from AS V2
  // Can be used in both pricing and shopping
  UpperBound = 999 };

typedef std::map<std::string, std::string> DiagParamMap;
typedef std::map<std::string, std::string>::iterator DiagParamMapVecI;
typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;

class Diagnostic
{
  Diagnostic(const Diagnostic& dg);
  Diagnostic& operator=(const Diagnostic& rhs);

public:
  Diagnostic() : _active(false), _diagnosticType(DiagnosticNone), _processAllServices(false) {}

  explicit Diagnostic(DiagnosticTypes diagType)
    : _active(false),
      _diagnosticType(DiagnosticNone),
      _processAllServices(false)
  {
    if (UNLIKELY((diagType > DiagnosticNone) && (diagType < UpperBound)))
    {
      _diagnosticType = diagType;
    }
  }
  virtual ~Diagnostic() {}

  bool isActive() const { return UNLIKELY(_active); }
  bool isActive(DiagnosticTypes diagnosticType) const { return (_active && _diagnosticType == diagnosticType); }
  void activate();
  void deActivate() { _active = false; }

  std::string toString();
  void insertDiagMsg(const std::string& diagMsg);

  DiagnosticTypes& diagnosticType() { return _diagnosticType; }
  const DiagnosticTypes& diagnosticType() const { return _diagnosticType; }

  DiagParamMap& diagParamMap() { return _diagParamMap; }
  const DiagParamMap& diagParamMap() const { return _diagParamMap; }

  bool diagParamMapItemPresent(const std::string& key) const
  {
    if (_diagParamMap.find(key) == _diagParamMap.end())
      return false;
    return true;
  }

  const std::string& diagParamMapItem(const std::string& key) const
  {
    DiagParamMapVecIC i = _diagParamMap.find(key);
    if (UNLIKELY(i != _diagParamMap.end()))
      return i->second;
    return _emptyString;
  }

  bool diagParamIsSet(const std::string& key, const std::string& value) const
  {
    return diagParamMapItem(key) == value;
  }

  bool shouldDisplay(const LocCode& origLoc,
                     const LocCode& boardMultiCity,
                     const LocCode& destLoc,
                     const LocCode& offMultiCity) const;

  bool shouldDisplay(const FareMarket& fareMarket) const;

  bool shouldDisplay(const PaxTypeFare& paxTypeFare) const;

  bool shouldDisplay(const PaxTypeFare& paxTypeFare, const unsigned short& categoryNumber) const;

  bool shouldDisplay(const PaxTypeFare& paxTypeFare,
                     const unsigned short& categoryNumber,
                     const PaxTypeCode& paxType) const;

  bool matchFareClass(const std::string& diagFareClass, const PaxTypeFare& paxTypeFare) const;

  std::string masterPricingDiagnostic();
  std::string masterAncillaryPricingDiagnostic();

  const bool& processAllServices() const { return _processAllServices; }
  bool& processAllServices() { return _processAllServices; }

  bool filterRulePhase(int phase) const;

  bool isDiag455ParamSet() const;

  void appendAdditionalDataSection(std::string xmlToAdd);
  std::string& getAdditionalData() { return _additionalData;}

  // Diagnostic arguments

  static const std::string NO_LIMIT;
  static const std::string UNLIMITED_RESPONSE;
  static const std::string RESPONSE_SIZE;

  static const std::string DIAG_CARRIER;
  static const std::string DIAG_VENDOR;
  static const std::string MISCELLANEOUS;
  static const std::string ALL_VALID;
  static const std::string DISPLAY_DETAIL;
  static const std::string IDENTIFICATION;

  static const std::string FVO_RULES_BEFORE_BKGCODE;
  static const std::string FVO_BKGCODE_BEFORE_RULES;
  static const std::string WPNC_SOLO_TEST;
  static const std::string ACTIVATE_JOURNEY;

  static const std::string FARE_MARKET;
  static const std::string GLOBAL_DIRECTION;

  static const std::string FARE_ASSIGNMENT;
  static const std::string FARE_BASIS_CODE;
  static const std::string FARE_CLASS_CODE;

  static const std::string FARE_TARIFF;
  static const std::string TARIFF_NUMBER;
  static const std::string FARE_TYPE;
  static const std::string FARE_TYPE_DESIGNATOR;
  static const std::string NO_DUPLICATE;

  static const std::string GATEWAY_PAIR;
  static const std::string ADDON_FARE_CLASS_CODE;
  static const std::string ADDON_FARE_TARIFF;
  static const std::string DATE_INTERVALS;

  static const std::string RULE_NUMBER;

  static const std::string FARE_USAGE_NUMBER;

  static const std::string FARE_PATH;
  static const std::string PRICING_UNIT;

  static const std::string FOOTNOTE;
  static const std::string PAX_TYPE;

  static const std::string TO_ROUTING;
  static const std::string ITIN_TYPE;
  static const std::string SUB_CODE;
  static const std::string SERVICE_GROUP;

  static const std::string SEQ_NUMBER;
  static const std::string BAG_PR_FLOW; // For Baggage
  static const std::string BAG_FB; // baggage data

  static const std::string SRV_GROUP;
  static const std::string SRV_CODE;
  static const std::string RULE_PHASE;
  static const std::string BRAND_ID;
  static const std::string PROGRAM_NAME;
  static const std::string TABLE_DETAILS;

  static const std::string SOP_IDS;
  static const std::string FOS_GENERATOR;

  static const std::string FF_RULE_ID;

  static const std::string TOPLINE_METRICS;
  static const std::string FULL_METRICS;

  static const std::string RBD_ALL;
  static const std::string BOOKING_CODE;
  static const std::string CITY_PAIR;
  static const std::string DIAG_CABIN;
  static const std::string TRAVEL_DATE;
  static const std::string TICKET_DATE;

  static const std::string FARE_RETAILER_TYPE;

  static const std::string COMMISSION_CONTRACT;
  static const std::string MAX_PEN;
  static const std::string PREVALIDATION;
  static const std::string BRANDING_SERVICE_FORMAT;
  static const std::string CDATA_XML;

  static const std::string CDATA_START_TAG;
  static const std::string CDATA_END_TAG;
  static const std::string MATCHING;
  static const std::string SPECIFIC_CATEGORY;

protected:
  virtual TseCallableTrxTask::SequenceID getCurrentTaskSequenceID()
  {
    const TseCallableTrxTask* currentTask = TseCallableTrxTask::currentTask();
    return (currentTask ? currentTask->getSequenceID() : TseCallableTrxTask::SequenceID::EMPTY);
  }

private:
  // lookup sortDiagMsgs() for the reason why tuple is used
  typedef std::tr1::tuple<TseCallableTrxTask::SequenceID,
                          // keep original sequence number in size_t (to do not sort by std::string)
                          std::size_t,
                          std::string> ThreadDiagMsgs;

  typedef std::deque<ThreadDiagMsgs> RootDiagMsgs;

  static std::string _emptyString;
  bool _active;
  DiagnosticTypes _diagnosticType;
  DiagParamMap _diagParamMap;
  boost::mutex _messageMutex;
  RootDiagMsgs _diagMessages;

  bool _processAllServices;

  std::string _additionalData;

  struct IsMsgFromTrxTask;
  void sortDiagMsgs();
  void escapeNullCharacters(std::string& input);
};
} // namespace tse

