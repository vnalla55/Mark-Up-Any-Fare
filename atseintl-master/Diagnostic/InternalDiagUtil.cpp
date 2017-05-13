//-------------------------------------------------------------------
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

#include "Diagnostic/InternalDiagUtil.h"

#include "Common/ItinUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

void
InternalDiagUtil::printSegmentInfo(const TravelSeg& ts)
{
  _dc << std::setw(2) << ts.pnrSegment();

  addCarrierFlightCode(ts);
  addDate(ts);
  addCityPair(ts);
}

void
InternalDiagUtil::addIataArea(const TravelSeg& ts)
{
  _dc << " AREAS: FROM " << ts.origin()->area() << " TO " << ts.destination()->area();

  AreaCrossingDeterminator tp;
  tp.determine(_trx, ts);

  if (!tp.transatlanticSectors().empty())
    _dc << " TRANSATL";

  if (!tp.transpacificSectors().empty())
    _dc << " TRANSPAC";

  for (const Loc* loc : ts.hiddenStops())
    _dc << "\n               HIDDEN STOP: " << loc->loc() << " AREA: " << loc->area();
}

void
InternalDiagUtil::addTripCharacteristic(const Itin& itin)
{
  _dc << "TRIP CHARACTERISTIC:";

  if (itin.tripCharacteristics().isSet(Itin::OneWay))
    _dc << " OW,";
  if (itin.tripCharacteristics().isSet(Itin::RoundTrip))
    _dc << " RT,";
  if (itin.tripCharacteristics().isSet(Itin::OriginatesUS))
    _dc << " ORIGIN US,";
  if (itin.tripCharacteristics().isSet(Itin::TerminatesUS))
    _dc << " TERMINATES US,";
  if (itin.tripCharacteristics().isSet(Itin::OriginatesCanadaMaritime))
    _dc << " ORGIN CANADA MARITIME,";
  if (itin.tripCharacteristics().isSet(Itin::USOnly))
    _dc << " US ONLY,";
  if (itin.tripCharacteristics().isSet(Itin::CanadaOnly))
    _dc << " CANADA ONLY,";
  if (itin.tripCharacteristics().isSet(Itin::TurnaroundPointForTax))
    _dc << " TAX TURNAROUND,";
  if (itin.tripCharacteristics().isSet(Itin::RussiaOnly))
    _dc << " RUSSIA ONLY,";
  if (itin.tripCharacteristics().isSet(Itin::RW_SFC))
    _dc << " RW SFC,";
  if (itin.tripCharacteristics().isSet(Itin::CT_SFC))
    _dc << " CT SFC,";
}

void
InternalDiagUtil::addCarrierFlightCode(const TravelSeg& ts)
{
  const AirSeg* tvlSegA = dynamic_cast<const AirSeg*>(&ts);
  if (tvlSegA)
  {
    _dc << std::setw(3) << tvlSegA->carrier();

    (tvlSegA->segmentType() == Open) ? _dc << "OPEN" : _dc << std::setw(4)
                                                           << tvlSegA->flightNumber();

    _dc << tvlSegA->getBookingCode();
  }

  else
    _dc << std::setw(5) << "ARNK"
        << "   ";
}

void
InternalDiagUtil::addDate(const TravelSeg& ts)
{
  _dc << std::setw(8);

  if (ts.segmentType() == Open)
  {
    std::string sDepartureDate = ts.pssDepartureDate();

    if (sDepartureDate != "")
    {
      sDepartureDate += " 12:00";
      DateTime dtDepartureDate = DateTime(sDepartureDate);
      _dc << dtDepartureDate.dateToString(DDMMMYY, "");
    }
    else
      _dc << "NONE";
  }
  else
    _dc << ts.departureDT().dateToString(DDMMMYY, "");
}

void
InternalDiagUtil::addCityPair(const TravelSeg& ts)
{
  _dc << std::setw(4) << ts.origAirport() << ts.destAirport();
}
}
