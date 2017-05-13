// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Diagnostic/DiagCollector.h"

namespace tse
{

class Diag825Collector : public DiagCollector
{
  unsigned _lineNumber = 1;
public:
  Diag825Collector() = default;

  void accept(DiagVisitor& diagVisitor) override
  {
    diagVisitor.visit(*this);
  }

  unsigned& lineNumber()
  {
    return _lineNumber;
  }
};

} // end of tse namespace
