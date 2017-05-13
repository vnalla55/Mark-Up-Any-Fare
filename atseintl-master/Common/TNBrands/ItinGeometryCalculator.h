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

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Assert.h"
#include "Common/TNBrands/TNBrandsInterfaces.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/TravelSeg.h"

#include <boost/scoped_ptr.hpp>

#include <map>
#include <vector>

namespace tse
{

namespace skipper
{

// A "bridge" exposing brand-related data/functionality of an Itin object
// in a convenient way.
//
// Shall be created as a proxy to one particular Itin, passed to the
// constructor. trxCalculator is used as a source of data belonging to the
// transaction which is common to all itins. This dependency is usually
// shared between multiple itin calculators.
class BrandInItinInclusionPolicy
{
public:
  // Tells whether brandCode has been requested for an itin using brands
  // to programs relations info calculated for some context (e.g. fare market).
  // Itin defines a map with filtering rules for brands and programs which is
  // given as brandsAllowedForItin.
  // For processing details see comments in code.
  bool isBrandCodeInItin(
      const BrandCode& brandCode,
      const BrandProgramRelations& relations,
      const BrandFilterMap& brandsAllowedForItin) const;

  // Returns set of qualified brands indices (defined in trx.brandProgramVec())
  // for given brandCode with applied filtering (if not empty it tells whether
  // brandCode has been requested for an itin using brands to programs relations).
  // Itin defines a map with filtering rules for brands and programs which is
  // given as brandsAllowedForItin.
  // For processing details see comments in code.
  QualifiedBrandIndices getIndicesForBrandCodeWithFiltering(
      const BrandCode& brandCode,
      const BrandProgramRelations& relations,
      const BrandFilterMap& brandsAllowedForItin) const;
};

template <typename BrandInItinInclusionPolicyT,
          typename FareMarketType = FareMarket>
class ItinGeometryCalculatorTemplate
{
public:
  typedef FareMarketType FareMarketT;
  ItinGeometryCalculatorTemplate(Itin& itin,
                         const ITrxGeometryCalculator& trxCalculator,
                         BrandInItinInclusionPolicyT* inclusionPolicy = 0):
                           _itin(itin), _trxCalculator(trxCalculator),
                           _isCarrierOrientedProgramsPerBrandFilled(false)
  {
    assignValidObject(inclusionPolicy, _brandInItinInclusionPolicy);
  }

  // Returns the total number of segments in the itin.
  const size_t getSegmentCount() const
  {
    return _itin.travelSeg().size();
  }

  // Returns itin's fare markets list.
  const std::vector<FareMarketType*>& getFareMarkets() const
  {
    return _itin.fareMarket();
  }

  // Returns the segment's index in itin.
  size_t getTravelSegmentIndex(const TravelSeg* segment) const;

  // Returns leg id for travel segment corresponding to given index
  size_t getTravelSegmentLegId(size_t segmentIndex) const
  {
    TSE_ASSERT(segmentIndex < _itin.travelSeg().size());
    TSE_ASSERT(_itin.travelSeg()[segmentIndex] != nullptr);
    return _itin.travelSeg()[segmentIndex]->legId();
  }

  // Calculates a vector of pairs - each pair contains starting and ending
  // segment index for specific leg. Leg index is determined by the position of
  // the pair in returned vector.
  std::vector<std::pair<size_t, size_t> > calculateLegsStartEndSegmentIndices() const;

  // Calculates a vector of pnr segments for succeeding segments of specific itin.
  std::vector<size_t> calculatePnrForSegments() const;

  // Calculates a pair of staring and ending segment indices for not fixed part
  // of Context Shopping travel. If all legs are fixed a pair of
  // TRAVEL_SEG_DEFAULT_ID is returned.
  std::pair<size_t, size_t> calculateNonFixedSegmentsForContextShopping() const;

  // For all itin-specific fare markets a set of brands is reduced it this fare
  // market crosses fixed leg and a brand is specified for this leg.
  void reduceFareMarketsBrandsOnFixedLegs();

