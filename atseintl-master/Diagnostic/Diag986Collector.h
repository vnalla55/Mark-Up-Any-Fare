
#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class RexShoppingTrx;

class Diag986Collector : public DiagCollector
{
public:
  explicit Diag986Collector(Diagnostic& root) : DiagCollector(root), _reshopTrx(nullptr) {}
  Diag986Collector() : _reshopTrx(nullptr) {}

  Diag986Collector& operator<<(RexShoppingTrx& trx);
  void printSegments(const std::vector<TravelSeg*>& segments);

private:
  RexShoppingTrx const* _reshopTrx;
};

} // namespace tse

