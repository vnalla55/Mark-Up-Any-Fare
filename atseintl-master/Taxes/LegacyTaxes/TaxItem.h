//----------------------------------------------------------------------------
//  File:           TaxItem.h
//  Description:    TaxItem header file for ATSE International Project
//  Created:        2/11/2004
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for all Tax Build functionality.
//          Tax Package will build TaxItemOut Objects for each Passenger Type
//          /Fare Calculation. The Fare Calculation Configuration and
//          Tax Diagnostics will utilize these objects in a vector in TaxOut.
//
//
//  Updates:
//          2/11/04 - DVD - updated for model changes.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef TAX_ITEM_H
#define TAX_ITEM_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseConsts.h"
#include "Taxes/Common/AtpcoTaxName.h"
#include "Taxes/Common/ReissueTaxInfoBuilder.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/Common/TaxSplitDetails.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingTrx;
class Tax;
class TaxCodeReg;
class FarePath;
class TaxResponse;

namespace YQYR
{
class Tax;
class ServiceFee;
class ServiceFeeRec1Validator;
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @class TaxItemOut
// Description:  Handles creation of all valid tax Items and elements.
//
// </PRE>
// ----------------------------------------------------------------------------

struct TaxItemInfo
{
  int seqNo;
  long specialProcessNo;
  Indicator showseparateInd;
  Indicator multioccconvrndInd;
  Indicator spclTaxRounding;
  Indicator feeApplInd;
  BookingCode bookingCode1;
  BookingCode bookingCode2;
  BookingCode bookingCode3;
  TaxTypeCode taxType;
  Indicator feeInd;
  Indicator taxOnTaxExcl;
  MoneyAmount taxAmt;
  MoneyAmount minTax;
  MoneyAmount maxTax;
  CurrencyCode minMaxTaxCurrency;
  CurrencyNoDec minMaxTaxCurrencyNoDec;
  CurrencyNoDec taxNodec;
  RoundingFactor taxcdRoundUnit;
  CurrencyNoDec taxcdRoundUnitNodec;
  CurrencyNoDec taxCurNodec;
  RoundingRule taxcdRoundRule;
  NationCode taxNation;
  TaxCode taxCode;
  CurrencyCode taxCur;
  TaxSpecConfigName specConfigName;
  std::vector<std::string> taxOnTaxCode;
  AtpcoTaxName atpcoTaxName;

  TaxItemInfo();
  void copyFrom(const TaxCodeReg& tcReg);
  void copyTo(TaxCodeReg& tcReg) const;
};

class TaxItem
{
public:
  static constexpr char YES = 'Y';

  enum FailCodes
  {
    NONE, // 0
    NOT_APPLICAPLE, // 1
    EXEMPT_ALL_TAXES, // 2
    EXEMPT_SPECIFIED_TAXES, // 3
    EXEMPT_SPECIAL_FEE // 4
  };

  TaxItem() = default;
  virtual ~TaxItem() = default;

  //-----------------------------------------------------------------------------
  // buildTaxItem will move items into this Object from TaxCodeData Object
  //-----------------------------------------------------------------------------
  void buildTaxItem(PricingTrx& trx,
      Tax& tax,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      bool isTaxOnChangeFee = false);

  void buildTaxItem(PricingTrx& trx,
                    YQYR::ServiceFee& serviceFee,
                    FarePath& farePath,
                    YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator);

  const char& failCode() const { return _failCode; }
  void setFailCode(char code) { _failCode = code; }
  const bool partialTax() const { return _partialTax; }
  void setPartialTax(bool v) { _partialTax = v; }
  const bool mixedTax() const { return _mixedTax; }
  bool& mixedTax() { return _mixedTax; }

  const MoneyAmount& taxAmount() const { return _taxAmount; }
  MoneyAmount& taxAmount() { return _taxAmount; }

  const MoneyAmount& taxAmountAdjusted() const { return _taxAmountAdjusted; }
  MoneyAmount& taxAmountAdjusted() { return _taxAmountAdjusted; }

  const MoneyAmount& taxExemptAmount() const { return _taxExemptAmount; }
  MoneyAmount& taxExemptAmount() { return _taxExemptAmount; }

