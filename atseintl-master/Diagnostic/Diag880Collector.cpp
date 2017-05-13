//----------------------------------------------------------------------------
//  File:        Diag880Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 880 Service Fee - OC Fees Slice and Dice
//
//  Updates:
//
//
//  Copyright Sabre 2010
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
#include "Diagnostic/Diag880Collector.h"

#include "Common/Money.h"
#include "DataModel/AirSeg.h"

#include <iomanip>

namespace tse
{
void
Diag880Collector::printHeader(const ServiceGroup groupCode,
                              ServiceSubTypeCode subCode,
                              PaxTypeCode paxTypeCode,
                              std::vector<TravelSeg*>::const_iterator first,
                              const std::vector<TravelSeg*>::const_iterator end,
                              uint16_t multiTktNbr)
{
  const std::string& diagSG = rootDiag()->diagParamMapItem(Diagnostic::SRV_GROUP);
  const std::string& diagSC = rootDiag()->diagParamMapItem(Diagnostic::SRV_CODE);

  _filteredPass = true;
  _filteredPass = diagSG.empty() ? _filteredPass : diagSG == groupCode;
  _filteredPass = diagSC.empty() ? _filteredPass : diagSC == subCode;
  if (!_filteredPass)
    return;
  DiagCollector& dc = *this;

  dc << "*********************** SLICE AND DICE FOR ********************\n";
  dc << "GROUP: " << groupCode << "    SUBCODE: " << subCode
     << "    REQUESTED PAXTYPE: " << paxTypeCode << "\n";
  dc << "----------------------- PORTION OF TRAVEL ---------------------\n";
  if (multiTktNbr == 1)
    dc << "TKT1\n";
  else if (multiTktNbr == 2)
    dc << "TKT2\n";
  printTravelRoute(first, end);
  dc << "\n";
  dc << "------------------------ FLIGHT SEGMENTS ----------------------\n";
  typedef std::vector<TravelSeg*>::const_iterator TSIt;
  for (TSIt tsIt = first; tsIt != end; ++tsIt)
  {
    dc << (*tsIt)->origAirport() << " " << (*tsIt)->destAirport();
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*tsIt);
    if (seg)
    {
      dc << "    M-" << seg->marketingCarrierCode() << "  O-" << seg->operatingCarrierCode();
    }
    dc << "\n";
  }
}

void
Diag880Collector::printSectorsHeader(uint16_t skippedSegs)
{
  DiagCollector& dc = *this;

  switch (skippedSegs)
  {
  case 0:
    dc << "---------------------- ALL SECTORS COVERED --------------------\n";
    break;
  case 1:
    dc << "----------------------1 SECTOR MISSING ------------------------\n";
    break;
  default:
    dc << "-------------------" << std::setfill('-') << std::right << std::setw(4) << skippedSegs
       << " SECTORS MISSING -----------------------\n";
    break;
  }
}

void
Diag880Collector::printSliceAndDiceMatrix(TseUtil::SolutionSet& solutions, uint32_t maxDiagSize)
{
  if (!_filteredPass || solutions.empty())
    return;
  DiagCollector& dc = *this;

  TseUtil::SolutionSet::nth_index<0>::type& index = solutions.get<0>();

  dc << "----------------------- SLICE-DICE MATRIX ---------------------\n";
  typedef TseUtil::SolutionSet::nth_index<0>::type::const_iterator SolutionIt;
  uint16_t i = 1;

  uint16_t skkiped = index.begin()->_skipped;
  printSectorsHeader(skkiped);
  if (rootDiag()->shouldDisplay((*(index.begin()->_routes).front()->first)->origAirport(),
                                (*(index.begin()->_routes).front()->first)->boardMultiCity(),
                                (*((index.begin()->_routes).back()->second - 1))->destAirport(),
                                (*((index.begin()->_routes).back()->second - 1))->offMultiCity()))
  {
    dc << i++ << ". ";
    for (const auto route : index.begin()->_routes)
      dc << (*route->first)->boardMultiCity() << " " << (*(route->second - 1))->offMultiCity()
         << "  ";
    dc << "\n";
  }

  for (SolutionIt solIt = ++index.begin(); solIt != index.end() && str().size() < maxDiagSize;
       ++solIt)
  {
    if (skkiped != solIt->_skipped)
    {
      skkiped = solIt->_skipped;
      printSectorsHeader(solIt->_skipped);
    }
    if (!rootDiag()->shouldDisplay((*(solIt->_routes).front()->first)->origAirport(),
                                   (*(solIt->_routes).front()->first)->boardMultiCity(),
                                   (*((solIt->_routes).back()->second - 1))->destAirport(),
                                   (*((solIt->_routes).back()->second - 1))->offMultiCity()))
      continue;
    dc << i++ << ". ";
    for (const auto route : solIt->_routes)
      dc << (*route->first)->boardMultiCity() << " " << (*(route->second - 1))->offMultiCity()
         << "  ";
    dc << "\n";
  }
  dc << "***************************************************************\n";
}

