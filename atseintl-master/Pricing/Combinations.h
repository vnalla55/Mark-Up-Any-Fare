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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/EndOnEnd.h"
#include "Pricing/CombinabilityScoreboard.h"

namespace tse
{
class FarePath;
class PricingTrx;
class FareUsage;

class Combinations
{
  friend class CombinationsTest;
  friend class PreserveRestorePURuleBasedFares;
  friend class CombinationsSubCat109Test;
  friend class CombinationsCheckOJMileageTest;

public:
  //-------------------------------
  //--- Combinations definition ---
  //-------------------------------
  static constexpr char PASSCOMB = 'P';
  static constexpr char FAILCOMB = 'F';
  static constexpr char MATCH = 'M';
  static constexpr char NO_MATCH = 'N';
  static constexpr char ABORT = 'A';
  static constexpr char IDLE = ' ';
  static constexpr char STOPCOMB = 'S';
  static constexpr char MAJOR_NO_MATCH = 'D';

  //--------- data string --------------------
  static const uint16_t OPEN_JAW = 101;
  static const uint16_t ROUND_TRIP = 102;
  static const uint16_t CIRCLE_TRIP = 103;
  static const uint16_t END_ON_END = 104;
  static const uint16_t ADD_ON = 105;
  static const uint16_t CARRIER = 106;
  static const uint16_t RULE_TARIFF = 107;
  static const uint16_t FARE_CLASS = 108;
  static const uint16_t OPEN_JAW_SET = 109;

  static const uint16_t ELIGIBILITY_RULE = 1;
  static const uint16_t FLIGHT_APPL_RULE = 4;
  static const uint16_t MINIMUM_STAY_RULE = 6;
  static const uint16_t MAXIMUM_STAY_RULE = 7;
  static const uint16_t TRAVEL_RESTRICTION_RULE = 14;
  static const uint16_t SALES_RESTRICTIONS_RULE = 15;

  //----------ATPCo Record 3 data ------------
  static constexpr char SINGLE_OPEN_JAW = 'S';
  static constexpr char DOUBLE_OPEN_JAW = 'D';
  static constexpr char MAX_TWO_INTL = 'I';
  static constexpr char UNLIMITED = ' ';
  static constexpr char ANY_SOJ = 'A';
  static constexpr char MILEAGE_SHORTEST = 'M';
  static constexpr char MILEAGE_LONGEST = 'K';
  static constexpr char MILEAGE_SHORTEST_INTL_ONLY = 'N';
  static constexpr char MILEAGE_LONGEST_INTL_ONLY = 'Y';
  static constexpr char MUST_BE_CHARGE = 'X';
  static constexpr char APPLICATION_NOT_PERMITTED = 'X';
  static constexpr char RESTRICTION_APPLIES = 'X';
  static constexpr char ALL_SEGMENTS = ' ';
  static constexpr char ADJACENT = 'X';
  static constexpr char COMMON_POINT = 'Y';
  static constexpr char MUST_NOT_BE_A_B_A = 'N';
  static constexpr char MUST_BE_A_B_A = 'Y';
  static constexpr char IF_EOE_MUST_BE_A_B_A = 'P';
  static constexpr char NO_APPLICATION = ' ';
  static constexpr char MAY_NOT_COMBINE_OW_RT = '1';
  static constexpr char APPLY_ONLY_SPECIAL_FARE = ' ';
  static constexpr char APPLY_TO_ENTIRE_TRIP = 'X';
  static constexpr char IGNORE_SPECIAL_FARE = 'N';
  static constexpr char MUST_BE = 'Y';
  static constexpr char NEED_NOT_BE = 'N';
  static constexpr char NEED_NOT_BE_X = 'X';
  static constexpr char MUST_NOT_BE = 'M';
  static constexpr char NORMAL = 'N';
  static constexpr char SPECIAL = 'S';
  static constexpr char US_CAN = 'U';
  static constexpr char EOE_DOMESTIC = 'D';
  static constexpr char EOE_INTERNATIONAL = 'I';
  static constexpr char NOT_ALLOWED = 'N';
  static const std::string NO_RESTRICTION_S;
  static const std::string NO_STOP_PERMITTED;
  static const CarrierCode MATCH_ANY_CARRIER;
  static const TariffNumber ANY_TARIFF = 999;
  static const TariffNumber NO_TARIFF_APPLICATION = 0;