  const MoneyAmount& taxableBaseFare() const { return _taxableBaseFare; }
  MoneyAmount& taxableBaseFare() { return _taxableBaseFare; }

  const MoneyAmount& taxableFare() const { return _taxableFare; }
  MoneyAmount& taxableFare() { return _taxableFare; }

  const MoneyAmount& taxablePartialFare() const { return _taxablePartialFare; }
  void setTaxablePartialFare(const MoneyAmount& v) { _taxablePartialFare = v; }

  const MoneyAmount& specialPercentage() const { return _specialPercentage; }

  const CalculationDetails& calculationDetails() const { return _calculationDetails; }
  CalculationDetails& calculationDetails() { return _calculationDetails; }

  const TaxSplitDetails& taxSplitDetails() const { return _taxSplitDetails; }
  TaxSplitDetails& taxSplitDetails() { return _taxSplitDetails; }

  const CurrencyCode& paymentCurrency() const { return _paymentCurrency; }
  void setPaymentCurrency(const CurrencyCode& v) { _paymentCurrency = v; }

  const uint16_t& paymentCurrencyNoDec() const { return _paymentCurrencyNoDec; }
  void setPaymentCurrencyNoDec(uint16_t v) { _paymentCurrencyNoDec = v; }

  const uint16_t& segmentOrderStart() const { return _segmentOrderStart; }
  const uint16_t& segmentOrderEnd() const { return _segmentOrderEnd; }

  const uint16_t& travelSegStartIndex() const { return _travelSegStartIndex; }
  const uint16_t& travelSegEndIndex() const { return _travelSegEndIndex; }
  const uint16_t& travelSegThruEndOrder() const { return _travelSegThruEndOrder; }

  void setTravelSegStartIndex(uint16_t index) { _travelSegStartIndex = index; }
  void setTravelSegEndIndex(uint16_t index) { _travelSegEndIndex = index; }

  void setTravelSegStart(TravelSeg* seg) { _travelSegStart = seg; }
  void setTravelSegEnd(TravelSeg* seg) { _travelSegEnd = seg; }

  const TravelSeg* travelSegStart() const { return _travelSegStart; }
  const TravelSeg* travelSegEnd() const { return _travelSegEnd; }

  const uint32_t& taxMilesLocal() const { return _taxMilesLocal; }
  void setTaxMilesLocal(uint32_t v) { _taxMilesLocal = v; }

  const uint32_t& taxMilesThru() const { return _taxMilesThru; }
  void setTaxMilesThru(uint32_t v) { _taxMilesThru = v; }

  const LocCode& taxLocalBoard() const { return _taxLocalBoard; }
  void setTaxLocalBoard(const LocCode& v) { _taxLocalBoard = v; }

  const LocCode& taxLocalOff() const { return _taxLocalOff; }
  void setTaxLocalOff(const LocCode& v) { _taxLocalOff = v; }

  const LocCode& taxThruBoard() const { return _taxThruBoard; }
  void setTaxThruBoard(const LocCode& v) { _taxThruBoard = v; }

  const LocCode& taxThruOff() const { return _taxThruOff; }
  void setTaxThruOff(const LocCode& v) { _taxThruOff = v; }

  TaxDescription& taxDescription() { return _taxDescription; }
  const TaxDescription& taxDescription() const { return _taxDescription; }

  const TaxOnTaxInfo& taxOnTaxInfo() const { return _taxOnTaxInfo; }
  void setTaxOnTaxInfo(const TaxOnTaxInfo& v) { _taxOnTaxInfo = v; }

  const bool interline() const { return _interline; }

  const bool gstTax() const { return _isGstTax; }
  bool& gstTax() { return _isGstTax; }

  const bool serviceFee() const { return _serviceFee; }
  bool& serviceFee() { return _serviceFee; }

