// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/FarePathUtils.h"

#include "DataModel/DifferentialData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Fares/AvailabilityChecker.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/MergedFareMarket.h"

namespace tse
{
namespace farepathutils
{
static Logger
logger("atseintl.Pricing.FarePathUtils");

bool
failedFareExistsInPU(const PaxTypeFare* const ptf, const PricingUnit& prU)
{
  for (const FareUsage* const fareUsage : prU.fareUsage())
  {
    if (fareUsage->paxTypeFare() == ptf)
      return true;
  }
  return false;
}

void
printFarePathToDiag(PricingTrx& trx, DiagCollector& diag, FarePath& farePath, PUPath* puPath)
{
  diag << farePath;

  if (trx.getRequest()->isBrandedFaresRequest())
  {
    TSE_ASSERT(puPath != nullptr);
    diag.printBrand(puPath->puPath().front()->fareMarket().front()->brandCode());
  }
}

void
copyPUPathEOEInfo(FarePath& fPath, const PUPath* puPath)
{
  uint16_t idx = 0;

  for (auto oeoPuAvalable : puPath->eoePUAvailable())
  {
    fPath.pricingUnit()[idx]->noPUToEOE() = !oeoPuAvalable;
    idx++;
  }
}

bool
putDiagMsg(PricingTrx& trx, FPPQItem& fppqItem, DiagCollector& diag, uint32_t fpCombTried)
{
  if (LIKELY(trx.diagnostic().diagnosticType() == DiagnosticNone))
  {
    return false;
  }

  const bool showItin = DiagnosticUtil::showItinInMipDiag(trx,
                                                          fppqItem.farePath()->itin()->itinNum());

  diag.enable(fppqItem.puPath(), Diagnostic609, Diagnostic610, Diagnostic620);
  if (diag.isActive())
  {
    if(showItin)
    {
      addMainTripSideTripLink(fppqItem);
    }
  }
  else
  {
    return false;
  }

  if (DiagnosticUtil::filter(trx, *fppqItem.farePath()))
  {
    if(showItin)
    {
      diag.printLine();
    }
    return true;
  }

  diag.enableFilter(Diagnostic620, 1, 0); // counter is not needed if 620MX
  if (diag.isActive())
  {
    if(showItin)
    {
      diag.setf(std::ios::right, std::ios::adjustfield);
      diag.printLine();
      // lint -e{530}
      const int width = diag.width(6);
      diag << std::setfill('0');
      diag << fpCombTried;
      diag << std::setfill(' ');
      diag << " - ";
      diag.width(width);
      diag.setf(std::ios::left, std::ios::adjustfield);
    }
  }

  diag.enable(fppqItem.puPath(), Diagnostic609, Diagnostic610, Diagnostic620);
  diag.enableFilter(Diagnostic620, 1, fpCombTried);

  if(showItin)
  {
    printFarePathToDiag(trx, diag, *fppqItem.farePath(), fppqItem.puPath());
  }

  if (!fppqItem.isValid())
  {
    if(showItin)
    {
      diag << "FAILED: BY PREV VALIDATION RESULT\n";
    }
  }

  return true;
}

void
addMainTripSideTripLink(FarePath& fpath, PUPath* puPath)
{
  if (fpath.itin()->hasSideTrip() == false)
  {
    return;
  }

  if (puPath->mainTripSideTripLink().empty())
  {
    return;
  }

  const PUPath::MainTripSideTripLink& aMainTripSideTripLink = puPath->mainTripSideTripLink();
  PUPath::MainTripSideTripLink::const_iterator puIdxIt = aMainTripSideTripLink.begin();
  const PUPath::MainTripSideTripLink::const_iterator puIdxItEnd = aMainTripSideTripLink.end();

  for (; puIdxIt != puIdxItEnd; ++puIdxIt)
  {
    // LOG4CXX_INFO(logger, "MainTrip PU Idx: "<< (int) puIdxIt->first)
    PricingUnit& mtPU = *fpath.pricingUnit()[puIdxIt->first];
    mtPU.hasSideTrip() = true;
    mtPU.sideTripPUs().clear(); // we delay the copying of PrU, It could be the Master PrU

    PUPath::FUToPULinkMap::const_iterator it = puIdxIt->second.begin();
    const PUPath::FUToPULinkMap::const_iterator itEnd = puIdxIt->second.end();
    for (; it != itEnd; ++it)
    {
      // LOG4CXX_INFO(_logger, "FareUsage Idx: "<< (int) it->first)
      FareUsage& mtFU = *mtPU.fareUsage()[it->first];
      mtFU.hasSideTrip() = true;
      mtFU.sideTripPUs().clear();

      std::vector<std::vector<uint8_t>>::const_iterator stIt = it->second.begin();
      const std::vector<std::vector<uint8_t>>::const_iterator stItEnd = it->second.end();
      for (uint8_t stNumber = 1; stIt != stItEnd; ++stIt, ++stNumber)
      {
        std::vector<uint8_t>::const_iterator stPuIdxIt = stIt->begin();
        const std::vector<uint8_t>::const_iterator stPuIdxItEnd = stIt->end();
        for (; stPuIdxIt != stPuIdxItEnd; ++stPuIdxIt)
        {
          // LOG4CXX_INFO(_logger, "PushBack ST PrU at idx="<< (int) *stPuIdxIt
          //             <<" to MainTrip PU and FU");
          if (LIKELY(fpath.pricingUnit().size() > *stPuIdxIt))
          {
            PricingUnit* stPU = fpath.pricingUnit()[*stPuIdxIt];
            stPU->isSideTripPU() = true;
            stPU->sideTripNumber() = stNumber;
            mtFU.sideTripPUs().push_back(stPU);
            mtPU.sideTripPUs().push_back(stPU);

            // debugOnlyDisplayPricingUnit(*stPU);
          }
          else
          {
            LOG4CXX_ERROR(logger, "ST PrU INDEX: " << (int)*stPuIdxIt << " is not correct")
          }
        }
      }
    }
  }
}

void
clearMainTripSideTripLink(FarePath& fpath)
{
  if (fpath.itin()->hasSideTrip() == false)
    return;

  // Clear previous flags because:
  // A Master PrU can be used by differe FarePath and
  // therefore, might have some field set, we clear it only when
  // needed - delayed evaluation
  //
  for (PricingUnit* pu : fpath.pricingUnit())
  {
    pu->isSideTripPU() = false;

    if (!pu->hasSideTrip())
      continue;

    pu->hasSideTrip() = false;
    pu->sideTripPUs().clear();

    for (FareUsage* fu : pu->fareUsage())
    {
      fu->hasSideTrip() = false;
      fu->sideTripPUs().clear();
    }
  }
}

void
findPUFactoryIdx(const FarePath& fpath,
                 const FareUsage* const failedSourceFareUsage,
                 const FareUsage* const failedTargetFareUsage,
                 int32_t& puFactIdx1,
                 int32_t& puFactIdx2)
{
  uint32_t puFactIdx = 0;
  for (PricingUnit* pricingUnit : fpath.pricingUnit())
  {
    if (UNLIKELY(pricingUnit->isCmdPricing()))
    {
      // no tuning of EOE-Combination for cmd pricing
      puFactIdx1 = -1;
      puFactIdx2 = -1;
      break;
    }
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (failedSourceFareUsage->paxTypeFare() == fareUsage->paxTypeFare())
      {
        puFactIdx1 = puFactIdx;
        break;
      }
      else if (failedTargetFareUsage->paxTypeFare() == fareUsage->paxTypeFare())
      {
        puFactIdx2 = puFactIdx;
        break;
      }
    }
    if (puFactIdx2 >= 0)
      break;
    puFactIdx++;
  }
}

bool
journeyDiag(PricingTrx& trx, FarePath& fp, DiagCollector& diag)
{
  if (UNLIKELY(!(trx.diagnostic().isActive())))
    return false;
  if (LIKELY(diag.diagnosticType() != Diagnostic556))
    return false;

  DiagParamMapVecI endI = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecI beginI = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  // size_t len = 0;
  bool slashFC = false;
  std::vector<FareClassCode> requestedFcs;
  if (beginI != endI)
  {
    slashFC = true;
    requestedFcs.push_back(((*beginI).second.c_str()));
  }
  beginI = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  if (beginI != endI)
  {
    slashFC = true;
    requestedFcs.push_back(((*beginI).second.c_str()));
  }
  beginI = trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);
  if (beginI != endI)
  {
    slashFC = true;
    requestedFcs.push_back(((*beginI).second.c_str()));
  }
  beginI = trx.diagnostic().diagParamMap().find(Diagnostic::WPNC_SOLO_TEST);
  if (beginI != endI)
  {
    slashFC = true;
    requestedFcs.push_back(((*beginI).second.c_str()));
  }
  beginI = trx.diagnostic().diagParamMap().find(Diagnostic::ACTIVATE_JOURNEY);
  if (beginI != endI)
  {
    slashFC = true;
    requestedFcs.push_back(((*beginI).second.c_str()));
  }

