//----------------------------------------------------------------------------
//  File:        Diag270Collector.h
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//  Created:     2004-05-20
//
//  Description: Diagnostic 270 formatter
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diag270Collector : public DiagCollector
{
public:
  explicit Diag270Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag270Collector() {}

  virtual Diag270Collector& operator<<(const FareMarket& fareMarket) override;
};

} // namespace tse

