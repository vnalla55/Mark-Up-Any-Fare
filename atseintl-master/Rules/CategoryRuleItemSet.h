//-------------------------------------------------------------------
//
//  File:	CategoryRuleItemSet.h
//  Authors:	Devpariya SenGupta
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/Record2Types.h"
#include "Rules/CategoryRuleItem.h"

#include <vector>

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class Itin;
class FarePath;
class PricingUnit;
class FareUsage;
class RuleProcessingData;
class RuleControllerDataAccess;

/**
 * @class CategoryRuleItemSet
 * Defines a set of Record 2 segments
 *
 */
class CategoryRuleItemSet
{

public:
  CategoryRuleItemSet() : _categoryPhase(NormalValidation) {}
  CategoryRuleItemSet(CategoryPhase phase) : _categoryRuleItem(phase), _categoryPhase(phase)
  {
    setCategoryPhase(phase);
  }

  virtual ~CategoryRuleItemSet() {}

  Record3ReturnTypes process(PricingTrx&,
                             Itin&,
                             const CategoryRuleInfo&,
                             PaxTypeFare&,
                             const std::vector<CategoryRuleItemInfoSet*>&,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& rcda);

  Record3ReturnTypes process(PricingTrx&,
                             const CategoryRuleInfo&,
                             FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfoSet*>&,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& rcda);

  Record3ReturnTypes process(PricingTrx&,
                             const CategoryRuleInfo&,
                             const Itin* itin,
                             const PricingUnit& pricingUnit,
                             FareUsage& fareUsage,
                             const std::vector<CategoryRuleItemInfoSet*>&,
                             RuleProcessingData& rpData,
                             bool isLocationSwapped,
                             bool isFareRule,
                             bool skipCat15Security,
                             RuleControllerDataAccess& rcda);

  CategoryRuleItem& categoryRuleItem() { return _categoryRuleItem; }
  Record3ReturnTypes processQualifyCat4(PricingTrx&,
                                        const CategoryRuleInfo&,
                                        const std::vector<CategoryRuleItemInfoSet*>&,
                                        PaxTypeFare&);

  Record3ReturnTypes processFareGroup(PricingTrx&,
                                      const CategoryRuleInfo&,
                                      PaxTypeFare&,
                                      const std::vector<CategoryRuleItemInfoSet*>&);

  void setCategoryPhase(CategoryPhase phase)
  {
    _categoryPhase = phase;
    switch(_categoryPhase)
    {
    case FCORuleValidation:
    case FCORuleValidationMIPALT:
      _categoryRuleItem.ruleItem().setPhase(RuleItem::FCOPhase);
      break;
    case DynamicValidation:
      _categoryRuleItem.ruleItem().setPhase(RuleItem::DynamicValidationPhase);
      break;
    default:
      _categoryRuleItem.ruleItem().setPhase(RuleItem::UnKnown);
    }
  }

private:
  void setWebFare(PricingTrx&,
                  const CategoryRuleInfo&,
                  PaxTypeFare&,
                  const std::vector<CategoryRuleItemInfoSet*>&,
                  bool reuseCheck = false);

  CategoryRuleItem _categoryRuleItem;
  CategoryPhase _categoryPhase;
};
}

