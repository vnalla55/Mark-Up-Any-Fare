//----------------------------------------------------------------------------
//        File:           NationStateHistIsCurrChk.h
//        Description:    Just Date fields for Historical filtering in DAO
//        Created:        2/4/2004
//              Authors:        Roger Kelly
//
//        Updates:
//
//       @ 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class NationStateHistIsCurrChk
{
public:
  NationStateHistIsCurrChk() : _inclExclInd(' ') {}

  Indicator& inclExclInd() { return _inclExclInd; }
  const Indicator& inclExclInd() const { return _inclExclInd; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  bool operator==(const NationStateHistIsCurrChk& rhs) const
  {
    return ((_inclExclInd == rhs._inclExclInd) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate));
  }

  static void dummyData(NationStateHistIsCurrChk& obj)
  {
    obj._inclExclInd = 'A';
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
  }

private:
  Indicator _inclExclInd;
  DateTime _createDate;
  DateTime _expireDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _inclExclInd);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
  }

};
}
