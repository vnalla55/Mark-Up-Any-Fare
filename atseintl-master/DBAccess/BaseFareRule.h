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
#include "DBAccess/Flattenizable.h"

namespace tse
{

class BaseFareRule
{
public:
  bool operator==(const BaseFareRule& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_baseminFare1 == rhs._baseminFare1) && (_baseMaxFare1 == rhs._baseMaxFare1) &&
            (_baseminFare2 == rhs._baseminFare2) && (_baseMaxFare2 == rhs._baseMaxFare2) &&
            (_nodec1 == rhs._nodec1) && (_nodec2 == rhs._nodec2) &&
            (_baseRuleTariff == rhs._baseRuleTariff) && (_inhibit == rhs._inhibit) &&
            (_validityInd == rhs._validityInd) && (_baseFareAppl == rhs._baseFareAppl) &&
            (_baseowrt == rhs._baseowrt) && (_basepubcalc == rhs._basepubcalc) &&
            (_baseseasonType == rhs._baseseasonType) && (_basedowType == rhs._basedowType) &&
            (_basepricingcatType == rhs._basepricingcatType) &&
            (_baseglobalDir == rhs._baseglobalDir) && (_carrier == rhs._carrier) &&
            (_baseRuleNo == rhs._baseRuleNo) && (_baseFareClass == rhs._baseFareClass) &&
            (_baseFareType == rhs._baseFareType) && (_basepsgType == rhs._basepsgType) &&
            (_baseRoutingAppl == rhs._baseRoutingAppl) && (_baseRouting == rhs._baseRouting) &&
            (_basefootNote1 == rhs._basefootNote1) && (_basefootNote2 == rhs._basefootNote2) &&
            (_bookingCode1 == rhs._bookingCode1) && (_bookingCode2 == rhs._bookingCode2) &&
            (_market1 == rhs._market1) && (_market2 == rhs._market2) &&
            (_baseCur1 == rhs._baseCur1) && (_baseCur2 == rhs._baseCur2));
  }

  static void dummyData(BaseFareRule& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._baseminFare1 = 3.33;
    obj._baseMaxFare1 = 4.44;
    obj._baseminFare2 = 5.55;
    obj._baseMaxFare2 = 6.66;
    obj._nodec1 = 7;
    obj._nodec2 = 8;
    obj._baseRuleTariff = 9;
    obj._inhibit = 'E';
    obj._validityInd = 'F';
    obj._baseFareAppl = 'G';
    obj._baseowrt = 'H';
    obj._basepubcalc = 'I';
    obj._baseseasonType = 'J';
    obj._basedowType = 'K';
    obj._basepricingcatType = 'L';
    obj._baseglobalDir = GlobalDirection::US;
    obj._carrier = "MNO";
    obj._baseRuleNo = "PQRS";
    obj._baseFareClass = "TUVWXYZa";
    obj._baseFareType = "bcd";
    obj._basepsgType = "jkl";
    obj._baseRoutingAppl = 'm';
    obj._baseRouting = "nopq";
    obj._basefootNote1 = "rs";
    obj._basefootNote2 = "tu";
    obj._bookingCode1 = "vw";
    obj._bookingCode2 = "xy";
    obj._market1 = "z1234567";
    obj._market2 = "890ABCDE";
    obj._baseCur1 = "FGH";
    obj._baseCur2 = "IJK";
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  VendorCode _vendor;
  int _itemNo = 0;
  int _seqNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  MoneyAmount _baseminFare1 = 0;
  MoneyAmount _baseMaxFare1 = 0;
  MoneyAmount _baseminFare2 = 0;
  MoneyAmount _baseMaxFare2 = 0;
  CurrencyNoDec _nodec1 = 0;
  CurrencyNoDec _nodec2 = 0;
  TariffNumber _baseRuleTariff = 0;
  Indicator _inhibit = ' ';
  Indicator _validityInd = ' ';
  Indicator _baseFareAppl = ' ';
  Indicator _baseowrt = ' ';
  Indicator _basepubcalc = ' ';
  Indicator _baseseasonType = ' ';
  Indicator _basedowType = ' ';
  Indicator _basepricingcatType = ' ';
  GlobalDirection _baseglobalDir = GlobalDirection::NO_DIR;
  CarrierCode _carrier;
  RuleNumber _baseRuleNo;
  FareClassCodeC _baseFareClass; // 8
  FareTypeAbbrevC _baseFareType; // 3
  PaxTypeCode _basepsgType;
  Indicator _baseRoutingAppl = ' ';
  RoutingNumber _baseRouting;
  Footnote _basefootNote1;
  Footnote _basefootNote2;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  LocCode _market1;
  LocCode _market2;
  CurrencyCode _baseCur1;
  CurrencyCode _baseCur2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _baseminFare1);
    FLATTENIZE(archive, _baseMaxFare1);
    FLATTENIZE(archive, _baseminFare2);
    FLATTENIZE(archive, _baseMaxFare2);
    FLATTENIZE(archive, _nodec1);
    FLATTENIZE(archive, _nodec2);
    FLATTENIZE(archive, _baseRuleTariff);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _baseFareAppl);
    FLATTENIZE(archive, _baseowrt);
    FLATTENIZE(archive, _basepubcalc);
    FLATTENIZE(archive, _baseseasonType);
    FLATTENIZE(archive, _basedowType);
    FLATTENIZE(archive, _basepricingcatType);
    FLATTENIZE(archive, _baseglobalDir);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _baseRuleNo);
    FLATTENIZE(archive, _baseFareClass);
    FLATTENIZE(archive, _baseFareType);
    FLATTENIZE(archive, _basepsgType);
    FLATTENIZE(archive, _baseRoutingAppl);
    FLATTENIZE(archive, _baseRouting);
    FLATTENIZE(archive, _basefootNote1);
    FLATTENIZE(archive, _basefootNote2);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _market1);
    FLATTENIZE(archive, _market2);
    FLATTENIZE(archive, _baseCur1);
    FLATTENIZE(archive, _baseCur2);
  }

