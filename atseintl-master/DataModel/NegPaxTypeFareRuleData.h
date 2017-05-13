//-------------------------------------------------------------------
//
//  File:        NegPaxTypeFareRuleData.h
//  Created:     September 30, 2004
//  Design:      Nakamon Thamsiriboon/Lipika Bardalai
//  Authors:
//
//  Description: Class wraps PaxTypeFare Rule Creation Records
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

#include "Common/Money.h"
#include "DataModel/PaxTypeFareRuleData.h"

namespace tse
{
class NegFareRestExt;
class NegFareRestExtSeq;
class FareProperties;
class ValueCodeAlgorithm;
class PrintOption;

class NegPaxTypeFareRuleData : public PaxTypeFareRuleData
{
public:
  const MoneyAmount& netAmount() const { return _netAmount; }
  MoneyAmount& netAmount() { return _netAmount; }

  const MoneyAmount& nucNetAmount() const { return _nucNetAmount; }
  MoneyAmount& nucNetAmount() { return _nucNetAmount; }

  const MoneyAmount& wholeSaleNetAmount() const { return _wholeSaleNetAmount; }
  MoneyAmount& wholeSaleNetAmount() { return _wholeSaleNetAmount; }

  const MoneyAmount& wholeSaleNucNetAmount() const { return _wholeSaleNucNetAmount; }
  MoneyAmount& wholeSaleNucNetAmount() { return _wholeSaleNucNetAmount; }

  const Indicator cat35Level() const { return _cat35Level; }
  Indicator& cat35Level() { return _cat35Level; }

  const Indicator tktIndicator() const { return _ticketingIndicator; }
  Indicator& tktIndicator() { return _ticketingIndicator; }

  const CurrencyCode& calculatedNegCurrency() const { return _calculatedNegCurrency; }
  CurrencyCode& calculatedNegCurrency() { return _calculatedNegCurrency; }

  const PseudoCityCode& creatorPCC() const { return _creatorPCC; }
  PseudoCityCode& creatorPCC() { return _creatorPCC; }

  const Indicator calcInd() const { return _calcInd; }
  Indicator& calcInd() { return _calcInd; }

  const Percent& percent() const { return _percent; }
  Percent& percent() { return _percent; }

  const Money& ruleAmt() const { return _ruleAmt; }
  Money& ruleAmt() { return _ruleAmt; }

  const int noDecPercent() const { return _noDecPercent; }
  int& noDecPercent() { return _noDecPercent; }

  const int noDecAmt() const { return _noDecAmt; }
  int& noDecAmt() { return _noDecAmt; }

  const bool axessCat35Fare() const { return _axessCat35Fare; }
  bool& axessCat35Fare() { return _axessCat35Fare; }

  const bool cat25Responsive() const { return _cat25Responsive; }
  bool& cat25Responsive() { return _cat25Responsive; }

  const bool rexCat35FareUsingPrevAgent() const { return _rexCat35FareUsingPrevAgent; }
  bool& rexCat35FareUsingPrevAgent() { return _rexCat35FareUsingPrevAgent; }

  NegFareSecurityInfo* const& securityRec() const { return _secRec; }
  NegFareSecurityInfo*& securityRec() { return _secRec; }

  const NegFareRestExt* negFareRestExt() const { return _negFareRestExt; }
  const NegFareRestExt*& negFareRestExt() { return _negFareRestExt; }

  const std::vector<NegFareRestExtSeq*>& negFareRestExtSeq() const { return _negFareRestExtSeq; }
  std::vector<NegFareRestExtSeq*>& negFareRestExtSeq() { return _negFareRestExtSeq; }

  const FareProperties* fareProperties() const { return _fareProperties; }
  const FareProperties*& fareProperties() { return _fareProperties; }

  const ValueCodeAlgorithm* valueCodeAlgorithm() const { return _valueCodeAlgorithm; }
  const ValueCodeAlgorithm*& valueCodeAlgorithm() { return _valueCodeAlgorithm; }

