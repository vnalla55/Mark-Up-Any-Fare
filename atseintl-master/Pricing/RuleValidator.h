/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include "Pricing/PUPQItem.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <vector>

namespace tse
{
class FarePathFactoryFailedPricingUnits;
class FPPQItem;
class PricingTrx;
class PricingUnitFactory;

class RuleValidator
{
public:
  RuleValidator(PricingTrx& trx,
                const std::vector<PricingUnitFactory*>& allPUF,
                RuleControllerWithChancelor<PricingUnitRuleController>& ruleController,
                FarePathFactoryFailedPricingUnits& failedPricingUnits,
                DiagCollector& diag)
    : _trx(trx),
      _allPUF(allPUF),
      _ruleController(ruleController),
      _failedPricingUnits(failedPricingUnits),
      _diag(diag)
  {
  }

  bool validate(FPPQItem& fppqItem, bool validatePULevel);
  bool checkPULevelRuleValidation(FPPQItem& fppqItem, const bool validatePULevel);

private:
  PricingTrx& _trx;
  const std::vector<PricingUnitFactory*>& _allPUF;
  RuleControllerWithChancelor<PricingUnitRuleController>& _ruleController;
  FarePathFactoryFailedPricingUnits& _failedPricingUnits;
  DiagCollector& _diag;

  void setPUScopeRuleReValStatus(const FPPQItem& fppqItem,
                                 const uint16_t puFactIdx,
                                 PUPQItem::PUValidationStatus status);
  void showPricingUnitDiag(const PricingUnit& prU) const;
  void showFarePathDiag(FPPQItem& fppqItem);
  const bool ignoreRuleForKeepFare(const PricingUnit& prU) const;

  PUPQItem::PUValidationStatus
  getPUScopeRuleReValStatus(const FPPQItem& fppqItem, const uint16_t puFactIdx, PricingUnit& pu);
  void saveFailedFare(const uint16_t puFactIdx,
                      FareUsage* failedFareUsage1,
                      FareUsage* failedFareUsage2);
  void reuseSurchargeData(FPPQItem& fppqItem) const;
  bool allFaresProcessedSurcharges(FPPQItem& fppqItem) const;
  bool checkFPLevelRuleValidation(FPPQItem& fppqItem);
  void revalidateResultReusedFU(FPPQItem& fppqItem);
};
}
