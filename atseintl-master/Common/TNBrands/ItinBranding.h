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

#pragma once

#include "Common/BrandingUtil.h"
#include "Common/TNBrands/BrandingOptionSpacesCalculator.h"
#include "Common/TNBrands/FareMarketsParityCalculator.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "Common/TNBrands/TrxGeometryCalculator.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TNBrandsTypes.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/Diag892Collector.h"
#include "Pricing/FareMarketPathMatrix.h"

#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/join.hpp>

#include <map>
#include <vector>

namespace tse {

namespace skipper {

// TODO add documentation
template <typename ItinGeometryCalculatorT = ItinGeometryCalculator,
          typename TNBrandsFunctionsT = TNBrandsFunctions,
          typename BrandingOptionSpacesCalculatorT = BrandingOptionSpacesCalculator<>,
          typename FareMarketsParityCalculatorFactoryT = Factory<FareMarketsParityCalculator<> > >
class ItinBrandingTemplate
{
public:
  ItinBrandingTemplate(ItinGeometryCalculatorT* itinGeometryCalculator,
      TNBrandsFunctionsT* functions = 0,
      BrandingOptionSpacesCalculatorT* spacesCalculator = 0,
      FareMarketsParityCalculatorFactoryT* parityCalculatorFactory = 0)
  {
    TSE_ASSERT(itinGeometryCalculator != nullptr);
    _itinGeometryCalculator.reset(itinGeometryCalculator);
    assignValidObject(functions, _functions);
    assignValidObject(spacesCalculator, _spacesCalculator);
    assignValidObject(parityCalculatorFactory, _parityCalculatorFactory);
  }

  static ItinBranding* createItinBranding(
      Itin& itin,
      const ITrxGeometryCalculator& trxGeometryCalculator,
      DataHandle& dataHandle)
  {
    ItinGeometryCalculator* itinGeometryCalculator =
        new ItinGeometryCalculator(itin, trxGeometryCalculator);
    return &dataHandle.safe_create<ItinBranding>(itinGeometryCalculator);
  }

  /*
   ----------------------------------------------------------------------------
   ______  __   __   ____  __   __  _____  _____   _____   __  __   __   ____
  |__  __||  \ |  | /  __\|  | |  ||  _  ||     \ |     \ |  ||  \ |  | /  __\
    |  |  |   \|  | \  \  |  |_|  || | | ||  |>  ||  |>  ||  ||   \|  ||  |  _
    |  |  |       |  \  \ |   _   || | | ||   __/ |   __/ |  ||       ||  | | |
    |  |  |  |\   | __\  \|  | |  || |_| ||  |    |  |    |  ||  |\   ||  \_/ |
    |__|  |__| \__| \____/|__| |__||_____||__|    |__|    |__||__| \__| \____/

  -----------------------------------------------------------------------------
  */

  // Generates pricing option spaces for the associated itin.
  void calculateBrandingOptionSpaces(
    const SegmentOrientedBrandCodesPerCarrier* validBrands,
    const SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCarrierByCabin,
    size_t limit,
    Logger& logger,
    Diag892Collector* diag892 = nullptr);

  const BrandingOptionSpace& getBrandingOptionSpace(size_t brandingOptionSpaceIndex) const
  {
    TSE_ASSERT(brandingOptionSpaceIndex < _brandingOptionsSpacesWithCabinInfo.size());
    return _brandingOptionsSpacesWithCabinInfo[brandingOptionSpaceIndex].second;
  }

  size_t getBrandingOptionSpaceCabin(size_t brandingOptionSpaceIndex) const
  {
    TSE_ASSERT(brandingOptionSpaceIndex < _brandingOptionsSpacesWithCabinInfo.size());
    return _brandingOptionsSpacesWithCabinInfo[brandingOptionSpaceIndex].first;
  }

  size_t getBrandingOptionSpacesCount() const
  {
    return _brandingOptionsSpacesWithCabinInfo.size();
  }

  // Returns (carrier, brand code) pairs applicable for the specified segment
  // (i.e. all FareMarkets in the itinerary starting at this segment) in the
  // specified BrandingOptionSpace.
  //
  // Throws if brandingOptionSpaceIndex is outside spaces range for this itin.
  const CarrierBrandPairs& getCarriersBrandsForSegment(
      size_t brandingOptionSpaceIndex, const TravelSeg* segment) const
  {
    TSE_ASSERT(brandingOptionSpaceIndex < _brandingOptionsSpacesWithCabinInfo.size());
    const BrandingOptionSpace& space =
        _brandingOptionsSpacesWithCabinInfo[brandingOptionSpaceIndex].second;
    const size_t segmentIndex = _itinGeometryCalculator->getTravelSegmentIndex(segment);
    return space[segmentIndex];
  }

  // returns true if carrier has an entry for a given segment on a given space
  // that entry can be a valid brand or a NO_BRAND. returns false if carrier is not found at all.
  bool isCarrierValidForSpaceSegment(uint16_t spaceIndex,
                                     uint16_t segmentIndex,
                                     const CarrierCode& carrierCode)
  {
    if (spaceIndex >= _brandingOptionsSpacesWithCabinInfo.size())
      return false;
    if (segmentIndex >= _brandingOptionsSpacesWithCabinInfo[spaceIndex].second.size())
      return false;
    for (auto& crDirBrand: _brandingOptionsSpacesWithCabinInfo[spaceIndex].second[segmentIndex])
      if (crDirBrand.first.carrier == carrierCode)
        return true;

    return false;
  }

