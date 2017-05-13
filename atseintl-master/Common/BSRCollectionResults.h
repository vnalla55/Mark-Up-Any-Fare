//----------------------------------------------------------------------------
//
//  File: BSRCollectionResults.h
//
//  Created: May 2004
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/CurrencyCollectionResults.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class BSRCollectionResults : public CurrencyCollectionResults
{
public:
  BSRCollectionResults() = default;
  BSRCollectionResults(CarrierCode& carrierCode, PaxTypeCode& paxTypeCode)
    : _carrier(carrierCode), _paxType(paxTypeCode)
  {
  }

  enum ApplicationType
  { TAXES,
    FARES,
    SURCHARGES };
  enum ConversionType
  { NUC,
    BSR };

  CurrencyCode& sourceCurrency() { return _sourceCurrencyCode; }
  const CurrencyCode& sourceCurrency() const { return _sourceCurrencyCode; }

  MoneyAmount& sourceAmount() { return _sourceAmount; }
  const MoneyAmount& sourceAmount() const { return _sourceAmount; }

  CurrencyNoDec& sourceAmountNoDec() { return _sourceAmountNoDec; }
  const CurrencyNoDec& sourceAmountNoDec() const { return _sourceAmountNoDec; }

  CurrencyCode& targetCurrency() { return _targetCurrencyCode; }
  const CurrencyCode& targetCurrency() const { return _targetCurrencyCode; }

  MoneyAmount& convertedAmount() { return _convertedAmount; }
  const MoneyAmount& convertedAmount() const { return _convertedAmount; }

  CurrencyNoDec& convertedAmountNoDec() { return _convertedAmountNoDec; }
  const CurrencyNoDec& convertedAmountNoDec() const { return _convertedAmountNoDec; }

  CurrencyCode& intermediateCurrency() { return _intermediateCurrencyCode; }
  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrencyCode; }

  CurrencyNoDec& intermediateNoDec() { return _intermediateNoDec; }
  const CurrencyNoDec& intermediateNoDec() const { return _intermediateNoDec; }

  ExchRate& exchangeRate1() { return _exchangeRate1; }
  const ExchRate& exchangeRate1() const { return _exchangeRate1; }

  CurrencyNoDec& exchangeRate1NoDec() { return _exchangeRate1NoDec; }
  const CurrencyNoDec& exchangeRate1NoDec() const { return _exchangeRate1NoDec; }

  Indicator& exchangeRateType1() { return _exchangeRateType1; }
  const Indicator& exchangeRateType1() const { return _exchangeRateType1; }

  ExchRate& exchangeRate2() { return _exchangeRate2; }
  const ExchRate& exchangeRate2() const { return _exchangeRate2; }

  CurrencyNoDec& exchangeRate2NoDec() { return _exchangeRate2NoDec; }
  const CurrencyNoDec& exchangeRate2NoDec() const { return _exchangeRate2NoDec; }

  Indicator& exchangeRateType2() { return _exchangeRateType2; }
  const Indicator& exchangeRateType2() const { return _exchangeRateType2; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  ApplicationType& applType() { return _applType; }
  const ApplicationType& applType() const { return _applType; }

  ConversionType& conversionType() { return _conversionType; }
  const ConversionType& conversionType() const { return _conversionType; }

  RoundingRule& roundingRule1() { return _roundingRule1; }
  const RoundingRule& roundingRule1() const { return _roundingRule1; }

  RoundingFactor& roundingFactor1() { return _roundingFactor1; }
  const RoundingFactor& roundingFactor1() const { return _roundingFactor1; }

  RoundingRule& roundingRule2() { return _roundingRule2; }
  const RoundingRule& roundingRule2() const { return _roundingRule2; }

  RoundingFactor& roundingFactor2() { return _roundingFactor2; }
  const RoundingFactor& roundingFactor2() const { return _roundingFactor2; }

  MoneyAmount& targetUnroundedAmount() { return _targetUnroundedAmount; }
  const MoneyAmount& targetUnroundedAmount() const { return _targetUnroundedAmount; }

  MoneyAmount& intermediateUnroundedAmount() { return _intermediateUnroundedAmount; }
  const MoneyAmount& intermediateUnroundedAmount() const { return _intermediateUnroundedAmount; }

  MoneyAmount& intermediateAmount() { return _intermediateAmount; }
  const MoneyAmount& intermediateAmount() const { return _intermediateAmount; }

  CurrencyNoDec& roundingFactorNoDec1() { return _roundingFactorNoDec1; }
  const CurrencyNoDec& roundingFactorNoDec1() const { return _roundingFactorNoDec1; }

  CurrencyNoDec& roundingFactorNoDec2() { return _roundingFactorNoDec2; }
  const CurrencyNoDec& roundingFactorNoDec2() const { return _roundingFactorNoDec2; }

  CurrencyCode& bsrPrimeCurrencyGroup1() { return _bsrPrimeCurrencyGroup1; }
  const CurrencyCode& bsrPrimeCurrencyGroup1() const { return _bsrPrimeCurrencyGroup1; }

  CurrencyCode& bsrPrimeCurrencyGroup2() { return _bsrPrimeCurrencyGroup2; }
  const CurrencyCode& bsrPrimeCurrencyGroup2() const { return _bsrPrimeCurrencyGroup2; }

  CurrencyCode& bsrCurrency1() { return _bsrCurrency1; }
  const CurrencyCode& bsrCurrency1() const { return _bsrCurrency1; }

  CurrencyCode& bsrCurrency2() { return _bsrCurrency2; }
  const CurrencyCode& bsrCurrency2() const { return _bsrCurrency2; }

  ExchRate& taxReciprocalRate1() { return _taxReciprocalRate1; }
  const ExchRate& taxReciprocalRate1() const { return _taxReciprocalRate1; }

  CurrencyNoDec& taxReciprocalRate1NoDec() { return _taxReciprocalRate1NoDec; }
  const CurrencyNoDec& taxReciprocalRate1NoDec() const { return _taxReciprocalRate1NoDec; }

  ExchRate& taxReciprocalRate2() { return _taxReciprocalRate2; }
  const ExchRate& taxReciprocalRate2() const { return _taxReciprocalRate2; }

  CurrencyNoDec& taxReciprocalRate2NoDec() { return _taxReciprocalRate2NoDec; }
  const CurrencyNoDec& taxReciprocalRate2NoDec() const { return _taxReciprocalRate2NoDec; }

  void setEquivCurrencyCode(const CurrencyCode& equivCurCode) { _equivCurrencyCode = equivCurCode; }

  const CurrencyCode& equivCurrencyCode() const { return _equivCurrencyCode; }

private:
  CurrencyCode _sourceCurrencyCode;
  MoneyAmount _sourceAmount = 0;
  CurrencyNoDec _sourceAmountNoDec = 0;
  MoneyAmount _convertedAmount = 0; // final rounded amount
  CurrencyCode _targetCurrencyCode;
  CurrencyNoDec _convertedAmountNoDec = 0;
  CurrencyCode _intermediateCurrencyCode;
  CurrencyNoDec _intermediateNoDec = 0;
  MoneyAmount _intermediateAmount = 0; // rounded
  ExchRate _exchangeRate1 = 0;
  CurrencyNoDec _exchangeRate1NoDec = 0;
  Indicator _exchangeRateType1 = 'B';
  ExchRate _exchangeRate2 = 0;
  CurrencyNoDec _exchangeRate2NoDec = 0;
  Indicator _exchangeRateType2 = 'B';
  CarrierCode _carrier;
  NationCode _nation;
  ApplicationType _applType = ApplicationType::TAXES; // application type Taxes/Surcharges/Fares
  ConversionType _conversionType = ConversionType::BSR; // NUC, or BSR
  PaxTypeCode _paxType;
  RoundingRule _roundingRule1 = RoundingRule::NONE;
  RoundingRule _roundingRule2 = RoundingRule::NONE;
  RoundingFactor _roundingFactor1 = 0;
  RoundingFactor _roundingFactor2 = 0;
  CurrencyNoDec _roundingFactorNoDec1 = 0;
  CurrencyNoDec _roundingFactorNoDec2 = 0;

  MoneyAmount _targetUnroundedAmount = 0;
  MoneyAmount _intermediateUnroundedAmount = 0;
  CurrencyCode _bsrPrimeCurrencyGroup1;
  CurrencyCode _bsrCurrency1;
  CurrencyCode _bsrPrimeCurrencyGroup2;
  CurrencyCode _bsrCurrency2;
  ExchRate _taxReciprocalRate1 = 0; // Reciprocal rate used by taxes in WPDF display
  CurrencyNoDec _taxReciprocalRate1NoDec = 0;
  ExchRate _taxReciprocalRate2 = 0; // Reciprocal rate used by taxes in WPDF display
  CurrencyNoDec _taxReciprocalRate2NoDec = 0;
  CurrencyCode _equivCurrencyCode;
};
} //tse

