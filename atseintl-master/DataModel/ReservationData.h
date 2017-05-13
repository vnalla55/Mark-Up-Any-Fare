//-------------------------------------------------------------------
//
//  File:        ReservationData.h
//  Created:     August 10, 2005
//  Authors:     Andrea Yang
//
//  Description: Reservation Data for Journey
//
//  Copyright Sabre 2005
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class Traveler;
class FrequentFlyerAccount;

// Itinerary Data segment
class ReservationSeg
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  FlightNumber& flightNumber() { return _flightNumber; }
  const FlightNumber& flightNumber() const { return _flightNumber; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  PSSDate& pssDepartureDate() { return _pssDepartureDate; }
  const PSSDate& pssDepartureDate() const { return _pssDepartureDate; }

  PSSTime& pssDepartureTime() { return _pssDepartureTime; }
  const PSSTime& pssDepartureTime() const { return _pssDepartureTime; }

  LocCode& origAirport() { return _origAirport; }
  const LocCode& origAirport() const { return _origAirport; }

  PSSDate& pssArrivalDate() { return _pssArrivalDate; }
  const PSSDate& pssArrivalDate() const { return _pssArrivalDate; }

  PSSTime& pssArrivalTime() { return _pssArrivalTime; }
  const PSSTime& pssArrivalTime() const { return _pssArrivalTime; }

  LocCode& destAirport() { return _destAirport; }
  const LocCode& destAirport() const { return _destAirport; }

  std::string& actionCode() { return _actionCode; }
  const std::string& actionCode() const { return _actionCode; }

  uint16_t& numInParty() { return _numInParty; }
  const uint16_t& numInParty() const { return _numInParty; }

  Indicator& marriedSegCtrlId() { return _marriedSegCtrlId; }
  const Indicator& marriedSegCtrlId() const { return _marriedSegCtrlId; }

  uint16_t& marriedGrpNo() { return _marriedGrpNo; }
  const uint16_t& marriedGrpNo() const { return _marriedGrpNo; }

  uint16_t& marriedSeqNo() { return _marriedSeqNo; }
  const uint16_t& marriedSeqNo() const { return _marriedSeqNo; }

  Indicator& pollingInd() { return _pollingInd; }
  const Indicator& pollingInd() const { return _pollingInd; }

  std::string& eticket() { return _eticket; }
  const std::string& eticket() const { return _eticket; }

private:
  CarrierCode _carrier; // Airline Code
  FlightNumber _flightNumber = 0; // Flight Number
  BookingCode _bookingCode; // Class of Service
  LocCode _origAirport; // Board Point
  PSSDate _pssDepartureDate; // Departure date (YYYY-MM-DD)
  PSSTime _pssDepartureTime; //  Departure Time
  PSSDate _pssArrivalDate; // Arrival Date
  PSSTime _pssArrivalTime; // Arrival Time
  LocCode _destAirport; // Off Point
  uint16_t _marriedGrpNo = 0; // Married Group Number
  uint16_t _marriedSeqNo = 0; // Married Sequence Number
  std::string _eticket; // Electronic Tkt Indicators
  std::string _actionCode; // ACtion Code
  uint16_t _numInParty = 0; //  Number in Party
  Indicator _marriedSegCtrlId = 0; // Married segment ctl ID.
  Indicator _pollingInd = 0; // Polling Indicator
};

class RecordLocatorInfo
{
public:
  CarrierCode& originatingCxr() { return _originatingCxr; }
  const CarrierCode& originatingCxr() const { return _originatingCxr; }

  std::string& recordLocator() { return _recordLocator; }
  const std::string& recordLocator() const { return _recordLocator; }

private:
  std::string _recordLocator; // Originating Record Locator
  CarrierCode _originatingCxr; // Origianting Carrier Code
};

class ReservationData
{
public:
  std::string& agent() { return _agent; }
  const std::string& agent() const { return _agent; }

  Indicator& agentInd() { return _agentInd; }
  const Indicator& agentInd() const { return _agentInd; }

  AgencyIATA& agentIATA() { return _agentIATA; }
  const AgencyIATA& agentIATA() const { return _agentIATA; }

  PseudoCityCode& agentPCC() { return _agentPCC; }
  const PseudoCityCode& agentPCC() const { return _agentPCC; }

  LocCode& agentCity() { return _agentCity; }
  const LocCode& agentCity() const { return _agentCity; }

  NationCode& agentNation() { return _agentNation; }
  const NationCode& agentNation() const { return _agentNation; }

  std::string& agentCRS() { return _agentCRS; }
  const std::string& agentCRS() const { return _agentCRS; }

  std::vector<ReservationSeg*>& reservationSegs() { return _reservationSegs; }
  const std::vector<ReservationSeg*>& reservationSegs() const { return _reservationSegs; }

  std::vector<RecordLocatorInfo*>& recordLocators() { return _recordLocators; }
  const std::vector<RecordLocatorInfo*>& recordLocators() const { return _recordLocators; }

  std::vector<Traveler*>& passengers() { return _passengers; }
  const std::vector<Traveler*>& passengers() const { return _passengers; }

  std::vector<FrequentFlyerAccount*>& corpFreqFlyerAccounts() { return _corpFreqFlyerAccounts; }
  const std::vector<FrequentFlyerAccount*>& corpFreqFlyerAccounts() const
  {
    return _corpFreqFlyerAccounts;
  }

  LocCode& pocAirport() { return _pocAirport; }
  const LocCode& pocAirport() const { return _pocAirport; }

  DateTime& pocDepartureDate() { return _pocDepartureDate; }
  const DateTime& pocDepartureDate() const { return _pocDepartureDate; }

  PSSTime& pocDepartureTime() { return _pocDepartureTime; }
  const PSSTime& pocDepartureTime() const { return _pocDepartureTime; }

private:
  PseudoCityCode _agentPCC; // Pseudo city code
  NationCode _agentNation; // Country
  LocCode _agentCity; // City
  LocCode _pocAirport; // Point of Commencement origin A03
  AgencyIATA _agentIATA; // IATA number
  std::string _agent; // Owner/Creator
  std::string _agentCRS; // CRS

  std::vector<ReservationSeg*> _reservationSegs;
  std::vector<RecordLocatorInfo*> _recordLocators;
  std::vector<Traveler*> _passengers;
  std::vector<FrequentFlyerAccount*> _corpFreqFlyerAccounts;

  PSSTime _pocDepartureTime; // Point of Commencement departure time (HHMM)  D02
  DateTime _pocDepartureDate; // Point of Commencement departure date (YYYY-MM-DD) D01

  Indicator _agentInd = 0; // Owner/Creator Ind.
};
} // tse namespace
