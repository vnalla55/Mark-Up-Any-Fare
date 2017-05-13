// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once
#include "DataModel/PaxTypeFare.h"

#include <string>
#include <vector>
namespace tse
{
class DiagCollector;
class FarePath;
class FareMarket;
class FareUsage;
class Itin;
class PricingTrx;
class TravelSeg;
class JourneyValidatorTest;
class JourneyValidator
{
  friend class ::tse::JourneyValidatorTest;

public:
  JourneyValidator(PricingTrx& trx, DiagCollector& diag) : _trx(trx), _diag(diag) {}
  bool validateJourney(FarePath& fpath, std::string& results);
  bool processJourneyAfterDifferential(FarePath& fpath, std::string& _results);

  bool checkMarriedConnection();
  bool processMarriedConnection(FarePath& fpath);

private:
  PricingTrx& _trx;
  DiagCollector& _diag;

  bool journey(FarePath& fpath, Itin& itinerary, bool journeyDiagOn);
  bool validShoppingJourney(std::vector<FareMarket*>& journeysForShopping, const FareMarket* fm);
  bool flowJourney(FarePath& fpath, const FareMarket& fm);
  bool foundFlow(FarePath& fpath, TravelSeg* startTvlseg, size_t& flowLen);
  bool checkBookedClassAvail(const TravelSeg* tvlSeg);
  size_t startFlow(FarePath& fpath, TravelSeg* startTvlseg, bool& failJourney);
  bool journeyAfterDifferential(FarePath& fpath);
  bool differentialFound(FarePath& fpath);
  bool reBookRequired(const FareUsage* fu,
                      const TravelSeg* tvlSeg,
                      const PaxTypeFare::SegmentStatus& fuSegStat);
  bool
  realAvailLocal(FareUsage* fu, uint16_t iTravelFu, TravelSeg* tvlSeg, bool checkBookedCosAlso);
  bool
  isLocalAvailForDualRbd(FareUsage* fu, uint16_t iTravelFu,
                         FareMarket& fm, uint16_t iTravelFm);
  bool needSetRebook(FareMarket& fm, FarePath& fp);
  bool realAvail(FareUsage* fu,
                 uint16_t iTravelFu,
                 const FareMarket& fm,
                 uint16_t iTravelFm,
                 bool checkBookedCosAlso,
                 bool useLocal);
  bool hasCat5Rebook(const FareUsage* fu,
                     const TravelSeg* tvlSeg,
                     const PaxTypeFare::SegmentStatus& segStat);
  bool useFlowAvail(FarePath& fpath, FareMarket& fm);
  bool sameBookingCode(FarePath& fpath, FareMarket& fm);
  bool flowAvailFailed(FareUsage& fu);
  bool checkLocalAvailStatus(FareUsage* fu, TravelSeg* tvlSeg, uint16_t tvlSegIndex);
  BookingCode bookingCode(const FareUsage* fu,
                          const TravelSeg* tvlSeg,
                          const PaxTypeFare::SegmentStatus& fuSegStat);
  bool recheckJourneyConnection(const uint16_t nonArunks,
                                const AirSeg* startAirSeg,
                                const AirSeg* prevAirSeg,
                                const AirSeg* airSeg);
  bool validConnection(FareMarket& fm, FarePath& fp);
  void adjustAvailBreaks(FareMarket& fm, FarePath& fp);
  bool processOtherSegs(FarePath& fpath, std::vector<TravelSeg*>& segmentsProcessed);
  bool processingKeepFare(const FareUsage& fu);
  bool processLocalJourneyCarriers(FarePath& fpath, Itin& itin);
};
} // tse namespace
