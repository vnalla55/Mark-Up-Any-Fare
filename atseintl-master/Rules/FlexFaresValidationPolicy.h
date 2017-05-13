//------------------------------------------------------------------
//
//  File: FlexFaresValidationPolicy.h
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Rules/RuleValidationContext.h"
#include "Rules/RuleValidationPolicy.h"

namespace tse
{

class PricingTrx;

class FlexFaresValidationPolicy : public RuleValidationPolicy
{
public:
  FlexFaresValidationPolicy();

  bool shouldPerform(const RuleValidationContext& context) override;
  bool shouldReturn() override;

protected:
  virtual bool checkGroupId(const PricingTrx* trx, const uint16_t groupId) const = 0;

  RuleValidationContext::ContextType _contextType;
};

class FlexFaresValidationPolicyNoEligibility : public FlexFaresValidationPolicy
{
private:
  bool checkGroupId(const PricingTrx* trx, const flexFares::GroupId groupId) const override;
};

class FlexFaresValidationPolicyNoPenalties : public FlexFaresValidationPolicy
{
private:
  bool checkGroupId(const PricingTrx* trx, const flexFares::GroupId groupId) const override;
};

class FlexFaresValidationPolicyNoMinMax : public FlexFaresValidationPolicy
{
private:
  bool checkGroupId(const PricingTrx* trx, const flexFares::GroupId groupId) const override;
};

class FlexFaresValidationPolicyNoAdvancePurchase : public FlexFaresValidationPolicy
{
private:
  bool checkGroupId(const PricingTrx* trx, const flexFares::GroupId groupId) const override;
};

} // namespace tse
