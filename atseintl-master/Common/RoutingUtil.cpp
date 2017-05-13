//----------------------------------------------------------------------------
//  Description:    Common functions required for ATSE shopping/pricing.
//
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

#include "Common/RoutingUtil.h"

#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"

#include <vector>

namespace tse
{

static Logger
logger("atseintl.Common.RoutingUtil");

//------------------------------------------------------------
// RoutingUtil::getRoutingType()
// -----------------------------------------------------------

bool
RoutingUtil::getRoutingType(PaxTypeFareMap& pMap, PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " Collectiong Routing Fare Type for CAT 25");

  PaxTypeFareMapItr itr = pMap.begin();

  std::map<RtgKey, RoutingFareType> tmpMap; // temporary Map to keep track all the processed
                                            // PaxTypeFare
  std::map<RtgKey, RoutingFareType>::iterator pos;
  for (; itr != pMap.end(); itr++)
  {
    const Routing* routing = RoutingUtil::getRoutingData((*itr).first, trx);

    // Create Unique Key
    RtgKey rKey;

    if (routing == nullptr)
    {
      rKey.vendor() = (*itr).first->vendor();
      rKey.carrier() = (*itr).first->carrier();
      rKey.routingTariff() = (*itr).first->tcrRoutingTariff1();
      rKey.routingNumber() = (*itr).first->routingNumber();
    }
    else
    {
      rKey.vendor() = routing->vendor();
      rKey.carrier() = routing->carrier();
      rKey.routingTariff() = routing->routingTariff();
      rKey.routingNumber() = routing->routing();
    }

    pos = tmpMap.find(rKey);
    if (pos == tmpMap.end())
    {
      RoutingFareType rtType;
      if (routing)
        rtType = getRoutingType(routing);
      else if ((*itr).first->routingNumber() == MILEAGE_ROUTING)
        rtType = MILEAGE_FARE;
      else
        rtType = ROUTING_FARE;
      (itr)->second = rtType;
      tmpMap.insert(std::map<RtgKey, RoutingFareType>::value_type(rKey, rtType));
    }
    else
    {
      (itr)->second = (pos)->second;
    }
  }

  LOG4CXX_DEBUG(logger, " Leaving Collectiong Routing Fare Type for CAT 25");
  return true;
}

//------------------------------------------------------------
// RoutingUtil::getRoutingType()
// -----------------------------------------------------------

RoutingFareType
RoutingUtil::getRoutingType(const Routing* routing)
{
  LOG4CXX_DEBUG(logger, " Selecting Routing Type ");

  if (routing->routing() == MILEAGE_ROUTING)
  {
    LOG4CXX_DEBUG(logger, " Routing Type Selection Done : MILEAGE_FARE ");
    return MILEAGE_FARE;
  }

  else
  {
    if (routing)
    {
      bool noRestrictions = routing->rests().empty();
      bool noMaps = routing->rmaps().empty();
      if (noRestrictions && noMaps)
      {
        LOG4CXX_DEBUG(logger, " Routing Type Selection Done : Unknown Routing Type ");
        return UNKNOWN_ROUTING_FARE_TYPE;
      }

      else if (noRestrictions)
      {
        LOG4CXX_DEBUG(logger, " Routing Type Selection Done : ROUTING_FARE ");
        return ROUTING_FARE;
      }

      else if (noMaps)
      {
        std::vector<RoutingRestriction*>::const_iterator it = routing->rests().begin();
        for (; it != routing->rests().end(); it++)
        {
          if ((*it)->restriction() == RTW_ROUTING_RESTRICTION_12 ||
              (*it)->restriction() == MILEAGE_RESTRICTION_16)

          {
            LOG4CXX_DEBUG(logger, " Routing Type Selection Done : MILEAGE_FARE ");
            return MILEAGE_FARE;
          }
        }
        LOG4CXX_DEBUG(logger, " Routing Type Selection Done : ROUTING_FARE ");
        return ROUTING_FARE;
      }
      else
      {
        LOG4CXX_DEBUG(logger, " Routing Type Selection Done : ROUTING_FARE ");
        return ROUTING_FARE; // both Map and Restrictions present
      }
    }

    LOG4CXX_DEBUG(logger,
                  " Routing Data Not Found for the Fare, returning UNKNOWN_ROUTING_FARE_TYPE");
    return UNKNOWN_ROUTING_FARE_TYPE; // routing pointer is NULL, DATABASE issue!!
  }
}

