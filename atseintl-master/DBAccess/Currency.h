//----------------------------------------------------------------------------
//
// File:           Currency.h
// Description:    Currency
//
// © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use,
// reproduction, or transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system or publication,
// is strictly prohibited.
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class Currency
{
public:
  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CurrencyFactor& domRoundingFactor() { return _domRoundingFactor; }
  const CurrencyFactor& domRoundingFactor() const { return _domRoundingFactor; }

  CurrencyNoDec& roundingFactorNoDec() { return _roundingFactorNoDec; }
  const CurrencyNoDec& roundingFactorNoDec() const { return _roundingFactorNoDec; }

  CurrencyNoDec& noDec() { return _noDec; }
  const CurrencyNoDec& noDec() const { return _noDec; }

  int& curNo() { return _curNo; }
  const int& curNo() const { return _curNo; }

  std::string& curName() { return _curName; }
  const std::string& curName() const { return _curName; }

  std::string& nucRoundingProcessNo() { return _nucRoundingProcessNo; }
  const std::string& nucRoundingProcessNo() const { return _nucRoundingProcessNo; }

  std::string& controllingEntityDesc() { return _controllingEntityDesc; }
  const std::string& controllingEntityDesc() const { return _controllingEntityDesc; }

  CurrencyNoDec& taxOverrideRoundingUnitNoDec() { return _taxOverrideRoundingUnitNoDec; }
  const CurrencyNoDec& taxOverrideRoundingUnitNoDec() const
  {
    return _taxOverrideRoundingUnitNoDec;
  }

  double& taxOverrideRoundingUnit() { return _taxOverrideRoundingUnit; }
  const double& taxOverrideRoundingUnit() const { return _taxOverrideRoundingUnit; }

  RoundingRule& taxOverrideRoundingRule() { return _taxOverrideRoundingRule; }
  const RoundingRule& taxOverrideRoundingRule() const { return _taxOverrideRoundingRule; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::vector<NationCode>& nationRes() { return _nationRes; }
  const std::vector<NationCode>& nationRes() const { return _nationRes; }

  bool operator==(const Currency& rhs) const
  {
    return ((_cur == rhs._cur) && (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_domRoundingFactor == rhs._domRoundingFactor) &&
            (_roundingFactorNoDec == rhs._roundingFactorNoDec) && (_noDec == rhs._noDec) &&
            (_curNo == rhs._curNo) && (_curName == rhs._curName) &&
            (_nucRoundingProcessNo == rhs._nucRoundingProcessNo) &&
            (_controllingEntityDesc == rhs._controllingEntityDesc) &&
            (_taxOverrideRoundingUnitNoDec == rhs._taxOverrideRoundingUnitNoDec) &&
            (_taxOverrideRoundingUnit == rhs._taxOverrideRoundingUnit) &&
            (_taxOverrideRoundingRule == rhs._taxOverrideRoundingRule) &&
            (_nationRes == rhs._nationRes) && (_createDate == rhs._createDate));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(Currency& obj)
  {
    obj._cur = "ABC";
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._domRoundingFactor = 1.111;
    obj._roundingFactorNoDec = 2;
    obj._noDec = 3;
    obj._curNo = 4;
    obj._curName = "aaaaaaaa";
    obj._nucRoundingProcessNo = "bbbbbbbb";
    obj._controllingEntityDesc = "cccccccc";
    obj._taxOverrideRoundingUnitNoDec = 5;
    obj._taxOverrideRoundingUnit = 6.666;
    obj._taxOverrideRoundingRule = UP;
    obj._nationRes.push_back("DEF");
    obj._nationRes.push_back("GHI");
    obj._createDate = time(nullptr);
  }

private:
  CurrencyCode _cur;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  CurrencyFactor _domRoundingFactor = 0;
  CurrencyNoDec _roundingFactorNoDec = 0;
  CurrencyNoDec _noDec = 0;
  int _curNo = 0;
  std::string _curName;
  std::string _nucRoundingProcessNo;
  std::string _controllingEntityDesc;
  CurrencyNoDec _taxOverrideRoundingUnitNoDec = 0;
  double _taxOverrideRoundingUnit = 0;
  RoundingRule _taxOverrideRoundingRule = RoundingRule::NONE;
  std::vector<NationCode> _nationRes;
  DateTime _createDate;

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_cur
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_domRoundingFactor
           & ptr->_roundingFactorNoDec
           & ptr->_noDec
           & ptr->_curNo
           & ptr->_curName
           & ptr->_nucRoundingProcessNo
           & ptr->_controllingEntityDesc
           & ptr->_taxOverrideRoundingUnitNoDec
           & ptr->_taxOverrideRoundingUnit
           & ptr->_taxOverrideRoundingRule
           & ptr->_nationRes
           & ptr->_createDate;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _domRoundingFactor);
    FLATTENIZE(archive, _roundingFactorNoDec);
    FLATTENIZE(archive, _noDec);
    FLATTENIZE(archive, _curNo);
    FLATTENIZE(archive, _curName);
    FLATTENIZE(archive, _nucRoundingProcessNo);
    FLATTENIZE(archive, _controllingEntityDesc);
    FLATTENIZE(archive, _taxOverrideRoundingUnitNoDec);
    FLATTENIZE(archive, _taxOverrideRoundingUnit);
    FLATTENIZE(archive, _taxOverrideRoundingRule);
    FLATTENIZE(archive, _nationRes);
    FLATTENIZE(archive, _createDate);
  }
};
}
