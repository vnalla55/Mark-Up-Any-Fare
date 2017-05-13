//----------------------------------------------------------------------------
//  File:        Diag908Collector.h
//  Created:     2004-08-20
//
//  Description: Diagnostic 908 formatter
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
class Diag908Collector : public Diag903Collector
{
public:
  explicit Diag908Collector(Diagnostic& root) : Diag903Collector(root) {}
  Diag908Collector() {}

  virtual Diag908Collector& operator<<(const ShoppingTrx::Leg& leg);
  virtual Diag908Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
  void printPrimeSeg(uint32_t legId, const TravelSeg* primeSeg, const Indicator& ind);
};

} // namespace tse

