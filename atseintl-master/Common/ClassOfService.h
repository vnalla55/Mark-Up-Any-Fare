//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------
#pragma once

#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"

namespace tse
{

class ClassOfService
{
public:
  virtual ~ClassOfService() = default;

  friend bool operator==(const ClassOfService& left, const ClassOfService& right)
  {
    return left._bookingCode == right._bookingCode && left._numSeats == right._numSeats &&
               left._cabin == right._cabin;
  }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  uint16_t& numSeats() { return _numSeats; }
  const uint16_t& numSeats() const { return _numSeats; }

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  bool isAvailable(const BookingCode bookingCode, const uint16_t numSeatsRequired) const
  {
    return _bookingCode == bookingCode && _numSeats >= numSeatsRequired;
  }

private:
  BookingCode _bookingCode = "";
  uint16_t _numSeats = 0;
  CabinType _cabin;
};
} // tse namespace