RoutingFareType
RoutingUtil::getRoutingType(const PaxTypeFare* paxTypeFare, PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " GET ROUTING TYPE");
  const Routing* routing = RoutingUtil::getRoutingData(paxTypeFare, trx);

  if (routing != nullptr)
  {
    return getRoutingType(routing);
  }

  if (paxTypeFare->routingNumber() == MILEAGE_ROUTING)
    return MILEAGE_FARE;
  else
    return ROUTING_FARE;

  /**
  RtgKey rKey;
  rKey.vendor()         = routing->vendor();
  rKey.carrier()        = routing->carrier();
  rKey.routingTariff()  = routing->routingTariff();
  rKey.routingNumber()  = routing->routing();
  **/
}

//------------------------------------------------------------
// RoutingUtil::getRoutingData()
// -----------------------------------------------------------
const Routing*
RoutingUtil::getRoutingData(const PaxTypeFare* paxTypeFare, PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " Collecting Routing Data For RoutingUtil ");

  const std::vector<Routing*>& routing =
      trx.dataHandle().getRouting(paxTypeFare->vendor(),
                                  paxTypeFare->carrier(),
                                  paxTypeFare->tcrRoutingTariff1(),
                                  paxTypeFare->routingNumber(),
                                  trx.travelDate());

  if (routing.empty()) // try with routingTariff2
  {
    LOG4CXX_DEBUG(logger, " No Routing Found with rTariff1, Trying with Tariff2--");

    const std::vector<Routing*>& routingOne =
        trx.dataHandle().getRouting(paxTypeFare->vendor(),
                                    paxTypeFare->carrier(),
                                    paxTypeFare->tcrRoutingTariff2(),
                                    paxTypeFare->routingNumber(),
                                    trx.travelDate());

    // if (!routing)
    if (routingOne.empty())
    {
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR VENDOR: " << paxTypeFare->vendor());
      LOG4CXX_DEBUG(logger,
                    "EMPTY ROUTING VECTOR FOR INDUSTRY CARRIER: " << paxTypeFare->carrier());
      LOG4CXX_DEBUG(
          logger,
          "EMPTY ROUTING VECTOR FOR ROUTING TARIFF2 : " << paxTypeFare->tcrRoutingTariff2());
      LOG4CXX_DEBUG(logger,
                    "EMPTY ROUTING VECTOR FOR ROUTING NUMBER : " << paxTypeFare->routingNumber());
      LOG4CXX_DEBUG(logger,
                    "EMPTY ROUTING VECTOR FOR TRAVEL DATE: " << trx.travelDate().toSimpleString());
      return nullptr;
      LOG4CXX_INFO(logger, "No Routing Data Found");
    }
    else
      return routingOne.front();
  }
  return routing.front();
}

const Routing*
RoutingUtil::getStaticRoutingMileageHeaderData(PricingTrx& trx,
                                               const CarrierCode& govCxr,
                                               const PaxTypeFare& paxTypeFare)
{
  Routing* routing = nullptr;

  trx.dataHandle().get(routing);
  if (!routing)
    return routing;

  routing->vendor() = paxTypeFare.vendor();
  routing->carrier() = govCxr;
  routing->routingTariff() = paxTypeFare.tcrRoutingTariff1();
  getVendor(&paxTypeFare, trx.dataHandle(), routing->vendor());
  routing->routing() = MILEAGE_ROUTING;

  return routing;
}

