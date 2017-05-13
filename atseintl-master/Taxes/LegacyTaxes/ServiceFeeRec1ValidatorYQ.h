// ---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#pragma once

namespace tse
{
class PricingTrx;
class Loc;
class YQYRFees;

namespace YQYR
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @class ServiceFeeRec1Validator
// Description: ATPCO passses Fuel and Insurance Fees that are
//              required to be collected under S1 and S2 record conditions.
//              These surcharges will be validated and applied with this object.
//              The TaxResponse record Tax Items will be updated with the
//              qualifying Fuel and Insurnce Surcharges.
//
// </PRE>
// ----------------------------------------------------------------------------

class ServiceFeeRec1Validator final
{
  friend class TaxYQFareBasisTest;
  friend class ServiceFeeRec1ValidatorTest;

public:
  ServiceFeeRec1Validator() = default;
  ServiceFeeRec1Validator(const ServiceFeeRec1Validator&) = delete;
  ServiceFeeRec1Validator& operator=(const ServiceFeeRec1Validator&) = delete;

  /// Begin From Tax Code Reg
  TaxTypeCode& taxType() { return _taxType; }
  const TaxTypeCode& taxType() const { return _taxType; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  MoneyAmount& taxAmt() { return _taxAmt; }
  const MoneyAmount& taxAmt() const { return _taxAmt; }

  Indicator& feeApplInd() { return _feeApplInd; }
  const Indicator& feeApplInd() const { return _feeApplInd; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  BookingCode& bookingCode3() { return _bookingCode3; }
  const BookingCode& bookingCode3() const { return _bookingCode3; }

  CarrierCode& carrierCode() { return _carrierCode; }
  const CarrierCode& carrierCode() const { return _carrierCode; }

  CurrencyCode& taxCur() { return _taxCur; }
  const CurrencyCode& taxCur() const { return _taxCur; }

  CurrencyNoDec& taxCurNodec() { return _taxCurNodec; }
  const CurrencyNoDec& taxCurNodec() const { return _taxCurNodec; }

  Indicator& itineraryType() { return _itineraryType; }
  const Indicator& itineraryType() const { return _itineraryType; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  RoundingRule& taxcdRoundRule() { return _taxcdRoundRule; }
  const RoundingRule& taxcdRoundRule() const { return _taxcdRoundRule; }

  Indicator& multioccconvrndInd() { return _multioccconvrndInd; }
  const Indicator& multioccconvrndInd() const { return _multioccconvrndInd; }

  CurrencyNoDec& taxcdRoundUnitNodec() { return _taxcdRoundUnitNodec; }
  const CurrencyNoDec& taxcdRoundUnitNodec() const { return _taxcdRoundUnitNodec; }

  RoundingFactor& taxcdRoundUnit() { return _taxcdRoundUnit; }
  const RoundingFactor& taxcdRoundUnit() const { return _taxcdRoundUnit; }

  Indicator& spclTaxRounding() { return _spclTaxRounding; }
  const Indicator& spclTaxRounding() const { return _spclTaxRounding; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& firstTvlDate() { return _firstTvlDate; }
  const DateTime& firstTvlDate() const { return _firstTvlDate; }

  DateTime& lastTvlDate() { return _lastTvlDate; }
  const DateTime& lastTvlDate() const { return _lastTvlDate; }

  Indicator& travelType() { return _travelType; }
  const Indicator& travelType() const { return _travelType; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  CurrencyNoDec& taxNodec() { return _taxNodec; }
  const CurrencyNoDec& taxNodec() const { return _taxNodec; }

  DateTime& modDate() { return _modDate; }
  const DateTime& modDate() const { return _modDate; }

  Indicator& tvlDateasoriginInd() { return _tvlDateasoriginInd; }
  const Indicator& tvlDateasoriginInd() const { return _tvlDateasoriginInd; }

  Indicator& taxIncludedInd() { return _taxIncludedInd; }
  const Indicator& taxIncludedInd() const { return _taxIncludedInd; }

  TaxTypeCode _taxType = '\0';
  int _seqNo = 0;
  MoneyAmount _taxAmt = 0;
  Indicator _feeApplInd = '\0';
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  BookingCode _bookingCode3;
  CarrierCode _carrierCode;
  CurrencyCode _taxCur;
  CurrencyNoDec _taxCurNodec = 0;
  Indicator _itineraryType = '\0';
  NationCode _nation;
  RoundingRule _taxcdRoundRule = RoundingRule::EMPTY;
  Indicator _multioccconvrndInd = '\0';
  RoundingFactor _taxcdRoundUnit = 0.0;
  CurrencyRoundUnit _taxcdRoundUnitNodec = 0;
  Indicator _spclTaxRounding = '\0';
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _firstTvlDate;
  DateTime _lastTvlDate;
  Indicator _travelType = '\0';
  TaxCode _taxCode;
  CurrencyNoDec _taxNodec = 0;
  DateTime _modDate;
  Indicator _tvlDateasoriginInd = '\0';
  Indicator _taxIncludedInd = '\0';

  /// End From Tax Code Reg

  uint16_t& segmentOrderBegin() { return _segmentOrderBegin; }
  const uint16_t& segmentOrderBegin() const { return _segmentOrderBegin; }

  uint16_t& segmentOrderEnd() { return _segmentOrderEnd; }
  const uint16_t& segmentOrderEnd() const { return _segmentOrderEnd; }

  void LoadData(YQYRFees& yqyrFees);

private:
  bool _oneWay = false;

  uint16_t _segmentOrderTurnAround = 0;

  uint16_t _segmentOrderBegin = 0;
  uint16_t _segmentOrderEnd = 0;

  const Loc* _turnAroundPoint = nullptr;
  constexpr static Indicator BLANK = ' ';
};

} // YQYR
} // tse
