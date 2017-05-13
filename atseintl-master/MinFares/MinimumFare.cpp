//----------------------------------------------------------------------------
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
#include "MinFares/MinimumFare.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace tse
{

FIXEDFALLBACK_DECL(APO29538_StopoverMinStay);

static Logger
logger("atseintl.MinFares.MinimumFare");

const std::string MinimumFare::MISSING_DATA_ERROR_MSG =
    "UNABLE TO VALIDATE MINIMUM FARE PROCESSING, MISSING DATA";

// Hold Exclusion Indicators
const std::string MinimumFare::ROUTING_EXCLUSION_IND = "R";
const std::string MinimumFare::MILEAGE_EXCLUSION_IND = "M";
const std::string MinimumFare::RULE_EXCLUSION_IND[] = {
  "-",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  "10", "11",
  "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
  "-",  "25", "-",  "-",  "-",  "-",  "-",  "-",  "-",  "-",  "-",  "35"
};

void
MinimumFare::printRexDatesAdjustment(DiagCollector& diag, const DateTime& retrievalDate)
{
  if (diag.isActive())
    diag << "RETRIEVAL DATE/" << retrievalDate.dateToString(YYYYMMDD, "-") << "\n";
}

void
MinimumFare::printFareInfo(const PtfPair& ptfPair,
                           MinFareFareSelection::FareDirectionChoice fareDirection,
                           DiagCollector& diag,
                           const MinimumFareModule module,
                           const char* prefix,
                           bool isRtFare,
                           const MoneyAmount& plusUpAndSurcharge)
{
  if (ptfPair.first && ptfPair.second)
  {
    if (diag.isActive())
    {
      if (IS_DEBUG_ENABLED(logger))
      {
        printFareInfo(*ptfPair.first, diag, module, "C ", isRtFare);
        printFareInfo(*ptfPair.second, diag, module, "C ", isRtFare);
      }

      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        diag << "  " << ptfPair.first->fareMarket()->boardMultiCity() << "-"
             << ptfPair.second->fareMarket()->offMultiCity();
      }
      else
      {
        diag << "  " << ptfPair.second->fareMarket()->offMultiCity() << "-"
             << ptfPair.first->fareMarket()->boardMultiCity();
      }

      diag << " CONST. C/" << ptfPair.first->fareMarket()->offMultiCity() << "  NUC";
      diag.setf(std::ios::right, std::ios::adjustfield);
      diag.setf(std::ios::fixed, std::ios::floatfield);
      diag.precision(2);
      diag << std::setw(9) << (ptfPair.first->nucFareAmount() + ptfPair.second->nucFareAmount() +
                               plusUpAndSurcharge) << "         "
           << (fareDirection == MinFareFareSelection::OUTBOUND ? "O" : "I") << "\n";
    }
  }
  else if (ptfPair.first)
  {
    printFareInfo(*ptfPair.first, diag, module, prefix, isRtFare, plusUpAndSurcharge);
  }
}

void
MinimumFare::printFareInfo(const PaxTypeFare& paxTypeFare,
                           DiagCollector& diag,
                           const MinimumFareModule module,
                           const char* prefix,
                           bool isRtFare,
                           const MoneyAmount& plusUpAndSurcharge,
                           const bool isNetFare)
{
  if (LIKELY(!diag.isActive()))
    return;

  const FareMarket* fareMarket = paxTypeFare.fareMarket();
  if (!fareMarket)
  {
    return;
  }

  if (prefix)
  {
    diag << prefix;
  }

  if (fareMarket->travelSeg().size() > 0)
  {
    if ((module == DMC) && (paxTypeFare.directionality() == TO))
    {
      diag << fareMarket->offMultiCity();
      diag << "-";
      diag << fareMarket->boardMultiCity();
    }
    else
    {
      diag << fareMarket->boardMultiCity();
      diag << "-";
      diag << fareMarket->offMultiCity();
    }
  }

  if (paxTypeFare.fare()->isIndustry())
    diag << "/"; // Replace Cross Lorraine
  else
    diag << " ";
  diag << "/" << fareMarket->governingCarrier().c_str() << "/ ";

  diag.setf(std::ios::left, std::ios::adjustfield);

  diag << std::setw(9) << paxTypeFare.fareClass();

  diag.setf(std::ios::left, std::ios::adjustfield);
  diag << "NUC";

  diag.setf(std::ios::right, std::ios::adjustfield);
  diag.setf(std::ios::fixed, std::ios::floatfield);
  diag.precision(2);

  MoneyAmount amount;
  if (isNetFare)
  {
    const NegPaxTypeFareRuleData* netFare = paxTypeFare.getNegRuleData();
    amount = netFare->nucNetAmount();
  }
  else
  {
    amount = paxTypeFare.nucFareAmount();
  }
  if (isRtFare)
  {
    diag << std::setw(9) << amount * 2 + plusUpAndSurcharge;
  }
  else
  {
    diag << std::setw(9) << amount + plusUpAndSurcharge;
  }

  if (paxTypeFare.isRouting())
  {
    diag << "   R ";
  }
  else
  {
    if (paxTypeFare.mileageSurchargePctg())
    {
      diag << std::setw(3) << paxTypeFare.mileageSurchargePctg();
      diag << "M ";
    }
    else
    {
      diag << "   M ";
    }
  }

  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
  {
    diag << "1 ";
  }
  else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    diag << "2 ";
  }
  else
  {
    diag << "3 ";
  }

  if (paxTypeFare.directionality() == FROM)
    diag << "  O   ";
  else if (paxTypeFare.directionality() == TO)
    diag << "  I   ";
  else
    diag << "      ";

  diag << std::setw(3) << paxTypeFare.fcaFareType();

  diag << "-";
  diag << paxTypeFare.cabin() << " ";

  std::string gd;
  globalDirectionToStr(gd, paxTypeFare.fareMarket()->getGlobalDirection());
  diag << gd;

  diag << "   X/-\n";
}

void
MinimumFare::printPaxTypeFare(const PaxTypeFare& paxTypeFare,
                              std::ostream& os,
                              MinimumFareModule module,
                              const char* prefix,
                              bool isRtFare,
                              const MoneyAmount& plusUpAndSurcharge)
{
  const FareMarket* fareMarket = paxTypeFare.fareMarket();
  if (!fareMarket)
  {
    return;
  }

  if (prefix)
  {
    os << prefix;
  }

  if (fareMarket->travelSeg().size() > 0)
  {
    if ((module == DMC) && (paxTypeFare.directionality() == TO))
    {
      os << fareMarket->offMultiCity();
      os << "-";
      os << fareMarket->boardMultiCity();
    }
    else
    {
      os << fareMarket->boardMultiCity();
      os << "-";
      os << fareMarket->offMultiCity();
    }
  }

  if (paxTypeFare.fare()->isIndustry())
    os << "/"; // Replace Cross Lorraine
  else
    os << " ";
  os << "/" << fareMarket->governingCarrier().c_str() << "/ ";

  os.setf(std::ios::left, std::ios::adjustfield);

  os << std::setw(9) << paxTypeFare.fareClass();

  os.setf(std::ios::left, std::ios::adjustfield);
  os << "NUC";

  os.setf(std::ios::right, std::ios::adjustfield);
  os.setf(std::ios::fixed, std::ios::floatfield);
  os.precision(2);
  if (isRtFare)
  {
    os << std::setw(9) << (paxTypeFare.nucFareAmount()) * 2 + plusUpAndSurcharge;
  }
  else
  {
    os << std::setw(9) << paxTypeFare.nucFareAmount() + plusUpAndSurcharge;
  }

  if (paxTypeFare.isRouting())
  {
    os << "   R ";
  }
  else
  {
    if (paxTypeFare.mileageSurchargePctg())
    {
      os << std::setw(3) << paxTypeFare.mileageSurchargePctg();
      os << "M ";
    }
    else
    {
      os << "   M ";
    }
  }

  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
  {
    os << "1 ";
  }
  else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    os << "2 ";
  }
  else
  {
    os << "3 ";
  }

  if (paxTypeFare.directionality() == FROM)
    os << "  O   ";
  else if (paxTypeFare.directionality() == TO)
    os << "  I   ";
  else
    os << "      ";

  os << std::setw(3) << paxTypeFare.fcaFareType();

  os << "-";
  os << paxTypeFare.cabin().getCabinIndicator() << " ";

  std::string gd;
  globalDirectionToStr(gd, paxTypeFare.fareMarket()->getGlobalDirection());
  os << gd;

  os << "   X/-\n";
}

const Loc*
MinimumFare::originOfPricingUnit(const PricingUnit& pu)
{
  for (const auto fu : pu.fareUsage())
  {
    if (!fu || !fu->paxTypeFare())
      continue;

    return fu->paxTypeFare()->fareMarket()->origin();
  }

  return nullptr;
}

