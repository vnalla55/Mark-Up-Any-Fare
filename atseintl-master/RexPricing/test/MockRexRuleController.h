#ifndef MOCK_REX_RULE_CONTROLLER
#define MOCK_REX_RULE_CONTROLLER

#include "Rules/RuleController.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "Rules/RuleConst.h"

namespace tse
{

class MockRexRuleController : public RuleController
{
public:
  MockRexRuleController(CategoryPhase phase) : RuleController(phase) {};

  /// Executes the logic to run through the various
  /// Record 2 Rules and validate them per Category

  virtual bool validate(PricingTrx& trx, Itin& itin, PaxTypeFare& paxTypeFare)
  {
    const bool isLocationSwapped = false;
    const bool isFareRule = true;

    if (paxTypeFare.ruleNumber() == "NRUL")
    {
      // do not do anything
    }
    else if (paxTypeFare.ruleNumber() == "NMAT")
    {
      GeneralFareRuleInfo* gfrRuleInfo = 0;
      trx.dataHandle().get(gfrRuleInfo);

      paxTypeFare.setCatRuleInfo(gfrRuleInfo,
                                 RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                 trx.dataHandle(),
                                 isLocationSwapped,
                                 isFareRule);
    }
    else if (paxTypeFare.ruleNumber() == "MATC")
    {
      GeneralFareRuleInfo* gfrRuleInfo = 0;
      trx.dataHandle().get(gfrRuleInfo);

      paxTypeFare.setCatRuleInfo(gfrRuleInfo,
                                 RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                 trx.dataHandle(),
                                 isLocationSwapped,
                                 isFareRule);
      paxTypeFare.setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    }
    else if (paxTypeFare.ruleNumber() == "BASF")
    {
      PaxTypeFare& baseFare = *(paxTypeFare.baseFare());

      if (baseFare.ruleNumber() == "NRUL")
      {
        // do not do anything
      }
      else if (baseFare.ruleNumber() == "NMAT")
      {
        GeneralFareRuleInfo* gfrRuleInfo = 0;
        trx.dataHandle().get(gfrRuleInfo);

        baseFare.setCatRuleInfo(gfrRuleInfo,
                                RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                trx.dataHandle(),
                                isLocationSwapped,
                                isFareRule);
      }
      else if (baseFare.ruleNumber() == "MATC")
      {
        GeneralFareRuleInfo* gfrRuleInfo = 0;
        trx.dataHandle().get(gfrRuleInfo);

        baseFare.setCatRuleInfo(gfrRuleInfo,
                                RuleConst::VOLUNTARY_EXCHANGE_RULE,
                                trx.dataHandle(),
                                isLocationSwapped,
                                isFareRule);
        baseFare.setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
      }
    }

    return true;
  }

protected:
  virtual Record3ReturnTypes validateBaseFare(uint16_t category,
                                              const FareByRuleItemInfo* fbrItemInfo,
                                              bool& checkFare,
                                              PaxTypeFare* fbrBaseFare,
                                              RuleControllerDataAccess& da)
  {
    return PASS;
  }

  virtual Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                          bool& checkFare,
                                                          PaxTypeFare* ptf,
                                                          RuleControllerDataAccess& da)
  {
    return PASS;
  }

  virtual Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                                     const CategoryRuleInfo&,
                                                     const std::vector<CategoryRuleItemInfoSet*>&,
                                                     RuleControllerDataAccess& da,
                                                     RuleProcessingData& rpData,
                                                     bool isLocationSwapped,
                                                     bool isFareRule,
                                                     bool skipCat15Security)
  {
    return PASS;
  }

  virtual void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                           RuleControllerDataAccess& da,
                                           uint16_t categoryNumber,
                                           RuleControllerParam& rcParam,
                                           bool skipCat15Security)
  {
  }

  virtual Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                                      RuleControllerDataAccess& da,
                                                      const uint16_t category,
                                                      RuleProcessingData& rpData,
                                                      const Record3ReturnTypes preResult)
  {
    return PASS;
  }
};

} // namespace tse

#endif // FARE_MARKET_RULE_CONTROLLER