  if (slashFC)
  {
    if (requestedFcs.empty())
      return false;
    std::vector<FareClassCode> fpfcs;
    bool fcFound = false;

    for (const auto pu : fp.pricingUnit())
    {
      for (const auto fu : pu->fareUsage())
      {
        fcFound = false;
        for (const auto fpfc : fpfcs)
        {
          if (fu->paxTypeFare()->fareClass() == fpfc)
          {
            fcFound = true;
            break;
          }
        }
        if (!fcFound)
          fpfcs.push_back(fu->paxTypeFare()->fareClass());
      }
    }

    if (requestedFcs.size() != fpfcs.size())
      return false;

    for (const auto& requestedFc : requestedFcs)
    {
      fcFound = false;
      for (const auto& fpfc : fpfcs)
      {
        if (requestedFc == fpfc)
        {
          fcFound = true;
          break;
        }
      }
      if (!fcFound)
        return false;
    } // for(uint16_t i = 0; i < requestedFcs.size(); i++)
  } // if(slashFC)

  diag.enable(Diagnostic556);
  diag << " \n*************** START JOURNEY DIAG ************************ \n";
  diag << "FARE PATH " << fp;
  diag << "------------------------------------------------------------ \n";

  if (trx.getOptions()->journeyActivatedForPricing())
    diag << "JOURNEY ACTIVATED: YES";
  else
    diag << "JOURNEY ACTIVATED: NO";

