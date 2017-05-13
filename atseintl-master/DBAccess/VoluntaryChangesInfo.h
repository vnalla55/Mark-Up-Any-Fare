// ----------------------------------------------------------------------------
//  � 2006, Sabre Inc.  All rights reserved.  This software/documentation
//    is the confidential and proprietary product of Sabre Inc. Any
//    unauthorized use, reproduction, or transfer of this
//    software/documentation, in any medium, or incorporation of this
//    software/documentation into any system or publication, is strictly
//    prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/SmallBitSet.h"
#include "Common/TseStream.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class VoluntaryChangesInfo : public RuleItemInfo, public tse::Stream
{
public:
  VoluntaryChangesInfo()
    : _createDate(0),
      _expireDate(0),
      _waiverTblItemNo(0),
      _reissueTblItemNo(0),
      _carrierApplTblItemNo(0),
      _advResLastTOD(-1),
      _psgOccurrenceFirst(0),
      _psgOccurrenceLast(0),
      _inhibit(' '),
      _unavailTag(' '),
      _psgType(""),
      _tktValidityInd(' '),
      _departureInd(' '),
      _priceableUnitInd(' '),
      _fareComponentInd(' '),
      _advResFrom(' '),
      _advResTo(' '),
      _advResPeriod(""),
      _advResUnit(""),
      _tktTimeLimit(' '),
      _changeInd(' '),
      _penaltyAmt1(0),
      _penaltyAmt2(0),
      _noDec1(0),
      _noDec2(0),
      _percent(0),
      _cur1(""),
      _cur2(""),
      _highLowInd(' '),
      _minAmt(0),
      _minNoDec(0),
      _minCur(""),
      _journeyInd(' '),
      _feeAppl(' '),
      _typeOfSvcInd(' '),
      _tktTransactionInd(' '),
      _discountTag1(' '),
      _discountTag2(' '),
      _discountTag3(' '),
      _discountTag4(' '),
      _residualInd(' '),
      _formOfRefund(' '),
      _endorsement(' '),
      _sameAirport(' '),
      _domesticIntlComb(' '),
      _residualHierarchy(' ')
  {
  }

  virtual ~VoluntaryChangesInfo() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  uint32_t& waiverTblItemNo() { return _waiverTblItemNo; }
  uint32_t waiverTblItemNo() const { return _waiverTblItemNo; }

  uint32_t& reissueTblItemNo() { return _reissueTblItemNo; }
  uint32_t reissueTblItemNo() const { return _reissueTblItemNo; }

  int16_t& advResLastTOD() { return _advResLastTOD; }
  int16_t advResLastTOD() const { return _advResLastTOD; }

  int16_t& psgOccurrenceFirst() { return _psgOccurrenceFirst; }
  int16_t psgOccurrenceFirst() const { return _psgOccurrenceFirst; }

  int16_t& psgOccurrenceLast() { return _psgOccurrenceLast; }
  int16_t psgOccurrenceLast() const { return _psgOccurrenceLast; }

  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }

  Indicator& unavailTag() { return _unavailTag; }
  Indicator unavailTag() const { return _unavailTag; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  Indicator& tktValidityInd() { return _tktValidityInd; }
  Indicator tktValidityInd() const { return _tktValidityInd; }

  Indicator& departureInd() { return _departureInd; }
  Indicator departureInd() const { return _departureInd; }

  Indicator& priceableUnitInd() { return _priceableUnitInd; }
  Indicator priceableUnitInd() const { return _priceableUnitInd; }

  Indicator& fareComponentInd() { return _fareComponentInd; }
  Indicator fareComponentInd() const { return _fareComponentInd; }

  Indicator& advResFrom() { return _advResFrom; }
  Indicator advResFrom() const { return _advResFrom; }

  Indicator& advResTo() { return _advResTo; }
  Indicator advResTo() const { return _advResTo; }

  ResPeriod& advResPeriod() { return _advResPeriod; }
  const ResPeriod& advResPeriod() const { return _advResPeriod; }

  ResUnit& advResUnit() { return _advResUnit; }
  const ResUnit& advResUnit() const { return _advResUnit; }

  Indicator& tktTimeLimit() { return _tktTimeLimit; }
  Indicator tktTimeLimit() const { return _tktTimeLimit; }

  Indicator& changeInd() { return _changeInd; }
  Indicator changeInd() const { return _changeInd; }

  MoneyAmount& penaltyAmt1() { return _penaltyAmt1; }
  const MoneyAmount& penaltyAmt1() const { return _penaltyAmt1; }

  MoneyAmount& penaltyAmt2() { return _penaltyAmt2; }
  const MoneyAmount& penaltyAmt2() const { return _penaltyAmt2; }

  int16_t& noDec1() { return _noDec1; }
  int16_t noDec1() const { return _noDec1; }

  int16_t& noDec2() { return _noDec2; }
  int16_t noDec2() const { return _noDec2; }

  double& percent() { return _percent; }
  const double& percent() const { return _percent; }

  CurrencyCode& cur1() { return _cur1; }
  const CurrencyCode& cur1() const { return _cur1; }

  CurrencyCode& cur2() { return _cur2; }
  const CurrencyCode& cur2() const { return _cur2; }

  Indicator& highLowInd() { return _highLowInd; }
  Indicator highLowInd() const { return _highLowInd; }

  MoneyAmount& minAmt() { return _minAmt; }
  const MoneyAmount& minAmt() const { return _minAmt; }

  int16_t& minNoDec() { return _minNoDec; }
  int16_t minNoDec() const { return _minNoDec; }

  CurrencyCode& minCur() { return _minCur; }
  const CurrencyCode& minCur() const { return _minCur; }

  Indicator& journeyInd() { return _journeyInd; }
  Indicator journeyInd() const { return _journeyInd; }

  Indicator& feeAppl() { return _feeAppl; }
  Indicator feeAppl() const { return _feeAppl; }

  Indicator& typeOfSvcInd() { return _typeOfSvcInd; }
  Indicator typeOfSvcInd() const { return _typeOfSvcInd; }

  Indicator& tktTransactionInd() { return _tktTransactionInd; }
  Indicator tktTransactionInd() const { return _tktTransactionInd; }

  Indicator& discountTag1() { return _discountTag1; }
  Indicator discountTag1() const { return _discountTag1; }

  Indicator& discountTag2() { return _discountTag2; }
  Indicator discountTag2() const { return _discountTag2; }

  Indicator& discountTag3() { return _discountTag3; }
  Indicator discountTag3() const { return _discountTag3; }

  Indicator& discountTag4() { return _discountTag4; }
  Indicator discountTag4() const { return _discountTag4; }

  Indicator& residualInd() { return _residualInd; }
  Indicator residualInd() const { return _residualInd; }

  Indicator& formOfRefund() { return _formOfRefund; }
  Indicator formOfRefund() const { return _formOfRefund; }

  Indicator& endorsement() { return _endorsement; }
  Indicator endorsement() const { return _endorsement; }

  Indicator& sameAirport() { return _sameAirport; }
  Indicator sameAirport() const { return _sameAirport; }

  Indicator& domesticIntlComb() { return _domesticIntlComb; }
  Indicator domesticIntlComb() const { return _domesticIntlComb; }

  Indicator& residualHierarchy() { return _residualHierarchy; }
  Indicator residualHierarchy() const { return _residualHierarchy; }

  uint32_t& carrierApplTblItemNo() { return _carrierApplTblItemNo; }
  uint32_t carrierApplTblItemNo() const { return _carrierApplTblItemNo; }

  virtual bool operator==(const VoluntaryChangesInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_waiverTblItemNo == rhs._waiverTblItemNo) &&
            (_reissueTblItemNo == rhs._reissueTblItemNo) &&
            (_carrierApplTblItemNo == rhs._carrierApplTblItemNo) &&
            (_advResLastTOD == rhs._advResLastTOD) &&
            (_psgOccurrenceFirst == rhs._psgOccurrenceFirst) &&
            (_psgOccurrenceLast == rhs._psgOccurrenceLast) && (_inhibit == rhs._inhibit) &&
            (_unavailTag == rhs._unavailTag) && (_psgType == rhs._psgType) &&
            (_tktValidityInd == rhs._tktValidityInd) && (_departureInd == rhs._departureInd) &&
            (_priceableUnitInd == rhs._priceableUnitInd) &&
            (_fareComponentInd == rhs._fareComponentInd) && (_advResFrom == rhs._advResFrom) &&
            (_advResTo == rhs._advResTo) && (_advResPeriod == rhs._advResPeriod) &&
            (_advResUnit == rhs._advResUnit) && (_tktTimeLimit == rhs._tktTimeLimit) &&
            (_changeInd == rhs._changeInd) && (_penaltyAmt1 == rhs._penaltyAmt1) &&
            (_penaltyAmt2 == rhs._penaltyAmt2) && (_noDec1 == rhs._noDec1) &&
            (_noDec2 == rhs._noDec2) && (_percent == rhs._percent) && (_cur1 == rhs._cur1) &&
            (_cur2 == rhs._cur2) && (_highLowInd == rhs._highLowInd) && (_minAmt == rhs._minAmt) &&
            (_minNoDec == rhs._minNoDec) && (_minCur == rhs._minCur) &&
            (_journeyInd == rhs._journeyInd) && (_feeAppl == rhs._feeAppl) &&
            (_typeOfSvcInd == rhs._typeOfSvcInd) &&
            (_tktTransactionInd == rhs._tktTransactionInd) &&
            (_discountTag1 == rhs._discountTag1) && (_discountTag2 == rhs._discountTag2) &&
            (_discountTag3 == rhs._discountTag3) && (_discountTag4 == rhs._discountTag4) &&
            (_residualInd == rhs._residualInd) && (_formOfRefund == rhs._formOfRefund) &&
            (_endorsement == rhs._endorsement) && (_sameAirport == rhs._sameAirport) &&
            (_domesticIntlComb == rhs._domesticIntlComb) &&
            (_residualHierarchy == rhs._residualHierarchy));
  }

  static void dummyDadta(VoluntaryChangesInfo& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._waiverTblItemNo = 1;
    obj._reissueTblItemNo = 2;
    obj._carrierApplTblItemNo = 3;
    obj._advResLastTOD = 4;
    obj._psgOccurrenceFirst = 5;
    obj._psgOccurrenceLast = 6;
    obj._inhibit = 'A';
    obj._unavailTag = 'B';
    obj._psgType = "CDE";
    obj._tktValidityInd = 'F';
    obj._departureInd = 'G';
    obj._priceableUnitInd = 'H';
    obj._fareComponentInd = 'I';
    obj._advResFrom = 'J';
    obj._advResTo = 'K';
    obj._advResPeriod = "LMN";
    obj._advResUnit = "OP";
    obj._tktTimeLimit = 'Q';
    obj._changeInd = 'R';
    obj._penaltyAmt1 = 7.77;
    obj._penaltyAmt2 = 8.88;
    obj._noDec1 = 9;
    obj._noDec2 = 10;
    obj._percent = 11.111;
    obj._cur1 = "STU";
    obj._cur2 = "UVW";
    obj._highLowInd = 'X';
    obj._minAmt = 12.12;
    obj._minNoDec = 13;
    obj._minCur = "YZa";
    obj._journeyInd = 'b';
    obj._feeAppl = 'c';
    obj._typeOfSvcInd = 'd';
    obj._tktTransactionInd = 'e';
    obj._discountTag1 = 'f';
    obj._discountTag2 = 'g';
    obj._discountTag3 = 'h';
    obj._discountTag4 = 'i';
    obj._residualInd = 'j';
    obj._formOfRefund = 'k';
    obj._endorsement = 'l';
    obj._sameAirport = 'm';
    obj._domesticIntlComb = 'n';
    obj._residualHierarchy = 'o';
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
  uint32_t _waiverTblItemNo;
  uint32_t _reissueTblItemNo;
  uint32_t _carrierApplTblItemNo;
  int16_t _advResLastTOD;
  int16_t _psgOccurrenceFirst;
  int16_t _psgOccurrenceLast;
  Indicator _inhibit;
  Indicator _unavailTag;
  PaxTypeCode _psgType;
  Indicator _tktValidityInd;
  Indicator _departureInd;
  Indicator _priceableUnitInd;
  Indicator _fareComponentInd;
  Indicator _advResFrom;
  Indicator _advResTo;
  ResPeriod _advResPeriod;
  ResUnit _advResUnit;
  Indicator _tktTimeLimit;
  Indicator _changeInd;
  MoneyAmount _penaltyAmt1;
  MoneyAmount _penaltyAmt2;
  int16_t _noDec1;
  int16_t _noDec2;
  double _percent;
  CurrencyCode _cur1;
  CurrencyCode _cur2;
  Indicator _highLowInd;
  MoneyAmount _minAmt;
  int16_t _minNoDec;
  CurrencyCode _minCur;
  Indicator _journeyInd;
  Indicator _feeAppl;
  Indicator _typeOfSvcInd;
  Indicator _tktTransactionInd;
  Indicator _discountTag1;
  Indicator _discountTag2;
  Indicator _discountTag3;
  Indicator _discountTag4;
  Indicator _residualInd;
  Indicator _formOfRefund;
  Indicator _endorsement;
  Indicator _sameAirport;
  Indicator _domesticIntlComb;
  Indicator _residualHierarchy;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _waiverTblItemNo);
    FLATTENIZE(archive, _reissueTblItemNo);
    FLATTENIZE(archive, _carrierApplTblItemNo);
    FLATTENIZE(archive, _advResLastTOD);
    FLATTENIZE(archive, _psgOccurrenceFirst);
    FLATTENIZE(archive, _psgOccurrenceLast);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _tktValidityInd);
    FLATTENIZE(archive, _departureInd);
    FLATTENIZE(archive, _priceableUnitInd);
    FLATTENIZE(archive, _fareComponentInd);
    FLATTENIZE(archive, _advResFrom);
    FLATTENIZE(archive, _advResTo);
    FLATTENIZE(archive, _advResPeriod);
    FLATTENIZE(archive, _advResUnit);
    FLATTENIZE(archive, _tktTimeLimit);
    FLATTENIZE(archive, _changeInd);
    FLATTENIZE(archive, _penaltyAmt1);
    FLATTENIZE(archive, _penaltyAmt2);
    FLATTENIZE(archive, _noDec1);
    FLATTENIZE(archive, _noDec2);
    FLATTENIZE(archive, _percent);
    FLATTENIZE(archive, _cur1);
    FLATTENIZE(archive, _cur2);
    FLATTENIZE(archive, _highLowInd);
    FLATTENIZE(archive, _minAmt);
    FLATTENIZE(archive, _minNoDec);
    FLATTENIZE(archive, _minCur);
    FLATTENIZE(archive, _journeyInd);
    FLATTENIZE(archive, _feeAppl);
    FLATTENIZE(archive, _typeOfSvcInd);
    FLATTENIZE(archive, _tktTransactionInd);
    FLATTENIZE(archive, _discountTag1);
    FLATTENIZE(archive, _discountTag2);
    FLATTENIZE(archive, _discountTag3);
    FLATTENIZE(archive, _discountTag4);
    FLATTENIZE(archive, _residualInd);
    FLATTENIZE(archive, _formOfRefund);
    FLATTENIZE(archive, _endorsement);
    FLATTENIZE(archive, _sameAirport);
    FLATTENIZE(archive, _domesticIntlComb);
    FLATTENIZE(archive, _residualHierarchy);
  }

