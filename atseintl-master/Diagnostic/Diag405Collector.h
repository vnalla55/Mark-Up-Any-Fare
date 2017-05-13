
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

#include "Common/TseCodeTypes.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
/**
Diag405Collector is the diagnostic for BookingCodeException data
*/
class Diag405Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag405Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag405Collector() {}

  Diag405Collector& operator<<(const BookingCodeExceptionSequence& bceSequence) override;
};

} // namespace tse

