//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "Common/Assert.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/SopCollection.h"

#include <boost/utility.hpp>

#include <memory>

namespace tse
{

// This appraiser is interested in placing the target number of direct options
// (containing only direct SOPs) in the result set.
// The user tells it what direct SOPs actually exist (addDirectSopForTracking)
// so the appraiser may verify if a particular option is a direct.
template <typename DirectSopCollection = utils::SopCollection>
class AllDirectOptionsRepresented
    : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
      boost::noncopyable
{
public:
  // Warning: we take ownership of directSops here.
  AllDirectOptionsRepresented(unsigned int legCount, DirectSopCollection* directSops = 0) :
      _targetCount(0), _collected(0)
  {
    if (directSops)
    {
      _directSops.reset(directSops);
    }
    else
    {
      _directSops.reset(new DirectSopCollection(legCount));
    }
  }

  // Inform this appraiser on existence of such direct SOP.
  void addDirectSopForTracking(unsigned int legId, unsigned int sopId)
  {
    _directSops->addSop(legId, sopId);
  }

  // Returns the cardinality of the cartesian product
  // of all direct SOPs across legs.
  unsigned int getAllSopCombinationsCount() const
  {
    return _directSops->getCartesianCardinality();
  }

  // Set the target number of direct options that
  // will satisfy this appraiser.
  void setTargetCount(unsigned int targetCount)
  {
    _targetCount = targetCount;
  }

  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    if (isOptionDirect(item))
    {
      ++_collected;
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::MUST_HAVE);
    }
    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::IGNORE);
  }

  void
  beforeItemRemoved(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    if (isOptionDirect(item))
    {
      --_collected;
    }
  }

  // Returns true iff the actual number of direct options in the result set
  // hit the target value.
  bool isSatisfied() const override
  {
    return (_collected >= _targetCount);
  }


  std::string toString() const override
  {
    std::ostringstream tmp;
    tmp << "Direct options preferred (";
    tmp << _collected;
    tmp << "/";
    tmp << _targetCount;
    tmp << ")";
    return tmp.str();
  }


private:

  bool isOptionDirect(const utils::SopCombination& item) const
  {
    for (unsigned int legId = 0; legId < item.size(); ++legId)
    {
      const unsigned int sopId = item[legId];
      if (!_directSops->containsSop(legId, sopId))
      {
        return false;
      }
    }
    return true;
  }

  unsigned int _targetCount;
  unsigned int _collected;
  std::unique_ptr<DirectSopCollection> _directSops;
};
} // namespace tse
