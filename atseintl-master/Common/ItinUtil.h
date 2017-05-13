//----------------------------------------------------------------------------
//
//  File:        ItinUtil.h
//  Created:     5/5/2004
//  Authors:     JY
//
//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
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
#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
class AirlineAllianceCarrierInfo;
class AirSeg;
class DateTime;
class Diag192Collector;
class FareMarket;
class Itin;
class Loc;
class PricingTrx;
class TimeAndUnit;
class TravelSegAnalysis;
class Trx;

class ItinUtil
{
  friend class ItinUtilTest;

public:
  static void removeAtaeMarkets(PricingTrx& trx);
  static void setRoundTheWorld(PricingTrx& trx, Itin& itin);
  static int16_t gcmBasedFurthestPoint(Itin& itin, Diag192Collector* dc = nullptr);
  static void roundTheWorldDiagOrThrow(PricingTrx& trx, bool isExcItin = false);
  static bool setFurthestPoint(PricingTrx& trx, Itin* itin);
  static const TravelSeg* getFurthestPoint(const Itin* itin);
  static NationCode originNation(const Itin& itin);

  static const TravelSeg* getFirstValidTravelSeg(const Itin* itin);
  static const TravelSeg* getLastValidTravelSeg(const Itin* itin);
  static bool atleastOneSegmentConfirm(const Itin* itin);

  //   @method getOriginationCurrency
  //
  //   @Description Determines the itinerary origination currency
  //
  //   @param Itin
  //   @param CurrencyCode
  //
  //   @return bool - true, retrieved origin nation currency , else false
  static bool getOriginationCurrency(const Itin& itin,
                                     CurrencyCode& originationCurrency,
                                     const DateTime& ticketingDate,
                                     const bool& isFareDisplayTrx = false);

  static const DateTime& getTravelDate(const std::vector<TravelSeg*> &travelSegs);

  static bool isOpenSegAfterDatedSeg(const Itin& itin, const TravelSeg* tvlSeg);

  static bool isRussian(const Itin* itin);
  static bool isDomesticPeru(PricingTrx* trx, const Itin* itin);
  static bool isDomesticOfNation(const Itin* itin, const NationCode nationCode);

  static void journeys(Itin& itin,
                       PricingTrx& trx,
                       std::vector<FareMarket*>& jnyMarkets,
                       std::vector<TravelSegJourneyType>* journeyOrder = nullptr);

  static bool journeyConnection(const Itin& itin,
                                const AirSeg* currFlt,
                                const AirSeg* prevFlt,
                                size_t jnyLength);

  static FareMarket* findMarket(Itin& itin, std::vector<TravelSeg*>& tvlSegs);

  static FareMarket*
  findMarket(const std::vector<TravelSeg*>& tvlSegs, const std::vector<FareMarket*>& fms);

  static bool checkMileage(PricingTrx& trx, std::vector<TravelSeg*>& tvlSegs, DateTime traveldate);

  static bool allFlightsBookedInSameCabin(const Itin& itin);
  static bool applyMultiCurrencyPricing(PricingTrx* trx, const Itin& itin);

  static double calculateEconomyAvailabilityScore(const TravelSeg& tvlSeg,
                                                  double point,
                                                  int numberOfClassesToAdd,
                                                  int requestedNumberOfSeats);

  static double calculateEconomyAvailabilityScore(const Itin* itin,
                                                  double point,
                                                  int numberOfClassesToAdd,
                                                  int requestedNumberOfSeats,
                                                  std::map<const Itin*, double>* sopScoreMap = nullptr);

  static void readConfigDataForSOPScore(double& point, int& numberOfClassesToAdd);
  static void setGeoTravelType(TravelSegAnalysis& tvlSegAnalysis, Boundary tvlboundary, Itin& itn);
  static void setGeoTravelType(Boundary tvlboundary, FareMarket& fareMarket);
  static void swapValidatingCarrier(PricingTrx& trx, Itin& itin);
  static void swapValidatingCarrier(PricingTrx &trx, CarrierCode &validatingCarrier);
  static uint32_t journeyMileage(const AirSeg* startFlt,
                                 const AirSeg* currFlt,
                                 PricingTrx& trx,
                                 DateTime travelDate);
  static uint32_t journeyMileage(const AirSeg* startFlt,
                                 const AirSeg* currFlt,
                                 const Loc *startLoc,
                                 const Loc *endLoc,
                                 PricingTrx& trx,
                                 DateTime travelDate);
  static void setItinCurrencies(Itin& itin, const DateTime& ticketingDate);

  static bool isStopover(const TravelSeg& from,
                         const TravelSeg& to,
                         const GeoTravelType geoTravelType,
                         const TimeAndUnit& minTime,
                         const FareMarket* tsm = nullptr);

  static bool
  isStayTimeWithinDays(const DateTime& arriveDT, const DateTime& departDT, const int16_t& stayDays);

  static int countContinents(const PricingTrx& trx,
                             const FareMarket& fm,
                             const AirlineAllianceCarrierInfo& carrierInfo);

  static size_t startFlow(Itin& itin, PricingTrx& trx, const TravelSeg* startTvlseg);

  static size_t foundFlow(Itin& itin,
                          const TravelSeg* startTvlseg,
                          size_t flowLen,
                          std::vector<FareMarket*>& jnyMarkets,
                          const bool isLocalJourneyCarrier,
                          PricingTrx& trx,
                          bool findFM = true);
  static std::vector<TravelSeg*> getArunks(const Itin* itin);

  static std::vector<int32_t> generateNextFareMarketList(Itin& itin);

  static void
  collectMarkupsForFarePaths(Itin& itin);

protected:
  static const DateTime _today;
  static const std::string nationListSwapValidCXR;

private:
  static uint32_t journeyMileage(PricingTrx& trx,
                                 const AirSeg* startFlt,
                                 const AirSeg* currFlt,
                                 DateTime travelDate);

  static bool connectionException(const Itin& itin,
                                  const AirSeg* currFlt,
                                  const AirSeg* prevFlt,
                                  size_t jnyLength);
};

class AreaCrossingDeterminator
{
  typedef std::vector<const TravelSeg*> Segments;

public:
  void determine(const Trx& trx, const std::vector<TravelSeg*>& segs);
  void determine(const Trx& trx, const TravelSeg& ts);

  const Segments& transatlanticSectors() const { return _transatlanticSectors; }
  const Segments& transpacificSectors() const { return _transpacificSectors; }
  const Segments& area2area3crossing() const { return _area2area3crossing; }

  bool isTransoceanicSurface(const TravelSeg& ts) const
  {
    return (ts.segmentType() == Surface || ts.segmentType() == Arunk) &&
           (find(ts, _transatlanticSectors) || find(ts, _transpacificSectors));
  }

private:

  bool find(const TravelSeg& ts, const Segments& destination) const;

  Segments _transatlanticSectors;
  Segments _transpacificSectors;
  Segments _area2area3crossing;
};

} // end tse namespace

