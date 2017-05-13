//-------------------------------------------------------------------
//
//  File:        RepriceSolutionValidator.h
//  Created:     September 10, 2007
//  Authors:     Grzegorz Cholewiak
//
//  Updates:
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

#include "Common/ExchangeUtil.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "RexPricing/ExpndKeepFareValidator.h"
#include "RexPricing/GenericRexMapper.h"
#include "RexPricing/NewTicketEqualOrHigherValidator.h"
#include "RexPricing/RexFareBytesValidator.h"
#include "RexPricing/StopoverConnectionValidator.h"
#include "Rules/AdvResOverride.h"


namespace tse
{
class RexPricingTrx;
class ProcessTagPermutation;
class Diag689Collector;
class FarePath;
class Money;
class PaxTypeFare;
class VoluntaryChangesInfoW;
class SequenceStopByteByTag;
class FareMarketRuleController;
class DiagCollector;
class FarePathChangeDetermination;
class RexAdvResTktValidator;
class RoutingController;

enum NewTicketEqualOrHigherCheckResult
{
  NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED,
  NEW_TICKET_EQUAL_OR_HIGHER_PASS,
  NEW_TICKET_EQUAL_OR_HIGHER_FAIL
};

class RepriceSolutionValidator
{
  friend class RepriceSolutionValidatorTest;

public:
  static constexpr Indicator EXTEND_NO_CHANGE = ' ';
  static constexpr Indicator EXTEND_NO_RESTRICTIONS = 'Y';
  static constexpr Indicator EXTEND_AT_LEAST_ONE_POINT = 'X';
  static constexpr Indicator EXTEND_BEYOND_DEST_ONLY = 'B';

  RepriceSolutionValidator(RexPricingTrx& trx, FarePath& fp);
  virtual ~RepriceSolutionValidator();
  bool process();

  typedef std::map<AdvResOverride, bool, AdvResOverride> AdvResOverrideCache;

  struct MixedTagsCacheKey
  {
    MixedTagsCacheKey() : _ptf(nullptr), _fa(UNKNOWN_FA), _tag((ProcessTag)0) {}

    MixedTagsCacheKey(const PaxTypeFare* ptf, FareApplication fa, ProcessTag tag)
      : _ptf(ptf), _fa(fa), _tag(tag)
    {
    }

    const PaxTypeFare* _ptf;
    FareApplication _fa;
    ProcessTag _tag;

    bool operator()(const MixedTagsCacheKey& p1, const MixedTagsCacheKey& p2) const
    {
      if (p1._ptf == p2._ptf)
      {
        if (p1._fa == p2._fa)
          return p1._tag < p2._tag;
        else
          return p1._fa < p2._fa;
      }
      else
        return p1._ptf < p2._ptf;
    }
  };

  struct MixedTagsCacheResult
  {
    MixedTagsCacheResult(std::vector<const PaxTypeFare*>::const_iterator newPTFIter, bool ret)
      : _newPTFIter(newPTFIter), _ret(ret)
    {
    }

    std::vector<const PaxTypeFare*>::const_iterator _newPTFIter;
    bool _ret;
  };

  typedef std::map<MixedTagsCacheKey, MixedTagsCacheResult, MixedTagsCacheKey> MixedTagsCache;

protected:
  static constexpr Indicator OWRT_ONLY_OW_FARES = '1';
  static constexpr Indicator OWRT_ONLY_RT_FARES = '2';
  static constexpr Indicator OWRT_ALL_FARES_AVAILABLE = ' ';

  void findFirstFare();
  void findFareRetrievalFlag();
  void analyseFarePath();
  bool isFarePathValidForPermutation(ProcessTagPermutation& permutation,
                                     RexAdvResTktValidator& rexAdvResTktValidator);

  inline bool permutationReissueChargesExists(ProcessTagPermutation* pt)
  {
    return _permutationReissueCharges.count(pt) > 0 && _permutationReissueCharges[pt];
  }

  bool checkTags(ProcessTagPermutation& permutation, std::string& errorMsg);
  bool checkOWRT(const std::vector<ProcessTagInfo*>& processTags, std::string& errorMsg);

  bool checkTerm(const ProcessTagPermutation& permutation);
  bool checkFirstBreak(const ProcessTagPermutation& permutation);

  bool checkFullyFlown(const ProcessTagPermutation& permutation);

  virtual const std::vector<const SamePoint*>&
  getSamePoint(const VendorCode& vendor, int itemNo, const DateTime& date);

  struct SameFareBreaks;

