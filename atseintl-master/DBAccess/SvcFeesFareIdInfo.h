#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class SvcFeesFareIdInfo // T189
{
public:
  SvcFeesFareIdInfo()
    : _itemNo(-1),
      _seqNo(-1),
      _validityInd('Y'),
      _fareApplInd(' '),
      _owrt(' '),
      _ruleTariff(-1),
      _routing(-1),
      _source(' '),
      _minFareAmt1(0),
      _maxFareAmt1(0),
      _noDec1(0),
      _minFareAmt2(0),
      _maxFareAmt2(0),
      _noDec2(0)
  {
  }
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
  long long& itemNo() { return _itemNo; }
  long long itemNo() const { return _itemNo; }
  SequenceNumber& seqNo() { return _seqNo; }
  SequenceNumber seqNo() const { return _seqNo; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }
  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }
  Indicator& fareApplInd() { return _fareApplInd; }
  Indicator fareApplInd() const { return _fareApplInd; }
  Indicator& owrt() { return _owrt; }
  Indicator owrt() const { return _owrt; }
  TariffNumber& ruleTariff() { return _ruleTariff; }
  TariffNumber ruleTariff() const { return _ruleTariff; }
  ServiceRuleTariffInd& ruleTariffInd() { return _ruleTariffInd; }
  const ServiceRuleTariffInd& ruleTariffInd() const { return _ruleTariffInd; }
  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }
  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }
  FareTypeAbbrev& fareType() { return _fareType; }
  const FareTypeAbbrev& fareType() const { return _fareType; }
  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }
  int& routing() { return _routing; }
  int routing() const { return _routing; }
  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }
  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }
  Indicator& source() { return _source; }
  Indicator source() const { return _source; }
  MoneyAmount& minFareAmt1() { return _minFareAmt1; }
  MoneyAmount minFareAmt1() const { return _minFareAmt1; }
  MoneyAmount& maxFareAmt1() { return _maxFareAmt1; }
  MoneyAmount maxFareAmt1() const { return _maxFareAmt1; }
  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }
  short& noDec1() { return _noDec1; }
  short noDec1() const { return _noDec1; }
  MoneyAmount& minFareAmt2() { return _minFareAmt2; }
  MoneyAmount minFareAmt2() const { return _minFareAmt2; }
  MoneyAmount& maxFareAmt2() { return _maxFareAmt2; }
  MoneyAmount maxFareAmt2() const { return _maxFareAmt2; }
  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }
  short& noDec2() { return _noDec2; }
  short noDec2() const { return _noDec2; }

  bool operator==(const SvcFeesFareIdInfo& rhs) const
  {
    return _vendor == rhs._vendor && _itemNo == rhs._itemNo && _seqNo == rhs._seqNo &&
           _createDate == rhs._createDate && _expireDate == rhs._expireDate &&
           _validityInd == rhs._validityInd && _fareApplInd == rhs._fareApplInd &&
           _owrt == rhs._owrt && _ruleTariff == rhs._ruleTariff &&
           _ruleTariffInd == rhs._ruleTariffInd && _rule == rhs._rule &&
           _fareClass == rhs._fareClass && _fareType == rhs._fareType && _paxType == rhs._paxType &&
           _routing == rhs._routing && _bookingCode1 == rhs._bookingCode1 &&
           _bookingCode2 == rhs._bookingCode2 && _source == rhs._source &&
           _minFareAmt1 == rhs._minFareAmt1 && _maxFareAmt1 == rhs._maxFareAmt1 &&
           _cur1 == rhs._cur1 && _noDec1 == rhs._noDec1 && _minFareAmt2 == rhs._minFareAmt2 &&
           _maxFareAmt2 == rhs._maxFareAmt2 && _cur2 == rhs._cur2 && _noDec2 == rhs._noDec2;
  }

  static void dummyData(SvcFeesFareIdInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 977;
    obj._seqNo = 22222;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'Y';
    obj._fareApplInd = 'N';
    obj._owrt = '1';
    obj._ruleTariff = 33333;
    obj._ruleTariffInd = "abc";
    obj._rule = "defg";
    obj._fareClass = "87654321";
    obj._fareType = "xyz";
    obj._paxType = "ADT";
    obj._routing = 54321;
    obj._bookingCode1 = "CN";
    obj._bookingCode2 = "Y";
    obj._source = 'S';
    obj._minFareAmt1 = 200;
    obj._maxFareAmt1 = 250;
    obj._cur1 = "USD";
    obj._noDec1 = 2;
    obj._minFareAmt2 = 100;
    obj._maxFareAmt2 = 150;
    obj._cur2 = "AUD";
    obj._noDec2 = 3;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _fareApplInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _ruleTariffInd);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _source);
    FLATTENIZE(archive, _minFareAmt1);
    FLATTENIZE(archive, _maxFareAmt1);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _minFareAmt2);
    FLATTENIZE(archive, _maxFareAmt2);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _noDec2);
  }

private:
  VendorCode _vendor;
  long long _itemNo;
  SequenceNumber _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd; // 'Y' | 'N'
  Indicator _fareApplInd; // ' ' | 'N'
  Indicator _owrt; // ' ' | '1' | '2' | '3'
  TariffNumber _ruleTariff;
  ServiceRuleTariffInd _ruleTariffInd;
  RuleNumber _rule;
  FareClassCode _fareClass;
  FareTypeAbbrev _fareType;
  PaxTypeCode _paxType;
  int _routing;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  Indicator _source;
  MoneyAmount _minFareAmt1;
  MoneyAmount _maxFareAmt1;
  CurrencyCode _cur1;
  short _noDec1;
  MoneyAmount _minFareAmt2;
  MoneyAmount _maxFareAmt2;
  CurrencyCode _cur2;
  short _noDec2;
};
} // tse