  //----------------------------------------------
  //--- ATPCO Record 2 Combinations definition ---
  //----------------------------------------------
  static constexpr char ONE_WAY_FARE = '1';
  static constexpr char ROUND_TRIP_FARE = '2';
  static constexpr char NOT_PERMITTED = 'N';
  static constexpr char PERMITTED = 'Y';
  static constexpr char PERMITTED_S = 'S';
  static constexpr char RESTRICTIONS = 'R';
  static constexpr char RESTRICTIONS_T = 'T';
  static constexpr char REQUIRED = 'M';
  static constexpr char NOT_APPLICABLE = 'X';
  static constexpr char UNAVAILABLE_DATA = 'X';
  static constexpr char TEXT_DATA_ONLY = 'Y';
  static constexpr char ALWAYS_APPLIES = ' ';
  static constexpr char NO_RESTRICTION = ' ';
  static constexpr char FROM_LOC1_TO_LOC2 = '1';
  static constexpr char TO_LOC1_FROM_LOC2 = '2';
  static constexpr char ORIGIN_FROM_LOC1_TO_LOC2 = '3';
  static constexpr char ORIGIN_FROM_LOC2_TO_LOC1 = '4';
  static constexpr char DEST_OPEN_JAW_REQ = 'D';
  static constexpr char ORIGIN_OPEN_JAW_REQ = 'O';
  static constexpr char SAME_CARRIER = 'S';
  static constexpr char SAME_RULE = 'R';
  static constexpr char SAME_TARIFF = 'T';
  static constexpr char ANY_PUBLIC_TARIFF = 'Y';
  static constexpr char ANY_PRIVATE_TARIFF = 'P';
  static constexpr char SAME_FARECLASS = 'C';
  static constexpr char SAME_FARETYPE = 'T';
  static constexpr char ROUND_TRIP_PERMITTED_MIRROR_IMAGE_PERMITTED = 'Y';
  static constexpr char ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_PERMITTED = 'N';
  static constexpr char ROUND_TRIP_RESTRICTIONS_MIRROR_IMAGE_PERMITTED = 'R';
  static constexpr char ROUND_TRIP_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED = 'V';
  static constexpr char ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED = 'X';
  static constexpr char ROUND_TRIP_RESTRICTIONS_MIRROR_IMAGE_NOT_PERMITTED = 'W';
  static constexpr char EOE_PERMITTED_SIDE_TRIP_NOT_PERMITTED = 'A';
  static constexpr char EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED = 'B';
  static constexpr char EOE_RESTRICTIONS_SIDE_TRIP_NOT_PERMITTED = 'C';
  static constexpr char EOE_REQUIRED_SIDE_TRIP_NOT_PERMITTED = 'D';
  static constexpr char OPEN_JAW_PERMITTED_DIFFERENT_COUNTRY = 'U';
  static constexpr char OPEN_JAW_RESTRICTED_DIFFERENT_COUNTRY = 'V';
  static constexpr char OPEN_JAW_PERMITTED_ORIGIN_SAME_COUNTRY = 'W';
  static constexpr char OPEN_JAW_RESTRICTED_ORIGIN_SAME_COUNTRY = 'X';
  static const CarrierCode JOINTCARRIER;


  enum MatchNumbers
  {
    m106,
    m107,
    m108,
    m109,
    m001,
    m004,
    m006,
    m007,
    m014,
    m015,

    M101,
    M102,
    M103,
    M104,

    subCategoriesNum
  };

  Combinations() = default;
  Combinations(const Combinations&) = delete;
  Combinations& operator=(const Combinations&) = delete;

  virtual ~Combinations() = default;

