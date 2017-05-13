//----------------------------------------------------------------------------
//  File: BrandedFaresParityValidator.cpp
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
#include "Fares/BrandedFaresParityValidator.h"

#include "Common/ErrorResponseException.h"
#include "Common/ShoppingUtil.h"


#include <algorithm>
#include <set>
#include <vector>

namespace tse
{

BrandedFaresParityValidator::BrandedFaresParityValidator(ShoppingTrx& trx)
  : _trx(trx), _legCount(trx.legs().size()), _realLegCount(0), _dc(trx)
{
  for (unsigned legIndex = 0; legIndex < _legCount; ++legIndex)
  {
    if (trx.legs()[legIndex].jumpedLegIndices().size() == 0)
      ++_realLegCount;
    else
      break;
  }
}

BrandCode&
BrandedFaresParityValidator::getBrandCodeFromTrx(int index) const
{
  return _trx.brandProgramVec()[index].second->brandCode();
}

void
BrandedFaresParityValidator::process()
{
  if (_trx.brandProgramVec().size() == 0)
  {
    if (_dc.isActive())
    {
      _dc.noBrandsFound();
      _dc.flush(FCO_BRAND_EXTRACT);
    }
  }

  BrandCodeSetVec brandsPerLeg(_realLegCount);

  getBrandsOnEachLeg(brandsPerLeg);

  BrandCodeSet brandsCommonForAllLegs = getIntersectionOfSets(brandsPerLeg);

  if (_dc.isActive())
  {
    _dc.collectCommonBrandsOnLegs(brandsPerLeg);
    _dc.collectCommonBrandsForAllLegs(brandsCommonForAllLegs);
    _dc.flush(FCO_BRAND_EXTRACT);
  }

  removeInvalidBrandsInFaresAndFareMarkets(_allFareMarkets, brandsCommonForAllLegs);

  if (_dc.isActive())
    _dc.flush(FCO_BRAND_PARITY);
}

void
BrandedFaresParityValidator::getBrandsOnEachLeg(BrandCodeSetVec& brandsPerLeg)
{
  std::map<LocCode, BrandCodeSetArray> BrandsPerMiddlePoint;

  for (unsigned legIndex = 0; legIndex < _legCount; ++legIndex)
  {
    const ShoppingTrx::Leg& leg = _trx.legs()[legIndex];
    IndexVector legsCovered;

    if (leg.stopOverLegFlag())
    {
      for (uint32_t legId : leg.jumpedLegIndices())
      {
        if (legId != ASOLEG_SURFACE_SECTOR_ID)
          legsCovered.push_back(legId);
      }
    }
    else
      legsCovered.push_back(legIndex);

    BrandsPerMiddlePoint.clear();
    const ItinIndex::ItinMatrix& itinMatrix = leg.carrierIndex().root();
    for (const auto& elem : itinMatrix)
    {
      const ItinIndex::Key& carrierKey = elem.first;
      // retrieve the first direct itin to obtain all fare markets
      ItinIndex::ItinCell* itinCell =
          ShoppingUtil::retrieveDirectItin(_trx, legIndex, carrierKey, ItinIndex::CHECK_NOTHING);
      if (!itinCell || !itinCell->second)
        continue;

      Itin& itin = *itinCell->second;

      const LocCode& origin = itin.travelSeg().front()->origin()->loc();
      const LocCode& destination = itin.travelSeg().back()->destination()->loc();

      std::vector<FareMarket*>& fareMarketsForCurrentCarrier = itin.fareMarket();
      std::set<FareMarket*> uniqueFareMarketsForCurrentCarrier;
      uniqueFareMarketsForCurrentCarrier.insert(fareMarketsForCurrentCarrier.begin(),
                                                fareMarketsForCurrentCarrier.end());

      if (fareMarketsForCurrentCarrier.size() > 0)
      {
        saveFareMarketsFoundForLater(uniqueFareMarketsForCurrentCarrier);
        extractAndSaveBrandsFromFMs(uniqueFareMarketsForCurrentCarrier,
                                    legsCovered,
                                    brandsPerLeg,
                                    BrandsPerMiddlePoint,
                                    origin,
                                    destination);
      }
    }
    if (BrandsPerMiddlePoint.size() >
        0) // There are local markets involved and we need to calculate parity on those as well
    {
      BrandCodeSet brandsForCurrentLegs;
      std::map<LocCode, BrandCodeSetArray>::iterator it;

      for (it = BrandsPerMiddlePoint.begin(); it != BrandsPerMiddlePoint.end(); ++it)
      {
        BrandCodeSetVec brandsPerMiddlePointVec((it->second).begin(), (it->second).end());
        ;
        BrandCodeSet commonBrandsOnLocalPath = getIntersectionOfSets(brandsPerMiddlePointVec);
        brandsForCurrentLegs.insert(commonBrandsOnLocalPath.begin(), commonBrandsOnLocalPath.end());
      }
      saveBrandsToLegs(brandsPerLeg, brandsForCurrentLegs, legsCovered);
    }
  }
}

void
BrandedFaresParityValidator::saveFareMarketsFoundForLater(const std::set<FareMarket*>& fareMarkets)
{
  std::copy(fareMarkets.begin(), fareMarkets.end(), std::back_inserter(_allFareMarkets));
}

void
BrandedFaresParityValidator::extractAndSaveBrandsFromFMs(
    const std::set<FareMarket*>& fareMarkets,
    const IndexVector& legs,
    BrandCodeSetVec& brandsPerLeg,
    std::map<LocCode, BrandCodeSetArray>& BrandsPerMiddlePoint,
    const LocCode& legOrigin,
    const LocCode& legDestination)
{
  BrandCodeSet brandsForCurrentLegs;

  for (const FareMarket* currentFm : fareMarkets)
  {

    TSE_ASSERT(currentFm != nullptr);

    bool isThruFM = isThruFareMarket(currentFm, legOrigin, legDestination);
    if (_dc.isActive())
      _dc.collectFareMarket(legs, currentFm, isThruFM);
    if (isThruFM)
    {
      // For Thru Markets simply copy brands to the set of brands covering this leg
      for (int brandIndex : currentFm->brandProgramIndexVec())
      {
        brandsForCurrentLegs.insert(getBrandCodeFromTrx(brandIndex));
      }
    }
    else
    {
      // For local fare markets it's needed to find common brands on each market path. We cannot do
      // it right away
      // as fare markets of different carriers may sum up to a whole leg. Here we only save local
      // brands for later.
      const LocCode& fmOrigin = currentFm->origin()->loc();
      const LocCode& fmDestination = currentFm->destination()->loc();

      LocCode intermediatePoint;
      unsigned int segmentNr;

      if (fmOrigin == legOrigin) // first segment
      {
        intermediatePoint = fmDestination;
        segmentNr = 0;
      }
      else // last segment
      {
        intermediatePoint = fmOrigin;
        segmentNr = 1;
      }

      for (int index : currentFm->brandProgramIndexVec())
        BrandsPerMiddlePoint[intermediatePoint][segmentNr].insert(getBrandCodeFromTrx(index));
    }
  }
  // In Case of ASO legs that cover more than one real leg we copy available brands to legs covered
  // by the ASO leg
  // We need that to find common brands for the whole journey ( ASO legs covering for example legs 1
  // and 2 with a brand MM
  // Allows us to state that brand MM is valid for 'legs 1 and 2' part of the trip.

  saveBrandsToLegs(brandsPerLeg, brandsForCurrentLegs, legs);
}

void
BrandedFaresParityValidator::saveBrandsToLegs(BrandCodeSetVec& brandsPerLeg,
                                              const BrandCodeSet& brandsForCurrentLegs,
                                              const IndexVector& legs) const
{
  for (unsigned int leg : legs)
  {
    TSE_ASSERT(leg < brandsPerLeg.size());
    brandsPerLeg[leg].insert(brandsForCurrentLegs.begin(), brandsForCurrentLegs.end());
  }
}

bool
BrandedFaresParityValidator::isThruFareMarket(const FareMarket* fm,
                                              const LocCode& legOrigin,
                                              const LocCode& legDestination) const
{
  if (fm->origin()->loc() != legOrigin)
    return false;
  if (fm->destination()->loc() != legDestination)
    return false;

  return true;
}

BrandCodeSet
BrandedFaresParityValidator::getIntersectionOfSets(const BrandCodeSetVec& sets) const
{
  BrandCodeSet result;
  if (sets.empty())
    return result;
  if (sets.size() == 1)
    return sets.front();

  result = sets.front();
  BrandCodeSet buffer;

  for (size_t i = 1; i < sets.size(); ++i)
  {
    buffer.clear();
    std::set_intersection(result.begin(),
                          result.end(),
                          sets[i].begin(),
                          sets[i].end(),
                          std::inserter(buffer, buffer.begin()));
    swap(result, buffer);
  }

  return result;
}

void
BrandedFaresParityValidator::removeInvalidBrandsInFaresAndFareMarkets(
    std::vector<FareMarket*>& allFareMarkets, const BrandCodeSet& commonBrands)
{
  for (FareMarket* fm : allFareMarkets)
  {
    std::vector<int> brandsRemoved;

    if (fm->brandProgramIndexVec().size() > 0)
      removeInvalidBrands(fm, commonBrands, brandsRemoved);

    if (fm->brandProgramIndexVec().empty())
    {
      markMarketsWithoutBrandedFaresAsInvalid(fm);
    }

    if (_dc.isActive())
      _dc.collectBrandsRemovedFromFareMarket(fm, brandsRemoved);

    for (PaxTypeFare* fare : fm->allPaxTypeFare())
      checkIfFareIsStillValidForIbf(fare);
  }
}

void
BrandedFaresParityValidator::removeInvalidBrands(FareMarket* fm,
                                                 const BrandCodeSet& commonBrands,
                                                 std::vector<int>& brandsRemoved)
{
  // We iterate backwards so when we remove elements on certain indexes those that are still to be
  // processed don't change position
  for (int index = (fm->brandProgramIndexVec().size() - 1); index >= 0; --index)
  {
    unsigned int brandId = fm->brandProgramIndexVec()[index];
    const BrandCode& currentBrand = getBrandCodeFromTrx(brandId);
    if (commonBrands.find(currentBrand) ==
        commonBrands.end()) // Brand doesn't have parity. We need to remove it from the fare market.
    {
      fm->brandProgramIndexVec().erase(fm->brandProgramIndexVec().begin() + index);
      // We also need to remove statuses of those brands in each fare in the fare market
      for (PaxTypeFare* fare : fm->allPaxTypeFare())
      {
        TSE_ASSERT(fare != nullptr);
        TSE_ASSERT(index < static_cast<int>(fare->getBrandStatusVec().size()));

        fare->getMutableBrandStatusVec().erase(
            fare->getMutableBrandStatusVec().begin() + index);
      }

      brandsRemoved.push_back(brandId);
    }
  }
}

void
BrandedFaresParityValidator::checkIfFareIsStillValidForIbf(PaxTypeFare* fare) const
{
  TSE_ASSERT(fare != nullptr);

  bool isValid = false;
  for (const PaxTypeFare::BrandStatusWithDirection& brandStatus : fare->getBrandStatusVec())
  {
    if (brandStatus.first != PaxTypeFare::BS_FAIL)
    {
      isValid = true;
      break;
    }
  }

  if (!isValid)
    fare->setIsValidForBranding(false);
}

void
BrandedFaresParityValidator::markMarketsWithoutBrandedFaresAsInvalid(FareMarket* fm) const
{
  fm->failCode() = ErrorResponseException::NO_VALID_BRAND_FOUND;
}

} // namespace tse