  const CarrierCode& carrierCode() const { return _carrierCode; }
  void setCarrierCode(const CarrierCode& v) { _carrierCode = v; }

  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrency; }
  const CurrencyNoDec& intermediateNoDec() const { return _intermediateNoDec; }
  const ExchRate& exchangeRate1() const { return _exchangeRate1; }
  const CurrencyNoDec& exchangeRate1NoDec() const { return _exchangeRate1NoDec; }
  const ExchRate& exchangeRate2() const { return _exchangeRate2; }
  const CurrencyNoDec& exchangeRate2NoDec() const { return _exchangeRate2NoDec; }
  const MoneyAmount& intermediateUnroundedAmount() const { return _intermediateUnroundedAmount; }
  const MoneyAmount& intermediateAmount() const { return _intermediateAmount; }

  CurrencyCode& intermediateCurrency() { return _intermediateCurrency; }
  CurrencyNoDec& intermediateNoDec() { return _intermediateNoDec; }
  ExchRate& exchangeRate1() { return _exchangeRate1; }
  CurrencyNoDec& exchangeRate1NoDec() { return _exchangeRate1NoDec; }
  ExchRate& exchangeRate2() { return _exchangeRate2; }
  CurrencyNoDec& exchangeRate2NoDec() { return _exchangeRate2NoDec; }
  MoneyAmount& intermediateUnroundedAmount() { return _intermediateUnroundedAmount; }
  MoneyAmount& intermediateAmount() { return _intermediateAmount; }

  const ReissueTaxInfoBuilder::ReissueTaxInfo& reissueTaxInfo() const { return _reissueTaxInfo; }
  ReissueTaxInfoBuilder::ReissueTaxInfo& reissueTaxInfo() { return _reissueTaxInfo; }

  const MoneyAmount& reissueTaxAmount() const { return _reissueTaxInfo.reissueTaxAmount; }
  MoneyAmount& reissueTaxAmount() { return _reissueTaxInfo.reissueTaxAmount; }

  const CurrencyCode& reissueTaxCurrency() const { return _reissueTaxInfo.reissueTaxCurrency; }
  CurrencyCode& reissueTaxCurrency() { return _reissueTaxInfo.reissueTaxCurrency; }

  const bool& taxOnChangeFee() const { return _taxOnChangeFee; }
  bool& taxOnChangeFee() { return _taxOnChangeFee; }

  const bool applyFeeOnTax() const { return _applyFeeOnTax; }
  bool& applyFeeOnTax() { return _applyFeeOnTax; }

  const bool taxIncluded() const { return _taxIncluded; }
  bool& taxIncluded() { return _taxIncluded; }

  const bool& exemptFromAtpco() const { return _exemptFromAtpco; }
  bool& exemptFromAtpco() { return _exemptFromAtpco; }

  const int16_t& legId() const { return _legId; }
  void setLegId(int16_t legId) { _legId = legId; }

  const bool& skipTaxOnTaxIfNoFare() const { return _skipTaxOnTaxIfNoFare; }
  bool& skipTaxOnTaxIfNoFare() { return _skipTaxOnTaxIfNoFare; }

  const bool& requireTaxOnTaxSeqMatch() const { return _requireTaxOnTaxSeqMatch; }
  bool& requireTaxOnTaxSeqMatch() { return _requireTaxOnTaxSeqMatch; }

  const bool& requireTaxOnDomTaxMatch() const { return _requireTaxOnDomTaxMatch; }
  bool& requireTaxOnDomTaxMatch() { return _requireTaxOnDomTaxMatch; }

  MoneyAmount& taxAmt() { return _taxItemInfo.taxAmt; }
  const MoneyAmount& taxAmt() const { return _taxItemInfo.taxAmt; }

  CurrencyNoDec& taxNodec() { return _taxItemInfo.taxNodec; }
  const CurrencyNoDec& taxNodec() const { return _taxItemInfo.taxNodec; }

  CurrencyCode& taxCur() { return _taxItemInfo.taxCur; }
  const CurrencyCode& taxCur() const { return _taxItemInfo.taxCur; }

  TaxCode& taxCode() { return _taxItemInfo.taxCode; }
  const TaxCode& taxCode() const { return _taxItemInfo.taxCode; }

  NationCode& nation() { return _taxItemInfo.taxNation; }
  const NationCode& nation() const { return _taxItemInfo.taxNation; }

