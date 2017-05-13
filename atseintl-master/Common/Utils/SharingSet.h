//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "Common/Assert.h"
#include "Common/Utils/DeepSizeof.h"
#include "Common/Utils/Pprint.h"
#include "Common/Utils/PtrCollections.h"
#include <functional>
#include <iostream>
#include <mutex>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

namespace tse
{

namespace tools
{

namespace
{

}

struct SharingSetMemStats
{
  uint64_t bytes_saved; // elements with more than one reference save memory
  uint64_t bytes_lost; // elements with single reference generate loss
  uint64_t actual_mem_used; // not including map internals
  uint64_t hypothetical_mem_used; // possible usage without SharingSet
};

template <class T>
class SharingSet: boost::noncopyable
{
public:
  typedef size_t Counter;
  typedef PtrHashMap<T*, Counter> Storage;

  // Takes ownership of the pointer being inserted
  T* insert(T* ptr)
  {
    TSE_ASSERT(ptr != 0);
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter_and_insertion_flag = _storage.emplace(ptr, Counter(1));
    // If the element was not inserted, increase ref count
    if (!iter_and_insertion_flag.second)
    {
      ++iter_and_insertion_flag.first->second;
      // If user inserted a pointer being currently in storage, do not delete it.
      if (ptr != iter_and_insertion_flag.first->first)
      {
        delete ptr;
      }
    }
    return iter_and_insertion_flag.first->first;
  }


  bool erase(T* ptr)
  {
    TSE_ASSERT(ptr != 0);
    std::lock_guard<std::mutex> lock(_mutex);

    auto found = _storage.find(ptr);
    if (found != _storage.end() && ptr == found->first)
    {
      --(found->second);
      if (found->second == 0)
      {
        // The pointer may be dereferenced to calculate hash
        // Free it only after removing the item from the map
        T* to_delete = found->first;
        _storage.erase(found);
        delete to_delete;
      }
      return true;
    }
    return false;
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(_mutex);

    return _storage.size();
  }


  size_t usageCount(T* ptr) const
  {
    TSE_ASSERT(ptr != 0);
    std::lock_guard<std::mutex> lock(_mutex);

    auto found = _storage.find(ptr);
    if (found == _storage.end())
    {
      return 0;
    }
    return found->second;
  }


  size_t totalUsageCount() const
  {
    std::lock_guard<std::mutex> lock(_mutex);

    return _totalUsageCount();
  }


  SharingSetMemStats stats() const
  {
    std::lock_guard<std::mutex> lock(_mutex);

    return _stats();
  }


  void print_summary(std::ostream& out, bool details=false) const
  {
    std::lock_guard<std::mutex> lock(_mutex);

    out << "SHARING SET SUMMARY" << std::endl;
    out << "Total " << _storage.size() << " key/item pairs" << std::endl;
    out << "Total usage count " << _totalUsageCount() << std::endl;
    const auto mem_stats = _stats();
    out << "Saved   " << mem_stats.bytes_saved << " B memory" << std::endl;
    out << "Lost    " << mem_stats.bytes_lost << " B memory" << std::endl;
    uint64_t balance;
    if (mem_stats.bytes_saved >= mem_stats.bytes_lost)
    {
      balance = mem_stats.bytes_saved - mem_stats.bytes_lost;
    }
    else
    {
      balance = mem_stats.bytes_lost - mem_stats.bytes_saved;
    }
    out << "Balance " << balance << " B memory" << std::endl;
    out << "Actual memory " << mem_stats.actual_mem_used
        << " B (not including map internals)" << std::endl;
    out << "Hypothetical memory " << mem_stats.hypothetical_mem_used
        << " B" << std::endl;

    if (details)
    {
      for (const auto& ptr_usage: _storage)
      {
        out << "Element: ";
        pprint(out, ptr_usage.first);
        out << std::endl;
        out << "Usage count: " << ptr_usage.second << std::endl;
        out << "Element deep size: " << deep_sizeof(*ptr_usage.first) << std::endl;
        out << "Element saving: " << elem_saving(ptr_usage) << std::endl;
      }
    }
    out << "END OF SET SUMMARY" << std::endl;
  }

private:
  size_t _totalUsageCount() const
  {
    size_t c = 0;
    for (const auto& ptr_usage: _storage)
    {
      c += ptr_usage.second;
    }
    return c;
  }

  SharingSetMemStats _stats() const
  {
    SharingSetMemStats st = {0, 0, 0, 0};
    for (const auto& ptr_usage: _storage)
    {
      updateStatsForElem(st, ptr_usage.first, ptr_usage.second);
    }
    return st;
  }


  // Elements with more than one reference give savings.
  // Elements with single reference generate loss.
  static void updateStatsForElem(SharingSetMemStats& st, const T* p, Counter usage_count)
  {
    const uint64_t deep_size = deep_sizeof(*p);
    const uint64_t actual = deep_size + sizeof(T*) + sizeof(Counter);
    const uint64_t hypo = usage_count * deep_size;
    st.actual_mem_used += actual;
    st.hypothetical_mem_used += hypo;

    if (hypo >= actual)
    {
      st.bytes_saved += (hypo - actual);
    }
    else
    {
      st.bytes_lost += (actual - hypo);
    }
  }


  int64_t elem_saving(const typename Storage::value_type& ptr_usage) const
  {
    return ((static_cast<int64_t>(ptr_usage.second) - 1) * deep_sizeof(*ptr_usage.first))
        - static_cast<int64_t>(sizeof(T*))
        - static_cast<int64_t>(sizeof(Counter));
  }

  Storage _storage;
  mutable std::mutex _mutex;
};


} // namespace tools

} // namespace tse

