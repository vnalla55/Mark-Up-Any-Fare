//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Common/TseEnums.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "ItinAnalyzer/SoloJourneyUtil.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tse
{

class Diag892Collector;
class FareCompInfo;
class FareMarket;
class GoverningCarrier;
class Itin;
class PricingTrx;
class TravelSeg;

using MultiPaxFCMapping = std::unordered_map<const PaxType*, std::vector<FareCompInfo*>>;
using FareBasisCodes = std::unordered_set<std::string>;
using FareMarketsWithCodes = std::vector<std::pair<FareMarket*, FareBasisCodes>>;

struct SOLFareMarketKey
{
  LocCode _boardMultiCity;
  LocCode _offMultiCity;
  CarrierCode _carrier;
  FareMarket::SOL_FM_TYPE _fmType;

  SOLFareMarketKey(const LocCode& boardMultiCity,
                   const LocCode& offMultiCity,
                   const CarrierCode& carrier,
                   FareMarket::SOL_FM_TYPE fmType)
    : _boardMultiCity(boardMultiCity),
      _offMultiCity(offMultiCity),
      _carrier(carrier),
      _fmType(fmType)
  {
  }
};

inline bool operator<(const SOLFareMarketKey& first, const SOLFareMarketKey& second)
{
  if (first._fmType < second._fmType)
    return true;
  if (first._fmType > second._fmType)
    return false;

  if (first._boardMultiCity < second._boardMultiCity)
    return true;
  if (first._boardMultiCity > second._boardMultiCity)
    return false;

  if (first._offMultiCity < second._offMultiCity)
    return true;
  if (first._offMultiCity > second._offMultiCity)
    return false;

  if (first._carrier < second._carrier)
    return true;
  if (first._carrier > second._carrier)
    return false;

  return false;
}

using SOLFareMarketMap = std::map<SOLFareMarketKey, FareMarket*>;
using SopInfo = std::pair<const uint32_t, bool>;
using Itin2SopInfoMap = std::map<const Itin*, SopInfo>;
using FirstUsedSopMap = std::map<std::vector<TravelSeg*>, int>;

struct GovCxrGroupParameters
{
  GovCxrGroupParameters(ItinIndex::Key key, const CarrierCode& govCarrier, int numSOPs)
    : govCarrierKey_(key), govCarrier_(govCarrier), numSOPs_(numSOPs)
  {
  }
  const ItinIndex::Key govCarrierKey_;
  const CarrierCode& govCarrier_;
  const int numSOPs_;
  FirstUsedSopMap firstUsedSopMap_;
};

namespace itinanalyzerutils
{
// Use trx.getBrandsFilterForIS() and filter out not wanted brands from
// trx.brandProgramVec() and trx.fareMarket():s brandProgramIndexVec()
void
filterBrandsForIS(PricingTrx& trx, Diag892Collector* diag892 = 0);

// A fare market is thru if it covers exactly a leg, i.e.
// it both starts and finishes a particular leg in itin
bool
isThruFareMarket(const FareMarket& fm, const Itin& itin);

// Gather all unique (identified by pointer) fare markets
// from itins and put them into unique_fms (output parameter).
// The initial content of unique_fms is lost.
// ---
// n = number of fare markets in all itins
// time complexity = O(n log n)
void
collectUniqueMarketsForItins(const std::vector<Itin*>& itins,
                             std::set<const FareMarket*>& unique_fms);

void
removeTrxFMsNotReferencedByAnyItin(PricingTrx& trx);

// By means of this function we can effectively enforce fare breaks at leg points
void
removeFMsExceedingLegs(std::vector<Itin*>& itins);

// Sets itin legs for MIP transactions
void
setItinLegs(Itin* itin);

int
getNumSOPs(const ItinIndex::ItinRow& row);

Itin*
getFirstItin(const ItinIndex::ItinRow& row);

CarrierCode
getGovCarrier(const Itin* firstItin);

void
createSopUsages(ShoppingTrx& trx);

Itin2SopInfoMap
getSopInfoMap(Itin& journeyItin, const ShoppingTrx::Leg& leg);

void
determineFMDirection(FareMarket* fareMarket, const LocCode& sopDestination);

void
initializeSopUsages(ShoppingTrx& trx,
                    FareMarket* fareMarket,
                    Itin* itin,
                    const std::vector<TravelSeg*>& fmts,
                    int startSeg,
                    int origSopId,
                    int bitIndex,
                    GovCxrGroupParameters& params);
void
printDiagnostic922(ShoppingTrx& trx, const std::map<ItinIndex::Key, CarrierCode>& carrierMap);

FareMarket*
buildFareMkt(ShoppingTrx& trx,
             const Itin* itin,
             const std::vector<TravelSeg*>& fmts,
             int legIdx,
             const GovCxrGroupParameters& params);

void
assignLegsToSegments(std::vector<TravelSeg*>& travelSegs,
                     DataHandle& dataHandle,
                     Diag892Collector* diag892);

void
setSopWithHighestTPM(ShoppingTrx& trx, int legIdx, int origSopId);

void
setGovCxrForSubIata21(PricingTrx& trx, FareMarket& fm, GoverningCarrier& govCxr, int percent);

void
removeRedundantFareMarket(MultiPaxFCMapping& multiPaxFCMapping);

template <typename TrxType, typename ItinAnalyzerServiceWrapperType>
void
initializeAndStoreFareMarkets(FareMarketsWithCodes& fareMarketsWithCodes,
                              ItinAnalyzerServiceWrapperType* baseItinAnalyzerWrapper,
                              TrxType& trx)
{
  auto* itin = trx.itin().front();
  for (auto& fareMarketInfo : fareMarketsWithCodes)
  {
    auto fareMarket = fareMarketInfo.first;
    auto& fareBasisCodes = fareMarketInfo.second;

    if ((fareMarket == nullptr) || fareMarket->travelSeg().empty())
      continue;

    fareMarket->setOrigDestByTvlSegs();
    fareMarket->travelDate() = itin->travelDate();

    if (!fareMarket->isMultiPaxUniqueFareBasisCodes())
      fareMarket->createMultiPaxUniqueFareBasisCodes();

    fareMarket->getMultiPaxUniqueFareBasisCodes() = fareBasisCodes;
    // Set travel boundary
    baseItinAnalyzerWrapper->setupAndStoreFareMarket(trx, *itin, fareMarket);
  }
}

template <typename TrxType, typename ItinAnalyzerServiceWrapperType>
void
initializeFareMarketsFromFareComponents(TrxType& trx,
                                        ItinAnalyzerServiceWrapperType* baseItinAnalyzerWrapper)
{
  auto& paxFCMapping = *trx.getMultiPassengerFCMapping();
  FareMarketsWithCodes fareMarketInfo;

  for (auto& mapElem : paxFCMapping)
  {
    auto& fareComponentVector = mapElem.second;
    for (auto fc : fareComponentVector)
    {
      auto* fareMarket = fc->fareMarket();
      std::string fareBasisCode(fc->fareBasisCode().begin(), fc->fareBasisCode().end());
      auto it = std::find_if(fareMarketInfo.begin(),
                             fareMarketInfo.end(),
                             [fareMarket](const auto& fareMarketInfoPair)
                             { return fareMarketInfoPair.first == fareMarket; });
      if (it != fareMarketInfo.end())
        it->second.insert(std::move(fareBasisCode));
      else
        fareMarketInfo.emplace_back(fareMarket, FareBasisCodes({std::move(fareBasisCode)}));
    }
  }
  initializeAndStoreFareMarkets(fareMarketInfo, baseItinAnalyzerWrapper, trx);
}
}
} // namespace tse

