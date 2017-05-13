//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

#include <vector>

namespace tse
{

class Tours : public RuleItemInfo
{
public:
  static const size_t WAIVER_SZ = 14;
  Tours()
    : _itemNo(0),
      _minNightPercent(0),
      _adultFarePercent(0),
      _refund1Percent(0),
      _refund2Percent(0),
      _groupRefundPercent(0),
      _minNightAmt1(0),
      _addlNightAmt1(0),
      _minNightAmt2(0),
      _addlNightAmt2(0),
      _minTourPrice1(0),
      _minTourPrice2(0),
      _refund1Amt1(0),
      _refund1Amt2(0),
      _refund1GeoTblItemNo(0),
      _refund2Amt1(0),
      _refund2Amt2(0),
      _refund2GeoTblItemNo(0),
      _textTblItemNo(0),
      _minNights(0),
      _nightNoDec1(0),
      _nightNoDec2(0),
      _minNightPercentNoDec(0),
      _minTourNoDec1(0),
      _minTourNoDec2(0),
      _minAge(0),
      _maxAge(0),
      _adultFarePctNoDec(0),
      _refund1NoDec1(0),
      _refund1NoDec2(0),
      _refund1PercentNoDec(0),
      _refund1NoDays(0),
      _refund2NoDec1(0),
      _refund2NoDec2(0),
      _refund2PercentNoDec(0),
      _refund2NoDays(0),
      _groupRefPercentNoDec(0),
      _segCnt(0),
      _validityInd(' '),
      _inhibit(' '),
      _unavailTag(' '),
      _minStayInd(' '),
      _minNightAmtApplInd1(' '),
      _addlNightAmtApplInd1(' '),
      _minNightAmtApplInd2(' '),
      _addlNightAmtApplInd2(' '),
      _prepayInd(' '),
      _psgrTypeAppl(' '),
      _psgrWaiver(' '),
      _nonrefundableInd(' '),
      _refund1ApplInd(' '),
      _refund2ApplInd(' '),
      _tktdInd(' '),
      _refundMinGrpSizeInd(' '),
      _refundTvlInd(' '),
      _waiverApplInd(' ')
  {
    _waiver[0] = ' ';
    _waiver[1] = ' ';
    _waiver[2] = ' ';
    _waiver[3] = ' ';
    _waiver[4] = ' ';
    _waiver[5] = ' ';
    _waiver[6] = ' ';
    _waiver[7] = ' ';
    _waiver[8] = ' ';
    _waiver[9] = ' ';
    _waiver[10] = ' ';
    _waiver[11] = ' ';
    _waiver[12] = ' ';
    _waiver[13] = ' ';
  }

  class ToursSeg
  {
  public:
    ToursSeg()
      : _orderNo(0),
        _minGroundAmt1(0),
        _minGroundAmt2(0),
        _tvlGeoTblItemNoBetw(0),
        _tvlGeoTblItemNoAnd(0),
        _minGroundNoDec1(0),
        _minGroundNoDec2(0),
        _minNoTransfers(0),
        _minNoSkiLiftTkts(0),
        _minNoCarRentaldays(0),
        _minNoParkDays(0),
        _minNoResortDays(0),
        _minNoShipDays(0),
        _minNoOtherActivities(0),
        _maxNoFreeDays(0),
        _minGroundApplInd(' ')
    {
    }

    virtual ~ToursSeg() {}

    int& orderNo() { return _orderNo; }
    const int& orderNo() const { return _orderNo; }

    MoneyAmount& minGroundAmt1() { return _minGroundAmt1; }
    const MoneyAmount& minGroundAmt1() const { return _minGroundAmt1; }

    MoneyAmount& minGroundAmt2() { return _minGroundAmt2; }
    const MoneyAmount& minGroundAmt2() const { return _minGroundAmt2; }

    int& tvlGeoTblItemNoBetw() { return _tvlGeoTblItemNoBetw; }
    const int& tvlGeoTblItemNoBetw() const { return _tvlGeoTblItemNoBetw; }

    int& tvlGeoTblItemNoAnd() { return _tvlGeoTblItemNoAnd; }
    const int& tvlGeoTblItemNoAnd() const { return _tvlGeoTblItemNoAnd; }

    int& minGroundNoDec1() { return _minGroundNoDec1; }
    const int& minGroundNoDec1() const { return _minGroundNoDec1; }

    int& minGroundNoDec2() { return _minGroundNoDec2; }
    const int& minGroundNoDec2() const { return _minGroundNoDec2; }

    int& minNoTransfers() { return _minNoTransfers; }
    const int& minNoTransfers() const { return _minNoTransfers; }

    int& minNoSkiLiftTkts() { return _minNoSkiLiftTkts; }
    const int& minNoSkiLiftTkts() const { return _minNoSkiLiftTkts; }

