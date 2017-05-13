//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/CategoryRuleInfo.h"

namespace tse
{
class FootNoteCtrlInfo final : public CategoryRuleInfo
{

public:
  FootNoteCtrlInfo() : _expireDate(_createDate), _effDate(_createDate), _discDate(_createDate) {}

  const Indicator& inhibit() const { return _inhibit; }
  const TariffNumber& fareTariff() const { return _fareTariff; }
  const Footnote& footNote() const { return _footNote; }
  const int& category() const { return _category; }
  const SequenceNumberLong& seqNo() const { return _seqNo; }
  const DateTime& expireDate() const { return _expireDate; }
  const DateTime& effDate() const { return _effDate; }
  const DateTime& discDate() const { return _discDate; }
  const int& segcount() const { return _segcount; }
  const SequenceNumberLong& newseqNo() const { return _newseqNo; }
  const int& jointCarrierTblItemNo() const { return _jointCarrierTblItemNo; }
  const FareClassCode& fareClass() const { return _fareClass; }
  const LocKey& loc1() const { return _loc1; }
  const LocKey& loc2() const { return _loc2; }
  const Indicator& owrt() const { return _owrt; }
  const Indicator& routingAppl() const { return _routingAppl; }
  const RoutingNumber& routing() const { return _routing; }

  Indicator& inhibit() { return _inhibit; }
  TariffNumber& fareTariff() { return _fareTariff; }
  Footnote& footNote() { return _footNote; }
  SequenceNumberLong& seqNo() { return _seqNo; }
  DateTime& expireDate() { return _expireDate; }
  DateTime& effDate() { return _effDate; }
  DateTime& discDate() { return _discDate; }
  int& category() { return _category; }
  int& segcount() { return _segcount; }
  SequenceNumberLong& newseqNo() { return _newseqNo; }
  int& jointCarrierTblItemNo() { return _jointCarrierTblItemNo; }
  FareClassCode& fareClass() { return _fareClass; }
  LocKey& loc1() { return _loc1; }
  LocKey& loc2() { return _loc2; }
  Indicator& owrt() { return _owrt; }
  Indicator& routingAppl() { return _routingAppl; }
  RoutingNumber& routing() { return _routing; }

  virtual bool operator==(const FootNoteCtrlInfo& rhs) const
  {
    bool eq =
        ((CategoryRuleInfo::operator==(rhs)) && (_fareTariff == rhs._fareTariff) &&
         (_footNote == rhs._footNote) && (_category == rhs._category) && (_seqNo == rhs._seqNo) &&
         (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
         (_discDate == rhs._discDate) && (_segcount == rhs._segcount) &&
         (_newseqNo == rhs._newseqNo) && (_jointCarrierTblItemNo == rhs._jointCarrierTblItemNo) &&
         (_fareClass == rhs._fareClass) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
         (_owrt == rhs._owrt) && (_routingAppl == rhs._routingAppl) && (_routing == rhs._routing) &&
         (_inhibit == rhs._inhibit));

    return eq;
  }

  static void dummyData(FootNoteCtrlInfo& obj)
  {
    CategoryRuleInfo::dummyData(obj);

    obj.inhibit() = 'A';
    obj.fareTariff() = 1;
    obj.footNote() = "BC";
    obj.category() = 2;
    obj.seqNo() = 30000000003;
    obj.expireDate() = time(nullptr);
    obj.effDate() = time(nullptr);
    obj.discDate() = time(nullptr);
    obj.segcount() = 4;
    obj.newseqNo() = 50000000005;
    obj.jointCarrierTblItemNo() = 6;
    obj.fareClass() = "DEFGHIJK";
    obj.loc1().loc() = "LMNOPQRS";
    obj.loc1().locType() = 'T';
    obj.loc2().loc() = "UVWXYZab";
    obj.loc2().locType() = 'c';
    obj.owrt() = 'd';
    obj.routingAppl() = 'e';
    obj.routing() = "FGHI";
    obj.applInd() = 'j';
  }

  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, CategoryRuleInfo);

    FLATTENIZE(archive, _fareTariff);
    FLATTENIZE(archive, _footNote);
    FLATTENIZE(archive, _category);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segcount);
    FLATTENIZE(archive, _newseqNo);
    FLATTENIZE(archive, _jointCarrierTblItemNo);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _routingAppl);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _inhibit);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  TariffNumber _fareTariff = 0;
  Footnote _footNote;
  SequenceNumberLong _seqNo = 0;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  int _category = 0;
  int _segcount = 0;
  SequenceNumberLong _newseqNo = 0;
  FareClassCode _fareClass;
  LocKey _loc1;
  LocKey _loc2;
  int _jointCarrierTblItemNo = 0;
  Indicator _owrt = ' ';
  Indicator _routingAppl = ' ';
  Indicator _inhibit = ' '; // Inhibit now checked at App Level
  RoutingNumber _routing;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    CategoryRuleInfo::convert(buffer, ptr);
    return buffer & ptr->_fareTariff & ptr->_footNote & ptr->_category & ptr->_seqNo &
           ptr->_expireDate & ptr->_effDate & ptr->_discDate & ptr->_segcount & ptr->_newseqNo &
           ptr->_jointCarrierTblItemNo & ptr->_fareClass & ptr->_loc1 & ptr->_loc2 & ptr->_owrt &
           ptr->_routingAppl & ptr->_routing & ptr->_inhibit;
  }
};
}
