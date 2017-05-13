//----------------------------------------------------------------------------
//  File:        Diag535Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 535 formatter
//
//  Updates:
//          date - initials - description.
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

#pragma once

#include "Diagnostic/DiagCollector.h"
#include "Rules/NegotiatedFareRuleUtil.h"

namespace tse
{
class FarePath;
class FareUsage;
class NegFareRest;
class NegFareRestExt;
class NegPaxTypeFareRuleData;
class PaxTypeFare;
class PricingTrx;
class PrintOption;
class ValueCodeAlgorithm;

class Diag535Collector : public DiagCollector
{
public:
  explicit Diag535Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag535Collector() = default;

  void diag535Collector(PricingTrx& trx, const Itin& itin, const FareUsage& fu);
  void diag535Collector(const PaxTypeFare& paxTypeFare);
  void diag535Request(PricingTrx& trx, const FarePath& farePath);
  void diag535Message(PricingTrx& trx, const NegotiatedFareRuleUtil::WARNING_MSG warningMsg);
  void displayHeader(bool farePathScope);
  void displayPaxTypeFare(const PaxTypeFare& paxTypeFare,
                          const NegFareRest* negFareRest,
                          bool shouldDisplayTfdpsc,
                          bool farePathScope,
                          const PricingTrx& trx,
                          const FarePath* fp);
  void displayValidationResult(bool result, const char* warningMsg);

private:
  void displayHeader();
  void displayPaxTypeFare(const PaxTypeFare& paxTypeFare);
  void
  displayCat27TourCode(PricingTrx& trx, const PaxTypeFare* ptf, const NegFareRest* negFareRest);
  void displayCoupons(PricingTrx& trx, const NegFareRest& negFareRest);
  void displayCommission(const NegFareRest* negFareRest);
  void displayTourCode(const PaxTypeFare* ptf, const NegFareRest* negFareRest);
  bool shouldDisplayTFDSPCInd(const NegPaxTypeFareRuleData*& negPaxTypeFare,
                              Indicator tktFareDataInd1) const;
  bool tktFareDataSegExists(const NegPaxTypeFareRuleData*& negPaxTypeFare) const;
  void displayTFDByte101(const NegFareRest* negFareRest);
  void displayFareBasisAmountInd(const NegFareRestExt* negFareRestExt);
  void displayTfdpsc(const NegPaxTypeFareRuleData* negPaxTypeFare);
  void displayMatchedSequencesResult(const FarePath* fp, const PaxTypeFare& ptf);
  void displayMatchedSequencesResult(const FareUsage& fu);
  void displayFareProperties(const FareProperties* fareProperties, bool addSpace);
  void displayValueCodeAlgorithm(const ValueCodeAlgorithm& valCodeAlg, bool addSpace);
  void displayStaticValueCode(const NegFareRestExt* negFareRestExt,
                              const std::string& cat18ValueCode,
                              bool addSpace);
  static std::string getCat18ValueCode(const FarePath* fp, const PaxTypeFare& ptf);
  void displayTourCodeCombInd(const NegFareRestExt* negFareRestExt, bool addSpace);
  void displayPrintOption(const NegPaxTypeFareRuleData* negPaxTypeFare, bool addSpace);

  friend class Diag535CollectorTest;
};

} // namespace tse

