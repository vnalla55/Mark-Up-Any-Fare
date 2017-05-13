//----------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DataModel/ShoppingTrx.h"

#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Pricing/ShoppingPQ.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace tse
{
static Logger
logger("atseintl.DataModel.ShoppingTrx.Leg");

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
uint32_t
ShoppingTrx::Leg::getFlightBitmapSize(ShoppingTrx& trx, const ItinIndex::Key& itinRowKey)
{
  TSELatencyData metrics(trx, "SHOPTRX CALCBITMAP SZ");

  // No need to compute this twice
  std::map<ItinIndex::Key, uint32_t>::const_iterator fBitSizeIter =
      _flightBitmapSizeMap.find(itinRowKey);
  if (fBitSizeIter != _flightBitmapSizeMap.end())
    return (fBitSizeIter->second);

  uint32_t flightSize = 0;

  if (!_stopOverLegFlag)
  {
    flightSize = _carrierIndex.retrieveNumberCellsInARow(itinRowKey);
  }
  else
  {
    uint32_t pickedIndex = _adoptedCrossedLegRefIndex;

    // Prepare to iterate over the leg indices we are jumping across
    IndexVector counts(0);
    for (const auto curIndex : _jumpedLegIndices)
    {
      if (curIndex == ASOLEG_SURFACE_SECTOR_ID)
      {
        continue;
      }

      ShoppingTrx::Leg& curLeg = trx.legs()[curIndex];
      ItinIndex& cxrIdx = curLeg.carrierIndex();

      // Push the adoptedLeg carrier index governing carrier's row size
      if (curIndex == pickedIndex)
      {
        counts.push_back(cxrIdx.retrieveNumberCellsInARow(itinRowKey));
        continue;
      }

      // Push the other legs entire number of cells onto the mult vector
      counts.push_back(cxrIdx.retrieveTotalNumberCells());
    }

    // Perform the count vector multiplication

    const bool actualMultiply = std::any_of(counts.cbegin(),
                                            counts.cend(),
                                            [](const uint32_t c)
                                            { return c > 0; });

    flightSize = actualMultiply ? std::accumulate(counts.cbegin(),
                                                  counts.cend(),
                                                  static_cast<uint32_t>(1),
                                                  [](const uint32_t acc, const uint32_t factor)
                                                  { return factor > 0 ? acc * factor : acc; })
                                : 0;
  }

  // Store result for re-use
  _flightBitmapSizeMap[itinRowKey] = flightSize;

  LOG4CXX_INFO(logger, "- Flight bitmap size computed = " << flightSize);
  return (flightSize);
}

namespace
{

// a functor to sort SOP indices for across stop over legs
class SopIndicesLess
{
public:
  SopIndicesLess(const ShoppingTrx& trx,
                 const ShoppingTrx::Leg& asoLeg,
                 const std::vector<uint32_t>& adoptedLegSopMap)
    : _trx(trx), _leg(asoLeg), _adoptedLegSopMap(adoptedLegSopMap)
  {
  }

  bool operator()(const IndexVector& a, const IndexVector& b) const;

private:
  uint32_t numConnections(const IndexVector& v) const;
  const ShoppingTrx::SchedulingOption& getSop(uint32_t leg, uint32_t sop) const;
  const ShoppingTrx& _trx;
  const ShoppingTrx::Leg& _leg;
  const std::vector<uint32_t>& _adoptedLegSopMap;
};

bool
SopIndicesLess::
operator()(const IndexVector& a, const IndexVector& b) const
{
  TSE_ASSERT(a.size() == b.size());

  const uint32_t conna = numConnections(a);
  const uint32_t connb = numConnections(b);
  if (conna < connb)
  {
    return true;
  }
  else if (conna > connb)
  {
    return false;
  }

  return std::accumulate(a.begin(), a.end(), 0) < std::accumulate(b.begin(), b.end(), 0);
}

uint32_t
SopIndicesLess::numConnections(const IndexVector& v) const
{
  uint32_t result = 0;

  TSE_ASSERT(v.size() == _leg.jumpedLegIndices().size());
  for (size_t n = 0; n != v.size(); ++n)
  {
    const uint32_t leg = _leg.jumpedLegIndices()[n];
    if (leg == ASOLEG_SURFACE_SECTOR_ID)
    {
      continue;
    }

    const ShoppingTrx::SchedulingOption& sop = getSop(leg, v[n]);
    result += static_cast<uint32_t>(sop.itin()->travelSeg().size());
  }

  return result;
}

const ShoppingTrx::SchedulingOption&
SopIndicesLess::getSop(uint32_t legNum, uint32_t sop) const
{
  const ShoppingTrx::Leg& leg = _trx.legs()[legNum];
  if (legNum != _leg.adoptedCrossedLegRefIndex())
    return leg.sop()[sop];

  TSE_ASSERT(sop < _adoptedLegSopMap.size());
  uint32_t sopId = _adoptedLegSopMap[sop];
  return leg.sop()[sopId];
}

} // anon ns

ShoppingTrx::INTERLINEONLINE
ShoppingTrx::SchedulingOption::checkInterline() const
{
  CarrierCode firstCarrier;
  for (const auto travelSeg : _itin->travelSeg())
  {
    if (travelSeg->isAir())
    {
      const AirSeg* currentAirSeg(static_cast<const AirSeg*>(travelSeg));
      const CarrierCode& currentCarrier(currentAirSeg->marketingCarrierCode());
      if (firstCarrier.empty())
      {
        firstCarrier = currentCarrier;
      }
      else if (firstCarrier != currentCarrier)
      {
        return INTERLINE;
      }
    }
  }
  return ONLINE;
}

bool
ShoppingTrx::SchedulingOption::isInterline() const
{
  if (NOTDETERMINED == _interline)
  {
    _interline = checkInterline();
  }
  return INTERLINE == _interline;
}

void
ShoppingTrx::Leg::generateAcrossStopOverCombinations(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "SHOPTRX GENASO COMBOS");

  TSE_ASSERT(_stopOverLegFlag);

  const ItinIndex::ItinMatrix& indexRoot = _carrierIndex.root();

  if (indexRoot.empty())
  {
    LOG4CXX_ERROR(logger, "Carrier index is empty.");
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Carrier index is empty");
  }

  size_t jumpedSz = _jumpedLegIndices.size();
  if (_jumpedLegIndices.empty())
  {
    LOG4CXX_ERROR(logger, "Jumped leg index vector for across stop over leg is empty.");
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Jumped leg index vector is empty");
  }

  std::vector<std::vector<ItinIndex::Key> > encodedGovCxrs;
  ShoppingUtil::getEncodedGovCxrs(trx, encodedGovCxrs);

  std::vector<uint32_t> adoptedLegSopMap;

  // We need to iterate through the across stop over leg governing carriers,
  // skipping the adopted leg
  ItinIndex::ItinMatrixConstIterator iMIter = indexRoot.begin();
  ItinIndex::ItinMatrixConstIterator iMEIter = indexRoot.end();
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  // LOG4CXX_DEBUG(logger, "-* Looping through across stop over leg carrier index, size =
  // "<<indexRootSz);

  // Loop through governing carriers in this across stop over leg
  for (uint32_t z = 0; iMIter != iMEIter; ++iMIter, ++z)
  {
    const uint32_t& rowKey = iMIter->first;
    IndexVectors& dataStore = _acrossStopOverCombinations[rowKey];
    //    dataStore.clear(); //Unnecessary
    dataStore.reserve(jumpedSz);
    uint32_t flightBitmapSize = getFlightBitmapSize(trx, rowKey);
    IndexVector limitSizes;
    //    limitSizes.clear(); //Unnecessary because of resize, no pushing back involved
    limitSizes.resize(jumpedSz);

    // LOG4CXX_DEBUG(logger, "--* Row Key for this loop iteration = " << rowKey);
    // LOG4CXX_DEBUG(logger, "--* Flight bitmap size for this key = " << flightBitmapSize);

    // For this across stop over leg, iterate over the legs it jumps across
    for (uint32_t index = 0; index < jumpedSz; index++)
    {
      uint32_t& jumpedLegIndex = _jumpedLegIndices[index];

      // Make limit sizes element at this index aware of a surface sector
      if (jumpedLegIndex == ASOLEG_SURFACE_SECTOR_ID)
      {
        limitSizes[index] = ASOLEG_SURFACE_SECTOR_ID;
        continue;
      }

      ShoppingTrx::Leg& jumpedLeg = legs[jumpedLegIndex];
      ItinIndex& jumpedLegCxrIdx = jumpedLeg.carrierIndex();

      // See if we are jumping the adopted leg, limit the size to the number of
      // sops governed by this carrier in the adopted leg
      if (jumpedLegIndex == _adoptedCrossedLegRefIndex)
      {
        limitSizes[index] = jumpedLegCxrIdx.retrieveNumberCellsInARow(rowKey);
        continue;
      }

      const ItinIndex::ItinMatrix& jumpedLegCxrIdxRoot = jumpedLegCxrIdx.root();

      // Ensure that the carrier index for the leg we are jumping is not empty
      if (jumpedLegCxrIdxRoot.empty() || jumpedLegCxrIdxRoot.size() == 0)
      {
        LOG4CXX_ERROR(logger,
                      "---* Leg " << jumpedLegIndex << ", which is jumped leg " << index
                                  << ", has an empty carrier index");
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "No available flights");
      }

      uint32_t totalNumberCells = jumpedLegCxrIdx.retrieveTotalNumberCells();

      // Set the limit entry to the number of carriers (rows) in
      // the jumped leg carrier index
      limitSizes[index] = totalNumberCells;
    }

    if (IS_DEBUG_ENABLED(logger))
    {
      IndexVectorIterator limitSizesIter = limitSizes.begin();
      IndexVectorIterator limitSizesEIter = limitSizes.end();
      for (; limitSizesIter != limitSizesEIter; ++limitSizesIter)
      {
        if (*limitSizesIter == ASOLEG_SURFACE_SECTOR_ID)
        {
          continue;
        }
        LOG4CXX_DEBUG(
            logger, "---* [" << (limitSizesIter - limitSizes.begin()) << "] = " << *limitSizesIter);
      }
    }

    std::vector<IndexVector> bitmapOrdering;
    IndexVector limitCtrs(jumpedSz);

    for (uint32_t k = 0; k < flightBitmapSize; k++)
    {
      bitmapOrdering.push_back(limitCtrs);

      // Update limit counters
      bool addOneNext = true;

      const size_t adoptedLegIndex = _adoptedCrossedLegRefIndex - _jumpedLegIndices.front();

      int32_t baseLimit = static_cast<int32_t>(jumpedSz) - 1;
      for (int32_t limCntIdx = baseLimit; addOneNext && limCntIdx >= 0; --limCntIdx)
      {
        if (_jumpedLegIndices[limCntIdx] == ASOLEG_SURFACE_SECTOR_ID ||
            static_cast<uint32_t>(limCntIdx) == adoptedLegIndex) // lint !e650
        {
          continue;
        }

        // Check if the carry addition must be made to the next column
        uint32_t& limCnt = limitCtrs[limCntIdx];
        limCnt++;
        if (limCnt == limitSizes[limCntIdx])
        {
          limCnt = 0;
          addOneNext = true;
        }
        else
        {
          addOneNext = false;
        }
      }

      // the adopted leg only changes here, if all other legs changed
      if (addOneNext)
      {
        uint32_t& limCnt = limitCtrs[adoptedLegIndex];
        ++limCnt;
        if (limCnt == limitSizes[adoptedLegIndex])
        {
          limCnt = 0;
        }
      }
    }

    createCxrSopMap(encodedGovCxrs[_adoptedCrossedLegRefIndex], iMIter->first, adoptedLegSopMap);

    SopIndicesLess ordering(trx, *this, adoptedLegSopMap);
    std::sort(bitmapOrdering.begin(), bitmapOrdering.end(), ordering);

    dataStore.clear();
    dataStore.resize(jumpedSz);

    for (std::vector<IndexVector>::const_iterator i = bitmapOrdering.begin();
         i != bitmapOrdering.end();
         ++i)
    {
      TSE_ASSERT(i->size() == dataStore.size());
      for (size_t n = 0; n != jumpedSz; ++n)
      {
        if (_jumpedLegIndices[n] == ASOLEG_SURFACE_SECTOR_ID)
        {
          dataStore[n].resize(1);
        }
        else
        {
          dataStore[n].push_back((*i)[n]);
        }
      }
    }

    if (IS_DEBUG_ENABLED(logger))
    {
      // Output combo table
      for (uint32_t y = 0; y < flightBitmapSize; y++)
      {
        std::ostringstream oStr;
        for (uint32_t x = 0; x < limitSizes.size(); x++) // lint !e574
        {
          if (limitSizes[x] == ASOLEG_SURFACE_SECTOR_ID)
          {
            continue;
          }
          IndexVector& vRef = dataStore[x];
          oStr << " " << vRef[y] << " ";
        }
        LOG4CXX_DEBUG(logger, oStr.str());
      }
    }
  }
}

