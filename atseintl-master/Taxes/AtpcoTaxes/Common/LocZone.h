// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DataModel/Common/Types.h"
#include <string>

namespace tax
{

class LocZone
{
public:
  LocZone() : _type(type::LocType::Blank), _code(UninitializedCode) {}
  explicit LocZone(type::LocType type) : _type(type), _code(UninitializedCode) {}
  explicit LocZone(type::LocType type, type::LocZoneText code) :
      _type(type), _code(code) {}

  const type::LocType& type() const { return _type; }

  type::LocType& type() { return _type; }

  const type::LocZoneText& code() const { return _code; }

  type::LocZoneText& code() { return _code; }

  std::string toString() const;

  friend bool operator==(const LocZone& lhs, const LocZone& rhs) {
    return lhs._type == rhs._type && lhs._code == rhs._code;
  }

  bool isSet() const { return _type != type::LocType::Blank; }

private:
  type::LocType _type;
  type::LocZoneText _code;
};
}

