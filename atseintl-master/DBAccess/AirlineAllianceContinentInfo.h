// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class AirlineAllianceContinentInfo
{
public:
  AirlineAllianceContinentInfo() : _continent(UNKNOWN_CONTINENT), _locType(' ') {}

  ~AirlineAllianceContinentInfo() {}

  Continent& continent() { return _continent; }
  const Continent& continent() const { return _continent; }

  LocTypeCode& locType() { return _locType; }
  const LocTypeCode& locType() const { return _locType; }

  LocCode& locCode() { return _locCode; }
  const LocCode& locCode() const { return _locCode; }

  GenericAllianceCode& genericAllianceCode() { return _genericAllianceCode; }
  const GenericAllianceCode& genericAllianceCode() const { return _genericAllianceCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lastModDate() { return _lastModDate; }
  const DateTime& lastModDate() const { return _lastModDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const AirlineAllianceContinentInfo& rhs) const
  {
    return ((_continent == rhs._continent) && (_locType == rhs._locType) &&
            (_locCode == rhs._locCode) && (_genericAllianceCode == rhs._genericAllianceCode) &&
            (_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
            (_expireDate == rhs._expireDate) && (_lastModDate == rhs._lastModDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate));
  }

  static void dummyData(AirlineAllianceContinentInfo& obj)
  {
    obj._continent = CONTINENT_AFRICA;
    obj._locType = 'Z';
    obj._locCode = "09958";
    obj._genericAllianceCode = "*O";
    obj._createDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lastModDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

protected:
  Continent _continent;
  LocTypeCode _locType;
  LocCode _locCode;
  GenericAllianceCode _genericAllianceCode;
  DateTime _createDate;
  DateTime _lockDate;
  DateTime _expireDate;
  DateTime _lastModDate;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _continent);
    FLATTENIZE(archive, _locType);
    FLATTENIZE(archive, _locCode);
    FLATTENIZE(archive, _genericAllianceCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

private:
  AirlineAllianceContinentInfo(const AirlineAllianceContinentInfo& rhs);
  AirlineAllianceContinentInfo& operator=(const AirlineAllianceContinentInfo& rhs);
};
}

