//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{
class TariffRuleRest : public RuleItemInfo
{
public:
  virtual bool operator==(const TariffRuleRest& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_orderNo == rhs._orderNo) &&
            (_ruleTariff == rhs._ruleTariff) && (_primeRuleInd == rhs._primeRuleInd) &&
            (_defaultRuleInd == rhs._defaultRuleInd) && (_trfRuleApplInd == rhs._trfRuleApplInd) &&
            (_sametrfRuleInd == rhs._sametrfRuleInd) && (_rule == rhs._rule) &&
            (_globalDir == rhs._globalDir));
  }

  static void dummyData(TariffRuleRest& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._orderNo = 1;
    obj._ruleTariff = 2;
    obj._primeRuleInd = 'A';
    obj._defaultRuleInd = 'B';
    obj._trfRuleApplInd = 'C';
    obj._sametrfRuleInd = 'D';
    obj._rule = "EFGH";
    obj._globalDir = GlobalDirection::US;
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  int _orderNo = 0;
  TariffNumber _ruleTariff = 0;
  Indicator _primeRuleInd = ' ';
  Indicator _defaultRuleInd = ' ';
  Indicator _trfRuleApplInd = ' ';
  Indicator _sametrfRuleInd = ' ';
  RuleNumber _rule;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _primeRuleInd);
    FLATTENIZE(archive, _defaultRuleInd);
    FLATTENIZE(archive, _trfRuleApplInd);
    FLATTENIZE(archive, _sametrfRuleInd);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _globalDir);
  }

protected:
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  Indicator& primeRuleInd() { return _primeRuleInd; }
  const Indicator& primeRuleInd() const { return _primeRuleInd; }

  Indicator& defaultRuleInd() { return _defaultRuleInd; }
  const Indicator& defaultRuleInd() const { return _defaultRuleInd; }

  Indicator& trfRuleApplInd() { return _trfRuleApplInd; }
  const Indicator& trfRuleApplInd() const { return _trfRuleApplInd; }

  Indicator& sametrfRuleInd() { return _sametrfRuleInd; }
  const Indicator& sametrfRuleInd() const { return _sametrfRuleInd; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  GlobalDirection& globalDirection() { return _globalDir; }
  const GlobalDirection& globalDirection() const { return _globalDir; }
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_orderNo
           & ptr->_ruleTariff
           & ptr->_primeRuleInd
           & ptr->_defaultRuleInd
           & ptr->_trfRuleApplInd
           & ptr->_sametrfRuleInd
           & ptr->_rule
           & ptr->_globalDir;
  }
};
}
