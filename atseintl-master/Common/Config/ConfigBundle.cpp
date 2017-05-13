// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigBundle.h"

#include "Common/Config/DynamicConfigurableValue.h"
#include "Common/Config/TypedConfigurableValueBase.h"
#include "Common/ConfigList.h"
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Util/FlatMap.h"

#include <cassert>
#include <type_traits>

namespace tse
{
namespace
{
using BundleKey = std::pair<std::string, std::string>;

template <typename T>
using BundlePool = FlatMultiMap<BundleKey, DynamicConfigurableValue<T>*>;

template <typename T>
BundlePool<T>&
getBundlePool()
{
  static BundlePool<T> pool;
  return pool;
}

template <typename Type, typename... Types>
struct TypeToIndex;

template <typename Type, typename... Rest>
struct TypeToIndex<Type, Type, Rest...>
{
  static constexpr uint32_t value = 0;
};

template <typename Type, typename First, typename... Rest>
struct TypeToIndex<Type, First, Rest...>
{
  static constexpr uint32_t value = 1 + TypeToIndex<Type, Rest...>::value;
};

template <typename... Types>
struct CheckAlignment;

template <typename First, typename Second, typename... Rest>
struct CheckAlignment<First, Second, Rest...> : CheckAlignment<Second, Rest...>
{
  static_assert(alignof(First) >= alignof(Second), "The types should be sorted based on aligmnent");
};

template <typename Last>
struct CheckAlignment<Last>
{
};

template <typename... Types>
struct TypeList : CheckAlignment<Types...>
{
  static constexpr uint32_t Size = sizeof...(Types);

  // GCC 5 doesn't like variable templates inside class templates. Use function template instead.
  template <typename Type>
  static constexpr uint32_t indexOf() { return TypeToIndex<Type, Types...>::value; }
};

// NOTE: Sort these based on alignment.
using Types = TypeList<
  std::string,
  ConfigSet<CarrierCode>,
  DateTime,
  uint32_t,
  bool
>;

uint32_t bundleSizes[Types::Size];
uint32_t bundleOffsets[Types::Size];
uint32_t bundleTotalSize;

template <uint32_t Index_, typename Type_>
struct BundleIndex
{
  static constexpr uint32_t index = Index_;
  using Type = Type_;

