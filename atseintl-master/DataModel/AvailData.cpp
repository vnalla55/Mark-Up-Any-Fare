//-------------------------------------------------------------------
//
//  File:        AvailData.cpp
//  Authors:     Kul Shekhar
//
//  Description: Availability Data for UpSell Entry
//
//  Copyright Sabre 2007
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

#include "DataModel/AvailData.h"

#include "Common/ClassOfService.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
void
FlightAvail::fillClassOfService(PricingTrx& trx)
{
  if (bookingCodeAndSeats().empty())
    return;
  size_t stringSize = bookingCodeAndSeats().size();
  std::string bookingCodeSeatCombo;
  ClassOfService* cs = nullptr;
  const std::string separator("|");
  size_t length = 0, i = 0, startPos = 0;
  for (i = bookingCodeAndSeats().find(separator, 0); i != std::string::npos && i < stringSize;
       i = bookingCodeAndSeats().find(separator, i))
  {
    length = i - startPos;
    bookingCodeSeatCombo = bookingCodeAndSeats().substr(startPos, length);
    cs = getCos(trx, bookingCodeSeatCombo);
    if (cs != nullptr)
      classOfService().push_back(cs);
    i++;
    startPos = i;
  }
  bookingCodeSeatCombo = bookingCodeAndSeats().substr(startPos, stringSize - startPos);
  cs = getCos(trx, bookingCodeSeatCombo);
  if (cs != nullptr)
    classOfService().push_back(cs);
  return;
}

//-------------------------------------------------------------------
ClassOfService*
FlightAvail::getCos(PricingTrx& trx, std::string bookingCodeSeatCombo)
{
  BookingCode bookingCode;
  std::string numSeat;

  for (size_t i = 0; i < bookingCodeSeatCombo.size(); i++)
  {
    if (i == 0)
    {
      if (isalpha(bookingCodeSeatCombo[i]))
        bookingCode.push_back(bookingCodeSeatCombo[i]);
      else
        return nullptr;
    }
    else if (i == 1)
    {
      if (isDigit(bookingCodeSeatCombo[i]))
        numSeat.push_back(bookingCodeSeatCombo[i]);
      else if (isalpha(bookingCodeSeatCombo[i]))
        bookingCode.push_back(bookingCodeSeatCombo[i]);
      else
        return nullptr;
    }
    else
    {
      if (isDigit(bookingCodeSeatCombo[i]))
        numSeat.push_back(bookingCodeSeatCombo[i]);
      else
        return nullptr;
    }
  }

  ClassOfService* cs = nullptr;
  trx.dataHandle().get(cs);
  if (cs == nullptr)
    return nullptr;
  cs->numSeats() = static_cast<uint16_t>(atoi(numSeat.c_str()));
  cs->bookingCode() = bookingCode;
  return cs;
}
} // tse namespace
