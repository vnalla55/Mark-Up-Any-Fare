//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Rules/CategoryValidationObserver.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/FlexFaresValidationPolicy.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleController.h"
#include "Rules/RuleValidationChancelor.h"
#include "Rules/RuleValidationContext.h"

#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>

namespace tse
{
namespace
{

template <class RC>
RuleValidationContext::ContextType
getContextType()
{
  TSE_ASSERT(false);
  return RuleValidationContext::FARE_MARKET;
}

template <>
RuleValidationContext::ContextType
getContextType<FareMarketRuleController>()
{
  return RuleValidationContext::FARE_MARKET;
}

template <>
RuleValidationContext::ContextType
getContextType<PricingUnitRuleController>()
{
  return RuleValidationContext::PU_FP;
}
}

// only RC(RuleController) argument is needed for production usage
// others are added for mocking purpose
template <class RC, class ChancelorClass = RuleValidationChancelor>
class RuleControllerWithChancelorBase : public RC
{
  BOOST_CONCEPT_ASSERT((boost::Convertible<ChancelorClass, RuleValidationChancelor>));

public:
  RuleControllerWithChancelorBase(const PricingTrx* trx = nullptr)
    : _isActive(checkIsActive(trx)), _chancelor(), _resultObserver()
  {
    initMembers();
    populateChancelor(_categoryRuleItemSet);
  }

  virtual ~RuleControllerWithChancelorBase() = default;

  RuleControllerWithChancelorBase(CategoryPhase categoryPhase, const PricingTrx* trx = nullptr)
    : RC(categoryPhase), _isActive(checkIsActive(trx)), _chancelor(), _resultObserver()
  {
    initMembers();
    populateChancelor(_categoryRuleItemSet);
  }

  RuleControllerWithChancelorBase(CategoryPhase categoryPhase,
                                  const std::vector<uint16_t>& categories,
                                  const PricingTrx* trx = nullptr)
    : RC(categoryPhase, categories), _isActive(checkIsActive(trx)), _chancelor(), _resultObserver()
  {
    initMembers();
    populateChancelor(_categoryRuleItemSet);
  }

  RuleControllerWithChancelorBase(const RC& rc)
    : RC(rc), _isActive(false), _chancelor(), _resultObserver()
  {
  }

  ChancelorClass& getMutableChancelor()
  {
    if (!_chancelor)
    {
      _chancelor = std::shared_ptr<ChancelorClass>(new ChancelorClass());
      initChancelor();
    }

    return *_chancelor;
  }

