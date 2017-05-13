//-----------------------------------------------------------------------------
//
//  File:     Diag430Collector.h
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
/**
Diag430Collector is the diagnostic for Revalidation of BookingCode for Pricing Unit
*/
class Diag430Collector : public DiagCollector
{
public:
  explicit Diag430Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag430Collector() {}

  virtual Diag430Collector& operator<<(const BookingCodeExceptionSequence& bceSequence) override;
};

} // namespace tse

