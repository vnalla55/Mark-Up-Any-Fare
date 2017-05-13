#pragma once
//----------------------------------------------------------------------------
// Limitation.h
//
// Copyright Sabre 2004
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class LimitationCmn
{
public:
  virtual ~LimitationCmn() = default;

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  void setSeqNo(int seqNo) { _seqNo = seqNo; }
  int seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& limitationAppl() { return _limitationAppl; }
  const Indicator& limitationAppl() const { return _limitationAppl; }

  Indicator& exceptTktgCxrInd() { return _exceptTktgCxrInd; }
  const Indicator& exceptTktgCxrInd() const { return _exceptTktgCxrInd; }

  std::vector<CarrierCode>& tktgCarriers() { return _tktgCarriers; }
  const std::vector<CarrierCode>& tktgCarriers() const { return _tktgCarriers; }

  Indicator& whollyWithinAppl() { return _whollyWithinAppl; }
  const Indicator& whollyWithinAppl() const { return _whollyWithinAppl; }

  LocKey& whollyWithinLoc() { return _whollyWithinLoc; }
  const LocKey& whollyWithinLoc() const { return _whollyWithinLoc; }

  Indicator& originTvlAppl() { return _originTvlAppl; }
  const Indicator& originTvlAppl() const { return _originTvlAppl; }

  LocKey& originLoc() { return _originLoc; }
  const LocKey& originLoc() const { return _originLoc; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  int32_t& intlDepartMaxNo() { return _intlDepartMaxNo; }
  const int32_t& intlDepartMaxNo() const { return _intlDepartMaxNo; }

  int32_t& intlArrivalMaxNo() { return _intlArrivalMaxNo; }
  const int32_t& intlArrivalMaxNo() const { return _intlArrivalMaxNo; }

  int32_t& maxRetransitAllowed() { return _maxRetransitAllowed; }
  const int32_t& maxRetransitAllowed() const { return _maxRetransitAllowed; }

  Indicator& retransitPointAppl() { return _retransitPointAppl; }
  const Indicator& retransitPointAppl() const { return _retransitPointAppl; }

  LocKey& retransitLoc() { return _retransitLoc; }
  const LocKey& retransitLoc() const { return _retransitLoc; }

  int32_t& maxStopsAtRetransit() { return _maxStopsAtRetransit; }
  const int32_t& maxStopsAtRetransit() const { return _maxStopsAtRetransit; }

  Indicator& viaPointStopoverAppl() { return _viaPointStopoverAppl; }
  const Indicator& viaPointStopoverAppl() const { return _viaPointStopoverAppl; }

  LocKey& stopoverLoc() { return _stopoverLoc; }
  const LocKey& stopoverLoc() const { return _stopoverLoc; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  LocKey& potLoc() { return _potLoc; }
  const LocKey& potLoc() const { return _potLoc; }

  virtual bool operator==(const LimitationCmn& rhs) const
  {
    return (
        (_versionDate == rhs._versionDate) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
        (_userAppl == rhs._userAppl) && (_limitationAppl == rhs._limitationAppl) &&
        (_exceptTktgCxrInd == rhs._exceptTktgCxrInd) && (_tktgCarriers == rhs._tktgCarriers) &&
        (_whollyWithinAppl == rhs._whollyWithinAppl) &&
        (_whollyWithinLoc == rhs._whollyWithinLoc) && (_originTvlAppl == rhs._originTvlAppl) &&
        (_originLoc == rhs._originLoc) && (_fareType == rhs._fareType) &&
        (_intlDepartMaxNo == rhs._intlDepartMaxNo) &&
        (_intlArrivalMaxNo == rhs._intlArrivalMaxNo) &&
        (_maxRetransitAllowed == rhs._maxRetransitAllowed) &&
        (_retransitPointAppl == rhs._retransitPointAppl) && (_retransitLoc == rhs._retransitLoc) &&
        (_maxStopsAtRetransit == rhs._maxStopsAtRetransit) &&
        (_viaPointStopoverAppl == rhs._viaPointStopoverAppl) &&
        (_stopoverLoc == rhs._stopoverLoc) && (_posLoc == rhs._posLoc) && (_potLoc == rhs._potLoc));
  }

  static void dummyData(LimitationCmn& obj)
  {
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._userAppl = "zzzz";
    obj._limitationAppl = 'E';
    obj._exceptTktgCxrInd = 'F';
    obj._tktgCarriers.push_back("GHI");
    obj._tktgCarriers.push_back("KLM");
    obj._whollyWithinAppl = 'N';
    LocKey::dummyData(obj._whollyWithinLoc);
    obj._originTvlAppl = 'O';
    LocKey::dummyData(obj._originLoc);
    obj._fareType = "PQRSTUVW";
    obj._intlDepartMaxNo = 2;
    obj._intlArrivalMaxNo = 3;
    obj._maxRetransitAllowed = 4;
    obj._retransitPointAppl = 'X';
    LocKey::dummyData(obj._retransitLoc);
    obj._maxStopsAtRetransit = 5;
    obj._viaPointStopoverAppl = 'Y';
    LocKey::dummyData(obj._stopoverLoc);
    LocKey::dummyData(obj._posLoc);
    LocKey::dummyData(obj._potLoc);
  }

private:
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  UserApplCode _userAppl;
  Indicator _limitationAppl = ' ';
  Indicator _exceptTktgCxrInd = ' ';
  std::vector<CarrierCode> _tktgCarriers;
  Indicator _whollyWithinAppl = ' ';
  LocKey _whollyWithinLoc;
  Indicator _originTvlAppl = ' ';
  LocKey _originLoc;
  FareType _fareType;
  int32_t _intlDepartMaxNo = 0;
  int32_t _intlArrivalMaxNo = 0;
  int32_t _maxRetransitAllowed = 0;
  Indicator _retransitPointAppl = ' ';
  LocKey _retransitLoc;
  int32_t _maxStopsAtRetransit = 0;
  Indicator _viaPointStopoverAppl = ' ';
  LocKey _stopoverLoc;
  LocKey _posLoc;
  LocKey _potLoc;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _limitationAppl);
    FLATTENIZE(archive, _exceptTktgCxrInd);
    FLATTENIZE(archive, _tktgCarriers);
    FLATTENIZE(archive, _whollyWithinAppl);
    FLATTENIZE(archive, _whollyWithinLoc);
    FLATTENIZE(archive, _originTvlAppl);
    FLATTENIZE(archive, _originLoc);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _intlDepartMaxNo);
    FLATTENIZE(archive, _intlArrivalMaxNo);
    FLATTENIZE(archive, _maxRetransitAllowed);
    FLATTENIZE(archive, _retransitPointAppl);
    FLATTENIZE(archive, _retransitLoc);
    FLATTENIZE(archive, _maxStopsAtRetransit);
    FLATTENIZE(archive, _viaPointStopoverAppl);
    FLATTENIZE(archive, _stopoverLoc);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _potLoc);
  }
};
} // namespace tse
