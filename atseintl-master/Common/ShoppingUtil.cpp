///----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Common/ShoppingUtil.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/DateTime.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag325Collector.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/GroupFarePath.h" //!!
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <algorithm>

#include <boost/range/adaptor/indexed.hpp>

namespace tse
{
using namespace std;

FALLBACK_DECL(fallbackHalfRTPricingForIbf);
FALLBACK_DECL(fallbackOneSecondHurryOut);
namespace
{
Logger
logger("atseintl.Common.ShoppingUtil");
ConfigurableValue<uint16_t>
hurryResponseThresholdPercentage("SHOPPING_OPT", "HURRY_RESPONSE_THRESHOLD_PERCENTAGE");
ConfigurableValue<int32_t>
hurryResponsePercentage("SHOPPING_OPT", "HURRY_RESPONSE_PERCENTAGE", 60);
}

//---------------------------------------------------------------------------
// Create a group key from a string
//---------------------------------------------------------------------------
void
ShoppingUtil::createKey(const std::string& value, ItinIndex::Key& key)
{
  ItinIndex::KeyGenerator keyGenerator(tse::Global::hasherMethod());
  keyGenerator << value;
  key = keyGenerator.hash();
}

//----------------------------------------------------------------------------
// Create a group key from a numeric value
//----------------------------------------------------------------------------
void
ShoppingUtil::createKey(const uint32_t& value, ItinIndex::Key& key)
{
  key = static_cast<ItinIndex::Key>(value);
}

//----------------------------------------------------------------------------
// Creates a governing carrier key for a specified itinerary by
// utilizing the thru-fare market contained in the itinerary.
//----------------------------------------------------------------------------
void
ShoppingUtil::createCxrKey(const Itin* curItin, ItinIndex::Key& cxrCodeKey)
{
  // If the current itinerary is null, return
  if (UNLIKELY(!curItin))
  {
    return;
  }

  // Retrieve the itineraries thru fare market
  FareMarket* thruFareMarket = curItin->fareMarket().front();

  // If the thru fare market is null, return
  if (UNLIKELY(!thruFareMarket))
  {
    return;
  }

  // Create the carrier key
  createKey(thruFareMarket->governingCarrier(), cxrCodeKey);
}

//----------------------------------------------------------------------------
// Creates a carrier key directly from a carrier code
//----------------------------------------------------------------------------
void
ShoppingUtil::createCxrKey(const CarrierCode& cxrCode, ItinIndex::Key& cxrCodeKey)
{
  createKey(cxrCode, cxrCodeKey);
}

//----------------------------------------------------------------------------
// Creates a schedule indexer for a direct flight
//----------------------------------------------------------------------------
void
ShoppingUtil::createScheduleKey(ItinIndex::Key& scheduleKey)
{
  createKey(ITININDEX_DEFAULT, scheduleKey);
}

//----------------------------------------------------------------------------
// Creates a schedule indexer based on the passed in Itin
//----------------------------------------------------------------------------
void
ShoppingUtil::createScheduleKey(const Itin* curItin, ItinIndex::Key& scheduleKey)
{
  // Set the default indexer
  createKey(ITININDEX_DEFAULT, scheduleKey);

  // If the itin is invalid, or the itin has no travel
  // segments, return
  if (UNLIKELY((curItin == nullptr) || (curItin->travelSeg().empty())))
  {
    return;
  }

  // If the travel segs are not empty, create the actual
  // indexer
  createKey(uint32_t(curItin->travelSeg().size() - ITININDEX_OFFSET), scheduleKey);
}

enum ConditionForInterlineAvl
{ ADD_CURRENT_SEGMENT = 0,
  EXTEND_1_FORWARD_SAME_CXR,
  EXTEND_1_BACKWARD_SAME_CXR,
  EXTEND_1_FORWARD_DIFF_CXR,
  EXTEND_2_FORWARD_SAME_CXR,
  EXTEND_1_BACKWARD_DIFF_CXR,
  EXTEND_2_BACKWARD_SAME_CXR,
  EXTEND_2_FORWARD_DIFF_CXR,
  EXTEND_2_BACKWARD_DIFF_CXR };

bool
ShoppingUtil::partOfJourney(Itin* itin, const TravelSeg* tvlSeg)
{
  if (itin->flowFareMarket().empty())
    return false;

  if ((tvlSeg->carrierPref()) && (tvlSeg->carrierPref()->localMktJourneyType() == YES))
    return false;

  return itin->hasTvlSegInFlowFareMarkets(tvlSeg);
}

//----------------------------------------------------------------------------
// Retrieves a direct flight itinerary for a specified carrier
// in a specified leg
//----------------------------------------------------------------------------
ItinIndex::ItinCell*
ShoppingUtil::retrieveDirectItin(ShoppingTrx& trx,
                                 const uint32_t& legId,
                                 const ItinIndex::Key& cxrKey,
                                 const ItinIndex::ItinCheckType& checkType)
{
  ItinIndex& cxrIdx = trx.legs()[legId].carrierIndex();
  // Retrieve the most direct flight available
  return (cxrIdx.retrieveTopItinCell(cxrKey, checkType));
}

//----------------------------------------------------------------------------
// Retrieves a direct flight itinerary for a specified carrier
// in a specified leg
//----------------------------------------------------------------------------
ItinIndex::ItinCell*
ShoppingUtil::retrieveDirectItin(ItinIndex& cxrIdx,
                                 const ItinIndex::Key& cxrKey,
                                 const ItinIndex::ItinCheckType& checkType)
{
  // Retrieve the most direct flight available
  return (cxrIdx.retrieveTopItinCell(cxrKey, checkType));
}
//----------------------------------------------------------------------------
// Retrieves a direct flight itinerary for a specified carrier
// in a specified leg
//----------------------------------------------------------------------------
const ItinIndex::ItinCell*
ShoppingUtil::retrieveDirectItin(const ItinIndex& cxrIdx,
                                 const ItinIndex::Key& cxrKey,
                                 const ItinIndex::ItinCheckType& checkType)
{
  // Retrieve the most direct flight available
  return (cxrIdx.retrieveTopItinCell(cxrKey, checkType));
}

//----------------------------------------------------------------------------
// Retrieves a vector of carrier codes
// from the carrier index in the specified leg
//----------------------------------------------------------------------------
bool
ShoppingUtil::retrieveCarrierList(const ShoppingTrx& trx,
                                  const size_t& legId,
                                  std::vector<CarrierCode>& cxrList)
{
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs(); // lint !e530

  if (UNLIKELY(legs.empty() || legs.size() == 0))
  {
    return (false);
  }

  if (UNLIKELY(legId >= legs.size()))
  {
    return (false);
  }

  const ItinIndex& legIdx = legs[legId].carrierIndex();
  // Iterate through the index, retrieving all of the carrier
  // codes from the itincell leaves
  const ItinIndex::ItinMatrix& legRoot = legIdx.root();

  if (UNLIKELY(legRoot.empty() || legRoot.size() == 0))
  {
    return (false);
  }

  // Iterate through the itin index, retrieving the top itin
  // cell for each row (ie each governing carrier
  ItinIndex::ItinMatrixConstIterator mtxIter = legRoot.begin();
  ItinIndex::ItinMatrixConstIterator mtxEIter = legRoot.end();

  for (; mtxIter != mtxEIter; ++mtxIter)
  {
    const ItinIndex::ItinMatrixPair& mtxPair =
        static_cast<const ItinIndex::ItinMatrixPair&>(*mtxIter);
    const ItinIndex::Key& itinRowKey = mtxPair.first;
    const ItinIndex::ItinCell* itinCell =
        legIdx.retrieveTopItinCell(itinRowKey, ItinIndex::CHECK_NOTHING);

    if (UNLIKELY(itinCell == nullptr))
    {
      continue;
    }

    const Itin* itin = itinCell->second;

    if (UNLIKELY(itin == nullptr))
    {
      continue;
    }

    const FareMarket* itinFM = itin->fareMarket().front();

    if (UNLIKELY(itinFM == nullptr))
    {
      continue;
    }

    cxrList.push_back(itinFM->governingCarrier());
  }

  return (!(cxrList.empty()));
}

void
ShoppingUtil::orderSegsByLeg(ShoppingTrx& trx)
{
  orderSegsInItin(trx.journeyItin());

  uint16_t index = 0;
  for (ShoppingTrx::Leg& leg : trx.legs())
  {
    ++index;

    for (ShoppingTrx::SchedulingOption& sop : leg.sop())
      for (TravelSeg* ts : sop.itin()->travelSeg())
        ts->segmentOrder() = index;
  }
}

void
ShoppingUtil::orderSegsInItin(Itin* itin)
{
  if (itin == nullptr)
    return;

  uint16_t index = 0;
  for (TravelSeg* ts : itin->travelSeg())
  {
    ++index;
    ts->segmentOrder() = index;
  }
}

void
ShoppingUtil::orderSegsByItin(ShoppingTrx& trx)
{
  orderSegsInItin(trx.journeyItin());

  for (Itin* itin : trx.itin())
    orderSegsInItin(itin);
}
void
ShoppingUtil::sortItinBySegCount(PricingTrx& trx)
{
  std::vector<Itin *> *vItin = &trx.itin();
  std::sort(vItin->begin(), vItin->end(), []( Itin* arg1, Itin* arg2 ) { return (arg1->travelSeg().size() <  arg2->travelSeg().size()); });
}
bool
ShoppingUtil::getFareMarketLegIndices(const ShoppingTrx& trx,
                                      const FareMarket* const fm,
                                      uint32_t& first,
                                      uint32_t& last)
{
  uint32_t index = 0;
  for (const ShoppingTrx::Leg& leg : trx.legs())
  {
    for (const ShoppingTrx::SchedulingOption& sop : leg.sop())
    {
      const std::vector<FareMarket*>& fareMarkets = sop.itin()->fareMarket();
      if (std::find(fareMarkets.begin(), fareMarkets.end(), fm) != fareMarkets.end())
      {
        if (leg.stopOverLegFlag())
        {
          TSE_ASSERT(leg.jumpedLegIndices().empty() == false);
          first = leg.jumpedLegIndices().front();
          last = leg.jumpedLegIndices().back();
        }
        else
        {
          first = last = index;
        }

        return true;
      }
    }
    ++index;
  }

  return false;
}

void
ShoppingUtil::getEncodedGovCxrs(const ShoppingTrx& trx,
                                std::vector<std::vector<ItinIndex::Key>>& res)
{
  res.reserve(trx.legs().size());

  for (std::vector<ShoppingTrx::Leg>::const_iterator i = trx.legs().begin(); i != trx.legs().end();
       ++i)
  {
    if (i->stopOverLegFlag())
    {
      continue;
    }

    const uint32_t sops = i->requestSops();
    res.push_back(std::vector<ItinIndex::Key>(sops));

    for (uint32_t n = 0; n != sops; ++n)
    {
      const ShoppingTrx::SchedulingOption& sop = i->sop()[n];
      const std::vector<FareMarket*>& fm = sop.itin()->fareMarket();

      if (fm.empty() == false)
      {
        createCxrKey(fm.front()->governingCarrier(), res.back()[n]);
      }
    }
  }
}

//----------------------------------------------------------------------------
// Strips the specified substring from the end of a money object
// to string result
//----------------------------------------------------------------------------
std::string
ShoppingUtil::stripMoneyStr(const Money& m, const std::string& stripStr)
{
  std::ostringstream ostr;
  ostr << m;
  std::string monStr = ostr.str();
  return (monStr.substr(0, monStr.find(stripStr)));
}

bool
ShoppingUtil::getLegTravelSegIndices(const ShoppingTrx& trx,
                                     const std::vector<TravelSeg*>& segs,
                                     uint32_t& startLeg,
                                     uint32_t& endLeg,
                                     uint32_t& adoptedLeg)
{
  static const uint32_t NO_INDEX = numeric_limits<uint32_t>::max();
  startLeg = endLeg = NO_INDEX;

  uint32_t index = 0;
  for (const ShoppingTrx::Leg& leg : trx.legs())
  {
    if (startLeg == NO_INDEX && leg.segBeginsLeg(segs.front()))
    {
      startLeg = leg.stopOverLegFlag() ? leg.jumpedLegIndices().front() : index;

      if (segs.size() == 1)
      {
        endLeg = startLeg;
      }
    }

    if (endLeg == NO_INDEX && leg.segEndsLeg(segs.back()))
    {
      endLeg = leg.stopOverLegFlag() ? leg.jumpedLegIndices().back() : index;
    }
    ++index;
  }

  if (UNLIKELY(startLeg == NO_INDEX || endLeg == NO_INDEX))
  {
    return false;
  }

  if (startLeg == endLeg)
  {
    ++endLeg;
    adoptedLeg = startLeg;
    return true;
  }

  for (const ShoppingTrx::Leg& leg : trx.legs())
  {
    if (leg.stopOverLegFlag() && leg.jumpedLegIndices().front() == startLeg &&
        leg.jumpedLegIndices().back() == endLeg)
    {
      adoptedLeg = leg.adoptedCrossedLegRefIndex();
      ++endLeg;
      return true;
    }
  }

  TSE_ASSERT(false);
  return false;
}

std::size_t
tse::ShoppingUtil::getTravelSegCountForExternalSopID(const ShoppingTrx& trx,
                                                     LegId leg,
                                                     int origSopId)
{
  uint32_t sopId = ShoppingUtil::findInternalSopId(trx, leg, origSopId);

  const ShoppingTrx::SchedulingOption& sop = trx.legs()[leg].sop()[sopId];
  const Itin& itin = *sop.itin();
  std::size_t result = itin.travelSeg().size();
  return result;
}

bool
ShoppingUtil::isIndustryFareUsed(const FarePath& farePath)
{
  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
      if (paxTypeFare->carrier() == INDUSTRY_CARRIER)
        return true;
    }
  }

  return false;
}

