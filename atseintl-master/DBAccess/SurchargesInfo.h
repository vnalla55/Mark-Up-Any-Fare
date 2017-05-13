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

class SurchargesInfo : public RuleItemInfo
{
public:
  SurchargesInfo()
    : _unavailTag(' '),
      _geoTblItemNo(0),
      _geoTblItemNoBtw(0),
      _geoTblItemNoAnd(0),
      _surchargeNoDec1(0),
      _surchargeAmt1(0),
      _surchargeNoDec2(0),
      _surchargeAmt2(0),
      _startTime(0),
      _stopTime(0),
      _surchargePercentNoDec(0),
      _surchargePercent(0),
      _startYear(0),
      _startMonth(0),
      _startDay(0),
      _stopYear(0),
      _stopMonth(0),
      _stopDay(0),
      _todAppl(' '),
      _surchargeType(' '),
      _tvlPortion(' '),
      _surchargeGroupInd(' '),
      _surchargeAppl(' '),
      _surchargePercentAppl(' '),
      _inhibit(' '),
      _carrierFltTblItemNo(0),
      _sectorPortion(' ')

  {
  }

  virtual ~SurchargesInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& geoTblItemNoBtw() { return _geoTblItemNoBtw; }
  const int& geoTblItemNoBtw() const { return _geoTblItemNoBtw; }

  int& geoTblItemNoAnd() { return _geoTblItemNoAnd; }
  const int& geoTblItemNoAnd() const { return _geoTblItemNoAnd; }

  int& surchargeNoDec1() { return _surchargeNoDec1; }
  const int& surchargeNoDec1() const { return _surchargeNoDec1; }

  MoneyAmount& surchargeAmt1() { return _surchargeAmt1; }
  const MoneyAmount& surchargeAmt1() const { return _surchargeAmt1; }

  int& surchargeNoDec2() { return _surchargeNoDec2; }
  const int& surchargeNoDec2() const { return _surchargeNoDec2; }

  MoneyAmount& surchargeAmt2() { return _surchargeAmt2; }
  const MoneyAmount& surchargeAmt2() const { return _surchargeAmt2; }

  int& startTime() { return _startTime; }
  const int& startTime() const { return _startTime; }

  int& stopTime() { return _stopTime; }
  const int& stopTime() const { return _stopTime; }

  int& surchargePercentNoDec() { return _surchargePercentNoDec; }
  const int& surchargePercentNoDec() const { return _surchargePercentNoDec; }

  Percent& surchargePercent() { return _surchargePercent; }
  const Percent& surchargePercent() const { return _surchargePercent; }

  uint32_t& startYear() { return _startYear; }
  const uint32_t& startYear() const { return _startYear; }

  uint32_t& startMonth() { return _startMonth; }
  const uint32_t& startMonth() const { return _startMonth; }

  uint32_t& startDay() { return _startDay; }
  const uint32_t& startDay() const { return _startDay; }

  uint32_t& stopYear() { return _stopYear; }
  const uint32_t& stopYear() const { return _stopYear; }

  uint32_t& stopMonth() { return _stopMonth; }
  const uint32_t& stopMonth() const { return _stopMonth; }

  uint32_t& stopDay() { return _stopDay; }
  const uint32_t& stopDay() const { return _stopDay; }

  Indicator& todAppl() { return _todAppl; }
  const Indicator& todAppl() const { return _todAppl; }

  std::string& dow() { return _dow; }
  const std::string& dow() const { return _dow; }

  Indicator& surchargeType() { return _surchargeType; }
  const Indicator& surchargeType() const { return _surchargeType; }

  std::string& equipType() { return _equipType; }
  const std::string& equipType() const { return _equipType; }

  Indicator& tvlPortion() { return _tvlPortion; }
  const Indicator& tvlPortion() const { return _tvlPortion; }

  Indicator& surchargeGroupInd() { return _surchargeGroupInd; }
  const Indicator& surchargeGroupInd() const { return _surchargeGroupInd; }

  Indicator& surchargeAppl() { return _surchargeAppl; }
  const Indicator& surchargeAppl() const { return _surchargeAppl; }

