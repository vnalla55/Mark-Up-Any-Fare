//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include <atomic>
#include <limits>

#include <cassert>
#include <cstddef>
#include "Util/BranchPrediction.h"

namespace tse
{
namespace Memory
{
class CompositeManager;

class Manager
{
public:
  virtual ~Manager();

  CompositeManager* parent() const { return _parent; }
  void setParent(CompositeManager* p) { _parent = p; }

  size_t threshold() const { return _threshold; }
  void setThreshold(const size_t t) { _threshold = t; updateNotificationThreshold(); }

  bool isOutOfMemory() const { return _outOfMemory.load(std::memory_order_acquire); }
  virtual void setOutOfMemory();

  // Returns the most recently cached memory used; might not be accurate.
  size_t getTotalMemory() const { return _totalMemory.load(std::memory_order_acquire); }

  // Update totalMemory from outside. The changes will be propagated using totalMemoryChanged().
  virtual void updateTotalMemory();

protected:
  // This threshold says above which value should the notifyTotalMemoryChanged() be called.
  size_t notificationThreshold() const { return _notificationThreshold; }
  void setNotificationThreshold(const size_t t) { _notificationThreshold = t; }

  virtual void updateNotificationThreshold() { setNotificationThreshold(threshold()); }

  // Returns whether this call set the flag from false to true.
  bool setOutOfMemoryFlag() { return !_outOfMemory.exchange(true, std::memory_order_acq_rel); }
  void restoreOutOfMemoryFlag() { _outOfMemory.store(false, std::memory_order_release); }

  size_t setTotalMemory(size_t to)
  {
    const ptrdiff_t m = to;
    const ptrdiff_t prev = _totalMemory.exchange(m, std::memory_order_acq_rel);

    if (prev != m)
      totalMemoryChanged(m - prev, to);

    return to;
  }

  size_t changeTotalMemory(ptrdiff_t by)
  {
    assert(by != 0 && "Don't call with no change");

    const ptrdiff_t prev = _totalMemory.fetch_add(by, std::memory_order_acq_rel);
    const size_t memory = prev + by;

    totalMemoryChanged(by, memory);

    return memory;
  }

  void totalMemoryChanged(ptrdiff_t by, size_t to);
  virtual void notifyTotalMemoryChanged(ptrdiff_t by, size_t to);

private:
  CompositeManager* _parent = nullptr;
  size_t _threshold = std::numeric_limits<size_t>::max();
  size_t _notificationThreshold = std::numeric_limits<size_t>::max();
  std::atomic<ptrdiff_t> _totalMemory{0};
  std::atomic<bool> _outOfMemory{false};
};
}
}
