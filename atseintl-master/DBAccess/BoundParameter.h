//----------------------------------------------------------------------------
//
//     File:           BoundParameter.h
//     Description:    BoundParameter
//     Created:        07/01/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>

namespace DBAccess
{
class ParameterBinder;

class BoundParameter
{
public:
  //
  // class Comparator compares two BoundParameter objects based on
  //  their relative position (within the SQL statement string).
  // This is needed to keep the collection sorted and help us determine
  //  the index number to use when binding the value.
  //
  class Comparator
  {
  public:
    bool operator()(const BoundParameter* lhs, const BoundParameter* rhs)
    {
      return lhs->position() < rhs->position();
    }
  };

public:
  BoundParameter() = default;
  BoundParameter(int32_t index, size_t position) : _index(index), _position(position) {}

  inline virtual ~BoundParameter() = 0;

  virtual void bind(const ParameterBinder& binder) = 0;

  void index(int32_t index) { _index = index; }
  int32_t index() const { return _index; }

  void position(size_t position) { _position = position; }
  size_t position() const { return _position; }

private:
  int32_t _index = -1;
  size_t _position = std::string::npos; // Position within SQL statement string
}; // class BoundParameter

BoundParameter::~BoundParameter() = default;

using BoundParameterCollection = std::set<BoundParameter*, BoundParameter::Comparator>;
} // namespace DBAccess