  virtual CombinabilityValidationResult process(PricingUnit& pu,
                                                FareUsage*& failedSourceFareUsage,
                                                FareUsage*& failedTargetFareUsage,
                                                DiagCollector& diag,
                                                Itin* itin);
  virtual CombinabilityValidationResult process(FarePath& farePath,
                                                uint32_t farePathIndex, // for diagnostics display
                                                FareUsage*& failedSourceFareUsage,
                                                FareUsage*& failedEOETargetFareUsage,
                                                DiagCollector& diag);

  PricingTrx*& trx() { return _trx; }
  const PricingTrx* trx() const { return _trx; }

  CombinabilityScoreboard*& comboScoreboard() { return _comboScoreboard; }
  const CombinabilityScoreboard* comboScoreboard() const { return _comboScoreboard; }

  class EndOnEndElements
  {
  public:
    PricingUnit* currentPU = nullptr;
    FareUsage* currentFU = nullptr;
    PricingUnit* targetPU = nullptr;
    FareUsage* targetFU = nullptr;
  };

  using EndOnEndItem = std::vector<Combinations::EndOnEndElements>;
  using EndOnEndList = std::vector<Combinations::EndOnEndItem*>;

  static void enablePULevelFailedFareUsageOptimization(bool enableIt)
  {
    _enablePULevelFailedFareUsageOptimization = enableIt;
  }
  static void enableFPLevelFailedFareUsageOptimization(bool enableIt)
  {
    _enableFPLevelFailedFareUsageOptimization = enableIt;
  }

  static Combinations* getNewInstance(PricingTrx* pricingTrx)
  {
    Combinations* combinations (nullptr);

    CombinabilityScoreboard* comboScoreboard (nullptr);
    pricingTrx->dataHandle().get(comboScoreboard);
    comboScoreboard->trx() = pricingTrx;

    pricingTrx->dataHandle().get(combinations);
    combinations->trx() = pricingTrx;
    combinations->comboScoreboard() = comboScoreboard;
    return combinations;
  }

private:
  PricingTrx* _trx = nullptr;
  CombinabilityScoreboard* _comboScoreboard = nullptr;

  std::pair<bool, CombinabilityRuleItemInfo>
  findCategoryRuleItem(const std::vector<CombinabilityRuleItemInfoSet*>& catRuleInfoSetVec);

  struct ValidationElement
  {
    FareUsage* _currentFareUsage = nullptr;
    FareUsage* _targetFareUsage = nullptr;
    const CombinabilityRuleInfo* _pCat10 = nullptr; // of _currentFareUsage

    bool _passMinor = false;
    bool _passMajor = false;

    char _match[subCategoriesNum];

    static constexpr char NOT_SET = '-';
    static constexpr char MATCHED = 'M';
    static constexpr char NOT_MATCHED = 'N';

    void initialize(FareUsage* currentFareUsage,
                    const CombinabilityRuleInfo* pCat10,
                    FareUsage* targetFareUsage)
    {
      _passMinor = false;
      _passMajor = false;

      _currentFareUsage = currentFareUsage;
      _targetFareUsage = targetFareUsage;
      _pCat10 = pCat10;

      reset();
    }

    void reset()
    {
      resetAll();
      _passMajor = false;
    }

    char& getSubCat(const MatchNumbers num)
    {
      return _match[(size_t) num];
    }

    const char& getSubCat(const MatchNumbers num) const
    {
      return _match[(size_t) num];
    }

    void resetAll()
    {
      for (size_t i(m106); i < subCategoriesNum; ++i)
        _match[i] = NOT_SET;

      _passMinor = false;
    }

    void setPassMinor()
    {
      _passMinor = true;

      for (size_t i (m106); i<=m015; ++i)
        _passMinor &= (_match[i] != NOT_MATCHED);

    }

    void setPassMajor()
    {
      _passMajor = true;

      for (size_t i(M101); i<=M104; ++i)
        _passMajor &= (_match[i] != FAILCOMB &&
                       _match[i] != ABORT &&
                       _match[i] != MAJOR_NO_MATCH);
    }
  };
  using ValidationBaseVec = std::vector<Combinations::ValidationElement>;

public:
  class ValidationFareComponents : public ValidationBaseVec
  {
  public:
    void reset();
    void resetMinor();
    void resetMajor();
    bool evaluateMajor(FareUsage*& failedSourceFareUsage, FareUsage*& failedTargetFareUsage);
    bool evaluateMinor();
    bool evaluateMinorNegAppl(FareUsage*& failedFareUsage, FareUsage*& failedEOETargetFareUsage);
    bool evaluate(const MatchNumbers num);