  static Type* array(void* base)
  {
    const auto baseByte = reinterpret_cast<char*>(base);
    return reinterpret_cast<Type*>(baseByte + bundleOffsets[index]);
  }
};

template <typename AllTypes, typename Types = AllTypes>
struct BundleForEach;

template <typename AllTypes, typename First, typename... Rest>
struct BundleForEach<AllTypes, TypeList<First, Rest...>>
{
  template <typename Func>
  static void forEach(Func func)
  {
    func(BundleIndex<AllTypes::template indexOf<First>(), First>());
    BundleForEach<AllTypes, TypeList<Rest...>>::forEach(std::move(func));
  }
};

template <typename AllTypes>
struct BundleForEach<AllTypes, TypeList<>>
{
  template <typename Func>
  static void forEach(Func) {}
};
}

template <typename T>
void
DynamicConfigurableValuesBundlePool<T>::registerInBundle(DynamicConfigurableValue<T>& cv)
{
  BundlePool<T>& pool = getBundlePool<T>();

  std::locale locale;
  BundleKey key{boost::to_upper_copy(cv.getSection(), locale),
                boost::to_upper_copy(cv.getOption(), locale)};

  pool.insert({std::move(key), &cv});
}

template <typename T>
void
DynamicConfigurableValuesBundlePool<T>::unregisterInBundle(DynamicConfigurableValue<T>& cv)
{
  BundlePool<T>& pool = getBundlePool<T>();

  std::locale locale;
  BundleKey key{boost::to_upper_copy(cv.getSection(), locale),
                boost::to_upper_copy(cv.getOption(), locale)};

  const auto range = pool.equal_range(std::move(key));
  for (auto it = range.first; it != range.second; ++it)
  {
    if (it->second == &cv)
    {
      pool.erase(it);
      return;
    }
  }

  assert(!"DynamicConfigurableValue unregistered without prior register!");
}

template <typename T>
uint32_t
DynamicConfigurableValuesBundlePool<T>::size()
{
  constexpr uint32_t index = Types::indexOf<T>();
  return bundleSizes[index];
}

void
allocateAllConfigBundles()
{
  uint32_t offset = 0;

  BundleForEach<Types>::forEach([&offset](auto i)
  {
    using Type = typename decltype(i)::Type;
    BundlePool<Type>& pool = getBundlePool<Type>();
    constexpr uint32_t index = Types::indexOf<Type>();

    bundleOffsets[index] = offset;

    if (pool.empty())
    {
      bundleSizes[index] = 0;
      return;
    }

    // We assume that there are not too many duplicates in the multi map.
    auto prev = pool.begin();
    uint32_t count = 0;
    prev->second->_index = offset;

    for (auto it = prev + 1, end = pool.end(); it != end; prev = it++)
    {
      if (it->first != prev->first)
      {
        ++count;
        offset += sizeof(Type);
      }
      it->second->_index = offset;
    }

    bundleSizes[index] = count + 1;
    offset += sizeof(Type);
  });

  bundleTotalSize = offset;
}

void
ConfigBundle::allocate()
{
  _memory.reset(new char[bundleTotalSize], [](void* memory)
  {
    const auto base = reinterpret_cast<char*>(memory);
    BundleForEach<Types>::forEach([base](auto i)
    {
      using Type = typename decltype(i)::Type;
      const uint32_t size = bundleSizes[i.index];
      const auto array = i.array(base);

      for (uint32_t i = 0; i < size; ++i)
        array[i].~Type();
    });

    delete[] base;
  });

  BundleForEach<Types>::forEach([base = _memory.get()](auto i)
  {
    using Type = typename decltype(i)::Type;
    const uint32_t size = bundleSizes[i.index];
    const auto array = i.array(base);

    for (uint32_t i = 0; i < size; ++i)
      new (&array[i]) Type;
  });
}

ConfigBundle
ConfigBundle::clone() const
{
  assert(*this);

  ConfigBundle clone;
  clone.allocate();

  BundleForEach<Types>::forEach([base = _memory.get(), cloneBase = clone._memory.get()](auto i)
  {
    const uint32_t size = bundleSizes[i.index];
    const auto array = i.array(base);
    const auto cloneArray = i.array(cloneBase);

    for (uint32_t i = 0; i < size; ++i)
      cloneArray[i] = array[i];
  });

  return clone;
}

void
ConfigBundle::makeUnique()
{
  assert(*this);

  // Since std::shared_ptr<T>::unique() does not use memory barriers (at least in libstdc++),
  // we have to perform a memory barrier by ourselves here.
  // Fortunately, the release operation of shared_ptr does use a memory barrier so that we can
  // synchronize with it.
  std::atomic_thread_fence(std::memory_order_acquire);
  if (_memory.unique())
    return;

  ConfigBundle uniqued = clone();
  *this = std::move(uniqued);
}

void
ConfigBundle::fill(const ConfigMan& config)
{
  if (!*this)
    allocate();

  BundleForEach<Types>::forEach([this, &config](auto i)
  {
    using Type = typename decltype(i)::Type;

    uint32_t prevIndex = -1;
    for (const auto& kv : getBundlePool<Type>())
    {
      const auto value = kv.second;

      if (value->index() == prevIndex)
        continue;
      prevIndex = value->index();

      auto& slot = this->get<Type>(value->index());
      const auto prev = slot;
      value->getValueFrom(config, slot);
      value->logIfValueHasChanged(prev, slot);
    }
  });
}

void
ConfigBundle::update(const ConfigMan& config, std::string section, std::string option)
{
  assert(*this);

  std::locale locale;
  boost::to_upper(section, locale);
  boost::to_upper(option, locale);
  BundleKey key{std::move(section), std::move(option)};

  BundleForEach<Types>::forEach([this, &config, &key](auto i)
  {
    using Type = typename decltype(i)::Type;

    const auto& pool = getBundlePool<Type>();
    const auto it = pool.find(key);
    if (it == pool.end())
      return;

    const auto value = it->second;
    auto& slot = this->get<Type>(value->index());
    const auto prev = slot;
    value->getValueFrom(config, slot);
    value->logIfValueHasChanged(prev, slot);
  });
}

// instantiate all bundles
template struct DynamicConfigurableValuesBundlePool<std::string>;
template struct DynamicConfigurableValuesBundlePool<ConfigSet<CarrierCode>>;
template struct DynamicConfigurableValuesBundlePool<DateTime>;
template struct DynamicConfigurableValuesBundlePool<uint32_t>;
template struct DynamicConfigurableValuesBundlePool<bool>;
}

#endif