  const PrintOption* printOption() const { return _printOption; }
  const PrintOption*& printOption() { return _printOption; }

  const bool isT979FareIndInRange() const { return _isT979FareIndInRange; }
  bool& isT979FareIndInRange() { return _isT979FareIndInRange; }

  NegPaxTypeFareRuleData* clone(DataHandle& dataHandle) const override;
  void copyTo(NegPaxTypeFareRuleData& cloneObj) const;

  const uint64_t& fareRetailerRuleId() const { return _frrId; }
  uint64_t& fareRetailerRuleId() { return _frrId; }

  const uint64_t& frrSeqNo() const { return _frrSeqNo; }
  uint64_t& frrSeqNo() { return _frrSeqNo; }

  const Indicator& updateInd() const { return _updateInd; }
  Indicator& updateInd() { return _updateInd; }

  const Indicator& redistributeInd() const { return _redistributeInd; }
  Indicator& redistributeInd() { return _redistributeInd; }

  const Indicator& sellInd() const { return _sellInd; }
  Indicator& sellInd() { return _sellInd; }

  const Indicator& ticketInd() const { return _ticketInd; }
  Indicator& ticketInd() { return _ticketInd; }

  const PseudoCityCode& sourcePseudoCity() const { return _frrSourcePcc; }
  PseudoCityCode& sourcePseudoCity() { return _frrSourcePcc; }

  const CarrierCode& valCarrier() const { return _validatingCxr; }
  CarrierCode& validatingCxr() { return _validatingCxr; }

  const FareRetailerCode& fareRetailerCode() const { return _fareRetailerCode; }
  FareRetailerCode& fareRetailerCode() { return _fareRetailerCode; }

  const NegPaxTypeFareRuleData* toNegPaxTypeFareRuleData() const override { return this; }
  NegPaxTypeFareRuleData* toNegPaxTypeFareRuleData() override { return this; }

private:
  MoneyAmount _netAmount = 0; // Net Fare Amount
  MoneyAmount _nucNetAmount = 0; // NUC Net Fare Amount
  MoneyAmount _wholeSaleNetAmount = 0; // WholeSale Net Fare Amount
  MoneyAmount _wholeSaleNucNetAmount = 0; // WholeSale NUC Net Fare Amount
  Indicator _cat35Level = 0; // Permission to view net level
  Indicator _ticketingIndicator = 0; // permission to ticket
  CurrencyCode _calculatedNegCurrency; // selected Currency for any method to calculate Neg Fare
                                       // amount.
  // for RD rule text
  PseudoCityCode _creatorPCC; // who made MU/REDIST data
  Indicator _calcInd = ' '; // 'S'pec, 'C'alc, 'A'dd, 'M'inus
  Percent _percent = 0;
  Money _ruleAmt = NUC; // S/A/M amount
  int _noDecPercent = 0;
  int _noDecAmt = 0;
  NegFareSecurityInfo* _secRec = nullptr;
  bool _axessCat35Fare = false;
  bool _rexCat35FareUsingPrevAgent = false;
  const NegFareRestExt* _negFareRestExt = nullptr;
  std::vector<NegFareRestExtSeq*> _negFareRestExtSeq;
  const FareProperties* _fareProperties = nullptr;
  const ValueCodeAlgorithm* _valueCodeAlgorithm = nullptr;
  const PrintOption* _printOption = nullptr;
  bool _isT979FareIndInRange = false;
  uint64_t _frrId = 0;
  uint64_t _frrSeqNo = 0;
  PseudoCityCode _frrSourcePcc;
  Indicator _updateInd = ' ';
  Indicator _redistributeInd = ' ';
  Indicator _sellInd = ' ';
  Indicator _ticketInd = ' ';
  CarrierCode _validatingCxr;
  bool _cat25Responsive = false;
  FareRetailerCode _fareRetailerCode;
};

} // tse namespace