std::vector<ClassOfServiceList>&
ShoppingUtil::getClassOfServiceInternal(const PricingTrx& trx,
                                        const PricingTrx::ClassOfServiceKey& key)
{
  AvailabilityMap::const_iterator availMapIt = trx.availabilityMap().end();

  // We're able to build a key only for up to 4 travel segments
  if (key.size() <= 4)
    availMapIt = trx.availabilityMap().find(ShoppingUtil::buildAvlKey(key));

  if (availMapIt == trx.availabilityMap().end())
  {
    // See if there were any Arunk segs in the key, in which case we remove
    // them and try again
    for (PricingTrx::ClassOfServiceKey::const_iterator i = key.begin(); i != key.end(); ++i)
    {
      if (dynamic_cast<const ArunkSeg*>(*i) != nullptr)
      {
        const size_t index = i - key.begin();
        // Build new key without arunk segment
        PricingTrx::ClassOfServiceKey newKey(key.begin(), i);
        ++i;
        newKey.insert(newKey.end(), i, key.end());
        // We insert an empty class of service list for the arunk
        // segment before returning it
        std::vector<ClassOfServiceList>* const res =
            trx.dataHandle().create<std::vector<ClassOfServiceList>>();
        *res = getClassOfServiceInternal(trx, newKey);
        res->insert(res->begin() + index, ClassOfServiceList());
        return *res;
      }
    }

    // try to construct the class of service out of smaller components.
    // search for the largest component that can match the start of the key
    std::vector<uint16_t> largestComponent;
    std::vector<ClassOfServiceList>* largestComponentCos = nullptr;
    std::vector<uint16_t> keyVec;
    std::vector<uint16_t> iterKeyVec;
    getIdsVecForKey(key, keyVec);

    for (availMapIt = trx.availabilityMap().begin(); availMapIt != trx.availabilityMap().end();
         ++availMapIt)
    {
      getIdsVecForKey(availMapIt->first, iterKeyVec);

      if ((iterKeyVec.size() < keyVec.size()) && (iterKeyVec.size() > largestComponent.size()) &&
          (std::equal(iterKeyVec.begin(), iterKeyVec.end(), keyVec.begin())))
      {
        largestComponent = iterKeyVec;
        largestComponentCos = availMapIt->second;
      }
    }

    if (largestComponentCos == nullptr)
    {
      const FlightFinderTrx* ffTrx = dynamic_cast<const FlightFinderTrx*>(&trx);

      // For Promotional Shopping return empty cos vector instead
      // of throwing exception
      if (ffTrx != nullptr && ffTrx->avlInS1S3Request())
      {
        std::vector<ClassOfServiceList>* emptyRes;
        emptyRes = trx.dataHandle().create<std::vector<ClassOfServiceList>>();
        return *emptyRes;
      }
    }

    VALIDATE_OR_THROW(largestComponentCos != nullptr, INVALID_INPUT, "Class of Service not found");
    std::vector<ClassOfServiceList> value(*largestComponentCos);
    // now recurse to try to find components that can match the rest
    // of the key
    const PricingTrx::ClassOfServiceKey newKey(key.begin() + largestComponent.size(), key.end());
    const std::vector<ClassOfServiceList>& restOfValue =
        getClassOfServiceInternal(trx, newKey);
    value.insert(value.end(), restOfValue.begin(), restOfValue.end());
    std::vector<ClassOfServiceList>* const res =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();
    *res = value;
    return *res;
  }

  return *(availMapIt->second);
}

const std::vector<ClassOfServiceList>&
ShoppingUtil::getClassOfService(const PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key)
{
  return getClassOfServiceInternal(trx, key);
}

std::vector<ClassOfServiceList>&
ShoppingUtil::getClassOfService(PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key)
{
  return getClassOfServiceInternal(trx, key);
}

std::vector<ClassOfServiceList>*
ShoppingUtil::getClassOfServiceNoThrow(PricingTrx& trx, const PricingTrx::ClassOfServiceKey& key)
{
  try
  {
    return &ShoppingUtil::getClassOfService(trx, key);
  }
  catch (ErrorResponseException&)
  {
    return nullptr;
  }
}

namespace
{
ClassOfServiceList emptyClassOfService;
}

ClassOfServiceList&
ShoppingUtil::getMaxThruClassOfServiceForSeg(PricingTrx& trx, TravelSeg* seg)
{
  PricingTrx::ThruFareAvailabilityMap::iterator itor = trx.maxThruFareAvailabilityMap().find(seg);

  if (itor == trx.maxThruFareAvailabilityMap().end())
  {
    return emptyClassOfService;
  }

  return itor->second;
}

const ClassOfServiceList&
ShoppingUtil::getMaxThruClassOfServiceForSeg(const PricingTrx& trx, TravelSeg* seg)
{
  const PricingTrx::ThruFareAvailabilityMap::const_iterator itor =
      trx.maxThruFareAvailabilityMap().find(seg);

  if (itor == trx.maxThruFareAvailabilityMap().end())
  {
    return emptyClassOfService;
  }

  return itor->second;
}

ShoppingUtil::ExternalSopId
ShoppingUtil::createExternalSopId(const LegId legId, const SopId extId)
{
  return std::make_tuple(legId, extId);
}

void
ShoppingUtil::singleExternalIdSearch(ShoppingTrx& trx,
                                     const ExternalSopId& externId,
                                     InternalSopId& internalId)
{
  TSELatencyData(trx, "SHOPUTIL SINGLE EXTERN2INTERN");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  const size_t legSize = legs.size();
  const LegId& externLegId = std::get<ExternalSopIdLegId>(externId);
  const SopId& externExtId = std::get<ExternalSopIdExtId>(externId); // lint !e1561
  TSE_ASSERT(externLegId < legSize);
  ShoppingTrx::Leg& leg = legs[externLegId];

  if (UNLIKELY(leg.stopOverLegFlag()))
  {
    throw ErrorResponseException(
        ErrorResponseException::INVALID_INPUT,
        "Cannot specify across stop over leg for singular extern2intern id search");
  }

  std::vector<ShoppingTrx::SchedulingOption>& sops = leg.sop();
  const std::vector<ShoppingTrx::SchedulingOption>::difference_type sopSize = sops.size();
  TSE_ASSERT(externExtId < sopSize);
  ShoppingTrx::SchedulingOption& sop = sops[externExtId];
  Itin* sopItin = sop.itin();
  TSE_ASSERT(sopItin != nullptr);
  CarrierCode govCxr = sop.governingCarrier();
  ItinIndex::Key govCxrKey;
  createKey(govCxr, govCxrKey);
  ItinIndex& cxrIdx = leg.carrierIndex();
  ItinIndex::ItinRowCellMap& iRowCellMap = cxrIdx.rowCellMap();
  ItinIndex::ItinRowCellMapIterator iVIter = iRowCellMap.find(govCxrKey);
  TSE_ASSERT(iVIter != iRowCellMap.end());
  const ItinIndex::ItinRowCellVector& iRCVector = iVIter->second;
  ItinIndex::ItinRowCellVectorConstIterator iRCVIter = iRCVector.begin();
  ItinIndex::ItinRowCellVectorConstIterator iRCVEIter = iRCVector.end();
  int bitIndexFound = -1;

  for (int n = 0; iRCVIter != iRCVEIter; ++iRCVIter, ++n)
  {
    if (iRCVIter->second == sopItin)
    {
      bitIndexFound = n;
      break;
    }
  }

  if (UNLIKELY(bitIndexFound == -1))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Could not find internal id that matches external id");
  }

  // Assign found data to the outgoing internal id reference
  std::get<InternalSopIdLegId>(internalId) = externLegId;
  std::get<InternalSopIdBitIndex>(internalId) = bitIndexFound;
  std::get<InternalSopIdGovCxr>(internalId) = govCxr;
}

uint32_t
ShoppingUtil::getFlightBitIndex(const ShoppingTrx& trx, const ExternalSopId& externalId)
{
  InternalSopId iSopId;

  try
  {
    ShoppingTrx& nonConstTrx = const_cast<ShoppingTrx&>(trx);
    singleExternalIdSearch(nonConstTrx, externalId, iSopId);
  }
  catch (ErrorResponseException& e)
  {
    // Re-throw exception for external usage
    LOG4CXX_ERROR(logger, "Exception occurred while using internal search method: " << e.what());
    throw ErrorResponseException(e);
  }

  return (std::get<InternalSopIdBitIndex>(iSopId));
}

void
ShoppingUtil::preparePathsFromFlightMatrix(const ShoppingTrx::FlightMatrix& matrix,
                                           std::vector<GroupFarePath*>& paths,
                                           std::vector<const SopIdVec*>* fMatVectors)
{
  paths.reserve(matrix.size());

  for (const ShoppingTrx::FlightMatrix::value_type& fmItem : matrix)
    if (fmItem.second)
      paths.push_back(fmItem.second);

  // Remove duplicates
  std::sort(paths.begin(), paths.end());
  paths.erase(std::unique(paths.begin(), paths.end()), paths.end());
  // Order prices from lowest first to highest last
  std::sort(paths.begin(), paths.end(), GroupFarePath::Greater()); // lint !e530
  std::reverse(paths.begin(), paths.end());

  if (!fMatVectors)
    return;

  // Need to re-align the flight matrix solutions with the sorted paths
  fMatVectors->reserve(paths.size());

  for (const GroupFarePath* curGFPath : paths)
  {
    for (const ShoppingTrx::FlightMatrix::value_type& fmItem : matrix)
    {
      if (fmItem.second == curGFPath)
      {
        fMatVectors->push_back(&fmItem.first);
        break;
      }
    }
  }

  TSE_ASSERT(paths.size() == fMatVectors->size());
}

namespace
{
// determines if two sets of SOPs are equal, ignoring arunk segments
bool
sopsEqualIgnoreArunk(const SopIdVec& sops1, const SopIdVec& sops2)
{
  const int SurfSector = int(ASOLEG_SURFACE_SECTOR_ID);
  const size_t count1 = sops1.size() - std::count(sops1.begin(), sops1.end(), SurfSector);
  const size_t count2 = sops2.size() - std::count(sops2.begin(), sops2.end(), SurfSector);
  TSE_ASSERT(count1 == count2);
  auto i1 = sops1.begin();
  auto i2 = sops2.begin();

  while (i1 != sops1.end() && i2 != sops2.end())
  {
    if (*i1 == SurfSector)
    {
      ++i1;
      continue;
    }

    if (*i2 == SurfSector)
    {
      ++i2;
      continue;
    }

    if (*i1 != *i2)
    {
      return false;
    }

    ++i1;
    ++i2;
  }

  return true;
}
}

uint32_t
ShoppingUtil::getBitmapForASOLeg(ShoppingTrx& trx,
                                 uint32_t legId,
                                 const CarrierCode& govCxr,
                                 const SopIdVec& sopIds)
{
  TSE_ASSERT(!sopIds.empty());
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  const uint32_t legSize = uint32_t(legs.size());
  TSE_ASSERT(legId < legSize);
  ShoppingTrx::Leg& leg = legs[legId];

  if (!leg.stopOverLegFlag())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Cannot use this method with non-across stop over legs");
  }

  ItinIndex::Key govCxrKey;
  createKey(govCxr, govCxrKey);
  ItinIndex& cxrIdx = leg.carrierIndex();
  ItinIndex::ItinIndexIterator itinIdxIter = cxrIdx.beginAcrossStopOverRow(trx, legId, govCxrKey);
  ItinIndex::ItinIndexIterator itinIdxEnd = cxrIdx.endAcrossStopOverRow();
  uint32_t foundBitIndex = std::numeric_limits<uint32_t>::max();

  for (; itinIdxIter != itinIdxEnd; ++itinIdxIter)
  {
    const SopIdVec& currentSopSet = itinIdxIter.currentSopSet();

    if (sopsEqualIgnoreArunk(currentSopSet, sopIds))
    {
      foundBitIndex = itinIdxIter.bitIndex();
      break;
    }
  }

  if (foundBitIndex == std::numeric_limits<uint32_t>::max())
  {
    throw ErrorResponseException(
        ErrorResponseException::INVALID_INPUT,
        "Could not find ASO leg bit index that matches the input sop indices.");
  }

  return foundBitIndex;
}

uint32_t
ShoppingUtil::findSopId(const PricingTrx& trx, uint32_t leg, uint32_t sop)
{
  TSE_ASSERT(leg < trx.indicesToSchedulingOption().size());
  const std::map<uint32_t, uint32_t>& sops = trx.indicesToSchedulingOption()[leg];
  std::map<uint32_t, uint32_t>::const_iterator j = sops.find(sop);

  if (LIKELY(j != sops.end()))
  {
    return j->second;
  }

  TSE_ASSERT(false);
  return 0;
}

uint32_t
ShoppingUtil::findInternalSopId(const PricingTrx& trx, LegId leg, uint32_t externalSopId)
{
  TSE_ASSERT(leg < trx.schedulingOptionIndices().size());
  const std::map<uint32_t, uint32_t>& external2InternalSopIdMap =
      trx.schedulingOptionIndices()[leg];
  std::map<uint32_t, uint32_t>::const_iterator itFound =
      external2InternalSopIdMap.find(externalSopId);

  if (LIKELY(itFound != external2InternalSopIdMap.end()))
    return itFound->second;

  throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                               "Could not find internal SOP indices");
  return 0;
}

std::vector<Itin*>
ShoppingUtil::expandEstimatedOptions(const PricingTrx& trx)
{
  std::vector<Itin*> res = trx.itin();

  for (Itin* itin : trx.itin())
  {
    if (itin->dcaSecondCall() || (trx.getOptions()->callToAvailability() == 'T'))
    {
      for (const SimilarItinData& data : itin->getSimilarItins())
        res.push_back(data.itin);
    }
  }

  return res;
}

Itin*
ShoppingUtil::getDummyItineraryFromCarrierIndex(ItinIndex& curCarrierIndex,
                                                ItinIndex::Key& carrierKey)
{
  LOG4CXX_DEBUG(logger,
                "ShoppingUtil::getDummyItineraryFromCarrierIndex(ItinIndex&, ItinIndex::Key&)");
  // Get row for specified carrier from carrier index
  ItinIndex::ItinMatrixIterator matrixIter = curCarrierIndex.root().find(carrierKey);

  // Check if row for specified carrier key was found
  if (curCarrierIndex.root().end() == matrixIter)
  {
    LOG4CXX_WARN(logger,
                 "ShoppingUtil::getDummyItineraryFromCarrierIndex - Carrier row for "
                 "specified carrier wasn't found");
    return nullptr;
  }

  ItinIndex::ItinRow& curRow = matrixIter->second;
  // Create direct flight schedule key
  ItinIndex::Key scheduleKey;
  createScheduleKey(scheduleKey);
  // Get direct flight column
  ItinIndex::ItinRowIterator rowIter = curRow.find(scheduleKey);

  // Check if direct flight column was found
  if (curRow.end() == rowIter)
  {
    LOG4CXX_WARN(logger,
                 "ShoppingUtil::getDummyItineraryFromCarrierIndex - Direct fly column for "
                 "specified carrier wasn't found");
    return nullptr;
  }

  ItinIndex::ItinColumn& curColumn = rowIter->second;

  // Go thru all direct flight columns and try to find the dummy one
  for (ItinIndex::ItinRowCellVectorIterator itinRowCellIter = curColumn.begin();
       itinRowCellIter != curColumn.end();
       itinRowCellIter++)
  {
    if (true == (itinRowCellIter->first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT))
    {
      // If current flight is a fake one return pointer to dummy itinerary
      return itinRowCellIter->second;
    }
  }

  return nullptr;
}

