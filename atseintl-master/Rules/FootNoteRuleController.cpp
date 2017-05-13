#include "Rules/FootNoteRuleController.h"

#include "Common/FallbackUtil.h"
#include "Common/MetricsUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag500Collector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackSkipDummyFareValidation);
FALLBACK_DECL(fallbackFootNoteR2Optimization);

FootNoteRuleController::FootNoteRuleController() : FareMarketRuleController(FootNotePrevalidation)
{
};

FootNoteRuleController::FootNoteRuleController(bool altDates) : FareMarketRuleController(
    altDates ? FootNotePrevalidationALT : FootNotePrevalidation)
{
};

bool
FootNoteRuleController::passCategory(Record3ReturnTypes retResultOfRule) const
{
  switch (retResultOfRule)
  {
  case PASS:
  case SOFTPASS:
  case NOTPROCESSED:
  case SKIP:
    return true;
  default:
    return false;
  }
}

bool
FootNoteRuleController::processCategorySequenceCommon(PricingTrx& trx,
                                                      RuleControllerDataAccess& da,
                                                      std::vector<uint16_t>& categorySequence)
{
  PaxTypeFare& paxTypeFare = da.paxTypeFare();

  paxTypeFare.setFootnotesPrevalidated();
  getFootnotes(paxTypeFare, da.footNoteTbl());

  if (da.footNoteTbl().empty())
    return true;

  if (!fallback::fixed::fallbackSkipDummyFareValidation())
  {
    if (UNLIKELY(paxTypeFare.isDummyFare()))
      return true;
  }

  for (const uint16_t category : categorySequence)
  {
    if (da.isSitaVendor() && category == RuleConst::SEASONAL_RULE)
      continue;

    da.currentCatNum() = category;

    Record3ReturnTypes retResultOfRule = processRules(trx, da, category, true);

    if (retResultOfRule == PASS)
      paxTypeFare.fare()->setFootnoteRec2Status(category);

    if (catSetCategoryAsProcessed(paxTypeFare, category, retResultOfRule))
      paxTypeFare.setCategoryProcessed(category);

    if (!passCategory(retResultOfRule) && !paxTypeFare.cat15SecurityFail())
    {
      if (UNLIKELY(skipCmdPricingOrErd(trx, paxTypeFare, category)))
        continue;

      paxTypeFare.setCategoryValid(category, false);

      return false;
    }
  }
  return true;
}

Record3ReturnTypes
FootNoteRuleController::processRules(PricingTrx& trx,
                                     RuleControllerDataAccess& da,
                                     const uint16_t categoryNumber,
                                     const bool skipCat15Security,
                                     const bool processSystemAssumption)
{
  Record3ReturnTypes retResultOfRule = SKIP;
  RuleControllerParam rcParam;
  RuleProcessingData rpData;

  if (UNLIKELY(PricingTrx::ESV_TRX == trx.getTrxType())) // ESV processing
  {
    if ((categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
         categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE))
    {
      retResultOfRule = fallback::fallbackFootNoteR2Optimization(&trx)
                            ? processFootnoteRule<FallBackOn>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
                            : processFootnoteRule<FallBackOff>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security);
    }
  }
  else // not ESV processing
  {
    da.setRuleType(FootNote);
    retResultOfRule = fallback::fallbackFootNoteR2Optimization(&trx)
                          ? processFootnoteRule<FallBackOn>(
                                trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
                          : processFootnoteRule<FallBackOff>(
                                trx, da, categoryNumber, rcParam, rpData, skipCat15Security);
  }

  da.paxTypeFare().fare()->setCheckFareRuleAltGenRule(categoryNumber,
                                                      rcParam._checkFareRuleAltGenRule);

  return retResultOfRule;
}

bool
FootNoteRuleController::canSkipProcessingCategoryLater(const uint16_t category) const
{
  return category != RuleConst::BLACKOUTS_RULE && category != RuleConst::TRAVEL_RESTRICTIONS_RULE
         && category != RuleConst::SALE_RESTRICTIONS_RULE;
}

bool
FootNoteRuleController::catSetCategoryAsProcessed(const PaxTypeFare& paxTypeFare, const uint16_t category,
                                                  const Record3ReturnTypes retResultOfRule) const
{
  return (retResultOfRule == PASS || retResultOfRule == SOFTPASS) && canSkipProcessingCategoryLater(category);
}

}
