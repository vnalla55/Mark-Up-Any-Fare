//----------------------------------------------------------------------------
//
//    File:           OpenJawRule.h
//    Description:    OpenJawRule processing data
//    Created:        3/27/2004
//      Authors:        Roger Kelly
//
//
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class OpenJawRule : public RuleItemInfo
{
public:
  OpenJawRule()
    : _validityInd(' '),
      _inhibit(' '),
      _unavailtag(' '),
      _ojApplInd(' '),
      _sameCarrierInd(' '),
      _halftransportInd(' '),
      _owrt(' '),
      _sojvalidityInd(' '),
      _highrtInd(' '),
      _ojbackhaulInd(' '),
      _farecompInd(' ')
  {
  }

  virtual ~OpenJawRule() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  Indicator& ojApplInd() { return _ojApplInd; }
  const Indicator& ojApplInd() const { return _ojApplInd; }

  Indicator& sameCarrierInd() { return _sameCarrierInd; }
  const Indicator& sameCarrierInd() const { return _sameCarrierInd; }

  Indicator& halftransportInd() { return _halftransportInd; }
  const Indicator& halftransportInd() const { return _halftransportInd; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& sojvalidityInd() { return _sojvalidityInd; }
  const Indicator& sojvalidityInd() const { return _sojvalidityInd; }

  Indicator& highrtInd() { return _highrtInd; }
  const Indicator& highrtInd() const { return _highrtInd; }

  Indicator& ojbackhaulInd() { return _ojbackhaulInd; }
  const Indicator& ojbackhaulInd() const { return _ojbackhaulInd; }

  std::string& stopoverCnt() { return _stopoverCnt; }
  const std::string& stopoverCnt() const { return _stopoverCnt; }

  Indicator& farecompInd() { return _farecompInd; }
  const Indicator& farecompInd() const { return _farecompInd; }

  virtual bool operator==(const OpenJawRule& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_validityInd == rhs._validityInd) &&
            (_inhibit == rhs._inhibit) && (_unavailtag == rhs._unavailtag) &&
            (_ojApplInd == rhs._ojApplInd) && (_sameCarrierInd == rhs._sameCarrierInd) &&
            (_halftransportInd == rhs._halftransportInd) && (_owrt == rhs._owrt) &&
            (_sojvalidityInd == rhs._sojvalidityInd) && (_highrtInd == rhs._highrtInd) &&
            (_ojbackhaulInd == rhs._ojbackhaulInd) && (_stopoverCnt == rhs._stopoverCnt) &&
            (_farecompInd == rhs._farecompInd));
  }

  static void dummyData(OpenJawRule& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'A';
    obj._inhibit = 'B';
    obj._unavailtag = 'C';
    obj._ojApplInd = 'D';
    obj._sameCarrierInd = 'E';
    obj._halftransportInd = 'F';
    obj._owrt = 'G';
    obj._sojvalidityInd = 'H';
    obj._highrtInd = 'I';
    obj._ojbackhaulInd = 'J';
    obj._stopoverCnt = "aaaaaaaa";
    obj._farecompInd = 'K';
  }

private:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailtag;
  Indicator _ojApplInd;
  Indicator _sameCarrierInd;
  Indicator _halftransportInd;
  Indicator _owrt;
  Indicator _sojvalidityInd;
  Indicator _highrtInd;
  Indicator _ojbackhaulInd;
  std::string _stopoverCnt;
  Indicator _farecompInd;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _ojApplInd);
    FLATTENIZE(archive, _sameCarrierInd);
    FLATTENIZE(archive, _halftransportInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _sojvalidityInd);
    FLATTENIZE(archive, _highrtInd);
    FLATTENIZE(archive, _ojbackhaulInd);
    FLATTENIZE(archive, _stopoverCnt);
    FLATTENIZE(archive, _farecompInd);
  }

};
}

