// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "Util/VectorFwd.h"

namespace tse
{
template <typename Type, typename Compare = std::less<Type>, typename Container = Vector<Type>>
class FlatSet;

template <typename Type, typename Compare = std::less<Type>>
using StdVectorFlatSet = FlatSet<Type, Compare, std::vector<Type>>;

template <typename Type, typename Compare = std::less<Type>, typename Container = Vector<Type>>
class FlatMultiSet;

template <typename Type, typename Compare = std::less<Type>>
using StdVectorFlatMultiSet = FlatMultiSet<Type, Compare, std::vector<Type>>;

// TODO use a different value_type to make users unable to change the key.
template <typename KeyType,
          typename MappedType,
          typename Compare = std::less<KeyType>,
          typename Container = Vector<std::pair<KeyType, MappedType>>>
class FlatMap;

template <typename KeyType, typename MappedType, typename Compare = std::less<KeyType>>
using StdVectorFlatMap =
    FlatMap<KeyType, MappedType, Compare, std::vector<std::pair<KeyType, MappedType>>>;

template <typename KeyType,
          typename MappedType,
          typename Compare = std::less<KeyType>,
          typename Container = Vector<std::pair<KeyType, MappedType>>>
class FlatMultiMap;

template <typename KeyType, typename MappedType, typename Compare = std::less<KeyType>>
using StdVectorFlatMultiMap =
    FlatMultiMap<KeyType, MappedType, Compare, std::vector<std::pair<KeyType, MappedType>>>;
}

