//----------------------------------------------------------------------------
//  Code extracted directly from ItinAnalyzerService.cpp
//
//  Copyright Sabre 2013
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"

#include <vector>

namespace tse
{
class AirSeg;
class Itin;
class FareMarket;
class CarrierPreference;
class PricingTrx;
struct SimilarItinData;

class JourneyPrepHelperTest;

namespace iadetail
{

class JourneyPrepHelper
{
  friend class ::tse::JourneyPrepHelperTest;

public:
  static void prepareForJourney(PricingTrx& trx);

  static void prepareItinForJourney(std::vector<Itin*>::const_iterator itinBegin,
                                    std::vector<Itin*>::const_iterator itinEnd,
                                    PricingTrx& trx);

  static void prepareItinForJourney(std::vector<SimilarItinData>::const_iterator itinBegin,
                                    std::vector<SimilarItinData>::const_iterator itinEnd,
                                    PricingTrx& trx);

  static void updateCarrierPreferences(PricingTrx& trx);


  static void updateCarrierPreferences(std::vector<Itin*>::const_iterator itinBegin,
                                       std::vector<Itin*>::const_iterator itinEnd,
                                       PricingTrx& trx);

  static const CarrierPreference*
  getCarrierPref(PricingTrx& trx, const CarrierCode& carrier, const DateTime& date);

private:
  static void markFlowMarkets(PricingTrx& trx, Itin& itin);

  static void markMarriedMarkets(PricingTrx& trx);

  static bool duplicateFareMarket(std::vector<FareMarket*>& marriedFm, FareMarket* fm);

  static void initAvailBreak(const PricingTrx& trx, FareMarket& fm);
  static void initAvailBreakForMIP(PricingTrx& trx, FareMarket& fm, Itin& itin);
  static bool isAvailBreak(const PricingTrx& trx,
                           const Itin& itin,
                           const size_t availFltCount,
                           const AirSeg* airSeg,
                           const AirSeg* nextAirSeg);

  static void initAvailWithoutCOS(FareMarket& fm,
                                  Itin& itin,
                                  PricingTrx& trx,
                                  std::vector<FareMarket*>& processedFM);

  static void initAvailSmallFMWithoutCOS(FareMarket& fm,
                                         Itin& itin,
                                         PricingTrx& trx,
                                         std::vector<FareMarket*>& processedFM);

  static void prepareItinForJourney(const bool initAvailBreakForRepricing, Itin&, PricingTrx&);

  static void updateCarrierPreferences(std::vector<SimilarItinData>::const_iterator itinBegin,
                                       std::vector<SimilarItinData>::const_iterator itinEnd,
                                       PricingTrx& trx);

  static void updateCarrierPreferences(PricingTrx& trx, Itin& itin);

  static bool shouldInitAvailBreakForRepricing(PricingTrx&);
  static bool isSoloCarrierAvailabilityApply(const PricingTrx& trx, const AirSeg* airSeg);
}; // End class JourneyPrepHelper

} // End namespace iadetail
} // End namespace tse

