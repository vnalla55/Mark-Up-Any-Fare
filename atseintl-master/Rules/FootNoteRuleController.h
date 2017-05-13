#pragma once

#include "Rules/FareMarketRuleController.h"

namespace tse
{

class FootNoteRuleController : public FareMarketRuleController
{
public:
  FootNoteRuleController();
  FootNoteRuleController(bool);

private:
  bool processCategorySequenceCommon(PricingTrx& trx,
                                     RuleControllerDataAccess& da,
                                     std::vector<uint16_t>& categorySequence) override;
  bool passCategory(Record3ReturnTypes retResultOfRule) const;
  Record3ReturnTypes processRules(PricingTrx& trx,
                               RuleControllerDataAccess& da,
                               const uint16_t categoryNumber,
                               const bool skipCat15Security,
                               const bool processSystemAssumption = true);

  inline bool canSkipProcessingCategoryLater(const uint16_t category) const;
  inline bool catSetCategoryAsProcessed(const PaxTypeFare& paxTypeFare, const uint16_t category,
                                        const Record3ReturnTypes retResultOfRule) const;

};

} // namespace tse
