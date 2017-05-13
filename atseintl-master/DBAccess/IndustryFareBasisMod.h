//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class IndustryFareBasisMod
{
public:
  IndustryFareBasisMod() : _userApplType(' '), _doNotChangeFareBasisInd(' ') {}
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& doNotChangeFareBasisInd() { return _doNotChangeFareBasisInd; }
  const Indicator& doNotChangeFareBasisInd() const { return _doNotChangeFareBasisInd; }

  bool operator==(const IndustryFareBasisMod& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_userApplType == rhs._userApplType) &&
            (_userAppl == rhs._userAppl) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) &&
            (_doNotChangeFareBasisInd == rhs._doNotChangeFareBasisInd));
  }

  static void dummyData(IndustryFareBasisMod& obj)
  {
    obj._carrier = "ABC";
    obj._userApplType = 'D';
    obj._userAppl = "EFGH";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._doNotChangeFareBasisInd = 'I';
  }

private:
  CarrierCode _carrier;
  Indicator _userApplType;
  UserApplCode _userAppl;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _doNotChangeFareBasisInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _doNotChangeFareBasisInd);
  }

};
}

