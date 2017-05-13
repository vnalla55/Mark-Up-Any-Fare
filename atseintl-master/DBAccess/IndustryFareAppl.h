//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class IndustryFareAppl
{
public:
  class ExceptAppl
  {
  public:
    // so called accessors: misbegotten mutant stepchildren of getters/setters

    CarrierCode& exceptCarrier() { return _exceptCarrier; }
    const CarrierCode& exceptCarrier() const { return _exceptCarrier; }

    int& itemNo() { return _itemNo; }
    const int& itemNo() const { return _itemNo; }

    TariffNumber& fareTariff() { return _fareTariff; }
    const TariffNumber& fareTariff() const { return _fareTariff; }

    VendorCode& vendor() { return _vendor; }
    const VendorCode& vendor() const { return _vendor; }

    UserApplCode& userAppl() { return _userAppl; }
    const UserApplCode& userAppl() const { return _userAppl; }

    Indicator& yyFareAppl() { return _yyFareAppl; }
    const Indicator& yyFareAppl() const { return _yyFareAppl; }

    Indicator& directionality() { return _directionality; }
    const Indicator& directionality() const { return _directionality; }

    LocKey& loc1() { return _loc1; }
    const LocKey& loc1() const { return _loc1; }

    LocKey& loc2() { return _loc2; }
    const LocKey& loc2() const { return _loc2; }

    GlobalDirection& globalDir() { return _globalDir; }
    const GlobalDirection& globalDir() const { return _globalDir; }

    RuleNumber& rule() { return _rule; }
    const RuleNumber& rule() const { return _rule; }

    RoutingNumber& routing() { return _routing; }
    const RoutingNumber& routing() const { return _routing; }

    Footnote& footNote() { return _footNote; }
    const Footnote& footNote() const { return _footNote; }

    CurrencyCode& cur() { return _cur; }
    const CurrencyCode& cur() const { return _cur; }

    FareClassCode& fareClass() { return _fareClass; }
    const FareClassCode& fareClass() const { return _fareClass; }

    FareType& fareType() { return _fareType; }
    const FareType& fareType() const { return _fareType; }

    Indicator& owrt() { return _owrt; }
    const Indicator& owrt() const { return _owrt; }

    Indicator& userApplType() { return _userApplType; }
    const Indicator& userApplType() const { return _userApplType; }

    bool operator<(const ExceptAppl& rhs) const;

    bool operator==(const ExceptAppl& rhs) const;

    static void dummyData(ExceptAppl& obj);

    void flattenize(Flattenizable::Archive& archive);

    WBuffer& write(WBuffer& os) const
    {
      return convert(os, this);
    }

    RBuffer& read(RBuffer& is)
    {
      return convert(is, this);
    }

  protected:
    CarrierCode _exceptCarrier;
    int _itemNo = 0;
    TariffNumber _fareTariff = 0;
    VendorCode _vendor;
    UserApplCode _userAppl;
    Indicator _yyFareAppl = ' ';
    Indicator _directionality = ' ';
    LocKey _loc1;
    LocKey _loc2;
    GlobalDirection _globalDir = GlobalDirection::NO_DIR;
    RuleNumber _rule;
    RoutingNumber _routing;
    Footnote _footNote;
    CurrencyCode _cur;
    FareClassCode _fareClass;
    FareType _fareType;
    Indicator _owrt = ' ';
    Indicator _userApplType = ' ';

private:

    template <typename B, typename T>
    static B& convert(B& buffer,
                      T ptr)
    {
      return buffer
             & ptr->_exceptCarrier
             & ptr->_itemNo
             & ptr->_fareTariff
             & ptr->_vendor
             & ptr->_userAppl
             & ptr->_yyFareAppl
             & ptr->_directionality
             & ptr->_loc1
             & ptr->_loc2
             & ptr->_globalDir
             & ptr->_rule
             & ptr->_routing
             & ptr->_footNote
             & ptr->_cur
             & ptr->_fareClass
             & ptr->_fareType
             & ptr->_owrt
             & ptr->_userApplType;
    }
  };

  Indicator& selectionType() { return _selectionType; }
  const Indicator& selectionType() const { return _selectionType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& yyFareAppl() { return _yyFareAppl; }
  const Indicator& yyFareAppl() const { return _yyFareAppl; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  Footnote& footNote() { return _footNote; }
  const Footnote& footNote() const { return _footNote; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  std::vector<ExceptAppl>& except() { return _except; }
  const std::vector<ExceptAppl>& except() const { return _except; }

  bool operator<(const IndustryFareAppl& rhs) const;

  bool operator==(const IndustryFareAppl& rhs) const;

  static void dummyData(IndustryFareAppl& obj);

  void flattenize(Flattenizable::Archive& archive);

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  Indicator _selectionType = ' ';
  CarrierCode _carrier;
  int _seqNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  TariffNumber _fareTariff = 0;
  VendorCode _vendor;
  UserApplCode _userAppl;
  Indicator _yyFareAppl = ' ';
  Indicator _directionality = ' ';
  LocKey _loc1;
  LocKey _loc2;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  RuleNumber _rule;
  RoutingNumber _routing;
  Footnote _footNote;
  CurrencyCode _cur;
  FareClassCode _fareClass;
  FareType _fareType;
  Indicator _owrt = ' ';
  Indicator _userApplType = ' ';
  std::vector<ExceptAppl> _except;

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_selectionType
           & ptr->_carrier
           & ptr->_seqNo
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_fareTariff
           & ptr->_vendor
           & ptr->_userAppl
           & ptr->_yyFareAppl
           & ptr->_directionality
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_globalDir
           & ptr->_rule
           & ptr->_routing
           & ptr->_footNote
           & ptr->_cur
           & ptr->_fareClass
           & ptr->_fareType
           & ptr->_owrt
           & ptr->_userApplType
           & ptr->_except;
  }
};

template<>
struct cdu_pod_traits<IndustryFareAppl::ExceptAppl>: std::true_type{};

} // tse
