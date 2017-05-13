//----------------------------------------------------------------
//
//  File: RuleController.h
//  Authors:  Devapriya SenGupta
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DBAForwardDecl.h"
#include "Rules/CategoryRuleItem.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/RuleControllerDataAccess.h"

#include <cstdint>
#include <map>
#include <vector>


namespace tse
{
class PricingTrx;
class Itin;
class PaxTypeFare;
class CategoryRule;
class GeneralFareRuleInfo;
class Loc;
class Fare;
class FareMarket;
class RuleControllerParam;
class PricingUnit;
class FareByRuleItemInfo;
class DCFactory;
class Diag500Collector;
class WarningMap;
class PaxTypeFareRuleData;
class FBRPaxTypeFareRuleData;
class RexBaseTrx;

struct PricingUnitFareRuleCaller;
struct ItinFareRuleCaller;

class RuleProcessingData;

/// A data encapsulation class

class RuleControllerParam
{
public:
  void reset();

  TariffNumber _genTariffNumber = 0;
  RuleNumber _genRuleNumber = RuleConst::NULL_GENERAL_RULE;
  bool _checkFareRuleAltGenRule = true;
  bool _generalRuleEnabled = false;
  bool _readRecord0 = true;
  bool _processGenRuleForFMS = false;
};

// @class RuleController
// @brief Defines the Rule validation coordinator.
class RuleController
{
  friend class RuleControllerTest;

public:
  using CategoryPhaseMap = std::map<CategoryPhase, std::vector<uint16_t>>;

  RuleController();
  RuleController(CategoryPhase categoryPhase);
  RuleController(CategoryPhase categoryPhase, const std::vector<uint16_t>& categories);
  void setCategoryPhase(CategoryPhase categoryPhase);
  virtual ~RuleController() = default;

  virtual bool setPhase(CategoryPhase categoryPhase);

  CategoryRuleItemSet& categoryRuleItemSet() { return _categoryRuleItemSet; }

  std::vector<uint16_t>& categorySequence() { return _categorySequence; }
  const std::vector<uint16_t>& categorySequence() const { return _categorySequence; }

  virtual bool validate(PricingTrx&, Itin&, PaxTypeFare&) { return true; }

  void removeCat(const uint16_t category);
  void setReuseResult(bool reuseResult) { _reuseResult = reuseResult; }
  void addCat(uint16_t category) { _categorySequence.push_back(category); }

  bool diag500Parameters(PricingTrx& trx, DCFactory*& factory, Diag500Collector*& diag);
  bool isCatNeededInDiag(const std::vector<CategoryRuleItemInfoSet*>& ruleSet) const;

  virtual Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                                     const CategoryRuleInfo&,
                                                     const std::vector<CategoryRuleItemInfoSet*>&,
                                                     RuleControllerDataAccess& da,
                                                     RuleProcessingData& rpData,
                                                     bool isLocationSwapped,
                                                     bool isFareRule,
                                                     bool skipCat15Security) = 0;

  bool isBasicValidationPhase() const;
  bool isCat35Fare(const PaxTypeFare& paxTypeFare) const;
  bool revalidateDiscountQualifiers(PricingTrx& trx,
                                    RuleControllerDataAccess& da,
                                    uint16_t category,
                                    PaxTypeFare& paxTypeFare,
                                    PaxTypeFareRuleData* ruleData);

  void getFootnotes(PaxTypeFare& paxTypeFare, std::vector<FootnoteTable>& footNoteTbl);
  uint16_t getProcessedCategory() const { return _processedCategory; }
  void setProcessedCategory(const uint16_t category) { _processedCategory = category; }
  bool doesPhaseContainCategory(const CategoryPhase phase, const uint16_t categoryNumber) const;

protected:
  class FallBackOn
  {
  public:
    bool operator()(PricingTrx& trx,
                    const FootNoteCtrlInfoPair& fnci,
                    const PaxTypeFare& paxTypeFare,
                    bool& isLocationSwapped,
                    const Footnote& footnote,
                    const TariffNumber& tariff);
  };