  Indicator validateFullyFlownForFC(ProcessTagInfo* pti, const FareMarket*& fareMarket);
  void diagFullyFlown(const ProcessTagInfo& pti,
                      const Indicator& matched,
                      const FareMarket* failedOnMarket,
                      int fcNumber);
  bool checkFullyFlownOnePoint(const ProcessTagInfo& pti, const FareMarket& fareMarket);
  bool checkFullyFlownDestOnly(const ProcessTagInfo& tag, const FareMarket& fareMarket);
  bool checkFullyFlownNoChange(const ProcessTagInfo& tag, const FareMarket& fareMarket);
  bool checkFareBreakLimitations(const ProcessTagPermutation& permutation, bool& permWithTag7);
  bool matchJourney(const ProcessTagPermutation& permutation);
  bool checkSamePointTable(const ProcessTagInfo& tag, bool runForFirstBreak = false);
  void analyseExcFarePath();

  bool checkReissueToLower(const ProcessTagPermutation& permutation);
  Indicator checkTable988Byte156(const ProcessTagPermutation& permutation);
  bool checkNewTicketEqualOrHigher(const ProcessTagPermutation& permutation);
  bool compareTotalFareCalculationAmounts();
  bool
  compareSumOfNonrefundableAmountOfExchangeTicketAndNewTicket(const ProcessTagPermutation& perm);
  void applyChangeFeeToFarePath();

  const MoneyAmount& lowestChangeFee() const { return _lowestChangeFee; }
  MoneyAmount& lowestChangeFee() { return _lowestChangeFee; }

  const CurrencyCode& calcCurrCode() const { return _calcCurrCode; }
  CurrencyCode& calcCurrCode() { return _calcCurrCode; }

  bool checkOverrideReservationDates(const ProcessTagPermutation& permutation);

  virtual const DateTime* latestBookingDate(const std::vector<TravelSeg*>& travelSegVec) const;
  virtual const DateTime* latestBookingDate(const std::vector<PricingUnit*>& pricingUnitVec) const;

  virtual bool
  validateLastBookingDate(const DateTime* latestBD, const VoluntaryChangesInfoW& vcRec3);
  bool fareComponentBookingDateValidation(std::vector<const PaxTypeFare*>& mappedFCvec,
                                          const VoluntaryChangesInfoW& vcRec3);
  bool journeyBookingDateValidation(const VoluntaryChangesInfoW& vcRec3);

  virtual const std::vector<DateOverrideRuleItem*>&
  getDateOverrideRuleItem(const VendorCode& vendor, int itemNo);

  bool revalidateRulesForKeepFares(const ProcessTagPermutation& permutation);
  bool validateBkgRevalInd(const ProcessTagPermutation& permutation);
  bool farePathHasKeepFares();
  bool validateFarePathBkgRevalInd(
      const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags);
  bool validateFareUsageBkgRevalInd(
      FareUsage& fu, const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags);

  bool validateCat35Security(const FareUsage& fu, const ProcessTagInfo* tag);

  void getKeepFareProcessTags(const ProcessTagPermutation& permutation,
                              std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags);
  bool revalidateRules(PricingUnit& pu,
                       const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags);
  bool
  anyFailAtZeroT988(const ProcessTagInfo* tag, const FareUsage& fu, const PricingUnit& pu) const;
  const PaxTypeFare* matchKeepFareInExcItin(const PaxTypeFare* keepFare);
  void getCategoryIgnored(const ProcessTagInfo& tag, std::set<uint16_t>& catgoryIgnored);
  virtual bool
  zeroT988ExcFare(const ProcessTagPermutation& permutation, const PaxTypeFare& ptf) const;
  bool determineNewKeepPTF(const PaxTypeFare& excFare,
                           std::vector<const PaxTypeFare*>::const_iterator& newPTFIter);

  bool checkTagsForMixedTags(ProcessTagPermutation& permutation, std::string& errorMsg);

  bool matchNewFcWithFa(std::vector<bool>& matchedNewFc, FareMarket::FareRetrievalFlags flag);
  FareMarket::FareRetrievalFlags decodeFareRetrievalFlag(FareApplication fa);
  virtual bool isRequiredRuleFailed(FareUsage& fu,
                                    const uint16_t& category,
                                    std::vector<uint16_t>& categoryNeedReval);
  virtual bool revalidateRuleInFareMarketScope(PaxTypeFare& keepFare,
                                               const std::vector<uint16_t>& fmCategoriesNeedReval);
  virtual bool revalidateRuleInPuScope(PricingUnit& pu,
                                       FareUsage& fu,
                                       std::vector<uint16_t>& categoriesNeedReval);

  bool isBaseFareAmountPlusChangeFeeHigher(const MoneyAmount& changeFee);

  bool isPUPartiallyFlown(const PricingUnit* pu) const;
  bool isSameCpaCxrFareInExc(const FareUsage* fu) const;
  void
  collectFlownOWFares(const std::vector<PricingUnit*>& puVec, std::vector<FareUsage*>& collectedFU);
  void analysePricingUnits();

  void saveStopByteInfo(const ProcessTagPermutation& permutation);
  bool skipByStopByte(const ProcessTagPermutation& permutation);
  uint32_t setLowestChangeFee(uint32_t validPermutations);
  bool savePermutationChangeFee(ProcessTagPermutation& permutation, bool permWithTag7);
  ReissueCharges* calculateReissueCharges(ProcessTagPermutation& permutation);

