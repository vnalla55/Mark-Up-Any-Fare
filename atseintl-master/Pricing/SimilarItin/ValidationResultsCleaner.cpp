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
#include "Pricing/SimilarItin/ValidationResultsCleaner.h"

#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace
{
void
clearStopoverCharges(FarePath& farePath, FareUsage& fareUsage)
{
  farePath.decreaseTotalNUCAmount(fareUsage.stopOverAmt());
  fareUsage.stopoverMinTime().set(-1, ' ');
  fareUsage.stopOverAmt() = 0.0;
  fareUsage.stopOverAmtUnconverted() = 0.0;
  fareUsage.stopoverSurcharges().clear();
  fareUsage.stopovers().clear();
  fareUsage.stopoverByDir().clear();
}

void
clearTransferCharges(FarePath& farePath, FareUsage& fareUsage)
{
  farePath.decreaseTotalNUCAmount(fareUsage.transferAmt());
  fareUsage.transferAmt() = 0.0;
  fareUsage.transferAmtUnconverted() = 0.0;
  fareUsage.transferSurcharges().clear();
  fareUsage.transfers().clear();
}
}

void
ValidationResultsCleaner::clearFuRuleValidationResults(const uint16_t category, FarePath& farePath)
{
  switch (category)
  {
  case RuleConst::STOPOVER_RULE:
    clearStopoverCharges(farePath, _fareUsage);
    break;
  case RuleConst::TRANSFER_RULE:
    clearTransferCharges(farePath, _fareUsage);
    break;
  }
}

void
ValidationResultsCleaner::clearCat12Surcharges(FarePath& farePath)
{
  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      PaxTypeFare& paxTypeFare = *fareUsage->paxTypeFare();
      paxTypeFare.setCategoryProcessed(RuleConst::SURCHARGE_RULE, false);
      paxTypeFare.setCategorySoftPassed(RuleConst::SURCHARGE_RULE, false);
      paxTypeFare.surchargeData().clear();
      paxTypeFare.needRecalculateCat12() = true;
      farePath.decreaseTotalNUCAmount(fareUsage->surchargeAmt());
      farePath.plusUpAmount() -= fareUsage->surchargeAmt();
      fareUsage->surchargeData().clear();
      fareUsage->surchargeAmt() = 0.0;
    }
  }
}

void
ValidationResultsCleaner::clearFpRuleValidationResults(const std::vector<uint16_t>& categories,
                                                       FarePath& fp)
{
  for (uint16_t category : categories)
  {
    switch (category)
    {
    case RuleConst::TRANSFER_RULE:
      for (PricingUnit* pu : fp.pricingUnit())
      {
        pu->totalTransfers() = 0;
        pu->mostRestrictiveMaxTransfer() = -1;
      }
      break;
    }
  }
}

bool
ValidationResultsCleaner::needRevalidation(const std::vector<uint16_t>& catSequence,
                                           FarePath& farePath)
{
  bool needRevalidation = false;
  const PaxTypeFare& paxTypeFare = *_fareUsage.paxTypeFare();
  for (uint16_t category : catSequence)
  {
    if (!paxTypeFare.isFareRuleDataApplicableForSimilarItin(category))
      continue;

    _guard.addCategoryToRevert(category);
    clearFuRuleValidationResults(category, farePath);
    needRevalidation = true;
  }
  return needRevalidation;
}

void
ValidationResultsCleaner::CategoriesReverterGuard::addCategoryToRevert(uint16_t category)
{
  categoriesWithStatus.push_back({category, paxTypeFare.isCategoryProcessed(category)});
  paxTypeFare.setCategoryProcessed(category, false);
}

ValidationResultsCleaner::CategoriesReverterGuard::~CategoriesReverterGuard()
{
  for (auto categoryWithStatus : categoriesWithStatus)
    paxTypeFare.setCategoryProcessed(categoryWithStatus.first, categoryWithStatus.second);
}
}
