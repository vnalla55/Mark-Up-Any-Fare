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

#include <boost/unordered_map.hpp>

namespace tse
{

namespace swp
{

// Maintains mapping for convenient access to individual
// appraiser scores:
// { item -> { appraiser* -> score } }
template <typename AppraiserType>
class AppraiserScoreBank
{
public:
  typedef boost::unordered_map<const AppraiserType*, typename AppraiserType::Score> Map;

  // Set score for given item and appraiser
  // May overwrite score or create new item key
  // Throws if source is zero
  void setScore(const typename AppraiserType::Item& item,
                const AppraiserType* source,
                const typename AppraiserType::Score& score)
  {
    TSE_ASSERT(source != nullptr);
    _itemsMap[item][source] = score;
  }

  // Returns mapping { appraiser* -> score } for given item
  // Throws if there are no scores for given item
  const Map& getScoresForItem(const typename AppraiserType::Item& item) const
  {
    typename ItemsMap::const_iterator it = _itemsMap.find(item);
    TSE_ASSERT(it != _itemsMap.end());
    return it->second;
  }

  // Removes whole mapping { appraiser* -> score } for given item
  void removeItem(const typename AppraiserType::Item& item) { _itemsMap.erase(item); }

  // Returns the number of contained items
  unsigned int getItemsCount() const { return _itemsMap.size(); }

  // Returns the total number of contained scores
  // (for all items, from all appraisers)
  unsigned int getScoresCount() const
  {
    unsigned int ans = 0;
    for (typename ItemsMap::const_iterator it = _itemsMap.begin(); it != _itemsMap.end(); ++it)
    {
      ans += it->second.size();
    }
    return ans;
  }

private:
  typedef boost::unordered_map<typename AppraiserType::Item, Map> ItemsMap;
  ItemsMap _itemsMap;
};

} // namespace swp

} // namespace tse