void
Diag880Collector::printTravelRoute(std::vector<TravelSeg*>::const_iterator first,
                                   const std::vector<TravelSeg*>::const_iterator end)
{
  DiagCollector& dc = *this;

  if (first == end)
    return;
  dc << (*first)->origAirport();

  typedef std::vector<TravelSeg*>::const_iterator TSIt;
  for (TSIt tsIt = first; tsIt != end; ++tsIt)
  {
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*tsIt);
    if (seg)
      dc << "-" << seg->marketingCarrierCode() << "-" << (*tsIt)->destAirport();
    else
      dc << "//" << (*tsIt)->destAirport();
  }
}

void
Diag880Collector::printSolution(const std::vector<TvlSegPair*>& routes, uint16_t counter)
{
  _filteredFMPass = rootDiag()->shouldDisplay((*routes.front()->first)->origAirport(),
                                              (*routes.front()->first)->boardMultiCity(),
                                              (*(routes.back()->second - 1))->destAirport(),
                                              (*(routes.back()->second - 1))->offMultiCity());

  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  dc << counter << ". ";
  for (const auto route : routes)
  {
    printTravelRoute(route->first, route->second);
    dc << "  ";
  }
  dc << "\n \n";
}

void
Diag880Collector::printPiece(const TvlSegPair& route,
                             Money feeMoney,
                             const CarrierCode& carrierCode,
                             const bool isMarketing,
                             const DateTime& tktDate)
{
  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  printTravelRoute(route.first, route.second);
  if (route.first == route.second)
    return;
  std::ostringstream operatingStream;

  dc << "    M- ";
  const AirSeg* seg = dynamic_cast<const AirSeg*>(*route.first);
  if (!seg)
    return;
  dc << seg->marketingCarrierCode();
  operatingStream << "  O- " << seg->operatingCarrierCode();

  typedef std::vector<TravelSeg*>::const_iterator TSIt;
  for (TSIt tsIt = route.first + 1; tsIt != route.second; ++tsIt)
  {
    seg = dynamic_cast<const AirSeg*>(*tsIt);
    if (!seg)
      continue;
    dc << "/" << seg->marketingCarrierCode();
    operatingStream << "/" << seg->operatingCarrierCode();
  }
  dc << operatingStream.str() << "\n"
     << "CXR: " << carrierCode << " - " << (isMarketing ? "MARKETING" : "OPERATING") << "    FEE: ";
  if (feeMoney.value() - EPSILON < 0.0 && feeMoney.value() + EPSILON > 0.0)
    dc << "0";
  else
    dc << feeMoney.toString(tktDate);
  dc << "\n \n";
}

void
Diag880Collector::printTravelAndMessage(const TvlSegPair& route, const std::string& message)
{
  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  printTravelRoute(route.first, route.second);
  dc << "\n" << message << "\n \n";
}

void
Diag880Collector::printPieceNotFound(const TvlSegPair& route)
{
  printTravelAndMessage(route, "NOT FOUND");
}

void
Diag880Collector::printHardMatchFound(const TvlSegPair& route)
{
  printTravelAndMessage(route, "HARD MATCH FOUND");
}

void
Diag880Collector::printSoftMatchFound(const TvlSegPair& route)
{
  printTravelAndMessage(route, "SOFT MATCH FOUND");
}

void
Diag880Collector::printResult(bool isWinning)
{
  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  dc << "STATUS: " << (isWinning ? "PASS" : "FAIL") << "\n"
     << "***************************************************************\n";
}

void
Diag880Collector::printResult(Money feeMoney, bool isWinning, const DateTime& tktDate)
{
  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  if (feeMoney.value() - EPSILON < 0.0 && feeMoney.value() + EPSILON > 0.0)
    dc << "TOTAL: 0\n";
  else
    dc << "TOTAL: " << feeMoney.toString(tktDate) << "\n";
  dc << "STATUS: " << (isWinning ? "PASS - LOWEST FEE/MOST FLIGHT SEGMENTS SELECTED"
                                 : "SOFT PASS - NOT LOWEST FEE OR TOO FEW FLIGHT SEGMENTS") << "\n"
     << "***************************************************************\n";
}

void
Diag880Collector::printResultFail(Money feeMoney, const DateTime& tktDate)
{
  if (!_filteredPass || !_filteredFMPass)
    return;
  DiagCollector& dc = *this;

  if (feeMoney.value() - EPSILON < 0.0 && feeMoney.value() + EPSILON > 0.0)
    dc << "TOTAL: 0\n";
  else
    dc << "TOTAL: " << feeMoney.toString(tktDate) << "\n";
  dc << "STATUS: SOFT PASS - NOT ALL PORTION MATCHED\n"
     << "***************************************************************\n";
}

} // namespace
