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

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

// A set of scheduling options (SOPs), each specified
// by the itinerary leg index legId and the schedule index sopId.
class SopCollection: boost::noncopyable
{
public:
  SopCollection(unsigned int legCount) :
    _legCount(legCount)
  {}

  // Adds a SOP with given legId and sopId to the collection.
  void addSop(unsigned int legId, unsigned int sopId)
  {
    _sops.insert(std::make_pair(legId, sopId));
  }

  // Tells if the collection contains SOP specified
  // by legId and sopId.
  bool containsSop(unsigned int legId, unsigned int sopId) const
  {
    return _sops.find(std::make_pair(legId, sopId)) != _sops.end();
  }

  // Returns the cardinality of the cartesian product
  // of disjoint sets "SOPs per leg". Each such set contains
  // all sops on a single leg.
  unsigned int getCartesianCardinality() const
  {
    if (_sops.empty())
    {
      return 0;
    }

    SopsCountByLeg countingMap;
    for (const auto& elem : _sops)
    {
      const unsigned int legId = elem.first;
      countingMap[legId] = countingMap[legId] + 1;
    }

    if (countingMap.size() != _legCount)
    {
      return 0;
    }

    unsigned int combinationsCount = 1;
    for (SopsCountByLeg::const_iterator it = countingMap.begin(); it != countingMap.end(); ++it)
    {
      combinationsCount = combinationsCount * (it->second);
    }
    return combinationsCount;
  }

private:
  typedef std::pair<unsigned int, unsigned int> SopId;
  typedef boost::unordered_set<SopId> Sops;
  typedef boost::unordered_map<unsigned int, unsigned int> SopsCountByLeg;
  Sops _sops;
  unsigned int _legCount;
};

} // namespace utils

} // namespace tse

