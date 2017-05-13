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

#include "Common/SmallBitSet.h"
#include "DataModel/Agent.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueOptionsMap.h"
#include "DataModel/RexBaseTrx.h"
#include "Rules/DepartureDateValidator.h"
#include "Service/Service.h"

#include <boost/logic/tribool.hpp>
#include <boost/noncopyable.hpp>

#include <memory>

namespace tse
{
class CsoPricingTrx;
class FarePath;
class Logger;
class PenaltyFee;
class SeasonalityDOW;

struct FareBytesData
{
  FareBytesData() = default;
  FareBytesData(ProcessTagInfo* pti);

  std::set<ProcessTagInfo*>& processTags() { return _processTags; }
  void add(ProcessTagInfo* pti);

  bool fareRulesApply() { return _fareRulesApply; }
  bool fareTrfNumberApply() { return _fareTrfNumberApply; }
  bool fareClassCodeApply() { return _fareClassCodeApply; }
  bool fareNormalSpecialApply() { return _fareNormalSpecialApply; }
  bool excludePrivateApply() { return _excludePrivateApply; }
  bool owrtApply() { return _owrtApply; }
  bool fareAmountApply() { return _fareAmountApply && _processTagsForAmount; }
  bool sameApply() { return _sameApply; }

private:
  static constexpr Indicator NOT_APPLICABLE = ' ';

  bool _fareRulesApply = true;
  bool _fareTrfNumberApply = true;
  bool _fareClassCodeApply = true;
  bool _fareNormalSpecialApply = true;
  bool _excludePrivateApply = true;
  bool _owrtApply = true;
  bool _fareAmountApply = true;
  bool _sameApply = true;

  std::set<ProcessTagInfo*> _processTags;
  bool _processTagsForAmount = false;
};

class ROEDateSetter
{
public:
  ROEDateSetter(PricingTrx& pricingTrx, FarePath& fp);
  ~ROEDateSetter();

  void reCalculateNucAmounts();

private:
  bool useHistoricalRoeDate() const;

  PricingTrx& _pricingTrx;
  FarePath& _fp;
  bool _recalculate = false;
  friend class RexPricingTrxTest;
};

class RaiiRexFarePathPlusUps : boost::noncopyable
{
public:
  RaiiRexFarePathPlusUps(PricingTrx& trx, FarePath& fp, bool& valid);
  bool plusUpsNeeded() const;
  ~RaiiRexFarePathPlusUps();

private:
  void determineNonRefundable(const FarePath& fp);

  RexBaseTrx* _rexBaseTrx;
  boost::tribool _nonRefundable{boost::indeterminate};
  const bool& _valid;
};

class RexPricingTrx : public RexBaseTrx
{
  friend class RexPricingTrxTest;

public:
  class DoubleQualifierWrapper
  {
    PricingOptions& _po;
    char _publishedFares = _po.publishedFares();
    char _privateFares = _po.privateFares();
    char _normalFare = _po.normalFare();
    char _xoFares = _po.xoFares();

  public:
    DoubleQualifierWrapper(PricingOptions& po) : _po(po)
    {
      _po.publishedFares() = 0;
      _po.privateFares() = 0;
      _po.normalFare() = 0;
      _po.xoFares() = 0;
    }

    ~DoubleQualifierWrapper() { restore(); }

    void restore()
    {
      _po.publishedFares() = _publishedFares;
      _po.privateFares() = _privateFares;
      _po.normalFare() = _normalFare;
      _po.xoFares() = _xoFares;
    }
  };

  using PenaltyFeeMap =
      std::map<std::pair<const PaxTypeFare*, const VoluntaryChangesInfo*>, const PenaltyFee*>;
  using NewItinKeepFareMap = std::map<const PaxTypeFare*, FareMarket*>;
  using NewToExcItinFareMarketMapForKeep = std::map<FareMarket*, const FareMarket*>;
  using WaivedChangeRecord3Set = std::set<const VoluntaryChangesInfo*>;

  using BoardOffToFareBytesData = std::map<std::pair<LocCode, LocCode>, FareBytesData>;

  using ExpndKeepMap = std::multimap<const FareMarket*, const PaxTypeFare*>;
  using ExpndKeepMapI = std::multimap<const FareMarket*, const PaxTypeFare*>::const_iterator;

  RexPricingTrx();
  ~RexPricingTrx() = default;

  enum Tag1Status
  {
    NONE = 0,
    TAG1PERMUTATION,
    TAG7PERMUTATION,
  };

