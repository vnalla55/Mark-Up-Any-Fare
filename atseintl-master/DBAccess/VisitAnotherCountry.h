//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class VisitAnotherCountry : public RuleItemInfo
{
public:
  VisitAnotherCountry()
    : _validityInd(' '),
      _inhibit(' '),
      _unavailTag(' '),
      _idRequiredInd(' '),
      _minAge(0),
      _maxAge(0),
      _residentInd(' '),
      _resGeoTblItemNo(0),
      _travelInd(' '),
      _miles(0),
      _ticket(' '),
      _timePeriod(0),
      _timeUnit(' '),
      _issueInd(' '),
      _issueGeoTblItemNo(0)
  {
  }

  virtual ~VisitAnotherCountry() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  Indicator& idRequiredInd() { return _idRequiredInd; }
  const Indicator& idRequiredInd() const { return _idRequiredInd; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  Indicator& residentInd() { return _residentInd; }
  const Indicator& residentInd() const { return _residentInd; }

  int& resGeoTblItemNo() { return _resGeoTblItemNo; }
  const int& resGeoTblItemNo() const { return _resGeoTblItemNo; }

  Indicator& travelInd() { return _travelInd; }
  const Indicator& travelInd() const { return _travelInd; }

  int& miles() { return _miles; }
  const int& miles() const { return _miles; }

  Indicator& ticket() { return _ticket; }
  const Indicator& ticket() const { return _ticket; }

  int& timePeriod() { return _timePeriod; }
  const int& timePeriod() const { return _timePeriod; }

  Indicator& timeUnit() { return _timeUnit; }
  const Indicator& timeUnit() const { return _timeUnit; }

  Indicator& issueInd() { return _issueInd; }
  const Indicator& issueInd() const { return _issueInd; }

  int& issueGeoTblItemNo() { return _issueGeoTblItemNo; }
  const int& issueGeoTblItemNo() const { return _issueGeoTblItemNo; }

  virtual bool operator==(const VisitAnotherCountry& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_validityInd == rhs._validityInd) &&
            (_inhibit == rhs._inhibit) && (_unavailTag == rhs._unavailTag) &&
            (_psgType == rhs._psgType) && (_idRequiredInd == rhs._idRequiredInd) &&
            (_minAge == rhs._minAge) && (_maxAge == rhs._maxAge) &&
            (_residentInd == rhs._residentInd) && (_resGeoTblItemNo == rhs._resGeoTblItemNo) &&
            (_travelInd == rhs._travelInd) && (_miles == rhs._miles) && (_ticket == rhs._ticket) &&
            (_timePeriod == rhs._timePeriod) && (_timeUnit == rhs._timeUnit) &&
            (_issueInd == rhs._issueInd) && (_issueGeoTblItemNo == rhs._issueGeoTblItemNo));
  }

  static void dummyData(VisitAnotherCountry& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'A';
    obj._inhibit = 'B';
    obj._unavailTag = 'C';
    obj._psgType = "DEF";
    obj._idRequiredInd = 'H';
    obj._minAge = 1;
    obj._maxAge = 2;
    obj._residentInd = 'I';
    obj._resGeoTblItemNo = 3;
    obj._travelInd = 'J';
    obj._miles = 4;
    obj._ticket = 'K';
    obj._timePeriod = 5;
    obj._timeUnit = 'L';
    obj._issueInd = 'M';
    obj._issueGeoTblItemNo = 6;
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailTag;
  PaxTypeCode _psgType;
  Indicator _idRequiredInd;
  int _minAge;
  int _maxAge;
  Indicator _residentInd;
  int _resGeoTblItemNo;
  Indicator _travelInd;
  int _miles;
  Indicator _ticket;
  int _timePeriod;
  Indicator _timeUnit;
  Indicator _issueInd;
  int _issueGeoTblItemNo;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _idRequiredInd);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _residentInd);
    FLATTENIZE(archive, _resGeoTblItemNo);
    FLATTENIZE(archive, _travelInd);
    FLATTENIZE(archive, _miles);
    FLATTENIZE(archive, _ticket);
    FLATTENIZE(archive, _timePeriod);
    FLATTENIZE(archive, _timeUnit);
    FLATTENIZE(archive, _issueInd);
    FLATTENIZE(archive, _issueGeoTblItemNo);
  }
};
}

