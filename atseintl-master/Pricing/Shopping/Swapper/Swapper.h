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
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/IObservableItemSet.h"
#include "Pricing/Shopping/Swapper/PriorityScoreBuilder.h"
#include "Pricing/Shopping/Swapper/ScoredSet.h"
#include "Pricing/Shopping/Swapper/ScoringMediator.h"

#include <boost/unordered_set.hpp>

#include <iostream>
#include <vector>

namespace tse
{

namespace swp
{

// Swapper is a container holding scored items. Each item score is combined
// from individual scores coming from a number of appraisers added to the swapper.
//
// Swapper has a fixed capacity. If an item arives and the swapper
// is already full, the item with the worst score is removed (swapped).
//
// Using this mechanism, we can solve the problem of generating a set
// of items fulfilling different criteria (represented by appraisers).
// We refine the item set iteratively introducing new items
// and removing the worst ones until all appraisers are satisfied.
template <typename ItemType, typename ScoreBuilderType = PriorityScoreBuilder>
class Swapper: public IObservableItemSet<ItemType>, public ISwapperInfo
{
public:
  typedef typename ScoreBuilderType::AppraiserScore AppraiserScore;
  typedef typename ScoreBuilderType::ItemScore ItemScore;
  typedef typename ScoreBuilderType::AppraiserInfo AppraiserInfo;
  typedef typename ScoreBuilderType::DerivedInfo DerivedInfo;

  typedef IAppraiser<ItemType, AppraiserScore> Appraiser;

  typedef ScoredSet<ItemType, ItemScore, MinComparePolicy<ItemScore> > ImplScoredSet;
  typedef typename ImplScoredSet::Iterator Iterator;

  typedef ScoringMediator<ItemType, ScoreBuilderType> ImplScoringMediator;
  typedef typename ImplScoringMediator::AppraiserScoresMap AppraiserScoresMap;
  typedef std::pair<ItemType, AppraiserScoresMap> AddResponse;

  typedef boost::unordered_set<ItemType> ItemSet;
  typedef IItemSetObserver<ItemType> ImplItemSetObserver;

  // Sets the initial capacity
  Swapper(unsigned int capacity)
    : _mapUpdater(_scoredItems),
      _scoringMediator(_scoreBuilder, _mapUpdater),
      _noProgressIterationsCount(0),
      _totalIterationsCount(0)
  {
    TSE_ASSERT(capacity > 0);
    _capacity = capacity;
  }

  // Adds a new appraiser to this swapper.
  // Info contains information that identifies
  // a particular appraiser from the builder's point of view.
  // Throws if appraiser is zero
  // Throws if appraiser has been already added
  void addAppraiser(Appraiser* appraiser, const AppraiserInfo& info)
  {
    _scoringMediator.addAppraiser(appraiser, info);
  }

  // Adds a new item to this swapper's collection.
  // Each added appraiser gives the item a score,
  // the final item score is produced from these scores
  // and a tuple (item, item score) is added to the collection.
  //
  // If an item is added to a full swapper,
  // the item with the lowest score is removed
  // to not exceed capacity
  //
  // Returns an item with the following possible values:
  // - an "empty" object (ItemType()) if the capacity has not been exceeded
  // - an identical item as supplied if the item has not been added
  //   (the capacity was exceeded and the new item's score was lower
  //   than that of any item present in the swapper)
  // - a different item than supplied (but non-empty) if the item was added
  //   and a different item (with the lowest score) has been removed
  //
  // Throws if item is currently in the swapper
  // Throws on attempt of adding an empty item, equal to ItemType()
  // Throws if there are no added appraisers
  //
  // Note: if an item has been added in the past but swapped out
  // in the meantime, this object will not throw
  // (it does not track potentially infinite history of updates)
  AddResponse addItem(const ItemType& item)
  {
    TSE_ASSERT(!_scoredItems.hasElement(item));
    TSE_ASSERT(!(item == ItemType()));

    _totalIterationsCount += 1;
    const ItemScore score = _scoringMediator.beforeItemAdded(item);
    _scoredItems.add(item, score);

    if (!isOverloaded())
    {
      // There was room for a new item so it was added
      _noProgressIterationsCount = 0;
      notifyItemAdded(item);
      return AddResponse();
    }

    // Now the table is overloaded - we must remove
    // one item. We simply return the removed element.
    const ItemType worst = _scoredItems.top();
    if (item != worst)
    {
      _noProgressIterationsCount = 0;
    }
    else
    {
      ++_noProgressIterationsCount;
    }

    const AddResponse ar = std::make_pair(worst, _scoringMediator.getScoresForItem(worst));

    _scoringMediator.beforeItemRemoved(worst);
    _scoredItems.pop();
    TSE_ASSERT(!isOverloaded());

    // Make sure you notify after all appraisers
    // have been called
    if (item != worst)
    {
      notifyItemAdded(item);
      notifyItemRemoved(worst);
    }

    return ar;
  }

