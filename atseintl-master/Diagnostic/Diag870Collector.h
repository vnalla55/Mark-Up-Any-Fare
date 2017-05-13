//----------------------------------------------------------------------------
//  File:        Diag870Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 870 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2006
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

#include <vector>

namespace tse
{
class FarePath;
class PricingTrx;
class TicketingFeesInfo;

class Diag870Collector : public SvcFeesDiagCollector
{
  friend class Diag870CollectorTest;

public:
  explicit Diag870Collector(Diagnostic& root) : SvcFeesDiagCollector(root) {}
  Diag870Collector() = default;

  virtual Diag870Collector& operator<<(const FarePath& x) override;

  void printSolutionHeader(PricingTrx& trx,
                           const CarrierCode& cxr,
                           const FarePath& farePath,
                           bool international,
                           bool roundTrip,
                           const LocCode& furthestPoint);
  void printS4GeneralInfo(const TicketingFeesInfo* obFee,
                          StatusS4Validation status); // Display S4 record
  void printS4CommonHeader();
  void printFistPartDetailedRequest(const TicketingFeesInfo* feeInfo);
  void printFinalPartDetailedRequest(StatusS4Validation status);
  void printS4NotFound();
  void printCanNotCollect(const StatusS4Validation status);
  void printCxrNotActive(const CarrierCode& cxr);
  void printFopBinNumber(const std::vector<FopBinNumber>& fopBinNumber);
  void printFopBinInfo(const std::vector<FopBinNumber>& fopBinNumber,
                       const std::vector<Indicator>& cardTypes,
                       const CurrencyCode& paymentCurrencyCode,
                       const MoneyAmount& chargeAmount);
  void printCardInfo(const FopBinNumber& fopBin, Indicator cardType);
  void printFopBinNotFound();
  void printOBFeesNotRequested();
  void printAmountFopResidualInd(const CurrencyCode& currency,
                                 const MoneyAmount mAmt,
                                 const bool chargeResInd);

private:
  void displayStatus(StatusS4Validation status);
};

} // namespace tse

