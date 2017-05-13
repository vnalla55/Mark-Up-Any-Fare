// ---------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/SmallBitSet.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

#include <stdlib.h>

namespace tse
{

class ReissueSequence
{

public:
  ReissueSequence()
    : _itemNo(0),
      _seqNo(0),
      _inhibit(' '),
      _validityInd(' '),
      _tvlGeoTblItemNoFrom(0),
      _tvlGeoTblItemNoTo(0),
      _samePointTblItemNo(0),
      _minGeoTblItemNoFrom(0),
      _minGeoTblItemNoTo(0),
      _fareCxrApplTblItemNo(0),
      _risRestCxrTblItemNo(0),
      _surchargeAmt1(0),
      _surchargeAmt2(0),
      _processingInd(0),
      _ruleTariffNo(0),
      _reissueTOD(0),
      _departure(0),
      _minStayTOD(0),
      _surchargeNoDec1(0),
      _surchargeNoDec2(0),
      _stopoverConnectInd(' '),
      _portionInd(' '),
      _terminalPointInd(' '),
      _firstBreakInd(' '),
      _couponInd(' '),
      _extendInd(' '),
      _journeyInd(' '),
      _unusedFlightInd(' '),
      _ruleInd(' '),
      _fareTypeInd(' '),
      _sameInd(' '),
      _fareAmtInd(' '),
      _normalspecialInd(' '),
      _owrt(' '),
      _dateInd(' '),
      _fromAdvResInd(' '),
      _toAdvResInd(' '),
      _revalidationInd(' '),
      _provision1(' '),
      _provision2(' '),
      _provision3(' '),
      _provision4(' '),
      _provision5(' '),
      _provision6(' '),
      _provision7(' '),
      _provision8(' '),
      _provision9(' '),
      _provision10(' '),
      _provision11(' '),
      _provision12(' '),
      _provision13(' '),
      _provision14(' '),
      _provision15(' '),
      _provision17(' '),
      _provision18(' '),
      _provision50(' '),
      _bkgCdRevalInd(' '),
      _ticketResvInd(' '),
      _optionInd(' '),
      _departureUnit(' '),
      _departureInd(' '),
      _minStayUnit(' '),
      _autoTktInd(' '),
      _electronicTktInd(' '),
      _carrierRestInd(' '),
      _agencyLocRest(' '),
      _surchargeType(' '),
      _surchargeAppl(' '),
      _stopInd(' '),
      _origSchedFltUnit(' '),
      _excludePrivate(' '),
      _flightNoInd(' '),
      _historicalTktTvlInd(' '),
      _outboundInd(' '),
      _reissueToLower(' '),
      _ticketEqualOrHigher(' '),
      _fareTypeTblItemNo(0),
      _seasonalityDOWTblItemNo(0),
      _expndKeep(' ')
  {
  }
  virtual ~ReissueSequence() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }
  int& itemNo() { return _itemNo; }
  int itemNo() const { return _itemNo; }
  int& seqNo() { return _seqNo; }
  int seqNo() const { return _seqNo; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }
  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }
  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }
  int& tvlGeoTblItemNoFrom() { return _tvlGeoTblItemNoFrom; }
  int tvlGeoTblItemNoFrom() const { return _tvlGeoTblItemNoFrom; }
  int& tvlGeoTblItemNoTo() { return _tvlGeoTblItemNoTo; }
  int tvlGeoTblItemNoTo() const { return _tvlGeoTblItemNoTo; }
  int& samePointTblItemNo() { return _samePointTblItemNo; }
  int samePointTblItemNo() const { return _samePointTblItemNo; }
  int& minGeoTblItemNoFrom() { return _minGeoTblItemNoFrom; }
  int minGeoTblItemNoFrom() const { return _minGeoTblItemNoFrom; }
  int& minGeoTblItemNoTo() { return _minGeoTblItemNoTo; }
  int minGeoTblItemNoTo() const { return _minGeoTblItemNoTo; }
  int& fareCxrApplTblItemNo() { return _fareCxrApplTblItemNo; }
  int fareCxrApplTblItemNo() const { return _fareCxrApplTblItemNo; }
  int& risRestCxrTblItemNo() { return _risRestCxrTblItemNo; }
  int risRestCxrTblItemNo() const { return _risRestCxrTblItemNo; }
  MoneyAmount& surchargeAmt1() { return _surchargeAmt1; }
  const MoneyAmount& surchargeAmt1() const { return _surchargeAmt1; }
  MoneyAmount& surchargeAmt2() { return _surchargeAmt2; }
  const MoneyAmount& surchargeAmt2() const { return _surchargeAmt2; }
  int& processingInd() { return _processingInd; }
  int processingInd() const { return _processingInd; }
  TariffNumber& ruleTariffNo() { return _ruleTariffNo; }
  const TariffNumber& ruleTariffNo() const { return _ruleTariffNo; }
  int& reissueTOD() { return _reissueTOD; }
  int reissueTOD() const { return _reissueTOD; }
  int& departure() { return _departure; }
  int departure() const { return _departure; }
  int& minStayTOD() { return _minStayTOD; }
  int minStayTOD() const { return _minStayTOD; }
  int& surchargeNoDec1() { return _surchargeNoDec1; }
  int surchargeNoDec1() const { return _surchargeNoDec1; }
  int& surchargeNoDec2() { return _surchargeNoDec2; }
  int surchargeNoDec2() const { return _surchargeNoDec2; }
  std::string& creatorId() { return _creatorId; }
  const std::string& creatorId() const { return _creatorId; }
  std::string& creatorBusinessUnit() { return _creatorBusinessUnit; }
  const std::string& creatorBusinessUnit() const { return _creatorBusinessUnit; }
  Indicator& stopoverConnectInd() { return _stopoverConnectInd; }
  Indicator stopoverConnectInd() const { return _stopoverConnectInd; }
  Indicator& portionInd() { return _portionInd; }
  Indicator portionInd() const { return _portionInd; }
  Indicator& terminalPointInd() { return _terminalPointInd; }
  Indicator terminalPointInd() const { return _terminalPointInd; }
  Indicator& firstBreakInd() { return _firstBreakInd; }
  Indicator firstBreakInd() const { return _firstBreakInd; }
  Indicator& couponInd() { return _couponInd; }
  Indicator couponInd() const { return _couponInd; }
  Indicator& extendInd() { return _extendInd; }
  Indicator extendInd() const { return _extendInd; }
  Indicator& journeyInd() { return _journeyInd; }
  Indicator journeyInd() const { return _journeyInd; }
  Indicator& unusedFlightInd() { return _unusedFlightInd; }
  Indicator unusedFlightInd() const { return _unusedFlightInd; }
  Indicator& ruleInd() { return _ruleInd; }
  Indicator ruleInd() const { return _ruleInd; }
  RuleNumber& ruleNo() { return _ruleNo; }
  const RuleNumber& ruleNo() const { return _ruleNo; }
  Indicator& fareTypeInd() { return _fareTypeInd; }
  Indicator fareTypeInd() const { return _fareTypeInd; }
  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }
  FareTypeAbbrev& fareType() { return _fareType; }
  const FareTypeAbbrev& fareType() const { return _fareType; }
  Indicator& sameInd() { return _sameInd; }
  Indicator sameInd() const { return _sameInd; }
  Indicator& fareAmtInd() { return _fareAmtInd; }
  Indicator fareAmtInd() const { return _fareAmtInd; }
  Indicator& normalspecialInd() { return _normalspecialInd; }
  Indicator normalspecialInd() const { return _normalspecialInd; }
  Indicator& owrt() { return _owrt; }
  Indicator owrt() const { return _owrt; }
  Indicator& dateInd() { return _dateInd; }
  Indicator dateInd() const { return _dateInd; }
  Indicator& fromAdvResInd() { return _fromAdvResInd; }
  Indicator fromAdvResInd() const { return _fromAdvResInd; }
  Indicator& toAdvResInd() { return _toAdvResInd; }
  Indicator toAdvResInd() const { return _toAdvResInd; }
  Indicator& revalidationInd() { return _revalidationInd; }
  Indicator revalidationInd() const { return _revalidationInd; }
  Indicator& provision1() { return _provision1; }
  Indicator provision1() const { return _provision1; }
  Indicator& provision2() { return _provision2; }
  Indicator provision2() const { return _provision2; }
  Indicator& provision3() { return _provision3; }
  Indicator provision3() const { return _provision3; }
  Indicator& provision4() { return _provision4; }
  Indicator provision4() const { return _provision4; }
  Indicator& provision5() { return _provision5; }
  Indicator provision5() const { return _provision5; }
  Indicator& provision6() { return _provision6; }
  Indicator provision6() const { return _provision6; }
  Indicator& provision7() { return _provision7; }
  Indicator provision7() const { return _provision7; }
  Indicator& provision8() { return _provision8; }
  Indicator provision8() const { return _provision8; }
  Indicator& provision9() { return _provision9; }
  Indicator provision9() const { return _provision9; }
  Indicator& provision10() { return _provision10; }
  Indicator provision10() const { return _provision10; }
  Indicator& provision11() { return _provision11; }
  Indicator provision11() const { return _provision11; }
  Indicator& provision12() { return _provision12; }
  Indicator provision12() const { return _provision12; }
  Indicator& provision13() { return _provision13; }
  Indicator provision13() const { return _provision13; }
  Indicator& provision14() { return _provision14; }
  Indicator provision14() const { return _provision14; }
  Indicator& provision15() { return _provision15; }
  Indicator provision15() const { return _provision15; }
  Indicator& provision17() { return _provision17; }
  Indicator provision17() const { return _provision17; }
  Indicator& provision18() { return _provision18; }
  Indicator provision18() const { return _provision18; }
  Indicator& provision50() { return _provision50; }
  Indicator provision50() const { return _provision50; }
  Indicator& bkgCdRevalInd() { return _bkgCdRevalInd; }
  Indicator bkgCdRevalInd() const { return _bkgCdRevalInd; }
  Indicator& ticketResvInd() { return _ticketResvInd; }
  Indicator ticketResvInd() const { return _ticketResvInd; }
  std::string& reissuePeriod() { return _reissuePeriod; }
  const std::string& reissuePeriod() const { return _reissuePeriod; }
  std::string& reissueUnit() { return _reissueUnit; }
  const std::string& reissueUnit() const { return _reissueUnit; }

  Indicator& optionInd() { return _optionInd; }
  Indicator optionInd() const { return _optionInd; }
  Indicator& departureUnit() { return _departureUnit; }
  Indicator departureUnit() const { return _departureUnit; }
  Indicator& departureInd() { return _departureInd; }
  Indicator departureInd() const { return _departureInd; }

  std::string& minStayPeriod() { return _minStayPeriod; }
  const std::string& minStayPeriod() const { return _minStayPeriod; }
  Indicator& minStayUnit() { return _minStayUnit; }
  Indicator minStayUnit() const { return _minStayUnit; }
  Indicator& autoTktInd() { return _autoTktInd; }
  Indicator autoTktInd() const { return _autoTktInd; }
  Indicator& electronicTktInd() { return _electronicTktInd; }
  Indicator electronicTktInd() const { return _electronicTktInd; }
  Indicator& carrierRestInd() { return _carrierRestInd; }
  Indicator carrierRestInd() const { return _carrierRestInd; }
  Indicator& agencyLocRest() { return _agencyLocRest; }
  Indicator agencyLocRest() const { return _agencyLocRest; }
  AgencyIATA& iataAgencyNo() { return _iataAgencyNo; }
  const AgencyIATA& iataAgencyNo() const { return _iataAgencyNo; }
  Indicator& surchargeType() { return _surchargeType; }
  Indicator surchargeType() const { return _surchargeType; }
  Indicator& surchargeAppl() { return _surchargeAppl; }
  Indicator surchargeAppl() const { return _surchargeAppl; }
  CurrencyCode& surchargeCur1() { return _surchargeCur1; }
  const CurrencyCode& surchargeCur1() const { return _surchargeCur1; }
  CurrencyCode& surchargeCur2() { return _surchargeCur2; }
  const CurrencyCode& surchargeCur2() const { return _surchargeCur2; }
  Indicator& stopInd() { return _stopInd; }
  Indicator stopInd() const { return _stopInd; }
  std::string& origSchedFltPeriod() { return _origSchedFltPeriod; }
  const std::string& origSchedFltPeriod() const { return _origSchedFltPeriod; }
  Indicator& origSchedFltUnit() { return _origSchedFltUnit; }
  Indicator origSchedFltUnit() const { return _origSchedFltUnit; }
  Indicator& excludePrivate() { return _excludePrivate; }
  Indicator excludePrivate() const { return _excludePrivate; }
  Indicator& flightNoInd() { return _flightNoInd; }
  Indicator flightNoInd() const { return _flightNoInd; }
  Indicator& historicalTktTvlInd() { return _historicalTktTvlInd; }
  Indicator historicalTktTvlInd() const { return _historicalTktTvlInd; }
  Indicator& outboundInd() { return _outboundInd; }
  Indicator outboundInd() const { return _outboundInd; }
  Indicator& reissueToLower() { return _reissueToLower; }
  Indicator reissueToLower() const { return _reissueToLower; }
  Indicator& ticketEqualOrHigher() { return _ticketEqualOrHigher; }
  Indicator ticketEqualOrHigher() const { return _ticketEqualOrHigher; }
  uint32_t& fareTypeTblItemNo() { return _fareTypeTblItemNo; }
  uint32_t fareTypeTblItemNo() const { return _fareTypeTblItemNo; }
  uint32_t& seasonalityDOWTblItemNo() { return _seasonalityDOWTblItemNo; }
  uint32_t seasonalityDOWTblItemNo() const { return _seasonalityDOWTblItemNo; }
  Indicator& expndKeep() { return _expndKeep; }
  Indicator expndKeep() const { return _expndKeep; }

  virtual bool operator==(const ReissueSequence& rhs) const
  {
    return (
        (_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_inhibit == rhs._inhibit) && (_validityInd == rhs._validityInd) &&
        (_tvlGeoTblItemNoFrom == rhs._tvlGeoTblItemNoFrom) &&
        (_tvlGeoTblItemNoTo == rhs._tvlGeoTblItemNoTo) &&
        (_samePointTblItemNo == rhs._samePointTblItemNo) &&
        (_minGeoTblItemNoFrom == rhs._minGeoTblItemNoFrom) &&
        (_minGeoTblItemNoTo == rhs._minGeoTblItemNoTo) &&
        (_fareCxrApplTblItemNo == rhs._fareCxrApplTblItemNo) &&
        (_risRestCxrTblItemNo == rhs._risRestCxrTblItemNo) &&
        (_surchargeAmt1 == rhs._surchargeAmt1) && (_surchargeAmt2 == rhs._surchargeAmt2) &&
        (_processingInd == rhs._processingInd) && (_ruleTariffNo == rhs._ruleTariffNo) &&
        (_reissueTOD == rhs._reissueTOD) && (_departure == rhs._departure) &&
        (_minStayTOD == rhs._minStayTOD) && (_surchargeNoDec1 == rhs._surchargeNoDec1) &&
        (_surchargeNoDec2 == rhs._surchargeNoDec2) && (_creatorId == rhs._creatorId) &&
        (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
        (_stopoverConnectInd == rhs._stopoverConnectInd) && (_portionInd == rhs._portionInd) &&
        (_terminalPointInd == rhs._terminalPointInd) && (_firstBreakInd == rhs._firstBreakInd) &&
        (_couponInd == rhs._couponInd) && (_extendInd == rhs._extendInd) &&
        (_journeyInd == rhs._journeyInd) && (_unusedFlightInd == rhs._unusedFlightInd) &&
        (_ruleInd == rhs._ruleInd) && (_ruleNo == rhs._ruleNo) &&
        (_fareTypeInd == rhs._fareTypeInd) && (_fareClass == rhs._fareClass) &&
        (_fareType == rhs._fareType) && (_sameInd == rhs._sameInd) &&
        (_fareAmtInd == rhs._fareAmtInd) && (_normalspecialInd == rhs._normalspecialInd) &&
        (_owrt == rhs._owrt) && (_dateInd == rhs._dateInd) &&
        (_fromAdvResInd == rhs._fromAdvResInd) && (_toAdvResInd == rhs._toAdvResInd) &&
        (_revalidationInd == rhs._revalidationInd) && (_provision1 == rhs._provision1) &&
        (_provision2 == rhs._provision2) && (_provision3 == rhs._provision3) &&
        (_provision4 == rhs._provision4) && (_provision5 == rhs._provision5) &&
        (_provision6 == rhs._provision6) && (_provision7 == rhs._provision7) &&
        (_provision8 == rhs._provision8) && (_provision9 == rhs._provision9) &&
        (_provision10 == rhs._provision10) && (_provision11 == rhs._provision11) &&
        (_provision12 == rhs._provision12) && (_provision13 == rhs._provision13) &&
        (_provision14 == rhs._provision14) && (_provision15 == rhs._provision15) &&
        (_provision17 == rhs._provision17) && (_provision18 == rhs._provision18) &&
        (_provision50 == rhs._provision50) && (_bkgCdRevalInd == rhs._bkgCdRevalInd) &&
        (_ticketResvInd == rhs._ticketResvInd) && (_reissuePeriod == rhs._reissuePeriod) &&
        (_reissueUnit == rhs._reissueUnit) && (_optionInd == rhs._optionInd) &&
        (_departureUnit == rhs._departureUnit) && (_departureInd == rhs._departureInd) &&
        (_minStayPeriod == rhs._minStayPeriod) && (_minStayUnit == rhs._minStayUnit) &&
        (_autoTktInd == rhs._autoTktInd) && (_electronicTktInd == rhs._electronicTktInd) &&
        (_carrierRestInd == rhs._carrierRestInd) && (_agencyLocRest == rhs._agencyLocRest) &&
        (_iataAgencyNo == rhs._iataAgencyNo) && (_surchargeType == rhs._surchargeType) &&
        (_surchargeAppl == rhs._surchargeAppl) && (_surchargeCur1 == rhs._surchargeCur1) &&
        (_surchargeCur2 == rhs._surchargeCur2) && (_stopInd == rhs._stopInd) &&
        (_origSchedFltPeriod == rhs._origSchedFltPeriod) &&
        (_origSchedFltUnit == rhs._origSchedFltUnit) && (_excludePrivate == rhs._excludePrivate) &&
        (_flightNoInd == rhs._flightNoInd) && (_historicalTktTvlInd == rhs._historicalTktTvlInd) &&
        (_outboundInd == rhs._outboundInd) && (_reissueToLower == rhs._reissueToLower) &&
        (_ticketEqualOrHigher == rhs._ticketEqualOrHigher) &&
        (_fareTypeTblItemNo == rhs._fareTypeTblItemNo) &&
        (_seasonalityDOWTblItemNo == rhs._seasonalityDOWTblItemNo) &&
        (_expndKeep == rhs._expndKeep));
  }

  static void dummyData(ReissueSequence& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'E';
    obj._validityInd = 'F';
    obj._tvlGeoTblItemNoFrom = 3;
    obj._tvlGeoTblItemNoTo = 4;
    obj._samePointTblItemNo = 5;
    obj._minGeoTblItemNoFrom = 6;
    obj._minGeoTblItemNoTo = 7;
    obj._fareCxrApplTblItemNo = 8;
    obj._risRestCxrTblItemNo = 9;
    obj._surchargeAmt1 = 10.10;
    obj._surchargeAmt2 = 11.11;
    obj._processingInd = 12;
    obj._ruleTariffNo = 13;
    obj._reissueTOD = 14;
    obj._departure = 15;
    obj._minStayTOD = 16;
    obj._surchargeNoDec1 = 17;
    obj._surchargeNoDec2 = 18;
    obj._creatorId = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._stopoverConnectInd = 'G';
    obj._portionInd = 'H';
    obj._terminalPointInd = 'I';
    obj._firstBreakInd = 'J';
    obj._couponInd = 'K';
    obj._extendInd = 'L';
    obj._journeyInd = 'M';
    obj._unusedFlightInd = 'N';
    obj._ruleInd = 'O';
    obj._ruleNo = "PQRS";
    obj._fareTypeInd = 'T';
    obj._fareClass = "cccccccc";
    obj._fareType = "UVW";
    obj._sameInd = 'X';
    obj._fareAmtInd = 'Y';
    obj._normalspecialInd = 'Z';
    obj._owrt = 'a';
    obj._dateInd = 'b';
    obj._fromAdvResInd = 'c';
    obj._toAdvResInd = 'g';
    obj._revalidationInd = 'e';
    obj._provision1 = 'f';
    obj._provision2 = 'g';
    obj._provision3 = 'h';
    obj._provision4 = 'i';
    obj._provision5 = 'j';
    obj._provision6 = 'k';
    obj._provision7 = 'l';
    obj._provision8 = 'm';
    obj._provision9 = 'n';
    obj._provision10 = 'o';
    obj._provision11 = 'p';
    obj._provision12 = 'q';
    obj._provision13 = 'r';
    obj._provision14 = 'd';
    obj._provision15 = 't';
    obj._provision17 = 'u';
    obj._provision18 = 'v';
    obj._provision50 = 'w';
    obj._bkgCdRevalInd = 'x';
    obj._ticketResvInd = 'y';
    obj._reissuePeriod = "dddddddd";
    obj._reissueUnit = "eeeeeeee";
    obj._optionInd = 'z';
    obj._departureUnit = '1';
    obj._departureInd = '2';
    obj._minStayPeriod = "ffffffff";
    obj._minStayUnit = '3';
    obj._autoTktInd = '4';
    obj._electronicTktInd = '5';
    obj._carrierRestInd = '6';
    obj._agencyLocRest = '7';
    obj._iataAgencyNo = "gggggggg";
    obj._surchargeType = '8';
    obj._surchargeAppl = '9';
    obj._surchargeCur1 = "0AB";
    obj._surchargeCur2 = "CDE";
    obj._stopInd = 'F';
    obj._origSchedFltPeriod = "hhhhhhhh";
    obj._origSchedFltUnit = 'G';
    obj._excludePrivate = 'H';
    obj._flightNoInd = 'I';
    obj._historicalTktTvlInd = 'J';
    obj._outboundInd = 'K';
    obj._reissueToLower = 'L';
    obj._ticketEqualOrHigher = 'M';
    obj._fareTypeTblItemNo = 19;
    obj._seasonalityDOWTblItemNo = 20;
    obj._expndKeep = 'N';
  }

protected:
  VendorCode _vendor;
  int _itemNo;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _inhibit;
  Indicator _validityInd;
  int _tvlGeoTblItemNoFrom;
  int _tvlGeoTblItemNoTo;
  int _samePointTblItemNo;
  int _minGeoTblItemNoFrom;
  int _minGeoTblItemNoTo;
  int _fareCxrApplTblItemNo;
  int _risRestCxrTblItemNo;
  MoneyAmount _surchargeAmt1;
  MoneyAmount _surchargeAmt2;
  int _processingInd;
  TariffNumber _ruleTariffNo;
  int _reissueTOD;
  int _departure;
  int _minStayTOD;
  int _surchargeNoDec1;
  int _surchargeNoDec2;
  std::string _creatorId;
  std::string _creatorBusinessUnit;
  Indicator _stopoverConnectInd;
  Indicator _portionInd;
  Indicator _terminalPointInd;
  Indicator _firstBreakInd;
  Indicator _couponInd;
  Indicator _extendInd;
  Indicator _journeyInd;
  Indicator _unusedFlightInd;
  Indicator _ruleInd;
  RuleNumber _ruleNo;
  Indicator _fareTypeInd;
  FareClassCode _fareClass;
  FareTypeAbbrev _fareType;
  Indicator _sameInd;
  Indicator _fareAmtInd;
  Indicator _normalspecialInd;
  Indicator _owrt;
  Indicator _dateInd;
  Indicator _fromAdvResInd;
  Indicator _toAdvResInd;
  Indicator _revalidationInd;
  Indicator _provision1;
  Indicator _provision2;
  Indicator _provision3;
  Indicator _provision4;
  Indicator _provision5;
  Indicator _provision6;
  Indicator _provision7;
  Indicator _provision8;
  Indicator _provision9;
  Indicator _provision10;
  Indicator _provision11;
  Indicator _provision12;
  Indicator _provision13;
  Indicator _provision14;
  Indicator _provision15;
  Indicator _provision17;
  Indicator _provision18;
  Indicator _provision50;
  Indicator _bkgCdRevalInd;
  Indicator _ticketResvInd;
  std::string _reissuePeriod;
  std::string _reissueUnit;
  Indicator _optionInd;
  Indicator _departureUnit;
  Indicator _departureInd;
  std::string _minStayPeriod;
  Indicator _minStayUnit;
  Indicator _autoTktInd;
  Indicator _electronicTktInd;
  Indicator _carrierRestInd;
  Indicator _agencyLocRest;
  AgencyIATA _iataAgencyNo;
  Indicator _surchargeType;
  Indicator _surchargeAppl;
  CurrencyCode _surchargeCur1;
  CurrencyCode _surchargeCur2;
  Indicator _stopInd;
  std::string _origSchedFltPeriod;
  Indicator _origSchedFltUnit;
  Indicator _excludePrivate;
  Indicator _flightNoInd;
  Indicator _historicalTktTvlInd;
  Indicator _outboundInd;
  Indicator _reissueToLower;
  Indicator _ticketEqualOrHigher;
  uint32_t _fareTypeTblItemNo;
  uint32_t _seasonalityDOWTblItemNo;
  Indicator _expndKeep;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _tvlGeoTblItemNoFrom);
    FLATTENIZE(archive, _tvlGeoTblItemNoTo);
    FLATTENIZE(archive, _samePointTblItemNo);
    FLATTENIZE(archive, _minGeoTblItemNoFrom);
    FLATTENIZE(archive, _minGeoTblItemNoTo);
    FLATTENIZE(archive, _fareCxrApplTblItemNo);
    FLATTENIZE(archive, _risRestCxrTblItemNo);
    FLATTENIZE(archive, _surchargeAmt1);
    FLATTENIZE(archive, _surchargeAmt2);
    FLATTENIZE(archive, _processingInd);
    FLATTENIZE(archive, _ruleTariffNo);
    FLATTENIZE(archive, _reissueTOD);
    FLATTENIZE(archive, _departure);
    FLATTENIZE(archive, _minStayTOD);
    FLATTENIZE(archive, _surchargeNoDec1);
    FLATTENIZE(archive, _surchargeNoDec2);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _stopoverConnectInd);
    FLATTENIZE(archive, _portionInd);
    FLATTENIZE(archive, _terminalPointInd);
    FLATTENIZE(archive, _firstBreakInd);
    FLATTENIZE(archive, _couponInd);
    FLATTENIZE(archive, _extendInd);
    FLATTENIZE(archive, _journeyInd);
    FLATTENIZE(archive, _unusedFlightInd);
    FLATTENIZE(archive, _ruleInd);
    FLATTENIZE(archive, _ruleNo);
    FLATTENIZE(archive, _fareTypeInd);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _sameInd);
    FLATTENIZE(archive, _fareAmtInd);
    FLATTENIZE(archive, _normalspecialInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _dateInd);
    FLATTENIZE(archive, _fromAdvResInd);
    FLATTENIZE(archive, _toAdvResInd);
    FLATTENIZE(archive, _revalidationInd);
    FLATTENIZE(archive, _provision1);
    FLATTENIZE(archive, _provision2);
    FLATTENIZE(archive, _provision3);
    FLATTENIZE(archive, _provision4);
    FLATTENIZE(archive, _provision5);
    FLATTENIZE(archive, _provision6);
    FLATTENIZE(archive, _provision7);
    FLATTENIZE(archive, _provision8);
    FLATTENIZE(archive, _provision9);
    FLATTENIZE(archive, _provision10);
    FLATTENIZE(archive, _provision11);
    FLATTENIZE(archive, _provision12);
    FLATTENIZE(archive, _provision13);
    FLATTENIZE(archive, _provision14);
    FLATTENIZE(archive, _provision15);
    FLATTENIZE(archive, _provision17);
    FLATTENIZE(archive, _provision18);
    FLATTENIZE(archive, _provision50);
    FLATTENIZE(archive, _bkgCdRevalInd);
    FLATTENIZE(archive, _ticketResvInd);
    FLATTENIZE(archive, _reissuePeriod);
    FLATTENIZE(archive, _reissueUnit);
    FLATTENIZE(archive, _optionInd);
    FLATTENIZE(archive, _departureUnit);
    FLATTENIZE(archive, _departureInd);
    FLATTENIZE(archive, _minStayPeriod);
    FLATTENIZE(archive, _minStayUnit);
    FLATTENIZE(archive, _autoTktInd);
    FLATTENIZE(archive, _electronicTktInd);
    FLATTENIZE(archive, _carrierRestInd);
    FLATTENIZE(archive, _agencyLocRest);
    FLATTENIZE(archive, _iataAgencyNo);
    FLATTENIZE(archive, _surchargeType);
    FLATTENIZE(archive, _surchargeAppl);
    FLATTENIZE(archive, _surchargeCur1);
    FLATTENIZE(archive, _surchargeCur2);
    FLATTENIZE(archive, _stopInd);
    FLATTENIZE(archive, _origSchedFltPeriod);
    FLATTENIZE(archive, _origSchedFltUnit);
    FLATTENIZE(archive, _excludePrivate);
    FLATTENIZE(archive, _flightNoInd);
    FLATTENIZE(archive, _historicalTktTvlInd);
    FLATTENIZE(archive, _outboundInd);
    FLATTENIZE(archive, _reissueToLower);
    FLATTENIZE(archive, _ticketEqualOrHigher);
    FLATTENIZE(archive, _fareTypeTblItemNo);
    FLATTENIZE(archive, _seasonalityDOWTblItemNo);
    FLATTENIZE(archive, _expndKeep);
  }