size_t
ShoppingUtil::countNonDummyItineraries(const ShoppingTrx::Leg& leg)
{
  size_t result = 0;
  for (const ShoppingTrx::SchedulingOption& sched : leg.sop())
  {
    const Itin& itin = *sched.itin();
    if (!itin.isDummy())
      ++result;
  }
  return result;
}

// given SOP Options counts total stop penalty for the itin
int
ShoppingUtil::totalStopPenalty(const Itin* itin, const int perStopPenalty)
{
  if ((itin == nullptr) || (itin->travelSeg().size() == 0))
  {
    return 0;
  }

  return (perStopPenalty * int(itin->travelSeg().size() - 1));
}

// given SOP Options counts total travel Duration penalty for the itin
int
ShoppingUtil::totalTravDurPenalty(const Itin* itin, const int travDurPenalty)
{
  if ((itin == nullptr) || (itin->travelSeg().size() == 0))
  {
    return 0;
  }

  int diffDurSeconds = abs(int(DateTime::diffTime(itin->travelSeg().back()->arrivalDT(),
                                                  itin->travelSeg().front()->departureDT())));
  return (travDurPenalty * diffDurSeconds / SECONDS_PER_HOUR);
}

// given SOP Options and requested Departure Time counts total DepTime Deviation penalty for the
// itin
int
ShoppingUtil::totalDepTimeDevPenalty(const Itin* itin,
                                     const int depTimeDevPenalty,
                                     const DateTime& reqDepDateTime)
{
  if ((itin == nullptr) || (itin->travelSeg().size() == 0))
  {
    return 0;
  }

  DateTime unixEmptyDateValue(boost::gregorian::date(1970, 1, 1),
                              boost::posix_time::time_duration(0, 0, 0));

  // check if request contained requested departure DT
  if ((!reqDepDateTime.isValid()) || (reqDepDateTime == unixEmptyDateValue))
  {
    return 0;
  }

  int diffDevSeconds =
      abs(int(DateTime::diffTime(reqDepDateTime, itin->travelSeg().front()->departureDT())));
  return (depTimeDevPenalty * diffDevSeconds / SECONDS_PER_HOUR);
}

void
ShoppingUtil::setTimeout(PricingTrx& trx, int timeout)
{
  TrxAborter* aborter = trx.aborter();

  if (aborter == nullptr)
  {
    return;
  }

  aborter->setTimeout(timeout);
  uint16_t hurry = 0;
  int hurryAt = timeout;
  int hurryPercentNormal = 60;

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (UNLIKELY(!fallback::fallbackOneSecondHurryOut(&trx)))
      hurryAt = timeout - 2; // 08Aug2016 for Jira SCI 1187
    else
      hurryAt = timeout - 1;
  }
  else
  {
    hurry = hurryResponseThresholdPercentage.getValue();
    if (!hurryResponseThresholdPercentage.isDefault())
    {
      hurryAt = (timeout * hurry) / 100;
    }
    else
    {
      hurryAt = (timeout * hurryPercentNormal) / 100;
    }
  }

  if (hurryAt < 1)
  {
    hurryAt = 1;
  }

  aborter->setHurry(hurryAt);
  std::string hurryWithCondition;
  hurryAt = (timeout * hurryResponsePercentage.getValue()) / 100;
  if (!hurryResponsePercentage.isDefault())
  {
    if (hurryAt < 1)
    {
      hurryAt = 1;
    }

    aborter->setHurryWithCond(hurryAt);
  }
}

bool
ShoppingUtil::setupFareMarketTravelSegESV(FareMarket* fareMarket, Itin* itin)
{
  LOG4CXX_DEBUG(logger, "ShoppingUtil::setupFareMarketTravelSegESV(FareMarket*, Itin*)");
  bool copySegment = false;
  fareMarket->travelSeg().clear();

  // Go thorough all travel segments and copy those which should belong to
  // processed fare market
  for (std::vector<TravelSeg*>::iterator segIter(itin->travelSeg().begin()),
       segIterEnd(itin->travelSeg().end());
       segIter != segIterEnd;
       ++segIter)
  {
    if ((*segIter)->origin()->loc() == fareMarket->origin()->loc())
    {
      copySegment = true;
    }

    if (true == copySegment)
    {
      fareMarket->travelSeg().push_back(*segIter);
    }

    if ((*segIter)->destination()->loc() == fareMarket->destination()->loc())
    {
      copySegment = false;
    }
  }

  if (fareMarket->travelSeg().empty())
  {
    LOG4CXX_ERROR(
        logger,
        "ShoppingUtil::setupFareMarketTravelSeg - Fare market travel segment vector is empty.");
    return false;
  }

  return true;
}

bool
ShoppingUtil::isCabinClassValid(const ShoppingTrx& trx, const SopIdVec& sops)
{
  for (uint32_t leg = 0; leg != sops.size(); ++leg)
  {
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[leg].sop()[sops[leg]];

    if (sop.cabinClassValid() == false)
    {
      return false;
    }
  }

  return true;
}

bool
ShoppingUtil::areSopConnectionPointsDifferent(const ShoppingTrx& trx,
                                              const SopIdVec& lhSops,
                                              const SopIdVec& rhSops)
{
  if (lhSops.size() != rhSops.size())
    return true;

  for (size_t leg = 0; leg != lhSops.size(); ++leg)
  {
    const ShoppingTrx::SchedulingOption& lhSop = trx.legs()[leg].sop()[lhSops[leg]];
    const ShoppingTrx::SchedulingOption& rhSop = trx.legs()[leg].sop()[rhSops[leg]];
    const std::vector<TravelSeg*>& lhTravelSegs = lhSop.itin()->travelSeg();
    const std::vector<TravelSeg*>& rhTravelSegs = rhSop.itin()->travelSeg();

    if (lhTravelSegs.size() != rhTravelSegs.size())
      return true;

    for (size_t seg = 0; seg < lhTravelSegs.size(); ++seg)
    {
      AirSeg* lhSeg = dynamic_cast<AirSeg*>(lhTravelSegs[seg]);
      AirSeg* rhSeg = dynamic_cast<AirSeg*>(rhTravelSegs[seg]);

      if (lhSeg->boardMultiCity() != rhSeg->boardMultiCity() ||
          lhSeg->offMultiCity() != rhSeg->offMultiCity())
        return true;
    }
  }

  return false;
}

bool
ShoppingUtil::isSimilarOption(const ShoppingTrx& trx,
                              ShoppingTrx::FlightMatrix const& solutions,
                              const SopIdVec& sops)
{
  const DatePair& curDatePair = ShoppingAltDateUtil::getDatePairSops(trx, sops);

  for (const ShoppingTrx::FlightMatrix::value_type& fmItem : solutions)
  {
    const SopIdVec& cellSops = fmItem.first;
    const DatePair& cellDatePair = ShoppingAltDateUtil::getDatePairSops(trx, cellSops);

    // process matrix cell only for the same dates
    if (cellDatePair != curDatePair)
      continue;

    if (!ShoppingUtil::areSopConnectionPointsDifferent(trx, cellSops, sops))
      return true;
  }

  // LOG4CXX_ERROR(logger, "isSimilarOption() ---NEW SOL-");
  return false;
}

bool
ShoppingUtil::isSameClassOfService(const ShoppingTrx& trx,
                                   const TravelSeg* basedTravelSeg,
                                   const TravelSeg* travelSeg)
{
  typedef std::vector<ClassOfService*> CosVec;

  const CosVec& basedCos = basedTravelSeg->classOfService();
  const CosVec& cos = travelSeg->classOfService();
  uint16_t totalNumSeats = PaxTypeUtil::totalNumSeats(trx);
  bool match = true;

  for (CosVec::const_iterator basedCosIter = basedCos.begin();
       basedCosIter != basedCos.end() && match;
       ++basedCosIter)
  {
    if ((*basedCosIter)->numSeats() < totalNumSeats)
      continue;

    match = false;

    for (CosVec::const_iterator cosIter = cos.begin(); cosIter != cos.end(); ++cosIter)
    {
      if ((*basedCosIter)->bookingCode() == (*cosIter)->bookingCode() &&
          (*cosIter)->numSeats() >= totalNumSeats)
      {
        match = true;
        break;
      }
    }
  }
  if (match)
    return true;
  return false;
}

bool
ShoppingUtil::isSameCxrAndCnxPointAndClassOfService(const ShoppingTrx& trx,
                                                    const ShoppingTrx::SchedulingOption& basedSop,
                                                    const ShoppingTrx::SchedulingOption& sop)
{
  const Itin* basedItin = basedSop.itin();
  const Itin* itin = sop.itin();
  TSE_ASSERT(basedItin);
  TSE_ASSERT(itin);
  const std::vector<TravelSeg*>& basedTravelSeg = basedItin->travelSeg();
  const std::vector<TravelSeg*>& travelSeg = itin->travelSeg();

  if (basedTravelSeg.size() < 1 || travelSeg.size() < 1 ||
      (basedTravelSeg.size() != travelSeg.size()))
  {
    return false;
  }

  for (std::vector<TravelSeg*>::size_type travelSegIndex = 0; travelSegIndex != itin->travelSeg().size();
       ++travelSegIndex)
  {
    const AirSeg* const basedAirSeg = dynamic_cast<const AirSeg*>(basedTravelSeg[travelSegIndex]);
    const AirSeg* const airSeg = dynamic_cast<const AirSeg*>(travelSeg[travelSegIndex]);

    if (UNLIKELY(basedAirSeg == nullptr || airSeg == nullptr))
      return false;

    if (basedAirSeg->marketingCarrierCode() == airSeg->marketingCarrierCode() &&
        basedTravelSeg[travelSegIndex]->origAirport() == travelSeg[travelSegIndex]->origAirport() &&
        basedTravelSeg[travelSegIndex]->destAirport() == travelSeg[travelSegIndex]->destAirport())
    {
      if (isSameClassOfService(trx, basedTravelSeg[travelSegIndex], travelSeg[travelSegIndex]))
        continue;
    }
    return false;
  }
  return true;
}

void
ShoppingUtil::getCheapestESVValues(const ShoppingTrx& trx,
                                   const Itin* itin,
                                   std::vector<MoneyAmount>& esvValues)
{
  if (1 == itin->paxTypeSOPFareListMap().count(trx.paxType()[0]))
  {
    std::map<PaxType*, SOPFareList>::const_iterator sopFareListMapIter =
        itin->paxTypeSOPFareListMap().find(trx.paxType()[0]);
    const SOPFareList& sopFareList = sopFareListMapIter->second;
    MoneyAmount cheapestOW = -1.0;
    MoneyAmount cheapestRT = -1.0;
    MoneyAmount cheapestCT = -1.0;
    MoneyAmount cheapestOJ = -1.0;
    std::vector<SOPFarePath*>::const_iterator sopFarePathIter;

    // Get cheapest OW ESV
    for (sopFarePathIter = sopFareList.owSopFarePaths().begin();
         sopFarePathIter != sopFareList.owSopFarePaths().end();
         sopFarePathIter++)
    {
      const SOPFarePath* sopFarePath = (*sopFarePathIter);

      if ((-1.0 == cheapestOW) || (sopFarePath->totalAmount() < cheapestOW))
      {
        cheapestOW = sopFarePath->totalAmount();
      }
    }

    // Get cheapest RT ESV
    for (sopFarePathIter = sopFareList.rtSopFarePaths().begin();
         sopFarePathIter != sopFareList.rtSopFarePaths().end();
         sopFarePathIter++)
    {
      const SOPFarePath* sopFarePath = (*sopFarePathIter);

      if ((-1.0 == cheapestRT) || (sopFarePath->totalAmount() < cheapestRT))
      {
        cheapestRT = sopFarePath->totalAmount();
      }
    }

    // Get cheapest CT ESV
    for (sopFarePathIter = sopFareList.ctSopFarePaths().begin();
         sopFarePathIter != sopFareList.ctSopFarePaths().end();
         sopFarePathIter++)
    {
      const SOPFarePath* sopFarePath = (*sopFarePathIter);

      if ((-1.0 == cheapestCT) || (sopFarePath->totalAmount() < cheapestCT))
      {
        cheapestCT = sopFarePath->totalAmount();
      }
    }

    // Get cheapest OJ ESV
    for (sopFarePathIter = sopFareList.ojSopFarePaths().begin();
         sopFarePathIter != sopFareList.ojSopFarePaths().end();
         sopFarePathIter++)
    {
      const SOPFarePath* sopFarePath = (*sopFarePathIter);

      if ((-1.0 == cheapestOJ) || (sopFarePath->totalAmount() < cheapestOJ))
      {
        cheapestOJ = sopFarePath->totalAmount();
      }
    }

    esvValues.push_back(cheapestOW);
    esvValues.push_back(cheapestRT);
    esvValues.push_back(cheapestCT);
    esvValues.push_back(cheapestOJ);
  }
  else
  {
    esvValues.insert(esvValues.begin(), 4, -1.0);
  }
}

