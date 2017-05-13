#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class LocKey
{
private:
  LocCode _loc;
  LocTypeCode _locType = ' ';

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _locType);
  }

  LocCode& loc() { return _loc; }
  const LocCode& loc() const { return _loc; }

  LocTypeCode& locType() { return _locType; }
  const LocTypeCode& locType() const { return _locType; }

  bool isNull() const
  {
    return _locType == ' ' && _loc.empty();
  };

  bool operator<(const LocKey& rhs) const
  {
    if (_loc != rhs._loc)
      return (_loc < rhs._loc);
    if (_locType != rhs._locType)
      return (_locType < rhs._locType);

    return false;
  }

  bool operator==(const LocKey& rhs) const
  {
    return ((_loc == rhs._loc) && (_locType == rhs._locType));
  }

  bool operator!=(const LocKey& rhs) const { return (!(*this == rhs)); }

  static void dummyData(LocKey& obj)
  {
    obj._loc = "ABCDEFGH";
    obj._locType = 'I';
  }

  friend inline std::ostream& operator<<(std::ostream& os, const LocKey& obj)
  {
    return os << "[" << obj._loc << "|" << obj._locType << "]";
  }
};
}
