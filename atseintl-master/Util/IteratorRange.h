#pragma once

#include <cstddef>
#include <utility>

namespace tse
{
template <class Iter>
class IteratorRange : protected std::pair<Iter, Iter>
{
  typedef std::pair<Iter, Iter> IterPair;

public:
  using IterPair::IterPair;

  Iter begin() const { return IterPair::first; }
  Iter end() const { return IterPair::second; }
  bool empty() const { return IterPair::first == IterPair::second; }
  std::size_t size() const { return std::distance(IterPair::first, IterPair::second); }
};

template <class Iter>
inline IteratorRange<typename std::decay<Iter>::type>
makeIteratorRange(Iter&& first, Iter&& last)
{
  typedef typename std::decay<Iter>::type PureIterType;
  return IteratorRange<PureIterType>(std::forward<Iter>(first), std::forward<Iter>(last));
}

template <class Iter>
inline IteratorRange<Iter>
makeIteratorRange(const std::pair<Iter, Iter>& iterPair)
{
  return IteratorRange<Iter>(iterPair);
}
}
