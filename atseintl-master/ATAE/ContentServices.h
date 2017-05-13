//----------------------------------------------------------------------------
//
//      File: ContentServices.h
//      Description: Access to available ATAE content services
//      Created: May 10, 2004
//      Authors: Mike Carroll
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

#include "ATAE/PricingDssRequest.h"
#include "ATAE/PricingDssFlightKey.h"
#include "ATAE/PricingDssFlightMapBuilder.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class DiagManager;
class TravelSeg;
class PricingTrx;
class BaggageTrx;
class AirSeg;
class FareMarket;
class DataHandle;
class ClassOfService;
class Itin;
class AvailData;
class FlightAvail;
class AtaeResponseHandler;
class Cabin;
class AncillaryPricingTrx;
class Logger;

class ContentServices
{
  friend class ItinAnalyzerService;
  friend class ContentServicesTest;

public:
  virtual ~ContentServices() = default;

  //--------------------------------------------------------------------------
  // @function ContentServices::getSchedAndAvail
  //
  // Description: Get schedules and availability from ATAE
  //
  // @param trx - a valid PricingTrx reference
  // @return void
  //--------------------------------------------------------------------------
  void getSchedAndAvail(PricingTrx& trx);

  void getAvailShopping(PricingTrx& trx);

  void getSched(AncillaryPricingTrx& trx);
  void getSched(BaggageTrx& trx);

  void getSchedule(PricingTrx& trx);

  void getAvailability(PricingTrx& trx);

  void processDssFlights(PricingTrx& trx,
                         std::vector<PricingDssFlight>::const_iterator begin,
                         std::vector<PricingDssFlight>::const_iterator end,
                         PricingDssFlightMap& flightMap) const;

  bool isAnySegmentInRequest(const PricingTrx& trx) const;

private:
  static Logger _logger;

  enum AvlStatusType
  {
    AVL_STATUS_NONE = 0,
    AVL_STATUS_AVS,
    AVL_STATUS_DCA_CACHE,
    AVL_STATUS_DCA,
    AVL_STATUS_AVS_DCA_TIMEOUT,
    AVL_STATUS_THROTTLING,
    AVL_STATUS_HOST,
    AVL_STATUS_HOST_TIMEOUT
  };

  //--------------------------------------------------------------------------
  // @function ContentServices::initialize
  //
  // Description: Perform initialization tasks
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  static bool initialize();

  void fillDummyCOS(FareMarket& fm, DataHandle& dataHandle,PricingTrx& trx );

  void getCosFlown(PricingTrx& trx, FareMarket& fm);

  ClassOfService* getACos(PricingTrx& trx, AirSeg* pAirSeg);

  ClassOfService* dummyCOS(uint16_t i, AirSeg* airSeg, DataHandle& dataHandle);

  bool stopOversArunkIncluded(FareMarket& fm);

  void restFareMarkets(Itin& itin, PricingTrx& trx);

  void soloTest(PricingTrx& trx, Itin& itin);

  void journeyTest(PricingTrx& trx, Itin& itin);

  void checkAndLogAs2Errors(const std::string& xmlData) const;

  virtual bool callAs2_old(PricingTrx& trx);
  virtual bool callAs2(PricingTrx& trx);

  bool callAs2Shopping(PricingTrx& trx, AtaeResponseHandler& ataeResponseHandler);

  void buildAvailMapMerge(PricingTrx& trx, std::vector<FareMarket*>& fareMarketsSentToAtae);

  virtual bool
  callDssV2_old(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule);

  virtual bool
  callDssV2(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule);

  void ataeDiag195(PricingTrx& trx, const std::vector<FareMarket*>& processedFms);

  void fareMarketsDiag195(const std::vector<FareMarket*>& processedFms,
                          DiagManager& diag,
                          PricingTrx& trx);

  void
  dssReqDiag195(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule);

  void dssRespDiag195(PricingTrx& trx);

  void diagReservationData(PricingTrx& trx, DiagManager& diag);
  void diagOACData(PricingTrx& trx, DiagManager& diag);

  bool sendResDataToAtae(PricingTrx& trx);

  bool xmlDiagRequired(PricingTrx& trx, std::string& diagParam);

  void xmlDiag(PricingTrx& trx, std::string& xmlData);

  void processDiag998(PricingTrx& trx, const std::string& xmlData);

  void checkDuplicateFareMarkets(Itin& itin, FareMarket* fm,
                                PricingTrx&  trx);
  void checkDuplicateFareMarketsShop(Itin& itin, FareMarket* fm);

  void upSellAvailDiag195(PricingTrx& trx, const std::vector<FareMarket*>& processedFms);

  void adjustAvail(PricingTrx& trx);

  void updateAvail(PricingTrx& trx, const FlightAvail& fltAvail);

  void updateFareMarketAvail(FareMarket& fm, const FlightAvail& fltAvail);

  void updateTravelSegAvail(std::vector<ClassOfService*>& cos, const FlightAvail& fltAvail);

  bool foundCos(const ClassOfService& cos, const FlightAvail& fltAvail);

  virtual const Cabin* getCabin(DataHandle& dataHandle,
                                const CarrierCode& carrier,
                                const BookingCode& classOfService,
                                const DateTime& date);

  virtual uint16_t numSeatsNeeded(PricingTrx& trx);
  void
  debugOnlyFakeASV2(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets, std::string& diagParam);
  void debugOnlyFakeASV2ForThruFMs(PricingTrx& trx,
                                   std::vector<FareMarket*>& fareMarkets,
                                   std::string& diagParam);

  ClassOfService*
  getCosFake(PricingTrx& trx, const BookingCode& bookingCode, const TravelSeg* travelSeg);

  void updateAvailabilityMap(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets) const;

  bool shouldPopulateAllItineraries(const PricingTrx& trx) const;
  void populateFlightMap(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule, PricingDssFlightMap& flightMap) const;

  void printDiag195DssRespForTravelSegment(const TravelSeg* travelSeg, const PricingTrx& trx, DiagManager& diag) const;
  void printDiag195DssReqForTravelSegment(const TravelSeg* travelSeg, const PricingTrx& trx,
                                          const MethodGetFlownSchedule getFlownSchedule, DiagManager& diag) const;
  void addDSSResponseToDiag(const std::string& payload, PricingTrx& trx);
  void addASResponseToDiag(const std::string& payload, PricingTrx& trx);
  };
} // End namespace tse
