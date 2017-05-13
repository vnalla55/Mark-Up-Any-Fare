//----------------------------------------------------------------------------
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
class Diag610Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag610Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag610Collector() {}

  virtual void printHeader() override;
  DiagCollector& operator<<(const FarePath& farePath) override;
};

} /* end tse namespace */

