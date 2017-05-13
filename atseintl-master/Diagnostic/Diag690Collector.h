//----------------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Fare;
class FarePath;
class PricingUnit;
class ProcessTagPermutation;
class RefundPermutation;

class Diag690Collector : public DiagCollector
{
public:
  static const std::string DISPLAY_SOLUTION;

  virtual void printHeader() override;
  void printSummaryForLowestFarePaths();

  virtual Diag690Collector& operator<<(const char* x) override;
  virtual Diag690Collector& operator<<(const std::string& x) override;
  virtual Diag690Collector& operator<<(std::ostream& (*pf)(std::ostream&)) override;
  virtual Diag690Collector& operator<<(const Fare& fare) override;
  virtual Diag690Collector& operator<<(const FarePath& farePath) override;
  virtual Diag690Collector& operator<<(const std::vector<FarePath*>& fpVect) override;
  virtual Diag690Collector& operator<<(const PricingUnit& pu) override;
  virtual Diag690Collector& operator<<(const FareUsage& fu) override;

  virtual void printLine() override;
  void printRefundFarePath();

protected:
  Indicator formOfRefundInd(const ProcessTagPermutation* permutation);
  std::string residualPenaltyIndicator(Indicator orginal, Indicator actual) const;
  void printRexFarePathHeader(const FarePath& fPath);
  void printRefundPlusUp();
  void printRefundFarePathHeader(const FarePath& fPath);
  void printCommonRefundPermutationInfo(const RefundPermutation& winner);
  void decodeFormOfRefund(const Indicator& formOfRefund);
  void printAmounts(const FareUsage& fu);
  void printPricingUnit(const PricingUnit& pu,
                        CurrencyCode calculationCurrency,
                        bool useInternationalRounding);
  void printFareUsage(const FareUsage& fu,
                      CurrencyCode calculationCurrency,
                      bool useInternationalRounding);

private:
  static bool _filterPassed;

  friend class Diag690CollectorTest;
};

} /* end tse namespace */

