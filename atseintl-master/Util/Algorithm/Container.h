//-------------------------------------------------------------------
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
//   The purpose of this file is to contain container-based algorithmic functions
//   in contrast to iterator-based provided by standard library and third parties.
//
//   E.g. alg::contains(v, 1) instead of std::find(v.begin, v.end(), 1) != v.end()
//
//-------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <type_traits>

namespace tse
{
namespace alg
{
template <class Container, class Value>
inline bool
contains(const Container& cont, const Value& val)
{
  return std::find(std::begin(cont), std::end(cont), val) != std::end(cont);
}

namespace helper
{
template <class T>
struct Void
{
  typedef void type;
};

template <class T, class U = void>
struct is_associative_container : std::false_type
{
};

template <class T>
struct is_associative_container<T, class Void<class T::key_type>::type> : std::true_type
{
};
}

template <class Container, class Predicate>
inline typename std::enable_if_t<!helper::is_associative_container<Container>::value>
erase_if(Container& cont, Predicate predicate)
{
  cont.erase(std::remove_if(std::begin(cont), std::end(cont), std::move(predicate)),
             std::end(cont));
}

template <class Container, class Predicate>
inline typename std::enable_if_t<helper::is_associative_container<Container>::value>
erase_if(Container& cont, Predicate predicate)
{
  for (auto itr = std::begin(cont); itr != std::end(cont);)
  {
    if (predicate(*itr))
      itr = cont.erase(itr);
    else
      ++itr;
  }
}

template <class Container, class Predicate, class OutputIterator>
inline void
copy_if(Container& cont, OutputIterator toCopy, Predicate predicate)
{
  std::copy_if(std::begin(cont), std::end(cont), toCopy, std::move(predicate));
}

template <class Container, class Predicate>
inline size_t
find_index_if(Container& cont, Predicate predicate)
{
  return std::find_if(std::begin(cont), std::end(cont), std::move(predicate)) - std::begin(cont);
}
}
}
