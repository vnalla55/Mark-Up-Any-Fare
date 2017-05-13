//-------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/MultiTransportDiagSection.h"

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include "AddonConstruction/SpecifiedFareCache.h"
#endif
#include "Common/Logger.h"
#include "Common/MultiTransportMarkets.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/MultiTransport.h"
#include "Fares/AddonFareController.h"

#include <sstream>
#include <string>

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.MultiTransportDiagSection");

LocCode MultiTransportDiagSection::MultiTransportMapElem::_requestedOrigin = "";
LocCode MultiTransportDiagSection::MultiTransportMapElem::_requestedDestination = "";

void
MultiTransportDiagSection::buildDisplay()
{
  LOG4CXX_DEBUG(logger, "Build Display ");
  if (_trx.getRequest() && _trx.getRequest()->inclusionCode() == FD_ADDON)
  {
    LOG4CXX_DEBUG(logger, "Skiping diag 217 fo AD inclusion code");
    return;
  }

  buildHeader();
  if (!initMap())
  {
    LOG4CXX_DEBUG(logger, "Failed to inif MultiTransport fare map - exit");
    return;
  }
  // process all markets (carriers)
  for (const auto fareMarket : _trx.fareMarket())
  {
    std::map<CarrierCode, std::set<MultiTransportMapElem> >::iterator itMap =
        _multiTransportMap.find(fareMarket->governingCarrier());
    if (itMap != _multiTransportMap.end())
    {
      // process whole set for carrier
      buildRequestLine(*fareMarket);
      for (const auto& multiTransportMapElem : itMap->second)
      {
        MultiTransportMapElem& el = const_cast<MultiTransportMapElem&>(multiTransportMapElem);
        //                   12345678901234567890123456789012345678901234567890123456789
        //                   AAA-BBB      AIRPORT - CITY             Y             Y
        _trx.response() << el.origin() << "-" << el.destination();
        _trx.response().setf(std::ios::right, std::ios::adjustfield);
        _trx.response() << std::setw(13) << el.locOriginType() << " - ";
        _trx.response().setf(std::ios::left, std::ios::adjustfield);
        _trx.response() << std::setw(17) << el.locDestinationType();
        _trx.response() << std::setw(14) << el.isFare() << el.isReturnedFare() << std::endl;
      }
    }
  }
}

MultiTransportDiagSection::MultiTransportMapElem*
MultiTransportDiagSection::getMultiTransportMapElem(const CarrierCode& carrier,
                                                    const LocCode& origin,
                                                    const LocCode& destination)
{
  std::map<CarrierCode, std::set<MultiTransportMapElem> >::iterator itMap =
      _multiTransportMap.find(carrier);
  if (itMap != _multiTransportMap.end())
  {
    // find set for carrier
    std::set<MultiTransportMapElem>& elemSet = (*itMap).second;
    MultiTransportMapElem elem(origin, destination);
    std::set<MultiTransportMapElem>::iterator it = elemSet.find(elem);
    if (it != elemSet.end())
    {
      // find element in set
      return &const_cast<MultiTransportMapElem&>(*it);
    }
    return nullptr;
  }
  return nullptr;
}

MultiTransportDiagSection::MultiTransportMapElem*
MultiTransportDiagSection::putMultiTransportMapElem(const CarrierCode& carrier,
                                                    const LocCode& origin,
                                                    const LocCode& destination)
{
  // check if we already have element in map
  MultiTransportMapElem* tmp = getMultiTransportMapElem(carrier, origin, destination);
  if (tmp)
    return tmp;
  std::map<CarrierCode, std::set<MultiTransportMapElem> >::iterator itMap =
      _multiTransportMap.find(carrier);
  if (itMap == _multiTransportMap.end())
  {
    // no element in map for carrier - insert new
    itMap = _multiTransportMap.insert(_multiTransportMap.end(),
                                      std::pair<CarrierCode, std::set<MultiTransportMapElem> >(
                                          carrier, std::set<MultiTransportMapElem>()));
  }
  if (itMap != _multiTransportMap.end())
  {
    // add new element to set
    MultiTransportMapElem elem(origin, destination);
    std::set<MultiTransportMapElem>& elemSet = (*itMap).second;
    std::set<MultiTransportMapElem>::iterator it = elemSet.insert(elemSet.end(), elem);
    return &const_cast<MultiTransportMapElem&>(*it);
  }
  return nullptr;
}

