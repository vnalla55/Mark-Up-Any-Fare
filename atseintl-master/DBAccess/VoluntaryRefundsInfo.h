//-------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class VoluntaryRefundsInfo : public RuleItemInfo
{
public:
  VoluntaryRefundsInfo()
    : _createDate(0),
      _expireDate(0),
      _validityInd(0),
      _inhibit(0),
      _psgType(),
      _waiverTblItemNo(0),
      _tktValidityInd(0),
      _tktValidityPeriod(),
      _tktValidityUnit(),
      _depOfJourneyInd(0),
      _puInd(0),
      _fareComponentInd(0),
      _advCancelFromTo(0),
      _advCancelLastTod(0),
      _advCancelPeriod(),
      _advCancelUnit(),
      _tktTimeLimit(0),
      _advResValidation(0),
      _fullyFlown(0),
      _origSchedFlt(0),
      _origSchedFltPeriod(),
      _origSchedFltUnit(),
      _cancellationInd(0),
      _fareBreakpoints(0),
      _repriceInd(0),
      _ruleTariff(0),
      _ruleTariffInd(0),
      _rule(),
      _fareClassInd(0),
      _fareClass(),
      _fareTypeTblItemNo(0),
      _sameFareInd(0),
      _nmlSpecialInd(0),
      _owrt(0),
      _fareAmountInd(0),
      _bookingCodeInd(0),
      _penalty1Amt(0.0),
      _penalty1Cur(),
      _penalty1NoDec(0),
      _penalty2Amt(0.0),
      _penalty2Cur(),
      _penalty2NoDec(0),
      _penaltyPercent(0.0),
      _penaltyPercentNoDec(0),
      _highLowInd(0),
      _minimumAmt(0.0),
      _minimumAmtCur(),
      _minimumAmtNoDec(0),
      _reissueFeeInd(0),
      _calcOption(),
      _discountTag1(0),
      _discountTag2(0),
      _discountTag3(0),
      _discountTag4(0),
      _formOfRefund(0),
      _taxNonrefundableInd(0),
      _customer1Period(),
      _customer1Unit(),
      _customer1ResTkt(0),
      _carrierApplTblItemNo(0),
      _unavailTag(0)
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  uint32_t& waiverTblItemNo() { return _waiverTblItemNo; }
  uint32_t waiverTblItemNo() const { return _waiverTblItemNo; }

  Indicator& tktValidityInd() { return _tktValidityInd; }
  Indicator tktValidityInd() const { return _tktValidityInd; }

  ResPeriod& tktValidityPeriod() { return _tktValidityPeriod; }
  const ResPeriod& tktValidityPeriod() const { return _tktValidityPeriod; }

  ResUnit& tktValidityUnit() { return _tktValidityUnit; }
  const ResUnit& tktValidityUnit() const { return _tktValidityUnit; }

  Indicator& depOfJourneyInd() { return _depOfJourneyInd; }
  Indicator depOfJourneyInd() const { return _depOfJourneyInd; }

  Indicator& puInd() { return _puInd; }
  Indicator puInd() const { return _puInd; }

  Indicator& fareComponentInd() { return _fareComponentInd; }
  Indicator fareComponentInd() const { return _fareComponentInd; }

  Indicator& advCancelFromTo() { return _advCancelFromTo; }
  Indicator advCancelFromTo() const { return _advCancelFromTo; }

  int16_t& advCancelLastTod() { return _advCancelLastTod; }
  int16_t advCancelLastTod() const { return _advCancelLastTod; }

  ResPeriod& advCancelPeriod() { return _advCancelPeriod; }
  const ResPeriod& advCancelPeriod() const { return _advCancelPeriod; }

  ResUnit& advCancelUnit() { return _advCancelUnit; }
  const ResUnit& advCancelUnit() const { return _advCancelUnit; }

  Indicator& tktTimeLimit() { return _tktTimeLimit; }
  Indicator tktTimeLimit() const { return _tktTimeLimit; }

  Indicator& advResValidation() { return _advResValidation; }
  Indicator advResValidation() const { return _advResValidation; }

  Indicator& fullyFlown() { return _fullyFlown; }
  Indicator fullyFlown() const { return _fullyFlown; }

  Indicator& origSchedFlt() { return _origSchedFlt; }
  Indicator origSchedFlt() const { return _origSchedFlt; }

  ResPeriod& origSchedFltPeriod() { return _origSchedFltPeriod; }
  const ResPeriod& origSchedFltPeriod() const { return _origSchedFltPeriod; }

  ResUnit& origSchedFltUnit() { return _origSchedFltUnit; }
  const ResUnit& origSchedFltUnit() const { return _origSchedFltUnit; }

  Indicator& cancellationInd() { return _cancellationInd; }
  Indicator cancellationInd() const { return _cancellationInd; }

  Indicator& fareBreakpoints() { return _fareBreakpoints; }
  Indicator fareBreakpoints() const { return _fareBreakpoints; }

  Indicator& repriceInd() { return _repriceInd; }
  Indicator repriceInd() const { return _repriceInd; }

  int16_t& ruleTariff() { return _ruleTariff; }
  int16_t ruleTariff() const { return _ruleTariff; }

  Indicator& ruleTariffInd() { return _ruleTariffInd; }
  Indicator ruleTariffInd() const { return _ruleTariffInd; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  Indicator& fareClassInd() { return _fareClassInd; }
  Indicator fareClassInd() const { return _fareClassInd; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  uint32_t& fareTypeTblItemNo() { return _fareTypeTblItemNo; }
  uint32_t fareTypeTblItemNo() const { return _fareTypeTblItemNo; }

  Indicator& sameFareInd() { return _sameFareInd; }
  Indicator sameFareInd() const { return _sameFareInd; }

  Indicator& nmlSpecialInd() { return _nmlSpecialInd; }
  Indicator nmlSpecialInd() const { return _nmlSpecialInd; }

  Indicator& owrt() { return _owrt; }
  Indicator owrt() const { return _owrt; }

  Indicator& fareAmountInd() { return _fareAmountInd; }
  Indicator fareAmountInd() const { return _fareAmountInd; }

  Indicator& bookingCodeInd() { return _bookingCodeInd; }
  Indicator bookingCodeInd() const { return _bookingCodeInd; }

  MoneyAmount& penalty1Amt() { return _penalty1Amt; }
  const MoneyAmount& penalty1Amt() const { return _penalty1Amt; }

  CurrencyCode& penalty1Cur() { return _penalty1Cur; }
  const CurrencyCode& penalty1Cur() const { return _penalty1Cur; }

  CurrencyNoDec& penalty1NoDec() { return _penalty1NoDec; }
  const CurrencyNoDec& penalty1NoDec() const { return _penalty1NoDec; }

  MoneyAmount& penalty2Amt() { return _penalty2Amt; }
  const MoneyAmount& penalty2Amt() const { return _penalty2Amt; }

  CurrencyCode& penalty2Cur() { return _penalty2Cur; }
  const CurrencyCode& penalty2Cur() const { return _penalty2Cur; }

  CurrencyNoDec& penalty2NoDec() { return _penalty2NoDec; }
  const CurrencyNoDec& penalty2NoDec() const { return _penalty2NoDec; }

  Percent& penaltyPercent() { return _penaltyPercent; }
  const Percent& penaltyPercent() const { return _penaltyPercent; }

  CurrencyNoDec& penaltyPercentNoDec() { return _penaltyPercentNoDec; }
  const CurrencyNoDec& penaltyPercentNoDec() const { return _penaltyPercentNoDec; }

  Indicator& highLowInd() { return _highLowInd; }
  Indicator highLowInd() const { return _highLowInd; }

  MoneyAmount& minimumAmt() { return _minimumAmt; }
  const MoneyAmount& minimumAmt() const { return _minimumAmt; }

  CurrencyCode& minimumAmtCur() { return _minimumAmtCur; }
  const CurrencyCode& minimumAmtCur() const { return _minimumAmtCur; }

  CurrencyNoDec& minimumAmtNoDec() { return _minimumAmtNoDec; }
  const CurrencyNoDec& minimumAmtNoDec() const { return _minimumAmtNoDec; }

  Indicator& reissueFeeInd() { return _reissueFeeInd; }
  Indicator reissueFeeInd() const { return _reissueFeeInd; }

  Indicator& calcOption() { return _calcOption; }
  Indicator calcOption() const { return _calcOption; }

  Indicator& discountTag1() { return _discountTag1; }
  Indicator discountTag1() const { return _discountTag1; }

  Indicator& discountTag2() { return _discountTag2; }
  Indicator discountTag2() const { return _discountTag2; }

  Indicator& discountTag3() { return _discountTag3; }
  Indicator discountTag3() const { return _discountTag3; }

  Indicator& discountTag4() { return _discountTag4; }
  Indicator discountTag4() const { return _discountTag4; }

  Indicator& formOfRefund() { return _formOfRefund; }
  Indicator formOfRefund() const { return _formOfRefund; }

  Indicator& taxNonrefundableInd() { return _taxNonrefundableInd; }
  Indicator taxNonrefundableInd() const { return _taxNonrefundableInd; }

  ResPeriod& customer1Period() { return _customer1Period; }
  const ResPeriod& customer1Period() const { return _customer1Period; }

  ResUnit& customer1Unit() { return _customer1Unit; }
  const ResUnit& customer1Unit() const { return _customer1Unit; }

  Indicator& customer1ResTkt() { return _customer1ResTkt; }
  Indicator customer1ResTkt() const { return _customer1ResTkt; }

  uint32_t& carrierApplTblItemNo() { return _carrierApplTblItemNo; }
  uint32_t carrierApplTblItemNo() const { return _carrierApplTblItemNo; }

  Indicator& unavailTag() { return _unavailTag; }
  Indicator unavailTag() const { return _unavailTag; }

  virtual bool operator==(const VoluntaryRefundsInfo& rhs) const
  {
    return (
        RuleItemInfo::operator==(rhs) && _createDate == rhs._createDate &&
        _expireDate == rhs._expireDate && _validityInd == rhs._validityInd &&
        _inhibit == rhs._inhibit && _psgType == rhs._psgType &&
        _waiverTblItemNo == rhs._waiverTblItemNo && _tktValidityInd == rhs._tktValidityInd &&
        _tktValidityPeriod == rhs._tktValidityPeriod && _tktValidityUnit == rhs._tktValidityUnit &&
        _depOfJourneyInd == rhs._depOfJourneyInd && _puInd == rhs._puInd &&
        _fareComponentInd == rhs._fareComponentInd && _advCancelFromTo == rhs._advCancelFromTo &&
        _advCancelLastTod == rhs._advCancelLastTod && _advCancelPeriod == rhs._advCancelPeriod &&
        _advCancelUnit == rhs._advCancelUnit && _tktTimeLimit == rhs._tktTimeLimit &&
        _advResValidation == rhs._advResValidation && _fullyFlown == rhs._fullyFlown &&
        _origSchedFlt == rhs._origSchedFlt && _origSchedFltPeriod == rhs._origSchedFltPeriod &&
        _origSchedFltUnit == rhs._origSchedFltUnit && _cancellationInd == rhs._cancellationInd &&
        _fareBreakpoints == rhs._fareBreakpoints && _repriceInd == rhs._repriceInd &&
        _ruleTariff == rhs._ruleTariff && _ruleTariffInd == rhs._ruleTariffInd &&
        _rule == rhs._rule && _fareClassInd == rhs._fareClassInd && _fareClass == rhs._fareClass &&
        _fareTypeTblItemNo == rhs._fareTypeTblItemNo && _sameFareInd == rhs._sameFareInd &&
        _nmlSpecialInd == rhs._nmlSpecialInd && _owrt == rhs._owrt &&
        _fareAmountInd == rhs._fareAmountInd && _bookingCodeInd == rhs._bookingCodeInd &&
        _penalty1Amt == rhs._penalty1Amt && _penalty1Cur == rhs._penalty1Cur &&
        _penalty1NoDec == rhs._penalty1NoDec && _penalty2Amt == rhs._penalty2Amt &&
        _penalty2Cur == rhs._penalty2Cur && _penalty2NoDec == rhs._penalty2NoDec &&
        _penaltyPercent == rhs._penaltyPercent &&
        _penaltyPercentNoDec == rhs._penaltyPercentNoDec && _highLowInd == rhs._highLowInd &&
        _minimumAmt == rhs._minimumAmt && _minimumAmtCur == rhs._minimumAmtCur &&
        _minimumAmtNoDec == rhs._minimumAmtNoDec && _reissueFeeInd == rhs._reissueFeeInd &&
        _calcOption == rhs._calcOption && _discountTag1 == rhs._discountTag1 &&
        _discountTag2 == rhs._discountTag2 && _discountTag3 == rhs._discountTag3 &&
        _discountTag4 == rhs._discountTag4 && _formOfRefund == rhs._formOfRefund &&
        _taxNonrefundableInd == rhs._taxNonrefundableInd &&
        _customer1Period == rhs._customer1Period && _customer1Unit == rhs._customer1Unit &&
        _customer1ResTkt == rhs._customer1ResTkt &&
        _carrierApplTblItemNo == rhs._carrierApplTblItemNo && _unavailTag == rhs._unavailTag);
  }

  static void dummyData(VoluntaryRefundsInfo& obj)
  {
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'A';
    obj._inhibit = 'B';
    obj._psgType = "C";
    obj._waiverTblItemNo = 1;
    obj._tktValidityInd = 'D';
    obj._tktValidityPeriod = "E";
    obj._tktValidityUnit = "F";
    obj._depOfJourneyInd = 'G';
    obj._puInd = 'H';
    obj._fareComponentInd = 'I';
    obj._advCancelFromTo = 'J';
    obj._advCancelLastTod = 2;
    obj._advCancelPeriod = "K";
    obj._advCancelUnit = "L";
    obj._tktTimeLimit = 'M';
    obj._advResValidation = 'N';
    obj._fullyFlown = 'O';
    obj._origSchedFlt = 'P';
    obj._origSchedFltPeriod = "R";
    obj._origSchedFltUnit = "S";
    obj._cancellationInd = 'T';
    obj._fareBreakpoints = 'U';
    obj._repriceInd = 'V';
    obj._ruleTariff = 3;
    obj._ruleTariffInd = 'W';
    obj._rule = "X";
    obj._fareClassInd = 'Y';
    obj._fareClass = "Z";
    obj._fareTypeTblItemNo = 4;
    obj._sameFareInd = 'a';
    obj._nmlSpecialInd = 'b';
    obj._owrt = 'c';
    obj._fareAmountInd = 'd';
    obj._bookingCodeInd = 'e';
    obj._penalty1Amt = 5.55;
    obj._penalty1Cur = "f";
    obj._penalty1NoDec = 6;
    obj._penalty2Amt = 7.77;
    obj._penalty2Cur = "g";
    obj._penalty2NoDec = 8;
    obj._penaltyPercent = 9.99;
    obj._penaltyPercentNoDec = 10;
    obj._highLowInd = 'h';
    obj._minimumAmt = 11.11;
    obj._minimumAmtCur = "i";
    obj._minimumAmtNoDec = 12;
    obj._reissueFeeInd = 'j';
    obj._calcOption = 'k';
    obj._discountTag1 = 'l';
    obj._discountTag2 = 'm';
    obj._discountTag3 = 'n';
    obj._discountTag4 = 'o';
    obj._formOfRefund = 'p';
    obj._taxNonrefundableInd = 'q';
    obj._customer1Period = "r";
    obj._customer1Unit = "s";
    obj._customer1ResTkt = 't';
    obj._carrierApplTblItemNo = 13;
    obj._unavailTag = 'u';
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _validityInd;
  Indicator _inhibit;
  PaxTypeCode _psgType;
  uint32_t _waiverTblItemNo;
  Indicator _tktValidityInd;
  ResPeriod _tktValidityPeriod;
  ResUnit _tktValidityUnit;
  Indicator _depOfJourneyInd;
  Indicator _puInd;
  Indicator _fareComponentInd;
  Indicator _advCancelFromTo;
  int16_t _advCancelLastTod;
  ResPeriod _advCancelPeriod;
  ResUnit _advCancelUnit;
  Indicator _tktTimeLimit;
  Indicator _advResValidation;
  Indicator _fullyFlown;
  Indicator _origSchedFlt;
  ResPeriod _origSchedFltPeriod;
  ResUnit _origSchedFltUnit;
  Indicator _cancellationInd;
  Indicator _fareBreakpoints;
  Indicator _repriceInd;
  int16_t _ruleTariff;
  Indicator _ruleTariffInd;
  RuleNumber _rule;
  Indicator _fareClassInd;
  FareClassCode _fareClass;
  uint32_t _fareTypeTblItemNo;
  Indicator _sameFareInd;
  Indicator _nmlSpecialInd;
  Indicator _owrt;
  Indicator _fareAmountInd;
  Indicator _bookingCodeInd;
  MoneyAmount _penalty1Amt;
  CurrencyCode _penalty1Cur;
  CurrencyNoDec _penalty1NoDec;
  MoneyAmount _penalty2Amt;
  CurrencyCode _penalty2Cur;
  CurrencyNoDec _penalty2NoDec;
  Percent _penaltyPercent;
  CurrencyNoDec _penaltyPercentNoDec;
  Indicator _highLowInd;
  MoneyAmount _minimumAmt;
  CurrencyCode _minimumAmtCur;
  CurrencyNoDec _minimumAmtNoDec;
  Indicator _reissueFeeInd;
  Indicator _calcOption;
  Indicator _discountTag1;
  Indicator _discountTag2;
  Indicator _discountTag3;
  Indicator _discountTag4;
  Indicator _formOfRefund;
  Indicator _taxNonrefundableInd;
  ResPeriod _customer1Period;
  ResUnit _customer1Unit;
  Indicator _customer1ResTkt;
  uint32_t _carrierApplTblItemNo;
  Indicator _unavailTag;

  //| LASTMODDATE           | bigint(20)  |
  //| CREATORID             | varchar(8)  |
  //| CREATORBUSINESSUNIT   | varchar(8)  |
  //| HASHCODE              | bigint(20)  |

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _waiverTblItemNo);
    FLATTENIZE(archive, _tktValidityInd);
    FLATTENIZE(archive, _tktValidityPeriod);
    FLATTENIZE(archive, _tktValidityUnit);
    FLATTENIZE(archive, _depOfJourneyInd);
    FLATTENIZE(archive, _puInd);
    FLATTENIZE(archive, _fareComponentInd);
    FLATTENIZE(archive, _advCancelFromTo);
    FLATTENIZE(archive, _advCancelLastTod);
    FLATTENIZE(archive, _advCancelPeriod);
    FLATTENIZE(archive, _advCancelUnit);
    FLATTENIZE(archive, _tktTimeLimit);
    FLATTENIZE(archive, _advResValidation);
    FLATTENIZE(archive, _fullyFlown);
    FLATTENIZE(archive, _origSchedFlt);
    FLATTENIZE(archive, _origSchedFltPeriod);
    FLATTENIZE(archive, _origSchedFltUnit);
    FLATTENIZE(archive, _cancellationInd);
    FLATTENIZE(archive, _fareBreakpoints);
    FLATTENIZE(archive, _repriceInd);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _ruleTariffInd);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _fareClassInd);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _fareTypeTblItemNo);
    FLATTENIZE(archive, _sameFareInd);
    FLATTENIZE(archive, _nmlSpecialInd);
    FLATTENIZE(archive, _owrt);
    FLATTENIZE(archive, _fareAmountInd);
    FLATTENIZE(archive, _bookingCodeInd);
    FLATTENIZE(archive, _penalty1Amt);
    FLATTENIZE(archive, _penalty1Cur);
    FLATTENIZE(archive, _penalty1NoDec);
    FLATTENIZE(archive, _penalty2Amt);
    FLATTENIZE(archive, _penalty2Cur);
    FLATTENIZE(archive, _penalty2NoDec);
    FLATTENIZE(archive, _penaltyPercent);
    FLATTENIZE(archive, _penaltyPercentNoDec);
    FLATTENIZE(archive, _highLowInd);
    FLATTENIZE(archive, _minimumAmt);
    FLATTENIZE(archive, _minimumAmtCur);
    FLATTENIZE(archive, _minimumAmtNoDec);
    FLATTENIZE(archive, _reissueFeeInd);
    FLATTENIZE(archive, _calcOption);
    FLATTENIZE(archive, _discountTag1);
    FLATTENIZE(archive, _discountTag2);
    FLATTENIZE(archive, _discountTag3);
    FLATTENIZE(archive, _discountTag4);
    FLATTENIZE(archive, _formOfRefund);
    FLATTENIZE(archive, _taxNonrefundableInd);
    FLATTENIZE(archive, _customer1Period);
    FLATTENIZE(archive, _customer1Unit);
    FLATTENIZE(archive, _customer1ResTkt);
    FLATTENIZE(archive, _carrierApplTblItemNo);
    FLATTENIZE(archive, _unavailTag);
  }

private:
  VoluntaryRefundsInfo(const VoluntaryRefundsInfo&);
  VoluntaryRefundsInfo& operator=(const VoluntaryRefundsInfo&);
};
}

