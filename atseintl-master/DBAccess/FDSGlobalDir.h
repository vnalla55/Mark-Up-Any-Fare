//----------------------------------------------------------------------------
//   2005, Sabre Inc.  All rights reserved.  This software/documentation is
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

class FDSGlobalDir
{
public:
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

  Indicator& fareDisplayType() { return _fareDisplayType; }
  const Indicator& fareDisplayType() const { return _fareDisplayType; }

  Indicator& domIntlAppl() { return _domIntlAppl; }
  const Indicator& domIntlAppl() const { return _domIntlAppl; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  uint64_t& seqno() { return _seqno; }
  const uint64_t& seqno() const { return _seqno; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  uint64_t& orderNo() { return _orderNo; }
  const uint64_t& orderNo() const { return _orderNo; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  bool operator==(const FDSGlobalDir& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_fareDisplayType == rhs._fareDisplayType) &&
            (_domIntlAppl == rhs._domIntlAppl) && (_versionDate == rhs._versionDate) &&
            (_seqno == rhs._seqno) && (_createDate == rhs._createDate) &&
            (_orderNo == rhs._orderNo) && (_globalDir == rhs._globalDir));
  }

  static void dummyData(FDSGlobalDir& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._ssgGroupNo = 1;
    obj._fareDisplayType = 'L';
    obj._domIntlAppl = 'M';
    obj._versionDate = time(nullptr);
    obj._seqno = 2;
    obj._createDate = time(nullptr);
    obj._orderNo = 3;
    obj._globalDir = GlobalDirection::US;
  }

private:
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Indicator _pseudoCityType = ' ';
  PseudoCityCode _pseudoCity;
  TJRGroup _ssgGroupNo = 0;
  Indicator _fareDisplayType = ' ';
  Indicator _domIntlAppl = ' ';
  DateTime _versionDate;
  uint64_t _seqno = 0;
  DateTime _createDate;
  uint64_t _orderNo = 0;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _ssgGroupNo);
    FLATTENIZE(archive, _fareDisplayType);
    FLATTENIZE(archive, _domIntlAppl);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqno);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _globalDir);
  }
};
}
