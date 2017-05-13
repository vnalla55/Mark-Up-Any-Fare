#pragma once

#include "Rules/RuleControllerWithChancelor.h"

namespace tse
{

class ShoppingTrx;
class Itin;
class FarePath;

class PrepareFareCalcForShoppingTrx
{
public:
  PrepareFareCalcForShoppingTrx(ShoppingTrx& trx);
  virtual ~PrepareFareCalcForShoppingTrx();

  void process();

protected:
  ShoppingTrx& _trx;
  RuleControllerWithChancelor<PricingUnitRuleController> _ruleController;

private:
  Itin* prepareItin(FarePath* farePath, const SopIdVec& sops, const uint16_t& numerOfItin);
};
}
