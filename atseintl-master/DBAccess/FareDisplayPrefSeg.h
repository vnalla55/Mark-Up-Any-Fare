//----------------------------------------------------------------------------
// � 2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareDisplayPrefSeg
{
public:
  FareDisplayPrefSeg()
    : _userApplType(' '), _pseudoCityType(' '), _ssgGroupNo(0), _surchargeType(' ')
  {
  }
  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& pseudoCityType() { return _pseudoCityType; }
  const Indicator& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  TJRGroup& ssgGroupNo() { return _ssgGroupNo; }
  const TJRGroup& ssgGroupNo() const { return _ssgGroupNo; }

  Indicator& surchargeType() { return _surchargeType; }
  const Indicator& surchargeType() const { return _surchargeType; }

  bool operator==(const FareDisplayPrefSeg& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_surchargeType == rhs._surchargeType));
  }

  static void dummyData(FareDisplayPrefSeg& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._ssgGroupNo = 1;
    obj._surchargeType = 'L';
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  TJRGroup _ssgGroupNo;
  Indicator _surchargeType;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _ssgGroupNo);
    FLATTENIZE(archive, _surchargeType);
  }

};
}

