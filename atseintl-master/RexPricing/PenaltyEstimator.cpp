//-------------------------------------------------------------------
//
//  File:        PenaltyEstimator.cpp
//  Created:     July, 2014
//  Authors:     Grzegorz Szczurek
//
//  Updates:
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
#include "RexPricing/PenaltyEstimator.h"

#include "Common/CurrencyUtil.h"
#include "Common/Logger.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"

#include <functional>

namespace tse
{

namespace
{
class FeeApplCompare : public std::unary_function<ProcessTagInfo*, bool>
{
public:

  FeeApplCompare(const Indicator& feeAppl) : _feeAppl(feeAppl)
  { }

  bool operator() (const ProcessTagInfo* pti) const
  {
    return _feeAppl == pti->record3()->feeAppl();
  }

  typedef ProcessTagInfo* argument_type;

private:
    const Indicator& _feeAppl;
};
}

Logger PenaltyEstimator::_logger("atseintl.RexPricing.PenaltyEstimator");

Indicator
PenaltyEstimator::determineFeeApplication(const ProcessTagPermutation* permutation) const
{
  LOG4CXX_DEBUG(_logger, "determine FeeApplication for permutation " << permutation->number());

  if(hasDifferentFeeApplication(permutation))
  {
    if(hasFeeApplication(permutation, HIGHEST_OF_ALL_FC))
      return HIGHEST_OF_ALL_FC;
    else if(hasFeeApplication(permutation, HIGHEST_FROM_CHANGED_PU_ADDS))
      return HIGHEST_FROM_CHANGED_PU_ADDS;
    else if(hasFeeApplication(permutation, HIGHEST_FROM_CHANGED_PU_ADDS))
      return HIGHEST_FROM_CHANGED_PU;
    else if(hasFeeApplication(permutation, HIGHEST_OF_CHANGED_FC))
      return HIGHEST_OF_CHANGED_FC;
    else
      return EACH_OF_CHANGED_FC;
 }
 else
 {
   return permutation->processTags().front()->record3()->feeAppl();
 }
}

MoneyAmount
PenaltyEstimator::collectFee(const ProcessTagPermutation* permutation, Indicator feeApplication) const
{
  switch(feeApplication)
  {
  case HIGHEST_OF_CHANGED_FC:
    return highestOfChangedFCs(permutation);
  case HIGHEST_OF_ALL_FC:
  case HIGHEST_FROM_CHANGED_PU:
  case HIGHEST_FROM_CHANGED_PU_ADDS:
    return highestOfAllFCs(permutation);
  case EACH_OF_CHANGED_FC:
    return eachOfChangedFCs(permutation);
  default:
    LOG4CXX_ERROR(_logger, "Unknown fee application value: " << feeApplication);
    return MoneyAmount(0.0);
  }
}

bool
PenaltyEstimator::hasDifferentFeeApplication(const ProcessTagPermutation* permutation) const
{
  Indicator feeAppl = permutation->processTags().front()->record3()->feeAppl();

  return std::find_if(permutation->processTags().begin(), permutation->processTags().end(),
                      std::not1(FeeApplCompare(feeAppl))) != permutation->processTags().end() ;


}

bool
PenaltyEstimator::hasFeeApplication(const ProcessTagPermutation* permutation, Indicator feeAppl) const
{
  return std::find_if(permutation->processTags().begin(), permutation->processTags().end(),
                      FeeApplCompare(feeAppl)) != permutation->processTags().end();
}

MoneyAmount
PenaltyEstimator::highestOfAllFCs(const ProcessTagPermutation* permutation) const
{
  MoneyAmount highest = 0;
  for (const ProcessTagInfo* pti : permutation->processTags())
  {
    if(pti->record3()->penaltyAmt1() > pti->record3()->minAmt())
    {
      MoneyAmount penaltyAmt = getPenaltyAmount(pti);
      if(highest < penaltyAmt)
      {
        highest = penaltyAmt;
      }
    }
    else
    {
      MoneyAmount minAmt = getMinAmount(pti);
      if(highest < minAmt)
      {
        highest = minAmt;
      }
    }
  }
  LOG4CXX_DEBUG(_logger, "highestOfAllFCs: " << highest);
  return highest;
}

MoneyAmount
PenaltyEstimator::highestOfChangedFCs(const ProcessTagPermutation* permutation) const
{
  MoneyAmount highest = 0;
  for (const ProcessTagInfo* pti : permutation->processTags())
  {
    if(_unmatchedFCs.count(pti->fareCompNumber() - 1) == 0)
      continue;

    if(pti->record3()->penaltyAmt1() > pti->record3()->minAmt())
    {
      MoneyAmount penaltyAmt = getPenaltyAmount(pti);
      if(highest < penaltyAmt)
      {
       highest = penaltyAmt;
      }
    }
    else
    {
      MoneyAmount minAmt = getMinAmount(pti);
      if(highest < minAmt)
      {
        highest = minAmt;
      }
    }
  }
  LOG4CXX_DEBUG(_logger, "highestOfChangedFCs: " << highest);
  return highest;
}

MoneyAmount
PenaltyEstimator::eachOfChangedFCs(const ProcessTagPermutation* permutation) const
{
  MoneyAmount sum = 0;
  for (const ProcessTagInfo* pti : permutation->processTags())
  {
    if(pti->record3()->penaltyAmt1() > pti->record3()->minAmt())
    {
        sum += getPenaltyAmount(pti);
    }
    else
    {
        sum += getMinAmount(pti);
    }
  }
  LOG4CXX_DEBUG(_logger, "eachOfChangedFCs: " << sum);
  return sum;
}

MoneyAmount
PenaltyEstimator::getPenaltyAmount(const ProcessTagInfo* pti) const
{
  if(pti->record3()->cur1() != _paymentCurrency &&
      _paymentCurrency != NUC &&
     pti->record3()->minAmt() != 0.0)
  {
    return CurrencyUtil::convertMoneyAmount(pti->record3()->penaltyAmt1(),
                                                            pti->record3()->cur1(),
                                                            _paymentCurrency,
                                                            _trx);
  }

  return pti->record3()->penaltyAmt1();
}

MoneyAmount
PenaltyEstimator::getMinAmount(const ProcessTagInfo* pti) const
{
  if(pti->record3()->minCur() != _paymentCurrency &&
     _paymentCurrency != NUC &&
     pti->record3()->minAmt() != 0.0)
  {
    return CurrencyUtil::convertMoneyAmount(pti->record3()->minAmt(),
                                                            pti->record3()->minCur(),
                                                            _paymentCurrency,
                                                            _trx);
  }

  return pti->record3()->minAmt();
}

void
PenaltyEstimator::operator()(ProcessTagPermutation* perm)
{
  Indicator feeApplLhs = determineFeeApplication(perm);
  perm->setEstimatedChangeFee(collectFee(perm, feeApplLhs));
}

} //tse
