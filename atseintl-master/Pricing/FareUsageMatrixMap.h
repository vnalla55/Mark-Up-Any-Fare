/*---------------------------------------------------------------------------
 *  File:    FareUsageMatrixMap.h
 *  Created: 21 Dec 2011
 *  Authors: Mateusz Juda
 *
 *  Copyright Sabre 2011
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include "Fares/FareValidatorOrchestrator.h"
#include "Pricing/PaxTypeFareBitmapValidator.h"

#include <vector>

namespace tse
{

class ShoppingTrx;
class Itin;
class FarePath;
class PaxTypeFare;
class ShoppingPQ;

// The FareUsageMatrixMap: this contains one FareUsageMatrixMapping
// for each FareUsage. Each Mapping will contain a SopList for each
// bit in the FareUsage's bitmap that passes. The SopList will
// always be 'nLegs' in size, and will contain the SOP numbers that
// that bit maps to.
class FareUsageMatrixMap
{
public:
  // for regular processing with ShoppingPQ
  FareUsageMatrixMap(ShoppingTrx* trx,
                     Itin* journeyItin,
                     FarePath* path,
                     BitmapOpOrderer* bOrder,
                     const DatePair* dates,
                     ShoppingPQ* shoppingPQ);

  void validate(FarePath& path);

  void addMapping(int startIndex, int endIndex);
  void
  addSops(ItinIndex::ItinIndexIterator& itinIt, int startIndex, int endIndex, bool acrossStopOver);
  const std::vector<int>& getDimensions() const;
  void getSops(const std::vector<int>& pos, std::vector<int>& sops) const;
  bool hasSops(const std::vector<int>& sops, std::vector<int>* pos = nullptr) const;
  const DatePair* getDates() const;
  const CarrierCode* carrier() const { return _carrier; }

private:
  typedef std::vector<int> SopList;

  struct FareUsageMatrixMapping
  {
    FareUsageMatrixMapping(int first, int n) : firstLeg(first), nLegs(n) {}
    int firstLeg, nLegs;

    std::vector<SopList> sops;
  };

  bool validate(FareUsage& fareUsage);

  std::vector<FareUsageMatrixMapping> _map;
  mutable std::vector<int> _dim;
  ShoppingTrx* _trx;
  const DatePair* _dates;
  PaxTypeFareBitmapValidator::SkippedBitValidator* _skippedBitValidator;
  bool _considerNonStopsOnly;
  const CarrierCode* _carrier;
};
}

