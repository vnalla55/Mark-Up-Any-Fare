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

#include "Common/TNBrands/TNBrandsFunctions.h"

#include "Common/BrandingUtil.h"
#include "Common/CabinType.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"

#include <vector>
#include <algorithm>

namespace tse
{

namespace skipper
{

const size_t TNBrandsFunctions::ALL_SEGMENTS = 255;

size_t TNBrandsFunctions::calculateMaxBrandsCountPerCarrier(
    const SegmentOrientedBrandCodeArraysPerCarrier& input, size_t onSegment)
{
  size_t maxBrandsPerCarrier = 0;

  size_t startIndex = (onSegment == ALL_SEGMENTS ? 0 : onSegment);
  size_t endIndex = (onSegment == ALL_SEGMENTS ? input.size() : onSegment + 1);
  if (endIndex > input.size())
  {
    return maxBrandsPerCarrier;
  }

  for (size_t segIndex = startIndex; segIndex < endIndex; ++segIndex)
  {
    for (const auto& elem : input[segIndex])
    {
      if (elem.second.size() > maxBrandsPerCarrier)
      {
        maxBrandsPerCarrier = elem.second.size();
      }
    }
  }
  return maxBrandsPerCarrier;
}

size_t TNBrandsFunctions::calculateProportionalIndex(
    size_t outputIndex, size_t inputSetSize, size_t outputSetSize)
{
  TSE_ASSERT(inputSetSize != 0);
  TSE_ASSERT(outputSetSize != 0);
  TSE_ASSERT(outputIndex < outputSetSize);
  return (outputIndex * inputSetSize) / outputSetSize;
}

size_t TNBrandsFunctions::calculateBottomPreferredIndex(
    size_t outputIndex, size_t inputSetSize, size_t outputSetSize)
{
  TSE_ASSERT(inputSetSize > 0);
  TSE_ASSERT(outputIndex < outputSetSize);
  // To calculate bottom preferred index we need to find where the preferred
  // elements start. For example for inputSetSize and outputSetSize of 3 and 13
  // respectively we expect that two first elements of input set will be
  // "propagated" into 4 elements each and the last element if the input set
  // will be "propagated" into 5 elements (giving 13 in total).

  // The number of preferred elements is the remainder of the output/input set
  // size division, i.e. 13 % 3 = 1
  size_t remainder = outputSetSize % inputSetSize;
  // Each "normal" element will occur exactly the number of times output set
  // contains input set, i.e. 13 / 3 = 4. Preferred elements will occur one
  // more time.
  size_t times = outputSetSize / inputSetSize;
  // To calculate index of the first preferred output element we need to count
  // all "normal" elements that will/were returned, that is subtract preferred
  // elements count form input set size (this gives number of "normal" elements
  // in the input set) and multiply by times each element will be used.
  size_t divider = (inputSetSize - remainder) * times;
  // For the "normal" elements we have simple dependence - each input element
  // is expected "times" number of times and is "uniform" distributed.

  if (times == 0)
  {
    // outputSetSize is smaller than inputSetSize
    return (inputSetSize - 1)
           - calculateProportionalIndex((outputSetSize - 1) - outputIndex,
                                        inputSetSize,
                                        outputSetSize);
  }

  if (outputIndex < divider)
  {
    return outputIndex / times;
  }
  //else
  // For the preferred elements we are left with "inputSetSize-(divider/times)"
  // elements in the input set that needs to be distributed into
  // "outputSetSize-divider" output elements "uniformly". So we do it by calling
  // calculateProportionalIndex but with requested output index recalculated to
  // 0..(outputSetSize-divider-1) elements. This will return correctly distributed
  // index from input set but from 0..(inputSetSize-(divider/times)-1) range,
  // so we need to add "divider/times" to "move" it after the "normal" indices.
  return calculateProportionalIndex(outputIndex - divider,
                                    inputSetSize - (divider / times),
                                    outputSetSize - divider)
      + (divider / times);
}

size_t TNBrandsFunctions::calculateTopPreferredIndex(
    size_t outputIndex, size_t inputSetSize, size_t outputSetSize)
{
  return (inputSetSize - 1)
      - calculateBottomPreferredIndex((outputSetSize - 1) - outputIndex,
                                      inputSetSize,
                                      outputSetSize);
}

void TNBrandsFunctions::fillEmptyBrandsArraysPerCarrier(SegmentOrientedBrandCodeArraysPerCarrier& brandInfo)
{
  for (auto& elem : brandInfo)
  {
    for (auto& cxBrand : elem)
    {
      if (cxBrand.second.size() == 0)
      {
        cxBrand.second.push_back(NO_BRAND);
      }
    }
  }
}

void TNBrandsFunctions::updateReservedBrandingOptionSpace(
  BrandingOptionSpaces& brandingSpaces)
{
  // at least reserved and one filled
  TSE_ASSERT(brandingSpaces.size() >= 2);
  const size_t reservedIndex = 0;
  const size_t firstFilledIndex = 1;

  for (size_t segmentIndex = 0;
       segmentIndex < brandingSpaces[firstFilledIndex].size();
       ++segmentIndex)
  {
    CarrierBrandPairs spaceBlock = brandingSpaces[firstFilledIndex][segmentIndex];
    std::map<CarrierCode, Direction> carrierDirection;
    std::set<CarrierCode> carriersToMerge;
    for (auto& elem : spaceBlock)
    {
      if (carrierDirection.find(elem.first.carrier) != carrierDirection.end())
      {
        if ((elem.first.direction == Direction::ORIGINAL) &&
            (carrierDirection.at(elem.first.carrier) == Direction::REVERSED))
          carriersToMerge.insert(elem.first.carrier);
        else if ((elem.first.direction == Direction::REVERSED) &&
                 (carrierDirection.at(elem.first.carrier) == Direction::ORIGINAL))
          carriersToMerge.insert(elem.first.carrier);
      }
      else
        carrierDirection[elem.first.carrier] = elem.first.direction;
      elem.second = NO_BRAND;
    }
    // Change ORIGINAL+REVERSED into BOTHWAYS
    for (const CarrierCode& cr: carriersToMerge)
    {
      spaceBlock.erase(CarrierDirection(cr, Direction::ORIGINAL));
      spaceBlock.erase(CarrierDirection(cr, Direction::REVERSED));
      spaceBlock[CarrierDirection(cr, Direction::BOTHWAYS)] = NO_BRAND;
    }
    brandingSpaces[reservedIndex].push_back(spaceBlock);

  }


  // Deduplicate if there are no brands; the first (and only) filled space
  // is then the same as the reserved non-branded one.
  if (brandingSpaces[reservedIndex] == brandingSpaces[firstFilledIndex])
  {
    TSE_ASSERT(brandingSpaces.size() == 2);
    brandingSpaces.resize(1);  // Doesn't matter which one we delete
  }
}

void
TNBrandsFunctions::buildBrandingOptionSpacesForAllItins(PricingTrx& trx,
                                                        Logger& logger,
                                                        Diag892Collector* diag892)
{
  if (diag892 != nullptr)
  {
    diag892->printBrandingOptionsHeader();
  }

  const bool stayInCabin = trx.getRequest()->getJumpCabinLogic() != JumpCabinLogic::ENABLED;
  bool shouldFilterByCabin = trx.isCabinHigherThanEconomyRequested() || stayInCabin;
  const size_t cabinOnWholeTripIndex = trx.getWholeTripCabinIndex();
  for (Itin* itin : trx.itin())
  {
    if (diag892)
      diag892->printItinInfo(itin, trx.activationFlags().isSearchForBrandsPricing());

    ItinBranding& itinBranding = itin->getItinBranding();

    SegmentOrientedBrandCodesPerCarrier brandsForFilteringByCabin;
    SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCarrierByCabin;

    if (cabinOnWholeTripIndex != CabinType::generalIndex(CabinType::UNKNOWN_CLASS))
    {
      shouldFilterByCabin = false;
      // brandsPerCarrierByCabin contains all brands assigned to cabin.
      // If filtering by cabin is requested the selectBrandsValidForReqCabin
      // should prepare the list of valid brands per segment/carrier.
      selectBrandsPerCabin(trx, itin, cabinOnWholeTripIndex,
                           brandsPerCarrierByCabin, diag892, stayInCabin);
    }
    else
    {
      selectBrandsValidForReqCabin(trx, itin, brandsForFilteringByCabin, diag892, stayInCabin);
    }

    itinBranding.calculateBrandingOptionSpaces(
        shouldFilterByCabin ? &brandsForFilteringByCabin : nullptr,
        brandsPerCarrierByCabin,
        trx.getNumberOfBrands(),
        logger,
        diag892);
  }
}

void
TNBrandsFunctions::selectBrandsValidForReqCabin(const PricingTrx& trx,
                                                const Itin* itin,
                                                SegmentOrientedBrandCodesPerCarrier& filteredBrands,
                                                Diag892Collector* diag892,
                                                bool stayInCabin)
{
  filteredBrands.resize(itin->travelSeg().size());

  for (const FareMarket* fm : itin->fareMarket())
  {
    const CabinType& requestedCabin = trx.getCabinForLeg(fm->legIndex());
    // subtracting 1 as segment order returns values starting from 1
    const size_t segIndex = itin->segmentOrder(fm->travelSeg().front()) - 1;
    for (const PaxTypeFare* paxTypeFare : fm->allPaxTypeFare())
    {
      // Cabins are ordered in a way that highest cabin has lowest number(SUPER_SONIC=0,ECONOMY=8)
      // since undefined cabin is 'cleverly' defined as ' ' it doesn't work well in comparison
      // and we can only compare cabins deemed as valid
      if (requestedCabin.isValidCabin())
      {
        if (!paxTypeFare->isValidForCabin(requestedCabin, stayInCabin))
          continue;
      }
      // If a fare has requested or higher cabin then we decide that hard passed brands
      // assigned to this fare are valid for requested cabin as well.
      std::vector<skipper::BrandCodeDirection> validBrands;
      paxTypeFare->getValidBrands(trx, validBrands, true);

      TSE_ASSERT(segIndex < filteredBrands.size());

      for (const auto& brandAndDirection: validBrands)
      {
        CarrierDirection cd(fm->governingCarrier(), brandAndDirection.direction);
        if (filteredBrands[segIndex][cd].insert(brandAndDirection.brandCode).second && diag892)
        {
          // when a given element is added to the set for the first time log which fare caused that
          diag892->collectFareForValidBrand(segIndex,
                                            fm->governingCarrier(),
                                            brandAndDirection.brandCode,
                                            paxTypeFare->fareClass());
        }
      }
    }
  }
}

namespace
{

std::vector<size_t> getApplicableCabinIndexes(size_t minimalCabinIndex, bool stayInCabin)
{
  std::vector<size_t> allCabins {CabinType::FIRST_CLASS,
                                 CabinType::BUSINESS_CLASS,
                                 CabinType::ECONOMY_CLASS};

  std::transform(allCabins.begin(), allCabins.end(), allCabins.begin(),
                 [](size_t cabin) { return CabinType::generalIndex(cabin);});

  std::vector<size_t> applicableCabinIndexes;
  std::function<bool(size_t)> cabinApplicable;

  if (stayInCabin)
    cabinApplicable = [=](size_t cabinType) { return cabinType == minimalCabinIndex; };
  else
    cabinApplicable = [=](size_t cabinType) { return cabinType >= minimalCabinIndex; };

  std::copy_if (allCabins.begin(), allCabins.end(),
                std::back_inserter(applicableCabinIndexes),
                cabinApplicable);

  return applicableCabinIndexes;
}

typedef std::map<size_t, std::set<CarrierCode> > CarriersPerSegment;

CarriersPerSegment
getUnbrandedCarriersPerSegment(const CarriersPerSegment& allCarriersOnSegments,
                               const CarriersPerSegment& allCarriersOnSegmentsWithBrands)
{
  CarriersPerSegment allCarriersOnSegmentsWithoutBrands;
  for (const auto& carriersPerSegment: allCarriersOnSegments)
  {
    const size_t segmentId = carriersPerSegment.first;
    // all carriers on this segment are non-branded
    if (allCarriersOnSegmentsWithBrands.count(segmentId) == 0)
    {
      allCarriersOnSegmentsWithoutBrands[segmentId] = carriersPerSegment.second;
      continue;
    }

    for (const auto& carrier : carriersPerSegment.second)
    {
      // this carrier doesn't have brands on this segment
      if (allCarriersOnSegmentsWithBrands.at(segmentId).count(carrier) == 0)
        allCarriersOnSegmentsWithoutBrands[segmentId].insert(carrier);
    }
  }
  return allCarriersOnSegmentsWithoutBrands;
}

void
fillMissingDirections(SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCabin,
                      const std::map<size_t, std::map<size_t, std::set<CarrierCode>>>& noBrandElements,
                      const CarriersPerSegment& carriersOnSegment)
{
  // noBrandElements: segment -> cabin -> set of carriers (fare without brands)

  // if we hit fare market without brands we need to add NO_BRAND option.
  // if nothing was there we need to add this as BOTHWAYS, but if there is
  // a brand in other direction we need NO_BRAND only in the opposite way.
  for (auto& cabin: noBrandElements)
  {
    for (auto& segment: cabin.second)
    {
      for (auto& carrier: segment.second)
      {
        bool hasOriginBrand = false;
        bool hasReversedBrand = false;
        CarrierDirection cxOrigin(carrier, Direction::ORIGINAL);
        CarrierDirection cxReversed(carrier, Direction::REVERSED);

        auto& carriersOnSegment = brandsPerCabin.at(cabin.first)[segment.first];

        if (carriersOnSegment.find(cxOrigin) != carriersOnSegment.end())
          hasOriginBrand = true;
        if (carriersOnSegment.find(cxReversed) != carriersOnSegment.end())
          hasReversedBrand = true;
        if (!hasOriginBrand && !hasReversedBrand) // no entry at all, add BOTHWAYS
          carriersOnSegment[CarrierDirection(carrier, Direction::BOTHWAYS)];
        else if (hasOriginBrand && !hasReversedBrand) // only ORIGIN, add REVERSED
          carriersOnSegment[cxReversed];
        else if (!hasOriginBrand && hasReversedBrand) // only REVERSED, add ORIGIN
          carriersOnSegment[cxOrigin];
        //else has both, nothing to do
      }
    }
  }
}

} // end unnamed namespace

void
TNBrandsFunctions::selectBrandsPerCabin(const PricingTrx& trx,
                                        const Itin* itin,
                                        size_t minimalCabinIndex,
                                        SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCabin,
                                        Diag892Collector* diag892,
                                        bool stayInCabin)
{
  const size_t unknownCabinIndex = CabinType::generalIndex(CabinType::UNKNOWN_CLASS);
  TSE_ASSERT(minimalCabinIndex != unknownCabinIndex);

  const size_t segSize = itin->travelSeg().size();
  for (auto cabinIndex : getApplicableCabinIndexes(minimalCabinIndex, stayInCabin))
    brandsPerCabin[cabinIndex].resize(segSize);

  // stores all carriers/segments information
  CarriersPerSegment allCarriersOnSegments;
  // stores all carriers/segments information if carrier has any valid brand
  CarriersPerSegment allCarriersOnSegmentsWithBrands;
  // stores all cabin/segment/carriers information if carrier has fare without brands
  std::map<size_t, std::map<size_t, std::set<CarrierCode>>> noBrandElements;

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  for (const FareMarket* fm : itin->fareMarket())
  {
    const CarrierCode& governingCarrier = fm->governingCarrier();
    // subtracting 1 as segment order returns values starting from 1
    const size_t segIndex = itin->segmentOrder(fm->travelSeg().front()) - 1;
    for (const PaxTypeFare* paxTypeFare : fm->allPaxTypeFare())
    {
      size_t paxCabinIndex = paxTypeFare->cabin().generalIndex();
      if (paxCabinIndex == unknownCabinIndex)
        continue;

      allCarriersOnSegments[segIndex].insert(governingCarrier);

      if (brandsPerCabin.count(paxCabinIndex) == 0)  // cabin not applicable
        continue;

      // Cabins are ordered in a way that highest cabin has lowest number(SUPER_SONIC=0,ECONOMY=8)
      // since undefined cabin is 'cleverly' defined as ' ' it doesn't work well in comparison
      // and we can only compare cabins deemed as valid
      std::vector<skipper::BrandCodeDirection> validBrands;
      paxTypeFare->getValidBrands(trx, validBrands, true);

      for (const auto& brandAndDirection: validBrands)
      {
        brandsPerCabin.at(paxCabinIndex)[segIndex]
         [CarrierDirection(governingCarrier, brandAndDirection.direction)].insert(brandAndDirection.brandCode);
      }

      if (!validBrands.empty())
      {
        allCarriersOnSegmentsWithBrands[segIndex].insert(governingCarrier);
      }
      else
      {
        if (useDirectionality)
          noBrandElements[paxCabinIndex][segIndex].insert(governingCarrier);
        else
          brandsPerCabin.at(paxCabinIndex)[segIndex]
            [CarrierDirection(governingCarrier, Direction::BOTHWAYS)];
      }
    }
  }

  // if a carrier is present on segment (is present in allCarriersOnSegments) but on
  // this segment all brands were filtered out, add its carrier without brands
  CarriersPerSegment allCarriersOnSegmentsWithoutBrands =
        getUnbrandedCarriersPerSegment(allCarriersOnSegments, allCarriersOnSegmentsWithBrands);

  for (const auto& unbrandedCarriersPerSegment : allCarriersOnSegmentsWithoutBrands)
  {
    const size_t segmentId = unbrandedCarriersPerSegment.first;
    for (const auto& carrier : unbrandedCarriersPerSegment.second)
    {
      brandsPerCabin.at(minimalCabinIndex)[segmentId][CarrierDirection(carrier, Direction::BOTHWAYS)];
      if (!useDirectionality)
        continue;

      // if we add NO BRANDS here we don't need to add this via noBrandElements
      if (noBrandElements.count(minimalCabinIndex) > 0)
        if (noBrandElements.at(minimalCabinIndex).count(segmentId)> 0)
          noBrandElements.at(minimalCabinIndex).at(segmentId).erase(carrier);
    }
  }

  if (useDirectionality)
    fillMissingDirections(brandsPerCabin, noBrandElements, allCarriersOnSegments);

  if (diag892 && diag892->isDDINFO())
    diag892->printCarrierBrandAndCabinInfo(brandsPerCabin);
}

bool
TNBrandsFunctions::filterAndDeduplicateCarrierBrandCodes(
  const UnorderedBrandCodes& availableBrandsForCarrier,
  OrderedBrandCodes& validDeduplicatedBrandsForCarrier,
  OrderedBrandCodes& brandInfoForCarrier
)
{
  bool isAnythingAdded = false;

  if (validDeduplicatedBrandsForCarrier.empty())
  {
    // all valid brands for this carrier were already used - add NO_BRAND
    isAnythingAdded = true;
    brandInfoForCarrier.push_back(NO_BRAND);
  }
  else
  {
    // add only valid, not duplicated brands for this carrier
    for (OrderedBrandCodes::iterator validBrandIterator =
           validDeduplicatedBrandsForCarrier.begin();
         validBrandIterator != validDeduplicatedBrandsForCarrier.end();)
         // ++validBrandIterator - iterator incremented in loop
    {
      if (availableBrandsForCarrier.find(*validBrandIterator) !=
          availableBrandsForCarrier.end())
      {
        // this brand is both valid (not yet used) and available (at this segment,
        // for this carrier, in this cabin).
        isAnythingAdded = true;
        brandInfoForCarrier.push_back(*validBrandIterator);
        // erase moves the iterator to the next element
        validBrandIterator = validDeduplicatedBrandsForCarrier.erase(validBrandIterator);
      }
      else
      {
        ++validBrandIterator;
      }
    }
  }

  return isAnythingAdded;
}

bool
TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
  const BrandCodesPerCarrier& brandsPerCarrierInCabinAtSegment,
  const CarrierDirection& validCarrierWithDirection,
  BrandCodeArraysPerCarrier& brandDeduplicationAtSegment,
  BrandCodeArraysPerCarrier& brandInfoArraysForThisCabinAtSegment,
  bool useDirectionality)
{
  bool isAnythingAdded = false;
  BrandCodesPerCarrier::const_iterator carrierBrands =
    brandsPerCarrierInCabinAtSegment.find(validCarrierWithDirection);
  if ((carrierBrands == brandsPerCarrierInCabinAtSegment.end()) &&
      useDirectionality && (validCarrierWithDirection.direction != Direction::BOTHWAYS))
  {
    carrierBrands = brandsPerCarrierInCabinAtSegment.find(
        CarrierDirection(validCarrierWithDirection.carrier, Direction::BOTHWAYS));
  }
  if (carrierBrands != brandsPerCarrierInCabinAtSegment.end())
  {
    // valid carrier found in requested cabin
    if (carrierBrands->second.empty())
    {
      // no brands for this carrier - add NO_BRAND
      isAnythingAdded = true;
      brandInfoArraysForThisCabinAtSegment[
           CarrierDirection(validCarrierWithDirection)].push_back(NO_BRAND);
    }
    else
    {
      // carrier has brands at this segment, we need to be sure each brand is
      // used only once.
      BrandCodeArraysPerCarrier::iterator found =
        brandDeduplicationAtSegment.find(validCarrierWithDirection);
      TSE_ASSERT(found != brandDeduplicationAtSegment.end());

      // anything = at least once true is returned, so logical | is useful
      isAnythingAdded |= filterAndDeduplicateCarrierBrandCodes(
          carrierBrands->second,
          found->second,
          brandInfoArraysForThisCabinAtSegment[validCarrierWithDirection]
          );
    }
  }
  return isAnythingAdded;
}

bool
TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
    const SegmentOrientedBrandCodesPerCarrier& brandsPerCarrierInCabin,
    const SegmentOrientedBrandCodeArraysPerCarrier& validBrandsInOrderPerSegment,
    SegmentOrientedBrandCodeArraysPerCarrier& brandDeduplicationPerSegment,
    SegmentOrientedBrandCodeArraysPerCarrier& brandInfoArraysForThisCabin,
    bool useDirectionality)
{
  TSE_ASSERT(brandsPerCarrierInCabin.size() == validBrandsInOrderPerSegment.size());
  TSE_ASSERT(brandDeduplicationPerSegment.size() == validBrandsInOrderPerSegment.size());

  bool isAnythingAdded = false;
  brandInfoArraysForThisCabin.clear();
  brandInfoArraysForThisCabin.resize(validBrandsInOrderPerSegment.size());

  for (size_t segmentIndex = 0;
       segmentIndex < validBrandsInOrderPerSegment.size();
       ++segmentIndex)
  {
    std::set<CarrierCode> carriers;
    for (BrandCodeArraysPerCarrier::const_iterator validCarrierIterator =
           validBrandsInOrderPerSegment.at(segmentIndex).begin();
         validCarrierIterator != validBrandsInOrderPerSegment.at(segmentIndex).end();
         ++validCarrierIterator)
    {
      carriers.insert(validCarrierIterator->first.carrier);
      // anything = at least once true is returned, so logical | is useful
      isAnythingAdded |= filterCarrierBrandCodesByCabinAtSegment(
          brandsPerCarrierInCabin.at(segmentIndex),
          validCarrierIterator->first,
          brandDeduplicationPerSegment.at(segmentIndex),
          brandInfoArraysForThisCabin[segmentIndex],
          useDirectionality);
    }
    if (useDirectionality)
    {
      // clean up if we have brand BOTHWAYS and NO BRAND in any direction, or
      // brand in direction and NO BRAND BOTHWAYS
      BrandCodeArraysPerCarrier& segment = brandInfoArraysForThisCabin[segmentIndex];
      for (CarrierCode crx: carriers)
      {
        BrandCodeArraysPerCarrier::iterator found =
            segment.find(CarrierDirection(crx, Direction::BOTHWAYS));
        if (found != segment.end())
        {
          if (ShoppingUtil::isAnyBrandReal(found->second))
          { // got BOTHWAYS with valid BRAND, remove NO_BRAND in ORIGINAL/REVERSED
            found = segment.find(CarrierDirection(crx, Direction::ORIGINAL));
            if (found != segment.end())
            {
              OrderedBrandCodes validBrands;
              for (BrandCode& brand: found->second)
              {
                if (ShoppingUtil::isThisBrandReal(brand))
                  validBrands.push_back(brand);
              }
              if (validBrands.empty())
                segment.erase(found);
              else
                found->second = validBrands;
            }
            found = segment.find(CarrierDirection(crx, Direction::REVERSED));
            if (found != segment.end())
            {
              OrderedBrandCodes validBrands;
              for (BrandCode& brand: found->second)
              {
                if (ShoppingUtil::isThisBrandReal(brand))
                  validBrands.push_back(brand);
              }
              if (validBrands.empty())
                segment.erase(found);
              else
                found->second = validBrands;
            }
          }
          else
          { // got BOTHWAYS with NO BRAND only
            BrandCodeArraysPerCarrier::iterator foundDir =
                segment.find(CarrierDirection(crx, Direction::ORIGINAL));
            bool onlyNoBrands = false;
            if (foundDir != segment.end())
            {
              if (ShoppingUtil::isAnyBrandReal(found->second))
              {
                segment.erase(found);
              }
              else
                onlyNoBrands = true;
            }
            if (onlyNoBrands)
            {
              BrandCodeArraysPerCarrier::iterator foundDir =
                  segment.find(CarrierDirection(crx, Direction::REVERSED));
              if (foundDir != segment.end())
              {
                if (ShoppingUtil::isAnyBrandReal(found->second))
                {
                  segment.erase(found);
                  onlyNoBrands = false;
                }
                else
                  onlyNoBrands = true;
              }
              if (onlyNoBrands) //NO BRANDS in all combinations
              {
                segment.erase(CarrierDirection(crx, Direction::ORIGINAL));
                segment.erase(CarrierDirection(crx, Direction::REVERSED));
              }
            }
          }
        }
      }
    }
  }
  return isAnythingAdded;
}

