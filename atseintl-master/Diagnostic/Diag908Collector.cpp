//----------------------------------------------------------------------------
//  File:        Diag908Collector.C
//  Created:     2004-08-20
//
//  Description: Diagnostic 908 formatter
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
#include "Diagnostic/Diag908Collector.h"

#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/DiagCollector.h"

#include <iomanip>

using namespace std;
namespace tse
{
Diag908Collector&
Diag908Collector::operator<<(const ShoppingTrx::Leg& leg)
{
  if (leg.sop().empty())
  {
    *this << "EMPTY LEG\n\n";
  }

  *this << "PREFERRED CABIN: " << leg.preferredCabinClass().printName() << endl;

  // set all fields to left-justified
  setf(std::ios::left, std::ios::adjustfield);

  for (const auto& elem : leg.sop())
  {
    if (true == elem.itin()->isDummy())
    {
      *this << "SOP - "
            << "DUMMY FLIGHT"
            << "\n";
    }
    else
    {
      *this << "SOP - " << elem.originalSopId() << "\n";

      if (PricingTrx::FF_TRX == _trx->getTrxType())
      {
        ShoppingUtil::prepareFFClassOfService(const_cast<ShoppingTrx::SchedulingOption&>(elem));
      }
    }

    const Itin& itin = *elem.itin();
    const bool valid = elem.cabinClassValid();

    const std::vector<TravelSeg*>& travelSeg = itin.travelSeg();

    if (travelSeg.empty())
    {
      *this << "TRAVEL SEGMENT EMPTY";
    }

    for (std::vector<TravelSeg*>::const_iterator j = travelSeg.begin(); j != travelSeg.end(); ++j)
    {
      const AirSeg* const airSeg = dynamic_cast<const AirSeg*>(*j);

      if (airSeg == nullptr)
      {
        continue;
      }

      *this << airSeg->carrier() << std::setw(4) << airSeg->flightNumber() << " "
            << (*j)->getBookingCode() << (*j)->bookedCabin() << " "
            << airSeg->departureDT().dateToString(DDMMM, "") << " " << airSeg->origin()->loc()
            << airSeg->destination()->loc() << " ";

      const std::vector<ClassOfService*>& classOfService = (*j)->classOfService();
      std::ostringstream bookingCode;
      //--------------------------------//

      for (const auto cos : classOfService)
      {
        bookingCode << cos->bookingCode();
        bookingCode << cos->numSeats();
      }

      const size_t BookingCodeFieldSize = 38;
      std::string bookingCodeStr = bookingCode.str();

      const size_t bookingCodeLen = std::min(bookingCodeStr.size(), BookingCodeFieldSize);
      *this << std::setw(BookingCodeFieldSize) << bookingCodeStr.substr(0, bookingCodeLen);
      bookingCodeStr.erase(bookingCodeStr.begin(), bookingCodeStr.begin() + bookingCodeLen);

      if (j == travelSeg.begin())
      {
        *this << " " << (valid ? "P" : "F");
      }

      *this << "\n";

      while (bookingCodeStr.empty() == false)
      {
        const size_t bookingCodeLen = std::min(bookingCodeStr.size(), BookingCodeFieldSize);
        *this << std::setw(23) << " " << std::setw(BookingCodeFieldSize)
              << bookingCodeStr.substr(0, bookingCodeLen) << "\n";
        bookingCodeStr.erase(bookingCodeStr.begin(), bookingCodeStr.begin() + bookingCodeLen);
      }
    }

    *this << "\n";
  }

  return *this;
}

Diag908Collector&
Diag908Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (!_active)
  {
    return *this;
  }

  *this << "***************************************************" << std::endl;
  *this << "908 : CARRIER CABIN WITH PASSED/FAILED FLIGHT VECTOR" << std::endl;
  *this << "      FOR BOOKING CODE CABIN VALIDATION" << std::endl;
  *this << "      P : PASS AND F : FAIL" << std::endl;
  *this << "***************************************************" << std::endl;
  *this << "" << std::endl;
  *this << "CABIN DEFINITION" << std::endl;
  CabinType cabinType;
  cabinType.setClass('1');
  *this << "1 - " << cabinType.printName() << std::endl;
  cabinType.setClass('2');
  *this << "2 - " << cabinType.printName() << std::endl;
  cabinType.setClass('4');
  *this << "4 - " << cabinType.printName() << std::endl;
  cabinType.setClass('5');
  *this << "5 - " << cabinType.printName() << std::endl;
  cabinType.setClass('7');
  *this << "7 - " << cabinType.printName() << std::endl;
  cabinType.setClass('8');
  *this << "8 - " << cabinType.printName() << std::endl;
  *this << "" << std::endl;

  if (shoppingTrx.legs().empty())
  {
    *this << "NO DATA TO DISPLAY\n";
  }
  std::vector<ShoppingTrx::Leg>::const_iterator i = shoppingTrx.legs().begin();

  const FlightFinderTrx* ffTrx = dynamic_cast<const FlightFinderTrx*>(&shoppingTrx);
  if (ffTrx != nullptr &&
      (ffTrx->bffStep() == FlightFinderTrx::STEP_4 || ffTrx->bffStep() == FlightFinderTrx::STEP_6))
  {
    ++i;
  }

  for (; i != shoppingTrx.legs().end(); ++i)
  {
    const Itin& itin = *i->sop().front().itin();

    const int num = i + 1 - shoppingTrx.legs().begin();
    *this << "LEG " << num << " " << itin.travelSeg().front()->boardMultiCity() << "-"
          << itin.travelSeg().back()->offMultiCity() << "\n\n";
    *this << *i;
  }

  return *this;
}

void
Diag908Collector::printPrimeSeg(uint32_t legId, const TravelSeg* primeSeg, const Indicator& ind)
{
  *this << "LEG"<<legId<<". PRIME SEG: " << primeSeg->origin()->loc()
        << "-" << primeSeg->destination()->loc()
        << ". CABIN: " << ind << "\n";
}
}
