//----------------------------------------------------------------------------
//
//  File:        TseWrappers.h
//  Created:     1/16/2009
//  Authors:     JW
//
//  Description: Wrappers for common types
//
//  Updates:
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

#include <sstream>
#include <string>

namespace tse
{
class IntWrapper final
{
  int _value = 0;

public:
  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _value); }

  IntWrapper(int value = 0) : _value(value) {}

  IntWrapper(const std::string& value) : _value(atoi(value.c_str())) {}

  bool operator==(const int& rhs) const { return (rhs == _value); }

  bool operator!=(const int& rhs) const { return (rhs != _value); }

  int operator=(const int& rhs)
  {
    _value = rhs;
    return *this;
  }

  operator int() const { return _value; }

  std::string toString() const
  {
    std::ostringstream os;
    os << _value;
    return os.str();
  }

  void fromString(const std::string& str) { _value = atoi(str.c_str()); }

  static void dummyData(IntWrapper& obj) { obj._value = 1; }

}; // class IntWrapper

class BoolWrapper final
{
  bool _value = false;

public:
  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _value); }

  BoolWrapper(bool value = false) : _value(value) {}

  BoolWrapper(const std::string& value) : _value(fromString(value)) {}

  bool operator==(const bool& rhs) const { return (rhs == _value); }

  bool operator!=(const bool& rhs) const { return (rhs != _value); }

  bool operator=(const bool& rhs)
  {
    _value = rhs;
    return *this;
  }

  operator bool() const { return _value; }

  std::string toString() const { return (_value ? "1" : "0"); }

  static bool fromString(const std::string& str)
  {
    return ((str == "Y") || (str == "y") || (str == "1") || (str == "true") || (str == "TRUE") ||
            (str == "yes") || (str == "YES"));
  }

  static void dummyData(BoolWrapper& obj) { obj._value = true; }

}; // class BoolWrapper

} // end tse namespace