  // Checks if given fare market is on fixed leg (any part of fare market is on
  // one or more of fixed legs). If so, returns true and sets fixedBrand to
  // value of fixed brand (if received in request) or to NO_BRAND if brand was
  // not fixed on this leg.
  // Another case for fixedBrand being set to NO_BRAND is when fm crosses two
  // fixed legs but on each of those legs different brand is fixed.
  bool isFareMarketOnFixedLeg(const FareMarketType& fm, BrandCode& fixedBrand) const;

  // Modifies fare market brands vector - leaves only fixedBrand (as this vector
  // contains indices to trx qualifiedBrands vector fixed brand can be present
  // several times (in different programs)). If fixedBrand was not present in
  // fare market brands vector it should be empty.
  void reduceFareMarketBrands(FareMarketType& fm, const BrandCode& fixedBrand);

  // Returns fare market first segment's index in itin.
  size_t getFareMarketStartSegmentIndex(const FareMarketType& fm) const
  {
    return getTravelSegmentIndex(fm.travelSeg().front());
  }

  // Returns fare market last segment's index in itin.
  size_t getFareMarketEndSegmentIndex(const FareMarketType& fm) const
  {
    return getTravelSegmentIndex(fm.travelSeg().back());
  }

  // Returns itin-specific brand codes for the fare market.
  // These are brands specified in the request for a given itinerary
  // or all brands associated with the fare market if no such
  // brands were specified.
  // It is possible that the result set is empty (if no brands are
  // available for faremarket or all such brands were filtered out).
  UnorderedBrandCodes getItinSpecificBrandCodes(const FareMarketType& fm) const;

  // Returns indices of itin-specific brand codes for the fare market.
  // These are brands specified in the request for a given itinerary
  // or all brands associated with the fare market if no such
  // brands were specified.
  // It is possible that the result set is empty (if no brands are
  // available for faremarket or all such brands were filtered out).
  QualifiedBrandIndices getItinSpecificBrandCodesIndices(const FareMarketType& fm) const;

  // Returns the calculator facade for its trx.
  const ITrxGeometryCalculator& getTrxGeometryCalculator() const
  {
    return _trxCalculator;
  }

  // Returns information if travel segment (given as an index within
  // travel segments vector in itin, see getTravelSegmentIndex()) is
  // an Arunk segment.
  bool isTravelSegmentArunk(size_t segmentIndex) const;

  // If a given travel segment is not an arunk and within the travel it is
  // returned. If it is an arunk, the next travel segment is returned (two
  // arunk segments in a row are not possible). If the given segment is outside
  // of the travel or is an arunk and it is the last segment, getSegmentCount()
  // is returned.
  size_t getNextTravelSegmentIfCurrentArunk(size_t segmentIndex) const;

  // Adds brand,program pair to internal itin programsForBrandMap (wiht brand
  // as a key. Multiple programs possible for single brand in map.
  void addBrandProgramPair(const BrandCode& brandCode, const ProgramID& programId)
  {
    _itin.getProgramsForBrandMap()[brandCode].insert(programId);
  }

  // Verifies if given fare market is "thru" type (covers any number of whole
  // legs). Verification needs to be done in a context of any init this fare
  // market belongs to (thats why it is itins, not fare markets property) but
  // the property of being "thru" is consistent in a context of all itins
  // containing this fare market, e.g. either this fare market is "thru" in a
  // context of all itins containing it or it is not "thru" in a context of all
  // all itins containing it. It is not possible that the same fare market in a
  // context of two different itins will return different "thru" value.
  bool isThruFareMarket(const FareMarketType& fareMarket) const;

  bool isThruFareOnlyItin() const { return _itin.isThruFareOnly(); }

  // This function populates internal carrier/brand/qualifiedBrandIdices
  // relation.
  void populateQualifiedBrandIndices() const;

