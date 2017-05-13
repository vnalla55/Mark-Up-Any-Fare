#pragma once

#include "Rules/FareMarketRuleController.h"

namespace tse
{

class FareByRuleItemInfo;

class FareByRuleBaseFareValidator : public FareMarketRuleController
{
public:
  FareByRuleBaseFareValidator();

  void setCat25R3(const FareByRuleItemInfo* fbrItemInfo);
  bool isFareValidInContext(const PaxTypeFare& paxTypeFare) const;
  Record3ReturnTypes getFareStatusInContext(const PaxTypeFare& paxTypeFare) const;

private:
  static const std::vector<uint16_t> _catListForUniqueFMs;
  bool _checkCat15 = false;
};

}
