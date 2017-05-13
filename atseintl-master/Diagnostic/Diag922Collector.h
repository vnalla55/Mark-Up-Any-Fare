#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class ShoppingTrx;
class FareMarket;
class Itin;
struct SOPUsage;

class Diag922Collector : public DiagCollector
{
public:
  explicit Diag922Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag922Collector() {}

  void printFareMarketHeader(const ShoppingTrx& trx);
  void printFareMarket(const ShoppingTrx& trx, const FareMarket& fareMarket);
  static void
  printFareMarket(std::ostream& out, const ShoppingTrx& trx, const FareMarket& fareMarket);
  void printFareMarketMultiAirPort(const FareMarket& fareMarket);
  void printSchedules(ShoppingTrx& trx, int sopIndex, const Itin& itin, const SOPUsage& usage);
};

} // namespace tse
