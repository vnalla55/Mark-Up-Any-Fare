//-------------------------------------------------------------------
//
//  File:        RoutingSection.cpp
//  Description: Inclusive construction for the routing section of
//               a FareDisplay response
//
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/RoutingSection.h"

#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RoutingUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Routing/MileageInfo.h"
#include "Routing/RoutingInfo.h"

#include <sstream>
#include <string>

namespace tse
{
namespace
{
// this value defines the Effective Date Indicator
static const std::string SINGLE_BLANK = " ";
static const uint16_t MAX_RTE_LEN_FQ = 53;

struct RestrictionRetriever
{
  RoutingRestriction* operator()(const RestrictionInfos::value_type& rest_pair) const
  {
    return const_cast<RoutingRestriction*>(rest_pair.first);
  }
} restrictionRetriever;
}

static Logger
logger("atseintl.FareDisplay.Templates.RoutingSection");

void
RoutingSection::buildDisplay()
{
  _trx.response() << " " << std::endl;

  // Is FL entry ?
  if (_trx.getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR &&
      (!FareDisplayUtil::isAxessUser(_trx))) // Disable this line for Axess User
  {
    _trx.response() << "ENTER RD LINE NUMBER *RTG TO VIEW ROUTING. EXAMPLE - RD4*RTG";
    _trx.response() << std::endl;
    return;
  }

  if (_trx.fdResponse()->uniqueRoutingMap().empty() &&
      (!FareDisplayUtil::isAxessUser(_trx))) // Disable this line for Axess User
  {
    if (_trx.isShopperRequest() && !_trx.allPaxTypeFare().empty())
    {
      _trx.response() << "ENTER RD LINE NUMBER *RTG TO VIEW ROUTING. EXAMPLE - RD4*RTG";
    }
    _trx.response() << std::endl;
    return;
  }

  if (_trx.getRequest()->requestedInclusionCode() == ADDON_FARES)
    buildADDisplay(_trx);
  else if (_trx.allPaxTypeFare().size() > 1 || !_trx.isRD())
    buildFDDisplay(_trx);
  else if (_trx.getOptions()->routingDisplay() == TRUE_INDICATOR)
    FareDisplayResponseUtil::buildRTGDisplay(_trx, *(_trx.allPaxTypeFare().front()));
}

bool
RoutingSection::buildFDDisplay(FareDisplayTrx& trx)
{
  for (const auto& rtgSeq2PaxTypeFareItem : trx.fdResponse()->uniqueRoutings())
  {
    PaxTypeFare* paxTypeFare = rtgSeq2PaxTypeFareItem.second;
    if (paxTypeFare->fareDisplayInfo() == nullptr)
    {
      LOG4CXX_WARN(logger,
                   "No FareDisplayInfo for routing number: " << rtgSeq2PaxTypeFareItem.first);
      continue;
    }

    bool hasConstructedInfo = paxTypeFare->hasConstructedRouting();
    bool hasConstructed = paxTypeFare->hasConstructedRouting();

    FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();

    // Get the RoutingInfo
    std::string rtgSeq;
    if (!fareDisplayInfo->routingSequence().empty())
      rtgSeq = fareDisplayInfo->routingSequence();
    else
      rtgSeq = paxTypeFare->routingNumber();

    if (displayAnyRoutingForFQ(
            trx, paxTypeFare->routingNumber(), rtgSeq, paxTypeFare->globalDirection()))
      continue;

    const auto uniqueRtgMapIter = trx.fdResponse()->uniqueRoutingMap().find(rtgSeq);
    if (uniqueRtgMapIter == trx.fdResponse()->uniqueRoutingMap().end())
    {
      LOG4CXX_WARN(logger, "No unique map for: " << fareDisplayInfo->routingSequence());
      continue;
    }
    RoutingInfo* routingInfo = (*uniqueRtgMapIter).second;
    if (routingInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty routingInfo for: " << fareDisplayInfo->routingSequence());
      continue;
    }
    else if (routingInfo->routing() == nullptr && routingInfo->origAddOnRouting() == nullptr &&
             routingInfo->destAddOnRouting() == nullptr &&
             paxTypeFare->routingNumber() != MILEAGE_ROUTING)
    {
      LOG4CXX_WARN(logger, "No routing data for: " << paxTypeFare->routingNumber());
      trx.response() << std::right << std::setfill(' ') << std::setw(4) << rtgSeq << "* ";
      trx.response() << " NO ROUTING INFORMATION\n";
      continue;
    }

    if (fareDisplayInfo->routingSequence().empty())
    {
      bool indentRest(true);
      bool mileageRestr = isMileageRestriction(routingInfo);
      int16_t routingNum = 0;
      trx.response() << FareDisplayResponseUtil::routingNumberToStringFormat(
                            paxTypeFare->routingNumber(), std::ios::right) << "* ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);

      if (routingInfo->mapInfo() != nullptr && !routingInfo->mapInfo()->routeStrings()->empty())
      {
        if (routingInfo->routing() == nullptr || routingInfo->routing()->rests().empty() ||
            !FareDisplayResponseUtil::isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
          FareDisplayResponseUtil::addRouteString(
              trx, routingInfo->mapInfo()->routeStrings(), false, true, true);
      }
      else
      {
        indentRest = false;
        LOG4CXX_INFO(logger, "Unable to get route string");
      }

      if ((trx.isDomestic() || trx.isForeignDomestic()) &&
          (FareDisplayResponseUtil::routingNumberToNumeric(paxTypeFare->routingNumber(),
                                                           routingNum) &&
           routingNum == 0 && !indentRest && mileageRestr))
      {
        FareDisplayResponseUtil::displayIncomplete(trx, true); // Domestic & 0 routing#
      }
      else if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
      {
        FareDisplayResponseUtil::addRestrictions(
            trx, routingInfo->routing()->rests(), indentRest, true);
      }
      else if (!indentRest) // there are no restrictions and no maps
      {
        FareDisplayResponseUtil::displayIncomplete(trx, true);
      }
    }
    else
    {
      trx.response() << fareDisplayInfo->routingSequence() << "* ";
      if (!FareDisplayResponseUtil::addGlobalDescription(trx, paxTypeFare->globalDirection(), true))
        LOG4CXX_INFO(logger, "Unable to get global direction desc");

      displayConstrVsPubl(trx, *paxTypeFare);
      RoutingNumber rtgNo = paxTypeFare->routingNumber();
      if (hasConstructedInfo && rtgNo.empty())
        rtgNo = paxTypeFare->fare()->constructedFareInfo()->fareInfo().routingNumber();

      addMPMvsRTG(trx, rtgNo, routingInfo, hasConstructed);
      if(FareDisplayResponseUtil::isIncomplete(*routingInfo))
      {
        FareDisplayResponseUtil::displayIncomplete(trx, true);
        continue;
      }

      RoutingRestrictions rests;
      if (routingInfo != nullptr && routingInfo->restrictions() != nullptr &&
          !routingInfo->restrictions()->empty())
      {
        transform(routingInfo->restrictions()->begin(),
                  routingInfo->restrictions()->end(),
                  back_inserter(rests),
                  restrictionRetriever);
      }

      if (!rests.empty())
        FareDisplayResponseUtil::addRestrictions(trx, rests, true, true);

      FareDisplayResponseUtil::displayTPD(trx, *routingInfo);
      FareDisplayResponseUtil::displayPSR(trx, *routingInfo);
      if (hasConstructed)
      {
        FareDisplayResponseUtil::displayConstructed(trx, *paxTypeFare, routingInfo, true);
      }
      else
      {
        if (routingInfo != nullptr && routingInfo->mapInfo() != nullptr)
        {
          if (!FareDisplayResponseUtil::isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
            FareDisplayResponseUtil::displayDRV(trx, routingInfo->routing(), *paxTypeFare, true);

          if (rests.empty() ||
              !FareDisplayResponseUtil::isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
            FareDisplayResponseUtil::addRouteString(
                trx, routingInfo->mapInfo()->routeStrings(), true, true, true);
        }
        else
        {
          LOG4CXX_INFO(logger, "Unable to get route string");
        }
      }
    }
  }
  return true;
}

bool
RoutingSection::buildADDisplay(FareDisplayTrx& trx)
{
  if (trx.fdResponse()->uniqueAddOnRoutings().empty())
    return false;
  for (const auto& addOnFareInfo : trx.fdResponse()->uniqueAddOnRoutings())
  {
    const FDAddOnFareInfo* fareInfo(addOnFareInfo.second);
    if (fareInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty fareInfo for: " << addOnFareInfo.first);
      continue;
    }
    std::string rtgSeq;
    if (!fareInfo->addOnRoutingSeq().empty())
      rtgSeq = fareInfo->addOnRoutingSeq();
    else
      rtgSeq = fareInfo->routing();
    if (displayAnyRoutingForFQ(trx, fareInfo->routing(), rtgSeq, fareInfo->globalDir()))
      continue;

    const auto i(trx.fdResponse()->uniqueRoutingMap().find(rtgSeq));
    if (i == trx.fdResponse()->uniqueRoutingMap().end())
    {
      LOG4CXX_WARN(logger, "No unique map for: " << fareInfo->addOnRoutingSeq());
      continue;
    }
    const RoutingInfo* routingInfo(i->second);
    if (routingInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty routingInfo for: " << fareInfo->addOnRoutingSeq());
      continue;
    }
    else if (routingInfo->routing() == nullptr && fareInfo->routing() != MILEAGE_ROUTING)
    {
      LOG4CXX_WARN(logger, "No routing data for: " << fareInfo->routing());
      trx.response() << std::right << std::setfill(' ') << std::setw(4) << rtgSeq << "* ";
      trx.response() << " NO ROUTING INFORMATION\n";
      continue;
    }
    if (fareInfo->addOnRoutingSeq().empty())
    {
      bool indentRest(true);
      int16_t routingNum = atoi(fareInfo->routing().c_str());
      trx.response().setf(std::ios::right, std::ios::adjustfield);
      trx.response().setf(std::ios::fixed, std::ios::floatfield);
      trx.response() << std::setfill(' ') << std::setw(4) << routingNum << "* ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);

      if (routingInfo->mapInfo() != nullptr)
      {
        if (routingInfo->routing() == nullptr || routingInfo->routing()->rests().empty() ||
            !FareDisplayResponseUtil::isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
          FareDisplayResponseUtil::addRouteString(
              trx, routingInfo->mapInfo()->routeStrings(), false, true, true);
      }
      else
      {
        indentRest = false;
        LOG4CXX_INFO(logger, "Unable to get route string");
      }
      if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
        FareDisplayResponseUtil::addRestrictions(
            trx, routingInfo->routing()->rests(), indentRest, true);
      else if (!indentRest) // there are no restrictions and no maps
        FareDisplayResponseUtil::displayIncomplete(trx, true);
    }
    else
    {
      trx.response() << fareInfo->addOnRoutingSeq() << "* ";
      if (!FareDisplayResponseUtil::addGlobalDescription(trx, fareInfo->globalDir(), true))
        LOG4CXX_INFO(logger, "Unable to get global direction desc");
      trx.response() << " PUBLISHED";
      addMPMvsRTG(trx, fareInfo->routing(), routingInfo, false);
      if (FareDisplayResponseUtil::isIncomplete(*routingInfo))
      {
        FareDisplayResponseUtil::displayIncomplete(trx, true);
        continue;
      }
      if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
        FareDisplayResponseUtil::addRestrictions(trx, routingInfo->routing()->rests(), true, true);
      FareDisplayResponseUtil::displayTPD(trx, *routingInfo);
      FareDisplayResponseUtil::displayPSR(trx, *routingInfo);
      if (routingInfo->mapInfo() != nullptr)
      {
        if (routingInfo->routing() == nullptr || routingInfo->routing()->rests().empty() ||
            !FareDisplayResponseUtil::isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
          FareDisplayResponseUtil::addRouteString(
              trx, routingInfo->mapInfo()->routeStrings(), true, true, true);
      }
      else
        LOG4CXX_INFO(logger, "Unable to get route string");
    }
  }
  return true;
}

bool
RoutingSection::addMPMvsRTG(FareDisplayTrx& trx,
                            const RoutingNumber& routingNumber,
                            const RoutingInfo* info,
                            bool constructed) const
{
  if (info == nullptr)
  {
    return false;
  }

  const MileageInfo* mileage(info->mileageInfo());
  if (mileage != nullptr && routingNumber == MILEAGE_ROUTING)
  {
    trx.response() << SINGLE_BLANK << "MPM ";
    if (mileage->totalApplicableMPM() > 0)
      trx.response() << mileage->totalApplicableMPM();
    trx.response() << "\n";
  }
  else
  {
    trx.response() << " RTG ";
    if (!constructed && info->routing() != nullptr)
      trx.response() << FareDisplayResponseUtil::routingNumberToString(routingNumber);
    trx.response() << "\n";
  }
  return true;
}

void
RoutingSection::displayConstrVsPubl(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare) const
{
  if (paxTypeFare.hasConstructedRouting())
  {
    trx.response() << " CONSTRUCTED";
  }
  else
  {
    trx.response() << " PUBLISHED";
  }
}

bool
RoutingSection::isMileageRestriction(RoutingInfo* routingInfo) const
{
  if (routingInfo != nullptr && routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
  {
    const RoutingRestrictions& tempRests = routingInfo->routing()->rests();

    return std::any_of(tempRests.cbegin(),
                       tempRests.cend(),
                       [](const RoutingRestriction* const rr)
                       {
      return (rr->restriction() == RTW_ROUTING_RESTRICTION_12) ||
             (rr->restriction() == MILEAGE_RESTRICTION_16);
    });
  }

  return true; // No routing restrictions;   displayincomplete message
}

bool
RoutingSection::displayAnyRoutingForFQ(FareDisplayTrx& trx,
                                       const std::string& rtg,
                                       const std::string& seq,
                                       const GlobalDirection& gld)
{
  if (rtg != "SEVN" && rtg != "EIGH")
    return false;

  std::string gd;
  globalDirectionToStr(gd, gld);
  std::string rtgSeq = "ANY";

  if (rtg != seq)
    rtgSeq = seq;

  trx.response() << std::right << std::setfill(' ') << std::setw(4) << rtgSeq << "*";
  if (trx.isDomestic() || trx.isForeignDomestic())
  {
    if (gd.empty())
      trx.response() << " ANY VALID ROUTING APPLIES\n";
    else
      trx.response() << " ANY VALID " << gd << " ROUTING APPLIES\n";
  }
  else
  {
    if (rtg == "SEVN")
    {
      if (gd.empty())
        trx.response() << " ANY VALID ROUTING OR MPM VIA ANY GLOBAL DIR APPLIES\n";
      else
        trx.response() << " ANY VALID " << gd << " ROUTING OR MPM APPLIES\n";
    }
    else
    {
      // 88888
      if (gd.empty())
        trx.response() << " ANY VALID ROUTING EXCEPT MPM APPLIES\n";
      else
        trx.response() << " ANY VALID " << gd << " ROUTING EXCEPT MPM APPLIES\n";
    }
  }
  return true;
}
} // tse namespace
