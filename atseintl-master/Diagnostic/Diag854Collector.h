//----------------------------------------------------------------------------
//  File:        Diag854Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 854 formatter
//
//  Updates:
//          date - initials - description.
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
class FarePath;

class Diag854Collector : public DiagCollector
{
public:
  explicit Diag854Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag854Collector() {}

  virtual Diag854Collector& operator<<(const FarePath& x) override;
};

} // namespace tse

