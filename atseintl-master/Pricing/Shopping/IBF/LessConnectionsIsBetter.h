//-------------------------------------------------------------------
//
//  Authors:     Grzegorz Szczurek
//
//  Copyright Sabre 2014
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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

namespace tse {

class LessConnectionsIsBetter
    : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
             boost::noncopyable
{
public:
  LessConnectionsIsBetter(const ShoppingTrx& trx) : _trx(trx), _targetCount(0),
                                                    _collected(0), _connectionCount(0)
  {
  }

  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override;

  void
  beforeItemRemoved(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override;

  std::string toString() const override;

  void setTargetCount(unsigned int targetCount) { _targetCount = targetCount; }


  bool isSatisfied() const override
  {
    return (_collected >= _targetCount);
  }

private:
  const ShoppingTrx& _trx;
  uint32_t _targetCount;
  uint32_t _collected;
  uint32_t _connectionCount;
};

} /* namespace tse */