  CurrencyCode& surchargeCur1() { return _surchargeCur1; }
  const CurrencyCode& surchargeCur1() const { return _surchargeCur1; }

  CurrencyCode& surchargeCur2() { return _surchargeCur2; }
  const CurrencyCode& surchargeCur2() const { return _surchargeCur2; }

  Indicator& surchargePercentAppl() { return _surchargePercentAppl; }
  const Indicator& surchargePercentAppl() const { return _surchargePercentAppl; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::string& surchargeDesc() { return _surchargeDesc; }
  const std::string& surchargeDesc() const { return _surchargeDesc; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  int& carrierFltTblItemNo() { return _carrierFltTblItemNo; }
  const int& carrierFltTblItemNo() const { return _carrierFltTblItemNo; }

  Indicator& sectorPortion() { return _sectorPortion; }
  const Indicator& sectorPortion() const { return _sectorPortion; }

  virtual bool operator==(const SurchargesInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailTag == rhs._unavailTag) &&
            (_geoTblItemNo == rhs._geoTblItemNo) && (_geoTblItemNoBtw == rhs._geoTblItemNoBtw) &&
            (_geoTblItemNoAnd == rhs._geoTblItemNoAnd) &&
            (_surchargeNoDec1 == rhs._surchargeNoDec1) && (_surchargeAmt1 == rhs._surchargeAmt1) &&
            (_surchargeNoDec2 == rhs._surchargeNoDec2) && (_surchargeAmt2 == rhs._surchargeAmt2) &&
            (_startTime == rhs._startTime) && (_stopTime == rhs._stopTime) &&
            (_surchargePercentNoDec == rhs._surchargePercentNoDec) &&
            (_surchargePercent == rhs._surchargePercent) && (_startYear == rhs._startYear) &&
            (_startMonth == rhs._startMonth) && (_startDay == rhs._startDay) &&
            (_stopYear == rhs._stopYear) && (_stopMonth == rhs._stopMonth) &&
            (_stopDay == rhs._stopDay) && (_todAppl == rhs._todAppl) && (_dow == rhs._dow) &&
            (_surchargeType == rhs._surchargeType) && (_equipType == rhs._equipType) &&
            (_tvlPortion == rhs._tvlPortion) && (_surchargeGroupInd == rhs._surchargeGroupInd) &&
            (_surchargeAppl == rhs._surchargeAppl) && (_surchargeCur1 == rhs._surchargeCur1) &&
            (_surchargeCur2 == rhs._surchargeCur2) &&
            (_surchargePercentAppl == rhs._surchargePercentAppl) && (_inhibit == rhs._inhibit) &&
            (_surchargeDesc == rhs._surchargeDesc) && (_bookingCode == rhs._bookingCode) &&
            (_carrierFltTblItemNo == rhs._carrierFltTblItemNo) &&
            (_sectorPortion == rhs._sectorPortion));
  }