  Record3ReturnTypes
  callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                          const CategoryRuleInfo& catRuleInfo,
                          const std::vector<CategoryRuleItemInfoSet*>& catRuleItemInfoSetV,
                          RuleControllerDataAccess& da,
                          RuleProcessingData& rpData,
                          bool isLocationSwapped,
                          bool isFareRule,
                          bool skipCat15Security) override
  {
    if (UNLIKELY(_isActive))
    {
      getMutableChancelor().updateContext(da);
      populateChancelor(catRuleIS);
    }
    return RC::callCategoryRuleItemSet(catRuleIS,
                                       catRuleInfo,
                                       catRuleItemInfoSetV,
                                       da,
                                       rpData,
                                       isLocationSwapped,
                                       isFareRule,
                                       skipCat15Security);
  }

protected:
  void initChancelor()
  {
    _chancelor->updateContextType(getContextType<RC>());
    _chancelor->setPolicy(RuleConst::ELIGIBILITY_RULE,
                          new FlexFaresValidationPolicyNoEligibility());
    _chancelor->setPolicy(RuleConst::ADVANCE_RESERVATION_RULE,
                          new FlexFaresValidationPolicyNoAdvancePurchase());
    _chancelor->setPolicy(RuleConst::MINIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
    _chancelor->setPolicy(RuleConst::MAXIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
    _chancelor->setPolicy(RuleConst::PENALTIES_RULE, new FlexFaresValidationPolicyNoPenalties());
  }

  void initMembers()
  {
    if (LIKELY(!_isActive))
      return;

    _chancelor = std::shared_ptr<ChancelorClass>(new ChancelorClass());
    initChancelor();

    _resultObserver = std::shared_ptr<CategoryValidationObserver>(
        new CategoryValidationObserver(_chancelor->getMutableMonitor()));
  }

  void populateChancelor(CategoryRuleItemSet& catRuleItemSet)
  {
    if (LIKELY(!_isActive))
      return;

    std::shared_ptr<RuleValidationChancelor> chancelor(_chancelor);
    catRuleItemSet.categoryRuleItem().ruleItem().setChancelor(chancelor);
  }

  Record3ReturnTypes validateBaseFare(uint16_t category,
                                      const FareByRuleItemInfo* fbrItemInfo,
                                      bool& checkFare,
                                      PaxTypeFare* fbrBaseFare,
                                      RuleControllerDataAccess& da) override
  {
    if (UNLIKELY(_isActive))
      getMutableChancelor().updateContext(da);

    return RC::validateBaseFare(category, fbrItemInfo, checkFare, fbrBaseFare, da);
  }

  bool checkIsActive(const PricingTrx* trx)
  {
    if (trx == nullptr)
      return false;

    return trx->getTrxType() == PricingTrx::MIP_TRX && trx->isFlexFare();
  }

  void setFlexFaresGroupId(const uint16_t& id) { getMutableChancelor().updateContextGroupId(id); }

  using RC::_categoryRuleItemSet;

  bool _isActive;
  std::shared_ptr<ChancelorClass> _chancelor;
  std::shared_ptr<CategoryValidationObserver> _resultObserver;
};

template <class RC, class ChancelorClass = RuleValidationChancelor>
class RuleControllerWithChancelor : public RuleControllerWithChancelorBase<RC, ChancelorClass>
{
  using ParentClass = RuleControllerWithChancelorBase<RC, ChancelorClass>;

public:
  RuleControllerWithChancelor(const PricingTrx* trx = nullptr) : ParentClass(trx) {}

  RuleControllerWithChancelor(CategoryPhase categoryPhase, const PricingTrx* trx = nullptr)
    : ParentClass(categoryPhase, trx)
  {
  }

  RuleControllerWithChancelor(CategoryPhase categoryPhase,
                              const std::vector<uint16_t>& categories,
                              const PricingTrx* trx = nullptr)
    : ParentClass(categoryPhase, categories, trx)
  {
  }

  RuleControllerWithChancelor(const RC& rc) : ParentClass(rc) {}
};

template <class ChancelorClass>
class RuleControllerWithChancelor<
    PricingUnitRuleController,
    ChancelorClass> : public RuleControllerWithChancelorBase<PricingUnitRuleController,
                                                             ChancelorClass>
{
  using ParentClass = RuleControllerWithChancelorBase<PricingUnitRuleController, ChancelorClass>;

public:
  RuleControllerWithChancelor(const PricingTrx* trx = nullptr) : ParentClass(trx) {}

  RuleControllerWithChancelor(CategoryPhase categoryPhase, const PricingTrx* trx = nullptr)
    : ParentClass(categoryPhase, trx)
  {
  }

  RuleControllerWithChancelor(CategoryPhase categoryPhase,
                              const std::vector<uint16_t>& categories,
                              const PricingTrx* trx = nullptr)
    : ParentClass(categoryPhase, categories, trx)
  {
  }

  RuleControllerWithChancelor(const PricingUnitRuleController& rc) : ParentClass(rc) {}

  // wrap validate methods to update groupId in PU or FP context
  bool validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit) override
  {
    if (UNLIKELY(_isActive))
      ParentClass::getMutableChancelor().updateContextGroupId(farePath.getFlexFaresGroupId());

    return ParentClass::validate(trx, farePath, pricingUnit);
  }

  bool validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit, FareUsage& fareUsage)
      override
  {
    if (UNLIKELY(_isActive))
      ParentClass::getMutableChancelor().updateContextGroupId(farePath.getFlexFaresGroupId());

    return ParentClass::validate(trx, farePath, pricingUnit, fareUsage);
  }

  bool
  validate(PricingTrx& trx, PricingUnit& pricingUnit, FareUsage& fareUsage, Itin& itin) override
  {
    if (UNLIKELY(_isActive))
      ParentClass::getMutableChancelor().updateContextGroupId(pricingUnit.getFlexFaresGroupId());

    return ParentClass::validate(trx, pricingUnit, fareUsage, itin);
  }

  bool validate(PricingTrx& trx, PricingUnit& pricingUnit, Itin& itin) override
  {
    if (_isActive)
      ParentClass::getMutableChancelor().updateContextGroupId(pricingUnit.getFlexFaresGroupId());

    return ParentClass::validate(trx, pricingUnit, itin);
  }

  bool validate(PricingTrx& trx, PricingUnit& pricingUnit, FareUsage*& failedFareUsage, Itin& itin)
      override
  {
    if (UNLIKELY(_isActive))
      ParentClass::getMutableChancelor().updateContextGroupId(pricingUnit.getFlexFaresGroupId());

    return ParentClass::validate(trx, pricingUnit, failedFareUsage, itin);
  }

protected:
  using ParentClass::_isActive;
};
}