protected:
private:
  VoluntaryChangesInfo(const VoluntaryChangesInfo&);
  VoluntaryChangesInfo& operator=(const VoluntaryChangesInfo&);

  void operator>>(std::ostream& os) override
  {
    int tab = 4, name_size = 40;
    os << std::endl;
    os << "--------------------------------------------------------------------------------"
       << std::endl;
    os << "COMMON" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size) << "ItemNo:" << _itemNo
       << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size) << "Vendor:" << _vendor
       << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size) << "CreateDate:" << _createDate
       << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size) << "ExpireDate:" << _expireDate
       << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size) << "Inhibit:" << _inhibit
       << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "TextTblItemNo (bytes 101-103):" << _textTblItemNo << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "OverrideDateTblItemNo (bytes 104-106):" << _overrideDateTblItemNo << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "UnavailTag (byte 107):" << _unavailTag << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "ReissueTblItemNo (bytes 44-46):" << _reissueTblItemNo << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "AdvResLastTOD (byte 92):" << _advResLastTOD << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DomesticIntlComb (byte 88):" << _domesticIntlComb << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "CarrierApplTblItemNo (byte 89-91):" << _carrierApplTblItemNo << std::endl;
    os << "WHO" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PsgType (bytes 8-10):" << _psgType << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PsgOccurrenceFirst (bytes 40-41):" << _psgOccurrenceFirst << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PsgOccurrenceLast (bytes 42-43):" << _psgOccurrenceLast << std::endl;
    os << "WAIVER TABLE" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "WaiverTblItemNo (bytes 11-13):" << _waiverTblItemNo << std::endl;
    os << "WHEN" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "TktValidityInd (byte 14):" << _tktValidityInd << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DepartureInd (byte 15):" << _departureInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PriceableUnitInd (byte 16):" << _priceableUnitInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "FareComponentInd (byte 17):" << _fareComponentInd << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "AdvResFrom (byte 18):" << _advResFrom << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "AdvResTo (byte 19):" << _advResTo << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "AdvResPeriod (bytes 33-35):" << _advResPeriod << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "AdvResUnit (bytes 36-37):" << _advResUnit << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "TktTimeLimit (byte 38):" << _tktTimeLimit << std::endl;
    os << "NUMBER OF CHANGES PERMITTED" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "ChangeInd (byte 39):" << _changeInd << std::endl;
    os << "AMOUNT" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PenaltyAmt1 (bytes 47-50):" << _penaltyAmt1 << _cur1 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "PenaltyAmt2 (bytes 51-54):" << _penaltyAmt2 << _cur2 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "NoDec1 (bytes 59-62):" << _noDec1 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "NoDec2 (bytes 63-66):" << _noDec2 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "Percent (bytes 55-58):" << _percent << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "HighLowInd (byte 67):" << _highLowInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "MinAmt (byte 68-70):" << _minAmt << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "MinNoDec (byte 71-72):" << _minNoDec << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "MinCur (byte 73-75):" << _minCur << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "JourneyInd (byte 76):" << _journeyInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "FeeAppl (byte 77):" << _feeAppl << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "TypeOfSvcInd (byte 78):" << _typeOfSvcInd << std::endl;
    os << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DiscountTag1 (byte 80):" << _discountTag1 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DiscountTag2 (byte 81):" << _discountTag2 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DiscountTag3 (byte 82):" << _discountTag3 << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "DiscountTag4 (byte 83):" << _discountTag4 << std::endl;
    os << "TICKET REISSUE" << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "TktTransactionInd (byte 79):" << _tktTransactionInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "ResidualInd (byte 84):" << _residualInd << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "FormOfRefund (byte 85):" << _formOfRefund << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "Endorsement (byte 86):" << _endorsement << std::endl;
    os << std::setw(tab) << "" << std::left << std::setw(name_size)
       << "SameAirport (byte 93):" << _sameAirport << std::endl;
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
           & ptr->_waiverTblItemNo
           & ptr->_reissueTblItemNo
           & ptr->_carrierApplTblItemNo
           & ptr->_advResLastTOD
           & ptr->_psgOccurrenceFirst
           & ptr->_psgOccurrenceLast
           & ptr->_inhibit
           & ptr->_unavailTag
           & ptr->_psgType
           & ptr->_tktValidityInd
           & ptr->_departureInd
           & ptr->_priceableUnitInd
           & ptr->_fareComponentInd
           & ptr->_advResFrom
           & ptr->_advResTo
           & ptr->_advResPeriod
           & ptr->_advResUnit
           & ptr->_tktTimeLimit
           & ptr->_changeInd
           & ptr->_penaltyAmt1
           & ptr->_penaltyAmt2
           & ptr->_noDec1
           & ptr->_noDec2
           & ptr->_percent
           & ptr->_cur1
           & ptr->_cur2
           & ptr->_highLowInd
           & ptr->_minAmt
           & ptr->_minNoDec
           & ptr->_minCur
           & ptr->_journeyInd
           & ptr->_feeAppl
           & ptr->_typeOfSvcInd
           & ptr->_tktTransactionInd
           & ptr->_discountTag1
           & ptr->_discountTag2
           & ptr->_discountTag3
           & ptr->_discountTag4
           & ptr->_residualInd
           & ptr->_formOfRefund
           & ptr->_endorsement
           & ptr->_sameAirport
           & ptr->_domesticIntlComb
           & ptr->_residualHierarchy;
  }
};

