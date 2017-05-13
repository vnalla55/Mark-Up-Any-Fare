//--------------------------------------------------------------
//
//  File:        BrandedFaresUtil.cpp
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

#include "BrandedFares/BrandedFaresUtil.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketResponse.h"
#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/FareMarket.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

namespace tse
{
namespace
{
  class BrandSortPredicate : public std::binary_function<FareMarket*, FareMarket*, bool>
  {
  public:
    BrandSortPredicate(){}
    bool operator() (const FareMarket* fm1, const FareMarket* fm2) const
    {
      TSE_ASSERT(fm1);
      TSE_ASSERT(fm2);
      return (fm1->getBrandCode() < fm2->getBrandCode());
    }
  };


  template<class T>
  class BrandAndProgram : public std::unary_function<const FareMarket*, bool>
  {
  public:
    BrandAndProgram(const PricingTrx::BrandedMarketMap& bmm, const BrandCode& brand) :
                  _brandedMarket(bmm), _brand(brand) {}

    BrandAndProgram(const PricingTrx::BrandedMarketMap& bmm) : _brandedMarket(bmm) {}

    bool operator()(const FareMarket* fm) const
    {
      for (const int& marketId : fm->marketIDVec())
      {
        PricingTrx::BrandedMarketMap::const_iterator i = _brandedMarket.find(marketId);
        if (i == _brandedMarket.end())
          continue;
        const std::vector<MarketResponse*>& mkts = i->second;
        if (boost::algorithm::any_of(mkts.begin(), mkts.end(), T(_brand)))
          return true;
      }
      return false;
    }

  private:
    const PricingTrx::BrandedMarketMap& _brandedMarket;
    BrandCode _brand;
  };


  class ProgramPredicate : public std::unary_function<const MarketResponse*, bool>
  {
  public:
    ProgramPredicate(const BrandCode& brand){}

    bool operator()(const MarketResponse* mkt) const
    {
      TSE_ASSERT(mkt);
      return !mkt->brandPrograms().empty();
    }
  };


  class BrandPredicate : std::unary_function<const MarketResponse*, bool>
  {
  public:
    BrandPredicate(const BrandCode& brand) : _brand(brand) {}

    bool operator()(const MarketResponse* mkt) const
    {
      TSE_ASSERT(mkt);
      const std::vector<BrandProgram*>& brandPrograms = mkt->brandPrograms();
      return boost::algorithm::any_of(brandPrograms.begin(), brandPrograms.end(), *this);
    }

    bool operator()(const BrandProgram* bp) const
    {
      TSE_ASSERT(bp);
      const std::vector<BrandInfo*>& brandsData = bp->brandsData();
      return boost::algorithm::any_of(brandsData.begin(), brandsData.end(), *this);
    }

    bool operator()(const  BrandInfo* bi) const
    {
      TSE_ASSERT(bi);
      return _brand == bi->brandCode();
    }

  private:
    BrandCode _brand;
  };
}


void
BrandAndProgramValidator::validate(const std::vector<FareMarket*>& markets) const
{
  SortedMarket sortedMarket = sortFmByBrands(markets);
  for (const std::vector<FareMarket*>& fms : sortedMarket)
    validateBrandProgram(fms);

  for (const std::vector<FareMarket*>& fms : sortedMarket)
    validateBrandCode(fms);
}

bool
BrandAndProgramValidator::validateBrandProgram(const FareMarket& fm) const
{
  BrandAndProgram<ProgramPredicate> program(_brandedMarket);
  return program(&fm);
}


BrandAndProgramValidator::SortedMarket
BrandAndProgramValidator::sortFmByBrands(const std::vector<FareMarket*>& markets) const
{
  SortedMarket sortedMarket;
  std::vector<FareMarket*> marketsCopy(markets);
  std::sort(marketsCopy.begin(), marketsCopy.end(), BrandSortPredicate());

  FareMarket* previousFm = nullptr;
  for (FareMarket* fm : marketsCopy)
  {
    if (!previousFm || previousFm->getBrandCode() != fm->getBrandCode())
      addToNewGroup(sortedMarket, fm);
    else
      addToCurrentGroup(sortedMarket, fm);

    previousFm = fm;
  }
  return sortedMarket;
}


void BrandAndProgramValidator::addToCurrentGroup(SortedMarket& sortedMarke, FareMarket* fareMarket) const
{
  if (sortedMarke.empty())
    addToNewGroup(sortedMarke, fareMarket);
  else
    sortedMarke.back().push_back(fareMarket);
}


void BrandAndProgramValidator::addToNewGroup(SortedMarket& sortedMarke, FareMarket* fareMarket) const
{
  std::vector<FareMarket*> fms;
  fms.push_back(fareMarket);
  sortedMarke.push_back(fms);
}


void
BrandAndProgramValidator::validateBrandProgram(const std::vector<FareMarket*>& markets) const
{
  if (boost::algorithm::none_of(markets.begin(), markets.end(), BrandAndProgram<ProgramPredicate>(_brandedMarket)))
    throw ErrorResponseException(ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
}


void
BrandAndProgramValidator::validateBrandCode(const std::vector<FareMarket*>& markets) const
{
  if (markets.empty() || markets.front()->travelSeg().empty())
    return;

  static const std::string error = "BRAND CODE INVALID - ";
  BrandCode brand = markets.front()->travelSeg().front()->getBrandCode();

  if (boost::algorithm::none_of(markets.begin(), markets.end(), BrandAndProgram<BrandPredicate>(_brandedMarket, brand)))
    throw ErrorResponseException(ErrorResponseException::NO_VALID_BRAND_FOUND, (error + brand).c_str());
}

} /* namespace tse */
