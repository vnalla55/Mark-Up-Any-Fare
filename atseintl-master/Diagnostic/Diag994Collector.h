//----------------------------------------------------------------------------
//  File:        Diag994Collector.h
//
//
//  Copyright Sabre 2010
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

#include "Common/TseEnums.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Itin;

class Diag994Collector : public DiagCollector
{
public:
  explicit Diag994Collector(Diagnostic& root) : DiagCollector(root), _trx(nullptr) {}
  Diag994Collector() : _trx(nullptr) {}

  void displayItins(PricingTrx& trx, const bool processingSwitched = false);
  void displayItinsDetails(PricingTrx& trx);

private:
  PricingTrx* _trx;
};

} // namespace tse

