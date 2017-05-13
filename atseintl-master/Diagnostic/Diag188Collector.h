//----------------------------------------------------------------------------
//
//  Description: Diagnostic 188 formatter
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
class PricingTrx;
class RexPricingTrx;
class TravelSeg;

class Diag188Collector : public DiagCollector
{
public:
  friend class Diag188CollectorTest;

  explicit Diag188Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag188Collector() {}

  Diag188Collector& operator<<(const ExchangePricingTrx& excTrx);
  Diag188Collector& operator<<(const PricingTrx& prcTrx) override;
  Diag188Collector& operator<<(const RefundPricingTrx& refTrx);
  Diag188Collector& operator<<(const RexPricingTrx& rexTrx);

private:
  void printArunk(const TravelSeg* travelSeg);
  void printFareMarket(const FareMarket& fm);
  void printFareMarketHeader(uint16_t fmNumber);
  void printFareMarkets(const PricingTrx& prcTrx);
  void printFooter();
  void printHeader() override;
  void printSideTrips(const FareMarket& fm);
  void printTpmMileage(const FareMarket& fm);
  void printTravelSegments(const std::vector<TravelSeg*> tvlSeg);
  void printTravelSegment(const TravelSeg* tvlSeg);
  void printTpmSeg(const AirSeg* airSeg, uint32_t tpmSeg, uint32_t tpmTotal);
  void printTpmSegs(const std::vector<TravelSeg*> tvlSeg);
};

} // namespace tse