  // Returns the number of items contained in
  // this swapper. The size can never exceed
  // the swapper capacity.
  unsigned int getSize() const { return _scoredItems.getSize(); }

  // Returns this swapper's capacity (the number of items
  // that can be held by this swapper).
  unsigned int getCapacity() const { return _capacity; }

  // Tells how many times an item has been added to this swapper
  unsigned int getTotalIterationsCount() const override { return _totalIterationsCount; }

  // Tells how many last calls to addItem did not make
  // any progress. A call to addItem does not make any progress
  // only if the item is not added.
  unsigned int getNoProgressIterationsCount() const { return _noProgressIterationsCount; }

  void resetNoProgressIterationsCount()
  {
    _noProgressIterationsCount = 0;
  }

  // Returns an iterator to the first tuple (item, score)
  // of type HeapItem in this swapper (iteration is ordered
  // from the worst score to the best score).
  Iterator begin() const { return _scoredItems.begin(); }

  // Returns an iterator after the last tuple (item, score)
  // of type HeapItem in this swapper (iteration is ordered
  // from the worst score to the best score).
  Iterator end() const { return _scoredItems.end(); }

  // Returns a set of all items contained in this Swapper
  ItemSet getItems() const
  {
    ItemSet items;
    for (Iterator it = begin(); it != end(); ++it)
    {
      items.insert(it->key);
    }
    return items;
  }

  // Returns mapping { appraiser* -> appraiser score } for the given item
  // Throws if there is no such item in the swapper
  const AppraiserScoresMap& getAppraiserScoresForItem(const ItemType& item) const
  {
    return _scoringMediator.getScoresForItem(item);
  }

  // Returns info for a given appraiser
  // Throws if there is no such appraiser
  AppraiserInfo getInfoForAppraiser(const Appraiser* appraiser) const
  {
    return _scoringMediator.getInfoForAppraiser(appraiser);
  }

  // Returns the score builder's derived info for a given appraiser
  // (e.g. info may be appraiser's priority and the derived
  // info created by the builder may be appraiser's rank
  // calculated for this priority)
  // Throws if there is no such appraiser
  DerivedInfo getDerivedInfoForAppraiser(const Appraiser* appraiser) const
  {
    return _scoreBuilder.getDerivedInfo(_scoringMediator.getInfoForAppraiser(appraiser));
  }

  // Returns a vector of added appraisers
  const std::vector<const Appraiser*>& getAppraisers() const
  {
    return _scoringMediator.getAppraisers();
  }

  // Tells if all appraisers are satisfied
  // Returns true if there are no appraisers added
  bool areAllAppraisersSatisfied() const
  {
    const std::vector<const Appraiser*>& appraisers = getAppraisers();
    for (unsigned int i = 0; i < appraisers.size(); ++i)
    {
      if (!appraisers[i]->isSatisfied())
      {
        return false;
      }
    }
    return true;
  }

  // Tells if this swapper is full, i.e. its size
  // (number of contained elements) equals its capacity
  bool isFull() const { return getSize() == getCapacity(); }

  // Returns true if and only if this swapper is done, i.e.
  // is full and all appraisers are satisfied
  bool isDone() const { return (isFull() && areAllAppraisersSatisfied()); }

  void addItemSetObserver(ImplItemSetObserver* observer) override
  {
    TSE_ASSERT(observer != nullptr);
    _itemSetObservers.push_back(observer);
  }

  void removeItemSetObserver(ImplItemSetObserver* observer) override
  {
    TSE_ASSERT(observer != nullptr);
    _itemSetObservers.erase(
        std::remove(_itemSetObservers.begin(), _itemSetObservers.end(), observer),
        _itemSetObservers.end());
  }

private:
  // Injects the modifications done by the mediator to the scored set.
  class MapUpdater : public IMapUpdater<ItemType, ItemScore>
  {
  public:
    MapUpdater(ImplScoredSet& refSet) : _scoredItems(refSet) {}
    virtual void updateValue(const ItemType& key, const ItemScore& value) override
    {
      _scoredItems.updateScore(key, value);
    }

  private:
    ImplScoredSet& _scoredItems;
  };

  bool isOverloaded() const { return _scoredItems.getSize() > _capacity; }

  void notifyItemAdded(const ItemType& item)
  {
    for (size_t i = 0; i < _itemSetObservers.size(); ++i)
    {
      _itemSetObservers[i]->itemAdded(item);
    }
  }

  void notifyItemRemoved(const ItemType& item)
  {
    for (size_t i = 0; i < _itemSetObservers.size(); ++i)
    {
      _itemSetObservers[i]->itemRemoved(item);
    }
  }

  unsigned int _capacity;
  ImplScoredSet _scoredItems;
  MapUpdater _mapUpdater;
  ScoreBuilderType _scoreBuilder;
  ImplScoringMediator _scoringMediator;

  unsigned int _noProgressIterationsCount;
  unsigned int _totalIterationsCount;
  std::vector<ImplItemSetObserver*> _itemSetObservers;
};

} // namespace swp

} // namespace tse

