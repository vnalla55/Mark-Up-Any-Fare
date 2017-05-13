#pragma once
#include <string>
#include <utility>
#include <vector>

namespace tse
{
class Diag922Collector;
class FareMarket;
struct GovCxrGroupParameters;
class Itin;
class TravelSeg;
class ShoppingTrx;

using SkippedFM = std::pair<const FareMarket*, std::string>;
using SkippedFMVec = std::vector<SkippedFM>;

struct Diag922Data
{
  ShoppingTrx& _trx;
  SkippedFMVec _skippedFM;
  SkippedFMVec _skippedFMByMileage;

  Diag922Data(ShoppingTrx& trx) : _trx(trx) {}
  int totalSkippedFM() const;
  void printSkippedFM(Diag922Collector& dc) const;
  void reportFareMktToDiag(const FareMarket* fareMarket,
                           const std::string& skipReason,
                           const bool skippedByMileage = false);

  void reportFareMktToDiag(const Itin* itin,
                           const std::vector<TravelSeg*>& fmts,
                           int legIdx,
                           const GovCxrGroupParameters& params,
                           const std::string& skipReason);

  void printSkippedFareMarkets();
};
}