  static void dummyData(SurchargesInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailTag = 'A';
    obj._geoTblItemNo = 1;
    obj._geoTblItemNoBtw = 2;
    obj._geoTblItemNoAnd = 3;
    obj._surchargeNoDec1 = 4;
    obj._surchargeAmt1 = 5.55;
    obj._surchargeNoDec2 = 6;
    obj._surchargeAmt2 = 7.77;
    obj._startTime = 8;
    obj._stopTime = 9;
    obj._surchargePercentNoDec = 10;
    obj._surchargePercent = 11.111;
    obj._startYear = 12;
    obj._startMonth = 13;
    obj._startDay = 14;
    obj._stopYear = 15;
    obj._stopMonth = 16;
    obj._stopDay = 17;
    obj._todAppl = 'B';
    obj._dow = "gggggggg";
    obj._surchargeType = 'C';
    obj._equipType = "hhhhhhhh";
    obj._tvlPortion = 'D';
    obj._surchargeGroupInd = 'E';
    obj._surchargeAppl = 'F';
    obj._surchargeCur1 = "GHI";
    obj._surchargeCur2 = "JKL";
    obj._surchargePercentAppl = 'M';
    obj._inhibit = 'N';
    obj._surchargeDesc = "iiiiiiii";
    obj._bookingCode = "OP";
    obj._carrierFltTblItemNo = 17;
    obj._sectorPortion = 'S';
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
  Indicator _unavailTag;
  int _geoTblItemNo;
  int _geoTblItemNoBtw;
  int _geoTblItemNoAnd;
  int _surchargeNoDec1;
  MoneyAmount _surchargeAmt1;
  int _surchargeNoDec2;
  MoneyAmount _surchargeAmt2;
  int _startTime;
  int _stopTime;
  int _surchargePercentNoDec;
  Percent _surchargePercent;
  uint32_t _startYear;
  uint32_t _startMonth;
  uint32_t _startDay;
  uint32_t _stopYear;
  uint32_t _stopMonth;
  uint32_t _stopDay;
  Indicator _todAppl;
  std::string _dow;
  Indicator _surchargeType;
  std::string _equipType;
  Indicator _tvlPortion;
  Indicator _surchargeGroupInd;
  Indicator _surchargeAppl;
  CurrencyCode _surchargeCur1;
  CurrencyCode _surchargeCur2;
  Indicator _surchargePercentAppl;
  Indicator _inhibit;
  std::string _surchargeDesc;
  BookingCode _bookingCode;
  int _carrierFltTblItemNo;
  Indicator _sectorPortion;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _geoTblItemNoBtw);
    FLATTENIZE(archive, _geoTblItemNoAnd);
    FLATTENIZE(archive, _surchargeNoDec1);
    FLATTENIZE(archive, _surchargeAmt1);
    FLATTENIZE(archive, _surchargeNoDec2);
    FLATTENIZE(archive, _surchargeAmt2);
    FLATTENIZE(archive, _startTime);
    FLATTENIZE(archive, _stopTime);
    FLATTENIZE(archive, _surchargePercentNoDec);
    FLATTENIZE(archive, _surchargePercent);
    FLATTENIZE(archive, _startYear);
    FLATTENIZE(archive, _startMonth);
    FLATTENIZE(archive, _startDay);
    FLATTENIZE(archive, _stopYear);
    FLATTENIZE(archive, _stopMonth);
    FLATTENIZE(archive, _stopDay);
    FLATTENIZE(archive, _todAppl);
    FLATTENIZE(archive, _dow);
    FLATTENIZE(archive, _surchargeType);
    FLATTENIZE(archive, _equipType);
    FLATTENIZE(archive, _tvlPortion);
    FLATTENIZE(archive, _surchargeGroupInd);
    FLATTENIZE(archive, _surchargeAppl);
    FLATTENIZE(archive, _surchargeCur1);
    FLATTENIZE(archive, _surchargeCur2);
    FLATTENIZE(archive, _surchargePercentAppl);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _surchargeDesc);
    FLATTENIZE(archive, _bookingCode);
    FLATTENIZE(archive, _carrierFltTblItemNo);
    FLATTENIZE(archive, _sectorPortion);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailTag
           & ptr->_geoTblItemNo
           & ptr->_geoTblItemNoBtw
           & ptr->_geoTblItemNoAnd
           & ptr->_surchargeNoDec1
           & ptr->_surchargeAmt1
           & ptr->_surchargeNoDec2
           & ptr->_surchargeAmt2
           & ptr->_startTime
           & ptr->_stopTime
           & ptr->_surchargePercentNoDec
           & ptr->_surchargePercent
           & ptr->_startYear
           & ptr->_startMonth
           & ptr->_startDay
           & ptr->_stopYear
           & ptr->_stopMonth
           & ptr->_stopDay
           & ptr->_todAppl
           & ptr->_dow
           & ptr->_surchargeType
           & ptr->_equipType
           & ptr->_tvlPortion
           & ptr->_surchargeGroupInd
           & ptr->_surchargeAppl
           & ptr->_surchargeCur1
           & ptr->_surchargeCur2
           & ptr->_surchargePercentAppl
           & ptr->_inhibit
           & ptr->_surchargeDesc
           & ptr->_bookingCode
           & ptr->_carrierFltTblItemNo
           & ptr->_sectorPortion;
  }

};
}