  if (trx.getOptions()->applyJourneyLogic())
    diag << "    JOURNEY APPLIED: YES \n";
  else
    diag << "    JOURNEY APPLIED: NO \n";

  return true;
}

FareUsage*
getFareUsage(FarePath& fpath, TravelSeg* tvlSeg, uint16_t& tvlSegIndex)
{
  uint16_t i = 0;
  for (PricingUnit* pricingUnit : fpath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      i = 0;
      for (const TravelSeg* const travelSeg : fareUsage->travelSeg())
      {
        if (tvlSeg == travelSeg)
        {
          tvlSegIndex = i;
          return fareUsage;
        }
        ++i;
      }
    }
  }
  return nullptr;
}

bool
journeyDiagShowStatus(FarePath& fp, DiagCollector& diag, bool beforeJourney)
{
  if (!(diag.isActive()))
    return false;
  if (diag.diagnosticType() != Diagnostic556)
    return false;
  std::vector<PricingUnit*>::const_iterator puI = fp.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puE = fp.pricingUnit().end();

  std::vector<FareUsage*>::const_iterator fuI;
  std::vector<FareUsage*>::const_iterator fuE;
  diag << " \n";

  if (beforeJourney)
  {
    diag << "BOOKING CODE STATUS BEFORE JOURNEY PROCESS: \n";
  }
  else
  {
    diag << "******************************************************* \n";
    diag << "BOOKING CODE STATUS AFTER JOURNEY PROCESS: \n";
  }

  for (; puI != puE; puI++)
  {
    PricingUnit& pu = **puI;
    fuI = pu.fareUsage().begin();
    fuE = pu.fareUsage().end();
    for (; fuI != fuE; fuI++)
    {
      FareUsage& fu = **fuI;
      diag << "FARE: " << fu.paxTypeFare()->fareClass()
           << "    FARE MARKET: " << fu.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
           << fu.paxTypeFare()->fareMarket()->offMultiCity() << " \n";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
        diag << "  FAIL:T";
      else
        diag << "  FAIL:F";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
        diag << "  PASS:T";
      else
        diag << "  PASS:F";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED))
        diag << "  MIXED:T";
      else
        diag << "  MIXED:F";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL))
        diag << "  PASS LOCAL AVAIL:T \n";
      else
        diag << "  PASS LOCAL AVAIL:F \n";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL))
        diag << "  PASS FLOW AVAIL:T";
      else
        diag << "  PASS FLOW AVAIL:F";

      if (fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
        diag << "  MIXED FLOW AVAIL:T \n";
      else
        diag << "  MIXED FLOW AVAIL:F \n";

      diag << "SEGMENT STATUS VECTOR: \n";
      std::vector<TravelSeg*>::const_iterator iterTvl =
          fu.paxTypeFare()->fareMarket()->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator iterTvlEnd =
          fu.paxTypeFare()->fareMarket()->travelSeg().end();
      ;

      for (uint16_t i = 0; ((iterTvl != iterTvlEnd) && i < fu.segmentStatus().size());
           iterTvl++, i++)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
        PaxTypeFare::SegmentStatus& segStat = fu.segmentStatus()[i];
        journeyDiagSegStat(segStat, diag, airSeg);
      }

      diag << "SEGMENT STATUS VECTOR2: \n";
      iterTvl = fu.paxTypeFare()->fareMarket()->travelSeg().begin();

      for (uint16_t i = 0; ((iterTvl != iterTvlEnd) && i < fu.segmentStatusRule2().size());
           iterTvl++, i++)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
        PaxTypeFare::SegmentStatus& segStat = fu.segmentStatusRule2()[i];
        journeyDiagSegStat(segStat, diag, airSeg);
      }
      diag << " \n";
    } // for (; fuI != fuE; fuI++)
  } // for (; puI != puE; puI++)
  if (beforeJourney)
  {
    diag << "******************************************************* \n";
    diag << "FLIGHTS PROCESSED FOR JOURNEY: \n";
  }
  return true;
}

