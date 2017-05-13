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

#include "Common/TypeConvert.h"
#include "Util/FlatSet.h"

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace tse
{
// ConfigList can be initialized with string of the form: "XXX|YY|ZZZZ" so that
// each part between vertical lines is considered a separate item.
// Items can be stored in container of any choice. Some containers e.g. FlatSet require
// additional actions after insertion and that's why policy pattern has been introduced
// as an abstraction layer between ConfigList and the container.

template <class T>
struct FlatSetPolicy
{
  typedef FlatSet<T> ContainerType;

  static void insertValue(const T& value, ContainerType& set) { set.unsafe_insert(value); }
  static void finalizeInsertion(ContainerType& set)
  {
    set.order();
    set.shrink_to_fit();
  }
  static bool has(const T& value, const ContainerType& set) { return set.find(value) != set.end(); }
};

template <class T, class Vec = std::vector<T>>
struct VectorPolicy
{
  typedef Vec ContainerType;

  static void insertValue(const T& value, ContainerType& vec) { vec.push_back(value); }
  static void finalizeInsertion(ContainerType&) {}
  static bool has(const T& value, const ContainerType& vec)
  {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
  }
};

template <typename T, class Policy>
class ConfigList
{
public:
  typedef typename Policy::ContainerType ContainerType;
  typedef typename ContainerType::iterator iterator;
  typedef typename ContainerType::const_iterator const_iterator;

  ConfigList() {}
  ConfigList(const std::string& listString) { load(listString); }

  operator std::string() const;
  bool load(const std::string& listString);
  bool has(const T& value) const { return Policy::has(value, _items); }

  const_iterator begin() const { return _items.begin(); }
  const_iterator end() const { return _items.end(); }

  friend bool operator==(const ConfigList& l, const ConfigList& r) { return l._items == r._items; }

private:
  ContainerType _items;
};

template <typename T, typename Policy>
inline
ConfigList<T, Policy>::operator std::string() const
{
  std::string result;
  for (const T& item : _items)
  {
    if (!result.empty())
      result += '|';
    result += TypeConvert::valueToString(item);
  }
  return result;
}

template <typename T, typename Policy>
inline bool
ConfigList<T, Policy>::load(const std::string& listString)
{
  if (listString.empty())
  {
    _items.clear();
    return true;
  }

  typedef boost::char_separator<char> Separator;
  typedef boost::tokenizer<Separator> Tokenizer;
  Tokenizer tokens(listString, Separator("|"));
  ContainerType newItems;

  for (const std::string& token : tokens)
  {
    T value;
    if (!TypeConvert::stringToValue(token, value))
      return false;
    Policy::insertValue(value, newItems);
  }

  Policy::finalizeInsertion(newItems);
  _items.swap(newItems);
  return true;
}

template <typename T, typename Vector = std::vector<T>>
using ConfigVector = ConfigList<T, VectorPolicy<T, Vector>>;

template <typename T>
using ConfigSet = ConfigList<T, FlatSetPolicy<T>>;

namespace TypeConvert
{
template <typename T, typename Policy>
inline std::string
valueToString(const ConfigList<T, Policy>& value)
{
  return std::string(value);
}

template <typename T, typename Policy>
inline bool
stringToValue(const std::string& stringValue, ConfigList<T, Policy>& output)
{
  return output.load(stringValue);
}

} // TypeConvert
} // tse

