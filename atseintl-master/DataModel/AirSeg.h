//-------------------------------------------------------------------
//
//  File:        AirSeg.h
//  Created:     March 8, 2004
//  Authors:
//
//  Description: Itinerary air segment
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Added new members
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/TravelSeg.h"

namespace tse
{

class PricingTrx;

class AirSeg : public TravelSeg
{
private:
  std::string _considerOnlyCabin; // CBP
  FlightNumber _marketingFlightNumber = 0; // FLN, also in ATAE schedules
  CarrierCode _marketingCarrierCode; // CXR, also in ATAE schedules
  FlightNumber _operatingFlightNumber = 0; // From ATAE schedules
  CarrierCode _operatingCarrierCode; // OCX
  int8_t _arrivalDayAdjust = 0; // DD2
  bool _eticket = false; // P0Z in shopping schema
  bool _bbrCarrier = false;
  char _marriageStatus = MARRIAGE_NONE; // BB3
  std::vector<uint32_t> _validAvailabilityIds;

public:
  static const char MARRIAGE_START;
  static const char MARRIAGE_MIDDLE;
  static const char MARRIAGE_END;
  static const char MARRIAGE_NONE;

  AirSeg();

  virtual bool isAir() const override;
  virtual AirSeg* toAirSeg() override { return this; }
  virtual const AirSeg* toAirSeg() const override { return this; }
  virtual AirSeg& toAirSegRef() override { return *this; }
  virtual const AirSeg& toAirSegRef() const override { return *this; }

  virtual AirSeg* clone(DataHandle& dh) const override;

  bool isFake() const { return (_marketingFlightNumber == 9999 && _operatingFlightNumber == 8765); }

  void makeFake()
  {
    _marketingFlightNumber = 9999;
    _operatingFlightNumber = 8765;
  }

  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------
  std::string& considerOnlyCabin() { return _considerOnlyCabin; }
  const std::string& considerOnlyCabin() const { return _considerOnlyCabin; }

  CarrierCode& carrier() { return _marketingCarrierCode; }
  const CarrierCode& carrier() const { return _marketingCarrierCode; }

  FlightNumber& flightNumber() { return _marketingFlightNumber; }
  const FlightNumber& flightNumber() const { return _marketingFlightNumber; }

  FlightNumber& marketingFlightNumber() { return _marketingFlightNumber; }
  const FlightNumber& marketingFlightNumber() const { return _marketingFlightNumber; }

  const CarrierCode& marketingCarrierCode() const { return _marketingCarrierCode; }
  void setMarketingCarrierCode(const CarrierCode& marketingCarrierCode) { _marketingCarrierCode = marketingCarrierCode; }
  void swapMarketingCarrierCode(CarrierCode& marketingCarrierCode) { std::swap(_marketingCarrierCode, marketingCarrierCode); }

  FlightNumber& operatingFlightNumber() { return _operatingFlightNumber; }
  const FlightNumber& operatingFlightNumber() const { return _operatingFlightNumber; }

  int8_t arrivalDayAdjust() const { return _arrivalDayAdjust; }
  void setArrivalDayAdjust(int8_t value) { _arrivalDayAdjust = value; }

  const CarrierCode& operatingCarrierCode() const { return _operatingCarrierCode; }
  void setOperatingCarrierCode(const CarrierCode& operatingCarrierCode) { _operatingCarrierCode = operatingCarrierCode; }

  bool eticket() const { return _eticket; }
  bool& eticket() { return _eticket; }

  bool bbrCarrier() const { return _bbrCarrier; }
  bool& bbrCarrier() { return _bbrCarrier; }

  char& marriageStatus() { return _marriageStatus; }
  const char& marriageStatus() const { return _marriageStatus; }

  bool flowJourneyCarrier() const;
  bool localJourneyCarrier() const;
  bool journeyByMarriageCarrier() const;

  bool isSoloShoppingCarrier() const;
  bool isSoloPricingCarrier() const;

  std::vector<uint32_t>& validAvailabilityIds() { return _validAvailabilityIds; }
  const std::vector<uint32_t>& validAvailabilityIds() const { return _validAvailabilityIds; }
};

} // tse namespace