protected:
private:
  ReissueSequence(const ReissueSequence&);
  ReissueSequence& operator=(const ReissueSequence&);
};

class ReissueSequenceW
{

public:
  enum ConditionallyOverridenByte
  {
    coNone = 0x00
  };

  typedef uint8_t BitsContainer; // change type when ConditionallyOverridenByte will out of current
                                 // range
  typedef SmallBitSet<BitsContainer, ConditionallyOverridenByte> ConditionallyOverridenBytesSet;

  ReissueSequenceW() : _orig(nullptr), _overriding(nullptr), _conditionallyOverridenBytes(coNone) {}
  ~ReissueSequenceW() {}

  // SC598
  Indicator origSchedFltUnit() const
  {
    return overridingWhenExists()->origSchedFltUnit();
  } // Bytes 88-91
  const std::string& origSchedFltPeriod() const
  {
    return overridingWhenExists()->origSchedFltPeriod();
  }
  Indicator unusedFlightInd() const { return overridingWhenExists()->unusedFlightInd(); }

  // SC604
  int processingInd() const { return overridingWhenExists()->processingInd(); }

  // SC645
  Indicator ticketEqualOrHigher() const { return overridingWhenExists()->ticketEqualOrHigher(); }

  // SC606
  Indicator terminalPointInd() const
  {
    return overridingWhenExists()->terminalPointInd();
  } // Bytes 24-31
  Indicator firstBreakInd() const { return overridingWhenExists()->firstBreakInd(); }
  Indicator couponInd() const { return overridingWhenExists()->couponInd(); }
  Indicator extendInd() const { return overridingWhenExists()->extendInd(); }
  Indicator journeyInd() const { return overridingWhenExists()->journeyInd(); }
  int samePointTblItemNo() const { return overridingWhenExists()->samePointTblItemNo(); }
  Indicator ruleInd() const { return overridingWhenExists()->ruleInd(); } // bytes 33-58
  int fareCxrApplTblItemNo() const { return overridingWhenExists()->fareCxrApplTblItemNo(); }
  const TariffNumber& ruleTariffNo() const { return overridingWhenExists()->ruleTariffNo(); }
  const RuleNumber& ruleNo() const { return overridingWhenExists()->ruleNo(); }
  Indicator fareTypeInd() const { return overridingWhenExists()->fareTypeInd(); }
  Indicator sameInd() const { return overridingWhenExists()->sameInd(); }
  const FareClassCode& fareClass() const { return overridingWhenExists()->fareClass(); }
  const FareTypeAbbrev& fareType() const { return overridingWhenExists()->fareType(); }
  Indicator fareAmtInd() const { return overridingWhenExists()->fareAmtInd(); }
  Indicator normalspecialInd() const { return overridingWhenExists()->normalspecialInd(); }
  Indicator owrt() const { return overridingWhenExists()->owrt(); }
  Indicator excludePrivate() const { return overridingWhenExists()->excludePrivate(); } // byte 81