  class FallBackOff
  {
  public:
    bool operator()(PricingTrx& trx,
                    const FootNoteCtrlInfoPair& fnci,
                    const PaxTypeFare& paxTypeFare,
                    bool& isLocationSwapped,
                    const Footnote& footnote,
                    const TariffNumber& tariff);
  };

  class FallBackOnGfr
  {
  public:
    bool operator()(PricingTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    const GeneralFareRuleInfoPair& gfri,
                    bool& isLocationSwapped);
  };

  class FallBackOffGfr
  {
  public:
    bool operator()(PricingTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    const GeneralFareRuleInfoPair& gfri,
                    bool& isLocationSwapped);
  };

  virtual Record3ReturnTypes validateBaseFare(uint16_t category,
                                              const FareByRuleItemInfo* fbrItemInfo,
                                              bool& checkFare,
                                              PaxTypeFare* fbrBaseFare,
                                              RuleControllerDataAccess& da) = 0;

  virtual Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                          bool& checkFare,
                                                          PaxTypeFare* ptf,
                                                          RuleControllerDataAccess& da) = 0;
  virtual void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                           RuleControllerDataAccess& da,
                                           uint16_t categoryNumber,
                                           RuleControllerParam& rcParam,
                                           bool skipCat15Security) = 0;

  virtual Record3ReturnTypes applySystemDefaultAssumption(PricingTrx& trx,
                                                          RuleControllerDataAccess& da,
                                                          const uint16_t category,
                                                          bool& displayDiag);

  bool passCategory(uint16_t categoryNumber,
                    Record3ReturnTypes retResultOfRule,
                    PaxTypeFare& paxTypeFare);

  bool needFootnoteRule(uint16_t categoryNumber) const;
  bool needFootnoteRule(uint16_t categoryNumber, const RuleControllerDataAccess& da,
                        Record3ReturnTypes& retResultOfRule) const;
  bool needFootnoteRule(const PricingTrx& trx, uint16_t categoryNumber, const RuleControllerDataAccess& da,
                        Record3ReturnTypes& retResultOfRule) const;

  bool needFareRule(const uint16_t categoryNumber,
                    Record3ReturnTypes retResultOfRule,
                    RuleControllerParam& rcParam);

  bool needGeneralRule(PricingTrx& trx,
                       const uint16_t categoryNumber,
                       Record3ReturnTypes retResultOfRule,
                       RuleControllerParam& rcParam);

  const GeneralFareRuleInfo* findFareRule(PricingTrx& trx,
                                          PaxTypeFare& paxTypeFare,
                                          GeneralFareRuleInfoVec& gfrInfoVec,
                                          bool& isLocationSwapped,
                                          const uint16_t categoryNumber);

  template <typename FallBackSwitch>
  const GeneralFareRuleInfo*
  findGnFareRuleOnFareMarket(PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             const TariffNumber& ruleTariff,
                             const RuleNumber& ruleNumber,
                             const uint16_t categoryNumber,
                             GeneralFareRuleInfoVec& gfrInfoVec,
                             bool& isLocationSwapped,
                             const bool isFareRule,
                             const FareMarket::FareMarketSavedGfrResult::Result*& savedRet,
                             FareMarket::FareMarketSavedGfrResult::Results*& savedRetMap,
                             Itin* itin,
                             bool skipCat15Security);

  template <typename FallBackSwitch>
  const FootNoteCtrlInfo*
  findFnCtrlInfoOnFareMarket(PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             FootNoteCtrlInfoVec& fnCtrlInfoVec,
                             bool& isLocationSwapped,
                             const TariffNumber& ruleTariff,
                             const Footnote& footnote,
                             const uint16_t& fnIndex,
                             const uint16_t categoryNumber,
                             const FareMarket::FareMarketSavedFnResult::Result*& savedRet,
                             FareMarket::FareMarketSavedFnResult::Results*& savedRetMap,
                             Itin* itin,
                             bool skipCat15Security);

  bool isDirectional(const CategoryRuleInfo& catRuleInfo);

  bool isThereQualifier(const CategoryRuleInfo& catRuleInfo, const uint16_t& qualifierCatNum) const;

  bool skipCategoryProcessing(const uint16_t category,
                              const PaxTypeFare& paxTypeFre,
                              const PricingTrx& trx) const;

  Record3ReturnTypes
  processCategoryNormalValidation(PricingTrx& trx, const uint16_t category,
                                  PaxTypeFare& paxTypeFare,
                                  bool fbrCalcFare,
                                  const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const;

  bool skipCategoryNormalValidation(PricingTrx& trx,
                                    const uint16_t category,
                                    PaxTypeFare& paxTypeFare,
                                    bool fbrCalcFare,
                                    const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const;
  bool
  skipCategoryFCORuleValidation(PricingTrx& trx, uint16_t category, PaxTypeFare& paxTypeFare) const;
  bool skipRuleDisplayValidation(uint16_t category, const PaxTypeFare& paxTypeFare) const;
  bool skipCalculatedFBR(const PaxTypeFare& paxTypeFare, bool fbrCalcFare, uint16_t category) const;
  bool skipCat15ForSellingNetFare(Indicator fcaDisplayCatType) const;

  bool
  skipCategoryPreValidation(PricingTrx& trx, uint16_t category, PaxTypeFare& paxTypeFare) const;

  virtual bool processCategorySequenceCommon(PricingTrx& trx,
                                     RuleControllerDataAccess& da,
                                     std::vector<uint16_t>& categorySequence);
  bool ifSkipCat15Security(const PaxTypeFare& paxTypeFare, uint16_t category, bool isFD) const;
  bool existCat15QualifyCat15ValidatingCxrRest(PricingTrx& trx, const CategoryRuleInfo& catRuleInfo) const;
  Record3ReturnTypes processCategoryPhase(PricingTrx& trx,
                                          RuleControllerDataAccess& da,
                                          PaxTypeFare& paxTypeFare,
                                          uint16_t category,
                                          const FBRPaxTypeFareRuleData* fbrPaxTypeFare,
                                          bool fbrCalcFare);
  Record3ReturnTypes processCategoryPurFprShopping(PricingTrx& trx,
                                                   RuleControllerDataAccess& da,
                                                   PaxTypeFare& paxTypeFare,
                                                   uint16_t category,
                                                   bool fbrCalcFare);
  bool passForErd(const PricingTrx& trx, const PaxTypeFare& paxTypeFare, uint16_t category) const;
  bool
  passForCmdPricing(const PricingTrx& trx, const PaxTypeFare& paxTypeFare, uint16_t category) const;
  bool
  skipCmdPricingOrErd(const PricingTrx& trx, PaxTypeFare& paxTypeFare, uint16_t category) const;

  void updateResultOfRule(Record3ReturnTypes& retResultOfRule,
                          uint16_t category,
                          const PaxTypeFare& paxTypeFare,
                          bool revalidC15ForDisc,
                          const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const;
  void updateResultOfRuleAndFareDisplayInfo(Record3ReturnTypes& retResultOfRule,
                                            uint16_t category,
                                            PaxTypeFare& paxTypeFare,
                                            const FBRPaxTypeFareRuleData* fbrPaxTypeFare,
                                            PricingTrx& trx) const;
  bool checkMissedFootnote(PaxTypeFare& paxTypeFare, PricingTrx& trx, RuleControllerDataAccess& da);
  void updateBaseFareRuleStatDiscounted(PaxTypeFare& paxTypeFare,
                                        const PricingTrx& trx,
                                        uint16_t category) const;
  bool checkIfPassForCmdPricing(PricingTrx& trx,
                                uint16_t category,
                                const PaxTypeFare& paxTypeFare,
                                RuleControllerDataAccess& da) const;

  Record3ReturnTypes processFareRuleCommon(PricingTrx& trx,
                                           RuleControllerDataAccess& da,
                                           const uint16_t categoryNumber,
                                           RuleControllerParam& rcParam,
                                           RuleProcessingData& rpData,
                                           bool skipCat15Security);
  void storeSalesRestrictionValuesFr(PaxTypeFare& paxTypeFare,
                                     uint16_t categoryNumber,
                                     const FareMarket::FareMarketSavedGfrResult::Result* savedRet,
                                     bool isFareDisplayTrx);
  void storeNotFailedValuesFr(PaxTypeFare& paxTypeFare,
                              uint16_t categoryNumber,
                              FareMarket::FareMarketSavedGfrResult::Result* savingRet,
                              RuleControllerParam& rcParam,
                              bool currentMatchCorpIDFlag);
  void processGeneralFareRuleInfoVecFr(Record3ReturnTypes& ret,
                                       GeneralFareRuleInfoVec& gfrInfoVec,
                                       PaxTypeFare& paxTypeFare,
                                       uint16_t categoryNumber,
                                       RuleControllerDataAccess& da,
                                       PricingTrx& trx,
                                       RuleProcessingData& rpData,
                                       RuleControllerParam& rcParam,
                                       const GeneralFareRuleInfo* gfrRuleInfo,
                                       FareMarket::FareMarketSavedGfrResult::Results* savedResults,
                                       bool isLocationSwapped,
                                       bool skipCat15Security);
  void storeResultsFr(PricingTrx& trx,
                      PaxTypeFare& paxTypeFare,
                      uint16_t categoryNumber,
                      const FareMarket::FareMarketSavedGfrResult::Result* savedRet,
                      RuleControllerParam& rcParam);

  template <typename FallBackSwitch>
  Record3ReturnTypes processGeneralRule(PricingTrx& trx,
                                        RuleControllerDataAccess& da,
                                        uint16_t categoryNumber,
                                        RuleControllerParam& rcParam,
                                        RuleProcessingData& rpData,
                                        bool skipCat15Security);
  void storeSalesRestrictionValuesGr(PricingTrx& trx,
                                     PaxTypeFare& paxTypeFare,
                                     uint16_t categoryNumber,
                                     const FareMarket::FareMarketSavedGfrResult::Result* savedRet);

  void storeEligibilityValues(PaxTypeFare& paxTypeFare,
                              uint16_t categoryNumber,
                              const FareMarket::FareMarketSavedGfrResult::Result* savedRet);
  void storeNotFailedValuesGr(PaxTypeFare& paxTypeFare,
                              uint16_t categoryNumber,
                              const FareMarket::FareMarketSavedGfrResult::Result* savedRet);
  void processGeneralFareRuleInfoVecGr(Record3ReturnTypes& ret,
                                       GeneralFareRuleInfoVec& gfrInfoVec,
                                       PaxTypeFare& paxTypeFare,
                                       uint16_t categoryNumber,
                                       RuleControllerDataAccess& da,
                                       PricingTrx& trx,
                                       RuleProcessingData& rpData,
                                       const GeneralFareRuleInfo* gfrRuleInfo,
                                       FareMarket::FareMarketSavedGfrResult::Results* savedResults,
                                       CarrierCode& carrier,
                                       bool isLocationSwapped,
                                       bool skipCat15Security);
  bool checkIfReuse(uint16_t categoryNumber) const;

  template <typename FallBackSwitch>
  Record3ReturnTypes processFootnoteRule(PricingTrx& trx,
                                         RuleControllerDataAccess& da,
                                         uint16_t categoryNumber,
                                         RuleControllerParam& rcParam,
                                         RuleProcessingData& rpData,
                                         bool skipCat15Security);

  bool checkIfShouldFindFnCtrlInfo(uint16_t categoryNumber, bool isFareDisplayTrx) const;
  bool checkIfShouldFindFnCtrlInfoForDynamicValidation(const PaxTypeFare& paxTypeFare,
                                                       uint16_t categoryNumber,
                                                       bool isFareDisplayTrx) const;
  void storeSalesRestrictionValuesFn_deprecated(PricingTrx& trx,
                                     PaxTypeFare& paxTypeFare,
                                     uint16_t categoryNumber,
                                     const FareMarket::FareMarketSavedFnResult::Result* savedRet);

  void storeSalesRestrictionValuesFn(PricingTrx& trx,
                                     PaxTypeFare& paxTypeFare,
                                     uint16_t categoryNumber,
                                     const FareMarket::FareMarketSavedFnResult::Result* savedRet);

  void saveFnIntoFareMarket_deprecated(PaxTypeFare& paxTypeFare,
                            PricingTrx& trx,
                            FareMarket::FareMarketSavedFnResult::Results* savedResults,
                            uint16_t categoryNumber,
                            CarrierCode& carrier,
                            Record3ReturnTypes ret,
                            FootnoteTable& fnTblIter,
                            const FootNoteCtrlInfo* fnCtrlInfo,
                            bool currentFOPFlag);

  void saveFnIntoFareMarket(PaxTypeFare& paxTypeFare,
                            PricingTrx& trx,
                            FareMarket::FareMarketSavedFnResult::Results* savedResults,
                            uint16_t categoryNumber,
                            CarrierCode& carrier,
                            Record3ReturnTypes ret,
                            FootnoteTable& fnTblIter,
                            const FootNoteCtrlInfo* fnCtrlInfo,
                            bool currentFOPFlag);

  void savedRetNotProcessedFn(Record3ReturnTypes& ret,
                              bool& isLocationSwapped,
                              PaxTypeFare& paxTypeFare,
                              PricingTrx& trx,
                              uint16_t categoryNumber,
                              const FootNoteCtrlInfo*& fnCtrlInfo,
                              FootNoteCtrlInfoVec& fnCtrlInfoVec,
                              RuleControllerDataAccess& da,
                              RuleProcessingData& rpData,
                              RuleControllerParam& rcParam,
                              bool skipCat15Security);

  CategoryRuleItemSet _categoryRuleItemSet;

  friend struct PricingUnitFareRuleCaller;
  friend struct ItinFareRuleCaller;

  Record3ReturnTypes processRules(PricingTrx& trx,
                                  RuleControllerDataAccess& da,
                                  const uint16_t categoryNumber,
                                  const bool& checkOtherRules,
                                  const bool skipCat15Security,
                                  const bool processSystemAssumption = true);

  virtual Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                                      RuleControllerDataAccess& da,
                                                      const uint16_t category,
                                                      RuleProcessingData& rpData,
                                                      const Record3ReturnTypes preResult) = 0;

  Record3ReturnTypes processFareGroupRules(PricingTrx& trx,
                                           RuleControllerDataAccess& da,
                                           const uint16_t categoryNumber);

  bool isCatQualifiedFltApp(const PaxTypeFare& paxTypeFare, const uint16_t category) const;

  bool setFBDisplayData(PricingTrx& trx,
                        uint16_t categoryNumber,
                        const FareRuleRecord2Info* fRR2,
                        const FootNoteRecord2Info* fNR2,
                        const GeneralRuleRecord2Info* gRR2,
                        PaxTypeFare& paxTypeFare);

  bool updateFareDisplayInfo(uint16_t cat, PricingTrx& trx, const PaxTypeFare& paxTypeFare) const;

  bool saveRuleInfo(PricingTrx& trx,
                    uint16_t cat,
                    const PaxTypeFare& paxTypeFare,
                    const FareByRuleItemInfo& fbrItemInfo);
  void setFBRBaseFareRuleInfo(PricingTrx& trx,
                              uint16_t cat,
                              const PaxTypeFare& paxTypeFare,
                              const FareByRuleItemInfo* fbrItemInfo);

  void updateRuleType(PaxTypeFare& paxTypeFare, const Indicator& ruleType);

  void
  saveTktDates(PaxTypeFare& paxTypeFare, FareMarket::FareMarketSavedGfrResult::Result* savingRet);

  void setTktDates(PaxTypeFare& paxTypeFare,
                   const DateTime& earliestTktDate,
                   const DateTime& latestTktDate);

  bool missedFootnote(PaxTypeFare& paxTypeFare,
                      const std::vector<FootnoteTable>& footNoteTbl,
                      PricingTrx& trx);

  bool noFailOnFNMissing(const PaxTypeFare& paxTypeFare);

  virtual Record3ReturnTypes
  reValidDiscQualifiers(PaxTypeFare&, const uint16_t&, RuleControllerDataAccess&)
  {
    return PASS;
  }

  bool fareRuleTakePrecedenceOverGenRule(PricingTrx& trx, uint16_t categoryNumber);

  void reuseWarningMap(const WarningMap& srcWarningMap,
                       const WarningMap& trgWarningMap,
                       uint16_t categoryNumber) const;

  void skipRexSecurityCheck(const RexBaseTrx& rexTrx,
                            PaxTypeFare& ptf,
                            std::vector<uint16_t>& categorySequence);

  static bool mayBeSkippedByCmdPricing(const PricingTrx& trx,
                                       const PaxTypeFare& paxTypeFare,
                                       uint16_t category);

  static bool
  dontReuseSaleRestrictionRule(const FareMarket::FareMarketSavedGfrResult::Result& savedRet,
                               const PaxTypeFare& paxTypeFare);

  static bool doNotReuseGrFareRuleResult(PricingTrx& trx,
                                         const uint16_t categoryNumber,
                                         const bool chkPsgEligibility,
                                         const bool skipCat15Security);

  const CategoryPhaseMap& getCategoryPhaseMap() const;

  bool shouldApplyGeneralRuleSurcharges(PricingTrx& trx, const PaxTypeFare& ptf) const;
  bool fareApplicableToGeneralRuleSurcharges(PricingTrx& trx,
                                             const PaxTypeFare& ptf,
                                             const PaxTypeFare* candidate) const;

  void copyFlexFaresValidationStatus(PricingTrx& trx,
                                     PaxTypeFare& target,
                                     const PaxTypeFare& source,
                                     const uint16_t& category);

  std::vector<uint16_t> _categorySequence;
  CategoryPhase _categoryPhase = CategoryPhase::NormalValidation;
  uint16_t _diagCategoryNumber = 0; // Used by the /RL diag parameter
  uint16_t _processedCategory = 0;
  bool _fmPhase = true;
  bool _reuseResult = true;
  bool _loopValidatingCxrs = false;
  bool _highLevelLoopValidatingCxrs = false;
  bool _fallbackCat25baseFarePrevalidation = false;

private:
  Record3ReturnTypes processGnFareRule(PricingTrx& trx,
                                       RuleControllerDataAccess& da,
                                       PaxTypeFare& paxTypeFare,
                                       const GeneralFareRuleInfo& gfrInfo,
                                       uint16_t categoryNumber,
                                       RuleControllerParam* rcPara,
                                       RuleProcessingData& rpData,
                                       const bool isLocationSwapped,
                                       const bool isFareRule,
                                       const bool skipCat15Security);

  Record3ReturnTypes processFareRuleInfo(PricingTrx& trx,
                                         RuleControllerDataAccess& da,
                                         GeneralFareRuleInfoVec& gfrInfoVec,
                                         const uint16_t categoryNumber,
                                         const GeneralFareRuleInfo* gfrInfo,
                                         bool& isLocationSwapped,
                                         RuleControllerParam* rcParam,
                                         RuleProcessingData& rpData,
                                         bool isFareRule,
                                         bool skipCat15Security);

  void copySavedValidationResult_deprecated(PaxTypeFare& paxTypeFare,
                                 uint16_t categoryNumber,
                                 const FareMarket::FareMarketSavedFnResult::Result& savedRet);

  void copySavedValidationResult(PaxTypeFare& paxTypeFare,
                                 uint16_t categoryNumber,
                                 const FareMarket::FareMarketSavedFnResult::Result& savedRet);

  void diagReuse(PricingTrx& trx,
                 const CategoryRuleInfo* gfrRuleInfo,
                 const PaxTypeFare& paxTypeFare,
                 const uint16_t categoryNumber,
                 const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const TariffNumber& ruleTariff,
                 const RuleNumber* ruleNumber,
                 const Footnote* footNote,
                 const Indicator ruleType,
                 const FareClassCode& fcReusedFrom,
                 const Record3ReturnTypes& reusedRet);

  void setupFallbacks();

  static const uint16_t MAX_REUSE_CAT = 22;
  static bool _skipReuseCat[MAX_REUSE_CAT + 1];

  static bool skipReuse(const uint16_t categoryNum);
  static bool collectSkipReuse();

protected:
  // for unit test only
  explicit RuleController(int) {}
};
} // tse namespace
