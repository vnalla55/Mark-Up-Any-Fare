//-------------------------------------------------------------------
//
//  File:        AvailData.h
//  Created:     Jul 31, 2006
//  Authors:     Andrea Yang
//
//  Description: Availability Data for UpSell Entry
//
//  Copyright Sabre 2006
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
#include "Common/TseStringTypes.h"

namespace tse
{
class ClassOfService;
class PricingTrx;

class FlightAvail
{
public:
  int16_t& upSellPNRsegNum() { return _upSellPNRsegNum; }
  const int16_t& upSellPNRsegNum() const { return _upSellPNRsegNum; }

  std::string& bookingCodeAndSeats() { return _bookingCodeAndSeats; }
  const std::string& bookingCodeAndSeats() const { return _bookingCodeAndSeats; }

  int16_t& availMethod() { return _availMethod; }
  const int16_t& availMethod() const { return _availMethod; }

  std::vector<ClassOfService*>& classOfService() { return _classOfService; }
  const std::vector<ClassOfService*>& classOfService() const { return _classOfService; }

  void fillClassOfService(PricingTrx& trx);

private:
  std::string _bookingCodeAndSeats; // Q17 - Flight Booking Codes and Num of Seats
  std::vector<ClassOfService*> _classOfService;
  int16_t _upSellPNRsegNum = 0; // Q1K - Flight pnr segment Number
  int16_t _availMethod = 0; // Q3F - Availability Method

  ClassOfService* getCos(PricingTrx& trx, std::string bookingCodeSeatCombo);
};

class AvailData
{
public:
  std::vector<FlightAvail*>& flightAvails() { return _flightAvails; }
  const std::vector<FlightAvail*>& flightAvails() const { return _flightAvails; }

private:
  std::vector<FlightAvail*> _flightAvails;
};

} // tse namespace