void
journeyDiagSegStat(PaxTypeFare::SegmentStatus& segStat, DiagCollector& diag, AirSeg* airSeg)
{
  if (!(diag.isActive()))
    return;
  if (diag.diagnosticType() != Diagnostic556)
    return;
  if (airSeg == nullptr)
  {
    diag << " ARUNK \n";
    return;
  }
  diag << " " << airSeg->carrier() << std::setw(4) << airSeg->flightNumber()
       << airSeg->getBookingCode();
  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
    diag << " PASS:T";
  else
    diag << " PASS:F";

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
    diag << " FAIL:T";
  else
    diag << " FAIL:F";

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
    diag << " NOMATCH:T";
  else
    diag << " NOMATCH:F";

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
    diag << " AVL BRK:T";
  else
    diag << " AVL BRK:F";

  if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    diag << " REBOOK:T-";
  else
    diag << " REBOOK:F-";
  diag << segStat._bkgCodeReBook << " \n";
  return;
}

void
jDiag(DiagCollector& diag,
      std::vector<TravelSeg*>& segmentsProcessed,
      JourneyDiagMsgType diagMsg,
      bool journeyDiagOn)
{
  if (LIKELY(!(diag.isActive())))
    return;
  if (diag.diagnosticType() != Diagnostic556)
    return;
  if (!journeyDiagOn)
    return;
  if (segmentsProcessed.empty())
  {
    diag << "NO FLIGHTS PROCESSED FOR JOURNEY\n";
    return;
  }

  if (diagMsg == PASS_OTHER_SEGS || diagMsg == FAIL_OTHER_SEGS)
  {
    diag << " \n";
    diag << "ALL FLIGHTS AFTER PROCESS OTHER SEGS: \n";
  }
  jDiagDisplayFlights(diag, segmentsProcessed);

  switch (diagMsg)
  {
  case FAIL_USE_FLOW_AVAIL:
  {
    diag << "  JOURNEY FAIL: USE FLOW AVAIL FUNCTION FALSE";
    break;
  }
  case FAIL_FU_NOT_FOUND:
  {
    diag << "  JOURNEY FAIL:  FARE USAGE NOT FOUND";
    break;
  }
  case FAIL_BKS_FAIL_SET:
  {
    diag << "  JOURNEY FAIL:  BKS FAIL SET IN FARE USAGE";
    break;
  }
  case FAIL_NOT_PASS_LOCAL_MIX:
  {
    diag << "  JOURNEY FAIL:  BKS PASS LOCAL OR MIXED NOT SET IN FU";
    break;
  }
  case FAIL_START_FLOW:
  {
    diag << "  JOURNEY FAIL:  START FLOW FUNCTION FALSE";
    break;
  }
  case FAIL_FOUND_FLOW:
  {
    diag << "  JOURNEY FAIL:  USE FLOW AVAIL FUNCTION FALSE";
    break;
  }
  case PASS_JOURNEY:
  {
    diag << "  JOURNEY PASS";
    break;
  }
  case FAIL_NEED_SET_REBOOK:
  {
    diag << "  JOURNEY FAIL: NEED SET REBOOK FUNCTION FALSE";
    break;
  }

  case PASS_OTHER_SEGS:
  {
    diag << "  JOURNEY PASS: AFTER PROCESS OTHER SEGS";
    break;
  }

  case FAIL_OTHER_SEGS:
  {
    diag << "  JOURNEY FAIL: AFTER PROCESS OTHER SEGS";
    break;
  }

  default:
    break;
  }

  diag << "\n";

  return;
}

