//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxAkHiFactor
{
public:
  TaxAkHiFactor()
    : _zoneANoDec(0),
      _zoneAPercent(0),
      _zoneBNoDec(0),
      _zoneBPercent(0),
      _zoneCNoDec(0),
      _zoneCPercent(0),
      _zoneDNoDec(0),
      _zoneDPercent(0),
      _hawaiiNoDec(0),
      _hawaiiPercent(0)
  {
  }
  LocCode& city() { return _city; }
  const LocCode& city() const { return _city; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& zoneANoDec() { return _zoneANoDec; }
  const int& zoneANoDec() const { return _zoneANoDec; }

  Percent& zoneAPercent() { return _zoneAPercent; }
  const Percent& zoneAPercent() const { return _zoneAPercent; }

  int& zoneBNoDec() { return _zoneBNoDec; }
  const int& zoneBNoDec() const { return _zoneBNoDec; }

  Percent& zoneBPercent() { return _zoneBPercent; }
  const Percent& zoneBPercent() const { return _zoneBPercent; }

  int& zoneCNoDec() { return _zoneCNoDec; }
  const int& zoneCNoDec() const { return _zoneCNoDec; }

  Percent& zoneCPercent() { return _zoneCPercent; }
  const Percent& zoneCPercent() const { return _zoneCPercent; }

  int& zoneDNoDec() { return _zoneDNoDec; }
  const int& zoneDNoDec() const { return _zoneDNoDec; }

  Percent& zoneDPercent() { return _zoneDPercent; }
  const Percent& zoneDPercent() const { return _zoneDPercent; }

  int& hawaiiNoDec() { return _hawaiiNoDec; }
  const int& hawaiiNoDec() const { return _hawaiiNoDec; }

  Percent& hawaiiPercent() { return _hawaiiPercent; }
  const Percent& hawaiiPercent() const { return _hawaiiPercent; }

  bool operator==(const TaxAkHiFactor& rhs) const
  {
    return ((_city == rhs._city) && (_versionDate == rhs._versionDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_expireDate == rhs._expireDate) &&
            (_zoneANoDec == rhs._zoneANoDec) && (_zoneAPercent == rhs._zoneAPercent) &&
            (_zoneBNoDec == rhs._zoneBNoDec) && (_zoneBPercent == rhs._zoneBPercent) &&
            (_zoneCNoDec == rhs._zoneCNoDec) && (_zoneCPercent == rhs._zoneCPercent) &&
            (_zoneDNoDec == rhs._zoneDNoDec) && (_zoneDPercent == rhs._zoneDPercent) &&
            (_hawaiiNoDec == rhs._hawaiiNoDec) && (_hawaiiPercent == rhs._hawaiiPercent));
  }

  static void dummyData(TaxAkHiFactor& obj)
  {
    obj.city() = "ABCDEFGH";
    obj.versionDate() = time(nullptr);
    obj.createDate() = time(nullptr);
    obj.effDate() = time(nullptr);
    obj.discDate() = time(nullptr);
    obj.expireDate() = time(nullptr);
    obj.zoneANoDec() = 1;
    obj.zoneAPercent() = 2.1;
    obj.zoneBNoDec() = 3;
    obj.zoneBPercent() = 4.2;
    obj.zoneCNoDec() = 5;
    obj.zoneCPercent() = 6.3;
    obj.zoneDNoDec() = 7;
    obj.zoneDPercent() = 8.4;
    obj.hawaiiNoDec() = 9;
    obj.hawaiiPercent() = 10.5;
  }

private:
  LocCode _city;
  DateTime _versionDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  int _zoneANoDec;
  Percent _zoneAPercent;
  int _zoneBNoDec;
  Percent _zoneBPercent;
  int _zoneCNoDec;
  Percent _zoneCPercent;
  int _zoneDNoDec;
  Percent _zoneDPercent;
  int _hawaiiNoDec;
  Percent _hawaiiPercent;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _city);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _zoneANoDec);
    FLATTENIZE(archive, _zoneAPercent);
    FLATTENIZE(archive, _zoneBNoDec);
    FLATTENIZE(archive, _zoneBPercent);
    FLATTENIZE(archive, _zoneCNoDec);
    FLATTENIZE(archive, _zoneCPercent);
    FLATTENIZE(archive, _zoneDNoDec);
    FLATTENIZE(archive, _zoneDPercent);
    FLATTENIZE(archive, _hawaiiNoDec);
    FLATTENIZE(archive, _hawaiiPercent);
  }
};
}
