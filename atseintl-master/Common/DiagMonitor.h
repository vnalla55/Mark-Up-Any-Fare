//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein is the property
//          of Sabre.
//
//          The program(s) may be used and/or copied only with the written
//          permission of Sabre or in accordance with the terms and
//          conditions stipulated in the agreement/contract under which the
//          program(s) have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"

namespace tse
{

class DiagMonitor
{
public:
  DiagMonitor(DiagCollector* diag) : _diag(diag) {}

  DiagMonitor(PricingTrx& trx, DiagnosticTypes diagType);

  DiagMonitor(PricingTrx& trx, DiagnosticTypes diagType1, DiagnosticTypes diagType2);

  DiagMonitor(PricingTrx& trx,
              DiagnosticTypes diagType1,
              DiagnosticTypes diagType2,
              DiagnosticTypes diagType3);

  DiagMonitor(DCFactory* diagFactory, PricingTrx& trx, DiagnosticTypes diagType);

  /**
   * Note: If you need more than three diags enabled, it is better to create
   * the diag yourself, and hook up with the monitor using the first
   * constructor
   */

  ~DiagMonitor();

  bool enabled() const { return (_diag && _diag->isActive()); }

  DiagCollector& diag() { return *_diag; }

  template <class T>
  DiagCollector& operator<<(const T& t)
  {
    if (LIKELY(enabled()))
      *_diag << t;

    return *_diag;
  }

private:
  DiagMonitor(const DiagMonitor&);
  DiagMonitor& operator=(const DiagMonitor&);

private:
  DiagCollector* _diag;
};
} // tse