void
jDiagDisplayFlights(DiagCollector& diag, const std::vector<TravelSeg*>& segmentsProcessed)
{
  std::vector<TravelSeg*>::const_iterator iterTvl = segmentsProcessed.begin();
  std::vector<TravelSeg*>::const_iterator iterTvlEnd = segmentsProcessed.end();
  ;
  // diag << "FLIGHTS PROCESSED FOR JOURNEY: \n";
  AirSeg* airSeg = nullptr;
  for (; iterTvl != iterTvlEnd; iterTvl++)
  {
    airSeg = dynamic_cast<AirSeg*>(*iterTvl);
    if (airSeg == nullptr)
    {
      diag << "  ARUNK \n";
      continue;
    }
    diag << "  " << airSeg->carrier() << " " << std::setw(4) << airSeg->flightNumber()
         << airSeg->getBookingCode() << " " << std::setw(5)
         << airSeg->departureDT().dateToString(DDMMM, "") << " " << airSeg->origAirport()
         << airSeg->destAirport() << " " << std::setw(6)
         << airSeg->departureDT().timeToString(HHMM_AMPM, "") << " " << std::setw(6)
         << airSeg->arrivalDT().timeToString(HHMM_AMPM, "") << " \n";
  }
  return;
}

void
jDiagInfo(DiagCollector& diag,
          const FareMarket& fm,
          JourneyDiagMsgType diagMsg,
          const bool journeyDiagOn)
{
  if (LIKELY(!(diag.isActive())))
    return;
  if (diag.diagnosticType() != Diagnostic556)
    return;
  if (!journeyDiagOn)
    return;

  diag << "  " << fm.boardMultiCity() << "-" << fm.offMultiCity() << " ";

  switch (diagMsg)
  {
  case INFO_SAME_BC:
  {
    diag << "SAME BOOKING CODE FOUND";
    break;
  }
  case INFO_FLOW_AVAIL_FAIL:
  {
    diag << "USE FLOW AVAIL FAIL";
    break;
  }
  case INFO_FLOW_AVAIL_PASS:
  {
    diag << "USE FLOW AVAIL PASS";
    break;
  }
  case INFO_TRY_LOCAL_AVAIL:
  {
    diag << "DIFFERENT BOOKING CODES FOUND - TRYING LOCAL AVAIL";
    break;
  }
  case FAIL_JOURNEY_SHOPPING:
  {
    diag << "REAL AVAILABILITY CHECK FAILED - SHOPPING";
    break;
  }
  default:
    break;
  }

  diag << "\n";

  return;
}