    int& minNoCarRentaldays() { return _minNoCarRentaldays; }
    const int& minNoCarRentaldays() const { return _minNoCarRentaldays; }

    int& minNoParkDays() { return _minNoParkDays; }
    const int& minNoParkDays() const { return _minNoParkDays; }

    int& minNoResortDays() { return _minNoResortDays; }
    const int& minNoResortDays() const { return _minNoResortDays; }

    int& minNoShipDays() { return _minNoShipDays; }
    const int& minNoShipDays() const { return _minNoShipDays; }

    int& minNoOtherActivities() { return _minNoOtherActivities; }
    const int& minNoOtherActivities() const { return _minNoOtherActivities; }

    int& maxNoFreeDays() { return _maxNoFreeDays; }
    const int& maxNoFreeDays() const { return _maxNoFreeDays; }

    CurrencyCode& minGroundCur1() { return _minGroundCur1; }
    const CurrencyCode& minGroundCur1() const { return _minGroundCur1; }

    CurrencyCode& minGroundCur2() { return _minGroundCur2; }
    const CurrencyCode& minGroundCur2() const { return _minGroundCur2; }

    Indicator& minGroundApplInd() { return _minGroundApplInd; }
    const Indicator& minGroundApplInd() const { return _minGroundApplInd; }

    friend inline std::ostream& dumpObject(std::ostream& os, const ToursSeg& obj)
    {
      os << "[" << obj._orderNo << "|" << obj._minGroundAmt1 << "|" << obj._minGroundAmt2 << "|"
         << obj._tvlGeoTblItemNoBetw << "|" << obj._tvlGeoTblItemNoAnd << "|"
         << obj._minGroundNoDec1 << "|" << obj._minGroundNoDec2 << "|" << obj._minNoTransfers << "|"
         << obj._minNoSkiLiftTkts << "|" << obj._minNoCarRentaldays << "|" << obj._minNoParkDays
         << "|" << obj._minNoResortDays << "|" << obj._minNoShipDays << "|"
         << obj._minNoOtherActivities << "|" << obj._maxNoFreeDays << "|" << obj._minGroundCur1
         << "|" << obj._minGroundCur2 << "|" << obj._minGroundApplInd << "]";

      return os;
    }

    bool operator==(const ToursSeg& rhs) const
    {
      return (
          (_orderNo == rhs._orderNo) && (_minGroundAmt1 == rhs._minGroundAmt1) &&
          (_minGroundAmt2 == rhs._minGroundAmt2) &&
          (_tvlGeoTblItemNoBetw == rhs._tvlGeoTblItemNoBetw) &&
          (_tvlGeoTblItemNoAnd == rhs._tvlGeoTblItemNoAnd) &&
          (_minGroundNoDec1 == rhs._minGroundNoDec1) &&
          (_minGroundNoDec2 == rhs._minGroundNoDec2) && (_minNoTransfers == rhs._minNoTransfers) &&
          (_minNoSkiLiftTkts == rhs._minNoSkiLiftTkts) &&
          (_minNoCarRentaldays == rhs._minNoCarRentaldays) &&
          (_minNoParkDays == rhs._minNoParkDays) && (_minNoResortDays == rhs._minNoResortDays) &&
          (_minNoShipDays == rhs._minNoShipDays) &&
          (_minNoOtherActivities == rhs._minNoOtherActivities) &&
          (_maxNoFreeDays == rhs._maxNoFreeDays) && (_minGroundCur1 == rhs._minGroundCur1) &&
          (_minGroundCur2 == rhs._minGroundCur2) && (_minGroundApplInd == rhs._minGroundApplInd));
    }

    static void dummyData(ToursSeg& obj)
    {
      obj._orderNo = 1;
      obj._minGroundAmt1 = 2.22;
      obj._minGroundAmt2 = 3.33;
      obj._tvlGeoTblItemNoBetw = 4;
      obj._tvlGeoTblItemNoAnd = 5;
      obj._minGroundNoDec1 = 6;
      obj._minGroundNoDec2 = 7;
      obj._minNoTransfers = 8;
      obj._minNoSkiLiftTkts = 9;
      obj._minNoCarRentaldays = 10;
      obj._minNoParkDays = 11;
      obj._minNoResortDays = 12;
      obj._minNoShipDays = 13;
      obj._minNoOtherActivities = 14;
      obj._maxNoFreeDays = 15;
      obj._minGroundCur1 = "ABC";
      obj._minGroundCur2 = "DEF";
      obj._minGroundApplInd = 'G';
    }

