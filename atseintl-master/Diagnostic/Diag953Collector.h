//----------------------------------------------------------------------------
//  File:        Diag953Collector.h
//  Created:     2008-02-06
//
//  Description: Diagnostic 953 formatter
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Diagnostic/Diag200Collector.h"

namespace tse
{
class ShoppingTrx;
class ItinIndex;
class FareMarket;
class PaxTypeFare;

class Diag953Collector : public Diag200Collector
{
public:
  Diag953Collector() : _legIndex(0), _sopIndex(0), _printRules(false) {}

  virtual Diag953Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
  virtual Diag953Collector& operator<<(const ItinIndex& itinIndex) override;
  virtual Diag953Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag953Collector& operator<<(const PaxTypeFare& paxTypeFare) override;

protected:
  uint32_t _legIndex;
  uint32_t _sopIndex;
  bool _printRules;
};

} // namespace tse