  // SC609
  Indicator ticketResvInd() const { return overridingWhenExists()->ticketResvInd(); } // Bytes
                                                                                      // 92-106
  int reissueTOD() const { return overridingWhenExists()->reissueTOD(); }
  const std::string& reissuePeriod() const { return overridingWhenExists()->reissuePeriod(); }
  const std::string& reissueUnit() const { return overridingWhenExists()->reissueUnit(); }
  Indicator optionInd() const { return overridingWhenExists()->optionInd(); }
  int departure() const { return overridingWhenExists()->departure(); }
  Indicator departureUnit() const { return overridingWhenExists()->departureUnit(); }
  Indicator departureInd() const { return overridingWhenExists()->departureInd(); }
  Indicator autoTktInd() const { return overridingWhenExists()->autoTktInd(); } // Bytes 122-135
  Indicator electronicTktInd() const { return overridingWhenExists()->electronicTktInd(); }
  Indicator carrierRestInd() const { return overridingWhenExists()->carrierRestInd(); }
  int risRestCxrTblItemNo() const { return overridingWhenExists()->risRestCxrTblItemNo(); }
  Indicator agencyLocRest() const { return overridingWhenExists()->agencyLocRest(); }
  const AgencyIATA& iataAgencyNo() const { return overridingWhenExists()->iataAgencyNo(); }
  Indicator reissueToLower() const { return overridingWhenExists()->reissueToLower(); } // Byte 155
  uint32_t fareTypeTblItemNo() const { return overridingWhenExists()->fareTypeTblItemNo(); }
  uint32_t seasonalityDOWTblItemNo() const
  {
    return overridingWhenExists()->seasonalityDOWTblItemNo();
  }
  Indicator expndKeep() const { return overridingWhenExists()->expndKeep(); }

