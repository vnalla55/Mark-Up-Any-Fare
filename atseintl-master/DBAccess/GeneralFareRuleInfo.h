//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/CategoryRuleInfo.h"

namespace tse
{
class GeneralFareRuleInfo final : public CategoryRuleInfo
{
public:
  GeneralFareRuleInfo() : _expireDate(_createDate), _effDate(_createDate), _discDate(_createDate) {}

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& segcount() { return _segcount; }
  const int& segcount() const { return _segcount; }

  int& jointCarrierTblItemNo() { return _jointCarrierTblItemNo; }
  const int& jointCarrierTblItemNo() const { return _jointCarrierTblItemNo; }

  TariffNumber& generalRuleTariff() { return _generalRuleTariff; }
  const TariffNumber& generalRuleTariff() const { return _generalRuleTariff; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& routingAppl() { return _routingAppl; }
  const Indicator& routingAppl() const { return _routingAppl; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  Footnote& footNote1() { return _footNote1; }
  const Footnote& footNote1() const { return _footNote1; }

  Footnote& footNote2() { return _footNote2; }
  const Footnote& footNote2() const { return _footNote2; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  Indicator& seasonType() { return _seasonType; }
  const Indicator& seasonType() const { return _seasonType; }

  Indicator& dowType() { return _dowType; }
  const Indicator& dowType() const { return _dowType; }

  RuleNumber& generalRule() { return _generalRule; }
  const RuleNumber& generalRule() const { return _generalRule; }

  Indicator& generalRuleAppl() { return _generalRuleAppl; }
  const Indicator& generalRuleAppl() const { return _generalRuleAppl; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const GeneralFareRuleInfo& rhs) const
  {
    return (
        (CategoryRuleInfo::operator==(rhs)) && (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
        (_segcount == rhs._segcount) && (_jointCarrierTblItemNo == rhs._jointCarrierTblItemNo) &&
        (_generalRuleTariff == rhs._generalRuleTariff) && (_fareClass == rhs._fareClass) &&
        (_owrt == rhs._owrt) && (_routingAppl == rhs._routingAppl) && (_routing == rhs._routing) &&
        (_footNote1 == rhs._footNote1) && (_footNote2 == rhs._footNote2) &&
        (_fareType == rhs._fareType) && (_seasonType == rhs._seasonType) &&
        (_dowType == rhs._dowType) && (_generalRule == rhs._generalRule) &&
        (_generalRuleAppl == rhs._generalRuleAppl) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(GeneralFareRuleInfo& obj)
  {
    CategoryRuleInfo::dummyData(obj);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._segcount = 1;
    obj._jointCarrierTblItemNo = 2;
    obj._generalRuleTariff = 3;
    obj._fareClass = "aaaaaaaa";
    obj._owrt = 'A';
    obj._routingAppl = 'B';
    obj._routing = "CDEF";
    obj._footNote1 = "GH";
    obj._footNote2 = "IJ";
    obj._fareType = "bbbbbbbb";
    obj._seasonType = 'K';
    obj._dowType = 'L';
    obj._generalRule = "MNOP";
    obj._generalRuleAppl = 'Q';
    obj._inhibit = 'R';
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  int _segcount = 0;
  int _jointCarrierTblItemNo = 0;
  TariffNumber _generalRuleTariff = 0;
  Indicator _owrt = ' ';
  Indicator _routingAppl = ' ';
  FareClassCode _fareClass;
  RoutingNumber _routing;
  Footnote _footNote1;
  Footnote _footNote2;
  FareType _fareType;
  RuleNumber _generalRule;
  Indicator _seasonType = ' ';
  Indicator _dowType = ' ';
  Indicator _generalRuleAppl = ' ';
  Indicator _inhibit = ' ';

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, CategoryRuleInfo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segcount);
    FLATTENIZE(archive, _jointCarrierTblItemNo);
    FLATTENIZE(archive, _generalRuleTariff);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _routingAppl);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _footNote1);
    FLATTENIZE(archive, _footNote2);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _seasonType);
    FLATTENIZE(archive, _dowType);
    FLATTENIZE(archive, _generalRule);
    FLATTENIZE(archive, _generalRuleAppl);
    FLATTENIZE(archive, _inhibit);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    CategoryRuleInfo::convert(buffer, ptr);
    return buffer & ptr->_expireDate & ptr->_effDate & ptr->_discDate & ptr->_segcount &
           ptr->_jointCarrierTblItemNo & ptr->_generalRuleTariff & ptr->_fareClass & ptr->_owrt &
           ptr->_routingAppl & ptr->_routing & ptr->_footNote1 & ptr->_footNote2 & ptr->_fareType &
           ptr->_seasonType & ptr->_dowType & ptr->_generalRule & ptr->_generalRuleAppl &
           ptr->_inhibit;
  }
};
}
