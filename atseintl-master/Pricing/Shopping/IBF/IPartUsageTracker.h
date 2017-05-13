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

#include <boost/unordered_set.hpp>

namespace tse
{

template <typename ItemType, typename PartType>
class IPartUsageTracker
{
public:
  typedef boost::unordered_set<ItemType> ItemSet;

  virtual const ItemSet& getItemsContainingPart(const PartType& part) const = 0;

  virtual ~IPartUsageTracker() {}
};


} // namespace tse

