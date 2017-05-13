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

class Diag605Collector : public DiagCollector
{
public:
  virtual void printHeader() override;
  Diag605Collector& operator<<(const PricingUnit& pu) override;

private:
  static const uint8_t DATE_PAIRS_PER_LINE = 3;
};

} /* end tse namespace */