  TaxTypeCode& taxType() { return _taxItemInfo.taxType; }
  const TaxTypeCode& taxType() const { return _taxItemInfo.taxType; }

  Indicator& showseparateInd() { return _taxItemInfo.showseparateInd; }
  const Indicator& showseparateInd() const { return _taxItemInfo.showseparateInd; }

  Indicator& multioccconvrndInd() { return _taxItemInfo.multioccconvrndInd; }
  const Indicator& multioccconvrndInd() const { return _taxItemInfo.multioccconvrndInd; }

  long& specialProcessNo() { return _taxItemInfo.specialProcessNo; }
  const long& specialProcessNo() const { return _taxItemInfo.specialProcessNo; }

  Indicator& spclTaxRounding() { return _taxItemInfo.spclTaxRounding; }
  const Indicator& spclTaxRounding() const { return _taxItemInfo.spclTaxRounding; }

  RoundingFactor& taxcdRoundUnit() { return _taxItemInfo.taxcdRoundUnit; }
  const RoundingFactor& taxcdRoundUnit() const { return _taxItemInfo.taxcdRoundUnit; }

  CurrencyNoDec& taxcdRoundUnitNodec() { return _taxItemInfo.taxcdRoundUnitNodec; }
  const CurrencyNoDec& taxcdRoundUnitNodec() const { return _taxItemInfo.taxcdRoundUnitNodec; }

  RoundingRule& taxcdRoundRule() { return _taxItemInfo.taxcdRoundRule; }
  const RoundingRule& taxcdRoundRule() const { return _taxItemInfo.taxcdRoundRule; }

  Indicator& feeApplInd() { return _taxItemInfo.feeApplInd; }
  const Indicator& feeApplInd() const { return _taxItemInfo.feeApplInd; }

  BookingCode& bookingCode1() { return _taxItemInfo.bookingCode1; }
  const BookingCode& bookingCode1() const { return _taxItemInfo.bookingCode1; }

  BookingCode& bookingCode2() { return _taxItemInfo.bookingCode2; }
  const BookingCode& bookingCode2() const { return _taxItemInfo.bookingCode2; }

  BookingCode& bookingCode3() { return _taxItemInfo.bookingCode3; }
  const BookingCode& bookingCode3() const { return _taxItemInfo.bookingCode3; }

  int& seqNo() { return _taxItemInfo.seqNo; }
  const int& seqNo() const { return _taxItemInfo.seqNo; }

  Indicator& feeInd() { return _taxItemInfo.feeInd; }
  const Indicator& feeInd() const { return _taxItemInfo.feeInd; }

  MoneyAmount& minTax() { return _taxItemInfo.minTax; }
  const MoneyAmount& minTax() const { return _taxItemInfo.minTax; }

  MoneyAmount& maxTax() { return _taxItemInfo.maxTax; }
  const MoneyAmount& maxTax() const { return _taxItemInfo.maxTax; }

  CurrencyCode& minMaxTaxCurrency() { return _taxItemInfo.minMaxTaxCurrency; }
  const CurrencyCode& minMaxTaxCurrency() const { return _taxItemInfo.minMaxTaxCurrency; }

  CurrencyNoDec& minMaxTaxCurrencyNoDec() { return _taxItemInfo.minMaxTaxCurrencyNoDec; }
  const CurrencyNoDec& minMaxTaxCurrencyNoDec() const
  {
    return _taxItemInfo.minMaxTaxCurrencyNoDec;
  }

  CurrencyNoDec& taxCurNodec() { return _taxItemInfo.taxCurNodec; }
  const CurrencyNoDec& taxCurNodec() const { return _taxItemInfo.taxCurNodec; }

  TaxSpecConfigName& specConfigName() { return _taxItemInfo.specConfigName; }
  const TaxSpecConfigName& specConfigName() const { return _taxItemInfo.specConfigName; }

  std::vector<std::string>& taxOnTaxCode() { return _taxItemInfo.taxOnTaxCode; }
  const std::vector<std::string>& taxOnTaxCode() const { return _taxItemInfo.taxOnTaxCode; }

