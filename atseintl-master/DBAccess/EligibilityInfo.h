//----------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class EligibilityInfo : public RuleItemInfo
{
public:
  EligibilityInfo()
    : _minAge(0),
      _maxAge(0),
      _firstOccur(0),
      _lastOccur(0),
      _unavailTag(' '),
      _psgId(' '),
      _psgAppl(' '),
      _psgStatus(' '),
      _inhibit(' ')
  {
  }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  int& firstOccur() { return _firstOccur; }
  const int& firstOccur() const { return _firstOccur; }

  int& lastOccur() { return _lastOccur; }
  const int& lastOccur() const { return _lastOccur; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  Indicator& psgId() { return _psgId; }
  const Indicator& psgId() const { return _psgId; }

  Indicator& psgAppl() { return _psgAppl; }
  const Indicator& psgAppl() const { return _psgAppl; }

  Indicator& psgStatus() { return _psgStatus; }
  const Indicator& psgStatus() const { return _psgStatus; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  AccountCode& acctCode() { return _acctCode; }
  const AccountCode& acctCode() const { return _acctCode; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const EligibilityInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_minAge == rhs._minAge) &&
            (_maxAge == rhs._maxAge) && (_firstOccur == rhs._firstOccur) &&
            (_lastOccur == rhs._lastOccur) && (_unavailTag == rhs._unavailTag) &&
            (_psgType == rhs._psgType) && (_psgId == rhs._psgId) && (_psgAppl == rhs._psgAppl) &&
            (_psgStatus == rhs._psgStatus) && (_loc1 == rhs._loc1) &&
            (_acctCode == rhs._acctCode) && (_inhibit == rhs._inhibit));
  }

  virtual WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(EligibilityInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._minAge = 1;
    obj._maxAge = 2;
    obj._firstOccur = 3;
    obj._lastOccur = 4;
    obj._unavailTag = 'A';
    obj._psgType = "BCD";
    obj._psgId = 'E';
    obj._psgAppl = 'F';
    obj._psgStatus = 'G';

    LocKey::dummyData(obj._loc1);

    obj._acctCode = "aaaaaaaa";
    obj._inhibit = 'H';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  int _minAge;
  int _maxAge;
  int _firstOccur;
  int _lastOccur;
  Indicator _unavailTag;
  PaxTypeCode _psgType;
  Indicator _psgId;
  Indicator _psgAppl;
  Indicator _psgStatus;
  LocKey _loc1;
  AccountCode _acctCode;
  Indicator _inhibit;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_expireDate & ptr->_minAge & ptr->_maxAge &
           ptr->_firstOccur & ptr->_lastOccur & ptr->_unavailTag & ptr->_psgType & ptr->_psgId &
           ptr->_psgAppl & ptr->_psgStatus & ptr->_loc1 & ptr->_acctCode & ptr->_inhibit;
  }

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _firstOccur);
    FLATTENIZE(archive, _lastOccur);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _psgId);
    FLATTENIZE(archive, _psgAppl);
    FLATTENIZE(archive, _psgStatus);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _acctCode);
    FLATTENIZE(archive, _inhibit);
  }

};
}
