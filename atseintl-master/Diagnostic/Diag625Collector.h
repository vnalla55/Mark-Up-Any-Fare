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
class Fare;

class Diag625Collector : public DiagCollector
{
public:
  virtual void printHeader() override;
  virtual Diag625Collector& operator<<(const PaxTypeFare& ptf) override;

private:
  bool outputFareInformation(const Fare& fare);
  bool outputFareInformation(const PaxTypeFare& fare);
};

} /* end tse namespace */

