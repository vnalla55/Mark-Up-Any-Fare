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

#include "ItinAnalyzer/ItinAnalyzerUtils.h"

#include "Common/Assert.h" // TSE_ASSERT
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "DataModel/Diversity.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag922Collector.h"
#include "Util/FlatSet.h"

#include <algorithm>
#include <set>
#include <vector>
#include <cmath>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackFMDirectionSetting)
FALLBACK_DECL(fallbackSOPUsagesResize);
FALLBACK_DECL(fallbackGovCxrForSubIata21);
FALLBACK_DECL(highestGovCxrCheckInForeignDomFareMkts);
namespace itinanalyzerutils
{
namespace
{
void
resolveGovCxrOverride(ShoppingTrx& trx, FareMarket* fareMarket)
{
  CarrierCode cxrOverride = trx.getRequest()->cxrOverride();
  if (UNLIKELY(cxrOverride != BLANK_CODE))
  {
    auto isSegWithOvrGovCxr = [fareMarket, cxrOverride](const TravelSeg* ts)
    {
      const AirSeg* airSeg = ts->toAirSeg();
      return airSeg && airSeg->carrier() == cxrOverride;
    };

    auto it = std::find_if(
        fareMarket->travelSeg().cbegin(), fareMarket->travelSeg().cend(), isSegWithOvrGovCxr);

    if (it != fareMarket->travelSeg().end())
    {
      fareMarket->primarySector() = *it;
      fareMarket->governingCarrier() = cxrOverride;
    }
  }
}
}
void
filterBrandsForIS(PricingTrx& trx, Diag892Collector* diag892)
{
  if (trx.getTrxType() != PricingTrx::IS_TRX)
    return;

  if (trx.getBrandsFilterForIS().empty())
  {
    //no filtering, just diag if needed
    if (diag892 && diag892->isDDINFO())
      diag892->printBrandsRemovedFromTrx(trx, trx.brandProgramVec());
  }
  else
  {
    std::vector<QualifiedBrand> newQualifiedBrands;

    const int REMOVED = -1;
    std::map<int, int> oldToNewIdx;
    int oldIdx = 0;
    int newIdx = 0;

    for (QualifiedBrand& qBrand: trx.brandProgramVec())
    {
      BrandCode brandCode = qBrand.second->brandCode();
      if (trx.getBrandsFilterForIS().find(brandCode) !=
          trx.getBrandsFilterForIS().end())
      {
        newQualifiedBrands.push_back(qBrand);
        oldToNewIdx[oldIdx] = newIdx;
        ++newIdx;
      }
      else
      {
        oldToNewIdx[oldIdx] = REMOVED;
      }
      ++oldIdx;
    }
    if (diag892 && diag892->isDDINFO())
      diag892->printBrandsRemovedFromTrx(trx, newQualifiedBrands);

    for (FareMarket* fm: trx.fareMarket())
    {
      std::vector<int> newBrandProgramIndexVec;
      for (int brandIndex: fm->brandProgramIndexVec())
      {
        TSE_ASSERT(trx.brandProgramVec()[brandIndex].second != nullptr);
        TSE_ASSERT(oldToNewIdx.find(brandIndex) != oldToNewIdx.end());
        if (oldToNewIdx.at(brandIndex) != REMOVED)
        {
          newBrandProgramIndexVec.push_back(oldToNewIdx.at(brandIndex));
        }
      }
      fm->brandProgramIndexVec() = newBrandProgramIndexVec;
    }
    trx.brandProgramVec() = newQualifiedBrands;
  }

  if (diag892)
    diag892->flushMsg();
}

bool
isThruFareMarket(const FareMarket& fm, const Itin& itin)
{
  TSE_ASSERT(!fm.travelSeg().empty());

  const TravelSeg* firstTvlSeg = fm.travelSeg().front();
  const TravelSeg* lastTvlSeg = fm.travelSeg().back();

  // Now check if the fare market
  // 'fills completly' a leg

  // Check if it starts a leg
  std::vector<TravelSeg*>::const_iterator foundTvlSeg =
      std::find(itin.travelSeg().begin(), itin.travelSeg().end(), firstTvlSeg);
  TSE_ASSERT(foundTvlSeg != itin.travelSeg().end());

  bool isThruFMStart = (*foundTvlSeg == itin.travelSeg().front() ||
                        ((*(foundTvlSeg - 1))->legId() != firstTvlSeg->legId()));
  if (false == isThruFMStart)
  {
    return false;
  }

  // Check if it finishes a leg
  foundTvlSeg = std::find(itin.travelSeg().begin(), itin.travelSeg().end(), lastTvlSeg);
  TSE_ASSERT(foundTvlSeg != itin.travelSeg().end());
  return (*foundTvlSeg == itin.travelSeg().back() ||
          ((*(foundTvlSeg + 1))->legId() != lastTvlSeg->legId()));
}
void
collectUniqueMarketsForItins(const std::vector<Itin*>& itins,
                             std::set<const FareMarket*>& unique_fms)
{
  unique_fms.clear();

  for (const auto itin : itins)
  {
    TSE_ASSERT(nullptr != itin);
    // For all fare markets inside an itin
    for (const auto fm : itin->fareMarket())
    {
      TSE_ASSERT(nullptr != fm);
      unique_fms.insert(fm);
    }
  }
}

namespace
{
struct IsFareMarketInSet : public std::unary_function<const FareMarket*, bool>
{
public:
  IsFareMarketInSet(const std::set<const FareMarket*>& fmSet) : _fmSet(fmSet) {}