void
ShoppingTrx::Leg::createCxrSopMap(const std::vector<ItinIndex::Key>& govCxrs,
                                  ItinIndex::Key cxr,
                                  std::vector<uint32_t>& sopMap) const
{
  sopMap.clear();
  for (uint32_t sopIdx = 0; sopIdx < govCxrs.size(); ++sopIdx)
  {
    if (govCxrs[sopIdx] == cxr)
      sopMap.push_back(sopIdx);
  }
}

ShoppingPQ*
ShoppingTrx::getCxrShoppingPQ(CarrierCode* cxr)
{
  if (!cxr)
  {
    if (_shoppingPQVector.back()->isInterline())
    {
      return &*_shoppingPQVector.back();
    }
    return nullptr;
  }
  for (auto& shoppingPQ : _shoppingPQVector)
  {
    if (!shoppingPQ->isInterline() && *cxr == *shoppingPQ->carrier())
    {
      return &*shoppingPQ;
    }
  }
  return nullptr;
}

bool
ShoppingTrx::isSumOfLocalsProcessingEnabled() const
{
  return diversity().isEnabled() &&
         !diversity().isV2DiversityModel() &&
         !diversity().isExchangeForAirlines() &&
         _simpleTrip &&
         1 == _paxType.size() &&
         legs().size() <= 2; // And we have a supported number of legs in the request
}