  // Check if brandCode exists for a given carrier, on a given segment,
  // in a given pricing space for given direction and BOTHWAYS.
  // If exists, returns it, otherwise returns empty BrandCode().
  // Direction is a direction of the fare.
  BrandCode getBrandCodeOrEmpty(uint16_t spaceIndex,
                                uint16_t segmentIndex,
                                const CarrierCode& carrierCode,
                                Direction direction) const
  {
    TSE_ASSERT(spaceIndex < _brandingOptionsSpacesWithCabinInfo.size());
    const BrandingOptionSpace& space =
        _brandingOptionsSpacesWithCabinInfo[spaceIndex].second;

    TSE_ASSERT(segmentIndex < space.size());
    const CarrierBrandPairs travelSegBrands = space[segmentIndex];

    CarrierDirection carrierDirection = CarrierDirection(carrierCode, direction);
    CarrierBrandPairs::const_iterator found = travelSegBrands.find(carrierDirection);

    if (found == travelSegBrands.end() && direction != Direction::BOTHWAYS)
    {
      carrierDirection.direction = Direction::BOTHWAYS;
      found = travelSegBrands.find(carrierDirection);
    }

    if (found == travelSegBrands.end())
      return BrandCode();
    return found->second;
  }

  // Check if brandCode exists for a given carrier, on a given segment,
  // in a given pricing space for given direction and BOTHWAYS.
  // If exists, returns it, otherwise throws an assertion.
  // Direction is a direction of the fare.
  BrandCode getBrandCode(uint16_t spaceIndex,
                         uint16_t segmentIndex,
                         const CarrierCode& carrierCode,
                         Direction direction) const
  {
    BrandCode brand = getBrandCodeOrEmpty(spaceIndex, segmentIndex, carrierCode, direction);
    TSE_ASSERT(!brand.empty());
    return brand;
  }

  // Check if brandCode exists for a given carrier, on a given segment,
  // in a given pricing space for given direction and BOTHWAYS.
  // If exists, returns it, otherwise throws an assertion.
  // If brand matching fare direction is not found and fare direction is BOTHWAYS
  // a fare usage direction is used for search.
  // TODO(andrzej.fediuk) DIR: verify if fareUsageDirection is sufficient
  BrandCode getBrandCode(uint16_t spaceIndex,
                         uint16_t segmentIndex,
                         const CarrierCode& carrierCode,
                         Direction fareDirection,
                         Direction fareUsageDirection) const
  {
    BrandCode brand = getBrandCodeOrEmpty(spaceIndex, segmentIndex, carrierCode, fareDirection);
    if (brand.empty() && fareDirection == Direction::BOTHWAYS)
      brand = getBrandCodeOrEmpty(spaceIndex, segmentIndex, carrierCode, fareUsageDirection);
    TSE_ASSERT(!brand.empty());
    return brand;
  }

  // Returns the FareMarketPathMatrix for pricing the itinerary in the
  // specified BrandingOptionSpace.
  //
  // Throws if no FareMarketPathMatrix was previously stored for
  // brandingOptionSpaceIndex.
  FmpMatrixPtr getFmpMatrix(size_t brandingOptionSpaceIndex)
  {
    std::map<size_t, FmpMatrixPtr>::iterator it =
      _poSpaceFmpMatrices.find(brandingOptionSpaceIndex);
    TSE_ASSERT(it != _poSpaceFmpMatrices.end());
    return it->second;
  }

  // Store the FareMarketPathMatrix that will be used to price the itinerary
  // in the specified BrandingOptionSpace.
  void setFmpMatrix(FmpMatrixPtr fmpMatrix, size_t brandingOptionSpaceIndex)
  {
    _poSpaceFmpMatrices[brandingOptionSpaceIndex] = fmpMatrix;
    _poFmpMatricesSpace[fmpMatrix] = brandingOptionSpaceIndex;
  }

  // For given FmpMatrixPtr returns corresponding branding space.
  size_t getBrandingSpaceIndex(FmpMatrixPtr fmpMatrix) const
  {
    std::map<FmpMatrixPtr, size_t>::const_iterator found =
        _poFmpMatricesSpace.find(fmpMatrix);
    TSE_ASSERT(found != _poFmpMatricesSpace.end());
    return found->second;
  }

  // Returns soldout status computed for the whole pricing space.
  IbfErrorMessage getItinSoldoutStatusForSpace(size_t brandingOptionSpaceIndex)
  {
    const SoldoutStatusInSpace& soldoutForSpace =
        getOrCalculateSoldoutStatusesForSpace(brandingOptionSpaceIndex);

    return soldoutForSpace.first;
  }

