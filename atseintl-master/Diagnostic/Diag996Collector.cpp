//----------------------------------------------------------------------------
//  File:        Diag996Collector.C
//
//  Authors:     Kavya Katam
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

#include "Diagnostic/Diag996Collector.h"

#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"

#include <iostream>

namespace tse
{
void
Diag996Collector::printAllBrandsEmptyMsg()
{
  *this << "BOOKING CODE VECTOR FOR ALL BRANDS IS EMPTY\n";
}

void
Diag996Collector::printBrandedFareBookingCodes(const Code<2>& brandId,
                                               const std::vector<BookingCode>& bookingCodes)
{
  *this << "BRAND ID: " << brandId << "\t\t";
  *this << "BOOKING CODES: ";
  for (const BookingCode& bkg : bookingCodes)
  {
    *this << bkg << " ";
  }
  *this << "\n";
}

void
Diag996Collector::printItinDirectionIsReverse(uint16_t id)
{
  *this << "ITIN: " << id << "  CANNOT PROCESS BECAUSE PROCESSING DIRECTION IS REVERSE\n";
}

void
Diag996Collector::printItinId(uint16_t id)
{
  *this << "ITIN: " << id << "\t";
}

void
Diag996Collector::printPassFailMsg(bool pass)
{
  if (pass)
    *this << "PASS\n";
  else
    *this << "FAIL\n";
}

void
Diag996Collector::printSoldOutBrandId(bool soldOut, const Code<2>& brandId)
{
  if (soldOut)
    *this << "BRAND ID " << brandId << " IS SOLD OUT\n";
}

void
Diag996Collector::printSeg(const AirSeg* seg)
{
  *this << std::setw(3) << seg->carrier() << std::setw(4) << seg->flightNumber() << "\t";
}

void
Diag996Collector::printDateTime(const DateTime& date)
{
  *this << "\nDATE PAIR: " << date << "\n";
}
}
