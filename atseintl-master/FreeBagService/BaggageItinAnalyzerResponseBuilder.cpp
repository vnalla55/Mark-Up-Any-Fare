//-------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include "FreeBagService/BaggageItinAnalyzerResponseBuilder.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/BaggageTripType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"


namespace tse
{
FALLBACK_DECL(fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852);

void
BaggageItinAnalyzerResponseBuilder::printItinAnalysis(std::ostringstream& output, const Itin& itin)
{
  const BaggageTripType baggageTripType = itin.getBaggageTripType();

  _output.clear();
  _output.str(std::string());

  _output << "-----------------ITINERARY ANALYSIS---------------------\n";
  _output << "STOPOVER POINTS :";

  printCheckedPoints(_analyzer.checkedPoints(), _analyzer.furthestTicketedPoint());

  if (!_analyzer.checkedPoints().empty())
  {
    _output << "FURTHEST STOPOVER POINT : ";
    printFurthestCheckedPoint(_analyzer.checkedPoints(), _analyzer.furthestCheckedPoint());
  }

  printOriginAndDestination(itin);

  _output << "JOURNEY TYPE : " << baggageTripType.getJourneyName(itin) << "\n";
  _output << "--------------BAGGAGE ALLOWANCE/CHARGES-----------------\n";

  printBaggageTravels(
      _analyzer.baggageTravels(), _analyzer.checkedPoints(), baggageTripType.isUsDot());

  output << _output.str();
}

void
BaggageItinAnalyzerResponseBuilder::printCheckedPoint(const CheckedPoint& checkedPoint)
{
  const Loc* loc = (checkedPoint.second == CP_AT_ORIGIN) ? (*(checkedPoint.first))->origin()
                                                         : (*(checkedPoint.first))->destination();
  _output << loc->loc();
}

void
BaggageItinAnalyzerResponseBuilder::printCheckedPoints(const CheckedPointVector& checkedPoints,
                                                       const TravelSeg* furthestTicketedPoint)
{
  for (const CheckedPoint& checkedPoint : checkedPoints)
  {
    // Skip furthest ticketed point. Print it as last checked point
    if (furthestTicketedPoint && (*(checkedPoint.first)) == furthestTicketedPoint)
      continue;

    _output << " ";
    printCheckedPoint(checkedPoint);
  }

  if (furthestTicketedPoint)
    _output << " " << furthestTicketedPoint->destination()->loc() << "*";

  _output << "\n";
}

void
BaggageItinAnalyzerResponseBuilder::printFurthestCheckedPoint(
    const CheckedPointVector& cp, const CheckedPoint& furthestCheckedPoint)
{
  const Loc* furthestCheckedPointLoc = (furthestCheckedPoint.second == CP_AT_ORIGIN)
                                           ? (*(furthestCheckedPoint.first))->origin()
                                           : (*(furthestCheckedPoint.first))->destination();

  _output << furthestCheckedPointLoc->loc() << "\n";
}

void
BaggageItinAnalyzerResponseBuilder::printOriginAndDestination(const Itin& itin)
{
  const std::vector<TravelSeg*>& travelSegs = itin.travelSeg();
  std::vector<TravelSeg*>::const_iterator origI;
  std::vector<TravelSeg*>::const_reverse_iterator destI;

  if (!itin.getBaggageTripType().isUsDot())
  {
    origI = std::find_if(travelSegs.begin(),
                         travelSegs.end(),
                         std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation)));
    destI = std::find_if(travelSegs.rbegin(),
                         travelSegs.rend(),
                         std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation)));
  }
  else
  {
    origI = travelSegs.begin();
    destI = travelSegs.rbegin();
  }

  if (origI == travelSegs.end())
    _output << "NO AIR TRANSPORTATION\n";
  else
  {
    _output << "ORIGIN : " << (*origI)->origin()->loc() << "\n";
    _output << "DESTINATION : " << (*destI)->destination()->loc() << "\n";
  }
}

void
BaggageItinAnalyzerResponseBuilder::printBaggageTravels(
    const std::vector<BaggageTravel*>& baggageTravels, const CheckedPointVector& cp, bool isUsDot)
{
  for (uint32_t i = 0; i < baggageTravels.size(); ++i)
  {
    if (i != 0)
      _output << "--------------------------------------------------------\n";

    printBaggageTravel(baggageTravels[i], cp, isUsDot, i + 1);
  }
}

void
BaggageItinAnalyzerResponseBuilder::printBaggageTravel(const BaggageTravel* baggageTravel,
                                                       const CheckedPointVector& cp,
                                                       bool isUsDot,
                                                       uint32_t index)
{
  const char prevFillChar = _output.fill('0');
  _output << setiosflags(std::ios::right) << std::setw(2) << index << resetiosflags(std::ios::right)
          << " BAGGAGE TRAVEL : ";
  _output.fill(prevFillChar);

  if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(baggageTravel->_trx))
    BaggageStringFormatter::old_printBaggageTravelSegments(*baggageTravel, _output);
  else
    _output << BaggageStringFormatter::printBaggageTravelSegmentsWithoutNumbering(*baggageTravel);

  _output << "\n";
  BaggageStringFormatter::printBtCarriers(*baggageTravel, isUsDot, false, _output);

  _output << "STOPOVER POINTS : ";

  for (TravelSegPtrVecCI travelSegIter = baggageTravel->getTravelSegBegin();
       travelSegIter != baggageTravel->getTravelSegEnd();
       ++travelSegIter)
  {
    for (const CheckedPoint& checkedPoint : cp)
    {
      if (checkedPoint.first == travelSegIter)
      {
        if ((travelSegIter == baggageTravel->getTravelSegBegin() &&
             checkedPoint.second == CP_AT_ORIGIN) ||
            (travelSegIter == baggageTravel->getTravelSegEnd() - 1 &&
             checkedPoint.second == CP_AT_DESTINATION))
          continue;

        printCheckedPoint(checkedPoint);
        _output << " ";
      }
    }
  }
  _output << "\n";
}
}
