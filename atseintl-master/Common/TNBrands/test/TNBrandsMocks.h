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

#ifndef COMMON_TNBRANDS_TNBRANDSMOCKS_H_
#define COMMON_TNBRANDS_TNBRANDSMOCKS_H_

#include <vector>
#include <gmock/gmock.h>

#include "BrandedFares/BrandedFaresComparator.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TNBrands/BrandProgramRelations.h"
#include "Common/TNBrands/TNBrandsInterfaces.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "DataModel/TNBrandsTypes.h"

namespace tse
{

class MockFareMarket
{
public:
  MOCK_CONST_METHOD0(brandProgramIndexVec,
      const std::vector<int>&());
  MOCK_CONST_METHOD0(governingCarrier,
      const CarrierCode&());
};

namespace skipper
{

template <typename FareMarketType = MockFareMarket>
class MockItinGeometryCalculator
{
public:
  typedef FareMarketType FareMarketT;
  MOCK_CONST_METHOD0_T(getSegmentCount,
      const size_t());
  MOCK_CONST_METHOD0_T(getFareMarkets,
      const std::vector<FareMarketType*>&());
  MOCK_CONST_METHOD1_T(getTravelSegmentIndex,
      size_t(const TravelSeg*));
  MOCK_CONST_METHOD1_T(getTravelSegmentLegId,
      size_t(size_t));
  MOCK_CONST_METHOD1_T(getFareMarketStartSegmentIndex,
      size_t(const FareMarketType&));
  MOCK_CONST_METHOD1_T(getFareMarketEndSegmentIndex,
      size_t(const FareMarketType&));
  MOCK_CONST_METHOD1_T(getItinSpecificBrandCodes,
      UnorderedBrandCodes(const FareMarketType&));
  MOCK_CONST_METHOD0_T(getTrxGeometryCalculator,
      const ITrxGeometryCalculator&());
  MOCK_CONST_METHOD1_T(isTravelSegmentArunk,
      bool(size_t));
  MOCK_CONST_METHOD1_T(getNextTravelSegmentIfCurrentArunk,
      size_t(size_t));
  MOCK_METHOD2_T(addBrandProgramPair,
      void(const BrandCode&, const ProgramID& programId));
  MOCK_CONST_METHOD1_T(isThruFareMarket,
      bool(const FareMarketType&));
  MOCK_CONST_METHOD0_T(isThruFareOnlyItin,
      bool());
  MOCK_METHOD0_T(populateQualifiedBrandIndices,
      void());
  MOCK_CONST_METHOD2_T(getQualifiedBrandIndicesForCarriersBrand,
      const QualifiedBrandIndices&(const CarrierCode&, const BrandCode&));
  MOCK_CONST_METHOD0_T(calculateLegsStartEndSegmentIndices,
      std::vector<std::pair<size_t, size_t> >());
  MOCK_CONST_METHOD0_T(calculateNonFixedSegmentsForContextShopping,
      std::pair<size_t, size_t>());
  MOCK_METHOD0_T(reduceFareMarketsBrandsOnFixedLegs,
      void());
  MOCK_CONST_METHOD2_T(isFareMarketOnFixedLeg,
      bool(const FareMarketType&, BrandCode&));
  MOCK_METHOD2_T(reduceFareMarketBrands,
      void(FareMarketType&, const BrandCode&));
  MOCK_CONST_METHOD0_T(getBrandFilterMap,
      const BrandFilterMap&());
  MOCK_CONST_METHOD0_T(calculateSegmentToBrandsMapping,
      std::map<uint16_t, UnorderedBrandCodes>());
  MOCK_CONST_METHOD1_T(getItinSpecificBrandCodesIndices,
      QualifiedBrandIndices(const FareMarketType&));
  MOCK_CONST_METHOD3_T(getProgramDirection,
      bool(const BrandProgram*, const FareMarketType&, Direction&));
};


class MockBrandInItinInclusionPolicy
{
public:
  MOCK_CONST_METHOD3(isBrandCodeInItin,
      bool(const BrandCode&,
           const BrandProgramRelations&,
           const BrandFilterMap&));
  MOCK_CONST_METHOD3(getIndicesForBrandCodeWithFiltering,
      QualifiedBrandIndices(const BrandCode&,
                            const BrandProgramRelations&,
                            const BrandFilterMap&));
};


class MockITrxGeometryCalculator : public ITrxGeometryCalculator
{
public:
  MOCK_CONST_METHOD2(getBrandsAndPrograms,
      void(const FareMarket& fm, BrandProgramRelations& response));
  MOCK_CONST_METHOD0(getQualifiedBrands,
      const std::vector<QualifiedBrand>&());
  MOCK_CONST_METHOD0(getDiagnostic,
      Diag892Collector*());
  MOCK_METHOD1(calculateBrandParityForAllItins,
      bool(Diag892Collector*));
  MOCK_METHOD1(calculateContextBrandParityForAllItins,
      bool(Diag892Collector*));
  MOCK_METHOD1(calculateParityBrandsOverrideForAllItins,
      bool(Diag892Collector*));
  MOCK_METHOD1(removeItinsWithNoBrands,
      void(Diag892Collector*));
  MOCK_METHOD2(removeBrandsNotMatchingParity,
      bool(BrandCodesPerFareMarket&, Diag892Collector*));
  MOCK_CONST_METHOD1(getFareComponentShoppingContext,
      const FareComponentShoppingContext*(size_t));
  MOCK_CONST_METHOD0(getFixedLegs,
      const std::vector<bool>&());
  MOCK_CONST_METHOD0(getTrx,
      PricingTrx&());
};


class FakeBrandedFaresComparator
{
public:
  bool operator()(const BrandCode& brand1, const BrandCode& brand2)
  {
    return !(brand1 < brand2);
  }
};


class MockTNBrandsFunctions
{
public:
  MOCK_CONST_METHOD1(calculateMaxBrandsCountPerCarrier,
      size_t(const SegmentOrientedBrandCodeArraysPerCarrier&));
  MOCK_CONST_METHOD2(calculateMaxBrandsCountPerCarrier,
      size_t(const SegmentOrientedBrandCodeArraysPerCarrier&, size_t));
  MOCK_CONST_METHOD3(calculateProportionalIndex,
      size_t(size_t, size_t, size_t));
  MOCK_CONST_METHOD3(calculateSegmentOrientedBrandCodesPerCarrier,
      void(const MockItinGeometryCalculator<FareMarket>&,
           const SegmentOrientedBrandCodesPerCarrier*,
           SegmentOrientedBrandCodesPerCarrier&));
  MOCK_CONST_METHOD3(sortSegmentOrientedBrandCodesPerCarrier,
      void(const SegmentOrientedBrandCodesPerCarrier&,
           BrandedFaresComparator&,
           SegmentOrientedBrandCodeArraysPerCarrier&));
  MOCK_CONST_METHOD1(fillEmptyBrandsArraysPerCarrier,
      void(SegmentOrientedBrandCodeArraysPerCarrier&));
  MOCK_CONST_METHOD1(updateReservedBrandingOptionSpace,
      void(BrandingOptionSpaces&));
  MOCK_CONST_METHOD3(selectBrandsValidForReqCabin,
      void(PricingTrx*, Itin*, SegmentOrientedBrandCodesPerCarrier&));
  MOCK_CONST_METHOD5(selectBrandsPerCabin,
      void(const PricingTrx&,
           const Itin*,
           size_t,
           SegmentOrientedBrandCodesPerCarrierInCabin&,
           Diag892Collector*));
  MOCK_CONST_METHOD5(filterSortedSegmentOrientedBrandCodesPerCarrierByCabin,
      bool(const SegmentOrientedBrandCodesPerCarrier&,
           const SegmentOrientedBrandCodeArraysPerCarrier&,
           SegmentOrientedBrandCodeArraysPerCarrier&,
           SegmentOrientedBrandCodeArraysPerCarrier&,
           bool));
  MOCK_CONST_METHOD5(filterCarrierBrandCodesByCabinAtSegment,
      bool (const BrandCodesPerCarrier&,
            const CarrierDirection&,
            BrandCodeArraysPerCarrier&,
            BrandCodeArraysPerCarrier&,
            bool));
  MOCK_CONST_METHOD2(isAnySegmentOnFixedLeg,
      bool(const std::vector<TravelSeg*>&, const std::vector<bool>&));
  MOCK_CONST_METHOD2(isAnySegmentOnNonFixedLeg,
      bool(const std::vector<TravelSeg*>&, const std::vector<bool>&));
  MOCK_CONST_METHOD2(isAnySegmentOnCurrentlyShoppedLeg,
      bool(const std::vector<TravelSeg*>&, const std::vector<bool>&));
  MOCK_CONST_METHOD2(setUpdateNoBrand,
      void(CarrierBrandPairs&, const CarrierDirection&));
};


class MockSpaceBlockCalculator
{
public:
  // 5th argument has default value
  MOCK_CONST_METHOD4(calculateSpaceBlock,
      CarrierBrandPairs(const BrandCodeArraysPerCarrier&, size_t, size_t, size_t));
  MOCK_CONST_METHOD5(calculateSpaceBlock,
      CarrierBrandPairs(const BrandCodeArraysPerCarrier&, size_t, size_t, size_t, bool));
};


class MockBrandingOptionSpacesCalculator
{
public:
  // 4th argument has default value
  MOCK_CONST_METHOD3(calculateSpaces,
      void(const SegmentOrientedBrandCodeArraysPerCarrier&,
           BrandingOptionSpaces&, size_t));
  MOCK_CONST_METHOD4(calculateSpaces,
      void(const SegmentOrientedBrandCodeArraysPerCarrier&,
           BrandingOptionSpaces&, size_t, bool));

};


class MockItinBranding
{
public:
  MOCK_METHOD4(calculateBrandingOptionSpaces,
      void(const SegmentOrientedBrandCodesPerCarrier*,
           const SegmentOrientedBrandCodesPerCarrierInCabin&,
           size_t,
           Logger&));
  MOCK_CONST_METHOD1(getBrandingOptionSpace,
      const BrandingOptionSpace&(size_t));
  MOCK_CONST_METHOD1(getBrandingOptionSpaceCabin,
      size_t(size_t));
  MOCK_CONST_METHOD0(getBrandingOptionSpacesCount,
      size_t());
  MOCK_CONST_METHOD2(getCarriersBrandsForSegment,
      const CarrierBrandPairs&(size_t, const TravelSeg*));
  MOCK_METHOD1(getFmpMatrix,
      FmpMatrixPtr(size_t));
  MOCK_METHOD2(setFmpMatrix,
      void(FmpMatrixPtr, size_t));
  MOCK_METHOD0(calculateBrandParity,
      void());
  MOCK_METHOD0(updateProgramsForCalculatedBrands,
      void());
  MOCK_CONST_METHOD0(getParityBrandsPerFareMarket,
      const BrandCodesPerFareMarket&());
  MOCK_CONST_METHOD0(getParityBrands,
      const UnorderedBrandCodes&());
  MOCK_CONST_METHOD1(isThruFareMarket,
      bool(FareMarket&));
  MOCK_METHOD2(getQualifiedBrandIndicesForCarriersBrand,
      const QualifiedBrandIndices&(const CarrierCode&, const BrandCode&));
  MOCK_CONST_METHOD0(getBrandsForCurrentlyShoppedLegPerFareMarket,
      const BrandCodesPerFareMarket&());
  MOCK_CONST_METHOD0(getBrandsForCurrentlyShoppedLeg,
      const UnorderedBrandCodes&());
};

class MockBrandedFareMarket
{
public:
  MOCK_CONST_METHOD0(getBrands,
    const UnorderedBrandCodes());
  MOCK_CONST_METHOD0(getStartSegmentIndex,
    size_t());
  MOCK_CONST_METHOD0(getEndSegmentIndex,
    size_t());
  MOCK_CONST_METHOD0(getFareMarket,
    const FareMarket*());
};

class MockIFareMarketsParityCalculator : public IFareMarketsParityCalculator
{
public:
  MOCK_METHOD2(possibleBrands,
    FareMarketsPerBrandCode(size_t, size_t));
  MOCK_METHOD1(possibleBrands,
    FareMarketsPerBrandCode(size_t));
  MOCK_METHOD1(addFareMarket,
    void(const FareMarket&));
};

class MockFareMarketsParityCalculatorFactory
{
public:
  typedef MockIFareMarketsParityCalculator Type;
  MOCK_METHOD1(create,
    Type*(const MockItinGeometryCalculator<FareMarket>&));
};

class MockBrandedFareMarketFactory
{
public:
  typedef MockBrandedFareMarket Type;
  MOCK_METHOD2(create,
    Type*(const MockItinGeometryCalculator<FareMarket>&, const FareMarket&));
};

} /* namespace skipper */

} // namespace tse

#endif /* COMMON_TNBRANDS_TNBRANDSMOCKS_H_ */