  int itemNo() const { return orig()->itemNo(); }
  int seqNo() const { return orig()->seqNo(); }
  const VendorCode& vendor() const { return orig()->vendor(); }
  const DateTime& createDate() const { return orig()->createDate(); }
  const DateTime& expireDate() const { return orig()->expireDate(); }
  Indicator inhibit() const { return orig()->inhibit(); }
  Indicator validityInd() const { return orig()->validityInd(); }
  int tvlGeoTblItemNoFrom() const { return orig()->tvlGeoTblItemNoFrom(); }
  int tvlGeoTblItemNoTo() const { return orig()->tvlGeoTblItemNoTo(); }
  int minGeoTblItemNoFrom() const { return orig()->minGeoTblItemNoFrom(); }
  int minGeoTblItemNoTo() const { return orig()->minGeoTblItemNoTo(); }
  const MoneyAmount& surchargeAmt1() const { return orig()->surchargeAmt1(); }
  const MoneyAmount& surchargeAmt2() const { return orig()->surchargeAmt2(); }
  int minStayTOD() const { return orig()->minStayTOD(); }
  int surchargeNoDec1() const { return orig()->surchargeNoDec1(); }
  int surchargeNoDec2() const { return orig()->surchargeNoDec2(); }
  const std::string& creatorId() const { return orig()->creatorId(); }
  const std::string& creatorBusinessUnit() const { return orig()->creatorBusinessUnit(); }
  Indicator stopoverConnectInd() const { return orig()->stopoverConnectInd(); }
  Indicator portionInd() const { return orig()->portionInd(); }
  Indicator dateInd() const { return orig()->dateInd(); }
  Indicator fromAdvResInd() const { return orig()->fromAdvResInd(); }
  Indicator toAdvResInd() const { return orig()->toAdvResInd(); }
  Indicator revalidationInd() const { return orig()->revalidationInd(); }
  Indicator provision1() const { return orig()->provision1(); }
  Indicator provision2() const { return orig()->provision2(); }
  Indicator provision3() const { return orig()->provision3(); }
  Indicator provision4() const { return orig()->provision4(); }
  Indicator provision5() const { return orig()->provision5(); }
  Indicator provision6() const { return orig()->provision6(); }
  Indicator provision7() const { return orig()->provision7(); }
  Indicator provision8() const { return orig()->provision8(); }
  Indicator provision9() const { return orig()->provision9(); }
  Indicator provision10() const { return orig()->provision10(); }
  Indicator provision11() const { return orig()->provision11(); }
  Indicator provision12() const { return orig()->provision12(); }
  Indicator provision13() const { return orig()->provision13(); }
  Indicator provision14() const { return orig()->provision14(); }
  Indicator provision15() const { return orig()->provision15(); }
  Indicator provision17() const { return orig()->provision17(); }
  Indicator provision18() const { return orig()->provision18(); }
  Indicator provision50() const { return orig()->provision50(); }
  Indicator bkgCdRevalInd() const { return orig()->bkgCdRevalInd(); }
  const std::string& minStayPeriod() const { return orig()->minStayPeriod(); }
  Indicator minStayUnit() const { return orig()->minStayUnit(); }
  Indicator surchargeType() const { return orig()->surchargeType(); }
  Indicator surchargeAppl() const { return orig()->surchargeAppl(); }
  const CurrencyCode& surchargeCur1() const { return orig()->surchargeCur1(); }
  const CurrencyCode& surchargeCur2() const { return orig()->surchargeCur2(); }
  Indicator stopInd() const { return orig()->stopInd(); }
  Indicator flightNoInd() const { return orig()->flightNoInd(); }
  Indicator historicalTktTvlInd() const { return orig()->historicalTktTvlInd(); }
  Indicator outboundInd() const { return orig()->outboundInd(); }

  const ReissueSequence*& orig() { return _orig; }
  const ReissueSequence* orig() const { return _orig; }

  const ReissueSequence*& overriding() { return _overriding; }
  const ReissueSequence* overriding() const { return _overriding; }

  void setConditionallyOverridenBytes(BitsContainer bytes, bool value)
  {
    _conditionallyOverridenBytes.set((ConditionallyOverridenByte)bytes, value);
  }

  const ReissueSequence* overridingWhenExists(ConditionallyOverridenByte byte = coNone) const
  {
    return (_overriding && (byte == coNone || _conditionallyOverridenBytes.isSet(byte)))
               ? _overriding
               : _orig;
  }

  bool operator==(const ReissueSequenceW& rhs) const
  {
    return ((_orig == rhs._orig) && (_overriding == rhs._overriding)
            //              && ( _conditionallyOverridenBytes == rhs._conditionallyOverridenBytes )
            );
  }

protected:
  const ReissueSequence* _orig; // for domestic Fc
  const ReissueSequence* _overriding; // for overriding international Fc
  ConditionallyOverridenBytesSet _conditionallyOverridenBytes;
};

} // end tse namespace

