// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DataModel/Common/SafeEnums.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace tax
{
// typename Container must meet the conditions of std's Container concept
template <typename Container>
class GroupedContainers
{
  static unsigned asInt(type::ProcessingGroup pg) { return static_cast<unsigned char>(pg); }

public:
  typedef type::ProcessingGroup Key;
  GroupedContainers() : _containers(asInt(Key::GroupsCount)) {}
  GroupedContainers(size_t internalSize) : _containers()
  {
    _containers.reserve(asInt(Key::GroupsCount));
    for (size_t i = 0U; i != asInt(Key::GroupsCount); ++i)
      _containers.emplace_back(internalSize);
  }

  Container& get(const Key&);
  const Container& get(const Key&) const;

  typename Container::difference_type
  getFilteredSize(const Key&,
                  const std::function<bool(typename Container::const_reference)>&) const;

  Container
  getFilteredCopy(const Key&,
                  const std::function<bool(typename Container::const_reference)>&) const;

  void swap(GroupedContainers& rhs) { _containers.swap(rhs._containers); }

private:
  std::vector<Container> _containers;
};

template <typename Container>
void swap(GroupedContainers<Container>& lhs, GroupedContainers<Container>& rhs)
{
  lhs.swap(rhs);
}

template <typename Container>
Container&
GroupedContainers<Container>::get(const Key& key)
{
  size_t index = asInt(key);
  if (UNLIKELY(index >= _containers.size()))
    throw std::range_error("Wrong processing group id!");
  return _containers[index];
}

template <typename Container>
const Container&
GroupedContainers<Container>::get(const Key& key) const
{
  size_t index = asInt(key);
  if (UNLIKELY(index >= _containers.size()))
    throw std::range_error("Wrong processing group id!");
  return _containers[index];
}

template <typename Container>
typename Container::difference_type
GroupedContainers<Container>::getFilteredSize(
    const Key& key,
    const std::function<bool(typename Container::const_reference)>& predicate) const
{
  const Container& container = get(key);
  return std::count_if(std::begin(container), std::end(container), predicate);
}

template <typename Container>
Container
GroupedContainers<Container>::getFilteredCopy(
    const Key& key,
    const std::function<bool(typename Container::const_reference)>& predicate) const
{
  const Container& container = get(key);
  Container filtered;
  std::copy_if(std::begin(container),
               std::end(container),
               std::back_inserter(filtered),
               predicate);

  return filtered;
}

} // tax

