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
#include "Common/TseStringTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareDispCldInfPsgType
{

public:
  FareDispCldInfPsgType() : _userApplType(' '), _pseudoCityType(' '), _psgTypeInd(' ') {}

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& pseudoCityType() { return _pseudoCityType; }
  const Indicator& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  InclusionCode& inclusionCode() { return _inclusionCode; }
  const InclusionCode& inclusionCode() const { return _inclusionCode; }

  Indicator& psgTypeInd() { return _psgTypeInd; }
  const Indicator& psgTypeInd() const { return _psgTypeInd; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  bool operator==(const FareDispCldInfPsgType& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_inclusionCode == rhs._inclusionCode) && (_psgTypeInd == rhs._psgTypeInd) &&
            (_psgType == rhs._psgType));
  }

  static void dummyData(FareDispCldInfPsgType& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._inclusionCode = "LMNO";
    obj._psgTypeInd = 'P';
    obj._psgType = "QRS";
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  InclusionCode _inclusionCode;
  Indicator _psgTypeInd;
  PaxTypeCode _psgType;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _inclusionCode);
    FLATTENIZE(archive, _psgTypeInd);
    FLATTENIZE(archive, _psgType);
  }

};
}

