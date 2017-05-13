//--------------------------------------------------------------
//
//  File:        BrandedFaresUtil.h
//  Created:     April 2014
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

#include "BrandedFares/MarketResponse.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class FareMarket;

class BrandAndProgramValidator : public std::unary_function<FareMarket*, bool>
{
  friend class BrandedFaresUtilTest;
public:
  BrandAndProgramValidator(const PricingTrx::BrandedMarketMap& brandedMarket) :
                          _brandedMarket(brandedMarket) {}

  void validate(const std::vector<FareMarket*>& markets) const;
  bool validateBrandProgram(const FareMarket& fm) const;

private:
  typedef std::vector<std::vector<FareMarket* > > SortedMarket;
  const PricingTrx::BrandedMarketMap& _brandedMarket;

  void validateBrandProgram(const std::vector<FareMarket*>& markets) const;
  void validateBrandCode(const std::vector<FareMarket*>& fms) const;
  SortedMarket sortFmByBrands(const std::vector<FareMarket*>& markets) const;
  void addToCurrentGroup(SortedMarket& sortedMarke, FareMarket* fareMarket) const;
  void addToNewGroup(SortedMarket& sortedMarke, FareMarket* fareMarket) const;
};


} /* namespace tse */
