//-------------------------------------------------------------------
//
//  File:        PaxDetail.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Passenger Detail object.
//
//  Updates:
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareCalcDetail.h"
#include "DataModel/SegmentDetail.h"
#include "DataModel/SellingFareData.h"
#include "DataModel/SurchargeDetail.h"
#include "DataModel/TaxDetail.h"

#include <string>

namespace tse
{
class TaxDetail;
class TaxBreakdown;
class CurrencyDetail;
class PlusUpDetail;
class FareCalcDetail;
class SurchargeDetail;
class SegmentDetail;
class SellingFareData;

class PaxDetail
{
public:
  PaxDetail();

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  CurrencyCode& constructionCurrencyCode() { return _constructionCurrencyCode; }
  const CurrencyCode& constructionCurrencyCode() const { return _constructionCurrencyCode; }

  MoneyAmount& constructionTotalAmount() { return _constructionTotalAmount; }
  const MoneyAmount& constructionTotalAmount() const { return _constructionTotalAmount; }

  CurrencyCode& baseCurrencyCode() { return _baseCurrencyCode; }
  const CurrencyCode& baseCurrencyCode() const { return _baseCurrencyCode; }

  MoneyAmount& baseFareAmount() { return _baseFareAmount; }
  const MoneyAmount& baseFareAmount() const { return _baseFareAmount; }

  CurrencyCode& equivalentCurrencyCode() { return _equivalentCurrencyCode; }
  const CurrencyCode& equivalentCurrencyCode() const { return _equivalentCurrencyCode; }

  MoneyAmount& equivalentAmount() { return _equivalentAmount; }
  const MoneyAmount& equivalentAmount() const { return _equivalentAmount; }

  CurrencyCode& currencyCodeMinimum() { return _currencyCodeMinimum; }
  const CurrencyCode& currencyCodeMinimum() const { return _currencyCodeMinimum; }

  MoneyAmount& currencyCodeMinimumAmount() { return _currencyCodeMinimumAmount; }
  const MoneyAmount& currencyCodeMinimumAmount() const { return _currencyCodeMinimumAmount; }

  uint16_t& stopoverCount() { return _stopoverCount; }
  const uint16_t& stopoverCount() const { return _stopoverCount; }

  MoneyAmount& stopoverCharges() { return _stopoverCharges; }
  const MoneyAmount& stopoverCharges() const { return _stopoverCharges; }

  uint16_t& transferCount() { return _transferCount; }
  const uint16_t& transferCount() const { return _transferCount; }

  MoneyAmount& transferCharges() { return _transferCharges; }
  const MoneyAmount& transferCharges() const { return _transferCharges; }

  CurrencyCode& paxFareCurrencyCode() { return _paxFareCurrencyCode; }
  const CurrencyCode& paxFareCurrencyCode() const { return _paxFareCurrencyCode; }

  MoneyAmount& totalPerPassenger() { return _totalPerPassenger; }
  const MoneyAmount& totalPerPassenger() const { return _totalPerPassenger; }

  MoneyAmount& totalTaxes() { return _totalTaxes; }
  const MoneyAmount& totalTaxes() const { return _totalTaxes; }

  MoneyAmount& totalPerPaxPlusImposedSrg() { return _totalPerPaxPlusImposedSrg; }
  const MoneyAmount& totalPerPaxPlusImposedSrg() const { return _totalPerPaxPlusImposedSrg; }

  MoneyAmount& totalTaxesPlusImposedSrg() { return _totalTaxesPlusImposedSrg; }
  const MoneyAmount& totalTaxesPlusImposedSrg() const { return _totalTaxesPlusImposedSrg; }

  uint16_t& commissionPercentage() { return _commissionPercentage; }
  const uint16_t& commissionPercentage() const { return _commissionPercentage; }

  MoneyAmount& commissionAmount() { return _commissionAmount; }
  const MoneyAmount& commissionAmount() const { return _commissionAmount; }

  MoneyAmount& commissionCap() { return _commissionCap; }
  const MoneyAmount& commissionCap() const { return _commissionCap; }

  MoneyAmount& travelAgencyTax() { return _travelAgencyTax; }
  const MoneyAmount& travelAgencyTax() const { return _travelAgencyTax; }

  std::string& fareCalcLine() { return _fareCalcLine; }
  const std::string& fareCalcLine() const { return _fareCalcLine; }

  Indicator& nonRefundable() { return _nonRefundable; }
  const Indicator& nonRefundable() const { return _nonRefundable; }
  const bool isNonRefundable() const { return TypeConvert::pssCharToBool(_nonRefundable); }

  Indicator& negotiatedWithCorp() { return _negotiatedWithCorp; }
  const Indicator& negotiatedWithCorp() const { return _negotiatedWithCorp; }

  Indicator& negotiatedWithoutCorp() { return _negotiatedWithoutCorp; }
  const Indicator& negotiatedWithoutCorp() const { return _negotiatedWithoutCorp; }