  const DateTime& originalTktIssueDT() const override;
  void prepareRequest() override;
  void setUpSkipSecurityForExcItin() override;
  virtual const DateTime& getRuleApplicationDate(const CarrierCode& govCarrier) const override;
  bool repriceWithSameFareDate() override;
  virtual void set(const RexBaseTrx::RequestTypes& reqTypes) override;

  virtual const DateTime& travelDate() const override;
  const DateTime& originalTravelDate() const { return _travelDate; }
  virtual const DateTime& adjustedTravelDate(const DateTime& travelDate) const override;
  const DateTime& adjustedTravelDate() const { return _adjustedTravelDate; }
  void setAdjustedTravelDate() override;

  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  std::string& reissuePricingErrorMsg() { return _reissuePricingErrorMsg; }
  const std::string& reissuePricingErrorMsg() const { return _reissuePricingErrorMsg; }

  std::string& csoPricingErrorMsg() { return _csoPricingErrorMsg; }
  const std::string& csoPricingErrorMsg() const { return _csoPricingErrorMsg; }

  ErrorResponseException::ErrorResponseCode& reissuePricingErrorCode()
  {
    return _reissuePricingErrorCode;
  }

  const ErrorResponseException::ErrorResponseCode& reissuePricingErrorCode() const
  {
    return _reissuePricingErrorCode;
  }

  ErrorResponseException::ErrorResponseCode& csoPricingErrorCode() { return _csoPricingErrorCode; }
  const ErrorResponseException::ErrorResponseCode& csoPricingErrorCode() const
  {
    return _csoPricingErrorCode;
  }
  
  virtual ReissueOptions& reissueOptions() { return _reissueOptions; }
  virtual const ReissueOptions& reissueOptions() const { return _reissueOptions; }

  virtual std::vector<ProcessTagPermutation*>& processTagPermutations()
  {
    return _tag1PricingSvcCallStatus ? _separatedPermutations : _processTagPermutations;
  }

  virtual const std::vector<ProcessTagPermutation*>& processTagPermutations() const
  {
    return _tag1PricingSvcCallStatus ? _separatedPermutations : _processTagPermutations;
  }

  virtual std::vector<ProcessTagPermutation*>& processTagPermutations(uint16_t itinIndex)
  {
    return processTagPermutations();
  }

  virtual const std::vector<ProcessTagPermutation*>&
  processTagPermutations(uint16_t itinIndex) const
  {
    return processTagPermutations();
  }

  virtual Tag1Status tag1PricingSvcCallStatus() const { return _tag1PricingSvcCallStatus; }
  void insert(ProcessTagPermutation& perm);
  bool shouldCallPricingSvcSecondTime();
  void prepareForSecondPricingSvcCall();
  void validSolutionsPlacement(std::pair<FarePath*, FarePath*> rebookedBooked);

  virtual FarePath*& lowestRebookedFarePath() { return _lowestRebookedFarePath; }
  virtual const FarePath* lowestRebookedFarePath() const { return _lowestRebookedFarePath; }

  virtual FarePath*& lowestBookedFarePath() { return _lowestBookedFarePath; }
  virtual const FarePath* lowestBookedFarePath() const { return _lowestBookedFarePath; }

  void calculateNonrefundableAmountForValidFarePaths();
  bool isNeedDetermineBaseFareForExcFarePath(uint16_t itinIndex = 0);

  FarePath*& lowestCSOFarePath() { return _lowestCSOFarePath; }
  const FarePath* lowestCSOFarePath() const { return _lowestCSOFarePath; }

  CsoPricingTrx*& pricingTrxForCSO() { return _pricingTrxForCSO; }
  const CsoPricingTrx* pricingTrxForCSO() const { return _pricingTrxForCSO; }

  const bool isRebookedSolutionValid() const { return _lowestRebookedFarePath != nullptr; }
  const bool isBookedSolutionValid() const { return _lowestBookedFarePath != nullptr; }
  const bool isCSOSolutionValid() const { return _lowestCSOFarePath != nullptr; }

  void setTktValidityDate() override;

  PenaltyFeeMap& penaltyFees() { return _penaltyFees; }
  const PenaltyFeeMap& penaltyFees() const { return _penaltyFees; }

  const PenaltyFee*
  getPenaltyFee(const PaxTypeFare* ptf, const VoluntaryChangesInfo* voluntaryChangeInfo) const;

  WaivedChangeRecord3Set& waivedChangeFeeRecord3() { return _waivedChangeRecord3; }
  const WaivedChangeRecord3Set& waivedChangeFeeRecord3() const { return _waivedChangeRecord3; }

  virtual NewItinKeepFareMap& newItinKeepFares() { return _newItinKeepFares; }
  virtual const NewItinKeepFareMap& newItinKeepFares() const { return _newItinKeepFares; }

