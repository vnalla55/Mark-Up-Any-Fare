//----------------------------------------------------------------------------
//  File:        Diag912Collector.h
//  Created:     2008-06-05
//
//  Description: Diagnostic 912 formatter
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

#include "Diagnostic/Diag911Collector.h"

namespace tse
{
class Diag912Collector : public Diag911Collector
{
public:
  explicit Diag912Collector(Diagnostic& root) : Diag911Collector(root) {}
  Diag912Collector() {}

private:
  virtual void printHeader(DiagCollector&) override;
};

} // namespace tse