bool
MultiTransportDiagSection::initMap()
{
  _multiTransportMap.clear();
  // process all markets, for each market, origin, destination get
  // multtransport markets
  DataHandle dataHandle(_trx.ticketingDate());
  for (const auto fareMarket : _trx.fareMarket())
  {
    MultiTransportMapElem::requestedOrigin() = fareMarket->origin()->loc();
    MultiTransportMapElem::requestedDestination() = fareMarket->destination()->loc();

    std::vector<MultiTransportMarkets::Market> markets;
    MultiTransportMarkets multiTransportMkts(fareMarket->origin()->loc(),
                                             fareMarket->destination()->loc(),
                                             fareMarket->governingCarrier(),
                                             fareMarket->geoTravelType(),
                                             _trx.ticketingDate(),
                                             _trx.travelDate(),
                                             fareMarket);
    multiTransportMkts.getMarkets(markets);

    for (const auto& market : markets)
    {
      // we have some market, put it into map
      MultiTransportMapElem* mte =
          putMultiTransportMapElem(fareMarket->governingCarrier(), market.first, market.second);
      // fail to put element into map - shouldn't happend
      if (!mte)
        return false;

      // set location types, check if there is published fare between
      // markets for carrier
      mte->locOriginType() = multiTransportMkts.getLocType(market.first, dataHandle);
      mte->locDestinationType() = multiTransportMkts.getLocType(market.second, dataHandle);
      mte->setPublishedFare(isMarketHasPublishedFare(market, *fareMarket));
    }
  }

  // process Construced fares

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  SpecifiedFareCache specFareCache;
#endif

  // loop through all markets
  for (const auto fareMarket : _trx.fareMarket())
  {
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    AddonFareController addonFareCtrl(_trx, *_trx.itin().front(), *fareMarket, &specFareCache);
#else
    AddonFareController addonFareCtrl(_trx, *_trx.itin().front(), *fareMarket);
#endif
    std::vector<Fare*> cxrConstructedFares;
    std::vector<Fare*> indConstructedFares;
    if (!addonFareCtrl.process(cxrConstructedFares, indConstructedFares))
      continue;

    for (const auto cxrConstructedFare : cxrConstructedFares)
    {
      // there are some fares for carrier
      Fare& fare = *cxrConstructedFare;
      MultiTransportMapElem* elem =
          getMultiTransportMapElem(fare.carrier(), fare.origin(), fare.destination());
      if (elem)
      {
        // foun constructed fare for markets and carrier
        elem->setConstructedFare();
      }
    }
    // process industry fares
    for (const auto fareYY : indConstructedFares)
    {
      Fare& fare = *fareYY;
      MultiTransportMapElem* elem =
          getMultiTransportMapElem(INDUSTRY_CARRIER, fare.origin(), fare.destination());
      if (elem)
      {
        // foun constructed fare for markets and carrier
        elem->setConstructedFare();
      }
    }
  }
  // check if there are returned fares
  for (const auto paxTypeFare : _trx.allPaxTypeFare())
  {
    MultiTransportMapElem* elem = getMultiTransportMapElem(
        paxTypeFare->carrier(), paxTypeFare->origin(), paxTypeFare->destination());
    if (!elem)
    {
      LOG4CXX_DEBUG(logger,
                    "Fare with not matching markets: " << paxTypeFare->carrier() << " "
                                                       << paxTypeFare->origin() << "-"
                                                       << paxTypeFare->destination());
      continue;
    }
    if (paxTypeFare->isPublished())
      elem->setReturnedPublishedFare(); // published fares
    else if (paxTypeFare->isConstructed())
      elem->setReturnedConstrucedFare(); // constucted fares
    else
      elem->setReturnedCreatedFare(); // cat19-22, 25, 35
  }
  return true;
}

void
MultiTransportDiagSection::buildHeader()
{
  _trx.response() << std::endl;
  _trx.response() << "***********************************************************" << std::endl;
  _trx.response() << "            MULTITRANSPORT FARE-GROUPING DIAGNOSTIC        " << std::endl;
  _trx.response() << "***********************************************************" << std::endl;
}

void
MultiTransportDiagSection::buildRequestLine(const FareMarket& fm) const
{
  const LocCode& origin = _trx.travelSeg().front()->origin()->loc();
  const LocCode& destination = _trx.travelSeg().back()->destination()->loc();
  const CarrierCode& carrier = fm.governingCarrier();
  const std::string& geoTravel =
      (fm.geoTravelType() == GeoTravelType::Domestic || fm.geoTravelType() == GeoTravelType::Transborder) ? "DOM " : "INTL";
  //                   12345678901234567890123456789012345678901234567890123456789
  //                   REQUEST: BWI-HBA  CXR-QF  TVLTYPE-INTL
  _trx.response() << "                                                           " << std::endl;
  _trx.response() << "REQUEST: " << origin << "-" << destination << "  CXR-" << carrier
                  << "  TVLTYPE-" << geoTravel << "          " << std::endl;
  _trx.response() << "-----------------------------------------------------------" << std::endl;
  _trx.response() << "POSSIBLE     MARKET TYPE           FARES           FARES   " << std::endl;
  _trx.response() << "ORIG DEST                          FILED/CREATED   RETURNED" << std::endl;
  _trx.response() << "-----------------------------------------------------------" << std::endl;
}

bool
MultiTransportDiagSection::isMarketHasPublishedFare(const MultiTransportMarkets::Market& market,
                                                    const FareMarket& fm)
{
  const std::vector<const FareInfo*>& publishedFares = _trx.dataHandle().getFaresByMarketCxr(
      market.first, market.second, fm.governingCarrier(), _trx.travelDate());
  return !publishedFares.empty();
}
} // tse namespace
