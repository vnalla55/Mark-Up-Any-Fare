//----------------------------------------------------------------------------
//  File:        Diag904Collector.h
//  Created:     2004-09-02
//
//  Description: Diagnostic 904 formatter
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

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Diagnostic/Diag902Collector.h"

namespace tse
{
class ShoppingTrx;
class ItinIndex;
class FareMarket;
class PaxTypeFare;

class Diag904Collector : public Diag902Collector
{
public:
  explicit Diag904Collector(Diagnostic& root)
    : Diag902Collector(root),
      _legIndex(0),
      _isSpanishDiscountTrx(false),
      _isBrandedFaresPath(false),
      _isCatchAllBucket(false)
  {
  }

  Diag904Collector()
    : _legIndex(0),
      _isSpanishDiscountTrx(false),
      _isBrandedFaresPath(false),
      _isCatchAllBucket(false)
  {
  }

  virtual Diag904Collector& operator<<(const ShoppingTrx& shoppingTrx) override;
  virtual Diag904Collector& operator<<(const ItinIndex& itinIndex) override;
  virtual Diag904Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag904Collector& operator<<(const PaxTypeFare& paxTypeFare) override;

private:
  void outputAltDateStatus(const PaxTypeFare& paxFare);
  uint32_t _legIndex;
  bool _isSpanishDiscountTrx;
  bool _isBrandedFaresPath;
  bool _isCatchAllBucket;
  std::vector<QualifiedBrand> _brandProgramVec;
};

} // namespace tse