bool
ShoppingUtil::travelSegsSimilar(ShoppingTrx& trx, const TravelSeg* seg1, const TravelSeg* seg2)
{
  const AirSeg* air1 = dynamic_cast<const AirSeg*>(seg1);
  const AirSeg* air2 = dynamic_cast<const AirSeg*>(seg2);

  if (UNLIKELY(trx.getTrxType() == PricingTrx::IS_TRX && trx.getOptions()->isCarnivalSumOfLocal()))
  {
    // Both segments are not air
    if ((air1 == nullptr) && (air2 == nullptr))
    {
      return true;
    }
  }

  if (UNLIKELY((air1 == nullptr) || (air2 == nullptr)))
  {
    return false;
  }

  if (UNLIKELY(
          (trx.getTrxType() == PricingTrx::IS_TRX && trx.getOptions()->isCarnivalSumOfLocal()) ||
          (trx.getTrxType() == PricingTrx::IS_TRX && trx.excTrxType() == PricingTrx::EXC_IS_TRX)))
  {
    if (air1->marketingCarrierCode() != air2->marketingCarrierCode())
      return false;
  }

  return air1->origAirport() == air2->origAirport() && air1->destAirport() == air2->destAirport() &&
         air1->departureDT().date() == air2->departureDT().date();
}

bool
ShoppingUtil::schedulingOptionsSimilar(ShoppingTrx& trx,
                                       const ShoppingTrx::SchedulingOption& sop1,
                                       const ShoppingTrx::SchedulingOption& sop2)
{
  if (LIKELY(trx.getTrxType() == PricingTrx::IS_TRX && !trx.getOptions()->isCarnivalSumOfLocal()))
  {
    if (sop1.governingCarrier() != sop2.governingCarrier())
    {
      return false;
    }
  }

  const Itin& itin1 = *sop1.itin();
  const Itin& itin2 = *sop2.itin();

  if (itin1.travelSeg().size() != itin2.travelSeg().size())
  {
    return false;
  }

  for (size_t index = 0; index < itin1.travelSeg().size(); ++index)
  {
    if (!travelSegsSimilar(trx, itin1.travelSeg()[index], itin2.travelSeg()[index]))
      return false;
  }

  return true;
}

uint64_t
ShoppingUtil::buildAvlKey(const std::vector<TravelSeg*>& seg)
{
  uint64_t key(0);

  for (uint32_t i(0); i < seg.size(); ++i)
  {
    key += (((uint64_t)(seg[i]->originalId())) << (i * 16));
  }

  return key;
}

void
ShoppingUtil::getIdsVecForKey(const uint64_t key, std::vector<uint16_t>& idsVec)
{
  idsVec.clear();

  for (uint32_t i(0); i < 4; ++i)
  {
    uint16_t originalId = (uint16_t)(key >> (i * 16));

    if (originalId != 0)
    {
      idsVec.push_back(originalId);
    }
  }
}

void
ShoppingUtil::getIdsVecForKey(const PricingTrx::ClassOfServiceKey& key,
                              std::vector<uint16_t>& idsVec)
{
  idsVec.clear();

  for (uint32_t i(0); i < key.size(); ++i)
  {
    uint16_t originalId = (key[i])->originalId();

    if (originalId != 0)
    {
      idsVec.push_back(originalId);
    }
  }
}

std::vector<ClassOfServiceList>&
ShoppingUtil::getThruAvailability(const PricingTrx& trx, std::vector<TravelSeg*>& travelSeg)
{
  return ShoppingUtil::getClassOfServiceInternal(trx, travelSeg);
}

std::vector<ClassOfServiceList>&
ShoppingUtil::getLocalAvailability(const PricingTrx& trx, std::vector<TravelSeg*>& travelSeg)
{
  std::vector<ClassOfServiceList>* outCosListVec =
      trx.dataHandle().create<std::vector<ClassOfServiceList>>();
  std::vector<TravelSeg*>::iterator segIter = travelSeg.begin();
  std::vector<TravelSeg*>::iterator segIterEnd = travelSeg.end();

  for (; segIter != segIterEnd; ++segIter)
  {
    ClassOfServiceList& cosList = getAvailability(trx, *segIter);
    outCosListVec->push_back(cosList);
  }

  return *outCosListVec;
}

ClassOfServiceList&
ShoppingUtil::getAvailability(const PricingTrx& trx, TravelSeg* travelSeg)
{
  PricingTrx::ClassOfServiceKey cosKey;
  cosKey.push_back(travelSeg);
  std::vector<ClassOfServiceList>& cosListVec =
      ShoppingUtil::getClassOfServiceInternal(trx, cosKey);

  if (cosListVec.empty())
  {
    LOG4CXX_ERROR(logger, "Could not find availability for single travel segment.");
    return *(trx.dataHandle().create<ClassOfServiceList>());
  }
  else
  {
    return cosListVec[0];
  }
}

void
ShoppingUtil::prepareFFClassOfService(ShoppingTrx::SchedulingOption& sop)
{
  const size_t cosVecSize = sop.thrufareClassOfService().size();

  if (cosVecSize != sop.itin()->travelSeg().size())
  {
    LOG4CXX_ERROR(logger,
                  "ShoppingUtil::prepareFFClassOfService - Thru fare class of service "
                  "vector size is different than travel segments count.");
    return;
  }

  for (size_t item = 0; item < cosVecSize; ++item)
  {
    sop.itin()->travelSeg()[item]->classOfService().clear();
    sop.itin()->travelSeg()[item]->classOfService() = *(sop.thrufareClassOfService()[item]);
  }
}

void
ShoppingUtil::buildCarriersSet(const Itin* itin, std::set<CarrierCode>& crxSet)
{
  std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin->travelSeg().end();

  for (; iter != iterEnd; ++iter)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iter);

    if (airSeg != nullptr)
    {
      crxSet.insert(airSeg->carrier());
    }
  }
}

bool
ShoppingUtil::checkCarriersSet(ShoppingTrx& trx,
                               CarrierCode& validatingCarrier,
                               InterlineTicketCarrier* itc,
                               std::set<CarrierCode>& crxSet)
{
  // If it's online always return true
  if (crxSet.size() <= 1)
  {
    return true;
  }

  std::set<CarrierCode>::iterator iter = crxSet.begin();
  std::set<CarrierCode>::iterator iterEnd = crxSet.end();

  for (; iter != iterEnd; ++iter)
  {
    if (validatingCarrier == (*iter))
    {
      continue;
    }

    if (!itc->validateAgreementBetweenValidatingAndInterlineCarrier(
            trx, validatingCarrier, (*iter)))
    {
      return false;
    }
  }

  return true;
}

FarePath*
ShoppingUtil::getFarePathForKey(const PricingTrx& trx,
                                const Itin* const itin,
                                const FarePath::FarePathKey& farePathKey)
{
  return getFarePathForKey(
      trx, itin, farePathKey._a, farePathKey._b, farePathKey._c, farePathKey._d, farePathKey._e);
}

FarePath*
ShoppingUtil::getFarePathForKey(const PricingTrx& trx,
                                const Itin* const itin,
                                const PaxType* paxType,
                                const uint16_t outBrandIndex,
                                const uint16_t inBrandIndex,
                                const Itin* outItin,
                                const Itin* inItin)
{
  std::vector<FarePath*>::const_iterator farePathIter = itin->farePath().begin();
  std::vector<FarePath*>::const_iterator farePathIterEnd = itin->farePath().end();

  for (; farePathIter != farePathIterEnd; ++farePathIter)
  {
    FarePath* farePath = (*farePathIter);

    if ((paxType == farePath->paxType()) && (farePath->brandIndexPair().first == outBrandIndex) &&
        (farePath->brandIndexPair().second == inBrandIndex) &&
        (farePath->parentItinPair().first == outItin) &&
        (farePath->parentItinPair().second == inItin))
    {
      return farePath;
    }
  }

  return nullptr;
}

TaxResponse*
ShoppingUtil::getTaxResponseForKey(const PricingTrx& trx,
                                   const Itin* const itin,
                                   const FarePath::FarePathKey& farePathKey)
{
  return getTaxResponseForKey(
      trx, itin, farePathKey._a, farePathKey._b, farePathKey._c, farePathKey._d, farePathKey._e);
}

TaxResponse*
ShoppingUtil::getTaxResponseForKey(const PricingTrx& trx,
                                   const Itin* const itin,
                                   const PaxType* paxType,
                                   const uint16_t outBrandIndex,
                                   const uint16_t inBrandIndex,
                                   const Itin* outItin,
                                   const Itin* inItin)
{
  std::vector<TaxResponse*>::const_iterator taxResIter = itin->getTaxResponses().begin();
  std::vector<TaxResponse*>::const_iterator taxResIterEnd = itin->getTaxResponses().end();

  for (; taxResIter != taxResIterEnd; ++taxResIter)
  {
    TaxResponse* taxResponse = (*taxResIter);

    if ((paxType == taxResponse->farePath()->paxType()) &&
        (taxResponse->farePath()->brandIndexPair().first == outBrandIndex) &&
        (taxResponse->farePath()->brandIndexPair().second == inBrandIndex) &&
        (taxResponse->farePath()->parentItinPair().first == outItin) &&
        (taxResponse->farePath()->parentItinPair().second == inItin))
    {
      return taxResponse;
    }
  }

  return nullptr;
}

bool
ShoppingUtil::isBrandValid(const PricingTrx& trx, const uint16_t brandIndex)
{
  if (trx.getRequest()->getBrandedFareSize() < 2)
    return true;

  const std::vector<bool>& validBrandIdVec = trx.validBrandIdVec();

  if (validBrandIdVec.size() > brandIndex)
    return trx.validBrandIdVec()[brandIndex];

  return true;
}

bool
ShoppingUtil::isBrandValid(PricingTrx& trx, const uint16_t brandIndex, const DatePair& datePair)
{
  if (!trx.getRequest()->brandedFareEntry())
    return true;

  std::multimap<std::string, DatePair>& validCalendarBrandIdMap = trx.validCalendarBrandIdMap();

  if (!validCalendarBrandIdMap.empty())
  {
    const std::string brandId = trx.getRequest()->brandId(brandIndex);
    typedef const std::pair<std::string, DatePair> InvBrandPair;
    for (InvBrandPair& invalidBrand : validCalendarBrandIdMap)
      if (invalidBrand.first == brandId && invalidBrand.second == datePair)
        return false;
  }

  return true;
}

namespace
{
// Execute func() for each avail segment span. Each span ends with an avail break.
//
// The func should have the following signature:
//   bool func(const std::vector<TravelSeg*>& travelSegVec, uint32_t limit, uint32_t firstSegIndex);
// The first argument is the segment span. The second argument counts how many segments are
// from this fare market. The third one is the index of the first span segment in the fare market.
//
// In case the last fare market segment has no avail break (and is not an arunk), the next
// fare market is taken in order to find the whole segment span that can be used as COSKey.
// If that happens, the limit value will be lower than travelSegVec.size().
//
// You should provide availBreakGetter(FareMarket*, uint32_t index) function that returns
// the avail break of segment index from the fare market.
template <typename Func, typename AvailBreakGetter>
bool
forEachAvailSegmentSpan(Itin* itin,
                        const std::vector<int32_t>& nextFareMarkets,
                        uint32_t fareMarketIndex,
                        Func func,
                        AvailBreakGetter availBreakGetter)
{
  FareMarket* const fareMarket = itin->fareMarket()[fareMarketIndex];

  std::vector<TravelSeg*> travelSegVec;

  for (const auto& el : boost::adaptors::index(fareMarket->travelSeg()))
  {
    travelSegVec.push_back(el.value());

    if (availBreakGetter(fareMarket, el.index()) == true)
    {
      if (!func(travelSegVec, travelSegVec.size(), el.index() + 1 - travelSegVec.size()))
        return false;

      travelSegVec.clear();
    }
  }

  if (std::find_if(travelSegVec.begin(),
                   travelSegVec.end(),
                   [](TravelSeg* seg)
                   { return seg->isAir(); }) == travelSegVec.end())
    return true;

  const uint32_t segmentsRemaining = travelSegVec.size();
  const uint32_t lastSpanIndex = fareMarket->travelSeg().size() - segmentsRemaining;

  int32_t nextFareMarketIndex = -1;
  while (fareMarketIndex < nextFareMarkets.size() &&
         (nextFareMarketIndex = nextFareMarkets[fareMarketIndex]) != -1)
  {
    fareMarketIndex = nextFareMarketIndex;

    FareMarket* nextFareMarket = itin->fareMarket()[nextFareMarketIndex];

    for (const auto& el : boost::adaptors::index(nextFareMarket->travelSeg()))
    {
      travelSegVec.push_back(el.value());

      if (availBreakGetter(nextFareMarket, el.index()) == true)
        return func(travelSegVec, segmentsRemaining, lastSpanIndex);
    }
  }

  return func(travelSegVec, segmentsRemaining, lastSpanIndex);
}
}

ClassOfServiceList*
ShoppingUtil::selectFareMarketCOS(PricingTrx& trx,
                                  Itin* itin,
                                  TravelSeg* travelSeg,
                                  ClassOfServiceList* cos,
                                  bool processPartOfJourneyWithMergedAvl /*=true*/)
{
  if (itin->interlineJourneyInfo().count(travelSeg) != 0)
  {
    return itin->interlineJourneyInfo()[travelSeg];
  }
  else if (ShoppingUtil::partOfJourney(itin, travelSeg))
  {
    if (processPartOfJourneyWithMergedAvl)
    {
      ClassOfServiceList* thruCos =
          &ShoppingUtil::getMaxThruClassOfServiceForSeg(trx, travelSeg);

      if (thruCos && !thruCos->empty())
        return thruCos;
    }
    else
    {
      ClassOfServiceList* jCos =
          ShoppingUtil::getFlowJourneyAvailability(trx, itin, travelSeg);

      if (LIKELY(jCos))
        return jCos;
    }
  }

  return cos;
}