bool
MinimumFare::isBetweenLocs(PricingTrx& trx,
                           const Loc& loc1,
                           const Loc& loc2,
                           const LocTypeCode& loc1Type,
                           const LocCode& loc1Code,
                           const LocTypeCode& loc2Type,
                           const LocCode& loc2Code)
{
  if (loc2Type == MinimumFare::BLANK) // between loc1 and anywhere
  {
    if (LocUtil::isInLoc(loc1,
                         loc1Type,
                         loc1Code,
                         Vendor::SABRE,
                         MANUAL,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()) ||
        LocUtil::isInLoc(loc2,
                         loc1Type,
                         loc1Code,
                         Vendor::SABRE,
                         MANUAL,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
      return true;
  }
  else // between loc1 and loc2
  {
    if ((LocUtil::isInLoc(loc1,
                          loc1Type,
                          loc1Code,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()) &&
         LocUtil::isInLoc(loc2,
                          loc2Type,
                          loc2Code,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT())) ||
        (LocUtil::isInLoc(loc2,
                          loc1Type,
                          loc1Code,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()) &&
         LocUtil::isInLoc(loc1,
                          loc2Type,
                          loc2Code,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT())))
      return true;
  }

  return false;
}

bool
MinimumFare::isWithinLoc(PricingTrx& trx,
                         const std::vector<TravelSeg*>& travelSegs,
                         const LocTypeCode& locType,
                         const LocCode& locCode,
                         bool checkHiddenStop /*=true*/)
{
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); i++)
  {
    AirSeg* curTvlSeg = dynamic_cast<AirSeg*>(*i);
    if (curTvlSeg == nullptr)
      continue;

    const Loc* loc = curTvlSeg->origin();
    if ((loc != nullptr) &&
        !LocUtil::isInLoc(*loc,
                          locType,
                          locCode,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()))
      return false;

    loc = curTvlSeg->destination();
    if ((loc != nullptr) &&
        !LocUtil::isInLoc(*loc,
                          locType,
                          locCode,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()))
      return false;

    if (LIKELY(checkHiddenStop))
    {
      std::vector<const Loc*>& hiddenStops = curTvlSeg->hiddenStops();
      std::vector<const Loc*>::iterator stopIter = hiddenStops.begin();
      for (; stopIter != hiddenStops.end(); stopIter++)
      {
        if (!LocUtil::isInLoc(**stopIter,
                              locType,
                              locCode,
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              trx.getRequest()->ticketingDT()))
          return false;
      }
    }
  }

  return true;
}

