//----------------------------------------------------------------------------
//  File:        Diag996Collector.h
//  Created:     2011-03-03
//    Authors:     Kavya Katam
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

#include "Common/TseCodeTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class AirSeg;

class Diag996Collector : public DiagCollector
{
public:
  void printAllBrandsEmptyMsg();
  void printBrandedFareBookingCodes(const Code<2>& brandId,
                                    const std::vector<BookingCode>& bookingCodes);
  void printItinDirectionIsReverse(uint16_t id);
  void printItinId(uint16_t id);
  void printPassFailMsg(bool pass);
  void printSoldOutBrandId(bool soldOut, const Code<2>& brandId);
  void printSeg(const AirSeg* seg);
  void printDateTime(const DateTime& date);
};
}

