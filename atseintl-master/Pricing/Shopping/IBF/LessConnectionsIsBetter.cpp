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

#include "Pricing/Shopping/IBF/LessConnectionsIsBetter.h"

namespace tse
{

swp::BasicAppraiserScore
LessConnectionsIsBetter::beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard)
{
  uint32_t sumOfConnections = 0;
  for (unsigned int legId = 0; legId < item.size(); ++legId)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legId].sop()[item[legId]];

    if (sop.itin()->travelSeg().size() == 1)
    {
      //Combinations with direct flight not allowed
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::WANT_TO_REMOVE);
    }

    sumOfConnections += sop.itin()->travelSeg().size() - 1;
  }

  if (sumOfConnections == item.size())
  {
    ++_collected;
  }

  _connectionCount = sumOfConnections;

  return swp::BasicAppraiserScore(swp::BasicAppraiserScore::NICE_TO_HAVE, -sumOfConnections);
}

void
LessConnectionsIsBetter::beforeItemRemoved(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard)
{
  uint32_t sumOfConnections = 0;
  for (unsigned int legId = 0; legId < item.size(); ++legId)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legId].sop()[item[legId]];

    if (sop.itin()->travelSeg().size() == 1)
    {
      sumOfConnections = 0;
      break;
    }

    sumOfConnections += sop.itin()->travelSeg().size() - 1;
  }

  if (sumOfConnections == item.size())
  {
    --_collected;
  }
}

std::string
LessConnectionsIsBetter::toString() const
{
  std::ostringstream tmp;
  tmp << "Less connections options preferred ("
      << _collected << "/" << _targetCount << ", connections count "
      << _connectionCount
      << ")";
  return tmp.str();
}


}



