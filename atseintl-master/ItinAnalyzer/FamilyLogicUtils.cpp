///-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "ItinAnalyzer/FamilyLogicUtils.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SimilarItinSegmentsBuilder.h"
#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/SimilarItinData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag982Collector.h"
#include "ItinAnalyzer/FamilyLogicSplitter.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "ItinAnalyzer/JourneyPrepHelper.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(fallbackEnableFamilyLogicOriginAvailabilityInBfa)

static Logger
logger("atseintl.ItinAnalyzer.FamilyLogicUtils");
uint16_t FamilyLogicUtils::_requestedNumberOfSeats = 1;
static const uint16_t NUMBER_OF_CLASSES_FOR_AVAIL_COMPARE = 9;

void
FamilyLogicUtils::setFareFamiliesIds(const PricingTrx& trx)
{
  for (Itin* itin : trx.itin())
  {
    itin->markAsHeadOfFamily();
    const int familyNumber = itin->itinNum();
    itin->setItinFamily(familyNumber);
    for (const SimilarItinData& similarItinData : itin->getSimilarItins())
    {
      similarItinData.itin->markAsSimilarItin();
      similarItinData.itin->setItinFamily(familyNumber);
    }
  }
}

namespace
{
bool
isAnArunk(const TravelSeg* tvlSeg)
{
  return tvlSeg->segmentType() == Arunk;
}
}

const Cabin*
FamilyLogicUtils::getCabin(PricingTrx& trx, const Itin* itin, ClassOfService* classOfService)
{
  classOfService->bookingCode() = "Y";
  if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    RBDByCabinUtil rbdCabin(trx, FAMILY_LOGIC);
    return rbdCabin.getCabinByRBD(ANY_CARRIER, classOfService->bookingCode(), itin->travelDate());
  }
  else
  {
    return trx.dataHandle().getCabin(
            ANY_CARRIER, classOfService->bookingCode(), itin->travelDate());
  }
  return nullptr;
}

void
FamilyLogicUtils::setInfoForSimilarItins(PricingTrx& trx, ItinAnalyzerService* iis)
{
  ClassOfService* classOfService = nullptr;
  if (!trx.itin().empty())
  {
    trx.dataHandle().get(classOfService);
    const Cabin* cabin = getCabin(trx, trx.itin().front(), classOfService);

    if (cabin != nullptr)
    {
      classOfService->cabin() = cabin->cabin();
    }
  }
  for (Itin* itin : trx.itin())
  {
    std::vector<TravelSeg*>::iterator tvl = itin->travelSeg().begin();
    std::vector<TravelSeg*>::iterator tvlEnd = itin->travelSeg().end();
    if (!(itin->origBooking().empty()) && (itin->origBooking().size() != itin->travelSeg().size()))
    {
      // set up origbooking code for arunk seg for mother itin
      for (size_t index = 0; tvl != tvlEnd; ++tvl, ++index)
      {
        if ((*tvl)->segmentType() == Arunk && index > 0)
        {
          if (index >= itin->origBooking().size())
          {
            throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "Missing booking code");
          }
          itin->origBooking().insert(itin->origBooking().begin() + index, classOfService);
        }
      }
    }
    for (const SimilarItinData& similarItinData : itin->getSimilarItins())
    {
      Itin& smItin = *similarItinData.itin;
      smItin.setTravelDate(itin->travelDate());
      smItin.useInternationalRounding() = itin->useInternationalRounding();

      smItin.travelSeg().erase(
          std::remove_if(smItin.travelSeg().begin(), smItin.travelSeg().end(), isAnArunk),
          smItin.travelSeg().end());
      if (itin->travelSeg().size() != smItin.travelSeg().size())
      {
        tvl = itin->travelSeg().begin();
        for (size_t index = 0; tvl != tvlEnd; ++tvl, ++index)
        {
          if ((*tvl)->segmentType() == Arunk && smItin.travelSeg().size() > index && index > 0 &&
              smItin.travelSeg()[index]->origAirport() == (*tvl)->destAirport() &&
              smItin.travelSeg()[index - 1]->destAirport() == (*tvl)->origAirport())
          {
            smItin.travelSeg().insert(smItin.travelSeg().begin() + index, *tvl);
            if (!(smItin.origBooking().empty()))
            {
              smItin.origBooking().insert(smItin.origBooking().begin() + index, classOfService);
            }
          }
        }
      }

      iis->setGeoTravelTypeAndTktCxr(smItin);
      iis->setTripCharacteristics(&smItin);
    }
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX && !trx.isAltDates())
  {
    iis->setPnrSegmentCollocation(&trx, trx.itin());
  }
  else
  {
    RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(&trx);
    if (rexExcTrx)
    {
      iis->setPnrSegmentCollocation(&trx, rexExcTrx->newItin());
    }
  }
}

