//----------------------------------------------------------------------------
//  File:        Diag866Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 866 formatter for the ticketing commissions.
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

namespace tse
{
class FarePath;
class CommissionCap;
class Commissions;
class PricingTrx;

class Diag866Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag866Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag866Collector() = default;

  void diag866Request(PricingTrx& trx, FarePath& farePath, const Commissions& comm);
  void diag866Commission(PricingTrx& trx, const FarePath& farePath, const Commissions& comm);
  void diag866DisplayComCap(PricingTrx& trx, const CommissionCap& cCap);
};

} // namespace tse