    void getFailedFUInMinor(FareUsage*& failedSourceFareUsage, FareUsage*& failedTargetFareUsage);

    bool hasOneCarrier() const { return _hasOneCarrier; }
    void setHasOneCarrier(const bool flag) { _hasOneCarrier = flag; }
    bool hasOneVendor() const { return _hasOneVendor; }
    void setHasOneVendor(const bool flag) { _hasOneVendor = flag; }
    bool allPublicFares() const { return _allPublicFares; }
    void setallPublicFares(const bool flag) { _allPublicFares = flag; }
    void setForcePass(const bool flag) { _forcePass = flag; }
    bool& needAllPassSameMajorItem() { return _needAllPassSameMajorItem; }
    bool needAllPassSameMajorItem() const { return _needAllPassSameMajorItem; }
    bool& anyPassMinor() { return _anyPassMinor; }
    bool anyPassMinor() const { return _anyPassMinor; }
    void setMinorPass();
    bool evaluateMajorByPassedMinor();
    bool evaluateMajorByPassedMinor_old();

    CarrierCode& validatingCarrier() { return _validatingCarrier; }
    const CarrierCode& validatingCarrier() const { return _validatingCarrier; }


    bool validateCarrierPreference(DataHandle& dataHandle,
                                   const FareUsage* sourceFareUsage,
                                   const PaxTypeFare* targetFare);

    void resetCarrierPref()
    {
      _hasOneCarrier = true;
      _hasOneVendor = true;
      _allPublicFares = true;
      _combPref = nullptr;
    }

    void getNotPassedFC(FareUsage*& failedFareUsage, FareUsage*& failedEOETargetFareUsage);
    void diagNotPassedFC(DiagCollector& diag);

  private:
    bool _forcePass = false;
    bool _hasOneCarrier = true;
    bool _hasOneVendor = true;
    bool _allPublicFares = false;
    const std::vector<CarrierPreference::CombPref>* _combPref = nullptr;
    bool _needAllPassSameMajorItem = true;
    bool _anyPassMinor = false;
    CarrierCode _validatingCarrier;
  };

private:
  enum ValidationLevel
  {
    PricingUnitLevel,
    FarePathLevel
  };

protected:
  struct DirectionalityInfo
  {
    DirectionalityInfo(ValidationLevel level,
                       const char* levelName,
                       const char loc1ToLoc2,
                       const char loc2ToLoc1)
      : validationLevel(level),
        levelText(levelName),
        fromLoc1ToLoc2(loc1ToLoc2),
        toLoc1FromLoc2(loc2ToLoc1)
    {
    }

    ValidationLevel validationLevel;
    const char* levelText = nullptr;
    const char fromLoc1ToLoc2;
    const char toLoc1FromLoc2;
    FareMarket* fareMarket = nullptr;
    uint32_t farePathNumber = 0; // Used for diagnostics
    uint32_t pricingUnitNumber = 0; // Used for diagnostics
    uint32_t fareUsageNumber = 0; // Used for diagnostics
    uint32_t diagFarePathNumber = 0; // Used for diagnostics
    uint32_t diagPricingUnitNumber = 0; // Used for diagnostics
    uint32_t diagFareUsageNumber = 0; // Used for diagnostics
  };

  class FareTypeAvailabilty
  {
  protected:
    enum
    {
      FT_NORMAL = 0,
      FT_SPECIAL = 1,
      FT_DOMESTIC = 2,
      FT_INTERNATIONAL = 3,
      FT_TRANSBORDER = 4,
      FT_COUNT = 5
    };

  public:
    FareTypeAvailabilty();

    void reset();
    void setNormalFarePresent();
    void setSpecialFarePresent();
    void setDomesticFarePresent();
    void setInternationalFarePresent();
    void setTransborderFarePresent();
    bool isRequiredFareMissing(const EndOnEnd& endOnEndRule) const;

