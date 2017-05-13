//----------------------------------------------------------------------------
//  File: BrandedFaresParityValidator.h
//
//  Author: Michal Mlynek
//  Created:      07/23/2013
//  Description: Collects brands that can be used for w whole journey ( common to all legs ) and
// removes those that cannot
//             assure coverage on the whole trip.
//  Copyright Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "BrandedFares/BrandInfo.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/IbfDiag901Collector.h"

#include <tr1/array>

namespace tse
{

typedef std::tr1::array<BrandCodeSet, 2> BrandCodeSetArray;

class BrandedFaresParityValidator
{
public:
  BrandedFaresParityValidator(ShoppingTrx& trx);
  void process();

private:
  void getBrandsOnEachLeg(BrandCodeSetVec& brandsPerLeg);

  void removeInvalidBrandsInFaresAndFareMarkets(std::vector<FareMarket*>& allFareMarkets,
                                                const BrandCodeSet& commonBrands);

  void removeInvalidBrands(FareMarket* fm,
                           const BrandCodeSet& commonBrands,
                           std::vector<int>& brandsRemoved);

  void checkIfFareIsStillValidForIbf(PaxTypeFare* fare) const;

  void markMarketsWithoutBrandedFaresAsInvalid(FareMarket* fm) const;
  void extractAndSaveBrandsFromFMs(const std::set<FareMarket*>& fms,
                                   const IndexVector& legs,
                                   BrandCodeSetVec& brandsPerLeg,
                                   std::map<LocCode, BrandCodeSetArray>& brandsPerMiddlePoints,
                                   const LocCode& legOrigin,
                                   const LocCode& legDestination);
  void saveFareMarketsFoundForLater(const std::set<FareMarket*>& fareMarkets);
  bool isThruFareMarket(const FareMarket* fm,
                        const LocCode& legOrigin,
                        const LocCode& legDestination) const;
  BrandCodeSet getIntersectionOfSets(const BrandCodeSetVec& sets) const;
  void saveBrandsToLegs(BrandCodeSetVec& brandsPerLeg,
                        const BrandCodeSet& brandIndicesForCurrentLegs,
                        const IndexVector& legs) const;
  BrandCode& getBrandCodeFromTrx(int index) const;

  ShoppingTrx& _trx;
  std::vector<FareMarket*> _allFareMarkets;
  unsigned int _legCount;
  unsigned int _realLegCount;
  IbfDiag901Collector _dc;

  friend class BrandedFaresParityValidatorTest;
};

} // namespace tse