const Routing*
RoutingUtil::getRoutingData(PricingTrx& trx,
                            PaxTypeFare& paxTypeFare,
                            const RoutingNumber& routingNumber)

{
  const FareMarket* fm = paxTypeFare.fareMarket();
  const DateTime& currentDate = trx.adjustedTravelDate(fm->travelDate());

  LOG4CXX_DEBUG(logger, " Collecting Routing Data ");
  LOG4CXX_DEBUG(logger,
                "ROUTING DATA DH DATE: " << trx.dataHandle().ticketDate().toSimpleString());
  LOG4CXX_DEBUG(logger, "TRAVEL ROUTE TRAVEL DATE: " << currentDate);

  if (routingNumber.empty())
  {
    return nullptr;
  }

  VendorCode vendor = paxTypeFare.vendor();
  TariffNumber routingTariff1 = paxTypeFare.tcrRoutingTariff1();
  TariffNumber routingTariff2 = paxTypeFare.tcrRoutingTariff2();
  getVendor(&paxTypeFare, trx.dataHandle(), vendor);

  FareDisplayTrx* fdTrx(dynamic_cast<FareDisplayTrx*>(&trx));
  if (UNLIKELY(fdTrx != nullptr))
  {
    FareDisplayInfo* fdi(paxTypeFare.fareDisplayInfo());
    if (fdi != nullptr)
    {
      if (!fdi->routingVendor().empty())
        vendor = fdi->routingVendor();
      if (fdi->routingTariff1() != 0)
        routingTariff1 = fdi->routingTariff1();
      if (fdi->routingTariff2() != 0)
        routingTariff2 = fdi->routingTariff2();
    }
  }

  const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(paxTypeFare.fare());
  const Routing* routing = nullptr;

  // if it is MultiLateral we should avoid routing map for carrier
  if (LIKELY(!(indFare != nullptr && indFare->isMultiLateral() == true)))
  {

    const std::vector<Routing*>& routingVect = trx.dataHandle().getRouting(
        vendor, fm->governingCarrier(), routingTariff1, routingNumber, currentDate);
    if (!routingVect.empty())
      routing = routingVect.front();

    if (!routing) // try with routingTariff2
    {
      LOG4CXX_DEBUG(logger, " No Routing Found with rTariff1, Trying with Tariff2--");

      const std::vector<Routing*>& routingVect = trx.dataHandle().getRouting(
          vendor, fm->governingCarrier(), routingTariff2, routingNumber, currentDate);
      if (UNLIKELY(!routingVect.empty()))
        routing = routingVect.front();
      else
      {
        LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR VENDOR: " << paxTypeFare.vendor());
        LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR GOV CARRIER: " << fm->governingCarrier());
        LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING TARIFF1 : " << routingTariff1);
        LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING TARIFF2 : " << routingTariff1);
        LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING NUMBER : " << routingNumber);
        LOG4CXX_DEBUG(logger,
                      "EMPTY ROUTING VECTOR FOR TVL DATE: " << currentDate.toSimpleString());
        LOG4CXX_DEBUG(logger,
                      "EMPTY ROUTING VECTOR FOR TKT DATE: "
                          << trx.dataHandle().ticketDate().toSimpleString());
      }
    }
  }

  if (!routing && indFare != nullptr) // try to get industry routing for Tariff 1
  {

    LOG4CXX_DEBUG(
        logger, " No Routing Found for rTariff1 or rTariff2, Trying with carrier YY and rTariff1");

    const std::vector<Routing*>& routingVect = trx.dataHandle().getRouting(
        paxTypeFare.vendor(), INDUSTRY_CARRIER, routingTariff1, routingNumber, currentDate);
    if (LIKELY(!routingVect.empty()))
    {
      routing = routingVect.front();
    }
    else
    {
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR VENDOR: " << paxTypeFare.vendor());
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR INDUSTRY CARRIER: " << INDUSTRY_CARRIER);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING TARIFF1 : " << routingTariff1);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING NUMBER : " << routingNumber);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR TVL DATE: " << currentDate.toSimpleString());
    }
  }

  if (!routing && indFare != nullptr) // try to get industry routing for Tariff 2
  {
    LOG4CXX_DEBUG(
        logger,
        " No Routing Found with rTariff1 or rTariff2, Trying with carrier YY and rTariff2");

    const std::vector<Routing*>& routingVect = trx.dataHandle().getRouting(
        paxTypeFare.vendor(), INDUSTRY_CARRIER, routingTariff2, routingNumber, currentDate);
    if (!routingVect.empty())
      routing = routingVect.front();
    else
    {
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR VENDOR: " << paxTypeFare.vendor());
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR INDUSTRY CARRIER: " << INDUSTRY_CARRIER);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING TARIFF2 : " << routingTariff2);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR ROUTING NUMBER : " << routingNumber);
      LOG4CXX_DEBUG(logger, "EMPTY ROUTING VECTOR FOR TVL DATE: " << currentDate.toSimpleString());
    }
  }

  if (!routing)
  {
    LOG4CXX_INFO(logger, "No Routing Data Found");
  }

  return routing;
}