bool
MinimumFare::matchGeo(PricingTrx& trx,
                      const Directionality directionality,
                      const LocKey& loc1,
                      const LocKey& loc2,
                      const std::vector<TravelSeg*>& tvlSegs,
                      const Itin& itin,
                      bool reverseOrigin)
{
  if (UNLIKELY(tvlSegs.empty()))
    return false;

  const Loc* setOrigin = (reverseOrigin ? tvlSegs.back()->destination() : tvlSegs[0]->origin());
  const Loc* setDest = (reverseOrigin ? tvlSegs[0]->origin() : tvlSegs.back()->destination());

  if (UNLIKELY(setOrigin == nullptr || setDest == nullptr))
    return false;

  switch (directionality)
  {
  case FROM:
    if (loc2.locType() == MinimumFare::BLANK) // From LOC1 to anywhere
    {
      if (LocUtil::isInLoc(*setOrigin,
                           loc1.locType(),
                           loc1.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))
        return true;
    }
    else
    {
      if (LocUtil::isInLoc(*setOrigin,
                           loc1.locType(),
                           loc1.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(*setDest,
                           loc2.locType(),
                           loc2.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))
        return true;
    }
    break;
  case BETWEEN:
    if (isBetweenLocs(
            trx, *setOrigin, *setDest, loc1.locType(), loc1.loc(), loc2.locType(), loc2.loc()))
      return true;

    break;
  case WITHIN:
    if (isWithinLoc(trx, tvlSegs, loc1.locType(), loc1.loc()))
      return true;

    break;
  case ORIGIN:
    if (LocUtil::isInLoc(*(itin.travelSeg()[0]->origin()),
                         loc1.locType(),
                         loc1.loc(),
                         Vendor::SABRE,
                         MANUAL,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
      return true;

    break;
  case TERMINATE:
    if ((loc2.locType() == MinimumFare::BLANK) ||
        (LocUtil::isInLoc(*(itin.travelSeg().back()->destination()),
                          loc2.locType(),
                          loc2.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT())))
      return true;

    break;
  default:
    return false;
  }

  return false;
}

/**
 * Return true : intermediate exclusion - no MinFare check at the intermediate point
 *        false otherwise
 **/
bool
MinimumFare::checkIntermediateExclusion(
    PricingTrx& trx,
    const MinimumFareModule module,
    const MinFareAppl& matchedApplItem,
    const MinFareDefaultLogic* matchedDefaultItem,
    const bool& selectNormalFare,
    const std::vector<TravelSeg*>& thruFareTravelSegs,
    const Itin& itin,
    const PricingUnit* pu,
    const FareUsage* fareUsage,
    const MinFareFareSelection::FareDirectionChoice fareDirection,
    std::vector<TravelSeg*>::const_iterator travelBoardIter,
    std::vector<TravelSeg*>::const_iterator travelOffIter,
    bool isBoardSegAfterSurfaceAtFareBreak,
    bool isOffSegFollowedBySurfaceAtFareBreak,
    bool reverseFcOrigin)
{

  bool ret = false;
  bool matchedGeo = true;

  bool stopOver = checkIntermediateStop(itin,
                                        fareUsage,
                                        thruFareTravelSegs,
                                        travelBoardIter,
                                        travelOffIter,
                                        isBoardSegAfterSurfaceAtFareBreak,
                                        isOffSegFollowedBySurfaceAtFareBreak);

  const Directionality& directionality = matchedApplItem.intermDirectionalInd();
  std::vector<TravelSeg*> travelSegVec(travelBoardIter, travelOffIter + 1);

  // Check Additional Intermediate Check if Intermediate Locations and stopover/conn restr matched
  if ((matchedApplItem.intermediateLoc1().locType() != BLANK) ||
      (matchedApplItem.intermediateLoc2().locType() != BLANK))
  {
    if (travelBoardIter == thruFareTravelSegs.begin()) // thru origin - intermidate point
    {
      matchedGeo = ((matchedApplItem.intermediateLoc2().locType() == BLANK) &&
                    LocUtil::isInLoc(*((*travelOffIter)->destination()),
                                     matchedApplItem.intermediateLoc1().locType(),
                                     matchedApplItem.intermediateLoc1().loc(),
                                     Vendor::SABRE,
                                     MANUAL,
                                     LocUtil::OTHER,
                                     GeoTravelType::International,
                                     EMPTY_STRING(),
                                     trx.getRequest()->ticketingDT()));
    }
    else if (travelOffIter == (thruFareTravelSegs.end() - 1)) // intermediate - thru dest
    {
      matchedGeo = ((matchedApplItem.intermediateLoc2().locType() == BLANK) &&
                    LocUtil::isInLoc(*((*travelBoardIter)->origin()),
                                     matchedApplItem.intermediateLoc1().locType(),
                                     matchedApplItem.intermediateLoc1().loc(),
                                     Vendor::SABRE,
                                     MANUAL,
                                     LocUtil::OTHER,
                                     GeoTravelType::International,
                                     EMPTY_STRING(),
                                     trx.getRequest()->ticketingDT()));
    }
    else // intermediate - intermediate
    {
      matchedGeo = matchGeo(trx,
                            directionality,
                            matchedApplItem.intermediateLoc1(),
                            matchedApplItem.intermediateLoc2(),
                            travelSegVec,
                            itin,
                            reverseFcOrigin);
    }

    if (matchedGeo && (((matchedApplItem.stopConnectRestr() == STOPOVER_POINT) &&
                        stopOver) || // Stop-Over required
                       ((matchedApplItem.stopConnectRestr() == CONNECTION_POINT) &&
                        !stopOver) || // Connection required
                       (matchedApplItem.stopConnectRestr() == ANY_POINT) ||
                       (matchedApplItem.stopConnectRestr() == BLANK)))
    {
      if (UNLIKELY(checkAdditionalPoint(module,
                               matchedApplItem,
                               matchedDefaultItem,
                               thruFareTravelSegs,
                               travelBoardIter,
                               travelOffIter,
                               stopOver)))
      {
        return false;
      }
      else
        return true;
    }
  }

  // check apply/do not apply per minimum fare module
  switch (module)
  {
  case HIP:
    if (LIKELY(matchedApplItem.hipCheckAppl() == YES))
    {
      if ((matchedApplItem.hipStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.hipStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.hipStopTktInd() == TICKET_POINT ||
             matchedApplItem.hipStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  case CTM:
    if (matchedApplItem.ctmCheckAppl() == YES)
    {
      if ((matchedApplItem.ctmStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.ctmStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.ctmCheckAppl() == NO)
    {
      return true;
    }
    else if (matchedApplItem.ctmStopTktInd() == TICKET_POINT ||
             matchedApplItem.ctmStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  case BHC:
    if (matchedApplItem.backhaulCheckAppl() == YES)
    {
      if ((matchedApplItem.backhaulStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.backhaulStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.backhaulStopTktInd() == TICKET_POINT ||
             matchedApplItem.backhaulStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  case DMC:
    if (matchedApplItem.dmcCheckAppl() == YES)
    {
      if ((matchedApplItem.dmcStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.dmcStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.dmcCheckAppl() == NO)
    {
      return true;
    }
    else if (matchedApplItem.dmcStopTktInd() == TICKET_POINT ||
             matchedApplItem.dmcStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  case COM:
    if (matchedApplItem.comCheckAppl() == YES)
    {
      if ((matchedApplItem.comStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.comStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.comStopTktInd() == TICKET_POINT ||
             matchedApplItem.comStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  case CPM:
    if (matchedApplItem.cpmCheckAppl() == YES)
    {
      if ((matchedApplItem.cpmStopTktInd() != TICKET_POINT) &&
          (matchedApplItem.cpmStopTktInd() != STOPOVER_POINT || !stopOver))
      {
        ret = true;
      }
    }
    else if (matchedApplItem.cpmCheckAppl() == NO)
    {
      return true;
    }
    else if (matchedApplItem.cpmStopTktInd() == TICKET_POINT ||
             matchedApplItem.cpmStopTktInd() == BLANK)
    {
      ret = true;
    }
    break;

  default:
    break;
  }

  if (ret && !stopOver && module == HIP &&
      checkAdditionalNation(matchedApplItem,
                            matchedDefaultItem,
                            selectNormalFare,
                            thruFareTravelSegs,
                            itin,
                            pu,
                            fareUsage,
                            fareDirection,
                            travelBoardIter,
                            travelOffIter,
                            isBoardSegAfterSurfaceAtFareBreak,
                            isOffSegFollowedBySurfaceAtFareBreak))
  {
    ret = false;
  }

  // -----------------------------------------------------------
  // Intermediate Location Travel Application match/action logic
  // -----------------------------------------------------------
  // If Intermediate Location Travel Application data is not coded skip check
  //
  // If Intermediate Location Travel Application data geo coded, geo matched
  //     and matched the others fields then use Apply/not Apply field directly
  //
  // If Intermediate Location Travel Application data geo coded, geo matched
  //     and no matched the others fields then reverse Apply/not Apply field value
  //
  // If Intermediate Location Travel Application data geo coded, geo not matched
  //     reverse Apply/not Apply field value

  if (UNLIKELY(matchedApplItem.betwLoc1().locType() != MinimumFare::BLANK ||
      matchedApplItem.betwLoc2().locType() != MinimumFare::BLANK))
  {
    const Directionality& intermediateDir = matchedApplItem.betwDirectionalInd();
    travelBoardIter = travelSegVec.begin();
    bool interlineFlight = false;

    if (matchGeo(trx,
                 intermediateDir,
                 matchedApplItem.betwLoc1(),
                 matchedApplItem.betwLoc2(),
                 travelSegVec,
                 itin,
                 reverseFcOrigin))
    {

      std::vector<TravelSeg*>::const_iterator tvlBrdIter = travelSegVec.begin();
      std::vector<TravelSeg*>::const_iterator tvlEndIter = travelSegVec.end();
      CarrierCode marketingCarrier;

      for (; tvlBrdIter < tvlEndIter; tvlBrdIter++)
      {
        const AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlBrdIter);
        if (airSeg == nullptr)
          continue;

        if (marketingCarrier.empty())
          marketingCarrier = airSeg->marketingCarrierCode();
        else if (marketingCarrier != airSeg->marketingCarrierCode())
          interlineFlight = true;

      } // for loop within intermediate board and off point

      // Check service restriction
      if (matchedApplItem.serviceRestr() != BLANK)
      {
        if ((matchedApplItem.serviceRestr() == ONLINE_SERVICE) && (interlineFlight == true))
          return !ret;

        if ((matchedApplItem.serviceRestr() == INLINE_SERVICE) && (interlineFlight == false))
          return !ret;
      }
      // Check direct flight
      if ((matchedApplItem.directInd() == YES) && (travelSegVec.size() > 1))
        return !ret;

      // Check non stop flight (if both direct and non stop is "YES" , either one pass is fine)
      if ((matchedApplItem.directInd() != YES) && (matchedApplItem.nonStopInd() == YES) &&
          ((travelSegVec.size() > 1) || (!(*(travelSegVec.begin()))->hiddenStops().empty())))
        return !ret;

      return ret; // all fields matched
    } // if geo match
    return !ret; // geo not matched
  } // geo coded for Location Travel Application data check

  return ret;
}

bool
MinimumFare::checkAdditionalPoint(const MinimumFareModule module,
                                  const MinFareAppl& matchedApplItem,
                                  const MinFareDefaultLogic* matchedDefaultItem,
                                  const std::vector<TravelSeg*>& travelSegs,
                                  std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                  std::vector<TravelSeg*>::const_iterator travelOffIter,
                                  bool stopOver)

{
  // check Additional Intermediate Check point per minimum fare module
  bool ret = false;
  switch (module)
  {
  case HIP:
    if (UNLIKELY((matchedApplItem.intHipCheckAppl() == YES) &&
        (((matchedApplItem.intHipStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intHipStopTktInd() != STOPOVER_POINT))))
    {
      ret = true;
    }
    break;

  case CTM:
    if ((matchedApplItem.intCtmCheckAppl() == YES) &&
        (((matchedApplItem.intCtmStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intCtmStopTktInd() != STOPOVER_POINT)))
    {
      ret = true;
    }
    break;

  case BHC:
    if ((matchedApplItem.intBackhaulChkAppl() == YES) &&
        (((matchedApplItem.intBackhaulStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intBackhaulStopTktInd() != STOPOVER_POINT)))
    {
      ret = true;
    }
    break;

  case DMC:
    if ((matchedApplItem.intDmcCheckAppl() == YES) &&
        (((matchedApplItem.intDmcStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intDmcStopTktInd() != STOPOVER_POINT)))
    {
      ret = true;
    }
    break;

  case COM:
    if ((matchedApplItem.intComCheckAppl() == YES) &&
        (((matchedApplItem.intComStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intComStopTktInd() != STOPOVER_POINT)))
    {
      ret = true;
    }
    break;

  case CPM:
    if ((matchedApplItem.intCpmCheckAppl() == YES) &&
        (((matchedApplItem.intCpmStopTktInd() == STOPOVER_POINT) && stopOver) ||
         (matchedApplItem.intCpmStopTktInd() != STOPOVER_POINT)))
    {
      ret = true;
    }
    break;

  default:
    break;
  }

  return ret;
}

bool
MinimumFare::checkAdditionalNation(const MinFareAppl& matchedApplItem,
                                   const MinFareDefaultLogic* matchedDefaultItem,
                                   const bool& selectNormalFare,
                                   const std::vector<TravelSeg*>& travelSegs,
                                   const Itin& itin,
                                   const PricingUnit* pu,
                                   const FareUsage* fareUsage,
                                   const MinFareFareSelection::FareDirectionChoice fareDirection,
                                   std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                   std::vector<TravelSeg*>::const_iterator travelOffIter,
                                   bool isBoardSegAfterSurfaceAtFareBreak,
                                   bool isOffSegFollowedBySurfaceAtFareBreak)

{
  //--- check Additional HIP Check Points using nation information
  NationCode fcOrigNation;
  if (fareDirection == MinFareFareSelection::OUTBOUND)
    fcOrigNation = travelSegs.front()->origin()->nation();
  else
    fcOrigNation = travelSegs.back()->destination()->nation();

  NationCode fareOrigNation = (*travelBoardIter)->origin()->nation();
  NationCode fareDestNation = (*travelOffIter)->destination()->nation();

  if (fareDirection == MinFareFareSelection::INBOUND)
  {
    fareOrigNation = fareDestNation;
    fareDestNation = (*travelBoardIter)->origin()->nation();
  }

  bool sameOrigNation = (fcOrigNation == fareOrigNation);
  bool sameDestNation = (fcOrigNation == fareDestNation);

  bool qualifyBoardPoint = false;
  bool qualifyOffPoint = false;

  if (selectNormalFare)
  {
    // TODO: QT - This pretty much same block of code is repeated four times. Refactor!

    if (matchedDefaultItem != nullptr)
    {
      if ((matchedDefaultItem->nmlHipOrigNationTktPt() == BLANK) &&
          (matchedDefaultItem->nmlHipTicketedPt() == BLANK))
        return false;

      if (LIKELY((matchedDefaultItem->nmlHipOrigNationTktPt() == NO) &&
          (matchedDefaultItem->nmlHipTicketedPt() == NO)))
        return false;

      // check intermediate fare board point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if (sameOrigNation && (matchedDefaultItem->nmlHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameOrigNation && (matchedDefaultItem->nmlHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(
                     itin, fareUsage, **(travelBoardIter - 1), **travelBoardIter))
        {
          qualifyBoardPoint = true;
        }
      }
      else
      {
        if (sameDestNation && (matchedDefaultItem->nmlHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameDestNation && (matchedDefaultItem->nmlHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }

      // check intermediate fare off point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if ((sameDestNation) && (matchedDefaultItem->nmlHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameDestNation) && (matchedDefaultItem->nmlHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }
      else
      {
        if ((sameOrigNation) && (matchedDefaultItem->nmlHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameOrigNation) && (matchedDefaultItem->nmlHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }

      if ((qualifyBoardPoint && qualifyOffPoint) ||
          (fareDirection == MinFareFareSelection::OUTBOUND && qualifyBoardPoint &&
           matchedDefaultItem->nmlHipTicketedPt() == YES) ||
          (fareDirection == MinFareFareSelection::INBOUND && qualifyOffPoint &&
           matchedDefaultItem->nmlHipTicketedPt() == YES))
      {
        return true;
      }
    }
    else // use application table
    {
      if ((matchedApplItem.nmlHipOrigNationTktPt() == BLANK) &&
          (matchedApplItem.nmlHipTicketedPt() == BLANK))
        return false;

      if ((matchedApplItem.nmlHipOrigNationTktPt() == NO) &&
          (matchedApplItem.nmlHipTicketedPt() == NO))
        return false;

      // check intermediate fare board point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if (sameOrigNation && (matchedApplItem.nmlHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameOrigNation && (matchedApplItem.nmlHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }
      else
      {
        if (sameDestNation && (matchedApplItem.nmlHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameDestNation && (matchedApplItem.nmlHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }

      // check intermediate fare off point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if ((sameDestNation) && (matchedApplItem.nmlHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameDestNation) && (matchedApplItem.nmlHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }
      else
      {
        if ((sameOrigNation) && (matchedApplItem.nmlHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameOrigNation) && (matchedApplItem.nmlHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }

      if ((qualifyBoardPoint && qualifyOffPoint) ||
          (fareDirection == MinFareFareSelection::OUTBOUND && qualifyBoardPoint &&
           matchedApplItem.nmlHipTicketedPt() == YES) ||
          (fareDirection == MinFareFareSelection::INBOUND && qualifyOffPoint &&
           matchedApplItem.nmlHipTicketedPt() == YES))
      {
        return true;
      }
    }
  }
  else // use special fare indicator
  {
    if (matchedDefaultItem != nullptr)
    {
      if (UNLIKELY((matchedDefaultItem->spclHipOrigNationTktPt() == BLANK) &&
          (matchedDefaultItem->spclHipTicketedPt() == BLANK)))
        return false;

      if (LIKELY((matchedDefaultItem->spclHipOrigNationTktPt() == NO) &&
          (matchedDefaultItem->spclHipTicketedPt() == NO)))
        return false;

      // check intermediate fare board point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if (sameOrigNation && (matchedDefaultItem->spclHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameOrigNation && (matchedDefaultItem->spclHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }
      else
      {
        if (sameDestNation && (matchedDefaultItem->spclHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameDestNation && (matchedDefaultItem->spclHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }

      // check intermediate fare off point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if ((sameDestNation) && (matchedDefaultItem->spclHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameDestNation) && (matchedDefaultItem->spclHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }
      else
      {
        if ((sameOrigNation) && (matchedDefaultItem->spclHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameOrigNation) && (matchedDefaultItem->spclHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }

      if ((qualifyBoardPoint && qualifyOffPoint) ||
          (fareDirection == MinFareFareSelection::OUTBOUND && qualifyBoardPoint &&
           matchedDefaultItem->spclHipTicketedPt() == YES) ||
          (fareDirection == MinFareFareSelection::INBOUND && qualifyOffPoint &&
           matchedDefaultItem->spclHipTicketedPt() == YES))
      {
        return true;
      }
    }
    else // use application table
    {
      if ((matchedApplItem.spclHipOrigNationTktPt() == BLANK) &&
          (matchedApplItem.spclHipTicketedPt() == BLANK))
        return false;

      if ((matchedApplItem.spclHipOrigNationTktPt() == NO) &&
          (matchedApplItem.spclHipTicketedPt() == NO))
        return false;

      // check intermediate fare board point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if (sameOrigNation && (matchedApplItem.spclHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameOrigNation && (matchedApplItem.spclHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }
      else
      {
        if (sameDestNation && (matchedApplItem.spclHipOrigNationTktPt() == YES))
          qualifyBoardPoint = true;
        else if (sameDestNation && (matchedApplItem.spclHipOrigNationStopPt() == YES) &&
                 MinimumFare::isStopOver(itin,
                                         fareUsage,
                                         **(travelBoardIter - 1),
                                         **travelBoardIter,
                                         isBoardSegAfterSurfaceAtFareBreak))
        {
          qualifyBoardPoint = true;
        }
      }

      // check intermediate fare off point
      if (fareDirection == MinFareFareSelection::OUTBOUND)
      {
        if ((sameDestNation) && (matchedApplItem.spclHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameDestNation) && (matchedApplItem.spclHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }
      else
      {
        if ((sameOrigNation) && (matchedApplItem.spclHipOrigNationTktPt() == YES))
          qualifyOffPoint = true;
        else if ((sameOrigNation) && (matchedApplItem.spclHipOrigNationStopPt() == YES) &&
                 ((travelOffIter + 1) != travelSegs.end()) &&
                 isStopOver(itin,
                            fareUsage,
                            **travelOffIter,
                            **(travelOffIter + 1),
                            isOffSegFollowedBySurfaceAtFareBreak))
        {
          qualifyOffPoint = true;
        }
      }

      if ((qualifyBoardPoint && qualifyOffPoint) ||
          (fareDirection == MinFareFareSelection::OUTBOUND && qualifyBoardPoint &&
           matchedApplItem.spclHipTicketedPt() == YES) ||
          (fareDirection == MinFareFareSelection::INBOUND && qualifyOffPoint &&
           matchedApplItem.spclHipTicketedPt() == YES))
      {
        return true;
      }
    }
  }
  return false;
}

bool
MinimumFare::checkDomesticExclusion(const Itin& itin,
                                    const PaxTypeFare& paxTypeFare,
                                    const MinFareAppl* matchedApplItem,
                                    const MinFareDefaultLogic* matchedDefaultItem,
                                    std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                    std::vector<TravelSeg*>::const_iterator travelOffIter)
{
  std::vector<TravelSeg*> travelSegVec(travelBoardIter, travelOffIter + 1);
  if (travelSegVec.empty() || !MinFareLogic::isDomestic(itin, travelSegVec))
    return false;

  bool matchedLoc = true;
  bool matchedFareType = true;

  if (matchedDefaultItem != nullptr)
  {
    // Match Domestic Loc
    if (!matchedDefaultItem->domLoc().loc().empty())
    {
      matchedLoc = LocUtil::isInLoc((*travelBoardIter)->origin()->loc(),
                                    matchedDefaultItem->domLoc().locType(),
                                    matchedDefaultItem->domLoc().loc(),
                                    Vendor::SABRE,
                                    MANUAL);
    }

    // Match Thru Fare Type
    const std::vector<FareType>& fareTypes = matchedDefaultItem->fareTypes();
    if (!fareTypes.empty())
    {
      std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
      for (; fareTypeIter != fareTypes.end(); ++fareTypeIter)
      {
        if ((*fareTypeIter).empty() ||
            RuleUtil::matchFareType(*fareTypeIter, paxTypeFare.fcaFareType()))
          break;
      }

      matchedFareType = (fareTypeIter != fareTypes.end());
    }

    if (matchedDefaultItem->domExceptInd() == YES)
      matchedLoc = !matchedLoc;

    if (matchedDefaultItem->domFareTypeExcept() == YES)
      matchedFareType = !matchedFareType;

    if (matchedDefaultItem->domAppl() == EXCLUDE)
    {
      return matchedLoc && matchedFareType;
    }
    else if (LIKELY(matchedDefaultItem->domAppl() == INCLUDE))
    {
      return !(matchedLoc && matchedFareType);
    }
    else
    {
      return false;
    }
  }
  else if (matchedApplItem != nullptr)
  {
    // Match Domestic Loc
    if (!matchedApplItem->domLoc().loc().empty())
    {
      matchedLoc = LocUtil::isInLoc((*travelBoardIter)->origin()->loc(),
                                    matchedApplItem->domLoc().locType(),
                                    matchedApplItem->domLoc().loc(),
                                    Vendor::SABRE,
                                    MANUAL);
    }

    // Match Thru Fare Type
    const std::vector<FareType>& fareTypes = matchedApplItem->fareTypes();
    if (!fareTypes.empty())
    {
      std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
      for (; fareTypeIter != fareTypes.end(); ++fareTypeIter)
      {
        if ((*fareTypeIter).empty() ||
            RuleUtil::matchFareType(*fareTypeIter, paxTypeFare.fcaFareType()))
          break;
      }

      matchedFareType = (fareTypeIter != fareTypes.end());
    }

    if (matchedApplItem->domExceptInd() == YES)
      matchedLoc = !matchedLoc;

    if (matchedApplItem->domFareTypeExcept() == YES)
      matchedFareType = !matchedFareType;

    if (matchedApplItem->domAppl() == EXCLUDE)
    {
      return matchedLoc && matchedFareType;
    }
    else if (matchedApplItem->domAppl() == INCLUDE)
    {
      return !(matchedLoc && matchedFareType);
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool
MinimumFare::matchHIPOrigin(std::vector<TravelSeg*>::const_iterator travelBoardIter,
                            std::vector<TravelSeg*>::const_iterator travelOffIter,
                            std::vector<TravelSeg*>::const_iterator tvlBrdThruIter,
                            std::vector<TravelSeg*>::const_iterator tvlOffThruIter,
                            bool reverseFcOrigin,
                            Indicator hipExemptInterToInter,
                            Indicator hipOrigInd,
                            Indicator hipOrigNationInd,
                            Indicator hipFromInterInd)
{
  if ((hipExemptInterToInter == YES) && (travelBoardIter != tvlBrdThruIter) &&
      (travelOffIter != tvlOffThruIter))
    return false;

  if ((hipOrigInd == YES) && ((!reverseFcOrigin && (travelBoardIter == tvlBrdThruIter)) ||
                              (reverseFcOrigin && (travelOffIter == tvlOffThruIter))))
    return true;

  if ((hipOrigNationInd == YES) &&
      ((!reverseFcOrigin &&
        ((**travelBoardIter).origin()->nation() == (**tvlBrdThruIter).origin()->nation())) ||
       (reverseFcOrigin &&
        ((**travelOffIter).destination()->nation() == (**tvlOffThruIter).destination()->nation()))))
    return true;

  if ((hipFromInterInd == YES) && ((!reverseFcOrigin && (travelBoardIter != tvlBrdThruIter)) ||
                                   (reverseFcOrigin && (travelOffIter != tvlOffThruIter))))
    return true;

  return false;
}

bool
MinimumFare::matchHIPDestination(std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                 std::vector<TravelSeg*>::const_iterator travelOffIter,
                                 std::vector<TravelSeg*>::const_iterator tvlBrdThruIter,
                                 std::vector<TravelSeg*>::const_iterator tvlOffThruIter,
                                 bool reverseFcOrigin,
                                 Indicator hipDestInd,
                                 Indicator hipDestNationInd,
                                 Indicator hipToInterInd)
{
  if ((hipDestInd == YES) && ((!reverseFcOrigin && (travelOffIter == tvlOffThruIter)) ||
                              (reverseFcOrigin && (travelBoardIter == tvlBrdThruIter))))
    return true;

  if ((hipDestNationInd == YES) &&
      ((!reverseFcOrigin && ((**travelOffIter).destination()->nation() ==
                             (**tvlOffThruIter).destination()->nation())) ||
       (reverseFcOrigin &&
        ((**travelBoardIter).origin()->nation() == (**tvlBrdThruIter).origin()->nation()))))
    return true;

  if ((hipToInterInd == YES) && ((!reverseFcOrigin && (travelOffIter != tvlOffThruIter)) ||
                                 (reverseFcOrigin && (travelBoardIter != tvlBrdThruIter))))
    return true;

  return false;
}

bool
MinimumFare::checkIntermediateCityPair(
    const MinimumFareModule module,
    const MinFareAppl& matchedApplItem,
    const MinFareDefaultLogic* defaultApplItem,
    const bool& selectNormalFare,
    const std::vector<TravelSeg*>& thruFareTravelSegs,
    const Itin& itin,
    std::vector<TravelSeg*>::const_iterator travelBoardIter,
    std::vector<TravelSeg*>::const_iterator travelOffIter,
    const MinFareFareSelection::FareDirectionChoice fareDirection)
{
  std::vector<TravelSeg*>::const_iterator tvlBrdThruIter = thruFareTravelSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlOffThruIter = thruFareTravelSegs.end() - 1;
  bool matchInd = false;

  if ((module == HIP) || (module == BHC))
  {
    bool reverseFcOrigin = (fareDirection == MinFareFareSelection::INBOUND);

    if (selectNormalFare)
    {
      if ((matchedApplItem.applyDefaultLogic() == YES) && (defaultApplItem != nullptr))
      {
        matchInd = matchHIPOrigin(travelBoardIter,
                                  travelOffIter,
                                  tvlBrdThruIter,
                                  tvlOffThruIter,
                                  reverseFcOrigin,
                                  defaultApplItem->nmlHipExemptInterToInter(),
                                  defaultApplItem->nmlHipOrigInd(),
                                  defaultApplItem->nmlHipOrigNationInd(),
                                  defaultApplItem->nmlHipFromInterInd());

        if (!matchInd)
          return false;

        // Check Destination point
        matchInd = matchHIPDestination(travelBoardIter,
                                       travelOffIter,
                                       tvlBrdThruIter,
                                       tvlOffThruIter,
                                       reverseFcOrigin,
                                       defaultApplItem->nmlHipDestInd(),
                                       defaultApplItem->nmlHipDestNationInd(),
                                       defaultApplItem->nmlHipToInterInd());
      }
      else // use data from application table
      {
        matchInd = matchHIPOrigin(travelBoardIter,
                                  travelOffIter,
                                  tvlBrdThruIter,
                                  tvlOffThruIter,
                                  reverseFcOrigin,
                                  matchedApplItem.nmlHipExemptInterToInter(),
                                  matchedApplItem.nmlHipOrigInd(),
                                  matchedApplItem.nmlHipOrigNationInd(),
                                  matchedApplItem.nmlHipFromInterInd());

        if (!matchInd)
          return false;

        // Check Destination point
        matchInd = matchHIPDestination(travelBoardIter,
                                       travelOffIter,
                                       tvlBrdThruIter,
                                       tvlOffThruIter,
                                       reverseFcOrigin,
                                       matchedApplItem.nmlHipDestInd(),
                                       matchedApplItem.nmlHipDestNationInd(),
                                       matchedApplItem.nmlHipToInterInd());
      }
    } // end if normal fare
    else // check special fare check point
    {
      if ((matchedApplItem.applyDefaultLogic() == YES) && (defaultApplItem != nullptr))
      {
        matchInd = matchHIPOrigin(travelBoardIter,
                                  travelOffIter,
                                  tvlBrdThruIter,
                                  tvlOffThruIter,
                                  reverseFcOrigin,
                                  defaultApplItem->spclHipExemptInterToInter(),
                                  defaultApplItem->spclHipOrigInd(),
                                  defaultApplItem->spclHipOrigNationInd(),
                                  defaultApplItem->spclHipFromInterInd());

        if (!matchInd)
          return false;

        // Check Destination point
        matchInd = matchHIPDestination(travelBoardIter,
                                       travelOffIter,
                                       tvlBrdThruIter,
                                       tvlOffThruIter,
                                       reverseFcOrigin,
                                       defaultApplItem->spclHipDestInd(),
                                       defaultApplItem->spclHipDestNationInd(),
                                       defaultApplItem->spclHipToInterInd());
      }
      else // use data from application table
      {
        matchInd = matchHIPOrigin(travelBoardIter,
                                  travelOffIter,
                                  tvlBrdThruIter,
                                  tvlOffThruIter,
                                  reverseFcOrigin,
                                  matchedApplItem.spclHipExemptInterToInter(),
                                  matchedApplItem.spclHipOrigInd(),
                                  matchedApplItem.spclHipOrigNationInd(),
                                  matchedApplItem.spclHipFromInterInd());

        if (!matchInd)
          return false;

        // Check Destination point
        matchInd = matchHIPDestination(travelBoardIter,
                                       travelOffIter,
                                       tvlBrdThruIter,
                                       tvlOffThruIter,
                                       reverseFcOrigin,
                                       matchedApplItem.spclHipDestInd(),
                                       matchedApplItem.spclHipDestNationInd(),
                                       matchedApplItem.spclHipToInterInd());
      }
    }
  } // end if hip/bhc
  else if (module == CTM)
  {
    if (selectNormalFare)
    {
      if ((matchedApplItem.applyDefaultLogic() == YES) && (defaultApplItem != nullptr))
      {
        // Check Destination point
        matchInd = false;

        if ((defaultApplItem->nmlCtmDestNationInd() == YES) &&
            ((**travelOffIter).destination()->nation() ==
             (**tvlOffThruIter).destination()->nation()))
        {
          matchInd = true;
        }
        else
        {
          if ((defaultApplItem->nmlCtmToInterInd() == YES) &&
              (itin.segmentOrder(*travelOffIter) != itin.segmentOrder(*tvlOffThruIter)))
          {
            matchInd = true;
          }
        }
      }
      else // use data from application table
      {
        // Check Destination point
        matchInd = false;

        if ((matchedApplItem.nmlCtmDestNationInd() == YES) &&
            ((**travelOffIter).destination()->nation() ==
             (**tvlOffThruIter).destination()->nation()))
        {
          matchInd = true;
        }
        else
        {
          if ((matchedApplItem.nmlCtmToInterInd() == YES) &&
              (itin.segmentOrder(*travelOffIter) != itin.segmentOrder(*tvlOffThruIter)))
          {
            matchInd = true;
          }
        }
      }
    } // end if normal fare
    else // check special fare check point
    {
      if ((matchedApplItem.applyDefaultLogic() == YES) && (defaultApplItem != nullptr))
      {
        // Check Destination point
        matchInd = false;

        if ((defaultApplItem->spclCtmDestNationInd() == YES) &&
            ((**travelOffIter).destination()->nation() ==
             (**tvlOffThruIter).destination()->nation()))
        {
          matchInd = true;
        }
        else
        {
          if ((defaultApplItem->spclCtmToInterInd() == YES) &&
              (itin.segmentOrder(*travelOffIter) != itin.segmentOrder(*tvlOffThruIter)))
          {
            matchInd = true;
          }
        }
      }
      else // use data from application table
      {
        // Check Destination point
        matchInd = false;

        if ((matchedApplItem.spclCtmDestNationInd() == YES) &&
            ((**travelOffIter).destination()->nation() ==
             (**tvlOffThruIter).destination()->nation()))
        {
          matchInd = true;
        }
        else
        {
          if ((matchedApplItem.spclCtmToInterInd() == YES) &&
              (itin.segmentOrder(*travelOffIter) != itin.segmentOrder(*tvlOffThruIter)))
          {
            matchInd = true;
          }
        }
      }
    }
  }

  return matchInd;
}

bool
MinimumFare::isStopOver(const Itin& itin,
                        const FareUsage* fareUsage,
                        const TravelSeg& travelSeg,
                        const TravelSeg& nextTravelSeg,
                        bool isNextTravelSegSurfaceAtFareBreak)
{
  if (isNextTravelSegSurfaceAtFareBreak)
    return true;

  if (UNLIKELY(travelSeg.isForcedStopOver()))
    return true;

  if (UNLIKELY(travelSeg.isForcedConx()))
    return false;

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    return nextTravelSeg.isStopOver(&travelSeg, itin.geoTravelType());
  }
  else
  {
    if (LIKELY(fareUsage))
    {
      const TimeAndUnit& minTime = fareUsage->stopoverMinTime();
      return nextTravelSeg.isStopOver(&travelSeg, itin.geoTravelType(), minTime);
    }
  }
  return false;
}

bool
MinimumFare::checkIntermediateStop(const Itin& itin,
                                   const FareUsage* fareUsage,
                                   const std::vector<TravelSeg*>& thruFareTravelSegs,
                                   std::vector<TravelSeg*>::const_iterator travelBoardIter,
                                   std::vector<TravelSeg*>::const_iterator travelOffIter,
                                   bool isBoardSegAfterSurfaceAtFareBreak,
                                   bool isOffSegFollowedBySurfaceAtFareBreak)
{
  if (UNLIKELY(std::find(thruFareTravelSegs.begin(), thruFareTravelSegs.end(), *travelBoardIter) ==
          thruFareTravelSegs.end() ||
      std::find(thruFareTravelSegs.begin(), thruFareTravelSegs.end(), *travelOffIter) ==
          thruFareTravelSegs.end()))
  {
    return false;
  }

  std::vector<TravelSeg*>::const_iterator tvlBrdThruIter = thruFareTravelSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlOffThruIter = thruFareTravelSegs.end() - 1;

  if (UNLIKELY((itin.segmentOrder(*travelOffIter) > itin.segmentOrder(*tvlOffThruIter)) ||
      (itin.segmentOrder(*travelBoardIter) < itin.segmentOrder(*tvlBrdThruIter))))
    return false;

  if (itin.segmentOrder(*travelBoardIter) == itin.segmentOrder(*tvlBrdThruIter))
  {
    if ((travelOffIter + 1) != thruFareTravelSegs.end())
    {
      if (isStopOver(itin,
                     fareUsage,
                     **travelOffIter,
                     **(travelOffIter + 1),
                     isOffSegFollowedBySurfaceAtFareBreak))
      {
        return true;
      }
    }
  }
  else if (itin.segmentOrder(*travelOffIter) == itin.segmentOrder(*tvlOffThruIter))
  {
    if (isStopOver(itin,
                   fareUsage,
                   **(travelBoardIter - 1),
                   **travelBoardIter,
                   isBoardSegAfterSurfaceAtFareBreak))
    {
      return true;
    }
  }
  else
  {
    if (isStopOver(
            itin, fareUsage, **(travelBoardIter - 1), **travelBoardIter) && // Check the Board Point
        ((travelOffIter + 1) != thruFareTravelSegs.end()) &&
        isStopOver(itin,
                   fareUsage,
                   **travelOffIter,
                   **(travelOffIter + 1),
                   isOffSegFollowedBySurfaceAtFareBreak)) // Check the Off Point
    {
      return true;
    }
  }

  return false;
}

void
MinimumFare::processIntermediate(const MinimumFareModule module,
                                 const MinFareFareSelection::EligibleFare eligibleFare,
                                 PricingTrx& trx,
                                 const Itin& itin,
                                 const PaxTypeFare& paxTypeFare,
                                 const PaxType& requestedPaxType,
                                 DiagCollector* diag,
                                 bool selectNormalFare,
                                 const MinFareFareSelection::FareDirectionChoice selectOutBound,
                                 std::set<uint32_t>& normalExemptSet,
                                 MinFarePlusUpItem& curPlusUp,
                                 const FarePath* farePath,
                                 const PricingUnit* pu,
                                 FareUsage* fu,
                                 const PaxTypeCode& actualPaxTypeCode,
                                 bool reverseFcOrigin)
{
  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
  const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
  std::vector<TravelSeg*>::const_iterator travelBoardIter = travelSegVec.begin();
  std::vector<TravelSeg*>::const_iterator travelOffIter;
  std::vector<TravelSeg*>::const_iterator travelEndIter = travelSegVec.end();
  const PaxTypeFare* paxTypeFareBase = nullptr;

  // Special check for HIP Special.
  PaxTypeStatus paxTypeStatus = PAX_TYPE_STATUS_UNKNOWN;

  if (module == HIP && paxTypeFare.isSpecial())
  {
    // If thru fare is ADT, then ADT fare should be used even though
    // INF or CNN fare is requested.
    if (LIKELY(paxTypeFare.paxType()))
      paxTypeStatus =
          PaxTypeUtil::paxTypeStatus(paxTypeFare.paxType()->paxType(), paxTypeFare.vendor());
    else if (farePath && farePath->paxType())
      paxTypeStatus =
          PaxTypeUtil::paxTypeStatus(farePath->paxType()->paxType(), paxTypeFare.vendor());
  }

  for (; travelBoardIter != travelEndIter; ++travelBoardIter)
  {
    if ((module == BHC) && reverseFcOrigin && (travelBoardIter == travelSegVec.begin()))
    {
      continue; // BHC only check from point of real fare origin to intermediate point
    }

    std::vector<LocCode> transitLocs;

    for (travelOffIter = travelBoardIter; travelOffIter != travelEndIter; ++travelOffIter)
    {
      if ((module == BHC) && reverseFcOrigin && (travelOffIter != (travelEndIter - 1)))
      {
        continue; // BHC only check from point of real fare origin to intermediate point
      }

      transitLocs.push_back((*travelOffIter)->boardMultiCity());

      std::vector<TravelSeg*> travelSegs(travelBoardIter, travelOffIter + 1);

      if ((dynamic_cast<AirSeg*>(*travelBoardIter) == nullptr) &&
          (dynamic_cast<AirSeg*>(*travelOffIter) == nullptr))
      {
        continue; // when both are not airseg
      }

      if ((travelBoardIter == travelSegVec.begin()) && (travelOffIter == travelEndIter - 1))
      {
        continue; // when both are the thru fare
      }

      if ((module != HIP) && (module != BHC)) // Ignore retransit for HIP and BHC
      {
        // Check retransit exempt.
        if ((travelSegs.size() > 1) &&
            (std::find(transitLocs.begin(), transitLocs.end(), (*travelOffIter)->offMultiCity()) !=
             transitLocs.end()))
        {
          // The destination is retransit point
          if (UNLIKELY(diag))
            printExceptionInfo(*diag, travelSegs, selectOutBound, "EXEMPT BY RE-TRANSIT\n");

          if ((*travelOffIter)->offMultiCity() == (*travelBoardIter)->boardMultiCity())
          {
            // From now on, the rest of tkt points won't have fare market from the the intermediate
            // origin because of retransit board point
            break;
          }
          else
          {
            continue;
          }
        }
      }

      if (checkDomesticExclusion(itin,
                                 paxTypeFare,
                                 _matchedApplItem,
                                 _matchedDefaultItem,
                                 travelBoardIter,
                                 travelOffIter))
      {
        if (UNLIKELY(diag))
          printExceptionInfo(*diag, travelSegs, selectOutBound, "EXEMPT BY DOMESTIC\n");

        continue;
      }

      bool isBoardSegAfterSurfaceAtFareBreak = false;
      bool isBoardSegAtFareBreak = (travelBoardIter == travelSegVec.begin());
      if (isBoardSegAtFareBreak == false)
      {
        bool isBoardSegSecondToFareBreak = (travelBoardIter == (travelSegVec.begin() + 1));
        if (isBoardSegSecondToFareBreak && (dynamic_cast<AirSeg*>(*(travelSegVec.begin())) == nullptr))
          isBoardSegAfterSurfaceAtFareBreak = true;
      }

      bool isOffSegFollowedBySurfaceAtFareBreak = false;
      bool isOffSegAtFareBreak = (travelOffIter == (travelEndIter - 1));
      if (isOffSegAtFareBreak == false)
      {
        bool isOffSegSecondToFareBreak = (travelOffIter == (travelEndIter - 2));
        if (isOffSegSecondToFareBreak && (dynamic_cast<AirSeg*>(*(travelEndIter - 1)) == nullptr))
          isOffSegFollowedBySurfaceAtFareBreak = true;
      }

      if (checkIntermediateExclusion(trx,
                                     module,
                                     *_matchedApplItem,
                                     _matchedDefaultItem,
                                     selectNormalFare,
                                     travelSegVec,
                                     itin,
                                     pu,
                                     fu,
                                     selectOutBound,
                                     travelBoardIter,
                                     travelOffIter,
                                     isBoardSegAfterSurfaceAtFareBreak,
                                     isOffSegFollowedBySurfaceAtFareBreak,
                                     reverseFcOrigin))
      {
        if (UNLIKELY(diag))
        {
          printExceptionInfo(
              *diag, travelSegs, selectOutBound, "EXEMPT BY INTERM. LOC. EXCLUSION APPL. TABLE ");

          *diag << _matchedApplItem->seqNo() << '\n';
        }
        continue;
      }
      if ((module == HIP) || (module == BHC) || (module == CTM))
      {
        if (!(checkIntermediateCityPair(module,
                                        *_matchedApplItem,
                                        _matchedDefaultItem,
                                        selectNormalFare,
                                        travelSegVec,
                                        itin,
                                        travelBoardIter,
                                        travelOffIter,
                                        selectOutBound)))
        {
          if (UNLIKELY(diag))
          {
            printExceptionInfo(*diag, travelSegs, selectOutBound);
            if ((_matchedApplItem->applyDefaultLogic() == YES) && (_matchedDefaultItem != nullptr))
            {
              *diag << "EXEMPT BY DETAIL COMPO. CHK IN DEFAULT LOGIC TABLE "
                    << _matchedDefaultItem->seqNo() << '\n';
            }
            else
            {
              *diag << "EXEMPT BY DETAIL COMPO CHK IN APPL. TABLE " << _matchedApplItem->seqNo()
                    << '\n';
            }
          }
          continue;
        }
      }

      // Check if BHC should be exempted by normal HIP checked
      if (module == BHC && paxTypeFare.isSpecial())
      {
        std::vector<TravelSeg*> tvlSeg(travelBoardIter, travelOffIter + 1);
        if (normalExemptSet.find(TravelSegUtil::hashCode(tvlSeg)) != normalExemptSet.end())
          continue;
      }

      if (module == HIP && paxTypeFare.isSpecial())
      {
        const PaxTypeFare* nmlInterm = nullptr;

        if (((_matchedApplItem->applyDefaultLogic() == YES &&
              _matchedDefaultItem->spclHipSpclOnlyInd() == YES) ||
             (_matchedApplItem->applyDefaultLogic() == NO &&
              _matchedApplItem->spclHipSpclOnlyInd() == YES)) &&
            matchSpecialGeo(trx, travelBoardIter, travelOffIter))
        {
          _spclHipSpclOnly = true;
        }
        else if (checkNormalExempt(module,
                                   eligibleFare,
                                   trx,
                                   itin,
                                   paxTypeFare,
                                   paxTypeFareBase,
                                   nmlInterm,
                                   requestedPaxType,
                                   travelBoardIter,
                                   travelOffIter,
                                   farePath,
                                   pu,
                                   paxTypeStatus))
        {
          std::vector<TravelSeg*> tvlSeg(travelBoardIter, travelOffIter + 1);

          normalExemptSet.insert(TravelSegUtil::hashCode(tvlSeg));

          if (UNLIKELY(diag))
          {
            printExceptionInfo(*diag, travelSegs, MinFareFareSelection::OUTBOUND);

            if (_matchedApplItem->applyDefaultLogic() == YES)
            {
              *diag << "EXEMPT BY NORMAL - DEFAULT TABLE " << _matchedDefaultItem->seqNo() << '\n';
            }
            else
            {
              *diag << "EXEMPT BY NORMAL - APPL TABLE " << _matchedApplItem->seqNo() << '\n';
            }

            if (paxTypeFareBase && nmlInterm)
            {
              // Display the normal fare comparision.
              *diag << "    COMPARE"
                    << "  INTER/N/" << nmlInterm->fcaFareType() << "-" << nmlInterm->cabin()
                    << " NUC" << nmlInterm->nucFareAmount() << "  THRU/N/"
                    << paxTypeFareBase->fcaFareType() << "-" << paxTypeFareBase->cabin() << " NUC"
                    << paxTypeFareBase->nucFareAmount() << "\n";
            }
          }
          continue;
        }
      } // end if HIP and special fare

      PtfPair ptfPair = MinFareLogic::selectQualifyConstFare(module,
                                                             trx,
                                                             itin,
                                                             paxTypeFare,
                                                             requestedPaxType,
                                                             paxTypeFare.cabin(),
                                                             selectNormalFare,
                                                             selectOutBound,
                                                             eligibleFare,
                                                             travelSegs,
                                                             _travelDate,
                                                             _matchedApplItem,
                                                             _matchedDefaultItem,
                                                             nullptr,
                                                             farePath,
                                                             pu,
                                                             actualPaxTypeCode);

      if (ptfPair.first != nullptr && ptfPair.second == nullptr)
      {
        checkNigeriaCurrencyAdjustment(
            diag,
            module,
            curPlusUp,
            trx,
            itin,
            farePath,
            pu,
            fu,
            *ptfPair.first,
            requestedPaxType,
            paxTypeFare.cabin(),
            selectNormalFare,
            selectOutBound,
            eligibleFare,
            travelSegs,
            _matchedApplItem,
            _matchedDefaultItem,
            ((paxTypeFare.isSpecial() && selectNormalFare && (paxTypeFareBase != nullptr))
                 ? paxTypeFareBase
                 : nullptr));
      }
      else if (ptfPair.first != nullptr)
      {
        if (UNLIKELY(diag))
          printFareInfo(ptfPair, selectOutBound, *diag, module, "  ");

        if (fu && fu->adjustedPaxTypeFare())
        {
          savePlusUpInfo(*fu->adjustedPaxTypeFare(),
                         ptfPair,
                         curPlusUp,
                         itin.useInternationalRounding(),
                         (selectOutBound == MinFareFareSelection::OUTBOUND));
        }
        else
        {
          savePlusUpInfo(((paxTypeFare.isSpecial() && selectNormalFare && (paxTypeFareBase != nullptr))
                              ? *paxTypeFareBase
                              : paxTypeFare),
                         ptfPair,
                         curPlusUp,
                         itin.useInternationalRounding(),
                         (selectOutBound == MinFareFareSelection::OUTBOUND));
        }
      }
      else if (fu && travelSegs.size() > 0)
      {
        if (UNLIKELY(diag))
          printExceptionInfo(*diag, travelSegs, selectOutBound, "NO FARE FOUND\n");
      }

    } // for loop for off point travel seg

    if ((module == BHC) && !reverseFcOrigin)
    {
      break; // BHC only do from point of fare origin
    }

  } // for loop for board point travel seg
}

bool
MinimumFare::checkNormalExempt(const MinimumFareModule module,
                               const MinFareFareSelection::EligibleFare eligibleFare,
                               PricingTrx& trx,
                               const Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               const PaxTypeFare*& paxTypeFareBase,
                               const PaxTypeFare*& paxTypeFareInterm,
                               const PaxType& requestedPaxType,
                               std::vector<TravelSeg*>::const_iterator travelBoardIter,
                               std::vector<TravelSeg*>::const_iterator travelOffIter,
                               const FarePath* farePath,
                               const PricingUnit* pu,
                               PaxTypeStatus selPaxTypeStatus)

{

  MinFareFareSelection::FareDirectionChoice selectOutBound = paxTypeFare.directionality() == FROM
                                                                 ? MinFareFareSelection::OUTBOUND
                                                                 : MinFareFareSelection::INBOUND;

  std::vector<PricingUnit*> pricingUnits;
  if (pu != nullptr)
    pricingUnits.push_back(const_cast<PricingUnit*>(pu));

  if (paxTypeFareBase == nullptr)
  {
    // Get thru normal fare
    MinFareNormalFareSelection fareSel(module,
                                       eligibleFare,
                                       selectOutBound,
                                       paxTypeFare.cabin(),
                                       trx,
                                       itin,
                                       paxTypeFare.fareMarket()->travelSeg(),
                                       pricingUnits,
                                       &requestedPaxType,
                                       _travelDate,
                                       farePath,
                                       &paxTypeFare,
                                       _matchedApplItem,
                                       _matchedDefaultItem);
    paxTypeFareBase = fareSel.selectFare(selPaxTypeStatus);
    if (paxTypeFareBase == nullptr)
      return false;
  }

  // Get intermediate normal fare
  std::vector<TravelSeg*> intermediateTravelSegs(travelBoardIter, travelOffIter + 1);

  MinFareNormalFareSelection fareSel(module,
                                     eligibleFare,
                                     selectOutBound,
                                     paxTypeFare.cabin(),
                                     trx,
                                     itin,
                                     intermediateTravelSegs,
                                     pricingUnits,
                                     &requestedPaxType,
                                     _travelDate,
                                     farePath,
                                     &paxTypeFare,
                                     _matchedApplItem,
                                     _matchedDefaultItem);
  paxTypeFareInterm = fareSel.selectFare(selPaxTypeStatus);
  if (paxTypeFareInterm == nullptr)
    return false;

  // See if we have HIP for normal fare
  if (paxTypeFareInterm->nucFareAmount() <=
      paxTypeFareBase->nucFareAmount()) // This should be the right one.
    return true;

  return false;
}

bool
MinimumFare::matchSpecialGeo(PricingTrx& trx,
                             std::vector<TravelSeg*>::const_iterator travelBoardIter,
                             std::vector<TravelSeg*>::const_iterator travelOffIter)
{
  std::vector<TravelSeg*> travelSegVec;
  copy(travelBoardIter, travelOffIter + 1, back_inserter(travelSegVec));

  if (_matchedApplItem->applyDefaultLogic() == YES)
  {
    if (isWithinLoc(trx,
                    travelSegVec,
                    _matchedDefaultItem->spclHipLoc().locType(),
                    _matchedDefaultItem->spclHipLoc().loc()))
      return true;
  }
  else
  {
    if (isWithinLoc(trx,
                    travelSegVec,
                    _matchedApplItem->spclHipLoc().locType(),
                    _matchedApplItem->spclHipLoc().loc()))
      return true;
  }

  return false;
}

void
MinimumFare::printExceptionInfo(const PaxTypeFare& paxTypeFare, DiagCollector& diag)
{
  const FareMarket* fareMarket = paxTypeFare.fareMarket();
  if (LIKELY(!fareMarket || !diag.isActive()))
  {
    return;
  }

  diag << "  " << fareMarket->boardMultiCity() << "-" << fareMarket->offMultiCity() << " /"
       << fareMarket->governingCarrier().c_str() << "/ ";
}

void
MinimumFare::printException(const PaxTypeFare& paxTypeFare, std::ostream& os)
{
  const FareMarket* fareMarket = paxTypeFare.fareMarket();

  if (fareMarket)
  {
    os << "  " << fareMarket->boardMultiCity() << "-" << fareMarket->offMultiCity() << " /"
       << fareMarket->governingCarrier().c_str() << "/ ";
  }
}

void
MinimumFare::printExceptionInfo(DiagCollector& diag,
                                const std::vector<TravelSeg*>& travelSegs,
                                const MinFareFareSelection::FareDirectionChoice direction,
                                const std::string& exceptionMsg)
{
  if (direction == MinFareFareSelection::OUTBOUND)
    printExceptionInfo(diag,
                       travelSegs.front()->boardMultiCity(),
                       travelSegs.back()->offMultiCity(),
                       exceptionMsg);
  else
    printExceptionInfo(diag,
                       travelSegs.back()->offMultiCity(),
                       travelSegs.front()->boardMultiCity(),
                       exceptionMsg);
}

void
MinimumFare::printExceptionInfo(DiagCollector& diag,
                                const LocCode& boardCity,
                                const LocCode& offCity,
                                const std::string& exceptionMsg)
{
  if (UNLIKELY(diag.isActive()))
  {
    diag << "  " << boardCity << "-" << offCity << " " << exceptionMsg;
  }
}

void
MinimumFare::printException(const TravelSeg& tvlBoard, const TravelSeg& tvlOff, std::ostream& os)
{
  os << "  " << tvlBoard.boardMultiCity() << "-" << tvlOff.offMultiCity() << " ";
}

const Loc*
MinimumFare::origin(const std::vector<TravelSeg*>& travelSegs)
{
  return (travelSegs.empty() ? nullptr : travelSegs.front()->origin());
}

const Loc*
MinimumFare::destination(const std::vector<TravelSeg*>& travelSegs)
{
  return (travelSegs.empty() ? nullptr : travelSegs.back()->destination());
}

bool
MinimumFare::savePlusUpInfo(const PaxTypeFare& paxTypeFareThru,
                            const PtfPair& ptfPair,
                            MinFarePlusUpItem& curPlusUp,
                            bool useInternationalRounding /* = false */,
                            bool outbound /* = true */)
{
  if (ptfPair.first && ptfPair.second)
  {
    return compareAndSaveFare(
        paxTypeFareThru, ptfPair, curPlusUp, useInternationalRounding, outbound);
  }
  else if (ptfPair.first)
  {
    return compareAndSaveFare(
        paxTypeFareThru, *ptfPair.first, curPlusUp, useInternationalRounding, outbound);
  }

  return false;
}

void
MinimumFare::checkNigeriaCurrencyAdjustment(DiagCollector* diag,
                                            MinimumFareModule module,
                                            MinFarePlusUpItem& curPlusUp,
                                            PricingTrx& trx,
                                            const Itin& itin,
                                            const FarePath* farePath,
                                            const PricingUnit* pu,
                                            const FareUsage* thruFareUsage,
                                            const PaxTypeFare& interPaxTypeFare,
                                            const PaxType& requestedPaxType,
                                            const CabinType& cabin,
                                            bool selectNormalFare,
                                            MinFareFareSelection::FareDirectionChoice fareDirection,
                                            MinFareFareSelection::EligibleFare eligibleFare,
                                            const std::vector<TravelSeg*>& travelSegs,
                                            const MinFareAppl* matchedApplItem,
                                            const MinFareDefaultLogic* matchedDefaultItem,
                                            const PaxTypeFare* nmlThruFare)
{
  // Additional reverse fare look up for Nigeria:
  if ((pu != nullptr && pu->nigeriaCurrencyAdjustment() == true) &&
      (((fareDirection == MinFareFareSelection::OUTBOUND) &&
        (travelSegs.front()->origin()->nation() == NIGERIA)) ||
       ((fareDirection == MinFareFareSelection::INBOUND) &&
        (travelSegs.back()->destination()->nation() == NIGERIA))))
  {
    MinFareFareSelection::FareDirectionChoice revDirection =
        (fareDirection == MinFareFareSelection::OUTBOUND ? MinFareFareSelection::INBOUND
                                                         : MinFareFareSelection::OUTBOUND);

    // Select the reverse Nigeria fare for comparison
    const PaxTypeFare* ngPaxTypeFare = MinFareLogic::selectQualifyFare(NCJ,
                                                                       trx,
                                                                       itin,
                                                                       interPaxTypeFare,
                                                                       requestedPaxType,
                                                                       cabin,
                                                                       selectNormalFare,
                                                                       revDirection,
                                                                       eligibleFare,
                                                                       travelSegs,
                                                                       _travelDate,
                                                                       matchedApplItem,
                                                                       matchedDefaultItem,
                                                                       nullptr,
                                                                       farePath,
                                                                       pu);

    MoneyAmount ngAdjustment = 0;
    const PaxTypeFare* ngInterFare = &interPaxTypeFare;

    if (ngPaxTypeFare != nullptr)
    {
      ngAdjustment = ngPaxTypeFare->nucFareAmount() - interPaxTypeFare.nucFareAmount();

      if (ngAdjustment > 0)
      {
        ngInterFare = ngPaxTypeFare;

        if (farePath && farePath->trailerCurrAdjMsg().empty())
        {
          (const_cast<FarePath*>(farePath))->trailerCurrAdjMsg() =
              "NIGERIA CURRENCY ADJUSTMENT APPLIED ";
        }
      }
    }

    const PaxTypeFare* ngThruFare = nullptr;
    if (thruFareUsage != nullptr)
    {
      if (thruFareUsage->adjustedPaxTypeFare() != nullptr)
      {
        ngThruFare = thruFareUsage->adjustedPaxTypeFare();
      }
      else
      {
        ngThruFare = ((nmlThruFare != nullptr) ? nmlThruFare : thruFareUsage->paxTypeFare());
      }
    }

    if (UNLIKELY(diag))
      printFareInfo(
          interPaxTypeFare, *diag, module, "  ", false, (ngAdjustment > 0 ? ngAdjustment : 0));

    compareAndSaveFare(*ngThruFare,
                       *ngInterFare,
                       curPlusUp,
                       itin.useInternationalRounding(),
                       (fareDirection == MinFareFareSelection::OUTBOUND));
  }
  else if (thruFareUsage != nullptr)
  {
    if (UNLIKELY(diag))
      printFareInfo(interPaxTypeFare, *diag, module, "  ", false);

    compareAndSaveFare(((nmlThruFare != nullptr) ? *nmlThruFare : *thruFareUsage->paxTypeFare()),
                       interPaxTypeFare,
                       curPlusUp,
                       itin.useInternationalRounding(),
                       (fareDirection == MinFareFareSelection::OUTBOUND));
  }
}

void
MinimumFare::adjustRexPricingDates(PricingTrx& trx, const DateTime& retrievalDate)
{
  trx.dataHandle().setTicketDate(retrievalDate);

  if (_travelDate < retrievalDate)
    _travelDate = retrievalDate;
}

void
MinimumFare::adjustPortExcDates(PricingTrx& trx)
{
  if (_travelDate < trx.getRequest()->ticketingDT())
    _travelDate = trx.getRequest()->ticketingDT();
}

void
MinimumFare::restorePricingDates(PricingTrx& trx, const DateTime& ticketDate)
{
  trx.dataHandle().setTicketDate(ticketDate);
}
} // namespace tse
