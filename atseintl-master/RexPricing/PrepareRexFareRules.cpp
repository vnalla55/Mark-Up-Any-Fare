//-------------------------------------------------------------------
//
//  File:
//  Created:     May 24, 2007
//  Authors:     Simon Li
//
//  Updates:
//
//  Copyright Sabre 2007
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "RexPricing/PrepareRexFareRules.h"

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TSEException.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseTrx.h"
#include "DBAccess/DataHandle.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.PrepareRexFareRules");

void
PrepareRexFareRules::process(PaxTypeFare* ptf)
{
  if (!ptf || !ptf->isCategoryValid(_subjectCategory))
    return;

  // when found matched cat31 record3, cat31 will be set as valid
  // preset as invalid now
  ptf->setCategoryValid(_subjectCategory, false);
  PaxTypeFare* baseFare = nullptr;
  bool oldBaseFareStat = true;
  if (ptf->isFareByRule())
  {
    try
    {
      baseFare = ptf->baseFare();
      if (baseFare)
      {
        oldBaseFareStat = baseFare->isCategoryValid(_subjectCategory);
        baseFare->setCategoryValid(_subjectCategory, false);
      }
    }
    catch (...) {};
  }

  skipRulesOnExcItin(*ptf);

  PricingTrx& trx = static_cast<PricingTrx&>(_rexTrx);
  Itin& itin = *(_rexTrx.exchangeItin().front());

  _ruleController->validate(trx, itin, *ptf);

  if (baseFare)
  {
    if (baseFare->isCategoryValid(_subjectCategory))
      ptf->setCategoryValid(_subjectCategory, true);
    else if (oldBaseFareStat)
    {
      // recover the original stat
      baseFare->setCategoryValid(_subjectCategory, true);
    }

    // force baseFare cat31 set as SoftPass
    baseFare->setCategorySoftPassed(_subjectCategory, true);
  }

  if (ptf->isCategoryValid(_subjectCategory))
  {
    // force cat31 set as SoftPass
    ptf->setCategorySoftPassed(_subjectCategory, true);

    if (ptf->rec2Cat10() == nullptr)
    {
      bool isLocationSwapped = false;

      ptf->rec2Cat10() = RuleUtil::getCombinabilityRuleInfo(trx, *ptf, isLocationSwapped);
    }
  }
}

bool
PrepareRexFareRules::process()
{
  ExcItin& excItin = *(_rexTrx.exchangeItin().front());

  // When reprice OK, we should have one FarePath in the itin
  if (excItin.farePath().empty())
  {
    LOG4CXX_ERROR(logger, "PrepareRexFareRules::process() - exchange Itin could not be re-priced");
    return false;
  }

  const FarePath& excFP = *excItin.farePath().front();

  std::vector<PricingUnit*>::const_iterator puIter = excFP.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd = excFP.pricingUnit().end();
  for (; puIter != puIterEnd; puIter++)
  {
    PricingUnit& pu = **puIter;
    FareUsage* failedFareUsage = nullptr;

    bool valid = ((PricingUnitRuleController*)_ruleController)
                     ->validate(_rexTrx, pu, failedFareUsage, excItin);

    if (!valid)
      return false;
  }

  return true;
}

// For repricing old itin, on most categories we do not need to validate
void
PrepareRexFareRules::skipRulesOnExcItin(PaxTypeFare& ptf) const
{
  ptf.setCategoryProcessed(RuleConst::DAY_TIME_RULE, true);
  ptf.setCategoryProcessed(RuleConst::SEASONAL_RULE, true);
  ptf.setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, true);
  ptf.setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, true);
  ptf.setCategoryProcessed(RuleConst::MAXIMUM_STAY_RULE, true);
  // ptf.setCategoryProcessed(RuleConst::STOPOVER_RULE, true);
  // ptf.setCategoryProcessed(RuleConst::TRANSFER_RULE, true);
  ptf.setCategoryProcessed(RuleConst::BLACKOUTS_RULE, true);
  // ptf.setCategoryProcessed(RuleConst::SURCHARGE_RULE, true);
  ptf.setCategoryProcessed(RuleConst::ACCOMPANIED_PSG_RULE, true);
  ptf.setCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE, true);
  if (_rexTrx.isExcNonRefInRequest())
  {
    ptf.setCategoryProcessed(RuleConst::PENALTIES_RULE, true);
  }
  ptf.setCategoryProcessed(RuleConst::HIP_RULE, true);
  ptf.setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, true);
  ptf.setCategoryProcessed(RuleConst::CHILDREN_DISCOUNT_RULE, true);
  ptf.setCategoryProcessed(RuleConst::TOUR_DISCOUNT_RULE, true);
  ptf.setCategoryProcessed(RuleConst::AGENTS_DISCOUNT_RULE, true);
  ptf.setCategoryProcessed(RuleConst::OTHER_DISCOUNT_RULE, true);
  ptf.setCategoryProcessed(RuleConst::MISC_FARE_TAG, true);
}
}