//------------------------------------------------------------
// RoutingUtil::processBaseTravelRoute()
//
// Called by RoutingController and RoutingDiagCollector to
// determine whether to validate or display the base travel
// route for a constructed routing.
//
// Returns true if the base travel route requires validation
// or display.
//
// It has already been determined that the base of the
// constructed routing is Routing and maps exist.
// -----------------------------------------------------------

bool
RoutingUtil::processBaseTravelRoute(const RoutingInfo& rtgInfo)
{

  const Routing* origAddOnRtg = rtgInfo.origAddOnRouting();
  const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();

  // MPM - RTG - MPM
  // MPM - RTG - RTG
  // MPM - RTG
  //       RTG - MPM
  if ((origAddOnRtg && origAddOnRtg->routing() == MILEAGE_ROUTING) ||
      (destAddOnRtg && destAddOnRtg->routing() == MILEAGE_ROUTING))
  {
    return true;
  }

  //  RTG - RTG - RTG
  //  RTG - RTG
  //        RTG
  else if ((origAddOnRtg && origAddOnRtg->rmaps().empty()) ||
           (destAddOnRtg && destAddOnRtg->rmaps().empty()))
  {
    return true;
  }

  return false;
}

//-------------------------------------------------------------
// Determine whether location Matches the Travel Route Origin
//
// If the location nation matches the origin, return true
//
// If the location is within the US/CA/PR/USVI and the origin is
// also within the US/CA/PR/USVI, return true.
//-------------------------------------------------------------
bool
RoutingUtil::locMatchesOrigin(const Loc* loc, const TravelRoute& tvlRoute)
{
  if (loc->nation() == tvlRoute.originNation())
  {
    return true;
  }

  if (LocUtil::isDomesticUSCA(*loc))
  {
    if (LocUtil::isDomesticUSCA(*(tvlRoute.mileageTravelRoute().front()->origin())))
    {
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------
// Determine whether location Matches the Travel Route Origin
//
// If the location nation matches the origin, return true
//
// If the location is within the US/CA/PR/USVI and the origin is
// also within the US/CA/PR/USVI, return true.
//-------------------------------------------------------------
bool
RoutingUtil::locMatchesDestination(const Loc* loc, const TravelRoute& tvlRoute)
{
  if (loc->nation() == tvlRoute.destinationNation())
  {
    return true;
  }

  if (LocUtil::isDomesticUSCA(*loc))
  {
    if (LocUtil::isDomesticUSCA(*(tvlRoute.mileageTravelRoute().back()->destination())))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
// Determine whether location matches the O & D of this travelSeg.
//
// Return true if the Origin AND the Destination of this TravelSeg match the
// given location.
//------------------------------------------------------------------------------
bool
RoutingUtil::locMatchesTvlSeg(const Loc* loc, TravelSeg& tvlSeg)
{
  if (loc->nation() == tvlSeg.origin()->nation() && loc->nation() == tvlSeg.destination()->nation())
  {
    return true;
  }

  if (LocUtil::isDomesticUSCA(*loc))
  {
    if (LocUtil::isDomesticUSCA(*(tvlSeg.origin())) &&
        LocUtil::isDomesticUSCA(*(tvlSeg.destination())))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------
// Update RoutingInfo for Constructed Fares
//------------------------------------------
void
RoutingUtil::updateRoutingInfo(PaxTypeFare& paxTypeFare,
                               const Routing* routing,
                               RoutingInfo& routingInfo,
                               bool fillRoutingTrafficDescriptions,
                               bool useDefaultMarkets)
{
  if (paxTypeFare.hasConstructedRouting())
  {
    if (paxTypeFare.isReversed())
    {
      if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
          paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        routingInfo.origAddOnGateway() = paxTypeFare.gateway2();
        routingInfo.origAddOnInterior() = paxTypeFare.market2();
      }

      if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
          paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        routingInfo.destAddOnGateway() = paxTypeFare.gateway1();
        routingInfo.destAddOnInterior() = paxTypeFare.market1();
      }
    }
    else
    {
      if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
          paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        routingInfo.origAddOnGateway() = paxTypeFare.gateway1();
        routingInfo.origAddOnInterior() = paxTypeFare.market1();
      }

      if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
          paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        routingInfo.destAddOnGateway() = paxTypeFare.gateway2();
        routingInfo.destAddOnInterior() = paxTypeFare.market2();
      }
    }

    if (paxTypeFare.isReversed())
    {
      routingInfo.market1() = paxTypeFare.gateway2();
      routingInfo.market2() = paxTypeFare.gateway1();
      if (LIKELY(useDefaultMarkets))
      {
        if (UNLIKELY(routingInfo.market1().empty()))
          routingInfo.market1() = routingInfo.market2();
        if (UNLIKELY(routingInfo.market2().empty()))
          routingInfo.market2() = routingInfo.market1();
      }
    }
    else
    {
      routingInfo.market1() = paxTypeFare.gateway1();
      routingInfo.market2() = paxTypeFare.gateway2();
    }
  }

  //------------------------------------------------
  // Update the RoutingTariffCode for the Base Fare
  //------------------------------------------------
  if (routing != nullptr)
  {
    if (LIKELY(routing->routingTariff() == paxTypeFare.tcrRoutingTariff1()))
    {
      routingInfo.routingTariff() = paxTypeFare.tcrRoutingTariff1();
      routingInfo.tcrRoutingTariffCode() = paxTypeFare.tcrRoutingTariff1Code();
    }
    else
    {
      routingInfo.tcrRoutingTariffCode() = paxTypeFare.tcrRoutingTariff2Code();
      routingInfo.routingTariff() = paxTypeFare.tcrRoutingTariff2();
    }
    if (UNLIKELY(fillRoutingTrafficDescriptions))
    {
      fillRoutingTrfDesc(paxTypeFare, routing, routingInfo);
    }
  }

  //---------------------------------------------------------------------------
  // Pick up tariff code for SITA Mileage Routings that have no routing pointer
  //---------------------------------------------------------------------------
  else
  {
    routingInfo.routingTariff() = paxTypeFare.tcrRoutingTariff1();
    routingInfo.tcrRoutingTariffCode() = paxTypeFare.tcrRoutingTariff1Code();
  }

  //---------------------------------------------
  // Update the RoutingTariffCode for the Addons
  //---------------------------------------------
  routingInfo.tcrAddonTariff1Code() = routingInfo.tcrRoutingTariffCode();
  routingInfo.tcrAddonTariff2Code() = routingInfo.tcrRoutingTariffCode();
}

bool
RoutingUtil::isRouting(const PaxTypeFare& fare, PricingTrx& trx)
{
  RoutingFareType fareType = getRoutingType(&fare, trx);
  return fareType != MILEAGE_FARE;
}

void
RoutingUtil::getVendor(const PaxTypeFare* p, DataHandle& dataHandle, VendorCode& vendor)
{
  if (p->isFareByRule())
  {
    if (UNLIKELY(p->vendor().equalToConst("POFO") || p->vendor().equalToConst("FMS"))) // old FMS
    {
      vendor = ATPCO_VENDOR_CODE;
    }
    else // a new FMS
    {
      PaxTypeFareRuleData* ptfRuleData = p->paxTypeFareRuleData(25);
      if (LIKELY(ptfRuleData != nullptr))
      {
        const FareByRuleItemInfo* fareByRuleInfo =
            dynamic_cast<const FareByRuleItemInfo*>(ptfRuleData->ruleItemInfo());
        if (UNLIKELY(fareByRuleInfo != nullptr && !fareByRuleInfo->resultRoutingVendor().empty()))
        {
          vendor = fareByRuleInfo->resultRoutingVendor();
          return;
        }
      }
      if (dataHandle.getVendorType(vendor) != PUBLISHED)
      {
        PaxTypeFareRuleData* ruleData = p->paxTypeFareRuleData(25);
        if (LIKELY(ruleData != nullptr))
        {
          PaxTypeFare* baseFare = ruleData->baseFare();
          if (LIKELY(baseFare != nullptr))
          {
            vendor = baseFare->vendor();
          }
          else
            vendor = ATPCO_VENDOR_CODE;
        }
      }
    }
  }
}

bool
RoutingUtil::isSpecialRouting(const PaxTypeFare& paxTypeFare, const bool checkEmptyRouting)
{
  if ((CAT25_DOMESTIC == paxTypeFare.routingNumber()) ||
      (CAT25_INTERNATIONAL == paxTypeFare.routingNumber()))
  {
    return true;
  }

  if ((true == checkEmptyRouting) && (CAT25_EMPTY_ROUTING == paxTypeFare.routingNumber()))
  {
    return true;
  }

  return false;
}

bool
RoutingUtil::isTicketedPointOnly(const Routing* routing, bool flightTrackingCxr)
{
  if (routing)
  {
    return (routing->unticketedPointInd() == TKTPTS_TKTONLY) ||
           (routing->unticketedPointInd() == IGNORE_TKTPTSIND && !flightTrackingCxr);
  }
  else
  {
    return !flightTrackingCxr;
  }
}

void
RoutingUtil::fillRoutingTrfDesc(PaxTypeFare& paxTypeFare,
                                const Routing* routing,
                                RoutingInfo& routingInfo)
{
  // cat25 with result routing only?
  if (!paxTypeFare.isFareByRule())
    return;

  const FareByRuleItemInfo& fbrItemInfo = paxTypeFare.fareByRuleInfo();
  if (fbrItemInfo.resultRouting().empty() || fbrItemInfo.resultRoutingTariff() == 0)
    return;

  if (routingInfo.tcrRoutingTariffCode().empty())
  {
    DataHandle dataHandle;
    const std::vector<TariffCrossRefInfo*>& trfXRef =
        dataHandle.getTariffXRef(routing->vendor(),
                                 routing->carrier(),
                                 paxTypeFare.isInternational() ? INTERNATIONAL : DOMESTIC);
    std::vector<TariffCrossRefInfo*>::const_iterator it = trfXRef.begin();
    std::vector<TariffCrossRefInfo*>::const_iterator ite = trfXRef.end();
    for (; it != ite; it++)
    {
      if (routingInfo.routingTariff() == (*it)->routingTariff1())
      {
        routingInfo.tcrRoutingTariffCode() = (*it)->routingTariff1Code();
        return;
      }
      if (routingInfo.routingTariff() == (*it)->routingTariff2())
      {
        routingInfo.tcrRoutingTariffCode() = (*it)->routingTariff2Code();
        return;
      }
    }
  }
}

} // namespace tse
