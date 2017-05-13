//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class GeneralRuleApp final : public CategoryRuleInfo
{
public:
  GeneralRuleApp() : _expireDate(_createDate) {}

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  RuleNumber& generalRule() { return _generalRule; }
  const RuleNumber& generalRule() const { return _generalRule; }

  TariffNumber& generalRuleTariff() { return _generalRuleTariff; }
  const TariffNumber& generalRuleTariff() const { return _generalRuleTariff; }

  CatNumber& category() { return _category; }
  const CatNumber& category() const { return _category; }

  bool operator==(const GeneralRuleApp& rhs) const
  {
    return ((CategoryRuleInfo::operator==(rhs)) && (_expireDate == rhs._expireDate) &&
            (_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_ruleTariff == rhs._ruleTariff) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) &&  (_rule == rhs._rule) &&
            (_orderNo == rhs._orderNo) && (_generalRule == rhs._generalRule) &&
            (_generalRuleTariff == rhs._generalRuleTariff) && (_category == rhs._category));
  }

  static void dummyData(GeneralRuleApp& obj)
  {
    CategoryRuleInfo::dummyData(obj);

    obj._expireDate = time(nullptr);
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._ruleTariff = 1;
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._rule = "HIJK";
    obj._orderNo = 2;
    obj._generalRule = "LMNO";
    obj._generalRuleTariff = 3;
    obj._category = 4;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _expireDate;
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  DateTime _effDate; // first date on which record is effective.
  DateTime _discDate; // last date on which record is effective.
  RuleNumber _rule;
  int _orderNo = 0;
  RuleNumber _generalRule;
  TariffNumber _generalRuleTariff = 0;
  CatNumber _category = 0;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, CategoryRuleInfo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _generalRule);
    FLATTENIZE(archive, _generalRuleTariff);
    FLATTENIZE(archive, _category);

  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    CategoryRuleInfo::convert(buffer, ptr);
    return buffer & ptr->_expireDate & ptr->_vendor & ptr->_carrier & ptr->_ruleTariff & ptr->_effDate & ptr->_discDate &
           ptr->_rule & ptr->_orderNo & ptr->_generalRule & ptr->_generalRuleTariff & ptr->_category;
  }
};
}

