/*---------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/SurchargesValidator.h"

#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/SavedSurcharges.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/VectorSwapper.h"
#include "Rules/Config.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

namespace tse
{
FALLBACK_DECL(validateSurchargesMotherSolution)
namespace similaritin
{
namespace
{
class RuleValidationResultsGuard
{
public:
  RuleValidationResultsGuard(FarePath& farePath) : _farePath(farePath)
  {
    for (PricingUnit* pricingUnit : _farePath.pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        saveCat12Surcharges(*fareUsage);
      }
    }
  }

  void rollback() { revertCat12Surcharges(); }

private:
  void saveCat12Surcharges(FareUsage& fareUsage)
  {
    PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();
    _cat12Surcharges.push_back(SavedSurcharges(
        fareUsage.surchargeAmt(), fareUsage.surchargeData(), paxTypeFare.surchargeData()));
    paxTypeFare.setCategoryProcessed(RuleConst::SURCHARGE_RULE, false);
    paxTypeFare.setCategorySoftPassed(RuleConst::SURCHARGE_RULE, false);
    paxTypeFare.surchargeData().clear();
    paxTypeFare.needRecalculateCat12() = true;
    _farePath.decreaseTotalNUCAmount(fareUsage.surchargeAmt());
    _farePath.plusUpAmount() -= fareUsage.surchargeAmt();
    fareUsage.surchargeData().clear();
    fareUsage.surchargeAmt() = 0.0;
  }

  void revertCat12Surcharges()
  {
    std::vector<SavedSurcharges>::const_iterator surchargesIt = _cat12Surcharges.begin();
    for (PricingUnit* pricingUnit : _farePath.pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
        paxTypeFare->setCategoryProcessed(RuleConst::SURCHARGE_RULE, true);
        paxTypeFare->setCategorySoftPassed(RuleConst::SURCHARGE_RULE, true);
        paxTypeFare->needRecalculateCat12() = false;
        paxTypeFare->surchargeData() = (*surchargesIt)._surchargeDataPaxTypeFare;
        fareUsage->surchargeData() = (*surchargesIt)._surchargeDataFareUsage;
        fareUsage->surchargeAmt() = (*surchargesIt)._surchargeAmt;
        _farePath.increaseTotalNUCAmount(fareUsage->surchargeAmt());
        _farePath.plusUpAmount() += fareUsage->surchargeAmt();
        ++surchargesIt;
      }
    }
  }

  std::vector<SavedSurcharges> _cat12Surcharges;
  FarePath& _farePath;
};

bool
validateSurcharges(PricingTrx& trx, FarePath& farePath)
{
  using PURuleController = RuleControllerWithChancelor<PricingUnitRuleController>;
  std::vector<uint16_t> categories(1, RuleConst::SURCHARGE_RULE);
  PURuleController ruleController(FPRuleValidation, categories, &trx);

  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      VectorSwapper<TravelSeg*> travelSegSwapper(
          fareUsage->travelSeg(), fareUsage->paxTypeFare()->fareMarket()->travelSeg());
      if (!ruleController.validate(trx, farePath, *pricingUnit, *fareUsage))
        return false;
    }
  }
  return true;
}
}

template <typename D>
void
SurchargesValidator<D>::applySurcharges(FarePath& farePath)
{
  RuleValidationResultsGuard guard(farePath);
  bool ok = validateSurcharges(_trx, farePath) && RuleUtil::getSurcharges(_trx, farePath);

  if (fallback::validateSurchargesMotherSolution(&_trx))
    ok = ok && PricingUtil::finalPricingProcess(_trx, farePath);

  if (!ok)
  {
    guard.rollback();
    _diagnostic.printSurchargesNotApplied();
  }
}

template <typename D>
void
SurchargesValidator<D>::applySurcharges(const std::vector<FarePath*>& farePathVec)
{
  for (FarePath* farePath : farePathVec)
    applySurcharges(*farePath);
}
template class SurchargesValidator<DiagnosticWrapper>;
template class SurchargesValidator<NoDiagnostic>;
}
}
