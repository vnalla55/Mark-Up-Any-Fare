//----------------------------------------------------------------------------
//  File:        Diag952Collector.h
//  Created:     2010-05-212
//
//  Description: Diagnostic 952 formatter
//
//  Updates:
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

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diag952Collector : public DiagCollector
{
public:
  explicit Diag952Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag952Collector() {}

  virtual Diag952Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

private:
  void printFares(const ShoppingTrx& shoppingTrx);
  void printRoutings(const ShoppingTrx& shoppingTrx);
  void printSingles(const MarketRoutingInfo& boundRoutingInfo);
  void printDoubles(const MarketRoutingInfo& boundRoutingInfo);
};

} // namespace tse

