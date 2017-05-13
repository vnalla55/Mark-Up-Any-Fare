//----------------------------------------------------------------------------
//     (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DiscountSegInfo.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class DiscountInfo : public RuleItemInfo
{
public:
  static enum
  {
    CHILD = 19,
    TOUR,
    AGENT,
    OTHERS
  } DiscountType;

  DiscountInfo()
    : _category(0),
      _unavailtag(' '),
      _discPercent(0),
      _fareAmt1(0),
      _fareAmt2(0),
      _geoTblItemNo(0),
      _segCnt(0),
      _discPercentNoDec(0),
      _noDec1(0),
      _noDec2(0),
      _minAge(0),
      _maxAge(0),
      _firstOccur(0),
      _lastOccur(0),
      _discAppl(' '),
      _psgid(' '),
      _farecalcInd(' '),
      _owrt(' '),
      _baseFareInd(' '),
      _bookingAppl(' '),
      _resultOwrt(' '),
      _accInd(' '),
      _accTvlAllSectors(' '),
      _accTvlOut(' '),
      _accTvlOneSector(' '),
      _accTvlSameCpmt(' '),
      _accTvlSameRule(' '),
      _accPsgAppl(' '),
      _fareClassBkgCodeInd(' '),
      _inhibit(' ')
  {
  }
  ~DiscountInfo()
  { // Nuke the Kids!
    std::vector<DiscountSegInfo*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    {
      delete *SegIt;
    }
    _segs.clear();
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  MoneyAmount& discPercent() { return _discPercent; }
  const MoneyAmount& discPercent() const { return _discPercent; }

  MoneyAmount& fareAmt1() { return _fareAmt1; }
  const MoneyAmount& fareAmt1() const { return _fareAmt1; }

  MoneyAmount& fareAmt2() { return _fareAmt2; }
  const MoneyAmount& fareAmt2() const { return _fareAmt2; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& category() { return _category; }
  const int& category() const { return _category; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  int& discPercentNoDec() { return _discPercentNoDec; }
  const int& discPercentNoDec() const { return _discPercentNoDec; }

  int& noDec1() { return _noDec1; }
  const int& noDec1() const { return _noDec1; }

  int& noDec2() { return _noDec2; }
  const int& noDec2() const { return _noDec2; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  int& firstOccur() { return _firstOccur; }
  const int& firstOccur() const { return _firstOccur; }

  int& lastOccur() { return _lastOccur; }
  const int& lastOccur() const { return _lastOccur; }

  Indicator& discAppl() { return _discAppl; }
  const Indicator& discAppl() const { return _discAppl; }

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  Indicator& psgid() { return _psgid; }
  const Indicator& psgid() const { return _psgid; }

  Indicator& farecalcInd() { return _farecalcInd; }
  const Indicator& farecalcInd() const { return _farecalcInd; }

  Indicator& owrt() { return _owrt; }
  const Indicator& owrt() const { return _owrt; }

  Indicator& baseFareInd() { return _baseFareInd; }
  const Indicator& baseFareInd() const { return _baseFareInd; }

  FareClassCode& baseFareClass() { return _baseFareClass; }
  const FareClassCode& baseFareClass() const { return _baseFareClass; }

  PaxTypeCode& basePsgType() { return _basePsgType; }
  const PaxTypeCode& basePsgType() const { return _basePsgType; }

  FareType& baseFareType() { return _baseFareType; }
  const FareType& baseFareType() const { return _baseFareType; }

  Indicator& bookingAppl() { return _bookingAppl; }
  const Indicator& bookingAppl() const { return _bookingAppl; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  LocCode& betweenMarket() { return _betweenMarket; }
  const LocCode& betweenMarket() const { return _betweenMarket; }

  LocCode& andMarket() { return _andMarket; }
  const LocCode& andMarket() const { return _andMarket; }

  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }

  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }

  Indicator& resultOwrt() { return _resultOwrt; }
  const Indicator& resultOwrt() const { return _resultOwrt; }

  FareClassCode& resultFareClass() { return _resultFareClass; }
  const FareClassCode& resultFareClass() const { return _resultFareClass; }

  BookingCode& resultBookingCode() { return _resultBookingCode; }
  const BookingCode& resultBookingCode() const { return _resultBookingCode; }

  TktCode& tktCode() { return _tktCode; }
  const TktCode& tktCode() const { return _tktCode; }

  TktCodeModifier& tktCodeModifier() { return _tktCodeModifier; }
  const TktCodeModifier& tktCodeModifier() const { return _tktCodeModifier; }

  TktDesignator& tktDesignator() { return _tktDesignator; }
  const TktDesignator& tktDesignator() const { return _tktDesignator; }

  TktDesignatorModifier& tktDesignatorModifier() { return _tktDesignatorModifier; }
  const TktDesignatorModifier& tktDesignatorModifier() const { return _tktDesignatorModifier; }

  Indicator& accInd() { return _accInd; }
  const Indicator& accInd() const { return _accInd; }

  Indicator& accTvlAllSectors() { return _accTvlAllSectors; }
  const Indicator& accTvlAllSectors() const { return _accTvlAllSectors; }

  Indicator& accTvlOut() { return _accTvlOut; }
  const Indicator& accTvlOut() const { return _accTvlOut; }

  Indicator& accTvlOneSector() { return _accTvlOneSector; }
  const Indicator& accTvlOneSector() const { return _accTvlOneSector; }

  Indicator& accTvlSameCpmt() { return _accTvlSameCpmt; }
  const Indicator& accTvlSameCpmt() const { return _accTvlSameCpmt; }

  Indicator& accTvlSameRule() { return _accTvlSameRule; }
  const Indicator& accTvlSameRule() const { return _accTvlSameRule; }

  Indicator& accPsgAppl() { return _accPsgAppl; }
  const Indicator& accPsgAppl() const { return _accPsgAppl; }

  Indicator& fareClassBkgCodeInd() { return _fareClassBkgCodeInd; }
  const Indicator& fareClassBkgCodeInd() const { return _fareClassBkgCodeInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<DiscountSegInfo*>& segs() { return _segs; }
  const std::vector<DiscountSegInfo*>& segs() const { return _segs; }

  bool operator==(const DiscountInfo& rhs) const
  {
    bool eq((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_category == rhs._category) &&
            (_unavailtag == rhs._unavailtag) && (_discPercent == rhs._discPercent) &&
            (_fareAmt1 == rhs._fareAmt1) && (_fareAmt2 == rhs._fareAmt2) &&
            (_geoTblItemNo == rhs._geoTblItemNo) && (_segCnt == rhs._segCnt) &&
            (_discPercentNoDec == rhs._discPercentNoDec) && (_noDec1 == rhs._noDec1) &&
            (_noDec2 == rhs._noDec2) && (_minAge == rhs._minAge) && (_maxAge == rhs._maxAge) &&
            (_firstOccur == rhs._firstOccur) && (_lastOccur == rhs._lastOccur) &&
            (_discAppl == rhs._discAppl) && (_paxType == rhs._paxType) && (_psgid == rhs._psgid) &&
            (_farecalcInd == rhs._farecalcInd) && (_owrt == rhs._owrt) &&
            (_baseFareInd == rhs._baseFareInd) && (_baseFareClass == rhs._baseFareClass) &&
            (_basePsgType == rhs._basePsgType) && (_baseFareType == rhs._baseFareType) &&
            (_bookingAppl == rhs._bookingAppl) && (_bookingCode == rhs._bookingCode) &&
            (_betweenMarket == rhs._betweenMarket) && (_andMarket == rhs._andMarket) &&
            (_cur1 == rhs._cur1) && (_cur2 == rhs._cur2) && (_resultOwrt == rhs._resultOwrt) &&
            (_resultFareClass == rhs._resultFareClass) &&
            (_resultBookingCode == rhs._resultBookingCode) && (_tktCode == rhs._tktCode) &&
            (_tktCodeModifier == rhs._tktCodeModifier) && (_tktDesignator == rhs._tktDesignator) &&
            (_tktDesignatorModifier == rhs._tktDesignatorModifier) && (_accInd == rhs._accInd) &&
            (_accTvlAllSectors == rhs._accTvlAllSectors) && (_accTvlOut == rhs._accTvlOut) &&
            (_accTvlOneSector == rhs._accTvlOneSector) &&
            (_accTvlSameCpmt == rhs._accTvlSameCpmt) && (_accTvlSameRule == rhs._accTvlSameRule) &&
            (_accPsgAppl == rhs._accPsgAppl) &&
            (_fareClassBkgCodeInd == rhs._fareClassBkgCodeInd) && (_inhibit == rhs._inhibit) &&
            (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(DiscountInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._category = OTHERS;
    obj._unavailtag = 'A';
    obj._discPercent = 2.22;
    obj._fareAmt1 = 3.33;
    obj._fareAmt2 = 4.44;
    obj._geoTblItemNo = 5;
    obj._segCnt = 6;
    obj._discPercentNoDec = 7;
    obj._noDec1 = 8;
    obj._noDec2 = 9;
    obj._minAge = 10;
    obj._maxAge = 11;
    obj._firstOccur = 12;
    obj._lastOccur = 13;
    obj._discAppl = 'B';
    obj._paxType = "CDE";
    obj._psgid = 'F';
    obj._farecalcInd = 'G';
    obj._owrt = 'H';
    obj._baseFareInd = 'I';
    obj._baseFareClass = "aaaaaaaa";
    obj._basePsgType = "JKL";
    obj._baseFareType = "MNOPQRST";
    obj._bookingAppl = 'U';
    obj._bookingCode = "VW";
    obj._betweenMarket = "defghijk";
    obj._andMarket = "lmnopqrs";
    obj._cur1 = "tuv";
    obj._cur2 = "wxy";
    obj._resultOwrt = '0';
    obj._resultFareClass = "123";
    obj._resultBookingCode = "45";
    obj._tktCode = "BCDEFGHIJK";
    obj._tktCodeModifier = "LMN";
    obj._tktDesignator = "OPQRSTUVWX";
    obj._tktDesignatorModifier = "YZa";
    obj._accInd = 'b';
    obj._accTvlAllSectors = 'c';
    obj._accTvlOut = 'd';
    obj._accTvlOneSector = 'e';
    obj._accTvlSameCpmt = 'f';
    obj._accTvlSameRule = 'g';
    obj._accPsgAppl = 'h';
    obj._fareClassBkgCodeInd = 'i';
    obj._inhibit = 'j';

    DiscountSegInfo* dsi1 = new DiscountSegInfo;
    DiscountSegInfo* dsi2 = new DiscountSegInfo;

    DiscountSegInfo::dummyData(*dsi1);
    DiscountSegInfo::dummyData(*dsi2);

    obj._segs.push_back(dsi1);
    obj._segs.push_back(dsi2);
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
  CatNumber _category;
  Indicator _unavailtag;
  MoneyAmount _discPercent;
  MoneyAmount _fareAmt1;
  MoneyAmount _fareAmt2;
  int _geoTblItemNo;
  int _segCnt;
  int _discPercentNoDec;
  int _noDec1;
  int _noDec2;
  int _minAge;
  int _maxAge;
  int _firstOccur;
  int _lastOccur;
  Indicator _discAppl;
  PaxTypeCode _paxType;
  Indicator _psgid;
  Indicator _farecalcInd;
  Indicator _owrt;
  Indicator _baseFareInd;
  FareClassCode _baseFareClass;
  PaxTypeCode _basePsgType;
  FareType _baseFareType;
  Indicator _bookingAppl;
  BookingCode _bookingCode;
  LocCode _betweenMarket;
  LocCode _andMarket;
  CurrencyCode _cur1;
  CurrencyCode _cur2;
  Indicator _resultOwrt;
  FareClassCode _resultFareClass;
  BookingCode _resultBookingCode;
  TktCode _tktCode;
  TktCodeModifier _tktCodeModifier;
  TktDesignator _tktDesignator;
  TktDesignatorModifier _tktDesignatorModifier;
  Indicator _accInd;
  Indicator _accTvlAllSectors;
  Indicator _accTvlOut;
  Indicator _accTvlOneSector;
  Indicator _accTvlSameCpmt;
  Indicator _accTvlSameRule;
  Indicator _accPsgAppl;
  Indicator _fareClassBkgCodeInd;
  Indicator _inhibit;
  std::vector<tse::DiscountSegInfo*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _category);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _discPercent);
    FLATTENIZE(archive, _fareAmt1);
    FLATTENIZE(archive, _fareAmt2);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _discPercentNoDec);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _noDec2);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _firstOccur);
    FLATTENIZE(archive, _lastOccur);
    FLATTENIZE(archive, _discAppl);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _psgid);
    FLATTENIZE(archive, _farecalcInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _baseFareInd);
    FLATTENIZE(archive, _baseFareClass);
    FLATTENIZE(archive, _basePsgType);
    FLATTENIZE(archive, _baseFareType);
    FLATTENIZE(archive, _bookingAppl);
    FLATTENIZE(archive, _bookingCode);
    FLATTENIZE(archive, _betweenMarket);
    FLATTENIZE(archive, _andMarket);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _resultOwrt);
    FLATTENIZE(archive, _resultFareClass);
    FLATTENIZE(archive, _resultBookingCode);
    FLATTENIZE(archive, _tktCode);
    FLATTENIZE(archive, _tktCodeModifier);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _tktDesignatorModifier);
    FLATTENIZE(archive, _accInd);
    FLATTENIZE(archive, _accTvlAllSectors);
    FLATTENIZE(archive, _accTvlOut);
    FLATTENIZE(archive, _accTvlOneSector);
    FLATTENIZE(archive, _accTvlSameCpmt);
    FLATTENIZE(archive, _accTvlSameRule);
    FLATTENIZE(archive, _accPsgAppl);
    FLATTENIZE(archive, _fareClassBkgCodeInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _segs);
  }