  virtual NewItinKeepFareMap& newItinKeepFares(uint16_t itinIndex) { return _newItinKeepFares; }
  virtual const NewItinKeepFareMap& newItinKeepFares(uint16_t itinIndex) const
  {
    return _newItinKeepFares;
  }

  virtual NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep()
  {
    return _newToExcItinFareMarketMapForKeep;
  }

  virtual const NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep() const
  {
    return _newToExcItinFareMarketMapForKeep;
  }

  virtual ExpndKeepMap& expndKeepMap() { return _expndKeepMap; }
  virtual const ExpndKeepMap& expndKeepMap() const { return _expndKeepMap; }

  std::map<const FareMarket*, FareMarket::FareRetrievalFlags>& excFlownFcFareAppl()
  {
    return _excFlownFcFareAppl;
  }

  const std::map<const FareMarket*, FareMarket::FareRetrievalFlags>& excFlownFcFareAppl() const
  {
    return _excFlownFcFareAppl;
  }

  void filterFareMarketByFlownFareAppl();

  bool& csoTransSuccessful() { return _csoTransSuccessful; }
  bool csoTransSuccessful() const { return _csoTransSuccessful; }

  virtual bool& repriceWithDiffDates() { return _repriceWithDiffDates; }
  virtual bool repriceWithDiffDates() const { return _repriceWithDiffDates; }

  virtual bool matchFareRetrievalDate(const FareMarket& fm) override;
  virtual bool matchFareRetrievalDate(const PaxTypeFare& paxTypeFare) override;

  virtual void createPricingTrxForCSO(bool forDiagnostic);

  static bool isRexTrxAndNewItin(const PricingTrx& trx)
  {
    return trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
           (static_cast<const RexPricingTrx*>(&trx))->trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE;
  }

  void incrementRSVCounter(bool pass)
  {
    ++_rsvCounter.total;
    if (pass)
      ++_rsvCounter.pass;
  }

  bool rsvCounterStatus() const
  {
    if (_rsvCounter.total == 0)
      return false;
    return _rsvCounter.pass == 0;
  }

  unsigned nbrOfSolutionsBuilt() const { return _rsvCounter.total; }

  BoardOffToFareBytesData& fareBytesData() { return _fareBytesData; }
  const BoardOffToFareBytesData& fareBytesData() const { return _fareBytesData; }

  std::multimap<CarrierCode, FareBytesData>& unmappedFareBytesData()
  {
    return _unmappedFareByteData;
  }

  void logFaresInfo() const;
  void logLowestResultInfo() const;

  bool isSubscriberTrx() const { return !_request->ticketingAgent()->tvlAgencyPCC().empty(); }

  bool isSecondaryReqExist() const { return !_secondaryExcReqType.empty(); }

  bool fareFailFareBytes() const { return _fareFailFareBytes; }
  bool& fareFailFareBytes() { return _fareFailFareBytes; }

  void setAnalyzingExcItin(const bool isAnalyzingExcItin) override;

  std::vector<std::map<uint32_t, uint32_t> >& excSchedulingOptionIndices()
  {
    return _excSchedulingOptionIndices;
  }

  const std::vector<std::map<uint32_t, uint32_t> >& excSchedulingOptionIndices() const
  {
    return _excSchedulingOptionIndices;
  }

  std::vector<std::map<uint32_t, uint32_t> >& excIndicesToSchedulingOption()
  {
    return _excIndicesToSchedulingOption;
  }

  const std::vector<std::map<uint32_t, uint32_t> >& excIndicesToSchedulingOption() const
  {
    return _excIndicesToSchedulingOption;
  }

  std::vector<TravelSeg*>& excTravelSeg() { return _excTravelSeg; }
  const std::vector<TravelSeg*>& excTravelSeg() const { return _excTravelSeg; }

  std::vector<FareComponentInfo*>& excFareCompInfo() { return _excFareCompInfo; }
  const std::vector<FareComponentInfo*>& excFareCompInfo() const { return _excFareCompInfo; }

  virtual void
  getRec2Cat10WithVariousRetrieveDates(std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                       GetRec2Cat10Function getRec2Cat10) override;

  virtual void
  initalizeForRedirect(std::vector<FareMarket*>& fm, std::vector<TravelSeg*>& ts) const override;

  const SeasonalityDOW* getSeasonalityDOW(const VendorCode& vendor,
                                          uint32_t tblItemNo,
                                          const DateTime& applicationDate) const;

  static bool expndKeepSameNation(const Loc& loc1, const Loc& loc2);

  virtual bool& allPermutationsRequireCurrentForFlown()
  {
    return _allPermutationsRequireCurrentForFlown;
  }

