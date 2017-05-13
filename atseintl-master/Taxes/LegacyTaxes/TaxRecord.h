//----------------------------------------------------------------------------
//  File:           TaxRecord.h
//  Description:    TaxRecord header file for ATSE International Project
//  Created:        2/11/2004
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for the Tax Ticketing functionality.
//          Tax Package will build TaxOut Objects that will contain
//          vectors of Tax Record Objects. TaxRecord Objects are a compressed
//          set of  Tax Information set up to fit on a ticket.
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

#ifndef TAX_RECORD_H
#define TAX_RECORD_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/TaxOrderTktIssue.h"
#include "Taxes/Common/AtpcoTaxName.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CountrySettlementPlanInfo;
class PricingTrx;
class PfcItem;
class TaxNation;
class TaxResponse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @class TaxRecOut
// Description:  Handles Building Tax records for ticketing fields.
//
// </PRE>
// ----------------------------------------------------------------------------

class TaxRecord
{
  friend class TaxRecordTest;
  friend class FareCalcCollectorTest;
  friend class PricingResponseFormatterTest;
  friend class LegacyTaxProcessorTest_testProcessAdjustedSellingLevelFarePath_Test;

public:
  static constexpr char YES = 'Y';
  static constexpr char PFC_TYPE = 'X';
  static const std::string TAX_CODE_XF;
  static const std::string TAX_CODE_US1;
  static const std::string TAX_CODE_US2;
  static const std::string PFC;

  TaxRecord();
  virtual ~TaxRecord();

  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------

  TaxRecord(const TaxRecord& ref);
  TaxRecord& operator=(const TaxRecord& ref);

  //-----------------------------------------------------------------------------
  // buildTicketLine will compress TaxItems and PfcItems into Ticket format
  //-----------------------------------------------------------------------------
  void buildTicketLine(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       const CountrySettlementPlanInfo* cspi,
                       bool isFT = false,
                       bool showExemptedTaxes = false,
                       bool isAtpcoProcessing = false);

  void buildTicketLine(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       bool isFT = false,
                       bool showExemptedTaxes = false,
                       bool isAtpcoProcessing = false);

  void addPfcItems(PricingTrx& trx, TaxResponse& taxResponse);

  const char& failCode() const { return _failCode; }
  void setFailCode(char v) { _failCode = v; }

  const bool isTaxFeeExempt() const
  {
    return (_failCode == TaxItem::EXEMPT_ALL_TAXES || _failCode == TaxItem::EXEMPT_SPECIFIED_TAXES);
  }

  const MoneyAmount& getTaxAmount() const { return _taxAmount; }
  void setTaxAmount(const MoneyAmount& amount) { _taxAmount = amount; }

  const MoneyAmount& taxAmountAdjusted() const { return _taxAmountAdjusted; }

  const MoneyAmount& publishedAmount() const { return _publishedAmount; }

  const uint16_t& taxNoDec() const { return _taxNoDec; }
  uint16_t& taxNoDec() { return _taxNoDec; }

  const uint16_t& publishedNoDec() const { return _publishedNoDec; }

  const CurrencyCode& taxCurrencyCode() const { return _taxCurrencyCode; }
  CurrencyCode& taxCurrencyCode() { return _taxCurrencyCode; }

  const CurrencyCode& publishedCurrencyCode() const { return _publishedCurrencyCode; }
  const TaxCode& taxCode() const { return _taxCode; }
  void setTaxCode(const TaxCode& v) { _taxCode = v; }
  const TaxDescription& taxDescription() const { return _taxDescription; }
  void setTaxDescription(const TaxDescription& v) { _taxDescription = v; }
  const NationCode& taxNation() const { return _taxNation; }
  void setTaxNation(const NationCode& v) { _taxNation = v; }
  const TaxTypeCode& taxType() const { return _taxType; }
  void setTaxType(const TaxTypeCode& v) { _taxType = v; }
  const AtpcoTaxName& atpcoTaxName() const { return _atpcoTaxName; }
  void setAtpcoTaxName(const AtpcoTaxName& v) { _atpcoTaxName = v; }
  const char& rollXTNotAllowedInd() const { return _taxRollXTNotAllowedInd; }

  const char& taxRolledXTInd() const { return _taxRolledXTInd; }
  char& taxRolledXTInd() { return _taxRolledXTInd; }

  const bool interlineTaxInd() const { return _interlineTaxInd; }
  const char& multiOccConvRndInd() const { return _multiOccConvRndInd; }
  void setMultiOccConvRndInd(char v) { _multiOccConvRndInd = v; }
  const bool gstTaxInd() const { return _gstTaxInd; }
  const bool& serviceFee() const { return _serviceFee; }
  const uint16_t& taxItemIndex() const { return _taxItemIndex; }
  void setTaxItemIndex(uint16_t v) { _taxItemIndex = v; }

  // WPDF Required Information