protected:
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_category
           & ptr->_unavailtag
           & ptr->_discPercent
           & ptr->_fareAmt1
           & ptr->_fareAmt2
           & ptr->_geoTblItemNo
           & ptr->_segCnt
           & ptr->_discPercentNoDec
           & ptr->_noDec1
           & ptr->_noDec2
           & ptr->_minAge
           & ptr->_maxAge
           & ptr->_firstOccur
           & ptr->_lastOccur
           & ptr->_discAppl
           & ptr->_paxType
           & ptr->_psgid
           & ptr->_farecalcInd
           & ptr->_owrt
           & ptr->_baseFareInd
           & ptr->_baseFareClass
           & ptr->_basePsgType
           & ptr->_baseFareType
           & ptr->_bookingAppl
           & ptr->_bookingCode
           & ptr->_betweenMarket
           & ptr->_andMarket
           & ptr->_cur1
           & ptr->_cur2
           & ptr->_resultOwrt
           & ptr->_resultFareClass
           & ptr->_resultBookingCode
           & ptr->_tktCode
           & ptr->_tktCodeModifier
           & ptr->_tktDesignator
           & ptr->_tktDesignatorModifier
           & ptr->_accInd
           & ptr->_accTvlAllSectors
           & ptr->_accTvlOut
           & ptr->_accTvlOneSector
           & ptr->_accTvlSameCpmt
           & ptr->_accTvlSameRule
           & ptr->_accPsgAppl
           & ptr->_fareClassBkgCodeInd
           & ptr->_inhibit
           & ptr->_segs;
  }

  DiscountInfo(const DiscountInfo&);
  DiscountInfo& operator=(const DiscountInfo&);
};
}