// Go through each family and check it
void
FamilyLogicUtils::splitItinFamilies(PricingTrx& trx,
                                    bool splitItinsForDomesticOnly,
                                    uint16_t requestedNumberOfSeats)
{
  FamilyLogicSplitter splitter(trx, requestedNumberOfSeats);
  splitter.splitItinFamilies(splitItinsForDomesticOnly);
}

void
FamilyLogicUtils::sumUpChildAvailabilityForMotherItins(PricingTrx& trx,
                                                       uint16_t requestedNumberOfSeats)
{
  std::stringstream diagStr;
  Diag982Collector* diag = nullptr;
  bool diag982 = diag982DDAllActive(trx);

  if (UNLIKELY(diag982))
  {
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag982Collector*>(factory->create(trx));
    diag->enable(Diagnostic982);
    diagStr << "\n*** SUMMING UP AVL OF WHOLE FAMILY FOR BC VALIDATIONS ***\n";
  }

  for (Itin* itin : trx.itin())
  {
    if (UNLIKELY(diag982))
      diagStr << "\n*** PROCESSING AVAILIBILITY FOR ITIN " << itin->itinNum() << " ***\n\n";

    if (itin->getSimilarItins().empty())
    {
      if (UNLIKELY(diag982))
        diagStr << "NO CHILDREN FOUND\n";
      continue;
    }

    if (UNLIKELY(diag982))
    {
      diagStr << "AVAILABILITY BEFORE SUMUP:\n";
      std::vector<std::string> motherInfo = diag->getMotherAvailabilityAsString(trx, *itin);
      for (std::string line: motherInfo)
        diagStr << line << "\n";
    }

    for (const FareMarket* fm: itin->fareMarket())
    {
      const PricingTrx::ClassOfServiceKey& key = fm->travelSeg();
      if (key.empty())
        continue;
      std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
      std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();
      for (size_t index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
      {
        TravelSeg* seg = *tvlI;
        if (UNLIKELY(diag982))
          diagStr << "\nPROCESSING SEGMENT " << seg->origin()->loc() << "-" << seg->destination()->loc()
                  << " FROM FARE MARKET " << fm->toString() << "\n";
        std::map<BookingCode, ClassOfService*> missingCodesForSegment;
        // find out which booking codes are not available for mother
        for (ClassOfService* cos : ShoppingUtil::getClassOfService(trx, key)[index])
        {
          if (cos->numSeats() < requestedNumberOfSeats)
            missingCodesForSegment[cos->bookingCode()] = cos;
        }
        if (UNLIKELY(diag982))
        {
          if (missingCodesForSegment.empty())
            diagStr << " NOTHING TO DO " << std::endl;
          else
          {
            diagStr << "AVL MISSING FOR BK: ";
            for (const auto& bk : missingCodesForSegment)
              diagStr << bk.first << " ";
            diagStr << "\nLOOKING FOR BETTER AVAILABILTY IN SIMILAR ITINS" << std::endl;
          }
        }
        if (missingCodesForSegment.empty())
          continue;

        // try to find better availability for this segment in children
        for (const SimilarItinData& similarItinData : itin->getSimilarItins())
        {
          if (UNLIKELY(diag982))
            diagStr << "SUBITIN " << similarItinData.itin->itinNum() << ": " << std::endl;
          const PricingTrx::ClassOfServiceKey& child_key =
            similarItinData.fareMarketData.at(fm).travelSegments;
          if (child_key.empty())
            continue;
          if (child_key.size() != key.size())
          {
            if (UNLIKELY(diag982))
              diagStr << "!!! WRONG CHILD KEY SIZE " << child_key.size() << " vs " << key.size() << "\n";
            continue;
          }

          for (ClassOfService* cos : ShoppingUtil::getClassOfService(trx, child_key)[index])
          {
            if (missingCodesForSegment.count(cos->bookingCode()))
            {
              // this is booking code we need better availability for
              if (cos->numSeats() >= requestedNumberOfSeats)
              {
                // insert new, valid value into mother itin (for use in fare booking code validator)
                ClassOfService* originalCosPtr = missingCodesForSegment.at(cos->bookingCode());
                // remember original and new availability for this booking code
                itin->getFamilyAggregatedAvlMap()[originalCosPtr] = originalCosPtr->numSeats();
                // change availability in mother itin
                originalCosPtr->numSeats() = cos->numSeats();
                // remove this booking code from the list of items that still need amendments
                missingCodesForSegment.erase(cos->bookingCode());
                if (UNLIKELY(diag982))
                  diagStr << "FOUND BETTER AVAILABILITY FOR BK " << (cos->bookingCode())
                          << " = " << (cos->numSeats()) << std::endl;
              }
            }
            if (missingCodesForSegment.empty())
              break;
          }
          if (missingCodesForSegment.empty())
            break;
        }
      }
    }

    if (UNLIKELY(diag982))
    {
      diagStr << "\nAVAILABILITY AFTER SUMUP:\n";
      std::vector<std::string> motherInfo = diag->getMotherAvailabilityAsString(trx, *itin);
      for (std::string line: motherInfo)
        diagStr << line << "\n";
      diagStr << "\n";
    }
  }

  if (UNLIKELY(diag982))
  {
    diagStr << "\n*** END OF AVL SUM UP ***\n";
    *diag << diagStr.str() << "\n";
    diag->flushMsg();
  }
}

void
FamilyLogicUtils::printFamily(Itin* itin, std::stringstream& diag)
{
  diag << "MOTHER ITIN: " << itin->itinNum() << "\n";
  diag << "CHILDREN: ";

  for (const SimilarItinData& similarItinData : itin->getSimilarItins())
    diag << similarItinData.itin->itinNum() << " ";

  diag << "\n";
}

bool
FamilyLogicUtils::diag982DDAllActive(PricingTrx& trx)
{
  return (trx.diagnostic().diagnosticType() == Diagnostic982) &&
         (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL");
}

int
FamilyLogicUtils::segmentOrderWithoutArunk(const Itin* itin, const TravelSeg* segment)
{
  std::vector<TravelSeg*>::const_iterator itor =
      std::find(itin->travelSeg().begin(), itin->travelSeg().end(), segment);

  int segmentOrder = -1;

  if (UNLIKELY(itor == itin->travelSeg().end()))
  {
    itor = itin->travelSeg().begin();

    for (; itor != itin->travelSeg().end(); itor++)
    {
      if (((*itor)->boardMultiCity() == segment->boardMultiCity()) &&
          ((*itor)->offMultiCity() == segment->offMultiCity()) &&
          ((*itor)->departureDT().get64BitRepDateOnly() ==
           segment->departureDT().get64BitRepDateOnly()))

      {
        segmentOrder = (itor - itin->travelSeg().begin()) + 1;
        break;
      }
    }
  }
  else
    segmentOrder = (itor - itin->travelSeg().begin()) + 1;

  if (UNLIKELY(segmentOrder < 0))
    return segmentOrder;

  const int arunks = std::count_if(itin->travelSeg().begin(), itor + 1, isAnArunk);

  return (segmentOrder - arunks);
}

namespace
{
void mergeClassOfService(const ClassOfService& cos, std::vector<ClassOfService>& classes)
{
  auto it = std::find_if(classes.begin(), classes.end(), [&cos](const ClassOfService& c)
                 { return cos.cabin() == c.cabin() && cos.bookingCode() == c.bookingCode(); });
  if (it == classes.end())
    classes.push_back(cos);
  else
    it->numSeats() = std::max(it->numSeats(), cos.numSeats());
}

void calculateClassOfServiceForBfa(const PricingTrx& trx, FareMarketData& data)
{
  const size_t tsSize = data.travelSegments.size();
  for (size_t i = 0; i < tsSize; ++i)
  {
    std::vector<ClassOfService>& classOfServiceVec = data.classOfService[i];

    const auto& thruAvailability = ShoppingUtil::getClassOfService(trx, data.travelSegments)[i];
    for (const ClassOfService* cos : thruAvailability)
    {
      mergeClassOfService(*cos, classOfServiceVec);
    }
  }
}

void calculateClassOfService(const PricingTrx& trx, FareMarketData& data)
{
  const auto& tsToAvailabilityMap = trx.maxThruFareAvailabilityMap();
  const size_t tsSize = data.travelSegments.size();
  for (size_t i = 0; i < tsSize; ++i)
  {
    TravelSeg& segment = *data.travelSegments[i];
    std::vector<ClassOfService>& classOfServiceVec = data.classOfService[i];

    for (const ClassOfService* cos : segment.classOfService())
      mergeClassOfService(*cos, classOfServiceVec);

    const auto& thruAvailability = tsToAvailabilityMap.find(&segment);
    if (thruAvailability != tsToAvailabilityMap.end())
    {
      for (const ClassOfService* cos : thruAvailability->second)
        mergeClassOfService(*cos, classOfServiceVec);
    }
  }
}
}

void
FamilyLogicUtils::fillSimilarItinData(PricingTrx& trx, Itin& itin, ItinAnalyzerService* ias)
{
  std::vector<SimilarItinData> result;

  struct GovCxrSegments
  {
    CarrierCode _govCxr;
    const std::vector<TravelSeg*>& _segments;
  };

  struct GovCxrSegmentsCompare
  {
    bool operator()(const FareMarket* l, const GovCxrSegments& r) const
    {
      const auto lGovCxr = l->governingCarrier();
      const auto rGovCxr = r._govCxr;
      if (lGovCxr != rGovCxr)
        return lGovCxr < rGovCxr;

      return l->travelSeg() < r._segments;
    }

    bool operator()(const FareMarket* l, const FareMarket* r) const
    {
      return (*this)(l, GovCxrSegments{r->governingCarrier(), r->travelSeg()});
    }
  };

  const bool isBRAllCos =
    !fallback::fallbackEnableFamilyLogicOriginAvailabilityInBfa(&trx) && trx.isBRAll();

  for (const SimilarItinData& itinData : itin.getSimilarItins())
  {
    Itin& similarItin = *itinData.itin;
    similaritin::SegmentsBuilder builder(trx, itin, similarItin);
    SimilarItinData data(&similarItin);

    ias->buildFareMarketsForSimilarItin(trx, similarItin);

    std::vector<FareMarket*> fareMarkets = similarItin.fareMarket();
    std::sort(fareMarkets.begin(), fareMarkets.end(), GovCxrSegmentsCompare());

    for (FareMarket* fareMarket : itin.fareMarket())
    {
      auto segments = builder.constructByOriginAndDestination(fareMarket->travelSeg());
      if (segments.empty())
        continue;

      auto& fmData = data.fareMarketData[fareMarket];
      fmData.initialize(std::move(segments));
      if (isBRAllCos)
        calculateClassOfServiceForBfa(trx, fmData);
      else
        calculateClassOfService(trx, fmData);

      const auto it = std::lower_bound(
          fareMarkets.begin(),
          fareMarkets.end(),
          GovCxrSegments{fareMarket->governingCarrier(), fmData.travelSegments},
          GovCxrSegmentsCompare());
      if (it != fareMarkets.end() &&
          (*it)->governingCarrier() == fareMarket->governingCarrier() &&
          (*it)->travelSeg() == fmData.travelSegments)
        fmData.fareMarket = *it;
    }
    // if we are not able to adjust segments in any FareMarket,
    // we will not be able to price this itin
    if (!data.fareMarketData.empty())
      result.push_back(std::move(data));
  }
  itin.swapSimilarItins(result);
}
}
