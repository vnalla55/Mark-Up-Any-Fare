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
#ifndef PRICING_OBJECT_POOL_MEMOORY_MANAGER
#define PRICING_OBJECT_POOL_MEMOORY_MANAGER

#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FactoriesConfig.h"

#include <stddef.h>

namespace tse
{
template <class T>
class ObjectPool
{
  struct Node
  {

#ifndef NDEBUG
    bool released;
#endif

    Node* next;
    Node* previous;
    union
    {
      char data[sizeof(T)];
      double dummy; // align hack
    } buffer;

    void* address() { return &buffer; }
  };

public:
  ObjectPool() : _allocated(nullptr), _free(nullptr) {}

  ~ObjectPool()
  {
    deleteObjects(_free);
    deleteObjects(_allocated);
  }

  T* construct()
  {
    Node* node;
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
    }
    T* t = new (node->address()) T;

#ifndef NDEBUG
    LOG4CXX_DEBUG(_logger, "Object constructed - " << t);
    node->released = false;
#endif

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

#ifndef NDEBUG
    LOG4CXX_DEBUG(_logger, "Releasing object - " << node->address());
    assert(!node->released);
#endif

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

#ifndef NDEBUG
    node->released = true;
#endif
  }

  void trim()
  {
    deleteObjects(_free);
    _free = nullptr;
  }

private:
  static Logger _logger;

  void deleteObjects(Node* node)
  {
    while (node)
    {
      Node* current = node;
      node = node->next;

      T* t = static_cast<T*>(current->address());
      t->~T();
      delete current;
    }
  }

  Node* _allocated;
  Node* _free;
};

template <class T>
Logger
ObjectPool<T>::_logger("atseintl.Pricing.FarePathFactoryStorage");

class ObjectPoolMemoryManager : public FarePathFactoryStorage::IMemoryManager
{
  virtual ~ObjectPoolMemoryManager() {}

  virtual FPPQItem& constructFPPQItem() override { return *_fppqItemPool.construct(); }

  virtual void releaseFPPQItem(FPPQItem& fppqItem) override { _fppqItemPool.destroy(&fppqItem); }

  virtual FarePath& constructFarePath() override { return *_farePathPool.construct(); }

  virtual void releaseFarePath(FarePath& farePath) override { _farePathPool.destroy(&farePath); }

  virtual FareUsage& constructFareUsage() override { return *_fareUsagePool.construct(); }

  virtual void releaseFareUsage(FareUsage& fareUsage) override { _fareUsagePool.destroy(&fareUsage); }

  virtual PricingUnit& constructPricingUnit() override { return *_pricingUnitPool.construct(); }

  virtual void releasePricingUnit(PricingUnit& pricingUnit) override
  {
    _pricingUnitPool.destroy(&pricingUnit);
  }

  virtual void trimMemory() override
  {
    _fppqItemPool.trim();
    _farePathPool.trim();
    _fareUsagePool.trim();
    _pricingUnitPool.trim();
  }

private:
  ObjectPool<FPPQItem> _fppqItemPool;
  ObjectPool<FarePath> _farePathPool;
  ObjectPool<FareUsage> _fareUsagePool;
  ObjectPool<PricingUnit> _pricingUnitPool;
};
} //tse

#endif