std::vector<std::vector<ClassOfService*>*>
ShoppingUtil::getFMCOSBasedOnAvailBreakFromFU(PricingTrx& trx,
                                              FarePath* farePath,
                                              const std::vector<int32_t>& nextFareMarkets,
                                              const uint32_t fareMarketIndex)
{
  Itin* const itin = farePath->itin();

  std::vector<std::vector<ClassOfService*>*> avlVec;

  forEachAvailSegmentSpan(
      itin,
      nextFareMarkets,
      fareMarketIndex,
      [&](std::vector<TravelSeg*> travelSegVec, uint32_t limit, uint32_t /*firstSegIndex*/) -> bool
      {
        if (travelSegVec.size() == 1)
        {
          TravelSeg* const segment = travelSegVec.front();
          ClassOfServiceList* cos = ShoppingUtil::selectFareMarketCOS(
              trx, itin, segment, &segment->classOfService(), false);
          avlVec.push_back(cos);
        }
        else
        {
          std::vector<ClassOfServiceList>* cosVec =
              &ShoppingUtil::getClassOfService(trx, travelSegVec);

          if (cosVec == 0)
          {
            avlVec.clear();
            return false;
          }

          for (size_t index = 0; index < limit; ++index)
          {
            ClassOfServiceList* cos = ShoppingUtil::selectFareMarketCOS(
                trx, itin, travelSegVec[index], &(*cosVec)[index], false);
            avlVec.push_back(cos);
          }
        }

        return true;
      },
      [&](FareMarket* fareMarket, uint32_t segmentIndex) -> bool
      {
        for (PricingUnit* pu : farePath->pricingUnit())
          for (FareUsage* fu : pu->fareUsage())
            if (fu->paxTypeFare()->fareMarket() == fareMarket)
            {
              if (LIKELY(segmentIndex < fu->segmentStatus().size()))
                return fu->segmentStatus()[segmentIndex]._bkgCodeSegStatus.isSet(
                    PaxTypeFare::BKSS_AVAIL_BREAK);
              break;
            }

        if (segmentIndex < fareMarket->availBreaks().size())
          return fareMarket->availBreaks()[segmentIndex];

        // Fall back to the old path.
        throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION);
      });

  return avlVec;
}

void
ShoppingUtil::getFMCOSEmpty(PricingTrx& trx, FareMarket* fareMarket)
{
  typedef ClassOfServiceList CosList;
  typedef std::vector<ClassOfServiceList> MarketCosList;

  MarketCosList* cosVec = trx.dataHandle().create<MarketCosList>();
  cosVec->resize(fareMarket->travelSeg().size());
  fareMarket->classOfServiceVec().clear();

  for (CosList& cos : *cosVec)
    fareMarket->classOfServiceVec().push_back(&cos);
}

void
ShoppingUtil::getFMCOSBasedOnAvailBreak(PricingTrx& trx,
                                        Itin* itin,
                                        FareMarket* fareMarket,
                                        bool processPartOfJourneyWithMergedAvl /*=true*/)
{
  if (UNLIKELY(TrxUtil::isCosExceptionFixActivated(trx)))
  {
    try
    {
      getFMCOSBasedOnAvailBreakImpl(trx, itin, fareMarket, processPartOfJourneyWithMergedAvl);
    }
    catch (ErrorResponseException& ere)
    {
      getFMCOSEmpty(trx, fareMarket);
      fareMarket->failCode() = ere.code();
    }
  }
  else
  {
    getFMCOSBasedOnAvailBreakImpl(trx, itin, fareMarket, processPartOfJourneyWithMergedAvl);
  }
}

void
ShoppingUtil::getFMCOSBasedOnAvailBreakImpl(PricingTrx& trx,
                                            Itin* itin,
                                            FareMarket* fareMarket,
                                            bool processPartOfJourneyWithMergedAvl /*=true*/)
{
  std::vector<TravelSeg*> travelSegVec;
  std::vector<TravelSeg*>::const_iterator segIter = fareMarket->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = fareMarket->travelSeg().end();

  for (int i = 0; segIter != segIterEnd; ++segIter, ++i)
  {
    travelSegVec.push_back(*segIter);

    if (fareMarket->availBreaks()[i] == true)
    {
      if (travelSegVec.size() == 1)
      {
        ClassOfServiceList* cos = ShoppingUtil::selectFareMarketCOS(
            trx, itin, *segIter, &(*segIter)->classOfService(), processPartOfJourneyWithMergedAvl);
        fareMarket->classOfServiceVec().push_back(cos);
      }
      else
      {
        std::vector<ClassOfServiceList>* cosVec;
        cosVec = &ShoppingUtil::getClassOfService(trx, travelSegVec);

        if (UNLIKELY(cosVec == nullptr))
        {
          fareMarket->classOfServiceVec().clear();
          break;
        }

        for (size_t index = 0; index < cosVec->size(); ++index)
        {
          ClassOfServiceList* cos = ShoppingUtil::selectFareMarketCOS(
              trx, itin, travelSegVec[index], &(*cosVec)[index], processPartOfJourneyWithMergedAvl);
          fareMarket->classOfServiceVec().push_back(cos);
        }
      }

      travelSegVec.clear();
    }
  }
}

namespace
{
struct GetCOSByBookingCode
{
  GetCOSByBookingCode(const BookingCode& bc) : _bookingCode(bc) {}
  GetCOSByBookingCode(const GetCOSByBookingCode& other) : _bookingCode(other._bookingCode) {}

  bool operator()(const tse::ClassOfService* cos) { return cos->bookingCode() == _bookingCode; }

protected:
  const BookingCode& _bookingCode;
};
}

const ClassOfService*
ShoppingUtil::getCOS(const std::vector<ClassOfService*>& cosVec, const BookingCode& bc)
{
  std::vector<ClassOfService*>::const_iterator fit =
      std::find_if(cosVec.begin(), cosVec.end(), GetCOSByBookingCode(bc));

  if (fit == cosVec.end())
    return nullptr;

  return *fit;
}

//---------------------------------------------------------------------------
/// mergeClassOfService()
/// Function selects maximal availability from two ClassOfServiceList
/// vectors.
///
/// \param orig Availability will be merged into this vector.
/// \param tbm Availability vector to be merged to orig.
/// \return true if orig changed
//---------------------------------------------------------------------------
bool
ShoppingUtil::mergeClassOfService(PricingTrx& trx,
                                  ClassOfServiceList** orig,
                                  ClassOfServiceList& tbm)
{
  typedef ClassOfServiceList::iterator CosIterator;
  ClassOfServiceList* res = trx.dataHandle().create<ClassOfServiceList>();
  bool result = false;

  for (CosIterator it((*orig)->begin()), origItEnd((*orig)->end()); it != origItEnd; ++it)
  {
    CosIterator fit =
        std::find_if(tbm.begin(), tbm.end(), GetCOSByBookingCode((*it)->bookingCode()));

    if (fit == tbm.end() || (*it)->numSeats() > (*fit)->numSeats())
    {
      res->push_back(*it);
      result = true;
    }
    else if ((*it)->numSeats() < (*fit)->numSeats())
    {
      res->push_back(*fit);
      result = true;
    }
    else
    {
      res->push_back(*it);
    }
  }

  for (CosIterator it(tbm.begin()), itEnd(tbm.end()); it != itEnd; ++it)
  {
    CosIterator fit =
        std::find_if((*orig)->begin(), (*orig)->end(), GetCOSByBookingCode((*it)->bookingCode()));

    if (fit == (*orig)->end())
    {
      res->push_back(*it);
      result = true;
    }
  }

  if (result)
  {
    *orig = res;
  }

  return result;
}

void
ShoppingUtil::mergeFMCOSBasedOnAvailBreak(PricingTrx& trx,
                                          Itin* itin,
                                          FareMarket* fareMarket,
                                          bool processPartOfJourneyWithMergedAvl /*= true*/)
{
  if (TrxUtil::isCosExceptionFixActivated(trx))
  {
    try
    {
      mergeFMCOSBasedOnAvailBreakImpl(trx, itin, fareMarket, processPartOfJourneyWithMergedAvl);
    }
    catch (ErrorResponseException& ere)
    {
      getFMCOSEmpty(trx, fareMarket);
      fareMarket->failCode() = ere.code();
    }
  }
  else
  {
    mergeFMCOSBasedOnAvailBreakImpl(trx, itin, fareMarket, processPartOfJourneyWithMergedAvl);
  }
}

void
ShoppingUtil::mergeFMCOSBasedOnAvailBreakImpl(PricingTrx& trx,
                                              Itin* itin,
                                              FareMarket* fareMarket,
                                              bool processPartOfJourneyWithMergedAvl /*= true*/)
{
  bool changed = false;
  std::vector<TravelSeg*> travelSegVec;
  std::vector<TravelSeg*>::const_iterator segIter = fareMarket->travelSeg().begin();

  //  std::vector<TravelSeg*>::const_iterator segIterEnd = fareMarket->travelSeg().end();
  for (int i = 0; segIter != fareMarket->travelSeg().end(); ++segIter, ++i)
  {
    travelSegVec.push_back(*segIter);

    if (fareMarket->availBreaks()[i] == true)
    {
      if (travelSegVec.size() == 1)
      {
        ClassOfServiceList* cos = ShoppingUtil::selectFareMarketCOS(
            trx, itin, *segIter, &(*segIter)->classOfService(), processPartOfJourneyWithMergedAvl);
        ClassOfServiceList** cosFM = &fareMarket->classOfServiceVec()[i];

        if (cos && *cosFM && *cosFM != cos)
          changed |= mergeClassOfService(trx, cosFM, *cos);
      }
      else
      {
        std::vector<ClassOfServiceList>* cosVec;
        cosVec = &ShoppingUtil::getClassOfService(trx, travelSegVec);

        if (cosVec)
        {
          for (size_t index = 0; index < travelSegVec.size(); ++index)
          {
            ClassOfServiceList* cos =
                ShoppingUtil::selectFareMarketCOS(trx,
                                                  itin,
                                                  travelSegVec[index],
                                                  &(*cosVec)[index],
                                                  processPartOfJourneyWithMergedAvl);
            ClassOfServiceList** cosFM =
                &fareMarket->classOfServiceVec()[i + index - travelSegVec.size() + 1];

            if (cos && *cosFM && *cosFM != cos)
              changed |= mergeClassOfService(trx, cosFM, *cos);
          }
        }
      }

      travelSegVec.clear();
    }
  }

  if (changed)
    fareMarket->setMergedAvailability(changed);
}

ClassOfServiceList*
ShoppingUtil::getFlowJourneyAvailability(PricingTrx& trx, Itin* itin, TravelSeg* travelSegment)
{
  size_t segPos = 0;
  bool avlCosFound = false;

  std::vector<FareMarket*>::iterator fmI = itin->flowFareMarket().begin();
  std::vector<FareMarket*>::iterator fmE = itin->flowFareMarket().end();

  for (; fmI != fmE; ++fmI)
  {
    FareMarket& fm = *(*fmI);

    std::vector<TravelSeg*>& travelSeg = fm.travelSeg();
    std::vector<TravelSeg*>::const_iterator foundTravelSeg =
        find(travelSeg.begin(), travelSeg.end(), travelSegment);

    if (foundTravelSeg != travelSeg.end())
    {
      segPos = foundTravelSeg - travelSeg.begin();
      avlCosFound = true;
      break;
    }
  }

  if (LIKELY(avlCosFound))
  {
    FareMarket& fm = *(*fmI);
    std::vector<TravelSeg*>& travelSeg = fm.travelSeg();
    std::vector<ClassOfServiceList>& cosVec =
        ShoppingUtil::getClassOfService(trx, travelSeg);

    if (LIKELY(travelSeg.size() == cosVec.size() && segPos < cosVec.size()))
    {
      return &cosVec[segPos];
    }
  }

  return nullptr;
}

Itin*
ShoppingUtil::getItinForFareMarketMIP(const PricingTrx& trx, const FareMarket* fareMarket)
{
  for (Itin* itin : trx.itin())
  {
    std::vector<FareMarket*>::const_iterator fareMarketIter =
        std::find(itin->fareMarket().begin(), itin->fareMarket().end(), fareMarket);

    // Assume that fare markets are not shared across itineraries
    if (fareMarketIter != itin->fareMarket().end())
    {
      return itin;
    }
  }
  return nullptr;
}

bool
ShoppingUtil::checkMinConnectionTime(const TravelSeg* firstTravelSeg,
                                     const TravelSeg* secondTravelSeg,
                                     int64_t minimumConnectTime)
{
  return (DateTime::diffTime(secondTravelSeg->departureDT(), firstTravelSeg->arrivalDT()) >=
          minimumConnectTime);
}

bool
ShoppingUtil::checkMinConnectionTime(const TravelSeg* firstTravelSeg,
                                     const TravelSeg* secondTravelSeg,
                                     const PricingOptions* pricingOptions)
{
  const TravelSeg* seg1 = firstTravelSeg;
  const TravelSeg* seg2 = secondTravelSeg;

  if (seg1->arrivalDT() > seg2->departureDT())
    return false;

  // If the next segment is an international trip, we must leave longer to connect
  const bool isInternational = LocUtil::isInternational(*seg2->origin(), *seg2->destination());

  const int64_t minConnectionTime = isInternational
                                        ? pricingOptions->getMinConnectionTimeInternational()
                                        : pricingOptions->getMinConnectionTimeDomestic();

  // If minimum connect time check is enabled
  if (LIKELY(minConnectionTime > 0))
  {
    if (!ShoppingUtil::checkMinConnectionTime(seg1, seg2, minConnectionTime))
      return false;
  }

  return true;
}

bool
ShoppingUtil::checkMinConnectionTime(const PricingOptions* pricingOptions,
                                     const SopIdVec& sops,
                                     const LegVec& legs)
{
  for (size_t n = 0; n < sops.size() - 1; ++n)
  {
    TSE_ASSERT(n + 1 < legs.size());
    TSE_ASSERT(size_t(sops[n]) < legs[n].sop().size());
    TSE_ASSERT(size_t(sops[n + 1]) < legs[n + 1].sop().size());
    const ShoppingTrx::SchedulingOption& sop1 = legs[n].sop()[sops[n]];
    const ShoppingTrx::SchedulingOption& sop2 = legs[n + 1].sop()[sops[n + 1]];

    const TravelSeg* const seg1 = sop1.itin()->travelSeg().back();
    const TravelSeg* const seg2 = sop2.itin()->travelSeg().front();

    if (!ShoppingUtil::checkMinConnectionTime(seg1, seg2, pricingOptions))
      return false;
  }

  return true;
}

bool
ShoppingUtil::checkPositiveConnectionTime(const SopIdVec& sops, const LegVec& legs)
{
  for (size_t n = 0; n < sops.size() - 1; ++n)
  {
    TSE_ASSERT(n + 1 < legs.size());
    TSE_ASSERT(size_t(sops[n]) < legs[n].sop().size());
    TSE_ASSERT(size_t(sops[n + 1]) < legs[n + 1].sop().size());
    const ShoppingTrx::SchedulingOption& sop1 = legs[n].sop()[sops[n]];
    const ShoppingTrx::SchedulingOption& sop2 = legs[n + 1].sop()[sops[n + 1]];

    const TravelSeg* const seg1 = sop1.itin()->travelSeg().back();
    const TravelSeg* const seg2 = sop2.itin()->travelSeg().front();

    if (seg1->arrivalDT() >= seg2->departureDT())
      return false;
  }

  return true;
}

