// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "Common/YQYR/ShoppingYQYRCalculator.h"

namespace tse
{
struct ShoppingSurchargesCortegeContext;

namespace YQYR
{
class V2ISYQYRCalculator : public ShoppingYQYRCalculator
{
  friend class V2ISCortegeCalculator;

public:
  V2ISYQYRCalculator(ShoppingTrx& trx) : ShoppingYQYRCalculator(trx) {}

  void init();

private:
  void determineOriginAndFurthestPoint(const Loc*& origin,
                                       const Loc*& furthestPoint,
                                       uint32_t& firstInboundLeg);

  void createBuckets(const Loc* furthestPoint, std::vector<YQYRBucket>& buckets);

  void printDiagHeader(const Loc* furthestPoint);

private:
  const Loc* _furthestPoint;
  uint32_t _firstInboundLeg = 0;
  bool _returnsToOrigin;
  bool _international;
};

class V2ISCortegeCalculator
{
public:
  V2ISCortegeCalculator(V2ISYQYRCalculator& calculator, ShoppingSurchargesCortegeContext& ctx);

  void initializeCortegeCarrierStorage(const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                       const std::vector<PaxTypeFare*>& applicableFares);

  void determineFurthestSeg(const ItinIndex::ItinIndexIterator& cellIterator);

  bool calculateYQYRs(const CarrierCode validatingCarrier,
                      const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                      const PaxTypeFare& fare,
                      FarePath& farePath,
                      YQYRCalculator::YQYRFeesApplicationVec& results);

private:
  ShoppingTrx& trx() const { return _calculator._trx; }
  DiagCollectorShopping* dc() const { return _calculator._dc.get(); }

  YQYRCalculator::Directions determineDirection() const;
  uint32_t determineBucket() const;
  void determineFurthestSegSop(int& sop, bool& destination) const;

private:
  V2ISYQYRCalculator& _calculator;
  FareMarket& _fareMarket;
  PaxTypeBucket& _cortege;
  const uint32_t* const _legsBegin;
  const uint32_t* const _legsEnd;

  YQYRCalculator::FareGeography _geography;
  uint32_t _bucket = 0;
  int _furthestSegSop = -1;
  bool _furthestSegSopDestination = false;
  const bool _aso;
};
}
}
