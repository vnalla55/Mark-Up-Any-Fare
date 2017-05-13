

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

#ifndef NUMBERS_ABOVE_X_H
#define NUMBERS_ABOVE_X_H

#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

#include <set>
#include <sstream>

namespace tse
{

namespace swp
{

class NumbersAboveX : public IAppraiser<int, BasicAppraiserScore>
{
public:
  NumbersAboveX(int threshold, unsigned int targetCount) : _threshold(threshold)
  {
    TSE_ASSERT(targetCount > 0);
    _targetCount = targetCount;
  }

  BasicAppraiserScore
  beforeItemAdded(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
    BasicAppraiserScore::CATEGORY cat = BasicAppraiserScore::IGNORE;
    if (item > _threshold)
    {
      const unsigned int n = _collected.size();
      if (n >= _targetCount)
      {
        if (n == _targetCount)
        {
          // We have enough
          rateCollectedItems(blackboard, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE));
        }
        cat = BasicAppraiserScore::NICE_TO_HAVE;
      }
      else
      {
        cat = BasicAppraiserScore::MUST_HAVE;
      }
      _collected.insert(item);
    }
    return BasicAppraiserScore(cat);
  }

  void
  beforeItemRemoved(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
    _collected.erase(item);
    if (_collected.size() == _targetCount)
    {
      // We have just enough
      rateCollectedItems(blackboard, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE));
    }
  }

  bool isSatisfied() const override
  {
    return _collected.size() >= _targetCount;
  }

  std::string toString() const override
  {
    std::ostringstream out;
    out << _targetCount << " numbers above " << _threshold;
    out << " (" << _collected.size() << " collected)";
    return out.str();
  }

private:
  void rateCollectedItems(IMapUpdater<int, BasicAppraiserScore>& blackboard,
                          const BasicAppraiserScore& score) const
  {
    for (std::set<int>::const_iterator it = _collected.begin(); it != _collected.end(); ++it)
    {
      blackboard.updateValue(*it, score);
    }
  }

  int _threshold;
  unsigned int _targetCount;
  std::set<int> _collected;
};

} // namespace swp

} // namespace tse

#endif // NUMBERS_ABOVE_X_H
