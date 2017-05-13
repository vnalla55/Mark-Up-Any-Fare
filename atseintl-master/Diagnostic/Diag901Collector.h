//----------------------------------------------------------------------------
//  File:        Diag901Collector.h
//  Authors:     Adrienne A. Stipe, David White
//  Created:     2004-07-15
//
//  Description: Diagnostic 901 formatter
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
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diag901Collector : public Diag200Collector
{
public:
  explicit Diag901Collector(Diagnostic& root) : Diag200Collector(root) {}
  Diag901Collector() {}

  Diag901Collector& operator<<(const PaxTypeFare& paxTypeFare) override;
  Diag901Collector& operator<<(const FareMarket& fareMarket) override;
  Diag901Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
};

} // namespace tse