void
ShoppingUtil::mergeCxrKeys(const shpq::CxrKeys& newCxrKeys, shpq::CxrKeys& carriers)
{
  if (carriers.empty())
  {
    carriers = newCxrKeys;
    return;
  }

  shpq::CxrKeys intersection;
  std::set_intersection(newCxrKeys.begin(),
                        newCxrKeys.end(),
                        carriers.begin(),
                        carriers.end(),
                        std::back_inserter(intersection));
  carriers.swap(intersection);
}

void
ShoppingUtil::mergeCxrKeys(const FareMarket& fm, shpq::CxrKeys& carriers)
{
  TSE_ASSERT(fm.getApplicableSOPs());
  shpq::CxrKeys newCxrKeys;
  newCxrKeys.reserve(fm.getApplicableSOPs()->size());

  for (const ApplicableSOP::value_type& cxrSops : *fm.getApplicableSOPs())
    newCxrKeys.push_back(cxrSops.first);

  mergeCxrKeys(newCxrKeys, carriers);
}

static inline void
throwUnknownExc(const char* msg)
{
  throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION, msg);
}

void
ShoppingUtil::collectFPCxrKeys(const FarePath& farePath,
                               const size_t noOfLegs,
                               shpq::CxrKeysPerLeg& result)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();
      const FareMarket& fm = *ptf.fareMarket();
      shpq::CxrKeys& cxrKeys = result[fm.legIndex()];
      mergeCxrKeys(fm, cxrKeys);

      if (UNLIKELY(cxrKeys.empty()))
        throwUnknownExc("Internal processing error. Different carriers detected in leg.");
    }
  }

  if (UNLIKELY(result.size() != noOfLegs))
    throwUnknownExc("Internal processing error. No carrier(s) detected in some legs.");
}

bool
ShoppingUtil::collectFPCxrKeysNew(ShoppingTrx& trx,
                                  const FarePath& farePath,
                                  const size_t noOfLegs,
                                  shpq::CxrKeysPerLeg& result)
{
  Diag910Collector* const collector =
      dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(trx));

  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();
      const FareMarket& fm = *ptf.fareMarket();
      shpq::CxrKeys& cxrKeys = result[fm.legIndex()];
      mergeCxrKeys(fm, cxrKeys);

      if (UNLIKELY(cxrKeys.empty()))
      {
        if (collector != nullptr)
        {
          *collector << "cxrKeys.empty and its "
                     << "requestedPaxType = " << farePath.paxType()->requestedPaxType() << "\n";
          *collector << "totalNUCAmount = " << farePath.getTotalNUCAmount() << "\n";
          ;
          (*collector).flushMsg();
        }
        return false;
      }
    }
  }

  if (UNLIKELY(result.size() != noOfLegs))
  {
    if (collector != nullptr)
    {
      *collector << "\n"
                 << "!!!result.size() != noOfLegs"
                 << "\n";
      (*collector).flushMsg();
    }
    return false;
  }

  return true;
}

void
ShoppingUtil::collectSopsCxrKeys(const ShoppingTrx& trx,
                                 const SopIdVec& sops,
                                 shpq::CxrKeyPerLeg& result)
{
  TSE_ASSERT(sops.size() == trx.legs().size());

  for (size_t legId = 0; legId < sops.size(); ++legId)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legId];
    const ShoppingTrx::SchedulingOption& sop = leg.sop()[sops[legId]];
    createCxrKey(sop.governingCarrier(), result[legId]);
  }

  if (UNLIKELY(result.size() != sops.size()))
    throwUnknownExc("Internal processing error. No carrier(s) detected in some legs.");
}

void
ShoppingUtil::collectOnlineCxrKeys(const shpq::CxrKeysPerLeg& cxrKeysPerLeg,
                                   shpq::CxrKeys& onlineCxrKeys,
                                   bool& isInterlineApplicable)
{
  bool multipleCxrsOnLeg = false;

  for (const shpq::CxrKeys& ck : cxrKeysPerLeg)
  {
    multipleCxrsOnLeg |= (ck.size() > 1u);
    mergeCxrKeys(ck, onlineCxrKeys);
  }

  isInterlineApplicable = cxrKeysPerLeg.size() > 1u && (multipleCxrsOnLeg || onlineCxrKeys.empty());
}

bool
ShoppingUtil::isOnlineConnectionFlight(const int& sopNum,
                                       const int& legIndex,
                                       const CarrierCode* carrier,
                                       const ShoppingTrx* trx,
                                       const bool needDirectFlt,
                                       const bool flightOnlySolution)
{
  if (UNLIKELY(carrier == nullptr))
    return false;

  const ShoppingTrx::SchedulingOption& sop = trx->legs()[legIndex].sop()[sopNum];

  if ((sop.itin()->travelSeg().size() == 1) && (!needDirectFlt))
    return false;

  if ((flightOnlySolution == true) && (sop.itin()->travelSeg().size() == 2))
  {
    if (*carrier == sop.governingCarrier())
      return true;
    return false;
  }

  // check whether sop is online flight
  const std::vector<TravelSeg*>& segs = sop.itin()->travelSeg();

  for (std::vector<TravelSeg*>::const_iterator s = segs.begin(); s != segs.end(); ++s)
  {
    const AirSeg* const airSeg = dynamic_cast<const AirSeg*>(*s);

    if (UNLIKELY(airSeg == nullptr))
      continue;

    if (*carrier != airSeg->marketingCarrierCode())
      return false;
  }
  return true;
}

bool
ShoppingUtil::isOnlineOptionForCarrier(const ShoppingTrx::SchedulingOption* outbound,
                                       const ShoppingTrx::SchedulingOption* inbound,
                                       const CarrierCode& carrier)
{
  if (outbound != nullptr && inbound != nullptr)
    return isOnlineFlight(*outbound->itin(), carrier) && isOnlineFlight(*inbound->itin(), carrier);
  if (outbound != nullptr)
    return isOnlineFlight(*outbound->itin(), carrier);
  else
    return false;
}

bool
ShoppingUtil::checkForcedConnection(const SopIdVec& sops, const ShoppingTrx& trx)
{
  std::set<TravelSeg*> usedTvlSeg;
  const ShoppingTrx::ForcedConnectionSet& forcedConnection = trx.forcedConnection();
  ShoppingTrx::ForcedConnectionSet::const_iterator fcIter = forcedConnection.begin();

  for (; fcIter != forcedConnection.end(); ++fcIter)
  {
    bool found = false;

    for (uint32_t legNo = 0; legNo < sops.size(); ++legNo)
    {
      const int sopNo = sops[legNo];
      TSE_ASSERT(legNo < trx.legs().size());
      TSE_ASSERT(size_t(sopNo) < trx.legs()[legNo].sop().size());
      const ShoppingTrx::SchedulingOption& sop = trx.legs()[legNo].sop()[sopNo];
      const std::vector<TravelSeg*>& tvlSeg = sop.itin()->travelSeg();
      std::vector<TravelSeg*>::const_iterator tvlSegIter = tvlSeg.begin();

      for (; tvlSegIter != tvlSeg.end(); ++tvlSegIter)
      {
        // last seg in last leg (without checking if it's connection) OR connection
        if ((legNo + 1 == trx.legs().size() && tvlSegIter + 1 == tvlSeg.end()) ||
            (!(*tvlSegIter)->isForcedStopOver() && !(*tvlSegIter)->stopOver())) // connection
        {
          if (usedTvlSeg.count(*tvlSegIter) == 0 && (*tvlSegIter)->offMultiCity() == *fcIter)
          {
            found = true;
            usedTvlSeg.insert(*tvlSegIter);
            break;
          }
        }
      }

      if (found)
        break;
    }

    if (!found)
      return false;
  }

  return true;
}

bool
ShoppingUtil::checkOverridedSegment(const SopIdVec& sops, const ShoppingTrx& trx)
{
  CarrierCode cxrOverride = trx.getRequest()->cxrOverride();

  for (uint32_t legNo = 0; legNo < sops.size(); ++legNo)
  {
    const int sopNo = sops[legNo];
    const ShoppingTrx::SchedulingOption& sop = trx.legs()[legNo].sop()[sopNo];
    const std::vector<TravelSeg*>& tvlSeg = sop.itin()->travelSeg();
    std::vector<TravelSeg*>::const_iterator tvlSegIter = tvlSeg.begin();

    for (; tvlSegIter != tvlSeg.end(); ++tvlSegIter)
    {
      const AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlSegIter);

      if (airSeg && airSeg->carrier() == cxrOverride)
      {
        return true;
      }
    }
  }

  return false;
}

bool
ShoppingUtil::isCustomSolutionGfp(const ShoppingTrx& trx, GroupFarePath* gfp)
{
  bool customLegFound = false;
  for (std::vector<FPPQItem*>::iterator it = gfp->groupFPPQItem().begin();
       it != gfp->groupFPPQItem().end();
       ++it)
  {
    FarePath& fp = *(*it)->farePath();
    for (std::vector<PricingUnit*>::iterator p = fp.pricingUnit().begin();
         p != fp.pricingUnit().end();
         ++p)
    {
      for (std::vector<FareUsage*>::iterator f = (*p)->fareUsage().begin();
           f != (*p)->fareUsage().end();
           ++f)
      {
        const FareMarket* fm = (*f)->paxTypeFare()->fareMarket();
        if (isCustomLegFM(trx, fm))
        {
          customLegFound = true;
          if (!trx.isCustomSolutionFM(fm))
          {
            // In custom Leg, all FMs have to be custom
            return false;
          }
        }
      }
    }
  }

  // If we have found custom legs and we didn't exited earlier,
  // that means all FMs associated to the custom legs are custom FM
  return customLegFound;
}

bool
ShoppingUtil::isCustomLegFM(const ShoppingTrx& trx, const FareMarket* fm)
{
  return (trx.legs()[fm->legIndex()].isCustomLeg());
}

bool
ShoppingUtil::isOpenJaw(const ShoppingTrx& shoppingTrx)
{
  TSE_ASSERT(shoppingTrx.legs().size() == 2);

  if (shoppingTrx.legs().front().sop().empty())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "No flight segments in the first stopover");
  }
  const ShoppingTrx::SchedulingOption& firstLegSOP = shoppingTrx.legs().front().sop().front();
  const TravelSeg* obFrontTvlSeg = firstLegSOP.itin()->travelSeg().front();
  const TravelSeg* obBackTvlSeg = firstLegSOP.itin()->travelSeg().back();
  const LocCode obBoardCity = obFrontTvlSeg->boardMultiCity();
  const LocCode obOffCity = obBackTvlSeg->offMultiCity();
  const ShoppingTrx::Leg inbLeg = shoppingTrx.legs().back();

  const AirSeg* airSegFront = (dynamic_cast<const AirSeg*>(obFrontTvlSeg));
  const AirSeg* airSegBack = (dynamic_cast<const AirSeg*>(obBackTvlSeg));

  std::vector<ShoppingTrx::SchedulingOption>::const_iterator inbSop = inbLeg.sop().begin();

  for (; inbSop != inbLeg.sop().end(); ++inbSop)
  {
    const TravelSeg* obTvlSeg = inbSop->itin()->travelSeg().back();
    const TravelSeg* inbTvlSeg = inbSop->itin()->travelSeg().front();
    if (obBoardCity == obTvlSeg->offMultiCity())
    {
      if (obOffCity == inbTvlSeg->boardMultiCity())
      {
        return false;
      }
      else
      {
        const AirSeg* obAirSeg = (dynamic_cast<const AirSeg*>(obTvlSeg));
        if ((obAirSeg != nullptr) && (airSegBack != nullptr))
        {
          if (airSegBack->carrier() == obAirSeg->carrier())
          {
            return true;
          }
        }
      }
    }
    else
    {
      const AirSeg* inbAirSeg = (dynamic_cast<const AirSeg*>(inbTvlSeg));
      if ((airSegFront != nullptr) && (inbAirSeg != nullptr))
      {
        if (airSegFront->carrier() == inbAirSeg->carrier())
        {
          return true;
        }
      }
    }
  }
  return true;
}

bool
ShoppingUtil::isBrazilDomestic(const ShoppingTrx& shoppingTrx)
{
  TSE_ASSERT((shoppingTrx.legs().size() == 1) || (shoppingTrx.legs().size() == 2));

  if (shoppingTrx.legs().front().sop().empty())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "No flight segments in the first stopover");
  }
  const ShoppingTrx::SchedulingOption& firstLegSOP = shoppingTrx.legs().front().sop().front();
  const TravelSeg* originTS = firstLegSOP.itin()->travelSeg().front();
  const Loc* origin = originTS->origin();

  const ShoppingTrx::SchedulingOption& lastLegSOP = shoppingTrx.legs().back().sop().front();
  const TravelSeg* destTS = (shoppingTrx.legs().size() == 1)
                                ? lastLegSOP.itin()->travelSeg().back()
                                : lastLegSOP.itin()->travelSeg().front();
  const Loc* dest = (shoppingTrx.legs().size() == 1) ? destTS->destination() : destTS->origin();

  return ((origin->nation() == dest->nation()) && (NATION_BRAZIL == origin->nation()));
}

bool
ShoppingUtil::isSpanishDiscountApplicableForShopping(const PricingTrx& trx, const Itin* itin)
{
  const SLFUtil::DiscountLevel spanishDiscountLevel =
      trx.getOptions()->getSpanishLargeFamilyDiscountLevel();

  return (UNLIKELY(itin && trx.getRequest() && trx.getRequest()->ticketingAgent() &&
                   (spanishDiscountLevel != SLFUtil::DiscountLevel::NO_DISCOUNT) &&
                   LocUtil::isSpain(*(trx.getRequest()->ticketingAgent()->agentLocation())) &&
                   LocUtil::isWholeTravelInSpain(itin->travelSeg())));
}

