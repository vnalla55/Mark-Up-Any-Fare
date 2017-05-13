//----------------------------------------------------------------------------
//
//  File:        ItinUtil.h
//  Created:     8/16/2004
//  Authors:     Mike Carroll
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
#include "DataModel/PaxTypeFare.h"

#include <string>

namespace tse
{
class FareMarket;
class Itin;
class TravelSeg;

class FareMarketUtil
{
public:
  /**
   * Returns the typical diagnostic FareMarket display
   * of travel segments and carriers.
   *
   * For example: DFW-AA-LON-BA-PAR
   *
   * @param FareMarket Farket to get travel segments from
   *
   * @return  formatted string
   */
  static std::string getDisplayString(const FareMarket&);

  /**
   * Returns the typical diagnostic FareMarket display
   * of travel segments and carriers, governing carrier,
   * global direction and directionality.
   *
   * For example: YTO-AA-BOS    /CXR-AA/ GI-WH  .OUT.
   *
   * @param FareMarket Farket to get travel segments from
   *
   * @return  formatted string
   */
  static std::string getFullDisplayString(const FareMarket&);

  static bool setMultiCities(FareMarket& fareMarket, const DateTime& tvlDate);

  static const LocCode& getBoardMultiCity(const FareMarket&, const TravelSeg&);

  static const LocCode& getOffMultiCity(const FareMarket&, const TravelSeg&);

  static LocCode getMultiCity(const CarrierCode&, const LocCode&, GeoTravelType, const DateTime&);

  static void getParticipatingCarrier(const std::vector<TravelSeg*>& travelSegs,
                                      std::set<CarrierCode>& carriers);

  static bool isParticipatingCarrier(const std::set<CarrierCode>& participatingCarriers,
                                     const CarrierCode& carrier);

  static bool isYYOverride(PricingTrx& trx, const std::vector<TravelSeg*>& travelSegs);

  static void getGovCxrOverride(PricingTrx& trx,
                                const std::vector<TravelSeg*>& travelSegs,
                                std::vector<CarrierCode>& govCxrOverride);

  static size_t numFlts(const Itin& itin, const FareMarket& fmkt, const size_t startIndex);

  static void group3(Itin& itin,
                     FareMarket& fm,
                     size_t startIndex,
                     size_t fltsCovered,
                     PricingTrx& trx,
                     std::vector<FareMarket*>& processedFM,
                     bool processCOS);

  static void buildMarket(Itin& itin,
                          FareMarket& fm,
                          std::vector<TravelSeg*>::iterator& groupStartTvlSeg,
                          PricingTrx& trx,
                          size_t startIndex,
                          size_t fltsCovered,
                          std::vector<FareMarket*>& processedFM,
                          bool processCOS);

  static void buildUsingLocal(FareMarket& fm,
                              PricingTrx& trx,
                              size_t fltsCovered,
                              const TravelSeg* startTvlSeg,
                              bool processCOS);

  static bool fmWithoutCOS(FareMarket& fm,
                           PricingTrx& trx,
                           FareMarket* srcFm,
                           std::vector<FareMarket*>& processedFM);

  static std::vector<Itin*>
  collectOwningItins(const FareMarket& fm, const std::vector<Itin*>& collection);

  static bool checkAvailability(const FareMarket& fm, BookingCode bkc, uint16_t numSeats);

private:
  static void pushBackArunkCOS(PricingTrx& trx, FareMarket& fm, bool processCOS);
};

class ValidateContinentBased
{
public:
  ValidateContinentBased(PricingTrx& trx, int maximumNumberOfContinents)
    : _trx(trx), _maximumNumberOfContinents(maximumNumberOfContinents)
  {
  }

  bool operator()(const PaxTypeFare* ptf)
  {
    std::string fbc = ptf->createFareBasis(_trx);
    return continentBasedFare(fbc) && int(*(--fbc.end()) - 48) < _maximumNumberOfContinents;
  }

  bool continentBasedFare(const std::string& fbc)
  {
    static const std::string continentBasedFareIdentifer = "ONE";
    return fbc.find(continentBasedFareIdentifer) != std::string::npos;
  }

private:
  PricingTrx& _trx;
  int _maximumNumberOfContinents;
};

} // end tse namespace

