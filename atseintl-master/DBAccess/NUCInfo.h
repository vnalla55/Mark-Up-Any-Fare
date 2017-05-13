
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class NUCInfo
{
public:
  const DateTime& expireDate() const { return _expireDate; }
  const DateTime& effDate() const { return _effDate; }
  const DateTime& createDate() const { return _createDate; }
  const DateTime& discDate() const { return _discDate; }

  CurrencyCode _cur;
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  CurrencyFactor _nucFactor = 0;
  CurrencyFactor _roundingFactor = 0;
  CurrencyNoDec _nucFactorNodec = 0;
  CurrencyNoDec _roundingFactorNodec = 0;
  RoundingRule _roundingRule = RoundingRule::EMPTY;

  bool operator==(const NUCInfo& rhs) const
  {
    return ((_cur == rhs._cur) && (_carrier == rhs._carrier) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_createDate == rhs._createDate) && (_nucFactor == rhs._nucFactor) &&
            (_roundingFactor == rhs._roundingFactor) && (_nucFactorNodec == rhs._nucFactorNodec) &&
            (_roundingFactorNodec == rhs._roundingFactorNodec) &&
            (_roundingRule == rhs._roundingRule));
  }

  friend inline std::ostream& operator<<(std::ostream& os, const NUCInfo& obj)
  {
    return os << "[" << obj._cur << "|" << obj._carrier << "|" << obj._expireDate << "|"
              << obj._effDate << "|" << obj._discDate << "|" << obj._createDate << "|"
              << obj._nucFactor << "|" << obj._roundingFactor << "|" << obj._nucFactorNodec << "|"
              << obj._roundingFactorNodec << "|" << obj._roundingRule << "]";
  }

  static void dummyData(NUCInfo& obj)
  {
    obj._cur = "ABC";
    obj._carrier = "DEF";
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._nucFactor = 1.111;
    obj._roundingFactor = 2.222;
    obj._nucFactorNodec = 3;
    obj._roundingFactorNodec = 4;
    obj._roundingRule = UP;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _nucFactor);
    FLATTENIZE(archive, _roundingFactor);
    FLATTENIZE(archive, _nucFactorNodec);
    FLATTENIZE(archive, _roundingFactorNodec);
    FLATTENIZE(archive, _roundingRule);
  }
};
} // namespace tse