  // Returns a soldout status for a given leg in a given pricing space.
  // Throws exception if requested leg not found in the computed map but status set
  // for whole space.
  IbfErrorMessage getSoldoutStatusForLeg(size_t brandingOptionSpaceIndex, LegId legId)
  {
    const SoldoutStatusInSpace& soldoutForSpace =
        getOrCalculateSoldoutStatusesForSpace(brandingOptionSpaceIndex);

    const SoldoutStatusPerLeg& soldoutForLeg = soldoutForSpace.second;

    SoldoutStatusPerLeg::const_iterator found = soldoutForLeg.find(legId);

    if (soldoutForSpace.first == IbfErrorMessage::IBF_EM_NOT_SET)
    {
      // Status not set is possible if fare market path is empty. In that
      // case if there is no status for selected leg return status of the
      // whole space.
      if (found != soldoutForLeg.end())
      {
        return found->second;
      }
      else
      {
        return soldoutForSpace.first;
      }
    }
    // Otherwise status should be always set for all legs.
    TSE_ASSERT(found != soldoutForLeg.end());
    return found->second;
  }

  /*
  -----------------------------------------------------------------------------
   __  _____   _____             ____    ____   __   __  ______  _____
  |  ||     \ |  ___|    _      /    \  /    \ |  \ |  ||__  __||  ___|
  |  ||  >  / |  |_    _| |_   |  / \_||  /\  ||   \|  |  |  |  |  |_
  |  ||     \ |   _|  |_   _|  |  |  _ |  ||  ||       |  |  |  |   _|
  |  ||  |>  ||  |      |_|    |  \_/ ||  \/  ||  |\   |  |  |  |  |__  __
  |__||_____/ |__|              \____/  \____/ |__| \__|  |__|  |_____||__|

  -----------------------------------------------------------------------------
  */

  // Generates list of brands matching parity requirement with fixed legs
  // limitations for the associated itin.
  // Used in Context Shopping.
  void calculateBrandParityForNonFixedLegs();

  // Verifies additional brand (carried over from fixed part of the journey)
  // versus parity requirements. Returns true if brand is valid.
  // Used in Context Shopping.
  bool verifyBrandPresentOnNonFixedPart(BrandCode brand);

  // Generates list of brands matching parity requirement (i.e. brand is
  // available through whole fare market path) for the associated itin.
  // Used in IBF.
  void calculateBrandParity();

  // Generates list of brands available on the shopped (first not fixed) leg and a list of brands
  // matching parity requirement (i.e. brand is available through the rest of the travel) for the
  // associated itin.
  // Used in both Context Shopping and IBF.
  void calculateBrandsForCurrentlyShoppedLegAndRestOfTheTravel();

  // Generates and stores in itin brand-programs relation using only brands
  // calculated as matching parity requirement.
  void updateProgramsForCalculatedBrands();

  // Returns calculated brands matching parity requirement as a map with
  // fare markets as a keys.
  const BrandCodesPerFareMarket& getParityBrandsPerFareMarket() const
  {
    return _parityBrandsPerFareMarket;
  }

  // Returns calculated brands for currently shopped leg as a map with
  // fare markets as a keys.
  const BrandCodesPerFareMarket& getBrandsForCurrentlyShoppedLegPerFareMarket() const
  {
    return _parityBrandsPerFareMarketForCurrentLeg;
  }

  // Returns calculated brands matching parity requirement. Used in IBF.
  const UnorderedBrandCodes& getParityBrands() const
  {
    return _parityBrands;
  }

  // Returns calculated brands for the currently shopped leg. Used in IBF and Context Shopping.
  const UnorderedBrandCodes& getBrandsForCurrentlyShoppedLeg() const
  {
    return _brandsForCurrentLeg;
  }

  // Verifies if given fare market is "thru" type (covers any number of whole
  // legs). For more documentation see ItinGeometryCalculator::isThruFareMarket
  bool isThruFareMarket(const FareMarket& fareMarket) const
  {
    return _itinGeometryCalculator->isThruFareMarket(fareMarket);
  }

  /*
  -----------------------------------------------------------------------------
    ____   _____  __   __  _____  _____     ____    __
   /  __\ |  ___||  \ |  ||  ___||     \   /    \  |  |
  |  |  _ |  |_  |   \|  ||  |_  |  |>  | /  /\  \ |  |
  |  | | ||   _| |       ||   _| |     / /   --   \|  |
  |  \_/ ||  |__ |  |\   ||  |__ |  |\ \ |   __   ||  |___
   \____/ |_____||__| \__||_____||__| \_\|__|  |__||______|

  -----------------------------------------------------------------------------
  */

  // Returns fare market first and last segment's index in itin.
  // For more documentation see ItinGeometryCalculator::getFareMarketStartSegmentIndex
  // and ItinGeometryCalculator::getFareMarketEndSegmentIndex
  std::pair<size_t, size_t> getFareMarketStartEndSegments(const FareMarket& fareMarket) const
  {
    return std::make_pair<size_t, size_t>(
      _itinGeometryCalculator->getFareMarketStartSegmentIndex(fareMarket),
      _itinGeometryCalculator->getFareMarketEndSegmentIndex(fareMarket));
  }

  // Returns leg id for travel segment corresponding to given index/
  // For more documentation see ItinGeometryCalculator::getTravelSegmentLegId
  size_t getTravelSegmentLegId(size_t segmentId) const
  {
    return _itinGeometryCalculator->getTravelSegmentLegId(segmentId);
  }

  // Returns valid (filtered) indices of brand/program element in trx.brandProgramVec()
  // For more documentation see ItinGeometryCalculator::getProgramsForCarriersBrand
  const QualifiedBrandIndices& getQualifiedBrandIndicesForCarriersBrand(
      const CarrierCode& carrier, const BrandCode& brand) const
  {
    return _itinGeometryCalculator->getQualifiedBrandIndicesForCarriersBrand(
        carrier, brand);
  }

