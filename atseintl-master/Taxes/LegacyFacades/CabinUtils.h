// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Taxes/AtpcoTaxes/DataModel/Common/SafeEnums.h"

namespace tse
{
class CabinType;
class FareUsage;
class Itin;
class TravelSeg;

class CabinUtils
{
public:
  static tax::type::CabinCode translateCabin(const CabinType& cabin);

  static std::pair<BookingCode, tax::type::CabinCode>
  getBookingCodeAndCabin(const Itin& itin,
                         const FareUsage& fareUsage,
                         const TravelSeg* const travelSegment);
};
}

