//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
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

#include "Common/TNBrands/TrxGeometryCalculator.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/IAIbfUtils.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"


#include <vector>

namespace tse
{

FALLBACK_DECL(fallbackSoldoutOriginRT)
FALLBACK_DECL(fallbackFixCrossBrandWithoutParity)

namespace skipper
{

namespace {
  bool isCrossBrandWithoutParity(const PricingTrx& trx)
  {
    return (!fallback::fallbackFixCrossBrandWithoutParity(&trx) &&
            (trx.getRequest()->isCheapestWithLegParityPath() &&
             trx.getRequest()->isProcessParityBrandsOverride()));
  }
}

void TrxGeometryCalculator::getBrandsAndPrograms(
    const FareMarket& fm, BrandProgramRelations& relations) const
{
  const std::vector<QualifiedBrand>& qualifiedBrands = _trx.brandProgramVec();

  for (int index : fm.brandProgramIndexVec())
  {
    TSE_ASSERT(index < static_cast<int>(qualifiedBrands.size()));
    TSE_ASSERT(index >= 0);
    TSE_ASSERT(qualifiedBrands[index].first != nullptr);
    TSE_ASSERT(qualifiedBrands[index].second != nullptr);
    relations.addBrandProgramPair(
        qualifiedBrands[index].second->brandCode(),
        qualifiedBrands[index].first->programID(),
        index);
  }
}


// TODO (andrzej.fediuk) clean up / decouple the code; unit test missing
bool TrxGeometryCalculator::removeBrandsNotMatchingParity(
    BrandCodesPerFareMarket& fareMarketsParityBrands,
    Diag892Collector* diag892)
{
  bool fmWithBrandsFound = false;
  bool isCatchAllBucket = _trx.getRequest()->isCatchAllBucketRequest();
  std::map<FareMarket*, bool> isFMThru;

  if (diag892 && diag892->isDDPARITY())
    diag892->printBrandsPerFareMarket(fareMarketsParityBrands);

  for (Itin* itin: _trx.itin())
  {
    for (FareMarket* fm: itin->fareMarket())
    {
      TSE_ASSERT(fm != nullptr);
      if (isFMThru.find(fm) != isFMThru.end())
        continue;
      isFMThru[fm] =
        itin->getItinBranding().isThruFareMarket(*fm);
    }
  }

  const std::vector<QualifiedBrand>& qualifiedBrands = _trx.brandProgramVec();
  for (auto& fmIsThru: isFMThru)
  {
    std::vector<int> newBrandProgramIndexVec;
    for (int brandIndex: fmIsThru.first->brandProgramIndexVec())
    {
      TSE_ASSERT(qualifiedBrands[brandIndex].second != nullptr);
      BrandCode brandCode = qualifiedBrands[brandIndex].second->brandCode();
      if (fareMarketsParityBrands[fmIsThru.first].find(brandCode) !=
          fareMarketsParityBrands[fmIsThru.first].end())
      {
        newBrandProgramIndexVec.push_back(brandIndex);
      }
    }
    fmIsThru.first->brandProgramIndexVec() = newBrandProgramIndexVec;

    if (!fmIsThru.first->brandProgramIndexVec().empty())
    {
      // isThru indicator is needed later in FCO to fail soft passed brands
      // for such FMs. That's why this attribute is computed only for fare
      // markets having brands
      fmIsThru.first->setThru(fmIsThru.second);
      fmWithBrandsFound = true;
    }
    else if (!isCatchAllBucket)
    {
      fmIsThru.first->failCode() = ErrorResponseException::NO_VALID_BRAND_FOUND;
    }
  }

  if (diag892 && diag892->isDDPARITY())
  {
    *diag892 << "\nFARE MARKET BRANDS AFTER PROCESSING ITINS:\n";
    BrandedDiagnosticUtil::displayFareMarketsWithBrands(*diag892, _trx.fareMarket(),
                                                        _trx.brandProgramVec());
  }

  return (fmWithBrandsFound || (isCatchAllBucket && !isFMThru.empty()));
}


// TODO (andrzej.fediuk) clean up / decouple the code; unit test missing
bool TrxGeometryCalculator::calculateBrandParityForAllItins(
    Diag892Collector* diag892)
{
  // helper variable to keep brands fulfilling the parity requirement for
  // fare markets across all itins (fare markets are shared between itins,
  // but can have different properties in different itins, i.e. different
  // start/end segment index in travel).
  BrandCodesPerFareMarket fareMarketsBrands;

  for (Itin* itin: _trx.itin())
  {
    TSE_ASSERT(itin != nullptr);

    if (diag892)
    {
      diag892->printItinInfo(itin, _trx.activationFlags().isSearchForBrandsPricing());
    }

    ItinBranding& itinBranding = itin->getItinBranding();

    itinBranding.calculateBrandParity();
    itin->brandCodes() = itinBranding.getParityBrands();

    if (diag892)
    {
      diag892->printItinParityInfo(
          itin, &_trx, itinBranding.getParityBrands(), itinBranding.getBrandsForCurrentlyShoppedLeg());
    }

    for (const auto& brandPerFM: itinBranding.getParityBrandsPerFareMarket())
    {
      fareMarketsBrands[brandPerFM.first].insert(
          brandPerFM.second.begin(), brandPerFM.second.end());
    }

    itinBranding.updateProgramsForCalculatedBrands();
  }

  if (!removeBrandsNotMatchingParity(fareMarketsBrands, diag892))
  {
    return false;
  }

  if (_trx.getRequest()->isCatchAllBucketRequest())
  {
    return true;
  }

  removeItinsWithNoBrands(diag892);
  return true;
}


// TODO (andrzej.fediuk) clean up / decouple the code; unit test missing
bool TrxGeometryCalculator::calculateParityBrandsOverrideForAllItins(
    Diag892Collector* diag892)
{
  TSE_ASSERT(getTrx().getRequest()->isProcessParityBrandsOverride());

  // helper variable to keep brands fulfilling the parity requirement for
  // fare markets across all itins (fare markets are shared between itins,
  // but can have different properties in different itins, i.e. different
  // start/end segment index in travel).
  BrandCodesPerFareMarket fareMarketsBrands;

  const bool areAllLegsFixed =
    std::find(_trx.getFixedLegs().begin(), _trx.getFixedLegs().end(), false) == _trx.getFixedLegs().end();

  for (Itin* itin: _trx.itin())
  {
    TSE_ASSERT(itin != nullptr);

    if (diag892)
    {
      if (_trx.isContextShopping())
        diag892->printContextShoppingItinInfo(itin, &_trx);
      else
        diag892->printItinInfo(itin, _trx.activationFlags().isSearchForBrandsPricing());
    }

    ItinBranding& itinBranding = itin->getItinBranding();

    itinBranding.calculateBrandsForCurrentlyShoppedLegAndRestOfTheTravel();

    //get parity brands for non-fixed, not currently shopped legs
    itin->brandCodes() = itinBranding.getParityBrands();

    if (diag892)
    {
      if (_trx.isContextShopping())
        diag892->printContextShoppingItinParityInfo(
          itin, &_trx, itinBranding.getParityBrands(), itinBranding.getBrandsForCurrentlyShoppedLeg());
      else
        diag892->printItinParityInfo(
          itin, &_trx, itinBranding.getParityBrands(), itinBranding.getBrandsForCurrentlyShoppedLeg());
    }

    for (const auto& brandPerFM: itinBranding.getParityBrandsPerFareMarket())
      fareMarketsBrands[brandPerFM.first].insert(brandPerFM.second.begin(), brandPerFM.second.end());

    // if parity on not-fixed, not currently shopped part returned anything add data from currently
    // shopped leg
    if (!itin->brandCodes().empty())
    {
      for (const auto& brandPerFM: itinBranding.getBrandsForCurrentlyShoppedLegPerFareMarket())
        fareMarketsBrands[brandPerFM.first].insert(brandPerFM.second.begin(), brandPerFM.second.end());

      const UnorderedBrandCodes& currentLegBrands = itinBranding.getBrandsForCurrentlyShoppedLeg();
      if (currentLegBrands.empty()) //no brands on current leg - trigger no valid brands
      {
        itin->brandCodes().clear();
        fareMarketsBrands.clear();
      }
      else
        itin->brandCodes().insert(currentLegBrands.begin(), currentLegBrands.end());
    }

    // When all legs are fixed the parity algorithm can return empty set of brands.
    // In this case use ANY_BRAND to price this itin - it will work as for context
    // shopping fare market merger uses "ANY BRAND" on fixed legs (and all legs
    // are fixed) so only branded fares will be used
    if (areAllLegsFixed)
      itin->brandCodes().insert(ANY_BRAND);

    // if no brands matching parity on on not-fixed, not currently shopped part of the travel, don't
    // update anything, the transaction will be terminated (no valid brands found)

    itinBranding.updateProgramsForCalculatedBrands();
  }

  if (!isCrossBrandWithoutParity(_trx))
  {
    if (!removeBrandsNotMatchingParity(fareMarketsBrands, diag892))
      return false;
  }

  if (_trx.getRequest()->isCatchAllBucketRequest() && !_trx.isContextShopping())
    return true;

  removeItinsWithNoBrands(diag892);

  return true;
}


// TODO (andrzej.fediuk) clean up / decouple the code; unit test missing
bool TrxGeometryCalculator::calculateContextBrandParityForAllItins(
    Diag892Collector* diag892)
{
  TSE_ASSERT(_trx.isContextShopping());

  // helper variable to keep brands fulfilling the parity requirement for
  // fare markets across all itins (fare markets are shared between itins,
  // but can have different properties in different itins, i.e. different
  // start/end segment index in travel).
  BrandCodesPerFareMarket fareMarketsBrands;

  const bool areAllLegsFixed =
    std::find(_trx.getFixedLegs().begin(), _trx.getFixedLegs().end(), false) == _trx.getFixedLegs().end();

  for (Itin* itin: _trx.itin())
  {
    TSE_ASSERT(itin != nullptr);

    if (diag892)
      diag892->printContextShoppingItinInfo(itin, &_trx);

    ItinBranding& itinBranding = itin->getItinBranding();

    itinBranding.calculateBrandParityForNonFixedLegs();
    itin->brandCodes() = itinBranding.getParityBrands();

    if (!fallback::fallbackSoldoutOriginRT(&_trx))
    {
      // for originBasedRTPricing with fixed brand use that brand
      if (areAllLegsFixed && itin->brandCodes().empty() && _trx.getRequest()->originBasedRTPricing())
        itin->brandCodes().insert(_trx.getFareComponentShoppingContexts().at(0)->brandCode);
    }

    // When all legs are fixed the parity algorithm will return empty set of brands.
    // In this case use ANY_BRAND to price this itin - it will work as for context
    // shopping fare market merger uses "ANY BRAND" on fixed legs (and all legs
    // are fixed) so only branded fares will be used
    if (areAllLegsFixed && itin->brandCodes().empty())
      itin->brandCodes().insert(ANY_BRAND);

    if (diag892)
    {
      diag892->printContextShoppingItinParityInfo(
          itin, &_trx, itinBranding.getParityBrands(), itinBranding.getBrandsForCurrentlyShoppedLeg());
    }

    for (const auto& fmBrand: itinBranding.getParityBrandsPerFareMarket())
    {
      fareMarketsBrands[fmBrand.first].insert(fmBrand.second.begin(), fmBrand.second.end());
    }

    itinBranding.updateProgramsForCalculatedBrands();
  }

  if (!removeBrandsNotMatchingParity(fareMarketsBrands, diag892))
  {
    return false;
  }

  removeItinsWithNoBrands(diag892);

  return true;
}


void TrxGeometryCalculator::removeItinsWithNoBrands(Diag892Collector* diag892)
{
  std::function<bool(const Itin*)> itinHasBrands = IAIbfUtils::ItinHasBrands();
  if (isCrossBrandWithoutParity(_trx))
  {
    itinHasBrands = IAIbfUtils::ItinHasBrandsOnAllSegments();
  }

  std::vector<Itin*>::iterator rmBegin =
      std::stable_partition(_trx.itin().begin(), _trx.itin().end(), itinHasBrands);

  if (diag892 && (rmBegin != _trx.itin().end()))
  {
    *diag892 << "\nITINS REMOVED DUE TO NO BRANDS :\n";
    for (std::vector<Itin*>::iterator it = rmBegin; it != _trx.itin().end(); ++it)
    {
      *diag892 << (*it)->itinNum() << " ";
    }
    *diag892 << "\n";
  }

  _trx.itin().erase(rmBegin, _trx.itin().end());

  if (_trx.itin().empty())
  {
    if (diag892)
      diag892->flushMsg();

    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
        "NO BRANDS IN ANY ITINS");
  }
}

} /* namespace skipper */

} /* namespace tse */