int
ShoppingUtil::getRequestedBrandIndex(PricingTrx& trx,
                                     const ProgramID& programId,
                                     const BrandCode& brandCode)
{
  for (unsigned int i = 0; i < trx.brandProgramVec().size(); ++i)
  {
    if ((trx.brandProgramVec()[i].first->programID() == programId) &&
        (trx.brandProgramVec()[i].second->brandCode() == brandCode))
      return i;
  }

  return -1;
}

BrandCode&
ShoppingUtil::getBrandCode(const PricingTrx& trx, int index)
{
  return trx.brandProgramVec()[index].second->brandCode();
}

ProgramCode&
ShoppingUtil::getProgramCode(const PricingTrx& trx, int index)
{
  return trx.brandProgramVec()[index].first->programCode();
}

ProgramID&
ShoppingUtil::getProgramId(const PricingTrx& trx, int index)
{
  return trx.brandProgramVec()[index].first->programID();
}

std::string
ShoppingUtil::getBrandPrograms(const PricingTrx& trx, const FarePath* fPath)
{
  std::set<std::string> programIds;

  for (PricingUnit* pu : fPath->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      int id = fu->paxTypeFare()->getValidBrandIndex(trx, &(fPath->getBrandCode()),
                                                     fu->getFareUsageDirection());
      if (id != INVALID_BRAND_INDEX)
        programIds.insert(getProgramId(trx, id));
    }
  }

  return DiagnosticUtil::containerToString(programIds);
}

std::string
ShoppingUtil::getFarePathBrandCode(const FarePath* farePath)
{
  const BrandCode& brandCode = farePath->getBrandCode();
  if (brandCode != NO_BRAND)
    return brandCode;

  return "";
}

std::string
ShoppingUtil::getBrandCodeString(const PricingTrx& trx, int brandIndex)
{
  const BrandCode& brandCode = trx.validBrands()[brandIndex];

  if (isThisBrandReal(brandCode))
    return brandCode;

  return "";
}

std::string
ShoppingUtil::getFakeBrandString(const PricingTrx& trx, int brandIndex)
{
  std::string brand = trx.validBrands()[brandIndex];

  if (brand == ANY_BRAND_LEG_PARITY)
    return "BPL"; // Brand Parity Per Leg Path

  if (brand == ANY_BRAND && trx.isContextShopping())
    return "CTX"; // Context Shopping with all legs fixed

  return "CAB"; // Catch All Bucket
}

bool
ShoppingUtil::isThisBrandReal(const BrandCode& brand)
{
  BrandCodeSet invalidBrands{NO_BRAND, ANY_BRAND, ANY_BRAND_LEG_PARITY};

  if (invalidBrands.find(brand) == invalidBrands.end())
    return true;

  return false;
}

std::string
ShoppingUtil::getFareBrandProgram(const PricingTrx& trx,
                                  const BrandCode& brandCode,
                                  const PaxTypeFare* ptf,
                                  Direction fareUsageDirection)
{
  if (brandCode.empty() || (brandCode == NO_BRAND || brandCode == ANY_BRAND_LEG_PARITY))
    return "";

  int id = ptf->getValidBrandIndex(trx, &brandCode, fareUsageDirection);
  if (id != INVALID_BRAND_INDEX)
    return getProgramId(trx, id);

  return "";
}

char
ShoppingUtil::getIbfErrorCode(IbfErrorMessage ibfErrorMessage)
{
  switch (ibfErrorMessage)
  {
  case IbfErrorMessage::IBF_EM_NO_FARE_FILED:
    return IBF_EC_NO_FARE_FILED;
  case IbfErrorMessage::IBF_EM_EARLY_DROP:
    return IBF_EC_EARLY_DROP;
  case IbfErrorMessage::IBF_EM_NOT_OFFERED:
    return IBF_EC_NOT_OFFERED;
  case IbfErrorMessage::IBF_EM_NOT_AVAILABLE:
    return IBF_EC_NOT_AVAILABLE;
  case IbfErrorMessage::IBF_EM_NO_FARE_FOUND:
    return IBF_EC_NO_FARE_FOUND;
  case IbfErrorMessage::IBF_EM_NOT_SET:
    return IBF_EC_NOT_SET;
  default:
    return '?';
  }
}

bool
ShoppingUtil::isValidItinBrand(const Itin* itin, BrandCode brandCode)
{
  BrandCodeSet::iterator brIt = itin->brandCodes().find(brandCode);
  if (brIt != itin->brandCodes().end())
  {
    return true;
  }
  return false;
}

ShoppingUtil::FaresPerLegMap
ShoppingUtil::buildFaresPerLegMap(const FarePath& farePath)
{
  FaresPerLegMap faresPerLegMap;
  for (const PricingUnit* pu : farePath.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare* ptf = fu->paxTypeFare();
      const FareMarket* fm = ptf->fareMarket();
      for (const TravelSeg* tvlSeg : fm->travelSeg())
        faresPerLegMap[tvlSeg->legId()].insert(ptf);
    }

  return faresPerLegMap;
}

std::set<BrandCode>
ShoppingUtil::getHardPassedBrandsFromFares(const PricingTrx& trx,
                                           const std::set<const PaxTypeFare*>& fares)
{
  std::set<BrandCode> hardPassedBrands;
  for (const PaxTypeFare* ptf : fares)
  {
    std::vector<BrandCode> thisFareHardPassedBrands;
    if (UNLIKELY(!fallback::fallbackHalfRTPricingForIbf(&trx) && ptf->fareMarket()->useDummyFare()))
      thisFareHardPassedBrands.push_back(ANY_BRAND);
    else
      ptf->getValidBrands(trx, thisFareHardPassedBrands, true); // hard passed brands only

    std::copy(thisFareHardPassedBrands.begin(),
              thisFareHardPassedBrands.end(),
              std::inserter(hardPassedBrands, hardPassedBrands.end()));
  }
  return hardPassedBrands;
}

// Checks if any brand from the provided set is valid for all the fares passed
// to this function as a parameter. It returns the first brand that matches
// this criterion, disregarding the rest.
BrandCode
ShoppingUtil::getFirstBrandValidForAllFares(const PricingTrx& trx,
                                            const std::set<BrandCode>& brands,
                                            const std::set<const PaxTypeFare*>& fares)
{
  for (const BrandCode& brandCode : brands)
  {
    bool thisBrandIsValidForAllFares = true;
    for (const PaxTypeFare* ptf : fares)
    {
      if (!ptf->isValidForBrand(trx, &brandCode, false)) // soft of hard pass
      {
        thisBrandIsValidForAllFares = false;
        break;
      }
    }
    if (thisBrandIsValidForAllFares)
      return brandCode;
  }

  return BrandCode();
}

void
ShoppingUtil::removeBrandsNotPresentInFilter(std::set<BrandCode>& brands,
                                             const BrandFilterMap& filter)
{
  std::set<BrandCode> validBrands;
  for (const BrandCode& brand : brands)
  {
    if (filter.find(brand) != filter.end())
      validBrands.insert(brand);
  }
  brands = validBrands;
}

// if parity per leg is met for all legs then this function returns a map legId->brandCode
// which stores info about which brand provides parity on which leg
// if there's one or more legs on which parity is not met - an empty map is returned
ShoppingUtil::ParityBrandPerLegMap
ShoppingUtil::createParityBrandPerLegMap(const PricingTrx& trx, const FarePath& farePath)
{
  FaresPerLegMap faresPerLegMap = buildFaresPerLegMap(farePath);
  FaresPerLegMap::const_iterator legIter = faresPerLegMap.begin();

  const BrandFilterMap& brandFilter = farePath.itin()->brandFilterMap();
  std::vector<bool> fixedLegs(faresPerLegMap.size(), false);
  if (trx.isContextShopping())
    fixedLegs = trx.getFixedLegs();

  ParityBrandPerLegMap parityBrandPerLegMap;
  for (const auto& legFaresPair : faresPerLegMap)
  {
    const std::set<const PaxTypeFare*>& faresOnThisLeg = legFaresPair.second;
    std::set<BrandCode> hardPassedBrands = getHardPassedBrandsFromFares(trx, faresOnThisLeg);

    if (!brandFilter.empty() && !fixedLegs.at(legFaresPair.first)) //no filtering on fixed
    {
      // In FareMarketMerger.h we already validated fares against brand filter
      // but we had to pass soft passed fares that may be hardpassed in brands not requested.
      // Here we finalize brand filtering by only allowing hard passes present in filter.
      removeBrandsNotPresentInFilter(hardPassedBrands, brandFilter);
    }

    // check if any of hard passed brands is valid for all fares (as either hard or soft pass)
    BrandCode parityBrand = getFirstBrandValidForAllFares(trx, hardPassedBrands, faresOnThisLeg);

    if (parityBrand.empty())
      return ParityBrandPerLegMap();

    parityBrandPerLegMap[legFaresPair.first] = parityBrand;
  }

  return parityBrandPerLegMap;
}

void
ShoppingUtil::saveParityBrandsToFareUsages(
    FarePath& farePath,
    const PricingTrx& trx,
    const ShoppingUtil::ParityBrandPerLegMap& parityBrandPerLegMap)
{
  for (PricingUnit* pu : farePath.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare* ptf = fu->paxTypeFare();
      FareMarket* fm = ptf->fareMarket();
      uint16_t legId = fm->travelSeg().front()->legId();
      const BrandCode& parityBrand = parityBrandPerLegMap.at(legId);
      // we have to make sure that the brand we set on FU level is valid for inner PTfare
      TSE_ASSERT(ptf->isValidForBrand(trx, &parityBrand));
      fu->setBrandCode(parityBrand);
    }
}

std::string
ShoppingUtil::getFlexFaresValidationStatus(const Record3ReturnTypes& result)
{
  std::string str;
  switch (result)
  {
  case PASS:
    str = "PASS";
    break;

  case FAIL:
    str = "FAIL";
    break;

  case SOFTPASS:
    str = "SOFTPASS";
    break;

  default:
    str = "OTHER";
    break;
  }
  return str;
}

std::vector<FareMarket*>
ShoppingUtil::getUniqueFareMarketsFromItinMatrix(ShoppingTrx& trx)
{
  std::set<FareMarket*> allFareMarkets;
  for (unsigned legIndex = 0; legIndex < trx.legs().size(); ++legIndex)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIndex];
    const ItinIndex::ItinMatrix& itinMatrix = leg.carrierIndex().root();
    for (ItinIndex::ItinMatrix::const_iterator itinMatrixIt = itinMatrix.begin();
         itinMatrixIt != itinMatrix.end();
         ++itinMatrixIt)
    {
      const ItinIndex::Key& carrierKey = itinMatrixIt->first;
      ItinIndex::ItinCell* itinCell =
          ShoppingUtil::retrieveDirectItin(trx, legIndex, carrierKey, ItinIndex::CHECK_NOTHING);

      if (!itinCell || !itinCell->second)
        continue;

      Itin& itin = *itinCell->second;
      std::copy(itin.fareMarket().begin(),
                itin.fareMarket().end(),
                std::inserter(allFareMarkets, allFareMarkets.end()));
    }
  }
  return std::vector<FareMarket*>(allFareMarkets.begin(), allFareMarkets.end());
}

bool
ShoppingUtil::isDuplicatedFare(const PaxTypeFare* ptf1, const PaxTypeFare* ptf2)
{
  bool rc = false;

  if ((ptf1->getBaseFare() == ptf2->getBaseFare()) && (ptf1->fareTariff() == ptf2->fareTariff()) &&
      (ptf1->carrier() == ptf2->carrier()) && (ptf1->fareClass() == ptf2->fareClass()) &&
      (ptf1->originalFareAmount() >= ptf2->originalFareAmount()))
    rc = true;

  return rc;
}

void
ShoppingUtil::displayRemovedFares(const FareMarket& fm,
                                  const uint16_t& uniqueFareCount,
                                  const std::vector<PaxTypeFare*>& removedFaresVec,
                                  DiagManager& diag)
{
  Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
  diag325->displayRemovedFares(fm, uniqueFareCount, removedFaresVec);
}

void
ShoppingUtil::updateFinalBooking(FarePath& farePath)
{
  for (PricingUnit* pu : farePath.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      updateFinalBooking(farePath, *fu);

  farePath.propagateFinalBooking();
}

void
ShoppingUtil::updateFinalBookingBasedOnAvailBreaks(PricingTrx& trx, FarePath& farePath, Itin& itin)
{
  try
  {
    const std::vector<int32_t> nextFareMarkets = ItinUtil::generateNextFareMarketList(itin);

    for (PricingUnit* pu : farePath.pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        if (fu->paxTypeFare()->fareMarket()->useDummyFare())
          continue;

        FareMarket* const fareMarket = fu->paxTypeFare()->fareMarket();

        const uint32_t index =
            std::find(itin.fareMarket().begin(), itin.fareMarket().end(), fareMarket) -
            itin.fareMarket().begin();
        TSE_ASSERT(index < itin.fareMarket().size());

        const std::vector<std::vector<ClassOfService*>*> avlVec =
            ShoppingUtil::getFMCOSBasedOnAvailBreakFromFU(trx, &farePath, nextFareMarkets, index);

        if (LIKELY(!avlVec.empty()))
          ShoppingUtil::updateFinalBooking(farePath, *fu, avlVec);
        else
          ShoppingUtil::updateFinalBooking(farePath, *fu);
      }
    }

    farePath.propagateFinalBooking();
  }
  catch (ErrorResponseException&)
  {
    // Fall back to the old path.
    ShoppingUtil::updateFinalBooking(farePath);
  }
}

void
ShoppingUtil::updateFinalBooking(FarePath& farePath, FareUsage& fu)
{
  FareMarket& fareMarket = *fu.paxTypeFare()->fareMarket();
  TSE_ASSERT(fareMarket.travelSeg().size() == fu.travelSeg().size());
  updateFinalBooking(farePath, fu, fareMarket.classOfServiceVec());
}