  protected:
    std::bitset<FT_COUNT> _fareTypePresent;
  };

  struct EndOnEndDataAccess
  {
    explicit EndOnEndDataAccess(const FarePath& farePath,
                                const PricingUnit& pu,
                                const CombinabilityRuleInfo& cat10Rule,
                                const uint32_t itemNo,
                                ValidationFareComponents& validationFareComponent,
                                const EndOnEnd& endOnEndRule,
                                DiagCollector& diag,
                                const Combinations::DirectionalityInfo& directionalityInfo)
      : _farePath(farePath),
        _pu(pu),
        _cat10Rule(cat10Rule),
        _itemNo(itemNo),
        _validationFareComponent(validationFareComponent),
        _endOnEndRule(endOnEndRule),
        _diag(diag),
        _directionalityInfo(directionalityInfo),
        _mustVerifyCxrPreference(!validationFareComponent.hasOneCarrier() ||
                                 !validationFareComponent.hasOneVendor() ||
                                 !validationFareComponent.allPublicFares())
    {
    }

    const FarePath& _farePath;
    const PricingUnit& _pu;
    const CombinabilityRuleInfo& _cat10Rule;
    const uint32_t _itemNo;
    ValidationFareComponents& _validationFareComponent;
    const EndOnEnd& _endOnEndRule;
    DiagCollector& _diag;
    const Combinations::DirectionalityInfo& _directionalityInfo;
    bool _diag614IsActive = false;
    bool _mustVerifyCxrPreference;
  };

public:
  virtual bool
  buildValidationFareComponent(DiagCollector& diag,
                               DirectionalityInfo& directionalityInfo,
                               const PricingUnit& curPu,
                               FareUsage& curFareUsage,
                               const CombinabilityRuleInfo* pCat10,
                               const FarePath& farePath,
                               Combinations::ValidationFareComponents& validationFareComponent);

  virtual char processEndOnEndRestriction(DiagCollector& diag,
                                          const FarePath& farePath,
                                          const PricingUnit& currentPU,
                                          const uint32_t itemNo,
                                          ValidationFareComponents& validationFareComponent,
                                          const DirectionalityInfo& directionalityInfo);
  static PaxTypeFare* validateFareCat10overrideCat25(const PaxTypeFare* fare1, PaxTypeFare* fare2,
                                                     const PricingUnit* pu);

  bool
  validate106Only(PricingUnit& pu, DiagCollector& diag);

private:
  enum EOEAllSegmentIndicator
  {
    AllSegment,
    CommonPoint,
    Adjacent
  };

  FareTypeAvailabilty _eoePresentFareTypes;

  EOEAllSegmentIndicator
  determineAllSegmentIndicatorValue(const CombinabilityRuleInfo* pCat10) const;
  const std::vector<PricingUnit*>*
  determineSideTripPUVector(const PricingUnit& curPu, const FarePath& farePath) const;

  bool adjacentLineOfFlight(const FareUsage& curFareUsage,
                            const FareUsage& targetFareUsage,
                            const Itin& itin);

  bool
  populateValidationFareComponent(DiagCollector& diag,
                                  const PricingUnit& curPu,
                                  FareUsage& curFareUsage,
                                  const CombinabilityRuleInfo* _pCat10,
                                  Combinations::ValidationFareComponents& validationFareComponent);

  bool
  populateValidationFareComponent(DiagCollector& diag,
                                  const PricingUnit& curPu,
                                  FareUsage& curFareUsage,
                                  const CombinabilityRuleInfo* pCat10,
                                  const FarePath& farePath,
                                  Combinations::ValidationFareComponents& validationFareComponent);

  bool validateCombination(DiagCollector& diag,
                           DirectionalityInfo& directionalityInfo,
                           ValidationFareComponents* validationFareComponent,
                           PricingUnit& curPu,
                           FarePath* farePath,
                           FareUsage*& failedFareUsage,
                           FareUsage*& failedEOETargetFareUsage,
                           Itin* itin);