void
setPriority(PricingTrx& trx, const PUPQItem& pupqItem, FPPQItem& fppqItem, Itin* itin)
{
  const PriorityStatus& sourceStatus = pupqItem.priorityStatus();

  PriorityStatus& status = fppqItem.mutablePriorityStatus();
  PriorityStatus& pausedStatus = fppqItem.mutablePausedPriorityStatus();

  if (sourceStatus.farePriority() > status.farePriority())
  {
    // regular priority= 1,
    // Cxr wants CxrFare but a FareUsage contains an Industry fare then
    // priority is lower (value is higher, 2)
    //

    status.setFarePriority(sourceStatus.farePriority());
    if (!pupqItem.paused())
    {
      pausedStatus.setFarePriority(sourceStatus.farePriority());
    }
  }

  if (sourceStatus.paxTypeFarePriority() > status.paxTypeFarePriority())
  {
    // Requested PaxType fare was not used,
    // default ADT fare was used instead
    // priority will be lower in PQ and will be in effect
    // for same price level

    status.setPaxTypeFarePriority(sourceStatus.paxTypeFarePriority());
    if (!pupqItem.paused())
    {
      pausedStatus.setPaxTypeFarePriority(sourceStatus.paxTypeFarePriority());
    }
  }

  status.setFareByRulePriority(PriorityStatus::mergeFbrPriorities(
      status.fareByRulePriority(), sourceStatus.fareByRulePriority()));

  if (!pupqItem.paused())
  {
    pausedStatus.setFareByRulePriority(PriorityStatus::mergeFbrPriorities(
        pausedStatus.fareByRulePriority(), sourceStatus.fareByRulePriority()));
  }

  if (sourceStatus.fareCxrTypePriority() > status.fareCxrTypePriority())
  {
    status.setFareCxrTypePriority(sourceStatus.fareCxrTypePriority());
    if (!pupqItem.paused())
    {
      pausedStatus.setFareCxrTypePriority(sourceStatus.fareCxrTypePriority());
    }
  }

  if (sourceStatus.negotiatedFarePriority() > status.negotiatedFarePriority())
  {
    status.setNegotiatedFarePriority(sourceStatus.negotiatedFarePriority());
    if (!pupqItem.paused())
    {
      pausedStatus.setNegotiatedFarePriority(sourceStatus.negotiatedFarePriority());
    }
  }

  if (!pupqItem.paused())
  {
    pausedStatus.setPtfRank(pausedStatus.ptfRank() + sourceStatus.ptfRank());
  }

  status.setPtfRank(status.ptfRank() + sourceStatus.ptfRank());

  if (UNLIKELY(trx.getOptions()->isZeroFareLogic()))
  {
    fppqItem.updateCabinPriority(pupqItem.getCabinPriority());
    fppqItem.setNumberOfStopsPriority(static_cast<unsigned>(itin->travelSeg().size()));
  }
}

