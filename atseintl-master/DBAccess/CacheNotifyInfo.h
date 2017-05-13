//----------------------------------------------------------------------------
//
//   File:           CacheNotifyInfo.h
//   Description:    Data transfer object for cache notifications.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

#include <string>

namespace tse
{

class CacheNotifyInfo
{
public:
  CacheNotifyInfo() : _id((uint64_t) - 1), _orderno((uint64_t) - 1), _entityType(""), _keyString("")
  {
  }
  uint64_t& id() { return _id; }
  const uint64_t& id() const { return _id; }
  uint64_t& orderno() { return _orderno; }
  const uint64_t& orderno() const { return _orderno; }
  std::string& entityType() { return _entityType; }
  const std::string& entityType() const { return _entityType; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  std::string& keyString() { return _keyString; }
  const std::string& keyString() const { return _keyString; }
  bool operator==(const CacheNotifyInfo& rhs) const
  {
    // we are not comparing _id, since it will never match.
    return _orderno == rhs._orderno && _createDate == rhs._createDate &&
           _entityType == rhs._entityType && _keyString == rhs._keyString;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _id);
    FLATTENIZE(archive, _orderno);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _entityType);
    FLATTENIZE(archive, _keyString);
  }

private:
  uint64_t _id;
  uint64_t _orderno;
  // std::string _priority;
  DateTime _createDate;
  std::string _entityType;
  std::string _keyString;
};
}

