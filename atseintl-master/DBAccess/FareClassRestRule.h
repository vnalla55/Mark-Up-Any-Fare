//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class FareClassRestRule : public RuleItemInfo
{
public:
  FareClassRestRule()
    : _orderNo(0),
      _fareClassTypeApplInd(' '),
      _normalFaresInd(' '),
      _owrt(' '),
      _sametrfRuleInd(' '),
      _samediffInd(' '),
      _sameminMaxInd(' '),
      _typeInd(' '),
      _penaltysvcchrgApplInd(' '),
      _penaltyRestInd(' ')
  {
  }

  virtual ~FareClassRestRule() {}

  virtual bool operator==(const FareClassRestRule& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_orderNo == rhs._orderNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_fareClassTypeApplInd == rhs._fareClassTypeApplInd) &&
            (_normalFaresInd == rhs._normalFaresInd) && (_owrt == rhs._owrt) &&
            (_sametrfRuleInd == rhs._sametrfRuleInd) && (_samediffInd == rhs._samediffInd) &&
            (_sameminMaxInd == rhs._sameminMaxInd) && (_typeInd == rhs._typeInd) &&
            (_typeCode == rhs._typeCode) &&
            (_penaltysvcchrgApplInd == rhs._penaltysvcchrgApplInd) &&
            (_penaltyRestInd == rhs._penaltyRestInd) && (_appendageCode == rhs._appendageCode));
  }

  static void dummyData(FareClassRestRule& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._orderNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._fareClassTypeApplInd = 'A';
    obj._normalFaresInd = 'B';
    obj._owrt = 'C';
    obj._sametrfRuleInd = 'D';
    obj._samediffInd = 'E';
    obj._sameminMaxInd = 'F';
    obj._typeInd = 'G';
    obj._typeCode = "HIJKLMNO";
    obj._penaltysvcchrgApplInd = 'P';
    obj._penaltyRestInd = 'Q';
    obj._appendageCode = "RSTUV";
  }

private:
  int _orderNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _fareClassTypeApplInd;
  Indicator _normalFaresInd;
  Indicator _owrt;
  Indicator _sametrfRuleInd;
  Indicator _samediffInd;
  Indicator _sameminMaxInd;
  Indicator _typeInd;
  FareType _typeCode;
  Indicator _penaltysvcchrgApplInd;
  Indicator _penaltyRestInd;
  AppendageCode _appendageCode;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fareClassTypeApplInd);
    FLATTENIZE(archive, _normalFaresInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _sametrfRuleInd);
    FLATTENIZE(archive, _samediffInd);
    FLATTENIZE(archive, _sameminMaxInd);
    FLATTENIZE(archive, _typeInd);
    FLATTENIZE(archive, _typeCode);
    FLATTENIZE(archive, _penaltysvcchrgApplInd);
    FLATTENIZE(archive, _penaltyRestInd);
    FLATTENIZE(archive, _appendageCode);
  }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& fareClassTypeApplInd() { return _fareClassTypeApplInd; }
  const Indicator& fareClassTypeApplInd() const { return _fareClassTypeApplInd; }

  Indicator& normalFaresInd() { return _normalFaresInd; }
  const Indicator& normalFaresInd() const { return _normalFaresInd; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& sametrfRuleInd() { return _sametrfRuleInd; }
  const Indicator& sametrfRuleInd() const { return _sametrfRuleInd; }

  Indicator& samediffInd() { return _samediffInd; }
  const Indicator& samediffInd() const { return _samediffInd; }

  Indicator& sameminMaxInd() { return _sameminMaxInd; }
  const Indicator& sameminMaxInd() const { return _sameminMaxInd; }

  Indicator& typeInd() { return _typeInd; }
  const Indicator& typeInd() const { return _typeInd; }

  FareType& typeCode() { return _typeCode; }
  const FareType& typeCode() const { return _typeCode; }

  Indicator& penaltysvcchrgApplInd() { return _penaltysvcchrgApplInd; }
  const Indicator& penaltysvcchrgApplInd() const { return _penaltysvcchrgApplInd; }

  Indicator& penaltyRestInd() { return _penaltyRestInd; }
  const Indicator& penaltyRestInd() const { return _penaltyRestInd; }

  AppendageCode& appendageCode() { return _appendageCode; }
  const AppendageCode& appendageCode() const { return _appendageCode; }
};
}

