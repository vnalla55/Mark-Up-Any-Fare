//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Diagnostic/ACDiagCollector.h"

namespace tse
{
class ConstructedFareInfo;

class Diag257Collector : public ACDiagCollector
{
public:
  enum DupRemoveReason
  {
    DRR_SINGLE_OVER_DBL,
    DRR_FARE_CLASS_PRIORITY,
    DRR_LESS_FARE_AMOUNT,
    DRR_LESS_OR_EQ_FARE_AMOUNT
  };

  explicit Diag257Collector(Diagnostic& root) : ACDiagCollector(root) {}
  Diag257Collector() : ACDiagCollector() {}

  void writeDupsRemovalHeader();
  void writeDupsRemovalFooter();
  void writeNotEffective(const ConstructedFareInfo& cfi);
  void writeDupDetail(const ConstructedFareInfo& cfi1,
                      const ConstructedFareInfo& cfi2,
                      const DupRemoveReason reason);
};

} // namespace tse