  CurrencyCode& localCurrency() { return _localCurrency; }
  const CurrencyCode& localCurrency() const { return _localCurrency; }

  uint16_t& paxFarePassengerNumber() { return _paxFarePassengerNumber; }
  const uint16_t& paxFarePassengerNumber() const { return _paxFarePassengerNumber; }

  std::string& tourCodeDescription() { return _tourCodeDescription; }
  const std::string& tourCodeDescription() const { return _tourCodeDescription; }

  MoneyAmount& netFareAmount() { return _netFareAmount; }
  const MoneyAmount& netFareAmount() const { return _netFareAmount; }

  Indicator& tourIndicator() { return _tourIndicator; }
  const Indicator& tourIndicator() const { return _tourIndicator; }

  std::string& textBox() { return _textBox; }
  const std::string& textBox() const { return _textBox; }

  MoneyAmount& netGross() { return _netGross; }
  const MoneyAmount& netGross() const { return _netGross; }

  uint16_t& cat35CommissionPercent() { return _cat35CommissionPercent; }
  const uint16_t& cat35CommissionPercent() const { return _cat35CommissionPercent; }

  MoneyAmount& cat35CommissionAmount() { return _cat35CommissionAmount; }
  const MoneyAmount& cat35CommissionAmount() const { return _cat35CommissionAmount; }

  std::string& bspMethodType() { return _bspMethodType; }
  const std::string& bspMethodType() const { return _bspMethodType; }

  std::string& cat35Warning() { return _cat35Warning; }
  const std::string& cat35Warning() const { return _cat35Warning; }

  Indicator& cat35Used() { return _cat35Used; }
  const Indicator& cat35Used() const { return _cat35Used; }

  Indicator& overrideCat35() { return _overrideCat35; }
  const Indicator& overrideCat35() const { return _overrideCat35; }

  Indicator& ticketRestricted() { return _ticketRestricted; }
  const Indicator& ticketRestricted() const { return _ticketRestricted; }

  Indicator& paperTicketSurchargeMayApply() { return _paperTicketSurchargeMayApply; }
  const Indicator& paperTicketSurchargeMayApply() const { return _paperTicketSurchargeMayApply; }

  Indicator& paperTicketSurchargeIncluded() { return _paperTicketSurchargeIncluded; }
  const Indicator& paperTicketSurchargeIncluded() const { return _paperTicketSurchargeIncluded; }

  uint16_t& wpnOptionNumber() { return _wpnOptionNumber; }
  const uint16_t& wpnOptionNumber() const { return _wpnOptionNumber; }

  std::string& wpnDetails() { return _wpnDetails; }
  const std::string& wpnDetails() const { return _wpnDetails; }

  std::string& baggageResponse() { return _baggageResponse; }
  const std::string& baggageResponse() const { return _baggageResponse; }

  std::string& vclInfo() { return _vclInfo; }
  const std::string& vclInfo() const { return _vclInfo; }

  std::vector<TaxDetail*>& taxDetails() { return _taxDetails; }
  const std::vector<TaxDetail*>& taxDetails() const { return _taxDetails; }

  std::vector<TaxBreakdown*>& taxBreakdowns() { return _taxBreakdowns; }
  const std::vector<TaxBreakdown*>& taxBreakdowns() const { return _taxBreakdowns; }

  std::vector<FareCalcDetail*>& fareCalcDetails() { return _fareCalcDetails; }
  const std::vector<FareCalcDetail*>& fareCalcDetails() const { return _fareCalcDetails; }

  std::vector<CurrencyDetail*>& fareIATARates() { return _fareIATARates; }
  const std::vector<CurrencyDetail*>& fareIATARates() const { return _fareIATARates; }

  std::vector<CurrencyDetail*>& fareBankerSellRates() { return _fareBankerSellRates; }
  const std::vector<CurrencyDetail*>& fareBankerSellRates() const { return _fareBankerSellRates; }

  std::vector<CurrencyDetail*>& taxBankerSellRates() { return _taxBankerSellRates; }
  const std::vector<CurrencyDetail*>& taxBankerSellRates() const { return _taxBankerSellRates; }

  std::vector<PlusUpDetail*>& plusUpDetails() { return _plusUpDetails; }
  const std::vector<PlusUpDetail*>& plusUpDetails() const { return _plusUpDetails; }

  std::string& accTvlData() { return _accTvlData; }
  const std::string& accTvlData() const { return _accTvlData; }

  Indicator& reqAccTvl() { return _reqAccTvl; }
  const Indicator& reqAccTvl() const { return _reqAccTvl; }

  CurrencyCode& transferPubCurrency() { return _transferSurchCurrencyCode; }
  const CurrencyCode& transferPubCurrency() const { return _transferSurchCurrencyCode; }

  CurrencyCode& stopOverPubCurrency() { return _stopOverSurchCurrencyCode; }
  const CurrencyCode& stopOverPubCurrency() const { return _stopOverSurchCurrencyCode; }

