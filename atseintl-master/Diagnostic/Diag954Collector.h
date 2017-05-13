//----------------------------------------------------------------------------
//  File:        Diag954Collector.h
//  Created:     2008-01-29
//
//  Description: Diagnostic 954 formatter
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

#include "Diagnostic/Diag953Collector.h"

namespace tse
{
class ShoppingTrx;
class PaxTypeFare;

class Diag954Collector : public Diag953Collector
{
public:
  virtual Diag954Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
  virtual Diag954Collector& operator<<(const ShoppingTrx::SchedulingOption& schedulingOption);
  virtual Diag954Collector& operator<<(const PaxTypeFare& paxTypeFare) override;

  void printSopFarePath(const SOPFarePath* sopFarePath, std::string pathType);
  void printSelectedBookingCodes(const PaxTypeFare* paxTypeFare);
};

} // namespace tse

