#pragma once

#include "Rules/RuleController.h"

namespace tse
{
class Itin;
class RuleProcessingData;

class FareMarketRuleController : public RuleController
{
  friend class FareMarketRuleControllerTest;

public:
  FareMarketRuleController() : RuleController() {};

  FareMarketRuleController(CategoryPhase phase) : RuleController(phase) {};

  FareMarketRuleController(CategoryPhase phase, const std::vector<uint16_t>& categories)
    : RuleController(phase, categories) {};

  /// Executes the logic to run through the various
  /// Record 2 Rules and validate them per Category
  /// For an entire fareMarket

  bool validate(PricingTrx& trx, FareMarket& fareMarket, Itin& itin);

  /// Executes the logic to run through the various
  /// Record 2 Rules and validate them per Category

  bool validate(PricingTrx& trx, Itin& itin, PaxTypeFare& paxTypeFare) override;

  Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                              RuleControllerDataAccess& da,
                                              const uint16_t category,
                                              RuleProcessingData& rpData,
                                              const Record3ReturnTypes preResult) override;

protected:
  bool processCategorySequence(PricingTrx&, Itin&, PaxTypeFare&);

  Record3ReturnTypes validateBaseFare(uint16_t category,
                                      const FareByRuleItemInfo* fbrItemInfo,
                                      bool& checkFare,
                                      PaxTypeFare* fbrBaseFare,
                                      RuleControllerDataAccess& da) override;

  Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                  bool& checkFare,
                                                  PaxTypeFare* ptf,
                                                  RuleControllerDataAccess& da) override;

  bool reValidateMixedRtnPTF(RuleControllerDataAccess& da,
                             PaxTypeFare& paxTypeFare,
                             std::vector<CarrierCode>& validatingCarriers);

  Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                             const CategoryRuleInfo&,
                                             const std::vector<CategoryRuleItemInfoSet*>&,
                                             RuleControllerDataAccess& da,
                                             RuleProcessingData& rpData,
                                             bool isLocationSwapped,
                                             bool isFareRule,
                                             bool skipCat15Security) override;

  void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                   RuleControllerDataAccess& da,
                                   uint16_t categoryNumber,
                                   RuleControllerParam& rcParam,
                                   bool skipCat15Security) override;

protected:
  // for unit test only
  explicit FareMarketRuleController(int test) : RuleController(test) {}
};
} // namespace tse

