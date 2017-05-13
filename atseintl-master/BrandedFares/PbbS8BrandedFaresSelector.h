//-------------------------------------------------------------------
//
//  File:        PbbS8BrandedFaresSelector.cpp
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "BrandedFares/BrandedFaresSelector.h"

namespace tse
{

class PricingTrx;
class FareMarket;
class PaxTypeFare;

class PbbBrandedFaresSelector : public BrandedFaresSelector
{
  friend class PbbBrandedFaresSelectorTest;
  typedef std::vector<std::pair<BrandProgram*, BrandInfo*> > BrandProgramVector;

  class FilterBrandsPredicate
  {
    const FareMarket& _fareMarket;
    const BrandProgramVector& _brandProgramVector;
  public:
    FilterBrandsPredicate(const FareMarket& fareMarket, const BrandProgramVector& brandProgramVector)
    : _fareMarket(fareMarket), _brandProgramVector(brandProgramVector)
    {
    }
    bool operator()(std::size_t brandProgramIndex) const;
  };

public:
  PbbBrandedFaresSelector(PricingTrx& trx, const BrandedFareValidatorFactory& brandSelectorFactory);
  virtual ~PbbBrandedFaresSelector(){}

  virtual void validate(FareMarket& fareMarket) override;

private:
  bool preprocessFareMarket(FareMarket& fareMarket, BrandedFareDiagnostics& diagnostics);
  void filterBrands(FareMarket& fareMarket) const;
  void markFaresOnMarketAsFailed(const FareMarket& fareMarket) const;
  void markFareAsFailed(PaxTypeFare& fare) const;
  std::vector<PaxTypeFare*> getAllUniqueFares(FareMarket& fareMarket);
};


} /* namespace tse */
