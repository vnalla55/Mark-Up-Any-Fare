//------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Memory/OutOfMemoryException.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Util/BranchPrediction.h"

#include <stddef.h>

namespace tse
{
template <class T>
class CountingObjectPool final
{
  struct Node
  {
    Node* next;
    Node* previous;
    union
    {
      char data[sizeof(T)];
      double dummy; // align hack
    } buffer;

    void* address() { return &buffer; }

    Node() : next(nullptr), previous(nullptr) {}
  };

public:
  CountingObjectPool() : _allocated(nullptr), _free(nullptr), _nodeCount(0) {}

  ~CountingObjectPool()
  {
    deleteObjects(_free);
    deleteObjects(_allocated);
  }

  T* construct()
  {
    Node* node(nullptr);
    if (_free)
    {
      node = _free;
      _free = _free->next;
      if (_free)
      {
        _free->previous = nullptr;
      }

      T* t = static_cast<T*>(node->address());
      t->~T();
    }
    else
    {
      node = new Node;
      ++_nodeCount;
    }
    T* t = new (node->address()) T;

    node->next = _allocated;
    node->previous = nullptr;
    if (_allocated)
    {
      _allocated->previous = node;
    }

    _allocated = node;
    return t;
  }

  void destroy(T* t)
  {
    char* address = static_cast<char*>(static_cast<void*>(t));
    Node* node = static_cast<Node*>(static_cast<void*>(address - offsetof(Node, buffer)));

    if (node->previous)
    {
      node->previous->next = node->next;
    }
    else
    {
      _allocated = _allocated->next;
    }

    if (node->next)
    {
      node->next->previous = node->previous;
    }

    if (_free)
    {
      _free->previous = node;
    }

    node->next = _free;
    node->previous = nullptr;
    _free = node;
  }

  size_t trim()
  {
    const size_t deleted = deleteObjects(_free);
    _free = nullptr;

    return deleted * sizeof(Node);
  }

  size_t getTotalMemory() const { return _nodeCount * sizeof(Node); }

private:
  size_t deleteObjects(Node* node)
  {
    size_t count = 0;

    while (node)
    {
      Node* current = node;
      node = node->next;

      T* t = static_cast<T*>(current->address());
      t->~T();
      delete current;

      ++count;
    }

    _nodeCount -= count;
    return count;
  }

  Node* _allocated;
  Node* _free;
  size_t _nodeCount;
};

class CountingObjectPoolMemoryManager : public FarePathFactoryStorage::IMemoryManager
{
public:
  CountingObjectPoolMemoryManager(size_t checkPeriod)
    : _checkPeriod(checkPeriod), _nextCheck(checkPeriod)
  {
  }

  FPPQItem& constructFPPQItem() override { return constructOrThrow(_fppqItemPool); }
  void releaseFPPQItem(FPPQItem& fppqItem) override { destroyOrThrow(fppqItem, _fppqItemPool); }

  FarePath& constructFarePath() override { return constructOrThrow(_fpPool); }
  void releaseFarePath(FarePath& fp) override { destroyOrThrow(fp, _fpPool); }

  FareUsage& constructFareUsage() override { return constructOrThrow(_fuPool); }
  void releaseFareUsage(FareUsage& fu) override { destroyOrThrow(fu, _fuPool); }

  PricingUnit& constructPricingUnit() override { return constructOrThrow(_puPool); }
  void releasePricingUnit(PricingUnit& pu) override { destroyOrThrow(pu, _puPool); }

  void trimMemory() override
  {
    size_t reclaimed = 0;

    reclaimed += _fppqItemPool.trim();
    reclaimed += _fpPool.trim();
    reclaimed += _fuPool.trim();
    reclaimed += _puPool.trim();

    _accurateTotalMemory -= reclaimed;
    setTotalMemory(_accurateTotalMemory);
  }

  void updateTotalMemory() override final { setTotalMemory(_accurateTotalMemory); }

private:
  template <class Item>
  Item& constructOrThrow(CountingObjectPool<Item>& pool)
  {
    if (UNLIKELY(isOutOfMemory()))
      throw Memory::OutOfMemoryException();

    const size_t oldSize = pool.getTotalMemory();
    Item& result = *pool.construct();
    const size_t newSize = pool.getTotalMemory();
    increaseTotalMemory(newSize - oldSize);

    return result;
  }

  template <class Item>
  void destroyOrThrow(Item& item, CountingObjectPool<Item>& pool)
  {
    if (UNLIKELY(isOutOfMemory()))
      throw Memory::OutOfMemoryException();

    pool.destroy(&item);
  }

  void increaseTotalMemory(const ptrdiff_t by)
  {
    assert(by >= 0 && "Not an increase");

    _accurateTotalMemory += by;

    if (LIKELY(_accurateTotalMemory < _nextCheck))
      return;

    // Ensure only one period is contained by this increase.
    assert(_accurateTotalMemory < _nextCheck + _checkPeriod && "Too big increase");

    _nextCheck += _checkPeriod;
    updateTotalMemory();
  }

  CountingObjectPool<FPPQItem> _fppqItemPool;
  CountingObjectPool<FarePath> _fpPool;
  CountingObjectPool<FareUsage> _fuPool;
  CountingObjectPool<PricingUnit> _puPool;

  const size_t _checkPeriod;
  size_t _accurateTotalMemory = 0;
  size_t _nextCheck;
};
} //tse