void
ShoppingUtil::updateFinalBooking(FarePath& farePath,
                                 FareUsage& fu,
                                 const std::vector<std::vector<ClassOfService*>*>& avlVec)
{
  Itin& itin = *farePath.itin();

  const std::vector<TravelSeg*>& segs = fu.travelSeg();
  const std::vector<PaxTypeFare::SegmentStatus>& segmentStatus = fu.segmentStatus();

  // Skip if no availability in FareMarket is present.
  if (UNLIKELY(avlVec.size() == 0))
    return;

  const size_t segsSize = segs.size();
  const size_t avlSize = avlVec.size();

  for (size_t segIdx = 0, avlIdx = 0; segIdx < segsSize; ++segIdx)
  {
    const TravelSeg& seg = *segs[segIdx];
    if (!seg.isAir())
      continue;

    TSE_ASSERT(avlIdx < avlSize);
    const std::vector<ClassOfService*>& avl = *avlVec[avlIdx++];

    const PaxTypeFare::SegmentStatus& status = segmentStatus[segIdx];
    BookingCode bc = seg.getBookingCode();

    if (status._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      bc = status._bkgCodeReBook;

    ClassOfService* cos = findCOS(avl, bc);
    if (!cos)
      continue;

    const int16_t nr = itin.segmentOrder(&seg);
    if (UNLIKELY(nr <= 0))
      continue;

    if (farePath.finalBooking().size() < size_t(nr))
      farePath.mutableFinalBooking().resize(nr);

    farePath.mutableFinalBooking()[nr - 1] = cos;
  }
}

ClassOfService*
ShoppingUtil::findCOS(const std::vector<ClassOfService*>& avl, BookingCode bc)
{
  for (ClassOfService* cos : avl)
    if (cos->bookingCode() == bc)
      return cos;

  return nullptr;
}

bool
ShoppingUtil::isJCBMatched(const PricingTrx& trx)
{
  std::vector<PaxType*>::const_iterator paxTypeIter = trx.paxType().begin();
  for (; paxTypeIter != trx.paxType().end(); ++paxTypeIter)
  {
    if ((*paxTypeIter)->paxType() == JCB || (*paxTypeIter)->paxType() == JNN ||
        (*paxTypeIter)->paxType() == JNF)
      return true;
  }

  return false;
}

std::vector<CarrierCode>
ShoppingUtil::getGoverningCarriersPerLeg(const Itin& itin)
{
  std::vector<std::vector<TravelSeg*>> segmentsPerLeg;

  for (TravelSeg* travelSeg : itin.travelSeg())
  {
    const LegId legId = travelSeg->legId();
    if (segmentsPerLeg.size() <= legId)
      segmentsPerLeg.resize(legId + 1);

    segmentsPerLeg[legId].push_back(travelSeg);
  }

  std::vector<CarrierCode> gcPerLeg;

  GoverningCarrier gc;
  for (const auto& tsVec : segmentsPerLeg)
  {
    if (tsVec.empty())
      continue;

    std::set<CarrierCode> cxrSet;
    gc.getGoverningCarrier(tsVec, cxrSet);
    if (LIKELY(!cxrSet.empty()))
      gcPerLeg.push_back(*cxrSet.begin());
  }
  return gcPerLeg;
}

ShoppingUtil::LegBrandQualifiedIndex
ShoppingUtil::createLegBrandIndexRelation(const PricingTrx& trx, const Itin& itin)
{
  LegBrandQualifiedIndex result;

  std::vector<TravelSeg*>::const_iterator found;
  for (const FareMarket* fm : itin.fareMarket())
  {
    found = std::find(itin.travelSeg().begin(), itin.travelSeg().end(), fm->travelSeg().front());
    TSE_ASSERT(found != itin.travelSeg().end());
    size_t startSegmentIndex = found - itin.travelSeg().begin();
    found = std::find(itin.travelSeg().begin(), itin.travelSeg().end(), fm->travelSeg().back());
    TSE_ASSERT(found != itin.travelSeg().end());
    size_t endSegmentIndex = found - itin.travelSeg().begin();

    TSE_ASSERT(startSegmentIndex < itin.travelSeg().size());
    TSE_ASSERT(endSegmentIndex < itin.travelSeg().size());
    size_t startLegId = itin.travelSeg()[startSegmentIndex]->legId();
    size_t endLegId = itin.travelSeg()[endSegmentIndex]->legId();

    // verify if last part of the fare market ends at leg end
    if ((itin.travelSeg().size() > 1) && (endSegmentIndex < (itin.travelSeg().size() - 1)))
    {
      if (itin.travelSeg()[endSegmentIndex + 1]->legId() == (int16_t)endLegId)
      {
        // still on the same leg. if 1st leg, ignore fm, else "cut" it at last leg
        if (endLegId == 0)
          continue;
        --endLegId;
      }
    }

    std::map<BrandCode, int> fmBrandIndexMap;
    for (int index : fm->brandProgramIndexVec())
    {
      BrandCode brand = ShoppingUtil::getBrandCode(trx, index);
      if (fmBrandIndexMap.find(brand) == fmBrandIndexMap.end())
        fmBrandIndexMap[brand] = index;
    }

    for (size_t leg = startLegId; leg <= endLegId; ++leg)
      result[leg].insert(fmBrandIndexMap.begin(), fmBrandIndexMap.end());
  }

  return result;
}

namespace
{
void
printHardPassToDiag(DiagCollector* diag,
                    PaxTypeFare* ptf,
                    unsigned int brandIndex,
                    bool useDirectionality)
{
  if (LIKELY(!diag))
    return;

  *diag << "HARD PASS FOUND: " << ptf->fareClass();
  if (useDirectionality)
  {
    *diag << " DIRECTION: " << ptf->getBrandStatusVec()[brandIndex].second;
  }
  *diag << " IBF STATUS: " << ShoppingUtil::getIbfErrorCode(ptf->getIbfErrorMessage()) << "\n";
}

void
printFareToDiag(DiagCollector* diag, PaxTypeFare* ptf, unsigned int brandIndex, bool isSkipped)
{
  if (LIKELY(!diag))
    return;

  if (isSkipped)
    *diag << "SKIPPED ";

  *diag << "FARE : " << ptf->fareClass()
        << " / IBF STATUS : " << ShoppingUtil::getIbfErrorCode(ptf->getIbfErrorMessage()) << " / "
        << static_cast<char>(ptf->getBrandStatusVec()[brandIndex].first) << std::endl;
}

} // end unnamed namespace

bool
ShoppingUtil::hardPassForBothDirectionsFound(const std::set<Direction>& hardPassedDirections)
{
  if (hardPassedDirections.find(Direction::BOTHWAYS) != hardPassedDirections.end())
    return true;

  return hardPassedDirections.size() == 2;
}

void
ShoppingUtil::removeSoftPassesFromFareMarket(FareMarket* fm,
                                             unsigned int brandIndex,
                                             DiagCollector* diag,
                                             Direction direction)
{
  if (UNLIKELY(diag))
    *diag << "REMOVING SOFT PASSES FROM THIS FARE MARKET\n\n";

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    if (!ptf->isValid())
      continue;

    TSE_ASSERT(brandIndex < ptf->getBrandStatusVec().size());
    if (ptf->getBrandStatusVec()[brandIndex].first != PaxTypeFare::BS_SOFT_PASS)
      continue;

    // processing SOFT_PASS
    Direction& fareStatusDirection = ptf->getMutableBrandStatusVec()[brandIndex].second;

    if (direction == Direction::BOTHWAYS || fareStatusDirection == direction)
    {
      ptf->getMutableBrandStatusVec()[brandIndex].first = PaxTypeFare::BS_FAIL;
    }
    else if (fareStatusDirection == Direction::BOTHWAYS)
    {
      // if we are to remove soft passes in one direction and a given fare is valid in both
      // the other direction has to stay as a soft pass
      fareStatusDirection =
          (direction == Direction::REVERSED) ? Direction::ORIGINAL : Direction::REVERSED;
    }
  }
}

bool
ShoppingUtil::validHardPassExists(const FareMarket* fm,
                                  unsigned int brandIndex,
                                  std::set<Direction>& hardPassedDirections,
                                  DiagCollector* diag,
                                  bool useDirectionality,
                                  bool printAllFares)
{
  bool validHardPassExists = false;
  hardPassedDirections.clear();

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    TSE_ASSERT(ptf != nullptr);
    TSE_ASSERT(brandIndex < ptf->getBrandStatusVec().size());

    const PaxTypeFare::BrandStatus brandStatus = ptf->getBrandStatusVec()[brandIndex].first;

    if (brandStatus == PaxTypeFare::BS_FAIL)
      continue; // this fare doesn't match requested brand index

    if (brandStatus == PaxTypeFare::BS_SOFT_PASS)
    {
      if (printAllFares)
        printFareToDiag(diag, ptf, brandIndex, false);
      continue;
    }

    // BS_HARD_PASS processing

    // We should only use soft passes if hard passes are not offered on all the segments in a
    // given fare market
    const bool hardPassValid =
        ptf->isValid() || (ptf->isSoldOut() && !(ptf->isNotOfferedOnAllSegments()));

    if (!hardPassValid)
    {
      printFareToDiag(diag, ptf, brandIndex, true);
      continue;
    }

    validHardPassExists = true;
    printHardPassToDiag(diag, ptf, brandIndex, useDirectionality);

    if (!useDirectionality && !printAllFares)
      return true;

    // with directionality enabled we need to check hard passes for both directions
    const Direction direction = ptf->getBrandStatusVec()[brandIndex].second;
    hardPassedDirections.insert(direction);

    if (hardPassForBothDirectionsFound(hardPassedDirections) && !printAllFares)
      return true;
  }
  return validHardPassExists;
}

namespace
{
std::pair<const TravelSeg*, const TravelSeg*>
findLatestDepartureAndEarliestArrival(const std::vector<ShoppingTrx::SchedulingOption>& sops)
{
  std::pair<const TravelSeg*, const TravelSeg*> extremeSops;
  bool thisIsFirstSop = true;
  for (auto& sop : sops)
  {
    const TravelSeg* const seg1 = sop.itin()->travelSeg().front();
    const TravelSeg* const seg2 = sop.itin()->travelSeg().back();

    if (thisIsFirstSop)
    {
      extremeSops.first = seg1;
      extremeSops.second = seg2;
      thisIsFirstSop = false;
      continue;
    }

    const DateTime& latestDeparture = extremeSops.first->departureDT();
    if (latestDeparture < seg1->departureDT())
      extremeSops.first = seg1;

    const DateTime& earliestArrival = extremeSops.second->arrivalDT();
    if (earliestArrival > seg2->arrivalDT())
      extremeSops.second = seg2;
  }
  return extremeSops;
}

} // end namespace

// function that checks that minConnectionTime requirement is possible to meet
// between all the legs in the transaction
// if it is not, it returns id's (starting from 0) of legs before which CMT is not met.
// for example if mct is not met between legs of ids 3 and 4 then '4' will be returned.
bool
ShoppingUtil::isMinConnectionTimeMet(const std::vector<ShoppingTrx::Leg>& legs,
                                     const PricingOptions* options,
                                     bool allowIllogicalFlights,
                                     std::vector<uint16_t>& problematicLegs)
{
  problematicLegs.clear();

  if (legs.size() < 2)
    return true;

  // for most requests MCT will be met for all itins so first basic check will suffice
  // to let the transaction continue;
  if (mctIsMetForFirstSopOnEachLeg(legs, options, allowIllogicalFlights))
    return true;

  return mctIsMetForAnySopCombination(legs, options, allowIllogicalFlights, problematicLegs);
}

using CheckConnTimeFnPtr = bool (*)(const TravelSeg*, const TravelSeg*, const PricingOptions*);
using CheckConnTimeFn = std::function<bool(const TravelSeg*, const TravelSeg*, const PricingOptions*)>;

namespace {
  bool isPositiveConnTime(const TravelSeg* seg1, const TravelSeg* seg2, const PricingOptions*)
  {
    return seg1->arrivalDT() < seg2->departureDT();
  }
}

bool
ShoppingUtil::mctIsMetForAnySopCombination(const std::vector<ShoppingTrx::Leg>& legs,
                                           const PricingOptions* options,
                                           bool allowIllogicalFlights,
                                           std::vector<uint16_t>& problematicLegs)
{
  std::vector<std::pair<const TravelSeg*, const TravelSeg*>> extremeSopsPerLeg;
  extremeSopsPerLeg.reserve(legs.size());

  bool minConnectionTimeMet = true;

  CheckConnTimeFn checkConnTime = allowIllogicalFlights ? isPositiveConnTime :
    static_cast<CheckConnTimeFnPtr>(checkMinConnectionTime);

  for (uint16_t legId = 0; legId < legs.size(); ++legId)
  {
    extremeSopsPerLeg.push_back(findLatestDepartureAndEarliestArrival(legs[legId].sop()));

    if (legId == 0)
      continue;

    const TravelSeg* earliestArrivalOnPreviousLeg = extremeSopsPerLeg[legId - 1].second;
    const TravelSeg* latestDepartureOnThisLeg = extremeSopsPerLeg[legId].first;

    TSE_ASSERT(earliestArrivalOnPreviousLeg != NULL && latestDepartureOnThisLeg != NULL);

    if (!checkConnTime(earliestArrivalOnPreviousLeg, latestDepartureOnThisLeg, options))
    {
      minConnectionTimeMet = false;
      problematicLegs.push_back(legId);
    }
  }

  return minConnectionTimeMet;
}

bool
ShoppingUtil::mctIsMetForFirstSopOnEachLeg(const std::vector<ShoppingTrx::Leg>& legs,
                                           const PricingOptions* options,
                                           bool allowIllogicalFlights)
{
  if (legs.size() < 2)
    return true;

  CheckConnTimeFn checkConnTime = allowIllogicalFlights ? isPositiveConnTime :
    static_cast<CheckConnTimeFnPtr>(checkMinConnectionTime);

  for (uint16_t legId = 1; legId < legs.size(); ++legId)
  {
    const ShoppingTrx::SchedulingOption& previousSop = legs[legId - 1].sop().front();
    const ShoppingTrx::SchedulingOption& currentSop = legs[legId].sop().front();

    const TravelSeg* const arrivingSegFromPreviousLeg = previousSop.itin()->travelSeg().back();
    const TravelSeg* const departingSegFromCurrectLeg = currentSop.itin()->travelSeg().front();

    if (!checkConnTime(arrivingSegFromPreviousLeg, departingSegFromCurrectLeg, options))
      return false;
  }
  return true;
}

size_t
ShoppingUtil::getNumLegs(const Itin& itin)
{
  //Leg ID starts from 0
  TSE_ASSERT(!itin.travelSeg().empty());
  return itin.travelSeg().back()->legId() + 1;
}

} // tse