  virtual bool& allPermutationsRequireNotCurrentForFlown()
  {
    return _allPermutationsRequireNotCurrentForFlown;
  }

  virtual bool& allPermutationsRequireCurrentForUnflown()
  {
    return _allPermutationsRequireCurrentForUnflown;
  }

  virtual bool& allPermutationsRequireNotCurrentForUnflown()
  {
    return _allPermutationsRequireNotCurrentForUnflown;
  }

  void setExcTktNonRefundable(boost::tribool status);
  boost::tribool excTktNonRefundable() const { return _excTktNonRefundable; }

  virtual DepartureDateValidator& departureDateValidator() { return _departureDateValidator; }
  virtual const DepartureDateValidator& departureDateValidator() const
  {
    return _departureDateValidator;
  }

  std::pair<size_t, size_t> permutationCount()
  {
    return std::make_pair(_processTagPermutations.size(), _separatedPermutations.size());
  }
  const PaxTypeFare* getKeepPtf(const FareMarket& fareMarket) const;
  bool isFareMarketNeededForMinFares(const FareMarket& fm) const;

  std::pair<const DateTime*, const Loc*> getLocAndDateForAdjustment() const;
  const DateTime adjustToCurrentUtcZone() const;
  virtual bool isPlusUpCalculationNeeded() const override;

protected:
  virtual Tag1Status& setTag1PricingSvcCallStatus() { return _tag1PricingSvcCallStatus; }

  std::string _reissuePricingErrorMsg;
  std::string _csoPricingErrorMsg;

  ErrorResponseException::ErrorResponseCode _reissuePricingErrorCode =
      ErrorResponseException::NO_ERROR;
  ErrorResponseException::ErrorResponseCode _csoPricingErrorCode = ErrorResponseException::NO_ERROR;

  ReissueOptions _reissueOptions;
  
  std::vector<ProcessTagPermutation*> _processTagPermutations;

  std::vector<ProcessTagPermutation*> _separatedPermutations;
  Tag1Status _tag1PricingSvcCallStatus = Tag1Status::NONE;

  FarePath* _lowestRebookedFarePath = nullptr;
  FarePath* _lowestBookedFarePath = nullptr;
  FarePath* _lowestCSOFarePath = nullptr;
  CsoPricingTrx* _pricingTrxForCSO = nullptr;

  PenaltyFeeMap _penaltyFees;
  WaivedChangeRecord3Set _waivedChangeRecord3;
  bool _csoTransSuccessful = false;
  bool _repriceWithDiffDates = false;
  NewItinKeepFareMap _newItinKeepFares;
  NewToExcItinFareMarketMapForKeep _newToExcItinFareMarketMapForKeep;
  ExpndKeepMap _expndKeepMap;
  std::map<const FareMarket*, FareMarket::FareRetrievalFlags> _excFlownFcFareAppl;

  BoardOffToFareBytesData _fareBytesData;
  std::multimap<CarrierCode, FareBytesData> _unmappedFareByteData;
  bool _fareFailFareBytes = false;

  struct RSVCounter
  {
    unsigned total = 0;
    unsigned pass = 0;
  } _rsvCounter;

  void buildFareInfoMessage(const std::set<FareMarket::FareRetrievalFlags>& fareTypes,
                            std::string& result) const;
  void getFareTypes(FarePath& farePath, std::set<FareMarket::FareRetrievalFlags>& fareTypes) const;
  void logLowestResultInfo(FarePath& farePath, const std::string& solutType) const;
  void
  getFlownSegRetrievalMap(std::map<int16_t, FareMarket::FareRetrievalFlags>& flownSegRetrievalMap);
  DateTime _adjustedTravelDate;

  std::vector<std::map<uint32_t, uint32_t> > _excSchedulingOptionIndices;
  std::vector<std::map<uint32_t, uint32_t> > _excIndicesToSchedulingOption;
  std::vector<TravelSeg*> _excTravelSeg;

  DepartureDateValidator _departureDateValidator{*this};

private:
  bool separateTag7Permutations();
  void disableRebookedIfGreaterEqualThanBooked();
  void switchIfCheaperOrEqual(FarePath* candidate, FarePath*& current);

  static Logger _logger;
  static Logger _faresInfoLogger;
  bool _allPermutationsRequireCurrentForFlown = true;
  bool _allPermutationsRequireNotCurrentForFlown = true;
  bool _allPermutationsRequireCurrentForUnflown = true;
  bool _allPermutationsRequireNotCurrentForUnflown = true;
  boost::tribool _excTktNonRefundable{boost::indeterminate};
};
} // tse namespace
