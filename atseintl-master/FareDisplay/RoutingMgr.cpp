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

#include "FareDisplay/RoutingMgr.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RoutingUtil.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "FareDisplay/FDPSRController.h"
#include "FareDisplay/FDTPDController.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "Fares/DRVController.h"
#include "Routing/AddOnRouteExtraction.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingEnums.h"
#include "Routing/RtgKey.h"
#include "Routing/SpecifiedRoutingValidator.h"
#include "Routing/TravelRouteBuilder.h"

#include <vector>

namespace tse
{

static Logger
logger("atseintl.FareDisplay.RoutingMgr");

namespace
{

static const uint16_t MAX_FD_RTG_LINES = 20;

// typedef std::map<std::string, RoutingInfo*> RoutingInfoMap;
static const std::string delims = "-/";

class Not3nor17 : public std::unary_function<const RoutingRestriction*, bool>
{
public:
  Not3nor17(bool constructed) : constructed_(constructed) {}
  bool operator()(const RoutingRestriction* restriction) const
  {
    return !constructed_ || (restriction->restriction() != ROUTING_RESTRICTION_3 &&
                             restriction->restriction() != ROUTING_RESTRICTION_17);
  }

private:
  bool constructed_;
};

class UniqueRoutingsBuilder
{
public:
  UniqueRoutingsBuilder(FareDisplayResponse& response) : response_(response) {}
  void operator()(PaxTypeFare* paxTypeFare)
  {
    if (paxTypeFare == nullptr)
      return;
    FareDisplayInfo* fareDisplayInfo(paxTypeFare->fareDisplayInfo());
    if (fareDisplayInfo != nullptr)
    {
      std::string routingString(fareDisplayInfo->routingSequence());
      if (routingString.empty())
      {
        routingString = paxTypeFare->routingNumber();
      }
      response_.uniqueRoutings().insert(
          RtgSeq2PaxTypeFareMap::value_type(routingString, paxTypeFare));
    }
  }
  void operator()(const FDAddOnFareInfo* addOnFareInfo)
  {
    if (addOnFareInfo == nullptr)
      return;
    std::string routingString(addOnFareInfo->addOnRoutingSeq());
    if (routingString.empty())
      routingString = addOnFareInfo->routing();
    response_.uniqueAddOnRoutings().insert(
        AddOnFareInfos::value_type(routingString, addOnFareInfo));
  }

private:
  FareDisplayResponse& response_;
};

class StringProcessor
{
public:
  StringProcessor(const LocCode& orig, const LocCode& dest) : orig_(orig), dest_(dest) {}
  void operator()(std::string& str)
  {
    if (str.empty())
      return;
    if (isReversed(str))
      reverseString(str);
  }

private:
  bool isReversed(const std::string& str) const
  {
    return str.substr(0, 3) == dest_ || str.substr(str.size() - 3) == orig_;
  }
  void reverseString(std::string& str) const
  {
    std::string rev;
    std::string::size_type pos;
    while ((pos = str.find_last_of(delims)) != std::string::npos)
    {
      rev.append(str.substr(pos + 1));
      rev.append(1, str[pos]);
      str.erase(pos);
    }
    rev.append(str);
    rev.swap(str);
  }
  LocCode orig_, dest_;
};
}

//---------------------------------------------------------------------------
// RoutingMgr::RoutingMgr()
//----------------------------------------------------------------------------
RoutingMgr::RoutingMgr(FareDisplayTrx& trx)
  : _trx(trx), _rtMap(nullptr), _tvlRouteMap(nullptr), _routingKeyMap(nullptr), _fareMarketMap(nullptr)
{
  _trx.dataHandle().get(_tvlRouteMap);
  _trx.dataHandle().get(_routingKeyMap);
  _trx.dataHandle().get(_fareMarketMap);
}

//---------------------------------------------------------------------------
// RoutingMgr::~RoutingMgr()
//----------------------------------------------------------------------------
RoutingMgr::~RoutingMgr() {}

//---------------------------------------------------------------------------
// RoutingMgr::getAddonRoutingData()
//----------------------------------------------------------------------------
const Routing*
RoutingMgr::getAddonRoutingData(const VendorCode& vendor,
                                const TariffNumber& routingTariff1,
                                const TariffNumber& routingTariff2,
                                const CarrierCode& carrier,
                                const RoutingNumber& routingNumber,
                                const DateTime& travelDate,
                                const Fare* fare)

{
  LOG4CXX_DEBUG(logger, "Collecting Routing Data ");

  if (routingNumber.empty())
  {
    return nullptr;
  }

  const Routing* routing = nullptr;
  const std::vector<Routing*>& routingVect =
      _trx.dataHandle().getRouting(vendor, carrier, routingTariff1, routingNumber, travelDate);
  if (!routingVect.empty())
    routing = routingVect.front();

  if (!routing) // try with routingTariff2
  {
    LOG4CXX_DEBUG(logger, "No Routing Found with rTariff1, Trying with Tariff2--");
    const std::vector<Routing*>& routingOne =
        _trx.dataHandle().getRouting(vendor, carrier, routingTariff2, routingNumber, travelDate);
    if (!routingOne.empty())
      routing = routingOne.front();
  }

  const IndustryFare* indFare(nullptr);
  if (fare != nullptr)
    indFare = dynamic_cast<const IndustryFare*>(fare);
  if (!routing && indFare != nullptr) // try to get industry routing for Tariff 1
  {
    LOG4CXX_DEBUG(logger,
                  "No Routing Found for rTariff1 or rTariff2, Trying with carrier YY and rTariff1");
    const std::vector<Routing*>& routingTwo = _trx.dataHandle().getRouting(
        vendor, INDUSTRY_CARRIER, routingTariff1, routingNumber, travelDate);
    if (!routingTwo.empty())
      routing = routingTwo.front();
  }

  if (!routing && indFare != nullptr) // try to get industry routing for Tariff 2
  {
    LOG4CXX_DEBUG(
        logger,
        " No Routing Found with rTariff1 or rTariff2, Trying with carrier YY and rTariff2");
    const std::vector<Routing*>& routingFour = _trx.dataHandle().getRouting(
        vendor, INDUSTRY_CARRIER, routingTariff2, routingNumber, travelDate);
    if (!routingFour.empty())
      routing = routingFour.front();
  }

  if (!routing)
  {
    LOG4CXX_INFO(logger, "No Routing Data Found");
  }

  return routing;
}

//------------------------------------------------------------
// RoutingMgr::buildTravelRouteAndMap()
//------------------------------------------------------------
bool
RoutingMgr::buildTravelRouteAndMap()
{
  LOG4CXX_DEBUG(logger, "buildTravelRouteAndMap...");

  TravelRouteBuilder builder;

  // Only needed for each unique RoutingNumber
  buildUniqueRoutings();

  FareMarket* fareMarket(_trx.fareMarket().front());
  TravelRoute* travelRoute(nullptr), *reversedTravelRoute(nullptr);
  _trx.dataHandle().get(travelRoute);
  _trx.dataHandle().get(reversedTravelRoute);
  if (!builder.buildTravelRoute(_trx, *fareMarket, *travelRoute))
  {
    LOG4CXX_ERROR(logger, "buildTravelRoute failed.");
    return false;
  }
  reverseTravelRoute(*travelRoute, *reversedTravelRoute);

  // Go through each unique routing string paxTypeFare
  RtgSeq2PaxTypeFareConstIter iter(_trx.fdResponse()->uniqueRoutings().begin());
  RtgSeq2PaxTypeFareConstIter iterEnd(_trx.fdResponse()->uniqueRoutings().end());

  for (; iter != iterEnd; iter++)
  {
    PaxTypeFare* paxTypeFare = (*iter).second;

    const Routing* routing;
    const Routing* origAddOnRouting = nullptr;
    const Routing* destAddOnRouting = nullptr;
    RoutingNumber rtgNo = paxTypeFare->routingNumber();

    bool hasConstructedInfo = paxTypeFare->hasConstructedRouting();

    if (rtgNo.empty() && hasConstructedInfo)
    {
      rtgNo = paxTypeFare->fare()->constructedFareInfo()->fareInfo().routingNumber();
    }
    routing = RoutingUtil::getRoutingData(_trx, *paxTypeFare, rtgNo);

    if (hasConstructedInfo)
    {

      if (paxTypeFare->constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
          paxTypeFare->constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        origAddOnRouting =
            RoutingUtil::getRoutingData(_trx, *paxTypeFare, paxTypeFare->origAddonRouting());
      }

      if (paxTypeFare->constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
          paxTypeFare->constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
      {
        destAddOnRouting =
            RoutingUtil::getRoutingData(_trx, *paxTypeFare, paxTypeFare->destAddonRouting());
      }
    }

    if (paxTypeFare->isReversed())
      processRouting(iter->first,
                     *reversedTravelRoute,
                     paxTypeFare,
                     routing,
                     origAddOnRouting,
                     destAddOnRouting);
    else
      processRouting(
          iter->first, *travelRoute, paxTypeFare, routing, origAddOnRouting, destAddOnRouting);
  }

  return true;
}

bool
RoutingMgr::buildAddOnRoutings()
{
  LOG4CXX_DEBUG(logger, "buildAddOnRoutings...");

  buildUniqueRoutings();

  AddOnFareInfosConstIter iter(_trx.fdResponse()->uniqueAddOnRoutings().begin());
  AddOnFareInfosConstIter iterEnd(_trx.fdResponse()->uniqueAddOnRoutings().end());

  for (; iter != iterEnd; iter++)
  {
    const FDAddOnFareInfo* fareInfo(iter->second);
    if (fareInfo != nullptr)
    {
      const std::vector<TariffCrossRefInfo*>& tariffXref =
          _trx.dataHandle().getTariffXRefByAddonTariff(fareInfo->vendor(),
                                                       fareInfo->carrier(),
                                                       INTERNATIONAL,
                                                       fareInfo->addonTariff(),
                                                       _trx.travelDate());

      if (!tariffXref.empty())
      {
        const Routing* routing = getAddonRoutingData(fareInfo->vendor(),
                                                     tariffXref.front()->routingTariff1(),
                                                     tariffXref.front()->routingTariff2(),
                                                     fareInfo->carrier(),
                                                     fareInfo->routing(),
                                                     _trx.travelDate());

        processAddOnRouting(iter->first, _trx.boardMultiCity(), routing, *fareInfo);
      }
      else
      {
        LOG4CXX_ERROR(logger,
                      "Failed to get tariffCrossRefInfo for vendor "
                          << fareInfo->vendor() << " carrier " << fareInfo->carrier()
                          << " addon tariff " << fareInfo->addonTariff());
      }
    }
    else
    {
      LOG4CXX_ERROR(logger, "fdAddOnFareInfo empty for " << iter->first);
    }
  }

  return true;
}

//------------------------------------------------------------------------------
// buildRtgKey()
//------------------------------------------------------------------------------
void
RoutingMgr::buildRtgKey(PaxTypeFare& paxTypeFare,
                        const FareMarket& fareMarket,
                        const Routing* routing,
                        const Routing* origAddOnRouting,
                        const Routing* destAddOnRouting,
                        RtgKey& rKey)
{
  // Mileage Routings for SITA have no routing data
  if (routing == nullptr)
  {
    rKey.vendor() = paxTypeFare.vendor();
    rKey.carrier() = fareMarket.governingCarrier();
    rKey.routingTariff() = paxTypeFare.tcrRoutingTariff1();
    rKey.routingNumber() = paxTypeFare.routingNumber();
  }
  else
  {
    rKey.vendor() = routing->vendor();
    rKey.carrier() = routing->carrier();
    rKey.routingTariff() = routing->routingTariff();
    rKey.routingNumber() = routing->routing();
  }
  if (origAddOnRouting != nullptr)
  {
    rKey.addOnRouting1() = origAddOnRouting->routing();
  }
  if (destAddOnRouting != nullptr)
  {
    rKey.addOnRouting2() = destAddOnRouting->routing();
  }
}

//------------------------------------------------------------------------------
// buildUniqueRoutings()
//------------------------------------------------------------------------------
void
RoutingMgr::buildUniqueRoutings()
{
  UniqueRoutingsBuilder uniqueRoutingsBuilder(*_trx.fdResponse());

  if (_trx.getRequest()->requestedInclusionCode() == ADDON_FARES)
  { // addon fare display entry
    std::for_each(
        _trx.allFDAddOnFare().begin(), _trx.allFDAddOnFare().end(), uniqueRoutingsBuilder);
  }
  else
  {
    std::for_each(
        _trx.allPaxTypeFare().begin(), _trx.allPaxTypeFare().end(), uniqueRoutingsBuilder);
  }
}

//------------------------------------------------------------------------------
// processRouting()
//------------------------------------------------------------------------------
bool
RoutingMgr::processRouting(const std::string& rtgSeq,
                           TravelRoute& tvlRoute,
                           PaxTypeFare* paxTypeFare,
                           const Routing* routing,
                           const Routing* origAddOnRouting,
                           const Routing* destAddOnRouting)
{
  if(tvlRoute.travelRoute().front().boardCity().loc() == tvlRoute.travelRoute().back().offCity().loc())
    _trx.getOptions()->setRtw(true);

  RoutingInfo *routingInfo = nullptr;
  _trx.dataHandle().get(routingInfo);
  _trx.fdResponse()->uniqueRoutingMap().insert(RoutingInfoMap::value_type(rtgSeq, routingInfo));
  LOG4CXX_DEBUG(logger,
                "**** routingInfos size: " << _trx.fdResponse()->uniqueRoutingMap().size());

  if (!routing && paxTypeFare->routingNumber() != MILEAGE_ROUTING)
  {
    return false;
  }

  routingInfo->routing() = routing;
  if (paxTypeFare->isReversed())
  {
    routingInfo->origAddOnRouting() = destAddOnRouting;
    routingInfo->destAddOnRouting() = origAddOnRouting;
  }
  else
  {
    routingInfo->origAddOnRouting() = origAddOnRouting;
    routingInfo->destAddOnRouting() = destAddOnRouting;
  }
  RoutingUtil::updateRoutingInfo(*paxTypeFare, routing, *routingInfo, true, false);

  bool mileageRestriction(false);

  // Do restrictions
  if ((routing != nullptr && !(routing->rests().empty())) ||
      (origAddOnRouting != nullptr && !origAddOnRouting->rests().empty()) ||
      (destAddOnRouting != nullptr && !destAddOnRouting->rests().empty()))
  {
    LOG4CXX_DEBUG(logger, "Adding restrictions...");

    RestrictionInfos* restrictionInfos = nullptr;
    _trx.dataHandle().get(restrictionInfos);
    routingInfo->restrictions() = restrictionInfos;

    Not3nor17 except3and17(paxTypeFare->hasConstructedRouting());

    std::vector<RoutingRestriction*> allRouteRestrictions;
    if (routing)
    {
      LOG4CXX_DEBUG(logger, "Have routing restriction");
      remove_copy_if(routing->rests().begin(),
                     routing->rests().end(),
                     back_inserter(allRouteRestrictions),
                     not1(except3and17));
    }
    if (origAddOnRouting)
    {
      LOG4CXX_DEBUG(logger, "Have origAddOnRouting restriction");
      remove_copy_if(origAddOnRouting->rests().begin(),
                     origAddOnRouting->rests().end(),
                     back_inserter(allRouteRestrictions),
                     not1(except3and17));
    }
    if (destAddOnRouting)
    {
      LOG4CXX_DEBUG(logger, "Have destAddOnRouting restriction");
      remove_copy_if(destAddOnRouting->rests().begin(),
                     destAddOnRouting->rests().end(),
                     back_inserter(allRouteRestrictions),
                     not1(except3and17));
    }
    LOG4CXX_DEBUG(logger, "allRouteRestrictions size: " << allRouteRestrictions.size());

    // Store away
    RestrictionInfo info;
    info.processed() = true;
    info.valid() = true;

    std::vector<RoutingRestriction*>::iterator restIter = allRouteRestrictions.begin();
    std::vector<RoutingRestriction*>::iterator restIterEnd = allRouteRestrictions.end();
    for (; restIter != restIterEnd; restIter++)
    {
      routingInfo->restrictions()->insert(RestrictionInfos::value_type((*restIter), info));
      if ((*restIter)->restriction() == RTW_ROUTING_RESTRICTION_12 ||
          (*restIter)->restriction() == MILEAGE_RESTRICTION_16)
      {
        mileageRestriction = true;
      }
    }
  }

  // Mileage Routings - from paxTypeFare, or gathered
  if (paxTypeFare->routingNumber() == MILEAGE_ROUTING ||
      (paxTypeFare->hasConstructedRouting() &&
       (paxTypeFare->origAddonRouting() == MILEAGE_ROUTING ||
        paxTypeFare->destAddonRouting() == MILEAGE_ROUTING)) ||
      mileageRestriction)
  {
    LOG4CXX_DEBUG(logger, "Adding mileage...");
    MileageInfo* mileageInfo = nullptr; // lint !e578
    _trx.dataHandle().get(mileageInfo);
    routingInfo->mileageInfo() = mileageInfo;
    getMPMMileageInfo(paxTypeFare->globalDirection(), _trx.travelDate(), *mileageInfo);

    // -----------------------
    // get PSR for FareDisplay
    // -----------------------
    FDPSRController fdPSR;
    fdPSR.getPSR(_trx, paxTypeFare->globalDirection(), *routingInfo);
    // -----------------------
    // get TPD for FareDisplay
    // -----------------------
    FDTPDController fdTPD;
    fdTPD.getTPD(_trx, paxTypeFare->carrier(), paxTypeFare->globalDirection(), *routingInfo);
  }

  // Route maps
  if ((routing != nullptr && !routing->rmaps().empty()) ||
      (origAddOnRouting != nullptr && !origAddOnRouting->rmaps().empty()) ||
      (destAddOnRouting != nullptr && !destAddOnRouting->rmaps().empty()))
  {
    MapInfo* mapInfo = nullptr;
    _trx.dataHandle().get(mapInfo);
    routingInfo->mapInfo() = mapInfo;

    const Routing* origAddOnWithMap(nullptr);
    if (origAddOnRouting != nullptr && !origAddOnRouting->rmaps().empty())
      origAddOnWithMap = origAddOnRouting;
    const Routing* destAddOnWithMap(nullptr);
    if (destAddOnRouting != nullptr && !destAddOnRouting->rmaps().empty())
      destAddOnWithMap = destAddOnRouting;

    if (origAddOnRouting == nullptr && destAddOnRouting == nullptr)
    {
      if (!routing->rmaps().empty())
        getRoutingMaps(tvlRoute, mapInfo, paxTypeFare->isReversed(), routing);
    }
    else if (routing == nullptr || routing->rmaps().empty())
    {
      const LocCode& origin(paxTypeFare->isReversed() ? _trx.offMultiCity()
                                                      : _trx.boardMultiCity()),
          &destination(paxTypeFare->isReversed() ? _trx.boardMultiCity() : _trx.offMultiCity());
      if (origAddOnWithMap)
      {
        getAddOnRoutingMaps(origin, mapInfo, origAddOnWithMap);
        if (destAddOnWithMap)
        {
          MapInfo* rtgAddonMapInfo(nullptr);
          _trx.dataHandle().get(rtgAddonMapInfo);
          routingInfo->rtgAddonMapInfo() = rtgAddonMapInfo;
          getAddOnRoutingMaps(destination, rtgAddonMapInfo, destAddOnWithMap);
        }
      }
      else if (destAddOnWithMap)
        getAddOnRoutingMaps(destination, mapInfo, destAddOnWithMap);
    }
    else
    {
      getRoutingMaps(tvlRoute,
                     mapInfo,
                     paxTypeFare->isReversed(),
                     routing,
                     origAddOnWithMap,
                     destAddOnWithMap);
      // this is an insane workaround for cases when no strings are extracted
      // but they do if addon routings are passed exchanged
      // no matter if fare is reversed or not
      if (mapInfo->routeStrings() == nullptr || mapInfo->routeStrings()->empty() ||
          (mapInfo->routeStrings()->size() == 1 && mapInfo->routeStrings()->front().empty()))
        getRoutingMaps(tvlRoute,
                       mapInfo,
                       paxTypeFare->isReversed(),
                       routing,
                       destAddOnWithMap,
                       origAddOnWithMap);
    }
  }
  return true;
}

bool
RoutingMgr::processAddOnRouting(const std::string& rtgSeq,
                                const LocCode& interiorLoc,
                                const Routing* routing,
                                const FDAddOnFareInfo& fareInfo)
{
  RoutingInfo* routingInfo(nullptr);
  _trx.dataHandle().get(routingInfo);
  _trx.fdResponse()->uniqueRoutingMap().insert(RoutingInfoMap::value_type(rtgSeq, routingInfo));
  if (!routing && fareInfo.routing() != MILEAGE_ROUTING)
  {
    return false;
  }
  routingInfo->routing() = routing;

  bool mileageRestriction(false);
  if (routing != nullptr && !(routing->rests().empty()))
  {
    RestrictionInfos* restrictionInfos(nullptr);
    _trx.dataHandle().get(restrictionInfos);
    routingInfo->restrictions() = restrictionInfos;
    std::vector<RoutingRestriction*> allRouteRestrictions;
    copy(routing->rests().begin(), routing->rests().end(), back_inserter(allRouteRestrictions));
    RestrictionInfo info;
    std::vector<RoutingRestriction*>::iterator restIter(allRouteRestrictions.begin());
    std::vector<RoutingRestriction*>::iterator restIterEnd(allRouteRestrictions.end());
    for (; restIter != restIterEnd; restIter++)
    {
      routingInfo->restrictions()->insert(RestrictionInfos::value_type((*restIter), info));
      if ((*restIter)->restriction() == RTW_ROUTING_RESTRICTION_12 ||
          (*restIter)->restriction() == MILEAGE_RESTRICTION_16)
      {
        mileageRestriction = true;
      }
    }
  }

  if (fareInfo.routing() == MILEAGE_ROUTING || mileageRestriction)
  {
    MileageInfo* mileageInfo(nullptr);
    _trx.dataHandle().get(mileageInfo);
    routingInfo->mileageInfo() = mileageInfo;
    getMPMMileageInfo(fareInfo.globalDir(), _trx.travelDate(), *mileageInfo);
    FDPSRController fdPSR;
    fdPSR.getPSR(_trx, fareInfo.globalDir(), *routingInfo);
    FDTPDController fdTPD;
    fdTPD.getTPD(_trx, fareInfo.carrier(), fareInfo.globalDir(), *routingInfo);
  }

  if (routing != nullptr && !routing->rmaps().empty())
  {
    MapInfo* mapInfo(nullptr);
    _trx.dataHandle().get(mapInfo);
    routingInfo->mapInfo() = mapInfo;
    getAddOnRoutingMaps(interiorLoc, mapInfo, routing);
  }

  return true;
}

//------------------------------------------------------------------------------
// getRoutingMaps()
//------------------------------------------------------------------------------
bool
RoutingMgr::getRoutingMaps(TravelRoute& tvlRoute,
                           MapInfo* mapInfo,
                           bool isFareReversed,
                           const Routing* routing,
                           const Routing* origAddOnRouting,
                           const Routing* destAddOnRouting)
{
  uint16_t rtgStrMaxResSize = 0;

  if (_trx.getRequest()->requestType() == FARE_DISPLAY_REQUEST)
  {
    rtgStrMaxResSize = MAX_FD_RTG_LINES;
    if (!(Global::config())
             .getValue("FQ_RTGSTRINGS_RESPONSESIZE", rtgStrMaxResSize, "FAREDISPLAY_SVC"))
    {
      CONFIG_MAN_LOG_KEY_ERROR(logger, "FQ_RTGSTRINGS_RESPONSESIZE", "FAREDISPLAY_SVC");
    }
  }
  else if (_trx.isRD())
  {
    if (!(Global::config())
             .getValue("RD_RTGSTRINGS_RESPONSESIZE", rtgStrMaxResSize, "FAREDISPLAY_SVC"))
    {
      CONFIG_MAN_LOG_KEY_ERROR(logger, "RD_RTGSTRINGS_RESPONSESIZE", "FAREDISPLAY_SVC");
    }
  }

  if (routing == nullptr)
    return false;

  DataHandle dataHandle;
  SpecifiedRoutingValidator specifiedValidator;
  RoutingMapStrings* strings;

  _trx.dataHandle().get(strings);
  mapInfo->routeStrings() = strings;

  SpecifiedRoutingDiag* map = nullptr;
  dataHandle.get(map);
  map->initialize(*routing, _trx);

  SpecifiedRoutingDiag* origAddOnMap = nullptr;
  if (origAddOnRouting != nullptr && routing != origAddOnRouting)
  {
    dataHandle.get(origAddOnMap);
    origAddOnMap->initialize(*origAddOnRouting, _trx);
  }
  SpecifiedRoutingDiag* destAddOnMap = nullptr;
  if (destAddOnRouting != nullptr && routing != destAddOnRouting)
  {
    dataHandle.get(destAddOnMap);
    destAddOnMap->initialize(*destAddOnRouting, _trx);
  }

  Indicator directionalInd =
      _trx.diagnostic().isDiag455ParamSet() ? MAPDIR_IGNOREIND : routing->directionalInd();

  if (origAddOnMap != nullptr)
    specifiedValidator.extractRouteStrings(_trx,
                                           *origAddOnMap,
                                           tvlRoute.govCxr(),
                                           tvlRoute,
                                           strings,
                                           map,
                                           destAddOnMap,
                                           rtgStrMaxResSize,
                                           directionalInd,
                                           isFareReversed);
  else
    specifiedValidator.extractRouteStrings(_trx,
                                           *map,
                                           tvlRoute.govCxr(),
                                           tvlRoute,
                                           strings,
                                           destAddOnMap,
                                           nullptr,
                                           rtgStrMaxResSize,
                                           directionalInd,
                                           isFareReversed);
  if (mapInfo->routeStrings() != nullptr)
    processRouteStrings(*mapInfo->routeStrings());
  return true;
}

bool
RoutingMgr::getAddOnRoutingMaps(const LocCode& interiorLoc,
                                MapInfo* mapInfo,
                                const Routing* routing) const
{
  if (routing == nullptr)
    return false;
  SpecifiedRouting map(*routing, _trx);
  const Loc* intLoc(_trx.dataHandle().getLoc(interiorLoc, _trx.travelDate()));
  if (intLoc == nullptr)
    return false;
  AddOnRouteExtraction extract(intLoc);
  RoutingMapStrings* strings;
  _trx.dataHandle().get(strings);
  mapInfo->routeStrings() = strings;
  extract.execute(_trx, strings, &map);
  if (mapInfo->routeStrings() != nullptr)
    processRouteStrings(*mapInfo->routeStrings());
  return true;
}

void
RoutingMgr::processRouteStrings(RoutingMapStrings& strings) const
{
  if (!strings.empty())
  {
    // unreverse reversed strings
    std::for_each(strings.begin(), strings.end(),
                  StringProcessor(_trx.boardMultiCity(), _trx.offMultiCity()));
    if (strings[0].empty())
      strings.erase(strings.begin());
  }
}

//------------------------------------------------------------------------------
// getMPMMileageInfo()
//------------------------------------------------------------------------------
void
RoutingMgr::getMPMMileageInfo(GlobalDirection global,
                              const DateTime& travelDate,
                              MileageInfo& mileageInfo)
{ // lint !e578
  const Mileage* mileage = _trx.dataHandle().getMileage(
      _trx.boardMultiCity(), _trx.offMultiCity(), MPM, global, travelDate);
  if (mileage != nullptr)
  {
    mileageInfo.totalApplicableMPM() = static_cast<uint16_t>(mileage->mileage());
  }
  else
  {
    LOG4CXX_WARN(logger,
                 "Failed to get MPM for market " << _trx.boardMultiCity() << "-"
                                                 << _trx.offMultiCity() << " global " << global);
  }
}

void
RoutingMgr::reverseTravelRoute(const TravelRoute& route, TravelRoute& reversed) const
{
  reversed = route;
  reversed.origin() = route.destination();
  reversed.originNation() = route.destinationNation();
  reversed.destination() = route.origin();
  reversed.destinationNation() = route.originNation();
  if (route.travelRoute().empty())
    return;
  const TravelRoute::CityCarrier& routeCC(route.travelRoute().front());
  TravelRoute::CityCarrier& reversedCC(reversed.travelRoute().front());
  reversedCC.boardCity() = routeCC.offCity();
  reversedCC.boardNation() = routeCC.offNation();
  reversedCC.offCity() = routeCC.boardCity();
  reversedCC.offNation() = routeCC.boardNation();
}

} // namespace tse