  private:
    int _orderNo;
    MoneyAmount _minGroundAmt1;
    MoneyAmount _minGroundAmt2;
    int _tvlGeoTblItemNoBetw;
    int _tvlGeoTblItemNoAnd;
    int _minGroundNoDec1;
    int _minGroundNoDec2;
    int _minNoTransfers;
    int _minNoSkiLiftTkts;
    int _minNoCarRentaldays;
    int _minNoParkDays;
    int _minNoResortDays;
    int _minNoShipDays;
    int _minNoOtherActivities;
    int _maxNoFreeDays;
    CurrencyCode _minGroundCur1;
    CurrencyCode _minGroundCur2;
    Indicator _minGroundApplInd;

  public:
    virtual void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _orderNo);
      FLATTENIZE(archive, _minGroundAmt1);
      FLATTENIZE(archive, _minGroundAmt2);
      FLATTENIZE(archive, _tvlGeoTblItemNoBetw);
      FLATTENIZE(archive, _tvlGeoTblItemNoAnd);
      FLATTENIZE(archive, _minGroundNoDec1);
      FLATTENIZE(archive, _minGroundNoDec2);
      FLATTENIZE(archive, _minNoTransfers);
      FLATTENIZE(archive, _minNoSkiLiftTkts);
      FLATTENIZE(archive, _minNoCarRentaldays);
      FLATTENIZE(archive, _minNoParkDays);
      FLATTENIZE(archive, _minNoResortDays);
      FLATTENIZE(archive, _minNoShipDays);
      FLATTENIZE(archive, _minNoOtherActivities);
      FLATTENIZE(archive, _maxNoFreeDays);
      FLATTENIZE(archive, _minGroundCur1);
      FLATTENIZE(archive, _minGroundCur2);
      FLATTENIZE(archive, _minGroundApplInd);
    }
  };

  virtual ~Tours()
  {
    std::vector<ToursSeg*>::iterator i = _segs.begin();
    for (; i != _segs.end(); ++i)
    { // Nuke 'em!
      delete *i;
    }
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Percent& minNightPercent() { return _minNightPercent; }
  const Percent& minNightPercent() const { return _minNightPercent; }

  Percent& adultFarePercent() { return _adultFarePercent; }
  const Percent& adultFarePercent() const { return _adultFarePercent; }

  Percent& refund1Percent() { return _refund1Percent; }
  const Percent& refund1Percent() const { return _refund1Percent; }

  Percent& refund2Percent() { return _refund2Percent; }
  const Percent& refund2Percent() const { return _refund2Percent; }

  Percent& groupRefundPercent() { return _groupRefundPercent; }
  const Percent& groupRefundPercent() const { return _groupRefundPercent; }

  MoneyAmount& minNightAmt1() { return _minNightAmt1; }
  const MoneyAmount& minNightAmt1() const { return _minNightAmt1; }

  MoneyAmount& addlNightAmt1() { return _addlNightAmt1; }
  const MoneyAmount& addlNightAmt1() const { return _addlNightAmt1; }

  MoneyAmount& minNightAmt2() { return _minNightAmt2; }
  const MoneyAmount& minNightAmt2() const { return _minNightAmt2; }

  MoneyAmount& addlNightAmt2() { return _addlNightAmt2; }
  const MoneyAmount& addlNightAmt2() const { return _addlNightAmt2; }

  MoneyAmount& minTourPrice1() { return _minTourPrice1; }
  const MoneyAmount& minTourPrice1() const { return _minTourPrice1; }

  MoneyAmount& minTourPrice2() { return _minTourPrice2; }
  const MoneyAmount& minTourPrice2() const { return _minTourPrice2; }

  MoneyAmount& refund1Amt1() { return _refund1Amt1; }
  const MoneyAmount& refund1Amt1() const { return _refund1Amt1; }

  MoneyAmount& refund1Amt2() { return _refund1Amt2; }
  const MoneyAmount& refund1Amt2() const { return _refund1Amt2; }

  int& refund1GeoTblItemNo() { return _refund1GeoTblItemNo; }
  const int& refund1GeoTblItemNo() const { return _refund1GeoTblItemNo; }

  MoneyAmount& refund2Amt1() { return _refund2Amt1; }
  const MoneyAmount& refund2Amt1() const { return _refund2Amt1; }

  MoneyAmount& refund2Amt2() { return _refund2Amt2; }
  const MoneyAmount& refund2Amt2() const { return _refund2Amt2; }

  int& refund2GeoTblItemNo() { return _refund2GeoTblItemNo; }
  const int& refund2GeoTblItemNo() const { return _refund2GeoTblItemNo; }

  int& minNights() { return _minNights; }
  const int& minNights() const { return _minNights; }

  int& nightNoDec1() { return _nightNoDec1; }
  const int& nightNoDec1() const { return _nightNoDec1; }

  int& nightNoDec2() { return _nightNoDec2; }
  const int& nightNoDec2() const { return _nightNoDec2; }

  int& minNightPercentNoDec() { return _minNightPercentNoDec; }
  const int& minNightPercentNoDec() const { return _minNightPercentNoDec; }

  int& minTourNoDec1() { return _minTourNoDec1; }
  const int& minTourNoDec1() const { return _minTourNoDec1; }

  int& minTourNoDec2() { return _minTourNoDec2; }
  const int& minTourNoDec2() const { return _minTourNoDec2; }

  int& minAge() { return _minAge; }
  const int& minAge() const { return _minAge; }

  int& maxAge() { return _maxAge; }
  const int& maxAge() const { return _maxAge; }

  int& adultFarePctNoDec() { return _adultFarePctNoDec; }
  const int& adultFarePctNoDec() const { return _adultFarePctNoDec; }

  int& refund1NoDec1() { return _refund1NoDec1; }
  const int& refund1NoDec1() const { return _refund1NoDec1; }

  int& refund1NoDec2() { return _refund1NoDec2; }
  const int& refund1NoDec2() const { return _refund1NoDec2; }

  int& refund1PercentNoDec() { return _refund1PercentNoDec; }
  const int& refund1PercentNoDec() const { return _refund1PercentNoDec; }

  int& refund1NoDays() { return _refund1NoDays; }
  const int& refund1NoDays() const { return _refund1NoDays; }

  int& refund2NoDec1() { return _refund2NoDec1; }
  const int& refund2NoDec1() const { return _refund2NoDec1; }

  int& refund2NoDec2() { return _refund2NoDec2; }
  const int& refund2NoDec2() const { return _refund2NoDec2; }

  int& refund2PercentNoDec() { return _refund2PercentNoDec; }
  const int& refund2PercentNoDec() const { return _refund2PercentNoDec; }

  int& refund2NoDays() { return _refund2NoDays; }
  const int& refund2NoDays() const { return _refund2NoDays; }

  int& groupRefPercentNoDec() { return _groupRefPercentNoDec; }
  const int& groupRefPercentNoDec() const { return _groupRefPercentNoDec; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  std::string& tourType() { return _tourType; }
  const std::string& tourType() const { return _tourType; }

  std::string& tourNo() { return _tourNo; }
  const std::string& tourNo() const { return _tourNo; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& minStayInd() { return _minStayInd; }
  const Indicator& minStayInd() const { return _minStayInd; }

  Indicator& minNightAmtApplInd1() { return _minNightAmtApplInd1; }
  const Indicator& minNightAmtApplInd1() const { return _minNightAmtApplInd1; }

  Indicator& addlNightAmtApplInd1() { return _addlNightAmtApplInd1; }
  const Indicator& addlNightAmtApplInd1() const { return _addlNightAmtApplInd1; }

  CurrencyCode& nightCur1() { return _nightCur1; }
  const CurrencyCode& nightCur1() const { return _nightCur1; }

  Indicator& minNightAmtApplInd2() { return _minNightAmtApplInd2; }
  const Indicator& minNightAmtApplInd2() const { return _minNightAmtApplInd2; }

  Indicator& addlNightAmtApplInd2() { return _addlNightAmtApplInd2; }
  const Indicator& addlNightAmtApplInd2() const { return _addlNightAmtApplInd2; }

  CurrencyCode& nightCur2() { return _nightCur2; }
  const CurrencyCode& nightCur2() const { return _nightCur2; }

  CurrencyCode& minTourCur1() { return _minTourCur1; }
  const CurrencyCode& minTourCur1() const { return _minTourCur1; }

  CurrencyCode& minTourCur2() { return _minTourCur2; }
  const CurrencyCode& minTourCur2() const { return _minTourCur2; }

  Indicator& prepayInd() { return _prepayInd; }
  const Indicator& prepayInd() const { return _prepayInd; }

  Indicator& psgrTypeAppl() { return _psgrTypeAppl; }
  const Indicator& psgrTypeAppl() const { return _psgrTypeAppl; }

  PaxTypeCode& psgrType() { return _psgrType; }
  const PaxTypeCode& psgrType() const { return _psgrType; }

  Indicator& psgrWaiver() { return _psgrWaiver; }
  const Indicator& psgrWaiver() const { return _psgrWaiver; }

  Indicator& nonrefundableInd() { return _nonrefundableInd; }
  const Indicator& nonrefundableInd() const { return _nonrefundableInd; }

  CurrencyCode& refund1Cur1() { return _refund1Cur1; }
  const CurrencyCode& refund1Cur1() const { return _refund1Cur1; }

  CurrencyCode& refund1Cur2() { return _refund1Cur2; }
  const CurrencyCode& refund1Cur2() const { return _refund1Cur2; }

  Indicator& refund1ApplInd() { return _refund1ApplInd; }
  const Indicator& refund1ApplInd() const { return _refund1ApplInd; }

  CurrencyCode& refund2Cur1() { return _refund2Cur1; }
  const CurrencyCode& refund2Cur1() const { return _refund2Cur1; }

  CurrencyCode& refund2Cur2() { return _refund2Cur2; }
  const CurrencyCode& refund2Cur2() const { return _refund2Cur2; }

  Indicator& refund2ApplInd() { return _refund2ApplInd; }
  const Indicator& refund2ApplInd() const { return _refund2ApplInd; }

  Indicator& tktdInd() { return _tktdInd; }
  const Indicator& tktdInd() const { return _tktdInd; }

  Indicator& refundMinGrpSizeInd() { return _refundMinGrpSizeInd; }
  const Indicator& refundMinGrpSizeInd() const { return _refundMinGrpSizeInd; }

  Indicator& refundTvlInd() { return _refundTvlInd; }
  const Indicator& refundTvlInd() const { return _refundTvlInd; }

  Indicator& waiverApplInd() { return _waiverApplInd; }
  const Indicator& waiverApplInd() const { return _waiverApplInd; }

  Indicator* waiver() { return _waiver; }
  const Indicator* waiver() const { return _waiver; }

  std::vector<ToursSeg*>& segs() { return _segs; }
  const std::vector<ToursSeg*>& segs() const { return _segs; }

  friend inline std::ostream& dumpObject(std::ostream& os, const Tours& obj)
  {
    os << "[";

    dumpObject(os, dynamic_cast<const RuleItemInfo&>(obj));

    os << "|" << obj._vendor << "|" << obj._itemNo << "|" << obj._createDate << "|"
       << obj._expireDate << "|" << obj._minNightPercent << "|" << obj._adultFarePercent << "|"
       << obj._refund1Percent << "|" << obj._refund2Percent << "|" << obj._groupRefundPercent << "|"
       << obj._minNightAmt1 << "|" << obj._addlNightAmt1 << "|" << obj._minNightAmt2 << "|"
       << obj._addlNightAmt2 << "|" << obj._minTourPrice1 << "|" << obj._minTourPrice2 << "|"
       << obj._refund1Amt1 << "|" << obj._refund1Amt2 << "|" << obj._refund1GeoTblItemNo << "|"
       << obj._refund2Amt1 << "|" << obj._refund2Amt2 << "|" << obj._refund2GeoTblItemNo << "|"
       << obj._textTblItemNo << "|" << obj._minNights << "|" << obj._nightNoDec1 << "|"
       << obj._nightNoDec2 << "|" << obj._minNightPercentNoDec << "|" << obj._minTourNoDec1 << "|"
       << obj._minTourNoDec2 << "|" << obj._minAge << "|" << obj._maxAge << "|"
       << obj._adultFarePctNoDec << "|" << obj._refund1NoDec1 << "|" << obj._refund1NoDec2 << "|"
       << obj._refund1PercentNoDec << "|" << obj._refund1NoDays << "|" << obj._refund2NoDec1 << "|"
       << obj._refund2NoDec2 << "|" << obj._refund2PercentNoDec << "|" << obj._refund2NoDays << "|"
       << obj._groupRefPercentNoDec << "|" << obj._segCnt << "|" << obj._validityInd << "|"
       << obj._inhibit << "|" << obj._unavailTag << "|" << obj._tourType << "|" << obj._tourNo
       << "|" << obj._carrier << "|" << obj._minStayInd << "|" << obj._minNightAmtApplInd1 << "|"
       << obj._addlNightAmtApplInd1 << "|" << obj._nightCur1 << "|" << obj._minNightAmtApplInd2
       << "|" << obj._addlNightAmtApplInd2 << "|" << obj._nightCur2 << "|" << obj._minTourCur1
       << "|" << obj._minTourCur2 << "|" << obj._prepayInd << "|" << obj._psgrTypeAppl << "|"
       << obj._psgrType << "|" << obj._psgrWaiver << "|" << obj._nonrefundableInd << "|"
       << obj._refund1Cur1 << "|" << obj._refund1Cur2 << "|" << obj._refund1ApplInd << "|"
       << obj._refund2Cur1 << "|" << obj._refund2Cur2 << "|" << obj._refund2ApplInd << "|"
       << obj._tktdInd << "|" << obj._refundMinGrpSizeInd << "|" << obj._refundTvlInd << "|"
       << obj._waiverApplInd;

    os << "[";
    for (size_t idx = 0; idx < WAIVER_SZ; ++idx)
    {
      if (idx > 0)
      {
        os << ",";
      }
      os << obj._waiver[idx];
    }
    os << "]";

    os << "[";
    for (size_t i = 0; i < obj._segs.size(); ++i)
    {
      if (i > 0)
      {
        os << ",";
      }
      dumpObject(os, *(obj._segs[i]));
    }
    os << "]";

    os << "]";

    return os;
  }

  virtual bool operator==(const Tours& rhs) const
  {
    bool eq(
        (RuleItemInfo::operator==(rhs)) && (_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_minNightPercent == rhs._minNightPercent) &&
        (_adultFarePercent == rhs._adultFarePercent) && (_refund1Percent == rhs._refund1Percent) &&
        (_refund2Percent == rhs._refund2Percent) &&
        (_groupRefundPercent == rhs._groupRefundPercent) && (_minNightAmt1 == rhs._minNightAmt1) &&
        (_addlNightAmt1 == rhs._addlNightAmt1) && (_minNightAmt2 == rhs._minNightAmt2) &&
        (_addlNightAmt2 == rhs._addlNightAmt2) && (_minTourPrice1 == rhs._minTourPrice1) &&
        (_minTourPrice2 == rhs._minTourPrice2) && (_refund1Amt1 == rhs._refund1Amt1) &&
        (_refund1Amt2 == rhs._refund1Amt2) && (_refund1GeoTblItemNo == rhs._refund1GeoTblItemNo) &&
        (_refund2Amt1 == rhs._refund2Amt1) && (_refund2Amt2 == rhs._refund2Amt2) &&
        (_refund2GeoTblItemNo == rhs._refund2GeoTblItemNo) &&
        (_textTblItemNo == rhs._textTblItemNo) && (_minNights == rhs._minNights) &&
        (_nightNoDec1 == rhs._nightNoDec1) && (_nightNoDec2 == rhs._nightNoDec2) &&
        (_minNightPercentNoDec == rhs._minNightPercentNoDec) &&
        (_minTourNoDec1 == rhs._minTourNoDec1) && (_minTourNoDec2 == rhs._minTourNoDec2) &&
        (_minAge == rhs._minAge) && (_maxAge == rhs._maxAge) &&
        (_adultFarePctNoDec == rhs._adultFarePctNoDec) && (_refund1NoDec1 == rhs._refund1NoDec1) &&
        (_refund1NoDec2 == rhs._refund1NoDec2) &&
        (_refund1PercentNoDec == rhs._refund1PercentNoDec) &&
        (_refund1NoDays == rhs._refund1NoDays) && (_refund2NoDec1 == rhs._refund2NoDec1) &&
        (_refund2NoDec2 == rhs._refund2NoDec2) &&
        (_refund2PercentNoDec == rhs._refund2PercentNoDec) &&
        (_refund2NoDays == rhs._refund2NoDays) &&
        (_groupRefPercentNoDec == rhs._groupRefPercentNoDec) && (_segCnt == rhs._segCnt) &&
        (_validityInd == rhs._validityInd) && (_inhibit == rhs._inhibit) &&
        (_unavailTag == rhs._unavailTag) && (_tourType == rhs._tourType) &&
        (_tourNo == rhs._tourNo) && (_carrier == rhs._carrier) &&
        (_minStayInd == rhs._minStayInd) && (_minNightAmtApplInd1 == rhs._minNightAmtApplInd1) &&
        (_addlNightAmtApplInd1 == rhs._addlNightAmtApplInd1) && (_nightCur1 == rhs._nightCur1) &&
        (_minNightAmtApplInd2 == rhs._minNightAmtApplInd2) &&
        (_addlNightAmtApplInd2 == rhs._addlNightAmtApplInd2) && (_nightCur2 == rhs._nightCur2) &&
        (_minTourCur1 == rhs._minTourCur1) && (_minTourCur2 == rhs._minTourCur2) &&
        (_prepayInd == rhs._prepayInd) && (_psgrTypeAppl == rhs._psgrTypeAppl) &&
        (_psgrType == rhs._psgrType) && (_psgrWaiver == rhs._psgrWaiver) &&
        (_nonrefundableInd == rhs._nonrefundableInd) && (_refund1Cur1 == rhs._refund1Cur1) &&
        (_refund1Cur2 == rhs._refund1Cur2) && (_refund1ApplInd == rhs._refund1ApplInd) &&
        (_refund2Cur1 == rhs._refund2Cur1) && (_refund2Cur2 == rhs._refund2Cur2) &&
        (_refund2ApplInd == rhs._refund2ApplInd) && (_tktdInd == rhs._tktdInd) &&
        (_refundMinGrpSizeInd == rhs._refundMinGrpSizeInd) &&
        (_refundTvlInd == rhs._refundTvlInd) && (_waiverApplInd == rhs._waiverApplInd) &&
        (_segs.size() == rhs._segs.size()));

    for (size_t idx = 0; (eq && (idx < WAIVER_SZ)); ++idx)
    {
      eq = (_waiver[idx] == rhs._waiver[idx]);
    }

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(Tours& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._minNightPercent = 2.222;
    obj._adultFarePercent = 3.333;
    obj._refund1Percent = 4.444;
    obj._refund2Percent = 5.555;
    obj._groupRefundPercent = 6.666;
    obj._minNightAmt1 = 7.77;
    obj._addlNightAmt1 = 8.88;
    obj._minNightAmt2 = 9.99;
    obj._addlNightAmt2 = 10.10;
    obj._minTourPrice1 = 11.11;
    obj._minTourPrice2 = 12.12;
    obj._refund1Amt1 = 13.13;
    obj._refund1Amt2 = 14.14;
    obj._refund1GeoTblItemNo = 15;
    obj._refund2Amt1 = 16.16;
    obj._refund2Amt2 = 17.17;
    obj._refund2GeoTblItemNo = 18;
    obj._textTblItemNo = 19;
    obj._minNights = 20;
    obj._nightNoDec1 = 21;
    obj._nightNoDec2 = 22;
    obj._minNightPercentNoDec = 23;
    obj._minTourNoDec1 = 24;
    obj._minTourNoDec2 = 25;
    obj._minAge = 26;
    obj._maxAge = 27;
    obj._adultFarePctNoDec = 28;
    obj._refund1NoDec1 = 29;
    obj._refund1NoDec2 = 30;
    obj._refund1PercentNoDec = 31;
    obj._refund1NoDays = 32;
    obj._refund2NoDec1 = 33;
    obj._refund2NoDec2 = 34;
    obj._refund2PercentNoDec = 35;
    obj._refund2NoDays = 36;
    obj._groupRefPercentNoDec = 37;
    obj._segCnt = 38;
    obj._validityInd = 'E';
    obj._inhibit = 'F';
    obj._unavailTag = 'G';
    obj._tourType = "aaaaaaaa";
    obj._tourNo = "bbbbbbbb";
    obj._carrier = "HIJ";
    obj._minStayInd = 'K';
    obj._minNightAmtApplInd1 = 'L';
    obj._addlNightAmtApplInd1 = 'M';
    obj._nightCur1 = "NOP";
    obj._minNightAmtApplInd2 = 'Q';
    obj._addlNightAmtApplInd2 = 'R';
    obj._nightCur2 = "STU";
    obj._minTourCur1 = "VWX";
    obj._minTourCur2 = "YZa";
    obj._prepayInd = 'b';
    obj._psgrTypeAppl = 'c';
    obj._psgrType = "def";
    obj._psgrWaiver = 'g';
    obj._nonrefundableInd = 'h';
    obj._refund1Cur1 = "ijk";
    obj._refund1Cur2 = "lmn";
    obj._refund1ApplInd = 'o';
    obj._refund2Cur1 = "pqr";
    obj._refund2Cur2 = "stu";
    obj._refund2ApplInd = 'v';
    obj._tktdInd = 'w';
    obj._refundMinGrpSizeInd = 'x';
    obj._refundTvlInd = 'y';
    obj._waiverApplInd = 'z';

    Indicator value('1');
    for (size_t idx = 0; idx < WAIVER_SZ; ++idx, ++value)
    {
      obj._waiver[idx] = value;
    }

    ToursSeg* ts1 = new ToursSeg;
    ToursSeg* ts2 = new ToursSeg;

    ToursSeg::dummyData(*ts1);
    ToursSeg::dummyData(*ts2);

    obj._segs.push_back(ts1);
    obj._segs.push_back(ts2);
  }

private:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  Percent _minNightPercent;
  Percent _adultFarePercent;
  Percent _refund1Percent;
  Percent _refund2Percent;
  Percent _groupRefundPercent;
  MoneyAmount _minNightAmt1;
  MoneyAmount _addlNightAmt1;
  MoneyAmount _minNightAmt2;
  MoneyAmount _addlNightAmt2;
  MoneyAmount _minTourPrice1;
  MoneyAmount _minTourPrice2;
  MoneyAmount _refund1Amt1;
  MoneyAmount _refund1Amt2;
  int _refund1GeoTblItemNo;
  MoneyAmount _refund2Amt1;
  MoneyAmount _refund2Amt2;
  int _refund2GeoTblItemNo;
  int _textTblItemNo;
  int _minNights;
  int _nightNoDec1;
  int _nightNoDec2;
  int _minNightPercentNoDec;
  int _minTourNoDec1;
  int _minTourNoDec2;
  int _minAge;
  int _maxAge;
  int _adultFarePctNoDec;
  int _refund1NoDec1;
  int _refund1NoDec2;
  int _refund1PercentNoDec;
  int _refund1NoDays;
  int _refund2NoDec1;
  int _refund2NoDec2;
  int _refund2PercentNoDec;
  int _refund2NoDays;
  int _groupRefPercentNoDec;
  int _segCnt;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailTag;
  std::string _tourType;
  std::string _tourNo;
  CarrierCode _carrier;
  Indicator _minStayInd;
  Indicator _minNightAmtApplInd1;
  Indicator _addlNightAmtApplInd1;
  CurrencyCode _nightCur1;
  Indicator _minNightAmtApplInd2;
  Indicator _addlNightAmtApplInd2;
  CurrencyCode _nightCur2;
  CurrencyCode _minTourCur1;
  CurrencyCode _minTourCur2;
  Indicator _prepayInd;
  Indicator _psgrTypeAppl;
  PaxTypeCode _psgrType;
  Indicator _psgrWaiver;
  Indicator _nonrefundableInd;
  CurrencyCode _refund1Cur1;
  CurrencyCode _refund1Cur2;
  Indicator _refund1ApplInd;
  CurrencyCode _refund2Cur1;
  CurrencyCode _refund2Cur2;
  Indicator _refund2ApplInd;
  Indicator _tktdInd;
  Indicator _refundMinGrpSizeInd;
  Indicator _refundTvlInd;
  Indicator _waiverApplInd;
  Indicator _waiver[WAIVER_SZ];
  std::vector<ToursSeg*> _segs;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _minNightPercent);
    FLATTENIZE(archive, _adultFarePercent);
    FLATTENIZE(archive, _refund1Percent);
    FLATTENIZE(archive, _refund2Percent);
    FLATTENIZE(archive, _groupRefundPercent);
    FLATTENIZE(archive, _minNightAmt1);
    FLATTENIZE(archive, _addlNightAmt1);
    FLATTENIZE(archive, _minNightAmt2);
    FLATTENIZE(archive, _addlNightAmt2);
    FLATTENIZE(archive, _minTourPrice1);
    FLATTENIZE(archive, _minTourPrice2);
    FLATTENIZE(archive, _refund1Amt1);
    FLATTENIZE(archive, _refund1Amt2);
    FLATTENIZE(archive, _refund1GeoTblItemNo);
    FLATTENIZE(archive, _refund2Amt1);
    FLATTENIZE(archive, _refund2Amt2);
    FLATTENIZE(archive, _refund2GeoTblItemNo);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _minNights);
    FLATTENIZE(archive, _nightNoDec1);
    FLATTENIZE(archive, _nightNoDec2);
    FLATTENIZE(archive, _minNightPercentNoDec);
    FLATTENIZE(archive, _minTourNoDec1);
    FLATTENIZE(archive, _minTourNoDec2);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _adultFarePctNoDec);
    FLATTENIZE(archive, _refund1NoDec1);
    FLATTENIZE(archive, _refund1NoDec2);
    FLATTENIZE(archive, _refund1PercentNoDec);
    FLATTENIZE(archive, _refund1NoDays);
    FLATTENIZE(archive, _refund2NoDec1);
    FLATTENIZE(archive, _refund2NoDec2);
    FLATTENIZE(archive, _refund2PercentNoDec);
    FLATTENIZE(archive, _refund2NoDays);
    FLATTENIZE(archive, _groupRefPercentNoDec);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _tourType);
    FLATTENIZE(archive, _tourNo);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _minStayInd);
    FLATTENIZE(archive, _minNightAmtApplInd1);
    FLATTENIZE(archive, _addlNightAmtApplInd1);
    FLATTENIZE(archive, _nightCur1);
    FLATTENIZE(archive, _minNightAmtApplInd2);
    FLATTENIZE(archive, _addlNightAmtApplInd2);
    FLATTENIZE(archive, _nightCur2);
    FLATTENIZE(archive, _minTourCur1);
    FLATTENIZE(archive, _minTourCur2);
    FLATTENIZE(archive, _prepayInd);
    FLATTENIZE(archive, _psgrTypeAppl);
    FLATTENIZE(archive, _psgrType);
    FLATTENIZE(archive, _psgrWaiver);
    FLATTENIZE(archive, _nonrefundableInd);
    FLATTENIZE(archive, _refund1Cur1);
    FLATTENIZE(archive, _refund1Cur2);
    FLATTENIZE(archive, _refund1ApplInd);
    FLATTENIZE(archive, _refund2Cur1);
    FLATTENIZE(archive, _refund2Cur2);
    FLATTENIZE(archive, _refund2ApplInd);
    FLATTENIZE(archive, _tktdInd);
    FLATTENIZE(archive, _refundMinGrpSizeInd);
    FLATTENIZE(archive, _refundTvlInd);
    FLATTENIZE(archive, _waiverApplInd);
    FLATTENIZE(archive, _waiver);
    FLATTENIZE(archive, _segs);
  }
};
}