  bool dataStringValidation(DiagCollector& diag,
                            DirectionalityInfo& directionalityInfo,
                            ValidationFareComponents* validationFareComponent,
                            const uint16_t subCat,
                            FareUsage& curFu,
                            const CombinabilityRuleInfo* pCat10,
                            const PricingUnit& curPu,
                            FarePath* farePath,
                            FareUsage*& failedFareUsage,
                            FareUsage*& failedEOETargetFareUsage,
                            Itin* itin);

  bool continueNextSet(bool passedMajor,
                       bool forceFailure,
                       bool majorFound,
                       bool passedMinor,
                       bool haveMinor,
                       bool haveNoMatch);

  bool checkMajorResultAndContinue(
      char majorResult,
      bool& passedMajor,
      const uint16_t subCat,
      bool& haveNoMatch,
      bool& needThenDelimitedDataset,
      bool& forceFailure,
      bool haveMinor,
      const std::vector<CombinabilityRuleItemInfo>::const_iterator& majorCatIter,
      const std::vector<CombinabilityRuleItemInfo>::const_iterator& majorCatEndIter,
      ValidationFareComponents* validationFareComponent,
      const CombinabilityRuleItemInfo* pCat10MajorSeg,
      int datasetNumber,
      FarePath* farePath,
      FareUsage& curFu,
      FareUsage*& failedFareUsage,
      FareUsage*& failedEOETargetFareUsage,
      DiagCollector& diag);

  bool passedCurrentFareComponent(ValidationFareComponents* validationFareComponent,
                                  const uint16_t subCat,
                                  FarePath* farePath,
                                  const PricingUnit& curPu,
                                  FareUsage& curFu,
                                  bool majorFound,
                                  bool passedMajor,
                                  bool failedDirectionality,
                                  FareUsage*& failedFareUsage,
                                  FareUsage*& failedEOETargetFareUsage,
                                  DiagCollector& diag);

  bool matchMajorSubCat(const CombinabilityRuleItemInfo& pCat10Seg,
                        const uint16_t subCat,
                        bool& needThenDelimitedDataset,
                        bool& forceFailure,
                        DiagCollector& diag,
                        bool diagMajorSubCat,
                        int datasetNumber,
                        FareUsage& curFu);

  bool validateDirectionality(const CombinabilityRuleItemInfo& cat10Seg,
                              const FareUsage& fusage,
                              const CombinabilityRuleInfo* pCat10,
                              const DirectionalityInfo& directionalityInfo);

  char processMajorSubCat(DiagCollector& diag,
                          const PricingUnit& pu,
                          FareUsage& curfu,
                          const CombinabilityRuleInfo* pCat10,
                          const CombinabilityRuleItemInfo& pCat10Seg,
                          ValidationFareComponents& validationFareComponent,
                          ValidationElement* negMatchedVfc);

  char processMajorSubCat(DiagCollector& diag,
                          const PricingUnit& pu,
                          const FareUsage& curfu,
                          const CombinabilityRuleInfo* pCat10,
                          const CombinabilityRuleItemInfo& pCat10Seg,
                          const FarePath& farePath,
                          ValidationFareComponents& validationFareComponent,
                          ValidationElement* negMatchedVfc,
                          const DirectionalityInfo& directionalityInfo);

  bool
  setsWithSameMajorCat(const CombinabilityRuleItemInfoSet& set1, const CombinabilityRuleItemInfoSet& set2);

  bool
  processMinorCatOverflow(const std::vector<CombinabilityRuleItemInfoSet*>::const_iterator& setIterBegin,
                          const std::vector<CombinabilityRuleItemInfoSet*>::const_iterator& setIterEnd,
                          bool diagMajorSubCat,
                          DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& curPu,
                          FareUsage& curFU,
                          const CombinabilityRuleInfo* pCat10,
                          ValidationFareComponents& validationFareComponent,
                          ValidationElement*& negMatchedVfc,
                          FareUsage*& failedFareUsage,
                          FareUsage*& failedEOETargetFareUsage,
                          Itin* itin);