DifferentialData*
differentialData(const FareUsage* fu, const TravelSeg* tvlSeg)
{
  if (fu->differentialPlusUp().empty())
    return nullptr;

  bool diffrentialFound = false;
  std::vector<TravelSeg*>::const_iterator i, iEnd;
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iter, iterEnd;

  std::vector<DifferentialData*>::const_iterator diffI = fu->differentialPlusUp().begin();
  std::vector<DifferentialData*>::const_iterator diffIEnd = fu->differentialPlusUp().end();

  DifferentialData* diff = nullptr;
  for (; diffI != diffIEnd; ++diffI)
  {
    diff = *diffI;
    if (diff == nullptr)
      continue;

    DifferentialData::STATUS_TYPE aStatus = diff->status();

    if (!(aStatus == DifferentialData::SC_PASSED ||
          aStatus == DifferentialData::SC_CONSOLIDATED_PASS))
    {
      diff = nullptr;
      continue;
    }

    for (i = diff->travelSeg().begin(),
        iEnd = diff->travelSeg().end(),
        iter = diff->fareHigh()->segmentStatus().begin(),
        iterEnd = diff->fareHigh()->segmentStatus().end();
         i != iEnd && iter != iterEnd;
         ++i, ++iter)
    {
      if ((*i) != tvlSeg)
        continue;
      diffrentialFound = true;
      break;
    }
    if (diffrentialFound)
      break;
  }
  if (!diffrentialFound)
    diff = nullptr;
  return diff;
}

const PaxTypeFare::SegmentStatus&
diffSegStatus(const DifferentialData* diff, const TravelSeg* tvlSeg)
{
  std::vector<TravelSeg*>::const_iterator i = diff->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iEnd = diff->travelSeg().end();

  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iter =
      diff->fareHigh()->segmentStatus().begin();
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iterEnd =
      diff->fareHigh()->segmentStatus().end();

  for (; i != iEnd && iter != iterEnd; ++i, ++iter)
  {
    if ((*i) != tvlSeg)
      continue;
    return *iter;
  }

  return *(diff->fareHigh()->segmentStatus().begin());
}

void
addMainTripSideTripLink(FPPQItem& fppqItem)
{
  if (fppqItem.hasCopiedPricingUnit() == false)
    clearMainTripSideTripLink(*fppqItem.farePath());
  addMainTripSideTripLink(*fppqItem.farePath(), fppqItem.puPath());
}

std::string
displayValCxr(const std::vector<CarrierCode>& vcList)
{
  std::ostringstream oss;
  if (!vcList.empty())
  {
    oss << " VAL-CXR: ";
    for (const CarrierCode& cxr : vcList)
    {
      oss << cxr << " ";
    }
  }
  return oss.str();
}

void
setFailedPricingUnits(const uint16_t puFactIdx,
                      FPPQItem& fppqItem,
                      FarePathFactoryFailedPricingUnits& failedPricingUnits)
{
  if (fppqItem.ignorePUIndices() || fppqItem.puPath()->totalPU() == 1)
  {
    return;
  }

  const uint32_t puIdx = fppqItem.puIndices()[puFactIdx];
  if (fppqItem.farePath()->validatingCarriers().empty())
    failedPricingUnits.setFailed(puFactIdx, puIdx);
  else
    failedPricingUnits.setFailed(
        puFactIdx, fppqItem.farePath()->validatingCarriers().front(), puIdx);
}
bool
copyPricingUnit(PricingUnit& oldPU, PricingUnit& newPU, FarePathFactoryStorage& storage)
{
  newPU.paxType() = oldPU.paxType();
  newPU.puType() = oldPU.puType();
  newPU.puSubType() = oldPU.puSubType();
  newPU.sameNationOJ() = oldPU.sameNationOJ();
  newPU.ojSurfaceStatus() = oldPU.ojSurfaceStatus();
  newPU.puFareType() = oldPU.puFareType();
  newPU.geoTravelType() = oldPU.geoTravelType();
  newPU.turnAroundPoint() = oldPU.turnAroundPoint();
  newPU.isSideTripPU() = oldPU.isSideTripPU();
  newPU.hasSideTrip() = oldPU.hasSideTrip();
  newPU.noPUToEOE() = oldPU.noPUToEOE();
  newPU.setTotalPuNucAmount(oldPU.getTotalPuNucAmount());
  newPU.mileage() = oldPU.mileage();
  newPU.taxAmount() = oldPU.taxAmount();
  newPU.setBaggageLowerBound(oldPU.baggageLowerBound());
  newPU.cpFailedStatus() = oldPU.cpFailedStatus();
  newPU.hasKeepFare() = oldPU.hasKeepFare();
  newPU.ruleFailedButSoftPassForKeepFare() = oldPU.ruleFailedButSoftPassForKeepFare();
  newPU.combinationFailedButSoftPassForKeepFare() = oldPU.combinationFailedButSoftPassForKeepFare();
  newPU.itinWithinScandinavia() = oldPU.itinWithinScandinavia();
  newPU.isOverrideCxrCat05TktAftRes() = oldPU.isOverrideCxrCat05TktAftRes();
  newPU.setFlexFaresGroupId(oldPU.getFlexFaresGroupId());
  newPU.validatingCarriers() = oldPU.validatingCarriers();

  for (FareUsage* oldFu : oldPU.fareUsage())
  {
    FareUsage* fareU = &storage.constructFareUsage();
    *fareU = *oldFu;
    newPU.fareUsage().push_back(fareU);
  }

  newPU.travelSeg().insert(
      newPU.travelSeg().end(), oldPU.travelSeg().begin(), oldPU.travelSeg().end());

  return true;
}

