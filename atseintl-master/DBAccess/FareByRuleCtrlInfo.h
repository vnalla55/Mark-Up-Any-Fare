//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CategoryRuleInfo.h"

namespace tse
{
class FareByRuleCtrlInfo final : public CategoryRuleInfo
{
public:
  FareByRuleCtrlInfo() : _expireDate(_createDate), _effDate(_createDate), _discDate(_createDate) {}

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& jointCarrierTblItemNo() { return _jointCarrierTblItemNo; }
  const int& jointCarrierTblItemNo() const { return _jointCarrierTblItemNo; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  TariffNumber& generalRuleTariff() { return _generalRuleTariff; }
  const TariffNumber& generalRuleTariff() const { return _generalRuleTariff; }

  LocCode& loc1zoneTblItemNo() { return _loc1zoneTblItemNo; }
  const LocCode& loc1zoneTblItemNo() const { return _loc1zoneTblItemNo; }

  LocCode& loc2zoneTblItemNo() { return _loc2zoneTblItemNo; }
  const LocCode& loc2zoneTblItemNo() const { return _loc2zoneTblItemNo; }

  Indicator& generalRuleAppl() { return _generalRuleAppl; }
  const Indicator& generalRuleAppl() const { return _generalRuleAppl; }

  RuleNumber& generalRule() { return _generalRule; }
  const RuleNumber& generalRule() const { return _generalRule; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const FareByRuleCtrlInfo& rhs) const
  {
    return ((CategoryRuleInfo::operator==(rhs)) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_jointCarrierTblItemNo == rhs._jointCarrierTblItemNo) && (_segCnt == rhs._segCnt) &&
            (_generalRuleTariff == rhs._generalRuleTariff) &&
            (_loc1zoneTblItemNo == rhs._loc1zoneTblItemNo) &&
            (_loc2zoneTblItemNo == rhs._loc2zoneTblItemNo) &&
            (_generalRuleAppl == rhs._generalRuleAppl) && (_generalRule == rhs._generalRule) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(FareByRuleCtrlInfo& obj)
  {
    CategoryRuleInfo::dummyData(obj);

    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._jointCarrierTblItemNo = 1;
    obj._segCnt = 2;
    obj._generalRuleTariff = 3;
    obj._loc1zoneTblItemNo = "ABCDEFGH";
    obj._loc2zoneTblItemNo = "IJKLMNOP";
    obj._generalRuleAppl = 'R';
    obj._generalRule = "STUV";
    obj._inhibit = 'W';
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  int _jointCarrierTblItemNo = 0;
  int _segCnt = 0;
  TariffNumber _generalRuleTariff = 0;
  LocCode _loc1zoneTblItemNo;
  LocCode _loc2zoneTblItemNo;
  Indicator _generalRuleAppl = ' ';
  RuleNumber _generalRule;
  Indicator _inhibit = ' ';

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, CategoryRuleInfo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _jointCarrierTblItemNo);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _generalRuleTariff);
    FLATTENIZE(archive, _loc1zoneTblItemNo);
    FLATTENIZE(archive, _loc2zoneTblItemNo);
    FLATTENIZE(archive, _generalRuleAppl);
    FLATTENIZE(archive, _generalRule);
    FLATTENIZE(archive, _inhibit);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    CategoryRuleInfo::convert(buffer, ptr);
    return buffer & ptr->_expireDate & ptr->_effDate & ptr->_discDate &
           ptr->_jointCarrierTblItemNo & ptr->_segCnt & ptr->_generalRuleTariff &
           ptr->_loc1zoneTblItemNo & ptr->_loc2zoneTblItemNo & ptr->_generalRuleAppl &
           ptr->_generalRule & ptr->_inhibit;
  }
};
}
