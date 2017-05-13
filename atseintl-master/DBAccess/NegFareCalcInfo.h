//----------------------------------------------------------------------------
//       (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class NegFareCalcInfo : public RuleItemInfo
{
public:
  NegFareCalcInfo()
    : _seqNo(0),
      _directionality(' '),
      _bundledInd(' '),
      _netSellingInd(' '),
      _fareInd(' '),
      _sellingPercentNoDec(0),
      _sellingPercent(0),
      _sellingNoDec1(0),
      _sellingFareAmt1(0),
      _sellingNoDec2(0),
      _sellingFareAmt2(0),
      _calcPercentMinNoDec(0),
      _calcPercentMin(0),
      _calcPercentMaxNoDec(0),
      _calcPercentMax(0),
      _calcNoDec1(0),
      _calcMinFareAmt1(0),
      _calcMaxFareAmt1(0),
      _calcNoDec2(0),
      _calcMinFareAmt2(0),
      _calcMaxFareAmt2(0),
      _inhibit(' ')
  {
  }
  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  Zone& userDefZone1() { return _userDefZone1; }
  const Zone& userDefZone1() const { return _userDefZone1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Zone& userDefZone2() { return _userDefZone2; }
  const Zone& userDefZone2() const { return _userDefZone2; }

  Indicator& bundledInd() { return _bundledInd; }
  const Indicator& bundledInd() const { return _bundledInd; }

  Indicator& netSellingInd() { return _netSellingInd; }
  const Indicator& netSellingInd() const { return _netSellingInd; }

  Indicator& fareInd() { return _fareInd; }
  const Indicator& fareInd() const { return _fareInd; }

  int& sellingPercentNoDec() { return _sellingPercentNoDec; }
  const int& sellingPercentNoDec() const { return _sellingPercentNoDec; }

  Percent& sellingPercent() { return _sellingPercent; }
  const Percent& sellingPercent() const { return _sellingPercent; }

  int& sellingNoDec1() { return _sellingNoDec1; }
  const int& sellingNoDec1() const { return _sellingNoDec1; }

  MoneyAmount& sellingFareAmt1() { return _sellingFareAmt1; }
  const MoneyAmount& sellingFareAmt1() const { return _sellingFareAmt1; }

  CurrencyCode& sellingCur1() { return _sellingCur1; }
  const CurrencyCode& sellingCur1() const { return _sellingCur1; }

  int& sellingNoDec2() { return _sellingNoDec2; }
  const int& sellingNoDec2() const { return _sellingNoDec2; }

  MoneyAmount& sellingFareAmt2() { return _sellingFareAmt2; }
  const MoneyAmount& sellingFareAmt2() const { return _sellingFareAmt2; }

  CurrencyCode& sellingCur2() { return _sellingCur2; }
  const CurrencyCode& sellingCur2() const { return _sellingCur2; }

  int& calcPercentMinNoDec() { return _calcPercentMinNoDec; }
  const int& calcPercentMinNoDec() const { return _calcPercentMinNoDec; }

  Percent& calcPercentMin() { return _calcPercentMin; }
  const Percent& calcPercentMin() const { return _calcPercentMin; }

  int& calcPercentMaxNoDec() { return _calcPercentMaxNoDec; }
  const int& calcPercentMaxNoDec() const { return _calcPercentMaxNoDec; }

  Percent& calcPercentMax() { return _calcPercentMax; }
  const Percent& calcPercentMax() const { return _calcPercentMax; }

  int& calcNoDec1() { return _calcNoDec1; }
  const int& calcNoDec1() const { return _calcNoDec1; }

  MoneyAmount& calcMinFareAmt1() { return _calcMinFareAmt1; }
  const MoneyAmount& calcMinFareAmt1() const { return _calcMinFareAmt1; }

  MoneyAmount& calcMaxFareAmt1() { return _calcMaxFareAmt1; }
  const MoneyAmount& calcMaxFareAmt1() const { return _calcMaxFareAmt1; }

  CurrencyCode& calcCur1() { return _calcCur1; }
  const CurrencyCode& calcCur1() const { return _calcCur1; }

  int& calcNoDec2() { return _calcNoDec2; }
  const int& calcNoDec2() const { return _calcNoDec2; }

  MoneyAmount& calcMinFareAmt2() { return _calcMinFareAmt2; }
  const MoneyAmount& calcMinFareAmt2() const { return _calcMinFareAmt2; }

  MoneyAmount& calcMaxFareAmt2() { return _calcMaxFareAmt2; }
  const MoneyAmount& calcMaxFareAmt2() const { return _calcMaxFareAmt2; }

  CurrencyCode& calcCur2() { return _calcCur2; }
  const CurrencyCode& calcCur2() const { return _calcCur2; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const NegFareCalcInfo& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
        (_userDefZone1 == rhs._userDefZone1) && (_loc2 == rhs._loc2) &&
        (_userDefZone2 == rhs._userDefZone2) && (_bundledInd == rhs._bundledInd) &&
        (_netSellingInd == rhs._netSellingInd) && (_fareInd == rhs._fareInd) &&
        (_sellingPercentNoDec == rhs._sellingPercentNoDec) &&
        (_sellingPercent == rhs._sellingPercent) && (_sellingNoDec1 == rhs._sellingNoDec1) &&
        (_sellingFareAmt1 == rhs._sellingFareAmt1) && (_sellingCur1 == rhs._sellingCur1) &&
        (_sellingNoDec2 == rhs._sellingNoDec2) && (_sellingFareAmt2 == rhs._sellingFareAmt2) &&
        (_sellingCur2 == rhs._sellingCur2) && (_calcPercentMinNoDec == rhs._calcPercentMinNoDec) &&
        (_calcPercentMin == rhs._calcPercentMin) &&
        (_calcPercentMaxNoDec == rhs._calcPercentMaxNoDec) &&
        (_calcPercentMax == rhs._calcPercentMax) && (_calcNoDec1 == rhs._calcNoDec1) &&
        (_calcMinFareAmt1 == rhs._calcMinFareAmt1) && (_calcMaxFareAmt1 == rhs._calcMaxFareAmt1) &&
        (_calcCur1 == rhs._calcCur1) && (_calcNoDec2 == rhs._calcNoDec2) &&
        (_calcMinFareAmt2 == rhs._calcMinFareAmt2) && (_calcMaxFareAmt2 == rhs._calcMaxFareAmt2) &&
        (_calcCur2 == rhs._calcCur2) && (_inhibit == rhs._inhibit));
  }

private:
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _directionality;
  LocKey _loc1;
  Zone _userDefZone1;
  LocKey _loc2;
  Zone _userDefZone2;
  Indicator _bundledInd;
  Indicator _netSellingInd;
  Indicator _fareInd;
  int _sellingPercentNoDec;
  Percent _sellingPercent;
  int _sellingNoDec1;
  MoneyAmount _sellingFareAmt1;
  CurrencyCode _sellingCur1;
  int _sellingNoDec2;
  MoneyAmount _sellingFareAmt2;
  CurrencyCode _sellingCur2;
  int _calcPercentMinNoDec;
  Percent _calcPercentMin;
  int _calcPercentMaxNoDec;
  Percent _calcPercentMax;
  int _calcNoDec1;
  MoneyAmount _calcMinFareAmt1;
  MoneyAmount _calcMaxFareAmt1;
  CurrencyCode _calcCur1;
  int _calcNoDec2;
  MoneyAmount _calcMinFareAmt2;
  MoneyAmount _calcMaxFareAmt2;
  CurrencyCode _calcCur2;
  Indicator _inhibit;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _userDefZone1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _userDefZone2);
    FLATTENIZE(archive, _bundledInd);
    FLATTENIZE(archive, _netSellingInd);
    FLATTENIZE(archive, _fareInd);
    FLATTENIZE(archive, _sellingPercentNoDec);
    FLATTENIZE(archive, _sellingPercent);
    FLATTENIZE(archive, _sellingNoDec1);
    FLATTENIZE(archive, _sellingFareAmt1);
    FLATTENIZE(archive, _sellingCur1);
    FLATTENIZE(archive, _sellingNoDec2);
    FLATTENIZE(archive, _sellingFareAmt2);
    FLATTENIZE(archive, _sellingCur2);
    FLATTENIZE(archive, _calcPercentMinNoDec);
    FLATTENIZE(archive, _calcPercentMin);
    FLATTENIZE(archive, _calcPercentMaxNoDec);
    FLATTENIZE(archive, _calcPercentMax);
    FLATTENIZE(archive, _calcNoDec1);
    FLATTENIZE(archive, _calcMinFareAmt1);
    FLATTENIZE(archive, _calcMaxFareAmt1);
    FLATTENIZE(archive, _calcCur1);
    FLATTENIZE(archive, _calcNoDec2);
    FLATTENIZE(archive, _calcMinFareAmt2);
    FLATTENIZE(archive, _calcMaxFareAmt2);
    FLATTENIZE(archive, _calcCur2);
    FLATTENIZE(archive, _inhibit);
  }

};
}
