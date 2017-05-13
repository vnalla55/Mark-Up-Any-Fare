//----------------------------------------------------------------------------
// � 2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareDisplayInclCd
{

public:
  FareDisplayInclCd()
    : _userApplType(' '),
      _pseudoCityType(' '),
      _memoNo(0),
      _exceptFareType(' '),
      _exceptPsgType(' '),
      _displTypeAndOrFareType(' '),
      _fareTypeAndOrPsgType(' '),
      _displTypeAndOrPsgType(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

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

  int& memoNo() { return _memoNo; }
  const int& memoNo() const { return _memoNo; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }

  Indicator& exceptFareType() { return _exceptFareType; }
  const Indicator& exceptFareType() const { return _exceptFareType; }

  Indicator& exceptPsgType() { return _exceptPsgType; }
  const Indicator& exceptPsgType() const { return _exceptPsgType; }

  Indicator& displTypeAndOrFareType() { return _displTypeAndOrFareType; }
  const Indicator& displTypeAndOrFareType() const { return _displTypeAndOrFareType; }

  Indicator& fareTypeAndOrPsgType() { return _fareTypeAndOrPsgType; }
  const Indicator& fareTypeAndOrPsgType() const { return _fareTypeAndOrPsgType; }

  Indicator& displTypeAndOrPsgType() { return _displTypeAndOrPsgType; }
  const Indicator& displTypeAndOrPsgType() const { return _displTypeAndOrPsgType; }

  std::string& creatorBusinessUnit() { return _creatorBusinessUnit; }
  const std::string& creatorBusinessUnit() const { return _creatorBusinessUnit; }

  std::string& creatorId() { return _creatorId; }
  const std::string& creatorId() const { return _creatorId; }

  bool operator==(const FareDisplayInclCd& rhs) const
  {
    return ((_createDate == rhs._createDate) && (_lockDate == rhs._lockDate) &&
            (_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_inclusionCode == rhs._inclusionCode) && (_memoNo == rhs._memoNo) &&
            (_description == rhs._description) && (_exceptFareType == rhs._exceptFareType) &&
            (_exceptPsgType == rhs._exceptPsgType) &&
            (_displTypeAndOrFareType == rhs._displTypeAndOrFareType) &&
            (_fareTypeAndOrPsgType == rhs._fareTypeAndOrPsgType) &&
            (_displTypeAndOrPsgType == rhs._displTypeAndOrPsgType) &&
            (_creatorBusinessUnit == rhs._creatorBusinessUnit) && (_creatorId == rhs._creatorId));
  }

  static void dummyData(FareDisplayInclCd& obj)
  {
    obj._createDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "FGHIJ";
    obj._inclusionCode = "KLMN";
    obj._memoNo = 1;
    obj._description = "aaaaaaaa";
    obj._exceptFareType = 'O';
    obj._exceptPsgType = 'P';
    obj._displTypeAndOrFareType = 'Q';
    obj._fareTypeAndOrPsgType = 'R';
    obj._displTypeAndOrPsgType = 'S';
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._creatorId = "cccccccc";
  }

private:
  DateTime _createDate;
  DateTime _lockDate;
  Indicator _userApplType;
  UserApplCode _userAppl;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  InclusionCode _inclusionCode;
  int _memoNo;
  Description _description;
  Indicator _exceptFareType;
  Indicator _exceptPsgType;
  Indicator _displTypeAndOrFareType;
  Indicator _fareTypeAndOrPsgType;
  Indicator _displTypeAndOrPsgType;
  std::string _creatorBusinessUnit;
  std::string _creatorId;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _inclusionCode);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _exceptFareType);
    FLATTENIZE(archive, _exceptPsgType);
    FLATTENIZE(archive, _displTypeAndOrFareType);
    FLATTENIZE(archive, _fareTypeAndOrPsgType);
    FLATTENIZE(archive, _displTypeAndOrPsgType);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _creatorId);
  }

};
}

