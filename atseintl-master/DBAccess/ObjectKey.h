//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Code.h"
#include "Common/DateTime.h"
#include "Common/ObjectComparison.h"

#include <boost/container/flat_map.hpp>
#include <boost/container/string.hpp>

#include <map>
#include <string>

/**
 *  ObjectKey - struct used as a generic representation of cache keys.
 *  ObjectKeys are NOT the same as the keys used to index the maps. They are,
 *  however, used to pass those keys to and from the cache update notification service.
 */

namespace tse
{

class ObjectKey
{
public:

  typedef boost::container::flat_map<std::string, std::string> KeyFields;

  ObjectKey(const std::string& tableName = "") : _tableName(tableName) {}

  std::string& tableName() { return _tableName; }
  const std::string& tableName() const { return _tableName; }

  std::string& entityType() { return _entityType; }
  const std::string& entityType() const { return _entityType; }

  KeyFields& keyFields() { return _keyFields; }
  const KeyFields& keyFields() const { return _keyFields; }

  // helper functions
  //
  bool getValue(const char* id, std::string& value) const;
  bool getValue(const char* id, boost::container::string& value) const;
  bool getValue(const char* id, int& value) const;
  bool getValue(const char* id, char& value) const;
  bool getValue(const char* id, long long& value) const;
  bool getValue(const char* id, RecordScope& value) const;
  bool getValue(const char* id, GlobalDirection& value) const;
  bool getValue(const char* id, uint64_t& value) const;
  bool getValue(const char* id, DateTime& value) const;

  template <size_t n, typename T>
  bool getValue(const char* id, Code<n, T>& value) const
  {
    KeyFields::const_iterator it(_keyFields.find(id));
    if (LIKELY(it != _keyFields.end()))
    {
      value = it->second;
      return true;
    }
    return false;
  }

  void setValue(const char* id, const char* value);
  void setValue(const char* id, const std::string& value);
  void setValue(const char* id, const boost::container::string& value);
  void setValue(const char* id, const int& value);
  void setValue(const char* id, const char& value);
  void setValue(const char* id, const long long& value);
  void setValue(const char* id, const RecordScope& value);
  void setValue(const char* id, const GlobalDirection& value);
  void setValue(const char* id, const uint64_t& value);
  void setValue(const char* id, const DateTime& value);

  template <size_t n, typename T>
  void setValue(const char* id, const Code<n, T>& value)
  {
    _keyFields[id] = value;
  }

  std::string toString(bool includeLabel = true) const;
  ObjectKey& fromString(const std::string& str);

  bool operator==(const ObjectKey& rhs) const
  {
    return ((_tableName == rhs._tableName) && (_entityType == rhs._entityType) &&
            (objectsIdentical(_keyFields, rhs._keyFields)));
  }

  bool operator<(const ObjectKey& other) const
  {
    if (_tableName < other._tableName)
    {
      return true;
    }
    else if (_tableName > other._tableName)
    {
      return false;
    }
    if (_entityType < other._entityType)
    {
      return true;
    }
    else if (_entityType > other._entityType)
    {
      return false;
    }
    return _keyFields < other._keyFields;
  }

private:
  std::string _tableName;
  std::string _entityType;
  KeyFields _keyFields;
};

} // tse namespace