  SellingFareData& sellingFare() { return _sellingFare; }
  const SellingFareData& sellingFare() const { return _sellingFare; }

private:
  MoneyAmount _constructionTotalAmount; // CTA
  MoneyAmount _baseFareAmount; // BFA
  MoneyAmount _equivalentAmount; // EMT
  MoneyAmount _currencyCodeMinimumAmount; // CMA
  MoneyAmount _stopoverCharges; // FSC
  MoneyAmount _totalPerPassenger; // TPP
  MoneyAmount _totalTaxes; // FTT
  MoneyAmount _totalPerPaxPlusImposedSrg; // C56
  MoneyAmount _totalTaxesPlusImposedSrg; // C6L
  MoneyAmount _commissionAmount; // CAM
  MoneyAmount _commissionCap; // CAP
  MoneyAmount _travelAgencyTax; // FTA
  MoneyAmount _netFareAmount; // FNA    -CAT35
  MoneyAmount _netGross; // FNG
  MoneyAmount _cat35CommissionAmount; // FCA
  MoneyAmount _transferCharges; // C70

  CurrencyCode _constructionCurrencyCode; // CCC
  CurrencyCode _baseCurrencyCode; // BCC
  CurrencyCode _equivalentCurrencyCode; // ECC
  CurrencyCode _currencyCodeMinimum; // CCM
  CurrencyCode _localCurrency; // LCM
  CurrencyCode _paxFareCurrencyCode; // FCC
  uint16_t _commissionPercentage; // CPC
  uint16_t _paxFarePassengerNumber; // FPN
  uint16_t _stopoverCount; // FST
  uint16_t _cat35CommissionPercent; // FCP
  uint16_t _wpnOptionNumber;
  uint16_t _transferCount; // C71

  Indicator _nonRefundable; // FNR
  Indicator _negotiatedWithCorp; // NWC
  Indicator _negotiatedWithoutCorp; // NOC
  Indicator _tourIndicator; // FTI
  Indicator _cat35Used; // FCU
  Indicator _overrideCat35; // FOC
  Indicator _ticketRestricted; // FTR
  Indicator _paperTicketSurchargeMayApply; // PSM

  std::string _fareCalcLine; // FCL
  std::string _tourCodeDescription; // FTC
  std::string _textBox; // FTB
  std::string _bspMethodType; // FBM
  std::string _cat35Warning; // FCW
  std::string _wpnDetails;
  std::string _baggageResponse;
  std::string _vclInfo;

  // Tax detail records
  std::vector<TaxDetail*> _taxDetails; // TAX
  std::vector<TaxBreakdown*> _taxBreakdowns; // TBD

  // Conversion rate records
  std::vector<CurrencyDetail*> _fareIATARates; // FIR
  std::vector<CurrencyDetail*> _fareBankerSellRates; // FBR
  std::vector<CurrencyDetail*> _taxBankerSellRates; // TBR

  // Plus Up records
  std::vector<PlusUpDetail*> _plusUpDetails; // PUP

  // Fare Calc detail records
  std::vector<FareCalcDetail*> _fareCalcDetails; // CAL

  // Accompanied Travel restriction
  std::string _accTvlData; // S85
  Indicator _reqAccTvl; // PBS

  Indicator _paperTicketSurchargeIncluded; // PSI

  // Pax detail Information // PXI
  PaxTypeCode _paxType; // PTP

  // Local currency codes for the non-geo specific
  // Transfer and Stopover surcharges
  CurrencyCode _transferSurchCurrencyCode; // C72
  CurrencyCode _stopOverSurchCurrencyCode; // C73

  SellingFareData _sellingFare; // SFD
};

inline PaxDetail::PaxDetail()
  : _constructionTotalAmount(0),
    _baseFareAmount(0),
    _equivalentAmount(0),
    _currencyCodeMinimumAmount(0),
    _stopoverCharges(0),
    _totalPerPassenger(0),
    _totalTaxes(0),
    _totalPerPaxPlusImposedSrg(0),
    _totalTaxesPlusImposedSrg(0),
    _commissionAmount(0),
    _commissionCap(0),
    _travelAgencyTax(0),
    _netFareAmount(0),
    _netGross(0),
    _cat35CommissionAmount(0),
    _transferCharges(0),
    _commissionPercentage(0),
    _paxFarePassengerNumber(0),
    _stopoverCount(0),
    _cat35CommissionPercent(0),
    _wpnOptionNumber(0),
    _transferCount(0),
    _nonRefundable('T'),
    _negotiatedWithCorp('F'),
    _negotiatedWithoutCorp('T'),
    _tourIndicator('F'),
    _cat35Used('F'),
    _overrideCat35('F'),
    _ticketRestricted('F'),
    _paperTicketSurchargeMayApply('F'),
    _reqAccTvl('F'),
    _paperTicketSurchargeIncluded(0)
{
}

typedef std::vector<TaxBreakdown*>::const_iterator TaxBreakdownVecIC;
} // tse namespace
