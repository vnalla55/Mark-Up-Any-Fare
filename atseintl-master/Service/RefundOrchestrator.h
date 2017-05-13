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

#include "Service/TransactionOrchestrator.h"

namespace tse
{
class RefundPricingTrx;
class TransactionOrchestrator;

class TransactionOrchestrator::RefundOrchestrator final
{
  friend class RefundOrchestratorTest;

public:
  RefundOrchestrator(RefundPricingTrx& refundTrx, TransactionOrchestrator& to)
    : _refundTrx(refundTrx), _to(to)
  {
  }

  RefundOrchestrator(const RefundOrchestrator&) = delete;
  RefundOrchestrator& operator=(const RefundOrchestrator&) = delete;

  bool process();

private:
  bool processExcItin(uint64_t services);
  bool processNewItin(uint64_t services);

  bool isEnforceRedirection(const ErrorResponseException& e) const;
  bool noDiagnosticProcess();

  bool internalDiagnostic() const;

  TransactionOrchestrator::DiagnosticQualifier determineDiagnosticQualifier() const;

  RefundPricingTrx& _refundTrx;
  TransactionOrchestrator& _to;
};
} // tse namespace