class VoluntaryChangesInfoW
{
public:
  enum ConditionallyOverridenByte
  {
    coNone = 0x00,
    coByte47_75 = 0x01
  };

  typedef uint8_t BitsContainer; // change here when ConditionallyOverridenByte will out of current
                                 // range
  typedef SmallBitSet<BitsContainer, ConditionallyOverridenByte> ConditionallyOverridenBytesSet;

  VoluntaryChangesInfoW() : _orig(nullptr), _overriding(nullptr), _conditionallyOverridenBytes(coNone) {}
  ~VoluntaryChangesInfoW() {}

  // SC599
  Indicator changeInd() const { return overridingWhenExists()->changeInd(); }

  // SC600
  const MoneyAmount& penaltyAmt1() const
  {
    return overridingWhenExists(coByte47_75)->penaltyAmt1();
  }
  const MoneyAmount& penaltyAmt2() const
  {
    return overridingWhenExists(coByte47_75)->penaltyAmt2();
  }
  int16_t noDec1() const { return overridingWhenExists(coByte47_75)->noDec1(); }
  int16_t noDec2() const { return overridingWhenExists(coByte47_75)->noDec2(); }
  const double& percent() const { return overridingWhenExists(coByte47_75)->percent(); }
  const CurrencyCode& cur1() const { return overridingWhenExists(coByte47_75)->cur1(); }
  const CurrencyCode& cur2() const { return overridingWhenExists(coByte47_75)->cur2(); }
  Indicator highLowInd() const { return overridingWhenExists(coByte47_75)->highLowInd(); }
  const MoneyAmount& minAmt() const { return overridingWhenExists(coByte47_75)->minAmt(); }
  int16_t minNoDec() const { return overridingWhenExists(coByte47_75)->minNoDec(); }
  const CurrencyCode& minCur() const { return overridingWhenExists(coByte47_75)->minCur(); }

