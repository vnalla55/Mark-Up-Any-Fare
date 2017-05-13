
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"

#include <boost/utility.hpp>

#include <sstream>

namespace tse
{

// If target count is not set, this appraiser is always
// satisfied. After target count is set, it is satisfied
// if collected at least target count RC online options.
class AllOnlinesForCarrier
    : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
      boost::noncopyable
{
public:
  AllOnlinesForCarrier(const CarrierCode& carrier, const ShoppingTrx& trx)
    : _carrier(carrier), _trx(trx), _targetCount(0), _collected(0)
  {
  }

  // Act greedy: do not let remove any RC Online even if
  // we have collected more than estimate (since estimate is
  // coarse) -- mark incoming RC online as "must have"
  // and never degrade.
  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    if (ShoppingUtil::isOnlineOptionForCarrier(_trx, item, _carrier))
    {
      ++_collected;
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::MUST_HAVE);
    }
    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::IGNORE);
  }

  void
  beforeItemRemoved(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    if (ShoppingUtil::isOnlineOptionForCarrier(_trx, item, _carrier))
    {
      --_collected;
    }
  }

  bool isSatisfied() const override
  {
    return (_collected >= _targetCount);
  }

  std::string toString() const override
  {
    std::ostringstream tmp;
    tmp << "Requesting carrier online options preferred ("
        << _collected << "/" << _targetCount << ", carrier ";
    tmp << getCarrierCode();
    tmp << ")";
    return tmp.str();
  }

  void setTargetCount(unsigned int targetCount) { _targetCount = targetCount; }

  unsigned int getCollectedOptionsCount() const { return _collected; }

  const CarrierCode& getCarrierCode() const { return _carrier; }

private:
  CarrierCode _carrier;
  const ShoppingTrx& _trx;
  unsigned int _targetCount;
  unsigned int _collected;
};

} // namespace tse

