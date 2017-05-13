//----------------------------------------------------------------------------
//  File:        Diag900Collector.h
//  Created:     2004-07-20
//
//  Description: Diagnostic 900 formatter
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

#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class ShoppingTrx;

class Diag900Collector : public Diag200Collector
{
public:
  explicit Diag900Collector(Diagnostic& root) : Diag200Collector(root), _legIndex(0) {}
  Diag900Collector() : _legIndex(0) {}

  virtual Diag900Collector& operator<<(const ShoppingTrx& trx) override;

private:
  virtual Diag900Collector& operator<<(const ShoppingTrx::SchedulingOption& sop);
  void printHeader() override;
  uint32_t _legIndex;
};

} // namespace tse

