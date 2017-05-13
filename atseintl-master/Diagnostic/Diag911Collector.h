//----------------------------------------------------------------------------
//  File:        Diag911Collector.h
//  Created:     2004-10-29
//
//  Description: Diagnostic 911 formatter
//
//  Updates:
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

#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag906Collector.h"

#include <vector>

namespace tse
{
class DiagCollector;
class FlightFinderTrx;
class FareMarket;
class ShoppingTrx;

class Diag911Collector : public Diag906Collector
{
public:
  explicit Diag911Collector(Diagnostic& root)
    : Diag906Collector(root),
      _firstNonFakedCell(nullptr),
      _flightFinderTrx(nullptr),
      _isSpanishDiscountTrx(false),
      _isBrandedFaresPath(false),
      _isCatchAllBucket(false),
      _isAllPaxTypeFareAsSource(false)
  {
  }

  Diag911Collector()
    : _firstNonFakedCell(nullptr),
      _flightFinderTrx(nullptr),
      _isSpanishDiscountTrx(false),
      _isBrandedFaresPath(false),
      _isCatchAllBucket(false),
      _isAllPaxTypeFareAsSource(false)
  {
  }

  virtual Diag911Collector& operator<<(const class ItinIndex& itinGroup) override;
  virtual Diag911Collector& operator<<(const class ShoppingTrx& shoppingTrx) override;
  virtual Diag911Collector& operator<<(const class FareMarket& fareMarket) override;
  virtual Diag911Collector& operator<<(const class PaxTypeFare& paxFare) override;

private:
  bool printSolFareMarkets(DiagCollector& dc,
                           const ItinIndex::ItinCell* curCell,
                           const Itin* curItin,
                           const uint32_t currentCarrierIndex,
                           const uint32_t totalCarrierCount,
                           const uint32_t carrierKey);

  bool printFareMarkets(DiagCollector& dc,
                        const ItinIndex::ItinCell* curCell,
                        const Itin* curItin,
                        const uint32_t currentCarrierIndex,
                        const uint32_t totalCarrierCount,
                        const uint32_t carrierKey);

  void processPaxTypeFares(DiagCollector& dc,
                           const FareMarket& fareMarket,
                           const std::vector<PaxTypeFare*>& paxTypeFares,
                           bool& hasNormal,
                           bool& hasSpecial,
                           bool& hasTag1,
                           bool& hasTag2,
                           bool& hasTag3);

  void printFlightBitmaps(DiagCollector& dc, const PaxTypeFare& paxFare);
  void printBitmaps(const PaxTypeFare& paxFare);
  void printFFBitmaps(const PaxTypeFare& paxFare);
  void printAltDatesBitmaps(const PaxTypeFare& paxFare);
  void printFlightBitmap(PaxTypeFare::FlightBitmapConstIterator iBegin,
                         PaxTypeFare::FlightBitmapConstIterator iEnd,
                         DiagCollector&);
  void printDatePair(PricingTrx::AltDatePairs::const_iterator iBegin,
                     PricingTrx::AltDatePairs::const_iterator iEnd,
                     DiagCollector&);
  void printAltDatesMainDetails();
  virtual void printHeader(DiagCollector&);

  bool hasAnyValidFlightBitmap(const PaxTypeFare& paxFare);
  bool isFFGsaApplicable();

  const ItinIndex::ItinCell* _firstNonFakedCell; // Cell instance
  const FlightFinderTrx* _flightFinderTrx;
  bool _isSpanishDiscountTrx;
  bool _isBrandedFaresPath;
  bool _isCatchAllBucket;
  bool _isAllPaxTypeFareAsSource;
  std::vector<QualifiedBrand> _brandProgramVec;
};

} // namespace tse

