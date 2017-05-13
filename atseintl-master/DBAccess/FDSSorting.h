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

class FDSSorting
{

public:
  FDSSorting()
    : _userApplType(' '),
      _pseudoCityType(' '),
      _ssgGroupNo(0),
      _fareDisplayType(' '),
      _domIntlAppl(' '),
      _seqno(0),
      _sortFareBasisChar1(' '),
      _sortFareBasisChar2(' '),
      _sortFareBasisChar3(' ')
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

  Indicator& sortFareBasisChar1() { return _sortFareBasisChar1; }
  const Indicator& sortFareBasisChar1() const { return _sortFareBasisChar1; }

  std::string& fareBasisChar1() { return _fareBasisChar1; }
  const std::string& fareBasisChar1() const { return _fareBasisChar1; }

  Indicator& sortFareBasisChar2() { return _sortFareBasisChar2; }
  const Indicator& sortFareBasisChar2() const { return _sortFareBasisChar2; }

  std::string& fareBasisChar2() { return _fareBasisChar2; }
  const std::string& fareBasisChar2() const { return _fareBasisChar2; }

  Indicator& sortFareBasisChar3() { return _sortFareBasisChar3; }
  const Indicator& sortFareBasisChar3() const { return _sortFareBasisChar3; }

  std::string& fareBasisChar3() { return _fareBasisChar3; }
  const std::string& fareBasisChar3() const { return _fareBasisChar3; }

  bool operator==(const FDSSorting& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_fareDisplayType == rhs._fareDisplayType) &&
            (_domIntlAppl == rhs._domIntlAppl) && (_seqno == rhs._seqno) &&
            (_sortFareBasisChar1 == rhs._sortFareBasisChar1) &&
            (_fareBasisChar1 == rhs._fareBasisChar1) &&
            (_sortFareBasisChar2 == rhs._sortFareBasisChar2) &&
            (_fareBasisChar2 == rhs._fareBasisChar2) &&
            (_sortFareBasisChar3 == rhs._sortFareBasisChar3) &&
            (_fareBasisChar3 == rhs._fareBasisChar3));
  }

  static void dummyData(FDSSorting& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._ssgGroupNo = 1;
    obj._fareDisplayType = 'L';
    obj._domIntlAppl = 'M';
    obj._seqno = 2;
    obj._sortFareBasisChar1 = 'N';
    obj._fareBasisChar1 = "aaaaaaaa";
    obj._sortFareBasisChar2 = 'O';
    obj._fareBasisChar2 = "bbbbbbbb";
    obj._sortFareBasisChar3 = 'P';
    obj._fareBasisChar3 = "cccccccc";
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
  Indicator _sortFareBasisChar1;
  std::string _fareBasisChar1;
  Indicator _sortFareBasisChar2;
  std::string _fareBasisChar2;
  Indicator _sortFareBasisChar3;
  std::string _fareBasisChar3;

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
    FLATTENIZE(archive, _sortFareBasisChar1);
    FLATTENIZE(archive, _fareBasisChar1);
    FLATTENIZE(archive, _sortFareBasisChar2);
    FLATTENIZE(archive, _fareBasisChar2);
    FLATTENIZE(archive, _sortFareBasisChar3);
    FLATTENIZE(archive, _fareBasisChar3);
  }

};
}