  bool operator()(const FareMarket* fm) const
  {
    TSE_ASSERT(nullptr != fm);
    return _fmSet.find(fm) != _fmSet.end();
  }

private:
  const std::set<const FareMarket*>& _fmSet;
};
}

void
removeTrxFMsNotReferencedByAnyItin(PricingTrx& trx)
{
  // From trx, remove all fare markets
  // that are no longer referenced from any itin
  std::set<const FareMarket*> markets_in_all_itins;
  collectUniqueMarketsForItins(trx.itin(), markets_in_all_itins);

  IsFareMarketInSet isMarketReferencedAnywhere(markets_in_all_itins);
  trx.fareMarket().erase(std::remove_if(trx.fareMarket().begin(),
        trx.fareMarket().end(),
        std::not1(isMarketReferencedAnywhere)),
      trx.fareMarket().end());
}

void
removeFMsExceedingLegs(std::vector<Itin*>& itins)
{
  for (Itin* itin : itins)
  {
    itin->fareMarket().erase(std::remove_if(itin->fareMarket().begin(),
                                            itin->fareMarket().end(),
                                            [](FareMarket* fm)
                                            {
                               TSE_ASSERT(nullptr != fm);
                               return fm->doesSpanMoreThanOneLeg();
                             }),
                             itin->fareMarket().end());
  }
}

void
setItinLegs(Itin* itin)
{
  itin->itinLegs().clear();

  std::map<uint16_t, std::vector<TravelSeg*> > legs;
  for (TravelSeg* tSeg : itin->travelSeg())
  legs[tSeg->legId()].push_back(tSeg);

  std::map<uint16_t, std::vector<TravelSeg*> >::iterator i = legs.begin();
  std::map<uint16_t, std::vector<TravelSeg*> >::iterator iEnd = legs.end();
  for (; i != iEnd; ++i)
  {
    std::vector<TravelSeg*>& legSegments = i->second;

    TravelSeg* firstSeg = legSegments[0];
    if (firstSeg->segmentType() == Arunk)
    {
      itin->itinLegs().push_back(std::vector<TravelSeg*>(1, firstSeg));

      if (legSegments.size() == 1)
        continue;

      legSegments.erase(legSegments.begin());
    }

    TravelSeg* lastSeg = legSegments.back();
    bool lastSegArunk = (lastSeg->segmentType() == Arunk);

    if (lastSegArunk)
      legSegments.pop_back();

    if (!legSegments.empty())
      itin->itinLegs().push_back(legSegments);

    if (lastSegArunk)
      itin->itinLegs().push_back(std::vector<TravelSeg*>(1, lastSeg));
  }
}

// Leg::getFlightBitmapSize(...) does not count fake itin
int
getNumSOPs(const ItinIndex::ItinRow& row)
{
  int result(0);
  for (const ItinIndex::ItinRow::value_type& pr : row)
  {
    for (const ItinIndex::ItinColumn::value_type& pc : pr.second)
    {
      if (!pc.second->isDummy())
        ++result;
    }
  }
  return result;
}

Itin*
getFirstItin(const ItinIndex::ItinRow& row)
{
  Itin* firstItin(nullptr);
  if (!row.empty())
  {
    const ItinIndex::ItinColumn& column(row.begin()->second);
    if (!column.empty())
    {
      firstItin = column.front().second;
    }
  }
  return firstItin;
}

CarrierCode
getGovCarrier(const Itin* firstItin)
{
  CarrierCode result;
  if (firstItin != nullptr && !firstItin->fareMarket().empty())
  {
    FareMarket* thruFm(firstItin->fareMarket().front());
    result = thruFm->governingCarrier();
  }
  return result;
}

void
createSopUsages(ShoppingTrx& trx)
{
  Itin& journeyItin = *trx.journeyItin();
  std::map<ItinIndex::Key, CarrierCode> carrierMap;

  for (ShoppingTrx::Leg& curLeg : trx.legs())
  {
    if (curLeg.stopOverLegFlag())
      continue;

    Itin2SopInfoMap itinOrigSopInfoMap = getSopInfoMap(journeyItin, curLeg);

    ItinIndex::ItinMatrixIterator matrixIt(curLeg.carrierIndex().root().begin());
    for (; matrixIt != curLeg.carrierIndex().root().end(); ++matrixIt)
    {
      const uint32_t carrierKey(matrixIt->first);
      const Itin* firstItin(getFirstItin(matrixIt->second));
      CarrierCode govCxr(getGovCarrier(firstItin));

      int numAllSOPs(getNumSOPs(matrixIt->second));
      GovCxrGroupParameters groupParameters(carrierKey, govCxr, numAllSOPs);
      carrierMap.insert(std::make_pair(carrierKey, govCxr));

      // reuse all thru fare markets
      if (firstItin && !firstItin->fareMarket().empty())
      {
        for (FareMarket* fareMarket : firstItin->fareMarket())
        {
          fareMarket->setFmTypeSol(FareMarket::SOL_FM_THRU);
          determineFMDirection(fareMarket, LocCode());
        }

        // initialize thru fm(s)
        for (FareMarket* fareMarket : firstItin->fareMarket())
        {
          ItinIndex::ItinIndexIterator rowIt(curLeg.carrierIndex().beginRow(carrierKey));
          for (; rowIt != curLeg.carrierIndex().endRow(); ++rowIt)
          {
            Itin* curItin(rowIt->second);

            Itin2SopInfoMap::const_iterator it(itinOrigSopInfoMap.find(curItin));
            SopInfo origSopInfo =
                (it != itinOrigSopInfoMap.end() ? it->second : SopInfo(-1, false));

            initializeSopUsages(trx,
                                fareMarket,
                                curItin,
                                curItin->travelSeg(),
                                0,
                                origSopInfo.first,
                                rowIt.bitIndex(),
                                groupParameters);
          }
        }
      }
    }
  }
  printDiagnostic922(trx, carrierMap);
}

Itin2SopInfoMap
getSopInfoMap(Itin& journeyItin, const ShoppingTrx::Leg& leg)
{
  Itin2SopInfoMap itinOrigSopInfoMap;
  for (const ShoppingTrx::SchedulingOption& sched : leg.sop())
  {
    const Itin* itin(sched.itin());
    if (!itin->isDummy())
    {
      if (journeyItin.getMaxDepartureDT() < itin->travelSeg().back()->departureDT())
        journeyItin.setMaxDepartureDT(itin->travelSeg().back()->departureDT());

      itinOrigSopInfoMap.insert(
          std::make_pair(itin, std::make_pair(sched.originalSopId(), sched.isCustomSop())));
    }
  }
  return itinOrigSopInfoMap;
}

void determineFMDirection(FareMarket* fareMarket,
                          const LocCode& sopDestination)
{
  const int16_t legIndex = fareMarket->legIndex();
  if (!fallback::fixed::fallbackFMDirectionSetting())
  {
    const bool isThru = (fareMarket->getFmTypeSol() == FareMarket::SOL_FM_THRU);
    if (0 == legIndex)
    {
      if (isThru || fareMarket->destination()->loc() != sopDestination)
        fareMarket->direction() = FMDirection::OUTBOUND;
    }
    else
    {
      if (!fareMarket->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
      {
        if (isThru || fareMarket->destination()->loc() == sopDestination)
          fareMarket->direction() = FMDirection::INBOUND;
      }
    }
  }
  else
  {
    if (0 == legIndex)
      fareMarket->direction() = FMDirection::OUTBOUND;
    if (legIndex > 0 && fareMarket->destination()->loc() == sopDestination)
      fareMarket->direction() = FMDirection::INBOUND;
  }
}

void
initializeSopUsages(ShoppingTrx& trx,
                    FareMarket* fareMarket,
                    Itin* itin,
                    const std::vector<TravelSeg*>& fmts,
                    int startSeg,
                    int origSopId,
                    int bitIndex,
                    GovCxrGroupParameters& params)
{
  SoloJourneyUtil journeyUtil(trx, *itin);
  ApplicableSOP*& applicableSOPs(fareMarket->getApplicableSOPs());
  if (!applicableSOPs)
    trx.dataHandle().get(applicableSOPs);

  std::pair<ApplicableSOP::iterator, bool> res(
      applicableSOPs->insert(std::make_pair(params.govCarrierKey_, SOPUsages())));

  if (fallback::fallbackSOPUsagesResize(&trx)){
    if (res.second)
      res.first->second.resize(params.numSOPs_);
  }
  else{
    if (res.second &&
        params.numSOPs_>0 &&
        res.first->second.size() < ((std::vector<SOPUsage>::size_type)params.numSOPs_))
      res.first->second.resize(params.numSOPs_);
  }
  SOPUsages& sopUsages(res.first->second);
  SOPUsage& usage(sopUsages[bitIndex]);

  if (trx.isSumOfLocalsProcessingEnabled())
    usage.applicable_ = true;

  else
  {
    usage.applicable_ = false;
    for (FareMarket* fm : itin->fareMarket())
    {
      if (fm->governingCarrier() == fareMarket->governingCarrier())
      {
        // [IATA New Fare Selection] The sop bit is applicable only when the FM is the FM with gov
        // carrier.
        // For additional carriers(like the FM created for highest TPM), the bits are not
        // applicable.
        usage.applicable_ = true;
        break;
      }
    }
  }
  usage.itin_ = itin;
  usage.origSopId_ = origSopId;
  usage.startSegment_ = startSeg;
  usage.endSegment_ = startSeg + fmts.size() - 1;

  std::pair<FirstUsedSopMap::iterator, bool> res2(
      params.firstUsedSopMap_.insert(std::make_pair(fmts, bitIndex)));
  usage.flightsFirstProcessedIn_ = res2.first->second;

  journeyUtil.fillAvailability(usage);
}
void printDiagnostic922(
    ShoppingTrx& trx,
    const std::map<ItinIndex::Key,
    CarrierCode>& carrierMap)
{
  if (Diagnostic922 != trx.diagnostic().diagnosticType())
    return;
  Diag922Collector* diag = dynamic_cast<Diag922Collector*>(DCFactory::instance()->create(trx));
  if (diag == nullptr)
    return;
  const std::string& diagArg = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
  bool flights = false, isScheduleInfo = false;
  if ("FLIGHTS" == diagArg)
    flights = true;
  else if ("SCHEDULEINFO" == diagArg)
    isScheduleInfo = true;
  else if ("ALL" == diagArg)
  {
    flights = true;
    isScheduleInfo = true;
  }
  diag->enable(Diagnostic922);
  diag->printHeader();
  *diag << "\n====== 922 - Thru and Local Fare Markets for Carriers ";
  if (trx.isSumOfLocalsProcessingEnabled())
  {
    *diag << "(NGS Path)";
  }
  else
  {
    *diag << "(V2 Path)";
  }
  *diag << "=======\n\n";

  if (flights)
  {
    CabinType cabinType;
    *diag << "CABIN DEFINITION" << std::endl;
    for (int i = 1; i < 9; ++i)
    {
      cabinType.setClass('0' + i);
      if (!cabinType.isInvalidClass())
      {
        *diag << static_cast<char>('0' + i) << " - " << cabinType.printName() << std::endl;
      }
    }
    *diag << std::endl;
    *diag << "SOP - SOP POSITION IN BITMAP\n"
          << "ORIG SOP - SOP ID IN THE REQUEST\n"
          << "START - START SEGMENT IN SOP FOR THIS FARE MARKET\n"
          << "END - END SEGMENT IN SOP FOR THIS FARE MARKET\n"
          << "FLIGHTS FIRST PROCESSED IN SOP - SOP POSITION WHERE FLIGHTS FIRST OCCURED\n\n";
  }
  *diag << "====== Fare Markets =======\n\n";
  diag->setf(std::ios::left, std::ios::adjustfield);
  diag->printFareMarketHeader(trx);
  int legId(0);
  int totalFareMarkets(0);
  for (ShoppingTrx::Leg& curLeg : trx.legs())
  {
    *diag << "\n====== Leg " << ++legId << " ======\n";

    if (!curLeg.stopOverLegFlag())
    {
      for (ItinIndex::ItinMatrix::value_type& pr : curLeg.carrierIndex().root())
      {
        const Itin* firstItin(getFirstItin(pr.second));
        if (firstItin != nullptr)
        {
          CarrierCode govCxr = getGovCarrier(firstItin);
          if (govCxr.empty())
          {
            continue;
          }
          *diag << "\n------ Governing Carrier " << govCxr << " ------\n\n";
          for (const FareMarket* fm : firstItin->fareMarket())
          {
            diag->printFareMarket(trx, *fm);
            if (isScheduleInfo && trx.isSumOfLocalsProcessingEnabled())
              trx.diversity().printScheduleInfo(diag, fm, govCxr);
            *diag << "applicable SOPs\n";
            const ApplicableSOP* sopmap(fm->getApplicableSOPs());
            if (sopmap != nullptr)
            {
              for (const ApplicableSOP::value_type& pr : *sopmap)
              {
                std::map<ItinIndex::Key, CarrierCode>::const_iterator it(carrierMap.find(pr.first));
                if (it != carrierMap.end())
                {
                  *diag << it->second;
                }
                for (const SOPUsage& usage : pr.second)
                {
                  if (usage.applicable_ && !usage.itin_->isDummy())
                  {
                    *diag << ' ' << usage.origSopId_;
                  }
                }
                *diag << '\n';
                if (flights)
                {
                  *diag << "SCHEDULES\n";
                  int idx(0);
                  for (const SOPUsage& usage : pr.second)
                  {
                    const Itin* itin(usage.itin_);
                    if (usage.applicable_ && !itin->isDummy())
                    {
                      diag->printSchedules(trx, idx, *itin, usage);
                    }
                    ++idx;
                  }
                }
              }
            }
            *diag << "===========\n";
            ++totalFareMarkets;
          }
        }
      }
    }
  }

  *diag << "#total fare markets " << totalFareMarkets << '\n';
  diag->flushMsg();
}

FareMarket*
buildFareMkt(ShoppingTrx& trx,
             const Itin* itin,
             const std::vector<TravelSeg*>& travelSegs,
             int legIdx,
             const GovCxrGroupParameters& params)
{
  FareMarket* fareMarket =
      &trx.dataHandle().safe_create<FareMarket>(travelSegs,
                                                itin->travelDate(),
                                                travelSegs.front()->origin(),
                                                travelSegs.back()->destination(),
                                                travelSegs.front()->boardMultiCity(),
                                                travelSegs.back()->offMultiCity(),
                                                legIdx - 1);

  Boundary tvlBoundary(TravelSegAnalysis::selectTravelBoundary(travelSegs));
  ItinUtil::setGeoTravelType(tvlBoundary, *fareMarket);

  const bool isThruFM = (itin->travelSeg().size() == travelSegs.size());
  fareMarket->setFmTypeSol(isThruFM ? FareMarket::SOL_FM_THRU : FareMarket::SOL_FM_LOCAL);
  // Set the fare market break indicator
  if (LIKELY(FMDirection::UNKNOWN == fareMarket->direction()))
  {
    itinanalyzerutils::determineFMDirection(fareMarket,
                                            itin->travelSeg().back()->destination()->loc());
  }

  // Select the governing carrier
  {
  GoverningCarrier govCxr(&trx);
  govCxr.process(*fareMarket);
  }

  resolveGovCxrOverride(trx, fareMarket);

  if (isThruFM)
    fareMarket->governingCarrier() = params.govCarrier_;

  // setBoardOff points (corectly resolves multicities)
  FareMarketUtil::setMultiCities(*fareMarket, itin->travelDate());
  fareMarket->boardMultiCity() =
      FareMarketUtil::getBoardMultiCity(*fareMarket, *fareMarket->travelSeg().front());
  fareMarket->offMultiCity() =
      FareMarketUtil::getOffMultiCity(*fareMarket, *fareMarket->travelSeg().back());

  return fareMarket;
}

namespace
{
  bool returnsToOriginCity(const std::vector<TravelSeg*>& leg, DataHandle& dataHandle)
  {
    // TravelSeg cannot return to its own point of origin.
    if (leg.size() < 2)
      return false;

    if (LocUtil::isSameCity(leg.front()->origin()->loc(),
                            leg.back()->destination()->loc(), dataHandle))
    {
      return true;
    }

    return false;
  }

  int firstSegmentAfterTurnaroundPoint(const std::vector<TravelSeg*>& leg)
  {
    uint32_t maxMiles = 0;
    for (size_t segId = 0; segId < leg.size(); ++segId)
    {
      uint32_t miles = TseUtil::greatCircleMiles(*(leg.front()->origin()),
                                                 *(leg[segId]->destination()));
      if (miles >= maxMiles)
      {
        maxMiles = miles;
      }
      else
      {
        return segId;
      }
    }
    return -1;
   }

   void assignLegIds(std::vector<std::vector<TravelSeg*> >& legVector)
   {
     for (size_t legId = 0; legId < legVector.size(); ++legId)
       for (auto segment : legVector[legId])
         segment->legId() = legId;
   }

   std::string debugSplitMessage(std::pair<int,int> legRange, int segId)
   {
     std::ostringstream ss;
     ss << "SPLITTING LEG: "
        << legRange.first << "-" << legRange.second << " INTO "
        << legRange.first << "-" << (segId-1) << " AND " << segId << "-" << legRange.second
        << std::endl;
     return ss.str();
   }

   void summarizeDiagMessages(Diag892Collector* diag,
                              const std::vector<TravelSeg*>& travelSegs,
                              const std::vector<std::string>& splitterMessages,
                              const std::vector<std::string>& turnaroundPointMessages)
   {
     if(!diag)
       return;

     *diag << "***** START LEG ASSIGNMENT *****\n";
     *diag << "FOLLOWING SEGMENTS PROCESSED:\n";
     for (const auto seg : travelSegs)
      *diag << seg->origin()->loc() << "-" << seg->destination()->loc()
            << "(" << static_cast<char>(seg->segmentType()) << ") "
            << seg->departureDT() << " " << seg->arrivalDT()
            << " BRAND: " << seg->getBrandCode() << "\n";
     *diag << "SPLIT BASED ON STOPOVER AND CHANGE OF BRAND CRITERIA:\n";
     for (auto msg : splitterMessages)
       *diag << msg;
     if (!turnaroundPointMessages.empty())
     {
       *diag << "FOLLOWING LEGS WERE SPLIT FURTHER BASED ON TURNAROUND:\n";
       for (const auto msg : turnaroundPointMessages)
         *diag << msg;
     }
     *diag << "FINAL LEGID ASSIGNED TO SEGMENTS:\n";
     diag->printSegmentsLegIdInfo(travelSegs);
     *diag << "****** END LEG ASSIGNMENT ******\n";
     diag->flushMsg();
   }

} // End unnamed namespace

void
assignLegsToSegments(std::vector<TravelSeg*>& travelSegs,
                     DataHandle& dataHandle,
                     Diag892Collector* diag892)
{
  // Predicated based on which legs are to be computed: 12h stopover :
  auto stopoverCheck = [](TravelSeg* first, TravelSeg* second) -> bool
  { return (abs(DateTime::diffTime(first->arrivalDT(), second->departureDT())) > (60*60*12) ); };
  // change of brand
  auto changeOfBrandCheck = [](TravelSeg* first, TravelSeg* second) -> bool
  { return (first->getBrandCode() != second->getBrandCode()); };

  VectorIntoRangeSplitter<TravelSeg*> splitter =
                   VectorIntoRangeSplitter<TravelSeg*>(travelSegs, (diag892 != 0));
  splitter.addPredicate(stopoverCheck);
  splitter.addPredicate(changeOfBrandCheck);

  std::vector<std::vector<TravelSeg*> > legVector; // segments for each leg in a separate entry
  std::vector<std::string> splitByTurnaroundMessages;
  while(splitter.hasMore())
  {
    // getting a potential new leg from splitter
    const auto newRange = splitter.getNextRange();
    std::vector<TravelSeg*> leg = std::vector<TravelSeg*>(travelSegs.begin() + newRange.first,
                                                          travelSegs.begin() + newRange.second+1);
    // Checking if origin and destination are different. For round trips returning at the same data
    // stopover condition will not be triggered and we would have O&D with the same city.
    if (!returnsToOriginCity(leg, dataHandle))
    {
      legVector.push_back(leg);
    }
    else
    {
      // we need to split such leg. Turnaround point will be used for that purpose
      int turnAroundSegId = firstSegmentAfterTurnaroundPoint(leg);
      TSE_ASSERT(turnAroundSegId >= 0);

      if (diag892)
        splitByTurnaroundMessages.push_back(debugSplitMessage(newRange, turnAroundSegId));

      legVector.push_back(std::vector<TravelSeg*>(leg.begin(), leg.begin() + turnAroundSegId));
      legVector.push_back(std::vector<TravelSeg*>(leg.begin() + turnAroundSegId, leg.end()));
    }
  }
  // assigning sequential legId to each travelSeg subvector identified
  assignLegIds(legVector);

  if (diag892)
    summarizeDiagMessages(diag892, travelSegs,
                          splitter.getDebugMessages(),
                          splitByTurnaroundMessages);

} // end assignLegsToSegments function

void
setSopWithHighestTPM(ShoppingTrx& trx, int legIdx, int origSopId)
{
  if ((size_t)legIdx>=trx.legs().size())
    return;

  std::vector<ShoppingTrx::SchedulingOption>& sops = trx.legs()[legIdx].sop();
  std::vector<ShoppingTrx::SchedulingOption>::iterator itr = sops.begin();
  std::vector<ShoppingTrx::SchedulingOption>::iterator itrE = sops.end();

  for (; itr!=itrE; itr++)
  {
    if (itr->originalSopId() == (uint32_t)origSopId)
      itr->isHighTPM() = true;
  }
}

void
setGovCxrForSubIata21(PricingTrx& trx,
                      FareMarket& fm,
                      GoverningCarrier& govCxr,
                      int percent)
{
  if (fallback::fallbackGovCxrForSubIata21(&trx))
    return;

  // must be IS transaction and within Europe
  if ((trx.getTrxType() == PricingTrx::IS_TRX) &&
      (fm.travelBoundary().isSet(FMTravelBoundary::TravelWithinSubIATA21) ||
      (fm.travelBoundary().isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA) &&
       !fallback::highestGovCxrCheckInForeignDomFareMkts(&trx))))
  {
    CarrierCode highestGC = govCxr.getHighestTPMCarrier(fm.travelSeg(),
                                                        fm.direction(),
                                                        fm.primarySector());

    bool foreignDomesticGCIndicator = false;
    //If highestGC.empty(), the travel segments are foreign domestic
    if (highestGC.empty() && !fallback::highestGovCxrCheckInForeignDomFareMkts(&trx))
    {
      highestGC = govCxr.getForeignDomHighestTPMCarrier(fm.travelSeg(),
                                                        fm.direction(), fm.primarySector());
      foreignDomesticGCIndicator = true;
    }

    if (!fm.governingCarrier().empty()&&
         fm.governingCarrier() != highestGC)
    {
      uint32_t currGovCxrMileage = 0;
      uint32_t highestMileage = 0;

      if (!fallback::highestGovCxrCheckInForeignDomFareMkts(&trx) && foreignDomesticGCIndicator)
      {
         currGovCxrMileage = govCxr.getForeignDomHighestTPMByCarrier(fm.governingCarrier(),
                                                                           fm.travelSeg());
         highestMileage = govCxr.getForeignDomHighestTPMByCarrier(highestGC,
                                                                           fm.travelSeg());
      }
      else
      {
        currGovCxrMileage = govCxr.getHighestTPMByCarrier(fm.governingCarrier(),
                                                                 fm.travelSeg());
        highestMileage = govCxr.getHighestTPMByCarrier(highestGC,
                                                              fm.travelSeg());
      }

      int diff = (currGovCxrMileage)?(int)((highestMileage-currGovCxrMileage)/currGovCxrMileage)*100:0;
      if (diff >= percent)
      {
        fm.setDualGoverningFlag(true);
        fm.governingCarrier() = highestGC;
      }
    }
  }
}

void
removeRedundantFareMarket(MultiPaxFCMapping& multiPaxFCMapping)
{
  auto lessFmPredicate = [](const FareMarket* fmFirst, const FareMarket* fmSecond)
  { return fmFirst->travelSeg() < fmSecond->travelSeg(); };

  FlatSet<FareMarket*, decltype(lessFmPredicate)> uniqueFareMarketSet(lessFmPredicate);
  for (auto& mapElem : multiPaxFCMapping)
  {
    for (FareCompInfo* fareComponent : mapElem.second)
    {
      auto insertRetVal = uniqueFareMarketSet.insert(fareComponent->fareMarket());
      fareComponent->fareMarket() = *(insertRetVal.first);
    }
  }
}

} // namespace itinanalyzerutils
} // namespace tse
