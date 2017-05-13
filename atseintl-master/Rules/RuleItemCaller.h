#pragma once

#include "DBAccess/Record2Types.h"

#include <vector>

namespace tse
{
class CategoryRuleItemInfo;
class PaxTypeFare;
class PricingTrx;
class RuleItem;
class RuleItemInfo;

struct RuleItemCaller
{
  RuleItemCaller(PricingTrx& trx,
                 const CategoryRuleInfo& cri,
                 const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                 PaxTypeFare& paxTypeFare,
                 bool& isCat15Security,
                 RuleItem& ruleItem,
                 bool skipCat15Security)
    : _trx(trx),
      _cri(cri),
      _cfrItemSet(cfrItemSet),
      _paxTypeFare(paxTypeFare),
      _isCat15Security(isCat15Security),
      _ruleItem(ruleItem),
      _skipCat15Security(skipCat15Security)
  {
  }

  virtual ~RuleItemCaller() {}

  PricingTrx& trx() const { return _trx; }
  const CategoryRuleInfo& record2() const { return _cri; }
  const std::vector<CategoryRuleItemInfo>& r2Segments() const { return _cfrItemSet; }

  virtual Record3ReturnTypes operator()(CategoryRuleItemInfo*) const = 0;
  virtual Record3ReturnTypes operator()(CategoryRuleItemInfo*, bool) const = 0;
  virtual Record3ReturnTypes operator()(CategoryRuleItemInfo*, const RuleItemInfo*) const = 0;
  virtual Record3ReturnTypes
  isDirectionPass(const CategoryRuleItemInfo* cfrItem, bool isLocationSwapped) const = 0;
  virtual Record3ReturnTypes
  isR8DirectionPass(Indicator directionality, bool isR8LocationSwapped) const = 0;

  virtual Record3ReturnTypes validateT994DateOverride(const RuleItemInfo* r3, uint32_t catNo) const = 0;

protected:
  PricingTrx& _trx;
  const CategoryRuleInfo& _cri;
  const std::vector<CategoryRuleItemInfo>& _cfrItemSet;
  PaxTypeFare& _paxTypeFare;
  bool& _isCat15Security;
  RuleItem& _ruleItem;
  bool _skipCat15Security;
};

}
