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
class ConstructedFare;

class Diag254Collector : public ACDiagCollector
{
public:
  explicit Diag254Collector(Diagnostic& root) : ACDiagCollector(root), _fareCount(0) {}
  Diag254Collector() : _fareCount(0) {}

  void writeConstructedFare(const ConstructedFare& cf);
  void writeFooter();

private:
  void writeHeader();
  void writeDateIntervals(const AddonFareCortege& afc,
                          const bool isOriginAddon,
                          const ConstructedFare& cf);

  void writeDateIntervals(const ConstructedFare& cf);
  void writeDateInterval(const TSEDateInterval& interval,
                         const Indicator description,
                         const Indicator splittedPart = '.',
                         const Indicator inhibit = INHIBIT_N);

  unsigned int _fareCount;
};

} // namespace tse