  std::map<Direction, BrandCode> getBrandCodeForMarket(
      const FareMarket& fareMarket, const BrandingOptionSpace& space) const
  {
    std::map<Direction, BrandCode> brandCodes;
    const CarrierBrandPairs& carrierBrandPairs =
        space.at(_itinGeometryCalculator->getFareMarketStartSegmentIndex(fareMarket));
    std::set<CarrierDirection> carrierDirections = {
        CarrierDirection(fareMarket.governingCarrier(), Direction::ORIGINAL),
        CarrierDirection(fareMarket.governingCarrier(), Direction::REVERSED),
        CarrierDirection(fareMarket.governingCarrier(), Direction::BOTHWAYS)};
    for (const CarrierDirection& cd: carrierDirections)
    {
      CarrierBrandPairs::const_iterator it = carrierBrandPairs.find(cd);
      if (it != carrierBrandPairs.end())
        brandCodes[cd.direction] = it->second;
    }

    return brandCodes;
  }

  std::string brandsFromSpaceToString(const BrandingOptionSpace& space) const
  {
    std::vector<std::string> brandsPerSegment;
    for (const CarrierBrandPairs& carriersBrands : space)
    {
      std::set<std::string> currentSegmentBrands;

      CarrierBrandPairs::const_iterator it = carriersBrands.begin();
      for ( ; it != carriersBrands.end(); ++it)
      {
        if (it->second.empty())
        {
          std::string b = "??";
          b.append("(?)");
          currentSegmentBrands.insert(b);
        }
        else
        {
          std::string b(it->second);
          b.append("(");
          b.append(directionToIndicator(it->first.direction));
          b.append(")");
          currentSegmentBrands.insert(b);
        }
      }
      std::string segmentBrandsAsString = boost::algorithm::join(currentSegmentBrands, ",");

      if (currentSegmentBrands.size() > 1)
        segmentBrandsAsString = "{" + segmentBrandsAsString + "}";

      // Adding only if different than previous and deduplicate is true
      if (brandsPerSegment.empty() || brandsPerSegment.back() != segmentBrandsAsString)
      {
        brandsPerSegment.push_back(segmentBrandsAsString);
      }
    }
    return boost::algorithm::join(brandsPerSegment, "-");
  }

  bool hasAnyBrandOnAnyFareMarket() const
  {
    for (const auto fareMarket : _itinGeometryCalculator->getFareMarkets())
    {
      if (!fareMarket->brandProgramIndexVec().empty())
        return true;
    }
    return false;
  }

protected:

  // Returns SoldoutStatusInSpace structure for a requested space index.
  // It retrieves it from the internal map or calculates it, stores it in the map
  // and returns reference to this newly added element if it didn't previously exist
  const SoldoutStatusInSpace&
  getOrCalculateSoldoutStatusesForSpace(size_t brandingOptionSpaceIndex)
  {
    SoldoutStatusPerSpace::iterator it = _soldoutStatusPerSpace.find(brandingOptionSpaceIndex);
    if (it != _soldoutStatusPerSpace.end())
      return it->second;

    SoldoutStatusPerSpace::value_type soldoutStatusPerSpace(brandingOptionSpaceIndex,
                                                            SoldoutStatusInSpace());
    soldoutStatusPerSpace.second.first =
        IbfAvailabilityTools::calculateAllStatusesForMatrix(soldoutStatusPerSpace.second.second,
                                  getFmpMatrix(brandingOptionSpaceIndex)->fareMarketPathMatrix());
    // returning newly added element
    return _soldoutStatusPerSpace.insert(soldoutStatusPerSpace).first->second;
  }

private:

  boost::scoped_ptr<ItinGeometryCalculatorT> _itinGeometryCalculator;
  boost::scoped_ptr<TNBrandsFunctionsT> _functions;
  boost::scoped_ptr<BrandingOptionSpacesCalculatorT> _spacesCalculator;
  boost::scoped_ptr<FareMarketsParityCalculatorFactoryT> _parityCalculatorFactory;
  std::map<size_t, FmpMatrixPtr> _poSpaceFmpMatrices;
  std::map<FmpMatrixPtr, size_t> _poFmpMatricesSpace;
  BrandCodesPerFareMarket _parityBrandsPerFareMarket;
  BrandCodesPerFareMarket _parityBrandsPerFareMarketForCurrentLeg;
  UnorderedBrandCodes _parityBrands;
  UnorderedBrandCodes _brandsForCurrentLeg;
  std::vector<std::pair<size_t, const BrandingOptionSpace> > _brandingOptionsSpacesWithCabinInfo;
  SoldoutStatusPerSpace _soldoutStatusPerSpace;
};

// This thing below was written as a replacement of the following typedef
// since it doesn't work with forward declaration of ItinBranding in Itin.h
//typedef ItinBrandingTemplate<> ItinBranding
struct ItinBranding : public ItinBrandingTemplate<>
{
  ItinBranding(ItinGeometryCalculator* itinGeometryCalculator):
    ItinBrandingTemplate<>(itinGeometryCalculator) {}
};

template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
void ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT,
  BrandingOptionSpacesCalculatorT, FareMarketsParityCalculatorFactoryT>::calculateBrandingOptionSpaces(
  const SegmentOrientedBrandCodesPerCarrier* brandsValidForCabin,
  const SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCarrierByCabin,
  size_t limit,
  Logger& logger,
  Diag892Collector* diag892)
{
  if (diag892 && brandsValidForCabin != nullptr)
  {
     // Filtering Brands by Cabin requested
     diag892->printValidBrandsForCabinHeader();
     diag892->printSegmentOrientedBrandCodes(*brandsValidForCabin);
     if (diag892->isDDINFO())
       diag892->printFilterByCabinInfo();
  }

  SegmentOrientedBrandCodesPerCarrier brandInfo;
  _functions->calculateSegmentOrientedBrandCodesPerCarrier(
    *_itinGeometryCalculator, brandsValidForCabin, brandInfo);
  if (diag892)
  {
    diag892->printAfterReqBrandsFilteringHeader();
    diag892->printSegmentOrientedBrandCodes(brandInfo);
  }

  BrandedFaresComparator comparator(
    _itinGeometryCalculator->getTrxGeometryCalculator().getQualifiedBrands(),
    logger);
  if (diag892 && diag892->isDDINFO())
  {
    diag892->printComparator(comparator);
  }

  SegmentOrientedBrandCodeArraysPerCarrier brandInfoArrays;
  _functions->sortSegmentOrientedBrandCodesPerCarrier(
    brandInfo, comparator, brandInfoArrays);
  if (diag892 && diag892->isDDINFO())
  {
    diag892->printSegmentOrientedBrandCodesAfterSorting(brandInfoArrays);
  }

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(
      _itinGeometryCalculator->getTrxGeometryCalculator().getTrx());
  if (!brandsPerCarrierByCabin.empty())
  {
    // filled brandsPerCarrierByCabin means the whole trip is requested with
    // same cabin (or cabin was not specified)

    size_t spacesGenerated = 0;
    bool isFirstCabin = true;

    boost::array<size_t, 3> cabinOrder = {
      CabinType::generalIndex(CabinType::ECONOMY_CLASS),
      CabinType::generalIndex(CabinType::BUSINESS_CLASS),
      CabinType::generalIndex(CabinType::FIRST_CLASS)
    };
    SegmentOrientedBrandCodeArraysPerCarrier brandDeduplication = brandInfoArrays;
    for (size_t cabinIndex : cabinOrder)
    {
      SegmentOrientedBrandCodesPerCarrierInCabin::const_iterator cabinIterator =
          brandsPerCarrierByCabin.find(cabinIndex);
      if (cabinIterator == brandsPerCarrierByCabin.end())
      {
        continue;
      }

      SegmentOrientedBrandCodeArraysPerCarrier brandInfoArraysInCabin;
      if (_functions->filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
                                  cabinIterator->second,
                                  brandInfoArrays,
                                  brandDeduplication,
                                  brandInfoArraysInCabin,
                                  useDirectionality))
      {
        if (diag892 && diag892->isDDINFO())
        {
          diag892->printCabinHeader(cabinIterator->first);
          diag892->printSegmentOrientedBrandCodesAfterSorting(brandInfoArraysInCabin);
        }

        _functions->fillEmptyBrandsArraysPerCarrier(brandInfoArraysInCabin);

        BrandingOptionSpaces temporarySpaces;
        size_t maxAvailableSpaces =
          _functions->calculateMaxBrandsCountPerCarrier(brandInfoArraysInCabin);
        size_t cabinLimit = 0;
        if (limit && (maxAvailableSpaces > limit - spacesGenerated))
          cabinLimit = limit - spacesGenerated;

        _spacesCalculator->calculateSpaces(
                    brandInfoArraysInCabin,
                    temporarySpaces,
                    cabinLimit,
                    isFirstCabin);

        if (isFirstCabin)
        {
          // only the cheapest cabin contains "the cheapest" space
          _functions->updateReservedBrandingOptionSpace(temporarySpaces);
          isFirstCabin = false;
        }

        for (const BrandingOptionSpace space : temporarySpaces)
        {
          _brandingOptionsSpacesWithCabinInfo.push_back(
              std::make_pair(cabinIterator->first, space));
          ++spacesGenerated;
          if (limit && spacesGenerated > limit)
            break;
        }

        if (diag892)
        {
          diag892->printCabinHeader(cabinIterator->first);
          diag892->printBrandingOptionSpaces(temporarySpaces);
        }

        // no point to calculate spaces for higher cabins as limit was reached
        if (limit && spacesGenerated >= limit)
          break;
      }
      else if (diag892 && diag892->isDDINFO()) // no brands for this cabin
      {
        diag892->printCabinHeader(cabinIterator->first);
        diag892->printNA();
      }

    }
  }
  else
  {
    _functions->fillEmptyBrandsArraysPerCarrier(brandInfoArrays);

    BrandingOptionSpaces temporarySpaces;
    _spacesCalculator->calculateSpaces(brandInfoArrays, temporarySpaces, limit);

    _functions->updateReservedBrandingOptionSpace(temporarySpaces);

    for (const BrandingOptionSpace space : temporarySpaces)
    {
      _brandingOptionsSpacesWithCabinInfo.push_back(
          std::make_pair(CabinType::generalIndex(CabinType::UNKNOWN_CLASS), space));
    }

    if (diag892)
    {
      diag892->printBrandingOptionSpaces(temporarySpaces);
    }
  }
}

template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
void ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT,
  BrandingOptionSpacesCalculatorT, FareMarketsParityCalculatorFactoryT>::calculateBrandParity()
{
  _parityBrands.clear();
  _parityBrandsPerFareMarket.clear();

  boost::scoped_ptr<typename FareMarketsParityCalculatorFactoryT::Type>
    parityCalculator(_parityCalculatorFactory->create(*_itinGeometryCalculator));

  for (const FareMarket* fm : _itinGeometryCalculator->getFareMarkets())
  {
    TSE_ASSERT(fm != nullptr);
    parityCalculator->addFareMarket(*fm);
  }
  size_t startingSegmentIndex = 0;
  startingSegmentIndex
    = _itinGeometryCalculator->getNextTravelSegmentIfCurrentArunk(startingSegmentIndex);
  FareMarketsPerBrandCode brandFMRelation
    = parityCalculator->possibleBrands(startingSegmentIndex);

  FareMarketsPerBrandCode::iterator iter = brandFMRelation.begin();
  for (; iter != brandFMRelation.end(); ++iter)
  {
    _parityBrands.insert(iter->first);
    FareMarkets::iterator fmIterator = iter->second.begin();
    for (; fmIterator != iter->second.end(); ++fmIterator)
    {
      _parityBrandsPerFareMarket[*fmIterator].insert(iter->first);
    }
  }
}


template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
bool ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT,
  BrandingOptionSpacesCalculatorT,
  FareMarketsParityCalculatorFactoryT>::verifyBrandPresentOnNonFixedPart(
      BrandCode brand)
{
  std::map<uint16_t, UnorderedBrandCodes> segmentBrandsMap =
      _itinGeometryCalculator->calculateSegmentToBrandsMapping();

  const std::vector<bool>& fixedLegs =
      _itinGeometryCalculator->getTrxGeometryCalculator().getFixedLegs();

  // check if given brand is available on non-fixed part of the journey
  bool foundAtAllSegments = true;
  for (auto& segmentBrandsPair: segmentBrandsMap)
  {
    size_t legId =
        _itinGeometryCalculator->getTravelSegmentLegId(segmentBrandsPair.first);
    if (fixedLegs.at(legId)) // skip fixed legs
      continue;
    if (segmentBrandsPair.second.find(brand) == segmentBrandsPair.second.end())
    {
      foundAtAllSegments = false;
      break;
    }
  }
  return foundAtAllSegments;
}

//TODO(andrzej.fediuk): refactor/split; many common parts with
//calculateBrandParityForNonFixedLegs() and calculateBrandParity()
template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
void ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT, BrandingOptionSpacesCalculatorT,
  FareMarketsParityCalculatorFactoryT>::calculateBrandsForCurrentlyShoppedLegAndRestOfTheTravel()
{
  // clear output data
  _parityBrands.clear();
  _parityBrandsPerFareMarket.clear();
  _brandsForCurrentLeg.clear();
  _parityBrandsPerFareMarketForCurrentLeg.clear();

  const bool isContextShopping =
      _itinGeometryCalculator->getTrxGeometryCalculator().getTrx().isContextShopping();

  // for context shopping on fixed legs allow only fixed brands
  if (isContextShopping)
     _itinGeometryCalculator->reduceFareMarketsBrandsOnFixedLegs();

  // prepare calculator
   boost::scoped_ptr<typename FareMarketsParityCalculatorFactoryT::Type>
     parityCalculator(_parityCalculatorFactory->create(*_itinGeometryCalculator));
   for (const FareMarket* fm : _itinGeometryCalculator->getFareMarkets())
   {
     TSE_ASSERT(fm != nullptr);
     parityCalculator->addFareMarket(*fm);
   }

   // find parity borders = for ibf whole travel, for context shopping calculate not fixed part
   size_t startingSegmentIndex = 0;
   size_t endingSegmentIndex = _itinGeometryCalculator->getSegmentCount();
   if (isContextShopping)
   {
     std::pair<size_t, size_t> notFixedSegments =
         _itinGeometryCalculator->calculateNonFixedSegmentsForContextShopping();
     startingSegmentIndex = notFixedSegments.first;
     endingSegmentIndex = notFixedSegments.second;
   }

   // maybe everything is fixed
   if (startingSegmentIndex == TRAVEL_SEG_DEFAULT_ID)
   {
     // add brands for fare markets on fixed legs
     for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
     {
       TSE_ASSERT(fm != nullptr);
       BrandCode brand = NO_BRAND;
       if (_itinGeometryCalculator->isFareMarketOnFixedLeg(*fm, brand) &&
           (brand != NO_BRAND))
         _parityBrandsPerFareMarket[fm].insert(brand);
     }
     // there is not currently shopped leg so leave _brandsForCurrentLeg and
     // _parityBrandsPerFareMarketForCurrentLeg empty
     return;
   }

   // something is not fixed

   // get brands for currently shopped leg
   startingSegmentIndex =
     _itinGeometryCalculator->getNextTravelSegmentIfCurrentArunk(startingSegmentIndex);
   for (const FareMarket* fm : _itinGeometryCalculator->getFareMarkets())
   {
     TSE_ASSERT(fm != nullptr);
     if (_itinGeometryCalculator->getFareMarketStartSegmentIndex(*fm) == startingSegmentIndex)
     {
       UnorderedBrandCodes fmBrands = _itinGeometryCalculator->getItinSpecificBrandCodes(*fm);
       _brandsForCurrentLeg.insert(fmBrands.begin(), fmBrands.end());
       _parityBrandsPerFareMarketForCurrentLeg[fm] = fmBrands;
     }
   }

   FareMarketsPerBrandCode brandFMRelation =
       parityCalculator->possibleBrands(startingSegmentIndex, endingSegmentIndex);

   BrandCode fixedBrand = NO_BRAND;

   if (isContextShopping)
   {
     // additional processing for context shopping - add brand for thru fares partially being fixed.
     const BrandFilterMap& brandsFilter =
       _itinGeometryCalculator->getBrandFilterMap();

     // add brands for fare markets on fixed legs (not calculated during
     // parity calculation)
     // additionally if this fare market is on both fixed and non-fixed path
     // of the travel mark this brand as possible parity brand
     for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
     {
       TSE_ASSERT(fm != nullptr);
       BrandCode brand = NO_BRAND;
       if (_itinGeometryCalculator->isFareMarketOnFixedLeg(*fm, brand))
       {
         if (brand != NO_BRAND)
         {
           _parityBrandsPerFareMarket[fm].insert(brand);
           const bool addBrand =
             (brandsFilter.empty() || (brandsFilter.find(brand) != brandsFilter.end()));
           const bool isOnFixed =
               _functions->isAnySegmentOnNonFixedLeg(fm->travelSeg(),
                   _itinGeometryCalculator->getTrxGeometryCalculator().getFixedLegs());
           if (isOnFixed && addBrand)
           {
             fixedBrand = brand;
             break; // max one brand can be carried over
           }
         }
       }
     }

     // verify if brands given on fixed part of the travel are accessible through
     // the non-fixed part of the journey
     if (fixedBrand != NO_BRAND)
     {
       if (verifyBrandPresentOnNonFixedPart(fixedBrand))
       {
         const std::vector<QualifiedBrand>& qualifiedBrands =
             _itinGeometryCalculator->getTrxGeometryCalculator().getQualifiedBrands();
         for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
         {
           for (int index: fm->brandProgramIndexVec())
           {
             TSE_ASSERT(index < int(qualifiedBrands.size()));
             if (fixedBrand == qualifiedBrands[index].second->brandCode())
               brandFMRelation[fixedBrand].insert(fm);
           }
         }
         _parityBrands.insert(fixedBrand);
       }
       else
         fixedBrand = NO_BRAND;
     }
   }

   // parse calculated data for non-fixed part of the travel to data structured
   for (auto& fmBrands: brandFMRelation)
   {
     _parityBrands.insert(fmBrands.first);
     for (auto* fareMarket: fmBrands.second)
     {
       _parityBrandsPerFareMarket[fareMarket].insert(fmBrands.first);
       if (fixedBrand != NO_BRAND)
       {
         // fixed brand is a brand that is present on fixed fare markets which
         // are also available on non-fixed part of the travel.
         // parityCalculator->possibleBrands() calculates brands parity only
         // for non-fixed part of the travel so we need to add these brands
         // if originally fare market had them (it is safe as we validate this
         // brand in verifyAdditionalBrandForNonFixedPart against improper
         // values given by the user - in normal circumstances this brand would
         // be calculated in previous step thus thus fulfilling parity requirement
         // but "trust noone").
         UnorderedBrandCodes fareMarketBrands =
             _itinGeometryCalculator->getItinSpecificBrandCodes(*fareMarket);
         if (fareMarketBrands.find(fixedBrand) != fareMarketBrands.end())
           _parityBrandsPerFareMarket[fareMarket].insert(fixedBrand);
       }
     }
   }
 }


template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
void ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT,
  BrandingOptionSpacesCalculatorT, FareMarketsParityCalculatorFactoryT>::calculateBrandParityForNonFixedLegs()
{
  _parityBrands.clear();
  _parityBrandsPerFareMarket.clear();

  _itinGeometryCalculator->reduceFareMarketsBrandsOnFixedLegs();

  boost::scoped_ptr<typename FareMarketsParityCalculatorFactoryT::Type>
    parityCalculator(_parityCalculatorFactory->create(*_itinGeometryCalculator));
  for (const FareMarket* fm : _itinGeometryCalculator->getFareMarkets())
  {
    TSE_ASSERT(fm != nullptr);
    parityCalculator->addFareMarket(*fm);
  }

  std::pair<size_t, size_t> notFixedSegments =
      _itinGeometryCalculator->calculateNonFixedSegmentsForContextShopping();
  size_t startingSegmentIndex = notFixedSegments.first;
  size_t endingSegmentIndex = notFixedSegments.second;

  if (startingSegmentIndex != TRAVEL_SEG_DEFAULT_ID)
  {
    // there is something not fixed
    startingSegmentIndex
      = _itinGeometryCalculator->getNextTravelSegmentIfCurrentArunk(startingSegmentIndex);

    FareMarketsPerBrandCode brandFMRelation =
        parityCalculator->possibleBrands(startingSegmentIndex, endingSegmentIndex);

    BrandCode fixedBrand = NO_BRAND;
    const BrandFilterMap& brandsFilter =
        _itinGeometryCalculator->getBrandFilterMap();

    // add brands for fare markets on fixed legs (not calculated during
    // parity calculation)
    // additionally if this fare market is on both fixed and non-fixed path
    // of the travel mark this brand as possible parity brand
    for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
    {
      TSE_ASSERT(fm != nullptr);
      BrandCode brand = NO_BRAND;
      if (_itinGeometryCalculator->isFareMarketOnFixedLeg(*fm, brand))
      {
        if (brand != NO_BRAND)
        {
          _parityBrandsPerFareMarket[fm].insert(brand);
          const bool addBrand =
            (brandsFilter.empty() || (brandsFilter.find(brand) != brandsFilter.end()));
          const bool isOnFixed =
              _functions->isAnySegmentOnNonFixedLeg(fm->travelSeg(),
                  _itinGeometryCalculator->getTrxGeometryCalculator().getFixedLegs());
          if (isOnFixed && addBrand)
          {
            fixedBrand = brand;
            break; // max one brand can be carried over
          }
        }
      }
    }

    // verify if brands given on fixed part of the travel are accessible through
    // the non-fixed part of the journey
    if (fixedBrand != NO_BRAND)
    {
      if (verifyBrandPresentOnNonFixedPart(fixedBrand))
      {
        const std::vector<QualifiedBrand>& qualifiedBrands =
            _itinGeometryCalculator->getTrxGeometryCalculator().getQualifiedBrands();
        for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
        {
          for (int index: fm->brandProgramIndexVec())
          {
            TSE_ASSERT(index < int(qualifiedBrands.size()));
            if (fixedBrand == qualifiedBrands[index].second->brandCode())
              brandFMRelation[fixedBrand].insert(fm);
          }
        }
        _parityBrands.insert(fixedBrand);
      }
      else
        fixedBrand = NO_BRAND;
    }

    // parse calculated data for non-fixed part of the travel to data structured
    for (auto& fmBrands: brandFMRelation)
    {
      _parityBrands.insert(fmBrands.first);
      for (auto* fareMarket: fmBrands.second)
      {
        _parityBrandsPerFareMarket[fareMarket].insert(fmBrands.first);
        if (fixedBrand != NO_BRAND)
        {
          // fixed brand is a brand that is present on fixed fare markets which
          // are also available on non-fixed part of the travel.
          // parityCalculator->possibleBrands() calculates brands parity only
          // for non-fixed part of the travel so we need to add these brands
          // if originally fare market had them (it is safe as we validate this
          // brand in verifyAdditionalBrandForNonFixedPart against improper
          // values given by the user - in normal circumstances this brand would
          // be calculated in previous step thus thus fulfilling parity requirement
          // but "trust noone").
          UnorderedBrandCodes fareMarketBrands =
              _itinGeometryCalculator->getItinSpecificBrandCodes(*fareMarket);
          if (fareMarketBrands.find(fixedBrand) != fareMarketBrands.end())
            _parityBrandsPerFareMarket[fareMarket].insert(fixedBrand);
        }
      }
    }
  }
  else
  {
    // add brands for fare markets on fixed legs
    for (const FareMarket* fm: _itinGeometryCalculator->getFareMarkets())
    {
      TSE_ASSERT(fm != nullptr);
      BrandCode brand = NO_BRAND;
      if (_itinGeometryCalculator->isFareMarketOnFixedLeg(*fm, brand) &&
          (brand != NO_BRAND))
        _parityBrandsPerFareMarket[fm].insert(brand);
    }
  }
}

template <typename ItinGeometryCalculatorT,
          typename TNBrandsFunctionsT,
          typename BrandingOptionSpacesCalculatorT,
          typename FareMarketsParityCalculatorFactoryT>
void ItinBrandingTemplate<ItinGeometryCalculatorT, TNBrandsFunctionsT,
  BrandingOptionSpacesCalculatorT, FareMarketsParityCalculatorFactoryT>::updateProgramsForCalculatedBrands()
{
  UnorderedBrandCodes brandsToUse = _parityBrands;
  if (_itinGeometryCalculator->getTrxGeometryCalculator().getTrx()
       .getRequest()->isProcessParityBrandsOverride())
  {
    //PBO = "T", use also brands from currently shopped leg
    brandsToUse.insert(_brandsForCurrentLeg.begin(), _brandsForCurrentLeg.end());
  }

  for (const FareMarket* fm : _itinGeometryCalculator->getFareMarkets())
  {
    TSE_ASSERT(fm != nullptr);
    BrandProgramRelations relations;
    _itinGeometryCalculator->getTrxGeometryCalculator()
        .getBrandsAndPrograms(*fm, relations);

    UnorderedBrandCodes fmBrands = relations.getAllBrands();
    UnorderedBrandCodes intersection;
    std::set_intersection(
        brandsToUse.begin(),
        brandsToUse.end(),
        fmBrands.begin(),
        fmBrands.end(),
        std::inserter(intersection, intersection.begin()));
    for (const auto& elem : intersection)
    {
      const ProgramIds programs = relations.getProgramsForBrand(elem);
      for (const auto& program : programs)
      {
        _itinGeometryCalculator->addBrandProgramPair(elem, program);
      }
    }
  }
}

} //namespace skipper

} /* namespace tse */

