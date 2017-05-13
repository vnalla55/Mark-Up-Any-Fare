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
class Diag259Collector : public ACDiagCollector
{
public:
  explicit Diag259Collector(Diagnostic& root) : ACDiagCollector(root) {}
  Diag259Collector() = default;

  void writeGWPairToReconstruct(const GatewayPair& gw);
  void writeGWPairToReconstructFooter();

private:
  int _numGWPairToReconstruct = 0;
  void writeGWPairToReconstructHeader();
};

} // namespace tse

