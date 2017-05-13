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
#include "DBAccess/Flattenizable.h"

namespace tse
{

class AirlineAllianceCarrierInfo
{
public:
  AirlineAllianceCarrierInfo() {}

  ~AirlineAllianceCarrierInfo() {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  GenericAllianceCode& genericAllianceCode() { return _genericAllianceCode; }
  const GenericAllianceCode& genericAllianceCode() const { return _genericAllianceCode; }

  std::string& genericName() { return _genericName; }
  const std::string& genericName() const { return _genericName; }

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

  bool operator==(const AirlineAllianceCarrierInfo& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_genericAllianceCode == rhs._genericAllianceCode) &&
            (_genericName == rhs._genericName) && (_createDate == rhs._createDate) &&
            (_lockDate == rhs._lockDate) && (_expireDate == rhs._expireDate) &&
            (_lastModDate == rhs._lastModDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate));
  }

  static void dummyData(AirlineAllianceCarrierInfo& obj)
  {
    obj._carrier = "LO";
    obj._genericAllianceCode = "*O";
    obj._genericName = "ONEWORLD";
    obj._createDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lastModDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
  }

protected:
  CarrierCode _carrier;
  GenericAllianceCode _genericAllianceCode;
  std::string _genericName;
  DateTime _createDate;
  DateTime _lockDate;
  DateTime _expireDate;
  DateTime _lastModDate;
  DateTime _effDate;
  DateTime _discDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _genericAllianceCode);
    FLATTENIZE(archive, _genericName);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lastModDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

private:
  AirlineAllianceCarrierInfo(const AirlineAllianceCarrierInfo& rhs);
  AirlineAllianceCarrierInfo& operator=(const AirlineAllianceCarrierInfo& rhs);
};
}

