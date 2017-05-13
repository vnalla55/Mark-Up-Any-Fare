//----------------------------------------------------------------------------
//  File:        Diag906Collector.h
//  Created:     2004-08-20
//
//  Description: Diagnostic 906 formatter
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

#include "Diagnostic/Diag903Collector.h"

namespace tse
{

class Diag906Collector : public Diag903Collector
{
public:
  explicit Diag906Collector(Diagnostic& root) : Diag903Collector(root), curItinRow(nullptr) {}
  Diag906Collector() : curItinRow(nullptr) {}

  virtual Diag906Collector& operator<<(const PaxTypeFare& paxFare) override;
  virtual Diag906Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag906Collector& operator<<(const ItinIndex& itinGroup) override;
  virtual Diag906Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

protected:
  const ItinIndex::ItinRow* curItinRow;
};

} // namespace tse

