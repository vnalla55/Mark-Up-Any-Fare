//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

#include <log4cxx/helpers/objectptr.h>

#include <map>
#include <set>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

using std::string;

class FareTypeQualMsg
{
public:
  FareTypeQualMsg() : _fareTypeReqInd(' '), _groupTrailerMsgInd(' '), _itTrailerMsgInd(' ') {}
  ~FareTypeQualMsg() {}

  Indicator fareTypeReqInd() const { return _fareTypeReqInd; }
  Indicator groupTrailerMsgInd() const { return _groupTrailerMsgInd; }
  Indicator itTrailerMsgInd() const { return _itTrailerMsgInd; }

  Indicator& fareTypeReqInd() { return _fareTypeReqInd; }
  Indicator& groupTrailerMsgInd() { return _groupTrailerMsgInd; }
  Indicator& itTrailerMsgInd() { return _itTrailerMsgInd; }

  bool operator==(const FareTypeQualMsg& rhs) const
  {
    return ((_fareTypeReqInd == rhs._fareTypeReqInd) &&
            (_groupTrailerMsgInd == rhs._groupTrailerMsgInd) &&
            (_itTrailerMsgInd == rhs._itTrailerMsgInd));
  }

  static void dummyData(FareTypeQualMsg& obj)
  {
    obj._fareTypeReqInd = 'A';
    obj._groupTrailerMsgInd = 'B';
    obj._itTrailerMsgInd = 'C';
  }

private:
  Indicator _fareTypeReqInd;
  Indicator _groupTrailerMsgInd;
  Indicator _itTrailerMsgInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareTypeReqInd);
    FLATTENIZE(archive, _groupTrailerMsgInd);
    FLATTENIZE(archive, _itTrailerMsgInd);
  }

private:
};

class FareTypeQualifier
{
  friend class QueryGetFareTypeQualifier;
  friend class QueryGetAllFareTypeQualifier;
  friend class QueryGetFareTypeQualifierHistorical;
  friend class QueryGetAllFareTypeQualifierHistorical;

public:
  FareTypeQualifier()
    : _userApplType(' '),
      _journeyTypeDom(' '),
      _journeyTypeIntl(' '),
      _journeyTypeEoe(' '),
      _pricingUnitDom(' '),
      _pricingUnitIntl(' '),
      _memoNo(0)
  {
  }
  ~FareTypeQualifier() {}

  const Indicator& userApplType() const { return _userApplType; }
  const UserApplCode& userAppl() const { return _userAppl; }
  const FareType& fareTypeQualifier() const { return _fareTypeQualifier; }
  const Indicator& journeyTypeDom() const { return _journeyTypeDom; }
  const Indicator& journeyTypeIntl() const { return _journeyTypeIntl; }
  const Indicator& journeyTypeEoe() const { return _journeyTypeEoe; }
  const Indicator& pricingUnitDom() const { return _pricingUnitDom; }
  const Indicator& pricingUnitIntl() const { return _pricingUnitIntl; }
  const DateTime& createDate() const { return _createDate; }
  const DateTime& expireDate() const { return _expireDate; }
  const DateTime& lockDate() const { return _lockDate; }
  const DateTime& effDate() const { return _effDate; }
  const DateTime& discDate() const { return _discDate; }
  const int& memoNo() const { return _memoNo; }
  const string& creatorId() const { return _creatorId; }
  const string& creatorBusinessUnit() const { return _creatorBusinessUnit; }

  const std::map<PaxTypeCode, FareTypeQualMsg>& qualifierMgs() const { return _qualMsgMap; }
  const std::set<PaxTypeCode>& fareTypeRequired() const { return _fareTypeRequired; }
  const std::set<PaxTypeCode>& psgType() const { return _psgTypeSet; }

