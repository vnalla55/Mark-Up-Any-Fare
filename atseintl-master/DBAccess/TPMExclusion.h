#pragma once
//----------------------------------------------------------------------------
// SouthAthlTPMExcl.h
//
// Copyright Sabre 2009
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <string>
#include <vector>

namespace tse
{

class TPMExclusion
{
public:
  // Primary Key Fields

  CarrierCode& carrier()
  {
    return _carrier;
  };
  const CarrierCode& carrier() const
  {
    return _carrier;
  };

  uint32_t& seqNo() { return _seqNo; }
  const uint32_t& seqNo() const { return _seqNo; }

  DateTime& createDate()
  {
    return _createDate;
  };
  const DateTime& createDate() const
  {
    return _createDate;
  };

  // Non-Key Data
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& notApplToYY() { return _notApplToYY; }
  const Indicator& notApplToYY() const { return _notApplToYY; }

  Indicator& onlineSrvOnly() { return _onlineSrvOnly; }
  const Indicator& onlineSrvOnly() const { return _onlineSrvOnly; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocTypeCode& loc1type() { return _loc1type; }
  const LocTypeCode& loc1type() const { return _loc1type; }

  LocCode& loc1() { return _loc1; }
  const LocCode& loc1() const { return _loc1; }

  LocTypeCode& loc2type() { return _loc2type; }
  const LocTypeCode& loc2type() const { return _loc2type; }

  LocCode& loc2() { return _loc2; }
  const LocCode& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Indicator& sec1Appl() { return _sec1Appl; }
  const Indicator& sec1Appl() const { return _sec1Appl; }

  LocTypeCode& sec1Loc1Type() { return _sec1Loc1Type; }
  const LocTypeCode& sec1Loc1Type() const { return _sec1Loc1Type; }

  LocCode& sec1Loc1() { return _sec1Loc1; }
  const LocCode& sec1Loc1() const { return _sec1Loc1; }

  LocTypeCode& sec1Loc2Type() { return _sec1Loc2Type; }
  const LocTypeCode& sec1Loc2Type() const { return _sec1Loc2Type; }

  LocCode& sec1Loc2() { return _sec1Loc2; }
  const LocCode& sec1Loc2() const { return _sec1Loc2; }

  Indicator& sec2Appl() { return _sec2Appl; }
  const Indicator& sec2Appl() const { return _sec2Appl; }

  LocTypeCode& sec2Loc1Type() { return _sec2Loc1Type; }
  const LocTypeCode& sec2Loc1Type() const { return _sec2Loc1Type; }

  LocCode& sec2Loc1() { return _sec2Loc1; }
  const LocCode& sec2Loc1() const { return _sec2Loc1; }

  LocTypeCode& sec2Loc2Type() { return _sec2Loc2Type; }
  const LocTypeCode& sec2Loc2Type() const { return _sec2Loc2Type; }

  LocCode& sec2Loc2() { return _sec2Loc2; }
  const LocCode& sec2Loc2() const { return _sec2Loc2; }

  Indicator& viaPointRest() { return _viaPointRest; }
  const Indicator& viaPointRest() const { return _viaPointRest; }

  Indicator& consecMustBeOnGovCxr() { return _consecMustBeOnGovCxr; }
  const Indicator& consecMustBeOnGovCxr() const { return _consecMustBeOnGovCxr; }

  Indicator& surfacePermitted() { return _surfacePermitted; }
  const Indicator& surfacePermitted() const { return _surfacePermitted; }

  bool operator==(const TPMExclusion& rhs) const
  {
    return (_carrier == rhs._carrier) && (_seqNo == rhs._seqNo) &&
           (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
           (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
           (_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
           (_notApplToYY == rhs._notApplToYY) && (_onlineSrvOnly == rhs._onlineSrvOnly) &&
           (_directionality == rhs._directionality) && (_loc1type == rhs._loc1type) &&
           (_loc1 == rhs._loc1) && (_loc2type == rhs._loc2type) && (_loc2 == rhs._loc2) &&
           (_globalDir == rhs._globalDir) && (_sec1Appl == rhs._sec1Appl) &&
           (_sec1Loc1Type == rhs._sec1Loc1Type) && (_sec1Loc1 == rhs._sec1Loc1) &&
           (_sec1Loc2Type == rhs._sec1Loc2Type) && (_sec1Loc2 == rhs._sec1Loc2) &&
           (_sec2Appl == rhs._sec2Appl) && (_sec2Loc1Type == rhs._sec2Loc1Type) &&
           (_sec2Loc1 == rhs._sec2Loc1) && (_sec2Loc2Type == rhs._sec2Loc2Type) &&
           (_sec2Loc2 == rhs._sec2Loc2) && (_viaPointRest == rhs._viaPointRest) &&
           (_consecMustBeOnGovCxr == rhs._consecMustBeOnGovCxr) &&
           (_surfacePermitted == rhs._surfacePermitted);
  }

  static void dummyData(TPMExclusion& obj)
  {
    obj._carrier = "AA";
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._userApplType = 'C';
    obj._userAppl = "BB";
    obj._notApplToYY = 'D';
    obj._onlineSrvOnly = 'E';
    obj._directionality = FROM;
    obj._loc1type = UNKNOWN_LOC;
    obj._loc1 = "XXX";
    obj._loc2type = UNKNOWN_LOC;
    obj._loc2 = "YYY";
    obj._globalDir = GlobalDirection::WH;
    obj._sec1Appl = 'H';
    obj._sec1Loc1Type = UNKNOWN_LOC;
    obj._sec1Loc1 = "ZZZ";
    obj._sec1Loc2Type = UNKNOWN_LOC;
    obj._sec1Loc2 = "QQQ";
    obj._sec2Appl = 'K';
    obj._sec2Loc1Type = UNKNOWN_LOC;
    obj._sec2Loc1 = "RRR";
    obj._sec2Loc2Type = UNKNOWN_LOC;
    obj._sec2Loc2 = "SSS";
    obj._viaPointRest = 'N';
    obj._consecMustBeOnGovCxr = 'O';
    obj._surfacePermitted = 'P';
  }

private:
  CarrierCode _carrier;
  uint32_t _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _notApplToYY;
  Indicator _onlineSrvOnly;
  Directionality _directionality;
  LocTypeCode _loc1type;
  LocCode _loc1;
  LocTypeCode _loc2type;
  LocCode _loc2;
  GlobalDirection _globalDir;
  Indicator _sec1Appl;
  LocTypeCode _sec1Loc1Type;
  LocCode _sec1Loc1;
  LocTypeCode _sec1Loc2Type;
  LocCode _sec1Loc2;
  Indicator _sec2Appl;
  LocTypeCode _sec2Loc1Type;
  LocCode _sec2Loc1;
  LocTypeCode _sec2Loc2Type;
  LocCode _sec2Loc2;
  Indicator _viaPointRest;
  Indicator _consecMustBeOnGovCxr;
  Indicator _surfacePermitted;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _notApplToYY);
    FLATTENIZE(archive, _onlineSrvOnly);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2type);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _sec1Appl);
    FLATTENIZE(archive, _sec1Loc1Type);
    FLATTENIZE(archive, _sec1Loc1);
    FLATTENIZE(archive, _sec1Loc2Type);
    FLATTENIZE(archive, _sec1Loc2);
    FLATTENIZE(archive, _sec2Appl);
    FLATTENIZE(archive, _sec2Loc1Type);
    FLATTENIZE(archive, _sec2Loc1);
    FLATTENIZE(archive, _sec2Loc2Type);
    FLATTENIZE(archive, _sec2Loc2);
    FLATTENIZE(archive, _viaPointRest);
    FLATTENIZE(archive, _consecMustBeOnGovCxr);
    FLATTENIZE(archive, _surfacePermitted);
  }
};

} // namespace tse

