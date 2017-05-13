//------------------------------------------------------------------
//
//  File: FlexFaresValidationPolicy.cpp
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

#include "Rules/FlexFaresValidationPolicy.h"

#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/PricingTrx.h"

namespace tse
{

FlexFaresValidationPolicy::FlexFaresValidationPolicy() : _contextType() {}

bool
FlexFaresValidationPolicy::shouldPerform(const RuleValidationContext& context)
{
  _contextType = context._contextType;

  if (context._trx->getTrxType() == PricingTrx::MIP_TRX)
    return checkGroupId(context._trx, context._groupId);
  return false;
}

bool
FlexFaresValidationPolicy::shouldReturn()
{
  return _contextType == RuleValidationContext::PU_FP;
}

bool
FlexFaresValidationPolicyNoEligibility::checkGroupId(const PricingTrx* trx,
                                                     const flexFares::GroupId groupId) const
{
  const bool isFareMarketContext = _contextType == RuleValidationContext::FARE_MARKET;

  if (!isFareMarketContext &&
      (!trx->getRequest()->getFlexFaresGroupsData().getCorpIds(groupId).empty() ||
       !trx->getRequest()->getFlexFaresGroupsData().getAccCodes(groupId).empty()))
    return true;

  if (isFareMarketContext &&
      (trx->getFlexFaresTotalAttrs().isValidationNeeded<flexFares::CORP_IDS>() ||
       trx->getFlexFaresTotalAttrs().isValidationNeeded<flexFares::ACC_CODES>()))
    return true;

  return false;
}

bool
FlexFaresValidationPolicyNoPenalties::checkGroupId(const PricingTrx* trx,
                                                   const flexFares::GroupId groupId) const
{
  const bool isFareMarketContext = _contextType == RuleValidationContext::FARE_MARKET;

  if (!isFareMarketContext &&
      trx->getRequest()->getFlexFaresGroupsData().isNoPenaltiesRequired(groupId))
    return true;

  if (isFareMarketContext &&
      trx->getFlexFaresTotalAttrs().isValidationNeeded<flexFares::NO_PENALTIES>())
    return true;

  return false;
}

bool
FlexFaresValidationPolicyNoMinMax::checkGroupId(const PricingTrx* trx,
                                                const flexFares::GroupId groupId) const
{
  const bool isFareMarketContext = _contextType == RuleValidationContext::FARE_MARKET;

  if (!isFareMarketContext &&
      trx->getRequest()->getFlexFaresGroupsData().isNoMinMaxStayRequired(groupId))
    return true;

  if (isFareMarketContext &&
      trx->getFlexFaresTotalAttrs().isValidationNeeded<flexFares::NO_MIN_MAX_STAY>())
    return true;

  return false;
}

bool
FlexFaresValidationPolicyNoAdvancePurchase::checkGroupId(const PricingTrx* trx,
                                                         const flexFares::GroupId groupId) const
{
  const bool isFareMarketContext = _contextType == RuleValidationContext::FARE_MARKET;

  if (!isFareMarketContext &&
      trx->getRequest()->getFlexFaresGroupsData().isNoAdvancePurchaseRequired(groupId))
    return true;

  if (isFareMarketContext &&
      trx->getFlexFaresTotalAttrs().isValidationNeeded<flexFares::NO_ADVANCE_PURCHASE>())
    return true;

  return false;
}

} // namespace tse
