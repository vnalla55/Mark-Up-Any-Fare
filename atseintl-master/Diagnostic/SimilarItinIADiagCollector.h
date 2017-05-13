//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/DiagCollector.h"

#include <vector>

namespace tse
{
class Itin;
struct SimilarItinData;

class SimilarItinIADiagCollector : public DiagCollector
{
public:
  void printSimilarItinData(const std::vector<Itin*>&);
  virtual DiagCollector& operator<<(const TravelSeg& x) override;

private:
  void printSimilarItinData(const SimilarItinData&);
  void printAvailability(const std::vector<ClassOfService>&);
};
}