  Indicator& userApplType() { return _userApplType; }
  UserApplCode& userAppl() { return _userAppl; }
  FareType& fareTypeQualifier() { return _fareTypeQualifier; }
  Indicator& journeyTypeDom() { return _journeyTypeDom; }
  Indicator& journeyTypeIntl() { return _journeyTypeIntl; }
  Indicator& journeyTypeEoe() { return _journeyTypeEoe; }
  Indicator& pricingUnitDom() { return _pricingUnitDom; }
  Indicator& pricingUnitIntl() { return _pricingUnitIntl; }
  DateTime& createDate() { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  DateTime& lockDate() { return _lockDate; }
  DateTime& effDate() { return _effDate; }
  DateTime& discDate() { return _discDate; }
  int& memoNo() { return _memoNo; }
  string& creatorId() { return _creatorId; }
  string& creatorBusinessUnit() { return _creatorBusinessUnit; }

  void addQualMsg(const FareType& fareType, const FareTypeQualMsg& qualMsg)
  {
    _qualMsgMap.insert(std::make_pair(fareType.c_str(), qualMsg));
    if (qualMsg.fareTypeReqInd() == 'Y')
    {
      _fareTypeRequired.insert(fareType.c_str());
    }
  }

  void addPsgType(const PaxTypeCode& psgType) { _psgTypeSet.insert(psgType); }

  bool operator==(const FareTypeQualifier& rhs) const
  {
    return (
        (_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
        (_fareTypeQualifier == rhs._fareTypeQualifier) &&
        (_journeyTypeDom == rhs._journeyTypeDom) && (_journeyTypeIntl == rhs._journeyTypeIntl) &&
        (_journeyTypeEoe == rhs._journeyTypeEoe) && (_pricingUnitDom == rhs._pricingUnitDom) &&
        (_pricingUnitIntl == rhs._pricingUnitIntl) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_lockDate == rhs._lockDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) && (_memoNo == rhs._memoNo) &&
        (_creatorId == rhs._creatorId) && (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
        (_qualMsgMap == rhs._qualMsgMap) && (_fareTypeRequired == rhs._fareTypeRequired) &&
        (_psgTypeSet == rhs._psgTypeSet));
  }

  static void dummyData(FareTypeQualifier& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._fareTypeQualifier = "FGHIJKLM";
    obj._journeyTypeDom = 'O';
    obj._journeyTypeIntl = 'P';
    obj._journeyTypeEoe = 'Q';
    obj._pricingUnitDom = 'R';
    obj._pricingUnitIntl = 'S';
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._memoNo = 1;
    obj._creatorId = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";

    FareTypeQualMsg::dummyData(obj._qualMsgMap["TUV"]);
    FareTypeQualMsg::dummyData(obj._qualMsgMap["WXY"]);

    obj._fareTypeRequired.insert("Zab");
    obj._fareTypeRequired.insert("cde");

    obj._psgTypeSet.insert("fgh");
    obj._psgTypeSet.insert("ijk");
  }

private:
  FareTypeQualifier(const FareTypeQualifier&);
  FareTypeQualifier& operator=(const FareTypeQualifier&);

  bool operator!=(const FareTypeQualifier& rhs) const;

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  FareType _fareTypeQualifier;
  Indicator _journeyTypeDom;
  Indicator _journeyTypeIntl;
  Indicator _journeyTypeEoe;
  Indicator _pricingUnitDom;
  Indicator _pricingUnitIntl;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _lockDate;
  DateTime _effDate;
  DateTime _discDate;
  int _memoNo;
  string _creatorId;
  string _creatorBusinessUnit;
  std::map<PaxTypeCode, FareTypeQualMsg> _qualMsgMap;
  std::set<PaxTypeCode> _fareTypeRequired;
  std::set<PaxTypeCode> _psgTypeSet;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _fareTypeQualifier);
    FLATTENIZE(archive, _journeyTypeDom);
    FLATTENIZE(archive, _journeyTypeIntl);
    FLATTENIZE(archive, _journeyTypeEoe);
    FLATTENIZE(archive, _pricingUnitDom);
    FLATTENIZE(archive, _pricingUnitIntl);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _qualMsgMap);
    FLATTENIZE(archive, _fareTypeRequired);
    FLATTENIZE(archive, _psgTypeSet);
  }
};
}