  // SC601
  Indicator journeyInd() const { return overridingWhenExists()->journeyInd(); }
  Indicator feeAppl() const { return overridingWhenExists()->feeAppl(); }
  Indicator discountTag1() const { return overridingWhenExists()->discountTag1(); }
  Indicator discountTag2() const { return overridingWhenExists()->discountTag2(); }
  Indicator discountTag3() const { return overridingWhenExists()->discountTag3(); }
  Indicator discountTag4() const { return overridingWhenExists()->discountTag4(); }

  // SC602
  Indicator residualInd() const { return overridingWhenExists()->residualInd(); }
  Indicator formOfRefund() const { return overridingWhenExists()->formOfRefund(); }
  Indicator endorsement() const { return overridingWhenExists()->endorsement(); }

  uint32_t reissueTblItemNo() const { return orig()->reissueTblItemNo(); }
  const VendorCode& vendor() const { return orig()->vendor(); }
  uint32_t itemNo() const { return orig()->itemNo(); }
  const DateTime& createDate() const { return orig()->createDate(); }
  const DateTime& expireDate() const { return orig()->expireDate(); }
  uint32_t waiverTblItemNo() const { return orig()->waiverTblItemNo(); }
  int16_t advResLastTOD() const { return orig()->advResLastTOD(); }
  int16_t psgOccurrenceFirst() const { return orig()->psgOccurrenceFirst(); }
  int16_t psgOccurrenceLast() const { return orig()->psgOccurrenceLast(); }
  Indicator inhibit() const { return orig()->inhibit(); }
  Indicator unavailTag() const { return orig()->unavailTag(); }
  const PaxTypeCode& psgType() const { return orig()->psgType(); }
  Indicator tktValidityInd() const { return orig()->tktValidityInd(); }
  Indicator departureInd() const { return orig()->departureInd(); }
  Indicator priceableUnitInd() const { return orig()->priceableUnitInd(); }
  Indicator fareComponentInd() const { return orig()->fareComponentInd(); }
  Indicator advResFrom() const { return orig()->advResFrom(); }
  Indicator advResTo() const { return orig()->advResTo(); }
  const ResPeriod& advResPeriod() const { return orig()->advResPeriod(); }
  const ResUnit& advResUnit() const { return orig()->advResUnit(); }
  Indicator tktTimeLimit() const { return orig()->tktTimeLimit(); }
  Indicator typeOfSvcInd() const { return orig()->typeOfSvcInd(); }
  Indicator tktTransactionInd() const { return orig()->tktTransactionInd(); }
  Indicator domesticIntlComb() const { return orig()->domesticIntlComb(); }
  Indicator residualHierarchy() const { return overridingWhenExists()->residualHierarchy(); }
  uint32_t carrierApplTblItemNo() const { return orig()->carrierApplTblItemNo(); }
  uint32_t textTblItemNo() const { return orig()->textTblItemNo(); }
  uint32_t overrideDateTblItemNo() const { return orig()->overrideDateTblItemNo(); }
  bool isAmountOverridden() const { return _conditionallyOverridenBytes.isSet(coByte47_75); }

  const VoluntaryChangesInfo*& orig() { return _orig; }
  const VoluntaryChangesInfo* orig() const { return _orig; }

  const VoluntaryChangesInfo*& overriding() { return _overriding; }
  const VoluntaryChangesInfo* overriding() const { return _overriding; }

  void setConditionallyOverridenBytes(BitsContainer bytes, bool value)
  {
    _conditionallyOverridenBytes.set((ConditionallyOverridenByte)bytes, value);
  }

  const VoluntaryChangesInfo* overridingWhenExists(ConditionallyOverridenByte byte = coNone) const
  {
    return (_overriding && (byte == coNone || _conditionallyOverridenBytes.isSet(byte)))
               ? _overriding
               : _orig;
  }

  bool operator==(const VoluntaryChangesInfoW& rhs) const
  {
    return _orig == rhs._orig && _overriding == rhs._overriding;
  }

protected:
  const VoluntaryChangesInfo* _orig; // for domestic Fc
  const VoluntaryChangesInfo* _overriding; // for overriding international Fc
  ConditionallyOverridenBytesSet _conditionallyOverridenBytes;
};
}

