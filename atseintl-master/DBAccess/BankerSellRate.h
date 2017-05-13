//----------------------------------------------------------------------------
//
//        File:           BankerSellRate.h
//        Description:    BSR processing data
//        Created:        2/4/2004
//              Authors:        Roger Kelly
//
//        Updates:
//
//       @ 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class BankerSellRate
{
public:
  BankerSellRate() : _rate(0), _rateNodec(0), _rateType(' ') {}

  CurrencyCode& primeCur() { return _primeCur; }
  const CurrencyCode& primeCur() const { return _primeCur; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  ExchRate& rate() { return _rate; }
  const ExchRate& rate() const { return _rate; }

  CurrencyNoDec& rateNodec() { return _rateNodec; }
  const CurrencyNoDec& rateNodec() const { return _rateNodec; }

  Indicator& rateType() { return _rateType; }
  const Indicator& rateType() const { return _rateType; }

  std::string& agentSine() { return _agentSine; }
  const std::string& agentSine() const { return _agentSine; }

  bool operator==(const BankerSellRate& rhs) const
  {
    return ((_primeCur == rhs._primeCur) && (_cur == rhs._cur) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_expireDate == rhs._expireDate) &&
            (_rate == rhs._rate) && (_rateNodec == rhs._rateNodec) &&
            (_rateType == rhs._rateType) && (_agentSine == rhs._agentSine));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(BankerSellRate& obj)
  {
    obj._primeCur = "ABC";
    obj._cur = "DEF";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._rate = 1.111;
    obj._rateNodec = 2;
    obj._rateType = 'G';
    obj._agentSine = "aaaaaaaa";
  }

private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_primeCur
           & ptr->_cur
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_expireDate
           & ptr->_rate
           & ptr->_rateNodec
           & ptr->_rateType
           & ptr->_agentSine;
  }

  CurrencyCode _primeCur;
  CurrencyCode _cur;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  ExchRate _rate;
  CurrencyNoDec _rateNodec;
  Indicator _rateType;
  std::string _agentSine;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _primeCur);
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _rate);
    FLATTENIZE(archive, _rateNodec);
    FLATTENIZE(archive, _rateType);
    FLATTENIZE(archive, _agentSine);
  }

};
}
