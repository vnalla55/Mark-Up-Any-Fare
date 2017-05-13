//-------------------------------------------------------------------
//
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

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"


#include <set>

namespace tse
{

class DataHandle;
class DateTime;
class Loc;
class FareMarket;
class MultiTransportMarkets
{

  friend class MultiTransportMarketsTest;

public:
  enum MarketType
  { CITY_CITY = 0,
    CITY_AIRPORT,
    AIRPORT_CITY,
    AIRPORT_AIRPORT,
    UNKNOWN_MARKET };
  enum LocType
  { CITY = 1,
    AIRPORT,
    UNKNOWN };
  typedef std::pair<LocCode, LocCode> Market;
  MultiTransportMarkets(const LocCode& boardCity,
                        const LocCode& offCity,
                        const CarrierCode& carrier,
                        const GeoTravelType& geoTravelType,
                        const DateTime& ticketingDate,
                        const DateTime& tvlDate,
                        const FareMarket* fm = nullptr)
    : _boardCity(boardCity),
      _offCity(offCity),
      _carrier(carrier),
      _geoTravelType(geoTravelType),
      _ticketingDate(ticketingDate),
      _tvlDate(tvlDate),
      _fareMarket(fm)
  {
  }
  virtual ~MultiTransportMarkets() {}
  bool getMarkets(std::vector<Market>& markets) const;
  void addMarket(const LocCode& mkt1, const LocCode& mkt2, std::vector<Market>&) const;
  std::string getLocType(const LocCode& loc, DataHandle& dataHandle) const;

protected:
  bool getAirports(std::set<LocCode>& airports, const LocCode&) const;
  bool buildMarkets(std::set<LocCode>& airports,
                    std::set<LocCode>& destAirports,
                    std::vector<Market>& markets) const;

private:
  const LocCode& _boardCity;
  const LocCode& _offCity;
  const CarrierCode& _carrier;
  const GeoTravelType& _geoTravelType;
  const DateTime& _ticketingDate;
  const DateTime& _tvlDate;
  const FareMarket* _fareMarket;
  static const std::string City;
  static const std::string Airport;

  bool isCity(const LocCode& loc) const;
};
}