  bool processMinorCat(const std::vector<CombinabilityRuleItemInfo>::const_iterator& minorCatIterBegin,
                       const std::vector<CombinabilityRuleItemInfo>::const_iterator& minorCatIterEnd,
                       bool diagMajorSubCat,
                       DiagCollector& diag,
                       FarePath* farePath,
                       const PricingUnit& curPu,
                       FareUsage& curFU,
                       const CombinabilityRuleInfo* pCat10,
                       ValidationFareComponents& validationFareComponent,
                       bool& negAppl,
                       FareUsage*& failedFareUsage,
                       FareUsage*& failedEOETargetFareUsage,
                       Itin* itin);

  char processOpenJawRestriction(DiagCollector& diag,
                                 const PricingUnit& pu,
                                 FareUsage& curFu,
                                 const CombinabilityRuleInfo* pCat10,
                                 const uint32_t itemNo,
                                 ValidationFareComponents& validationFareComponent);

  char processRoundTripRestriction(DiagCollector& diag,
                                   const PricingUnit& pu,
                                   FareUsage& curFu,
                                   const CombinabilityRuleInfo* pCat10,
                                   const uint32_t itemNo,
                                   ValidationFareComponents& validationFareComponent);

  char processCircleTripRestriction(DiagCollector& diag,
                                    const PricingUnit& pu,
                                    const FareUsage& curFu,
                                    const CombinabilityRuleInfo* pCat10,
                                    const uint32_t itemNo,
                                    ValidationFareComponents& validationFareComponent);

  char processEndOnEndRestriction(EndOnEndDataAccess& eoeDA);

  bool sameCarrierCheck(DiagCollector& diag, const PricingUnit& currentPU, const FareUsage& curFu);

  bool sameCarrierCheck(DiagCollector& diag,
                        const FareUsage& curFu,
                        const FareUsage& targetFu,
                        const CombinabilityRuleInfo* pCat10);

  bool processMinorSubCat(DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& currentPU,
                          FareUsage& currentFu,
                          const CombinabilityRuleInfo* pCat10,
                          const CombinabilityRuleItemInfo& pCat10Seg,
                          bool& negative,
                          ValidationFareComponents& validationFareComponent,
                          Itin* itin);

  char processTariffRuleRestriction(DiagCollector& diag,
                                    const FareUsage& curFu,
                                    const CombinabilityRuleInfo* pCat10,
                                    const uint32_t itemNo,
                                    bool& negative,
                                    ValidationFareComponents& validationFareComponent,
                                    const PricingUnit& pu);

  char processFareClassTypeRestriction(DiagCollector& diag,
                                       const FareUsage& curFu,
                                       const CombinabilityRuleInfo* pCat10,
                                       const uint32_t itemNo,
                                       bool& negative,
                                       ValidationFareComponents& validationFareComponent,
                                       const PricingUnit& pu);

  char processFlightRestriction(DiagCollector& diag,
                                const FareUsage& curFu,
                                const CombinabilityRuleInfo* pCat10,
                                const uint32_t itemNo,
                                ValidationFareComponents& validationFareComponent);

  char processSalesRestrictions(DiagCollector& diag,
                                FarePath* farePath,
                                const PricingUnit& curPu,
                                FareUsage& curFu,
                                const CombinabilityRuleInfo& ruleInfo,
                                const CombinabilityRuleItemInfo& ruleItemInfo,
                                const uint32_t itemNo,
                                ValidationFareComponents& validationFareComponent,
                                Itin* itin);

  char processEligibility(DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& curPu,
                          FareUsage& curFu,
                          const CombinabilityRuleInfo& ruleInfo,
                          const CombinabilityRuleItemInfo& ruleItemInfo,
                          const uint32_t itemNo,
                          ValidationFareComponents& validationFareComponent);

  char processMinimumStay(DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& curPu,
                          FareUsage& curFu,
                          const CombinabilityRuleInfo& ruleInfo,
                          const CombinabilityRuleItemInfo& ruleItemInfo,
                          const uint32_t itemNo,
                          ValidationFareComponents& validationFareComponent,
                          Itin* itin);