  // Returns valid (filtered, see getItinSpecificBrandCodes) indices of
  // brand/program element in trx.brandProgramVec() for given carrier and its
  // brand.
  // Throws if either carrier or brand for this carrier is not defined.
  const QualifiedBrandIndices& getQualifiedBrandIndicesForCarriersBrand(
      const CarrierCode& carrier, const BrandCode& brand) const
  {
    if (!_isCarrierOrientedProgramsPerBrandFilled)
    {
      _isCarrierOrientedProgramsPerBrandFilled = true;
      populateQualifiedBrandIndices();
    }
    TSE_ASSERT(_carrierOrientedProgramsPerBrand.find(carrier) !=
        _carrierOrientedProgramsPerBrand.end());
    TSE_ASSERT(_carrierOrientedProgramsPerBrand.at(carrier).find(brand) !=
        _carrierOrientedProgramsPerBrand.at(carrier).end());

    return _carrierOrientedProgramsPerBrand.at(carrier).at(brand);
  }

  // Return itin brand filter map.
  const BrandFilterMap& getBrandFilterMap() const
  {
    return _itin.brandFilterMap();
  }

  // Create map of brands available on specific segments for all fare markets
  // within this itin.
  std::map<uint16_t, UnorderedBrandCodes> calculateSegmentToBrandsMapping() const;

  // Calculates program direction on given fare market. Moved here, instead
  // of direct call to program->calculateDirectionality() to mock in unit tests.
  bool getProgramDirection(const BrandProgram* program,
                           const FareMarketType& fm,
                           Direction& direction) const
  {
    return program->calculateDirectionality(fm, direction);
  }

private:
  Itin& _itin;
  const ITrxGeometryCalculator& _trxCalculator;
  boost::scoped_ptr<BrandInItinInclusionPolicyT> _brandInItinInclusionPolicy;

