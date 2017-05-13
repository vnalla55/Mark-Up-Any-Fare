
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
#include "Pricing/Shopping/IBF/IPartUsageTracker.h"
#include "Pricing/Shopping/Utils/CountingMultimap.h"

#include <set>

namespace tse
{

namespace utils
{

template <typename DeconstructorType, typename CountingEventPredicate>
class PartUsageTracker : public ICountingEventReceiver<typename DeconstructorType::ItemType>,
                         public IPartUsageTracker<typename DeconstructorType::ItemType, typename DeconstructorType::PartType>
{
public:
  typedef typename DeconstructorType::ItemType ItemType;
  typedef typename DeconstructorType::PartType PartType;
  typedef ICountingEventReceiver<ItemType> ImplICountingEventReceiver;
  typedef boost::unordered_set<ItemType> ItemSet;

  PartUsageTracker(CountingEventPredicate& predicate) : _receiver(nullptr), _cmap(predicate)
  {
    _cmap.setReceiver(this);
  }

  void setReceiver(ImplICountingEventReceiver* receiver)
  {
    TSE_ASSERT(receiver != nullptr);
    _receiver = receiver;
  }

  // Will throw on item duplicate if
  // same parts are produced
  void add(const ItemType& item)
  {
    _deconstructor.insertItem(item);
    const typename DeconstructorType::PartVector& parts = _deconstructor.getItemParts();
    for (auto& part : parts)
    {
      _cmap.add(part, item);
    }
    forwardEvents(true);
  }

  void remove(const ItemType& item)
  {
    _deconstructor.insertItem(item);
    const typename DeconstructorType::PartVector& parts = _deconstructor.getItemParts();
    for (auto& part : parts)
    {
      _cmap.remove(part, item);
    }
    forwardEvents(false);
  }

  const ItemSet& getItemsContainingPart(const PartType& part) const override
  {
    return _cmap.getValues(part);
  }

  void eventForValue(const ItemType& item, bool increased) override
  {
    _itemsWithEvent.insert(item);
  }

private:
  void forwardEvents(bool increased)
  {
    if (!_itemsWithEvent.empty())
    {
      TSE_ASSERT(_receiver != nullptr);
      for (const auto& elem : _itemsWithEvent)
      {
        _receiver->eventForValue(elem, increased);
      }
      _itemsWithEvent.clear();
    }
  }

  typedef CountingMultimap<PartType, ItemType, CountingEventPredicate> ImplCountingMultimap;
  ImplICountingEventReceiver* _receiver;
  ImplCountingMultimap _cmap;
  DeconstructorType _deconstructor;

  // Cache items with event and forward
  // all events only when _cmap is fully updated
  std::set<ItemType> _itemsWithEvent;
};

} // namespace utils

} // namespace tse

