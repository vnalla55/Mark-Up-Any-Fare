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

#ifndef DUMMY_BLACKBOARD_H
#define DUMMY_BLACKBOARD_H

#include "Pricing/Shopping/Swapper/IMapUpdater.h"

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace tse
{

namespace utils
{

template <typename ItemType, typename ScoreType>
class DummyBlackboard : public swp::IMapUpdater<ItemType, ScoreType>
{
public:
  typedef std::vector<std::string> Log;
  void updateValue(const ItemType& key, const ScoreType& value) override
  {
    std::ostringstream out;
    out << "{" << key << ":" << value << "}";
    _log.push_back(out.str());
  }

  std::string getLog()
  {
    Log myCopy = _log;
    std::sort(myCopy.begin(), myCopy.end());
    std::ostringstream out;
    for (unsigned int i = 0; i < myCopy.size(); ++i)
    {
      out << myCopy[i];
    }
    clearLog();
    return out.str();
  }

  void clearLog() { _log.clear(); }

private:
  Log _log;
};

} // namespace utils

} // namespace tse

#endif // DUMMY_BLACKBOARD_H