  const CarrierCode& carrierCode() const { return _carrierCode; }
  void setCarrierCode(const CarrierCode& v) { _carrierCode = v; }
  const LocCode& localBoard() const { return _localBoard; }
  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrency; }
  const CurrencyNoDec& intermediateNoDec() const { return _intermediateNoDec; }
  const ExchRate& exchangeRate1() const { return _exchangeRate1; }
  const CurrencyNoDec& exchangeRate1NoDec() const { return _exchangeRate1NoDec; }
  const ExchRate& exchangeRate2() const { return _exchangeRate2; }
  const CurrencyNoDec& exchangeRate2NoDec() const { return _exchangeRate2NoDec; }
  const MoneyAmount& intermediateUnroundedAmount() const { return _intermediateUnroundedAmount; }
  const MoneyAmount& intermediateAmount() const { return _intermediateAmount; }
  const int16_t& legId() const { return _legId; }
  void setLegId(int16_t v) { _legId = v; }
  const long& specialProcessNo() const { return _specialProcessNo; }
  const bool& taxOnChangeFee() const { return _taxOnChangeFee; }
  bool& taxOnChangeFee() { return _taxOnChangeFee; }

private:
  char _failCode;
  MoneyAmount _taxAmount;
  MoneyAmount _taxAmountAdjusted;
  MoneyAmount _publishedAmount;
  uint16_t _taxNoDec;
  uint16_t _publishedNoDec;
  CurrencyCode _taxCurrencyCode;
  CurrencyCode _publishedCurrencyCode;
  TaxCode _taxCode;
  TaxDescription _taxDescription;
  NationCode _taxNation;
  TaxTypeCode _taxType;
  AtpcoTaxName _atpcoTaxName;
  char _taxRollXTNotAllowedInd; // If set, this item cannot be
  char _taxRolledXTInd; // This tax rolled into XT
  bool _interlineTaxInd; // Interlinable tax (rev acctg)
  char _multiOccConvRndInd; // Round after all taxes are totalled for specific Tax Type
  bool _gstTaxInd;
  bool _serviceFee;
  uint16_t _taxItemIndex; // Index of the first TaxItem combined into TaxRec
  CarrierCode _carrierCode; // Carrier for WPDF Display
  LocCode _localBoard; // Local board for WPDF display
  CurrencyCode _intermediateCurrency;
  CurrencyNoDec _intermediateNoDec;
  ExchRate _exchangeRate1;
  CurrencyNoDec _exchangeRate1NoDec;
  ExchRate _exchangeRate2;
  CurrencyNoDec _exchangeRate2NoDec;
  MoneyAmount _intermediateUnroundedAmount;
  MoneyAmount _intermediateAmount;
  int16_t _legId;
  long _specialProcessNo;
  bool _taxOnChangeFee;

  static log4cxx::LoggerPtr _logger;

  //-----------------------------------------------------------------------------
  // initialize TaxRecords with consolidated TaxItems.
  //-----------------------------------------------------------------------------

  void initialize(const TaxItem& taxItem);

  //-----------------------------------------------------------------------------
  // initialize TaxRecords wiht PFC items..
  //-----------------------------------------------------------------------------

  void initialize(PricingTrx& trx, const PfcItem& pfcItem);

  //-----------------------------------------------------------------------------
  // adjustTaxRecord will combine taxcode taxAmounts
  //-----------------------------------------------------------------------------

  bool adjust(PricingTrx& trx, const TaxItem& taxItem, TaxResponse& taxResponse, bool isFT);

  //-----------------------------------------------------------------------------
  // sort will place Tax Codes In Request Nation Order
  //-----------------------------------------------------------------------------

  void sortOrigin(PricingTrx& trx, TaxResponse& taxResponse);

  //-----------------------------------------------------------------------------
  // sort will place Tax Codes In Request Nation Order
  //-----------------------------------------------------------------------------

  void sort(PricingTrx& trx, TaxResponse& taxResponse);
  void preSortAtpco(PricingTrx& trx, TaxResponse& taxResponse);

  void sortLegacy(const std::vector<TaxOrderTktIssue>& taxOrderTktIssues, TaxResponse& taxResponse);
  void sortAtpco(const std::vector<TaxOrderTktIssue>& taxOrderTktIssues, TaxResponse& taxResponse);

  //-----------------------------------------------------------------------------
  // round will take specific Tax Types to Round
  //-----------------------------------------------------------------------------

  void round(PricingTrx& trx, TaxResponse& taxResponse);

  //-----------------------------------------------------------------------------
  // ticketBoxCompression will set required Tax Display indicator
  //-----------------------------------------------------------------------------

  void ticketBoxCompression(PricingTrx& trx, TaxResponse& taxResponse);

  //-----------------------------------------------------------------------------
  // fillTaxRecordVector will fill taxRecordVector with data from
  // taxItemVector or changeFeeTaxItemVector
  //-----------------------------------------------------------------------------
  void fillTaxRecordVector(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           bool isFT,
                           bool showExemptedTaxes,
                           bool isChangeFeeVector = false,
                           bool isAtpcoProcessing = false);
};

} // namespace tse

#endif // TAX_RECORD_H