bool
ShoppingTrx::isLngCnxProcessingEnabled() const
{
  return getRequest()->percentOfLngCnxSolutions();
}

// Return true if a given FareMarket exists in CustomSolution FareMarket set
bool
ShoppingTrx::isCustomSolutionFM(const FareMarket* fm) const
{
  return (_customSolutionFmSet.find(fm) != _customSolutionFmSet.end());
}

// Insert the given FareMarket in the CustomSolution FareMarket set.
// If the FareMarket already exists in the set, do nothing
void
ShoppingTrx::setCustomSolutionFM(const FareMarket* fm)
{
  _customSolutionFmSet.insert(fm);
}

// Return true if a given FareMarket exists in Spanish Large Family Discount FareMarket set
bool
ShoppingTrx::isSpanishDiscountFM(const FareMarket* fm) const
{
  return (_spanishDiscountFmSet.find(fm) != _spanishDiscountFmSet.end());
}

// Insert the given FareMarket in the Spanish Large Family Discount FareMarket set.
// If the FareMarket already exists in the set, do nothing
void
ShoppingTrx::setSpanishDiscountFM(const FareMarket* fm)
{
  _spanishDiscountFmSet.insert(fm);
}

void
ShoppingTrx::setIbfData(IbfData* data)
{
  TSE_ASSERT(data != nullptr);
  _ibfData = data;
}

IbfData&
ShoppingTrx::getIbfData() const
{
  TSE_ASSERT(_ibfData != nullptr);
  return *_ibfData;
}
} // tse namespace