  char processMaximumStay(DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& curPu,
                          FareUsage& curFu,
                          const CombinabilityRuleInfo& ruleInfo,
                          const CombinabilityRuleItemInfo& ruleItemInfo,
                          const uint32_t itemNo,
                          ValidationFareComponents& validationFareComponent,
                          Itin* itin);

  char processTravelRestriction(DiagCollector& diag,
                          FarePath* farePath,
                          const PricingUnit& curPu,
                          FareUsage& curFu,
                          const CombinabilityRuleInfo& ruleInfo,
                          const CombinabilityRuleItemInfo& ruleItemInfo,
                          const uint32_t itemNo,
                          ValidationFareComponents& validationFareComponent,
                          Itin* itin);

  void displayDiag(DiagCollector& diag, const int16_t errNum);
  void displayDiag(DiagCollector& diag,
                   const int16_t errNum,
                   const FareUsage& fareU,
                   const uint32_t itemNo);

  bool checkOJMileage(DiagCollector& diag,
                      const PricingUnit& pu,
                      bool UseLongerLeg = true,
                      bool restrictDomesticSurf = true);

  bool compareSurface(uint32_t leg1Miles,
                      uint32_t leg2Miles,
                      uint32_t origSurfMiles,
                      uint32_t destSurfMiles,
                      bool UseLongerLeg);

  uint32_t getTPM(const FareMarket& fm, const GeoTravelType& geoTvlType);
  uint32_t getTPM(const Loc& loc1, const Loc& loc2, DateTime travelDate);

  struct TargetIsDummyFare
  {
    bool operator()(const ValidationElement& vEle) const
    {
      return (vEle._targetFareUsage->paxTypeFare()->isDummyFare());
    }
  };

  bool getMultipleValidatingCarriers(const PricingUnit& pu,
                                     const FarePath* fp,                   // during PU-scope, fp will be null
                                     std::vector<CarrierCode>& valCxrList);
  void removeValCxr(CarrierCode& valCxr, PricingUnit& pu, FarePath* fp);  // in PU-Scope, fp will be null
  bool checkQualifyingCat15(CombinabilityRuleInfo* pCat10) const;

  static bool _enablePULevelFailedFareUsageOptimization;
  static bool _enableFPLevelFailedFareUsageOptimization;

  static uint16_t getSubCat(const PricingUnit::Type puType);
  static bool diagInMajorSubCat(const DiagnosticTypes& diagType);

  bool validateInternationalTravel(DiagCollector& diag,
                                   const FarePath& farePath,
                                   ValidationFareComponents& validationFareComponent,
                                   const EndOnEnd* pEndOnEnd);

  char checkGeoSpec(const EndOnEndDataAccess& eoeDA, const size_t fuCount);

  void diagEndOnEndFareInfo(EndOnEndDataAccess& eoeDA);
  void diagEndOnEndTargetFare(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  void diagEndOnEndTargetFUPriorPassed(const EndOnEndDataAccess& eoeDA);
  void diagEndOnEndFailedSameCxr(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  void
  diagEndOnEndPassBlankNmlSpclInd(const EndOnEndDataAccess& eoeDA, const PaxTypeFare& targetPTF);
  void
  diagEndOnEndPassBlankDomIntlTransInd(const EndOnEndDataAccess& eoeDA, const Fare& targetFare);

  char processUnavailTag(const EndOnEndDataAccess& eoeDA);
  char processEndOnEndTbl994(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndABA(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndRestInd(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndSameCxr(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndCxrPreference(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndNormalFare(EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndSpecialFare(EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndDomIntlTransbFare(EndOnEndDataAccess& eoeDA, const size_t fuCount);
  char processEndOnEndFareGeo(EndOnEndDataAccess& eoeDA,
                              const size_t fuCount,
                              const Indicator geoInd,
                              const std::string& diagGeoStr);
  char processEndOnEndTktInd(const EndOnEndDataAccess& eoeDA, const size_t fuCount);
  static PaxTypeFare* fareChangedCat10overrideCat25(PaxTypeFare* fare, const PricingUnit* pu);
};
} /* end tse namespace */
