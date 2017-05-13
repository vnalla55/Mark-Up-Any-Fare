/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Pricing/SimilarItin/Context.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <cstdint>
#include <vector>

namespace tse
{
class Combinations;
class FPPQItem;
class FarePath;
class FareUsage;
class PUPath;
class PricingTrx;
class PricingUnit;

namespace similaritin
{
template <typename D>
class Revalidator
{
  friend class RoutingRevalidatorTest;
  struct RevalidatorConfig
  {
    explicit RevalidatorConfig(const PricingTrx& trx);

    std::vector<uint16_t> categories;
    bool routing;
    bool combinability106;
    bool combinabilityAll;
  };

public:
  Revalidator(const Context&, D& diag);
  bool validate(const std::vector<FarePath*>& farePathVec, const std::vector<FPPQItem*>& gfp, Itin& est);
  bool validateFbr(const PaxTypeFare& ptf);

private:
  using PURuleController = RuleControllerWithChancelor<PricingUnitRuleController>;

  bool validateRule(const std::vector<FarePath*>& farePathVec, uint16_t& category) const;
  bool validateFarePathRule(FarePath* farePath,
                            PricingUnit* pricingUnit,
                            FareUsage* fareUsage,
                            PURuleController& ruleController) const;
  bool validateRouting(const std::vector<FarePath*>& farePathVec) const;
  bool validateCombinability(const std::vector<FarePath*>& farePathVec,
                             const std::vector<FPPQItem*>& gfp,
                             Itin& est);
  bool validateCombinabilityPricingUnit(Combinations* combinations,
                                        const std::vector<FarePath*>& farePathVec,
                                        Itin& est);
  bool validateCombinability106(const std::vector<FarePath*>& farePathVec);
  bool validateCombinabilityAll(const std::vector<FarePath*>& farePathVec,
                                const std::vector<PUPath*>& puPathVec,
                                Itin& est);

  D& _diagnostic;
  Context _context;
  RevalidatorConfig _config;
};
}
}
