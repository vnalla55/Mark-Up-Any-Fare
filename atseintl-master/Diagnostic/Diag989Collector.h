//----------------------------------------------------------------------------
//  File:        Diag989Collector.h
//  Created:     2009-05-22
//
//  Description: Diagnostic 989 formatter
//
//  Updates:
//
//  Copyright Sabre 2009
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
class RexShoppingTrx;

class Diag989Collector : public DiagCollector
{
public:
  explicit Diag989Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag989Collector() {}

  Diag989Collector& operator<<(RexShoppingTrx& trx);

private:
  void printFareComponentInfo(const PaxTypeFare* ptf, RexShoppingTrx& trx);
  void printVCTR(const PaxTypeFare* ptf, const FareMarket* fareMarket);
  void printRecord3Info(const PaxTypeFare* ptf, RexShoppingTrx& trx);
};

} // namespace tse