protected:
public:
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  MoneyAmount& baseminFare1() { return _baseminFare1; }
  const MoneyAmount& baseminFare1() const { return _baseminFare1; }

  MoneyAmount& baseMaxFare1() { return _baseMaxFare1; }
  const MoneyAmount& baseMaxFare1() const { return _baseMaxFare1; }

  MoneyAmount& baseminFare2() { return _baseminFare2; }
  const MoneyAmount& baseminFare2() const { return _baseminFare2; }

  MoneyAmount& baseMaxFare2() { return _baseMaxFare2; }
  const MoneyAmount& baseMaxFare2() const { return _baseMaxFare2; }

  CurrencyNoDec& nodec1() { return _nodec1; }
  const CurrencyNoDec& nodec1() const { return _nodec1; }

  CurrencyNoDec& nodec2() { return _nodec2; }
  const CurrencyNoDec& nodec2() const { return _nodec2; }

  TariffNumber& baseRuleTariff() { return _baseRuleTariff; }
  const TariffNumber& baseRuleTariff() const { return _baseRuleTariff; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& baseFareAppl() { return _baseFareAppl; }
  const Indicator& baseFareAppl() const { return _baseFareAppl; }

  Indicator& baseowrt() { return _baseowrt; }
  const Indicator& baseowrt() const { return _baseowrt; }

  Indicator& basepubcalc() { return _basepubcalc; }
  const Indicator& basepubcalc() const { return _basepubcalc; }

  Indicator& baseseasonType() { return _baseseasonType; }
  const Indicator& baseseasonType() const { return _baseseasonType; }

  Indicator& basedowType() { return _basedowType; }
  const Indicator& basedowType() const { return _basedowType; }

  Indicator& basepricingcatType() { return _basepricingcatType; }
  const Indicator& basepricingcatType() const { return _basepricingcatType; }

  GlobalDirection& baseglobalDir() { return _baseglobalDir; }
  const GlobalDirection& baseglobalDir() const { return _baseglobalDir; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  RuleNumber& baseRuleNo() { return _baseRuleNo; }
  const RuleNumber& baseRuleNo() const { return _baseRuleNo; }

  FareClassCodeC& baseFareClass() { return _baseFareClass; }
  const FareClassCodeC& baseFareClass() const { return _baseFareClass; }

  FareTypeAbbrevC& baseFareType() { return _baseFareType; }
  const FareTypeAbbrevC& baseFareType() const { return _baseFareType; }

  PaxTypeCode& basepsgType() { return _basepsgType; }
  const PaxTypeCode& basepsgType() const { return _basepsgType; }

  Indicator& baseRoutingAppl() { return _baseRoutingAppl; }
  const Indicator& baseRoutingAppl() const { return _baseRoutingAppl; }

  RoutingNumber& baseRouting() { return _baseRouting; }
  const RoutingNumber& baseRouting() const { return _baseRouting; }

  Footnote& basefootNote1() { return _basefootNote1; }
  const Footnote& basefootNote1() const { return _basefootNote1; }

  Footnote& basefootNote2() { return _basefootNote2; }
  const Footnote& basefootNote2() const { return _basefootNote2; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  LocCode& market1() { return _market1; }
  const LocCode& market1() const { return _market1; }

  LocCode& market2() { return _market2; }
  const LocCode& market2() const { return _market2; }

  CurrencyCode& baseCur1() { return _baseCur1; }
  const CurrencyCode& baseCur1() const { return _baseCur1; }

  CurrencyCode& baseCur2() { return _baseCur2; }
  const CurrencyCode& baseCur2() const { return _baseCur2; }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_itemNo & ptr->_seqNo & ptr->_expireDate &
           ptr->_createDate & ptr->_baseminFare1 & ptr->_baseMaxFare1 & ptr->_baseminFare2 &
           ptr->_baseMaxFare2 & ptr->_nodec1 & ptr->_nodec2 & ptr->_baseRuleTariff & ptr->_inhibit &
           ptr->_validityInd & ptr->_baseFareAppl & ptr->_baseowrt & ptr->_basepubcalc &
           ptr->_baseseasonType & ptr->_basedowType & ptr->_basepricingcatType &
           ptr->_baseglobalDir & ptr->_carrier & ptr->_baseRuleNo & ptr->_baseFareClass &
           ptr->_baseFareType & ptr->_basepsgType & ptr->_baseRoutingAppl & ptr->_baseRouting &
           ptr->_basefootNote1 & ptr->_basefootNote2 & ptr->_bookingCode1 & ptr->_bookingCode2 &
           ptr->_market1 & ptr->_market2 & ptr->_baseCur1 & ptr->_baseCur2;
  }
};
}

