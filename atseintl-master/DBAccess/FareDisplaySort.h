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

class FareDisplaySort
{

public:
  FareDisplaySort()
    : _userApplType(' '),
      _pseudoCityType(' '),
      _ssgGroupNo(0),
      _fareDisplayType(' '),
      _domIntlAppl(' '),
      _seqno(0),
      _newSeqno(0),
      _versionInheritedInd(' '),
      _versionDisplayInd(' '),
      _sortByGlobalDir(' '),
      _sortByRouting(' '),
      _sortByNMLSpecial(' '),
      _sortByOWRT(' '),
      _sortByFareBasis(' '),
      _sortByFareBasisCharComb(' '),
      _sortByPsgType(' '),
      _sortByExpireDate(' '),
      _sortByPublicPrivate(' '),
      _sortByAmount(' '),
      _doubleOWFares(' ')
  {
  }

  ~FareDisplaySort() {}

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

  uint64_t& newSeqno() { return _newSeqno; }
  const uint64_t& newSeqno() const { return _newSeqno; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& versionInheritedInd() { return _versionInheritedInd; }
  const Indicator& versionInheritedInd() const { return _versionInheritedInd; }

  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  const Indicator& versionDisplayInd() const { return _versionDisplayInd; }

  Indicator& sortByGlobalDir() { return _sortByGlobalDir; }
  const Indicator& sortByGlobalDir() const { return _sortByGlobalDir; }

  Indicator& sortByRouting() { return _sortByRouting; }
  const Indicator& sortByRouting() const { return _sortByRouting; }

  Indicator& sortByNMLSpecial() { return _sortByNMLSpecial; }
  const Indicator& sortByNMLSpecial() const { return _sortByNMLSpecial; }

  Indicator& sortByOWRT() { return _sortByOWRT; }
  const Indicator& sortByOWRT() const { return _sortByOWRT; }

  Indicator& sortByFareBasis() { return _sortByFareBasis; }
  const Indicator& sortByFareBasis() const { return _sortByFareBasis; }

  Indicator& sortByFareBasisCharComb() { return _sortByFareBasisCharComb; }
  const Indicator& sortByFareBasisCharComb() const { return _sortByFareBasisCharComb; }

  Indicator& sortByPsgType() { return _sortByPsgType; }
  const Indicator& sortByPsgType() const { return _sortByPsgType; }

  Indicator& sortByExpireDate() { return _sortByExpireDate; }
  const Indicator& sortByExpireDate() const { return _sortByExpireDate; }

  Indicator& sortByPublicPrivate() { return _sortByPublicPrivate; }
  const Indicator& sortByPublicPrivate() const { return _sortByPublicPrivate; }

  Indicator& sortByAmount() { return _sortByAmount; }
  const Indicator& sortByAmount() const { return _sortByAmount; }

  Indicator& doubleOWFares() { return _doubleOWFares; }
  const Indicator& doubleOWFares() const { return _doubleOWFares; }

  bool operator==(const FareDisplaySort& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_fareDisplayType == rhs._fareDisplayType) &&
            (_domIntlAppl == rhs._domIntlAppl) && (_versionDate == rhs._versionDate) &&
            (_seqno == rhs._seqno) && (_newSeqno == rhs._newSeqno) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_versionInheritedInd == rhs._versionInheritedInd) &&
            (_versionDisplayInd == rhs._versionDisplayInd) &&
            (_sortByGlobalDir == rhs._sortByGlobalDir) && (_sortByRouting == rhs._sortByRouting) &&
            (_sortByNMLSpecial == rhs._sortByNMLSpecial) && (_sortByOWRT == rhs._sortByOWRT) &&
            (_sortByFareBasis == rhs._sortByFareBasis) &&
            (_sortByFareBasisCharComb == rhs._sortByFareBasisCharComb) &&
            (_sortByPsgType == rhs._sortByPsgType) &&
            (_sortByExpireDate == rhs._sortByExpireDate) &&
            (_sortByPublicPrivate == rhs._sortByPublicPrivate) &&
            (_sortByAmount == rhs._sortByAmount) && (_doubleOWFares == rhs._doubleOWFares));
  }

  static void dummyData(FareDisplaySort& obj)
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
    obj._newSeqno = 3;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._versionInheritedInd = 'N';
    obj._versionDisplayInd = 'O';
    obj._sortByGlobalDir = 'P';
    obj._sortByRouting = 'Q';
    obj._sortByNMLSpecial = 'R';
    obj._sortByOWRT = 'S';
    obj._sortByFareBasis = 'T';
    obj._sortByFareBasisCharComb = 'U';
    obj._sortByPsgType = 'V';
    obj._sortByExpireDate = 'W';
    obj._sortByPublicPrivate = 'X';
    obj._sortByAmount = 'Y';
    obj._doubleOWFares = 'Z';
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  TJRGroup _ssgGroupNo;
  Indicator _fareDisplayType;
  Indicator _domIntlAppl;
  DateTime _versionDate;
  uint64_t _seqno;
  uint64_t _newSeqno;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _versionInheritedInd;
  Indicator _versionDisplayInd;
  Indicator _sortByGlobalDir;
  Indicator _sortByRouting;
  Indicator _sortByNMLSpecial;
  Indicator _sortByOWRT;
  Indicator _sortByFareBasis;
  Indicator _sortByFareBasisCharComb;
  Indicator _sortByPsgType;
  Indicator _sortByExpireDate;
  Indicator _sortByPublicPrivate;
  Indicator _sortByAmount;
  Indicator _doubleOWFares;

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
    FLATTENIZE(archive, _newSeqno);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _versionInheritedInd);
    FLATTENIZE(archive, _versionDisplayInd);
    FLATTENIZE(archive, _sortByGlobalDir);
    FLATTENIZE(archive, _sortByRouting);
    FLATTENIZE(archive, _sortByNMLSpecial);
    FLATTENIZE(archive, _sortByOWRT);
    FLATTENIZE(archive, _sortByFareBasis);
    FLATTENIZE(archive, _sortByFareBasisCharComb);
    FLATTENIZE(archive, _sortByPsgType);
    FLATTENIZE(archive, _sortByExpireDate);
    FLATTENIZE(archive, _sortByPublicPrivate);
    FLATTENIZE(archive, _sortByAmount);
    FLATTENIZE(archive, _doubleOWFares);
  }

};
}

