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

#include "BrandedFares/PbbS8BrandedFaresSelector.h"

#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/MarketResponse.h"
#include "Common/BrandingUtil.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag889Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Rules/RuleUtil.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

namespace tse
{

FALLBACK_DECL(fallbackFixUnbrandedInfFares);

PbbBrandedFaresSelector::PbbBrandedFaresSelector(PricingTrx& trx, const BrandedFareValidatorFactory& brandSelectorFactory)
  : BrandedFaresSelector(trx, brandSelectorFactory)
{
}

void
PbbBrandedFaresSelector::validate(FareMarket& fareMarket)
{
  BrandedFareDiagnostics diagnostics(_trx, _diag889);

  if (!matchFareMarketForDiagParam(fareMarket))
  {
    return;
  }

  diagnostics.printT189Banner();
  if(!preprocessFareMarket(fareMarket, diagnostics))
  {
    markFaresOnMarketAsFailed(fareMarket);
    return;
  }

  if (!diagnostics.isForCarrier(fareMarket.governingCarrier()))
  {
    diagnostics.printCarrierNotMatched(fareMarket);
    return;
  }

  bool hasPrintedPaxTypeFareCommonHeader = false;

  DCFactory* diagFactory = DCFactory::instance();
  Diag894Collector* diag894 = dynamic_cast<Diag894Collector*>(diagFactory->create(_trx));
  if (UNLIKELY(diag894 != nullptr))
    diag894->enable(Diagnostic894);

  std::vector<PaxTypeFare*> allMarketFares;
  if (!fallback::fallbackFixUnbrandedInfFares(&_trx))
  {
    allMarketFares = getAllUniqueFares(fareMarket);
  }
  else
  {
    allMarketFares = fareMarket.allPaxTypeFare();
  }

  for (PaxTypeFare* fare : allMarketFares)
  {
    if (fare->fare() == nullptr)
    {
      LOG4CXX_ERROR(_logger, "S8BrandedFaresSelector::validate FareMarket Fare is NULL");
      continue;
    }

    if (!diagnostics.isForFareClassCode(fare->fareClass()))
     continue;

    if (!fare->isValidForPricing())
    {
      diagnostics.printHeaderInfoSeparator(fare->fareMarket(), hasPrintedPaxTypeFareCommonHeader);
      diagnostics.printPaxTypeFare(fare);
      diagnostics.printFareNotValid();
      markFareAsFailed(*fare);
      continue;
    }

    diagnostics.printHeaderInfoSeparator(fare->fareMarket(), hasPrintedPaxTypeFareCommonHeader);
    displayBrandProgramMapSize(fare, fareMarket, diagnostics);
    bool rc = processPaxTypeFare(fare, diagnostics, diag894);

    //TODO Verify do we need logic for failed soft pass fare in ThruFareMarket
    if( !rc )
      fare->setIsValidForBranding(false);

    diagnostics.printIsValidForBranding(*fare);
  }
  if (UNLIKELY(diag894 != nullptr))
    diag894->flushMsg();
}

void
PbbBrandedFaresSelector::filterBrands(FareMarket& fareMarket) const
{
  if(!fareMarket.hasBrandCode() || fareMarket.brandProgramIndexVec().empty())
    return;

  if (BrandingUtil::isDirectionalityToBeUsed(_trx))
  {
    std::vector<int> validIndices;
    for (int index: fareMarket.brandProgramIndexVec())
    {
      TSE_ASSERT((size_t)index < _trx.brandProgramVec().size());
      const QualifiedBrand& brandProgramPair = _trx.brandProgramVec()[index];
      if (fareMarket.getBrandCode() == brandProgramPair.second->brandCode())
        validIndices.push_back(index);
    }
    fareMarket.brandProgramIndexVec() = validIndices;
  }
  else
  {
    std::vector<int>::const_iterator it =
      std::find_if(fareMarket.brandProgramIndexVec().begin(),
                   fareMarket.brandProgramIndexVec().end(),
                   FilterBrandsPredicate(fareMarket, _trx.brandProgramVec()));

    int foundIndex = (it != fareMarket.brandProgramIndexVec().end()) ? *it : -1;
    fareMarket.brandProgramIndexVec().clear();
    if(foundIndex != -1)
    {
      fareMarket.brandProgramIndexVec().push_back(foundIndex);
    }
  }
}

void
PbbBrandedFaresSelector::markFaresOnMarketAsFailed(const FareMarket& fareMarket) const
{
  std::for_each(fareMarket.allPaxTypeFare().begin(), fareMarket.allPaxTypeFare().end(),
                boost::lambda::bind(&PbbBrandedFaresSelector::markFareAsFailed, this, *boost::lambda::_1));
}

void
PbbBrandedFaresSelector::markFareAsFailed(PaxTypeFare& fare) const
{
  fare.setIsValidForBranding(false);
  uint16_t fmBrandSize = fare.fareMarket()->brandProgramIndexVec().size();
  fare.getMutableBrandStatusVec().assign(fmBrandSize,
      std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
}

bool
PbbBrandedFaresSelector::FilterBrandsPredicate::operator()(std::size_t brandProgramIndex) const
{
  TSE_ASSERT(brandProgramIndex < _brandProgramVector.size());
  const QualifiedBrand& brandProgramPair = _brandProgramVector[brandProgramIndex];
  return _fareMarket.getBrandCode() == brandProgramPair.second->brandCode();
}

bool
PbbBrandedFaresSelector::preprocessFareMarket(FareMarket& fareMarket, BrandedFareDiagnostics& diagnostics)
{
  if (fareMarket.allPaxTypeFare().empty())
  {
    diagnostics.printNoFaresFound(fareMarket);
    return false;
  }

  if (_trx.brandedMarketMap().empty())
  {
    diagnostics.printDataNotFound(fareMarket);
    return false;
  }

  BrandAndProgramValidator marketValidator(_trx.brandedMarketMap());
  if(!marketValidator.validateBrandProgram(fareMarket))
  {
    diagnostics.printNoProgramFound(fareMarket);
    return false;
  }

  filterBrands(fareMarket);
  diagnostics.printBrandFilter(fareMarket, fareMarket.getBrandCode(),
                               !fareMarket.brandProgramIndexVec().empty());

  return !fareMarket.brandProgramIndexVec().empty();
}

namespace {
  class UniqueFares
  {
    std::set<PaxTypeFare*> _unique;
  public:
    std::vector<PaxTypeFare*> fares;

    void insert(PaxTypeFare* ptf)
    {
      if (_unique.insert(ptf).second)
        fares.push_back(ptf);
    }
  };
}

std::vector<PaxTypeFare*>
PbbBrandedFaresSelector::getAllUniqueFares(FareMarket& fareMarket)
{
  UniqueFares result;

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
    result.insert(ptf);

  for (PaxTypeBucket& cortege : fareMarket.paxTypeCortege())
  {
    for (PaxTypeFare* ptf : cortege.paxTypeFare())
      result.insert(ptf);
  }

  return result.fares;
}
} /* namespace tse */
