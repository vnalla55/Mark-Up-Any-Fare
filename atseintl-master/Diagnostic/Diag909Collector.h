//----------------------------------------------------------------------------
//  File:        Diag909Collector.h
//  Created:     2004-09-26
//
//  Description: Diagnostic 909 : Routing validation against flightBitmap
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

#include "DataModel/ItinIndex.h"
#include "Diagnostic/Diag906Collector.h"

namespace tse
{
class PaxTypeFare;
class FareMarket;
class ShoppingTrx;

class Diag909Collector : public Diag906Collector
{
public:
  explicit Diag909Collector(Diagnostic& root) : Diag906Collector(root), _currentItinRow(nullptr) {}
  Diag909Collector() : _currentItinRow(nullptr) {}

  virtual Diag909Collector& operator<<(const PaxTypeFare& paxFare) override;
  virtual Diag909Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag909Collector& operator<<(const ItinIndex& itinGroup) override;
  virtual Diag909Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

private:
  const ItinIndex::ItinRow* _currentItinRow;
};

} // namespace tse

