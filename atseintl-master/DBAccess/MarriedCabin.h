#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class MarriedCabin
{
public:
  MarriedCabin()
    : _seqNo(0),
      _newSeqNo(0),
      _memoNo(0),
      _status(' '),
      _directionality(' '),
      _versionDisplayInd(' '),
      _versionInheritedInd(' ')
  {
  }

  // accessors and corruptors
  ///////////////////////////
  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  const BookingCode& premiumCabin() const { return _premiumCabin; }
  BookingCode& premiumCabin() { return _premiumCabin; }
  const BookingCode& marriedCabin() const { return _marriedCabin; }
  BookingCode& marriedCabin() { return _marriedCabin; }

  const DateTime& versionDate() const { return _versionDate; }
  DateTime& versionDate() { return _versionDate; }
  const DateTime& lockDate() const { return _lockDate; }
  DateTime& lockDate() { return _lockDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& createDate() { return _createDate; }
  const DateTime& expireDate() const { return _expireDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& effDate() const { return _effDate; }
  DateTime& effDate() { return _effDate; }
  const DateTime& discDate() const { return _discDate; }
  DateTime& discDate() { return _discDate; }

  std::string creatorId() const { return _creatorId; }
  std::string& creatorId() { return _creatorId; }
  std::string creatorBusinessUnit() const { return _creatorId; }
  std::string& creatorBusinessUnit() { return _creatorId; }

  int seqNo() const { return _seqNo; }
  int& seqNo() { return _seqNo; }
  int newSeqNo() const { return _newSeqNo; }
  int& newSeqNo() { return _newSeqNo; }
  int memoNo() const { return _memoNo; }
  int& memoNo() { return _memoNo; }

  const LocKey& loc1() const { return _loc1; }
  LocKey& loc1() { return _loc1; }
  const LocKey& loc2() const { return _loc2; }
  LocKey& loc2() { return _loc2; }

  Indicator directionality() const { return _directionality; }
  Indicator& directionality() { return _directionality; }
  Indicator versionDisplayInd() const { return _versionDisplayInd; }
  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  Indicator versionInheritedInd() const { return _versionInheritedInd; }
  Indicator& versionInheritedInd() { return _versionInheritedInd; }

  bool operator==(const MarriedCabin& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_premiumCabin == rhs._premiumCabin) &&
            (_marriedCabin == rhs._marriedCabin) && (_versionDate == rhs._versionDate) &&
            (_lockDate == rhs._lockDate) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_versionAppl == rhs._versionAppl) &&
            (_creatorId == rhs._creatorId) && (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
            (_seqNo == rhs._seqNo) && (_newSeqNo == rhs._newSeqNo) && (_memoNo == rhs._memoNo) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_status == rhs._status) &&
            (_directionality == rhs._directionality) &&
            (_versionDisplayInd == rhs._versionDisplayInd) &&
            (_versionInheritedInd == rhs._versionInheritedInd));
  }

  static void dummyData(MarriedCabin& obj)
  {
    obj._carrier = "ABC";
    obj._premiumCabin = "DE";
    obj._marriedCabin = "FG";
    obj._versionDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._versionAppl = time(nullptr);
    obj._creatorId = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._seqNo = 1;
    obj._newSeqNo = 2;
    obj._memoNo = 3;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._status = 'H';
    obj._directionality = 'I';
    obj._versionDisplayInd = 'J';
    obj._versionInheritedInd = 'K';
  }

private:
  CarrierCode _carrier;
  BookingCode _premiumCabin;
  BookingCode _marriedCabin;
  DateTime _versionDate;
  DateTime _lockDate;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _versionAppl;
  std::string _creatorId;
  std::string _creatorBusinessUnit;
  int _seqNo;
  int _newSeqNo;
  int _memoNo;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _status;
  Indicator _directionality;
  Indicator _versionDisplayInd;
  Indicator _versionInheritedInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _premiumCabin);
    FLATTENIZE(archive, _marriedCabin);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _versionAppl);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _newSeqNo);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _status);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _versionDisplayInd);
    FLATTENIZE(archive, _versionInheritedInd);
  }
};
} // namespace tse
