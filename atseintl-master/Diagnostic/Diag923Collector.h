// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/SoloSurcharges.h"

#include <map>
#include <utility>

namespace tse
{
class FareMarket;
class PaxTypeFare;
class TravelSeg;

class Diag923Collector : public DiagCollector
{
public:
  Diag923Collector() : _shoppingTrx(nullptr), _surchargesDetailsMap(nullptr) {}
  explicit Diag923Collector(Diagnostic& root)
    : DiagCollector(root), _shoppingTrx(nullptr), _surchargesDetailsMap(nullptr)
  {
  }

  void setSurchargesDetailsMap(const SoloSurcharges::SurchargesDetailsMap* surchargesDetailsMap);

  Diag923Collector& operator<<(ShoppingTrx&);
  Diag923Collector& operator<<(FareMarket&);
  Diag923Collector& operator<<(const PaxTypeFare&) override;
  Diag923Collector& operator<<(const TravelSeg& travelSeg) override;
  Diag923Collector& operator<<(SoloSurcharges::SurchargesDetail surchargesDetail);

private:
  ShoppingTrx* _shoppingTrx;
  const SoloSurcharges::SurchargesDetailsMap* _surchargesDetailsMap;
};

} // namespace tse