void
copyReusablePUToMutablePU(FarePathFactoryStorage& storage, FPPQItem& fppqItem)
{
  if (UNLIKELY(fppqItem.hasCopiedPricingUnit()))
    return;

  FarePath& fpath = *fppqItem.farePath();
  std::vector<PricingUnit*>& puVect = fpath.pricingUnit();
  int size = puVect.size();
  for (int i = 0; i < size; ++i)
  {
    PricingUnit& masterPU = *puVect[i];
    PricingUnit* prU = &storage.constructPricingUnit();
    copyPricingUnit(masterPU, *prU, storage);
    puVect[i] = prU;
  }

  fppqItem.hasCopiedPricingUnit() = true;

  addMainTripSideTripLink(fppqItem);

  //
  // debugOnlyDisplayMainTripSTLink(fpath);
}

uint16_t
fareComponentCount(const FarePath& farePath)
{
  uint16_t fcCount = 0;
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    fcCount += pu->fareUsage().size();
  }
  return fcCount;
}

bool
checkMotherAvailability(const FarePath* farePath,
                        const uint16_t numSeatsRequired)
{
  AvailabilityChecker availChecker;

  for (const PricingUnit* pu : farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();
      const FareMarket& fm = *ptf.fareMarket();

      if (fm.classOfServiceVec().size() != fm.travelSeg().size())
      {
        std::vector<std::vector<ClassOfService*>*> cosVec;
        for (TravelSeg* const segment : fm.travelSeg())
          cosVec.push_back(&segment->classOfService());

        if (!availChecker.checkBookedAvailability(numSeatsRequired, fu->segmentStatus(),
                                                  cosVec, fm.travelSeg()))
          return false;
      }
      else
      {
        if (!availChecker.checkBookedAvailability(numSeatsRequired, fu->segmentStatus(),
                                                  fm.classOfServiceVec(), fm.travelSeg()))
          return false;
      }
    }
  }
  return true;
}

bool
checkSimilarItinAvailability(const FarePath* farePath,
                             const uint16_t numSeatsRequired,
                             const Itin& motherItin)
{
  AvailabilityChecker availChecker;

  for (const PricingUnit* pu : farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();
      const FareMarket& fm = *ptf.fareMarket();

      auto isForItin = [&](const SimilarItinData& similar) {
        return similar.itin == farePath->itin();
      };
      auto similarIt = std::find_if(motherItin.getSimilarItins().cbegin(),
                                    motherItin.getSimilarItins().cend(),
                                    isForItin);
      TSE_ASSERT(similarIt != motherItin.getSimilarItins().cend());
      auto& fareMarketData = similarIt->fareMarketData.at(&fm);
      TSE_ASSERT(fareMarketData.classOfService.size() == fareMarketData.travelSegments.size());
      if (!availChecker.checkBookedAvailability(numSeatsRequired, fu->segmentStatus(),
                                                fareMarketData.classOfService,
                                                fareMarketData.travelSegments))
        return false;
    }
  }
  return true;
}

} // FarePathUtils namespace
} // tse namespace
