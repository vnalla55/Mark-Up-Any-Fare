//-------------------------------------------------------------------
//
// MultiTransportMarkets.cpp
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
//-------------------------------------------------------------------
#include "Common/MultiTransportMarkets.h"

#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"

#include <algorithm>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.MultiTransportMarkets");

const std::string MultiTransportMarkets::City = "CITY";
const std::string MultiTransportMarkets::Airport = "AIRPORT";

bool
MultiTransportMarkets::getMarkets(std::vector<Market>& markets) const
{
  LOG4CXX_INFO(logger,
               " Entered MultiTransportMarkets::getMarkets() For " << _boardCity << "-" << _offCity
                                                                   << " --" << _carrier);
  std::set<LocCode> originAirports;
  originAirports.insert(_boardCity);
  std::set<LocCode> destinationAirports;
  destinationAirports.insert(_offCity);
  getAirports(originAirports, _boardCity);
  getAirports(destinationAirports, _offCity);
  if (_fareMarket != nullptr &&
      (_fareMarket->boardMultiCity() != _boardCity || _fareMarket->offMultiCity() != _offCity))
  {
    originAirports.insert(_fareMarket->boardMultiCity());
    destinationAirports.insert(_fareMarket->offMultiCity());
  }
  return buildMarkets(originAirports, destinationAirports, markets);
}

struct GetAirports : public std::unary_function<MultiTransport, LocCode>

{
  LocCode operator()(const tse::MultiTransport* multitransport) const
  {
    return multitransport->multitransLoc();
  }
};

bool
MultiTransportMarkets::getAirports(std::set<LocCode>& airports, const LocCode& city) const
{
  DataHandle dataHandle(_ticketingDate);
  const std::vector<MultiTransport*>& multitransports =
      dataHandle.getMultiTransportLocs(city, _carrier, _geoTravelType, _tvlDate);

  std::transform(multitransports.begin(),
                 multitransports.end(),
                 inserter(airports, airports.end()),
                 GetAirports());

  LOG4CXX_DEBUG(logger, city << " -- Has " << airports.size() << " Airports");
  return !airports.empty();
}

bool
MultiTransportMarkets::buildMarkets(std::set<LocCode>& origAirports,
                                    std::set<LocCode>& destAirports,
                                    std::vector<Market>& markets) const

{
  std::set<LocCode>::iterator i(origAirports.begin()), j;

  for (; i != origAirports.end(); ++i)
  {
    j = destAirports.begin();
    for (; j != destAirports.end(); ++j)
    {
      LOG4CXX_DEBUG(logger, "Creating Market ---   " << *i << " -- " << *j);
      addMarket(*i, *j, markets);
    }
  }

  LOG4CXX_DEBUG(logger, "Total Unique Market For the Request = " << markets.size());
  return !markets.empty();
}

std::string
MultiTransportMarkets::getLocType(const LocCode& loc, DataHandle& dataHandle) const
{
  if (isCity(loc))
    return MultiTransportMarkets::City;
  return MultiTransportMarkets::Airport;
}

bool
MultiTransportMarkets::isCity(const LocCode& loc) const
{
  DataHandle dataHandle(_ticketingDate);
  const std::vector<MultiTransport*>& multitransports =
      dataHandle.getMultiTransportLocs(loc, _carrier, _geoTravelType, _tvlDate);
  return !multitransports.empty();
}

void
MultiTransportMarkets::addMarket(const LocCode& mkt1,
                                 const LocCode& mkt2,
                                 std::vector<Market>& markets) const
{
  markets.push_back(std::make_pair(mkt1, mkt2));
}

}