  bool isTktResvBytesBlank(const ReissueSequenceW& tbl988);
  bool hasDiagAndFilterPassed() const;
  void printStopSequences(const ProcessTagPermutation& permutation);
  bool matchOutboundPortionOfTravel(const ProcessTagPermutation& permutation) const;

  bool matchExpndKeepSeasonalityDOW(const ProcessTagPermutation& permutation);

  bool& isRebookSolution() { return _isRebookSolution; }
  bool isRebookSolution() const { return _isRebookSolution; }

  void setFareMarketChangeStatus(ExcItin* itinFirst, Itin* itinSecond);
  FCChangeStatus getChangeStatus(const std::vector<TravelSeg*>& tvlSegs,
                                 const int16_t& pointOfChgFirst,
                                 const int16_t& pointOfChgSecond);

  MoneyAmount getTotalChangeFee(ProcessTagPermutation& permutation);
  void analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(FareUsage& fu,
                                                      std::vector<uint16_t>& categoriesNeedReval);
  void getMinFarePlusUps();
  const MoneyAmount getNewTicketAmount();
  const MoneyAmount getExchTotalAmount();
  const MoneyAmount getExchNUCAmount();
  const CurrencyCode getCurrency();
  const MoneyAmount getNonRefAmountInBaseCurrencyOfExchangeTicket();
  const MoneyAmount getNonRefAmountInBaseCurrencyOfNewTicket();
  const MoneyAmount convertToBaseCurrency(MoneyAmount& mA, const CurrencyCode& curr);

  struct IsUnflown;
  struct IsStopOver;

  const PricingUnit* determineExcPrU(const ProcessTagInfo& pti) const;
  bool matchOutboundPortionOfTvl(const ProcessTagInfo& pti,
                                 uint16_t fcNumber,
                                 bool diagAndIfFilteredFilterPassed,
                                 DiagCollector* _dc) const;

  FareApplication
  getFareApplication(const ProcessTagPermutation& permutation, const PaxTypeFare& excFare);

  void checkTimeout(const uint32_t& permNumber);
  void checkMemory(uint32_t permNumber);
  void checkDiag();

  static const uint32_t DIAG_OUTPUT_LIMIT = 1024000;

private:
  typedef std::pair<int, int> FareBreakMapKey;
  typedef std::map<FareBreakMapKey, bool> FareBreakMap;

  bool isTravelSegmentStatusChanged(const std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>& ts2ss,
                                    TravelSeg& ts) const;

  RexPricingTrx& _trx;
  FarePath& _farePath;
  Diag689Collector* _dc;
  std::vector<const PaxTypeFare*> _allRepricePTFs;
  std::vector<std::vector<const PaxTypeFare*> > _fcMapping;
  std::vector<std::vector<const FareMarket*> > _fmMapping;
  const DateTime* _journeyLatestBD;

  FareBreakMap _fbMappingTB;
  FareBreakMap _fbMappingFB;

  std::map<std::pair<int, int>, bool> _record3Cache;

  const PaxTypeFare* _firstFare;
  FareMarket::FareRetrievalFlags _retrievalFlag;
  bool _journeyCheckingFlag;
  bool _travelCommenced;
  MoneyAmount _lowestChangeFee;
  MoneyAmount _lowestChangeFeeInQueue;
  ProcessTagPermutation* _permWithLowestChangeFee;

  RepriceSolutionValidator(const RepriceSolutionValidator&);
  RepriceSolutionValidator& operator=(const RepriceSolutionValidator&);

  CurrencyCode _calcCurrCode;

  SequenceStopByteByTag* _sequenceStopByteByTag;
  std::map<ProcessTagPermutation*, ReissueCharges*> _permutationReissueCharges;

  NewTicketEqualOrHigherCheckResult _byte156ValueB;
  NewTicketEqualOrHigherCheckResult _byte156ValueN;

  bool _isRebookSolution;
  std::map<TravelSeg*, PaxTypeFare::SegmentStatus*> _ts2ss;
  std::map<FareMarket*, FareMarket*> _FM2PrevReissueFMCache;
  AdvResOverrideCache _advResOverrideCache;
  MixedTagsCache _mixedTagsCache;
  uint16_t _itinIndex;
  uint32_t _maxDiagSize;

  GenericRexMapper _genericRexMapper;
  FarePathChangeDetermination* _farePathChangeDetermination;
  ExpndKeepFareValidator _expndKeepValidator;
  RexFareBytesValidator _fareBytesValidator;

  NewTicketEqualOrHigherValidator _newTicketEqualOrHigherValidator;
  StopoverConnectionValidator _stopoverConnectionValidator;
  unsigned _solutionCountLimit;
  uint32_t _permCheckInterval;
  uint32_t _memCheckTrxInterval;
};

} // tse