bool
TNBrandsFunctions::isAnySegmentOnFixedLeg(const std::vector<TravelSeg*>& segments,
                                          const std::vector<bool>& fixedLegs)
{
  for (const TravelSeg* seg: segments)
  {
    if (fixedLegs.at(seg->legId()))
      return true;
  }
  return false;
}

bool
TNBrandsFunctions::isAnySegmentOnNonFixedLeg(const std::vector<TravelSeg*>& segments,
                                             const std::vector<bool>& fixedLegs)
{
  for (const TravelSeg* seg: segments)
  {
    if (!fixedLegs.at(seg->legId()))
      return true;
  }
  return false;
}

bool
TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(const std::vector<TravelSeg*>& segments,
                                  const std::vector<bool>& fixedLegs)
{
  const uint16_t UNDEFINED = std::numeric_limits<uint16_t>::max();
  uint16_t firstNotFixedLegId = UNDEFINED;

  // we assume that data has been already validated
  if (fixedLegs.front() && fixedLegs.back())
  {
    // all legs are fixed
    return true;
  }
  else if (fixedLegs.back())
  {
    // last is fixed, so chceck from ending
    auto it = std::find(fixedLegs.rbegin(), fixedLegs.rend(), false);
    TSE_ASSERT(it != fixedLegs.rend());
    firstNotFixedLegId = static_cast<uint16_t>(std::distance(fixedLegs.begin(), it.base()) - 1);
  }
  else
  {
    // first is fixed or none, check from beginning
    auto it = std::find(fixedLegs.begin(), fixedLegs.end(), false);
    TSE_ASSERT(it != fixedLegs.end());
    firstNotFixedLegId = static_cast<uint16_t>(std::distance(fixedLegs.begin(), it));
  }

  for (const TravelSeg* seg : segments)
  {
    if (seg->legId() == firstNotFixedLegId)
      return true;
  }
  return false;
}

void
TNBrandsFunctions::setUpdateNoBrand(CarrierBrandPairs& spaceBlock,
                                    const CarrierDirection& carrierWithDirection)
{
  Direction direction = carrierWithDirection.direction;
  if (direction != Direction::BOTHWAYS)
  {
    // if we are going to add NO_BRAND in ORIGINAL or REVERSED direction
    // and there is already NO_BRAND in the opposite direction, remove it
    // and merge to BOTHWAYS
    Direction oppositeDirection = (direction == Direction::ORIGINAL ?
                                   Direction::REVERSED : Direction::ORIGINAL);
    CarrierBrandPairs::iterator oppositeDirectionFound =
      spaceBlock.find(CarrierDirection(carrierWithDirection.carrier, oppositeDirection));
    if ((oppositeDirectionFound != spaceBlock.end()) && (oppositeDirectionFound->second == NO_BRAND))
    {
      direction = Direction::BOTHWAYS;
      spaceBlock.erase(oppositeDirectionFound);
    }
  }
  spaceBlock[CarrierDirection(carrierWithDirection.carrier, direction)] = NO_BRAND;
}

} // namespace skipper

} /* namespace tse */