  typedef std::map<BrandCode, QualifiedBrandIndices> IndicesPerBrand;
  mutable bool _isCarrierOrientedProgramsPerBrandFilled;
  mutable std::map<CarrierCode, IndicesPerBrand> _carrierOrientedProgramsPerBrand;
};

typedef ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicy> ItinGeometryCalculator;

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
size_t ItinGeometryCalculatorTemplate<
  BrandInItinInclusionPolicyT, FareMarketType>::getTravelSegmentIndex(
    const TravelSeg* segment) const
{
  // Instead of using _itin.segmentOrder(), which is unclear (contains
  // undocumented logic) we use simpler approach. In case of any problems
  // revert this code to segmentOrder() call.
  TSE_ASSERT(segment != nullptr);
  const std::vector<TravelSeg*>::const_iterator found =
      std::find(_itin.travelSeg().begin(), _itin.travelSeg().end(), segment);
  TSE_ASSERT(found != _itin.travelSeg().end());
  return found - _itin.travelSeg().begin();
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
std::vector<std::pair<size_t, size_t> > ItinGeometryCalculatorTemplate<
  BrandInItinInclusionPolicyT, FareMarketType>::calculateLegsStartEndSegmentIndices() const
{
  std::map<size_t, std::pair<size_t, size_t> > legInfo;
  std::map<size_t, std::pair<size_t, size_t> >::iterator found;
  size_t lastLeg = 0;
  for (size_t segmentIndex = 0; segmentIndex < _itin.travelSeg().size(); ++segmentIndex)
  {
    TSE_ASSERT(_itin.travelSeg()[segmentIndex] != nullptr);
    size_t legIndex = _itin.travelSeg()[segmentIndex]->legId();
    if (legIndex > lastLeg)
      lastLeg = legIndex;

    found = legInfo.find(legIndex);
    if (found == legInfo.end())
      legInfo[legIndex] = std::make_pair(segmentIndex, segmentIndex);
    else
    {
      if (legInfo.at(legIndex).second < segmentIndex)
        legInfo[legIndex].second = segmentIndex;
    }
  }
  std::vector<std::pair<size_t, size_t> > result;
  for (size_t legIndex = 0; legIndex <= lastLeg; ++legIndex)
  {
    found = legInfo.find(legIndex);
    TSE_ASSERT(found != legInfo.end());
    result.push_back(found->second);
  }
  return result;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
std::vector<size_t> ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::calculatePnrForSegments() const
{
  std::vector<size_t> pnrInfo;
  for (const auto elem : _itin.travelSeg())
  {
    TSE_ASSERT(elem != nullptr);
    pnrInfo.push_back(elem->pnrSegment());
  }
  return pnrInfo;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
QualifiedBrandIndices ItinGeometryCalculatorTemplate<
BrandInItinInclusionPolicyT, FareMarketType>::getItinSpecificBrandCodesIndices(
  const FareMarketType& fm) const
{
  BrandProgramRelations relations;
  _trxCalculator.getBrandsAndPrograms(fm, relations);

  const std::vector<QualifiedBrand>& allBrands = _trxCalculator.getQualifiedBrands();

  QualifiedBrandIndices itinSpecificBrandIndices;
  for (int index: fm.brandProgramIndexVec())
  {
    if (_brandInItinInclusionPolicy->isBrandCodeInItin(
        allBrands[index].second->brandCode(), relations, _itin.brandFilterMap()))
    {
      itinSpecificBrandIndices.insert(index);
    }
  }
  return itinSpecificBrandIndices;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
UnorderedBrandCodes ItinGeometryCalculatorTemplate<
  BrandInItinInclusionPolicyT, FareMarketType>::getItinSpecificBrandCodes(
    const FareMarketType& fm) const
{
  BrandProgramRelations relations;
  _trxCalculator.getBrandsAndPrograms(fm, relations);
  const UnorderedBrandCodes allFareMarketBrands = relations.getAllBrands();
  UnorderedBrandCodes itinSpecificBrands;

  for (const BrandCode& brandCode : allFareMarketBrands)
  {
    if (_brandInItinInclusionPolicy->isBrandCodeInItin(
        brandCode, relations, _itin.brandFilterMap()))
    {
      itinSpecificBrands.insert(brandCode);
    }
  }
  return itinSpecificBrands;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
bool ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::isTravelSegmentArunk(
    size_t segmentIndex) const
{
  TSE_ASSERT(segmentIndex < _itin.travelSeg().size());
  const TravelSeg* segment = _itin.travelSeg()[segmentIndex];
  if (segment->segmentType() == Arunk ||
      (segment->segmentType() == UnknownTravelSegType && segment->toArunkSeg()))
  {
    return true;
  }
  return false;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
size_t ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::getNextTravelSegmentIfCurrentArunk(
    size_t segmentIndex) const
{
  size_t segCount = getSegmentCount();
  if (segmentIndex >= segCount)
  {
    return segCount;
  }
  if (isTravelSegmentArunk(segmentIndex))
  {
    segmentIndex++; //next of segCount if last
  }
  return segmentIndex;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
bool ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::isThruFareMarket(const FareMarketType& fareMarket) const
{
  size_t startIndex = getFareMarketStartSegmentIndex(fareMarket);
  size_t endIndex = getFareMarketEndSegmentIndex(fareMarket);

  // Now check if the fare market 'fills completely' a leg

  // Check if it starts a leg, i.e. either first segment or previous belongs to
  // different leg. If not, no chance for thru.
  if ((startIndex != 0) &&
      (getTravelSegmentLegId(startIndex - 1) == getTravelSegmentLegId(startIndex))
     )
  {
    return false;
  }

  // Check if it finished a leg, i.e. either last segment or next belongs to
  // different leg. If not, no chance for thru.
  if ((endIndex != getSegmentCount() - 1) &&
      (getTravelSegmentLegId(endIndex + 1) == getTravelSegmentLegId(endIndex))
     )
  {
    return false;
  }

  //Both conditions met - starts and finishes a leg = thru
  return true;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
void ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::populateQualifiedBrandIndices() const
{
  for (const FareMarketType* fm : getFareMarkets())
  {
    TSE_ASSERT(fm != nullptr);

    BrandProgramRelations relations;
    _trxCalculator.getBrandsAndPrograms(*fm, relations);
    const UnorderedBrandCodes allFareMarketBrands = relations.getAllBrands();

    for (const BrandCode& brandCode : allFareMarketBrands)
    {
      QualifiedBrandIndices qbIndices =
          _brandInItinInclusionPolicy->getIndicesForBrandCodeWithFiltering(
              brandCode, relations, _itin.brandFilterMap());
      _carrierOrientedProgramsPerBrand[fm->governingCarrier()][brandCode].insert(
          qbIndices.begin(), qbIndices.end());
    }
  }
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
std::pair<size_t, size_t> ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::calculateNonFixedSegmentsForContextShopping() const
{
  const std::vector<bool>& fixedLegs = _trxCalculator.getFixedLegs();
  TSE_ASSERT(!fixedLegs.empty());

  std::vector<std::pair<size_t, size_t> > legsStartEndSegmentIndices =
      calculateLegsStartEndSegmentIndices();

  size_t startingSegmentIndex = TRAVEL_SEG_DEFAULT_ID;
  for (size_t legIndex = 0; legIndex < fixedLegs.size(); ++legIndex)
  {
    if (!fixedLegs.at(legIndex))
    {
      startingSegmentIndex = legsStartEndSegmentIndices.at(legIndex).first;
      break;
    }
  }
  if (startingSegmentIndex == TRAVEL_SEG_DEFAULT_ID)
  {
    //all legs fixed
    return std::make_pair(TRAVEL_SEG_DEFAULT_ID, TRAVEL_SEG_DEFAULT_ID);
  }

  size_t endingSegmentIndex = legsStartEndSegmentIndices.front().first;
  for (size_t legIndex = fixedLegs.size() - 1; legIndex >= 0; --legIndex)
  {
    if (!fixedLegs.at(legIndex))
    {
      endingSegmentIndex = legsStartEndSegmentIndices.at(legIndex).second;
      break;
    }
  }
  //endingSegmentIndex is not inclusive when calculating parity
  ++endingSegmentIndex;
  return std::make_pair(startingSegmentIndex, endingSegmentIndex);
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
void ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::reduceFareMarketsBrandsOnFixedLegs()
{
  for (FareMarketType* fm: _itin.fareMarket())
  {
    TSE_ASSERT(fm != nullptr);
    BrandCode brand = NO_BRAND;
    if (isFareMarketOnFixedLeg(*fm, brand))
    {
      reduceFareMarketBrands(*fm, brand);
    }
  }
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
bool ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::isFareMarketOnFixedLeg(const FareMarketType& fm, BrandCode& fixedBrand) const
{
  size_t fmStartSegmentIndex = getFareMarketStartSegmentIndex(fm);
  size_t fmEndSegmentIndex = getFareMarketEndSegmentIndex(fm);
  size_t fmStartLeg = getTravelSegmentLegId(fmStartSegmentIndex);
  size_t fmEndLeg = getTravelSegmentLegId(fmEndSegmentIndex);

  const std::vector<bool>& fixedLegs = _trxCalculator.getFixedLegs();

  TSE_ASSERT(fmStartLeg < fixedLegs.size());
  TSE_ASSERT(fmEndLeg < fixedLegs.size());
  TSE_ASSERT(fmStartLeg <= fmEndLeg);

  std::vector<std::pair<size_t, size_t> > legsStartEndSegmentIndices =
      calculateLegsStartEndSegmentIndices();

  std::vector<size_t> pnrForSegment = calculatePnrForSegments();
  //segments are counted from 0, size is 1..
  TSE_ASSERT(pnrForSegment.size() == legsStartEndSegmentIndices.back().second + 1);

  //NO_BRAND indicates that no brand matched the fixed criteria - either FM is
  //not on fixed leg or requested brands on multiple fixed legs crossed by this
  //fare market have empty intersection.
  fixedBrand = NO_BRAND;
  bool isOnFixed = false;
  for (size_t legIndex = fmStartLeg; legIndex <= fmEndLeg; ++legIndex)
  {
    if (fixedLegs.at(legIndex))
    {
      //this FM is on fixed leg
      isOnFixed = true;
      for (size_t segmentIndex = legsStartEndSegmentIndices.at(legIndex).first;
           segmentIndex <= legsStartEndSegmentIndices.at(legIndex).second;
           ++segmentIndex)
      {
        //get context shopping data for all segments on this leg
        const FareComponentShoppingContext* shoppingContext =
            _trxCalculator.getFareComponentShoppingContext(
                pnrForSegment.at(segmentIndex));
        if (shoppingContext && !shoppingContext->brandCode.empty())
        {
          //if context shopping is defined for this segment (it should be! as
          //this is the segment on fixed leg) and brand code is filled (fare
          //can be used instead) verify if it is consistent with brand
          //previously found for this FM
          //if fixedBrand == NO_BRAND this is the first brand information for
          //this FM so we can use it.
          //otherwise brand info must be the same as previous - if not, break
          //the loops and set fixedBrand to NO_BRAND to indicate that this
          //FM has no valid brands matching fixed criteria. it can happend if
          //FM crosses multiple fixed legs with different brands fixed.
          BrandCode brandOnFixedLeg = shoppingContext->brandCode;
          if (fixedBrand == NO_BRAND)
            fixedBrand = brandOnFixedLeg;
          else
          {
            if (fixedBrand != brandOnFixedLeg)
            {
              // no intersection of brands for different fixed legs
              fixedBrand = NO_BRAND;
              return isOnFixed;
            }
          }
        }
      }
    }
  }

  if (isOnFixed && (fixedBrand != NO_BRAND))
  {
    bool found = false;
    // verify if fixedBrand is available on this FM
    const std::vector<QualifiedBrand>& qualifiedBrands = _trxCalculator.getQualifiedBrands();
    for (int brandIndex: fm.brandProgramIndexVec())
    {
      TSE_ASSERT(qualifiedBrands[brandIndex].second != nullptr);
      if (fixedBrand == qualifiedBrands[brandIndex].second->brandCode())
      {
        found = true;
        break;
      }
    }
    if (!found)
      fixedBrand = NO_BRAND;
  }

  return isOnFixed;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
void ItinGeometryCalculatorTemplate<BrandInItinInclusionPolicyT,
  FareMarketType>::reduceFareMarketBrands(FareMarketType& fm, const BrandCode& fixedBrand)
{
  const std::vector<QualifiedBrand>& qualifiedBrands =
      _trxCalculator.getQualifiedBrands();
  std::vector<int> newBrandProgramIndexVec;
  if (fixedBrand != NO_BRAND)
  {
    for (int brandIndex: fm.brandProgramIndexVec())
    {
      TSE_ASSERT(qualifiedBrands[brandIndex].second != nullptr);
      BrandCode brandCode = qualifiedBrands[brandIndex].second->brandCode();
      if (fixedBrand == brandCode)
      {
        newBrandProgramIndexVec.push_back(brandIndex);
      }
    }
  }
  fm.brandProgramIndexVec() = newBrandProgramIndexVec;
}

template <typename BrandInItinInclusionPolicyT, typename FareMarketType>
std::map<uint16_t, UnorderedBrandCodes> ItinGeometryCalculatorTemplate<
  BrandInItinInclusionPolicyT, FareMarketType>::calculateSegmentToBrandsMapping() const
{
  std::map<uint16_t, UnorderedBrandCodes> segmentBrandsMap;

  const std::vector<QualifiedBrand>& qualifiedBrands =
      getTrxGeometryCalculator().getQualifiedBrands();

  for (const FareMarket* fm: getFareMarkets())
  {
    size_t startSegmentIndex = getTravelSegmentIndex(fm->travelSeg().front());
    size_t endSegmentIndex = getTravelSegmentIndex(fm->travelSeg().back());

    UnorderedBrandCodes fmBrands;
    for (int index: fm->brandProgramIndexVec())
    {
      TSE_ASSERT(index < int(qualifiedBrands.size()));
      BrandCode brand = qualifiedBrands[index].second->brandCode();
      fmBrands.insert(brand);
    }

    for (size_t segment = startSegmentIndex; segment <= endSegmentIndex; ++segment)
      segmentBrandsMap[segment].insert(fmBrands.begin(), fmBrands.end());
  }

  return segmentBrandsMap;
}


} /* namespace skipper */

} // namespace tse