  Indicator& taxOnTaxExcl() { return _taxItemInfo.taxOnTaxExcl; }
  const Indicator& taxOnTaxExcl() const { return _taxItemInfo.taxOnTaxExcl; }

  AtpcoTaxName& atpcoTaxName() { return _taxItemInfo.atpcoTaxName; }
  const AtpcoTaxName& atpcoTaxName() const { return _taxItemInfo.atpcoTaxName; }

  std::vector<TaxItem*>& taxOnTaxItems() { return _taxOnTaxItems; }
  const std::vector<TaxItem*>& taxOnTaxItems() const { return _taxOnTaxItems; }

  void applyAutomaticPfcTaxExemption(PricingTrx& trx, TaxResponse& taxResponse);

  const TaxItemInfo& taxItemInfo() const { return _taxItemInfo; };

  MoneyAmount getFareSumAmount() const;

  bool useTaxableTaxSumAmount() const;

  bool isCalculationDetailsSet() const;

  Indicator getRefundableTaxTag() const { return _refundableTaxTag; }
  void setRefundableTaxTag(const Indicator& ind) { _refundableTaxTag = ind; }

protected:
  TaxItemInfo _taxItemInfo;

  char _failCode{0};
  bool _partialTax{false};
  bool _mixedTax{false};

  MoneyAmount _taxAmount{0};
  MoneyAmount _taxAmountAdjusted{0};
  MoneyAmount _taxExemptAmount{0};
  MoneyAmount _taxableBaseFare{0};
  MoneyAmount _taxableFare{0};
  MoneyAmount _taxablePartialFare{0};
  MoneyAmount _specialPercentage{0};

  CalculationDetails _calculationDetails;
  TaxSplitDetails _taxSplitDetails;

  CurrencyCode _paymentCurrency{""};
  uint16_t _paymentCurrencyNoDec{0};

  uint16_t _segmentOrderStart{0};
  uint16_t _segmentOrderEnd{0};

  uint16_t _travelSegStartIndex{0};
  uint16_t _travelSegEndIndex{0};
  uint16_t _travelSegThruEndOrder{0};

  TravelSeg* _travelSegStart{nullptr};
  TravelSeg* _travelSegEnd{nullptr};

  uint32_t _taxMilesLocal{0};
  uint32_t _taxMilesThru{0};

  LocCode _taxLocalBoard{""};
  LocCode _taxLocalOff{""};
  LocCode _taxThruBoard{""};
  LocCode _taxThruOff{""};

  TaxDescription _taxDescription{""};
  TaxOnTaxInfo _taxOnTaxInfo{""};

  bool _taxRecProcessed{false};
  bool _interline{false};
  bool _isGstTax{false};
  bool _serviceFee{false};
  bool _exemptFromAtpco{false};

  CarrierCode _carrierCode{""};
  CurrencyCode _intermediateCurrency{""};
  CurrencyNoDec _intermediateNoDec{0};
  ExchRate _exchangeRate1{0};
  CurrencyNoDec _exchangeRate1NoDec{0};
  ExchRate _exchangeRate2{0};
  CurrencyNoDec _exchangeRate2NoDec{0};
  MoneyAmount _intermediateUnroundedAmount{0};
  MoneyAmount _intermediateAmount{0};

  ReissueTaxInfoBuilder::ReissueTaxInfo _reissueTaxInfo{};

  bool _applyFeeOnTax{true};
  bool _taxIncluded{false};
  int16_t _legId{-1};
  bool _taxOnChangeFee{false};

  bool _skipTaxOnTaxIfNoFare{false};
  bool _requireTaxOnTaxSeqMatch{false};
  bool _requireTaxOnDomTaxMatch{false};
  std::vector<TaxItem*> _taxOnTaxItems;

  Indicator _refundableTaxTag{'Y'};

private:
  static log4cxx::LoggerPtr _logger;

  friend class PricingUnitFactoryTest;
  friend class LegacyTaxProcessorTest_testProcessAdjustedSellingLevelFarePath_Test;
};
} // namespace tse
#endif // INCLUDE_TAX_ITEM_H
