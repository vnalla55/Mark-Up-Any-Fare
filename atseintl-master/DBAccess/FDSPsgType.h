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

class FDSPsgType
{

public:
  FDSPsgType()
    : _userApplType(' '),
      _pseudoCityType(' '),
      _ssgGroupNo(0),
      _fareDisplayType(' '),
      _domIntlAppl(' '),
      _seqno(0),
      _orderNo(0)
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

  Indicator& fareDisplayType() { return _fareDisplayType; }
  const Indicator& fareDisplayType() const { return _fareDisplayType; }

  Indicator& domIntlAppl() { return _domIntlAppl; }
  const Indicator& domIntlAppl() const { return _domIntlAppl; }

  uint64_t& seqno() { return _seqno; }
  const uint64_t& seqno() const { return _seqno; }

  uint64_t& orderNo() { return _orderNo; }
  const uint64_t& orderNo() const { return _orderNo; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  bool operator==(const FDSPsgType& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_fareDisplayType == rhs._fareDisplayType) &&
            (_domIntlAppl == rhs._domIntlAppl) && (_seqno == rhs._seqno) &&
            (_orderNo == rhs._orderNo) && (_psgType == rhs._psgType));
  }

  static void dummyData(FDSPsgType& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._ssgGroupNo = 1;
    obj._fareDisplayType = 'L';
    obj._domIntlAppl = 'M';
    obj._seqno = 2;
    obj._orderNo = 3;
    obj._psgType = "NOP";
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  TJRGroup _ssgGroupNo;
  Indicator _fareDisplayType;
  Indicator _domIntlAppl;
  uint64_t _seqno;
  uint64_t _orderNo;
  PaxTypeCode _psgType;

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
    FLATTENIZE(archive, _seqno);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _psgType);
  }

};
}

