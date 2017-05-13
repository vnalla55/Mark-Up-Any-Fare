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
#pragma once

#include "DataModel/BaggageTravel.h"
#include "FreeBagService/BaggageItinAnalyzer.h"

namespace tse
{
class Itin;

class BaggageItinAnalyzerResponseBuilder
{
  friend class BaggageItinAnalyzerResponseBuilderTest;

public:
  BaggageItinAnalyzerResponseBuilder(const BaggageItinAnalyzer& itinAnalyzer)
    :_analyzer(itinAnalyzer) {}

  void printItinAnalysis(std::ostringstream& output, const Itin& itin);

private:
  void printCheckedPoint(const CheckedPoint& checkedPoint);

  void printCheckedPoints(const CheckedPointVector& checkedPoints,
                          const TravelSeg* furthestTicketedPoint);

  void
  printFurthestCheckedPoint(const CheckedPointVector& cp, const CheckedPoint& furthestCheckedPoint);

  void printOriginAndDestination(const Itin& itin);

  void printBaggageTravels(const std::vector<BaggageTravel*>& baggageTravels,
                           const CheckedPointVector& cp,
                           bool isUsDot);

  void printBaggageTravel(const BaggageTravel* baggageTravel,
                          const CheckedPointVector& cp,
                          bool isUsDot,
                          uint32_t index);

  std::ostringstream _output;
  const BaggageItinAnalyzer& _analyzer;
};

} // tse
