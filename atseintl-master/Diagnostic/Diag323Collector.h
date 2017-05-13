//----------------------------------------------------------------------------
//  File:        Diag323Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 323 formatter
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
class MiscFareTag;
class PaxTypeFare;

class Diag323Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag323Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag323Collector() = default;

  void diag323Collector(const PaxTypeFare& paxTypeFare, const MiscFareTag* mft);

private:
  void writeHeader(const PaxTypeFare& paxTypeFare, const MiscFareTag& mft);
};

} // namespace tse

