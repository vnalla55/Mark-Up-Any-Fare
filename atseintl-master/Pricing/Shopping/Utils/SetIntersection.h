//-------------------------------------------------------------------
//
//  Authors:     Michal Mlynek, Piotr Bartosik
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

#include <boost/assign/std/set.hpp>
#include <boost/utility.hpp>

#include <algorithm>
#include <set>

namespace tse
{

namespace utils
{

// Represents an intersection of a number of sets
template <typename T>
class SetIntersection : boost::noncopyable
{
public:
  SetIntersection() : _initialized(false) {}

  // Adds a new set for the intersection
  // For the first added set, the resulting intersection
  // is the added set itself
  // For next sets, the resulting intersection is updated
  // so that only the common elements remain
  void addSet(const std::set<T>& toBeIntersected)
  {
    if (!_initialized)
    {
      _intersection = toBeIntersected;
      _initialized = true;
    }
    else
    {
      std::set_intersection(_intersection.begin(),
                            _intersection.end(),
                            toBeIntersected.begin(),
                            toBeIntersected.end(),
                            std::inserter(_tmp, _tmp.begin()));
      _intersection.swap(_tmp);
      _tmp.clear();
    }
  }

  // Returns the stored intersection
  const std::set<T>& get() const { return _intersection; }

private:
  std::set<T> _intersection;
  std::set<T> _tmp;
  bool _initialized;
};

} // namespace utils

} // namespace tse

