//----------------------------------------------------------------------------
//  File:        Diag676Collector.h
//
//  Copyright Sabre 2014
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

#include "Common/MultiTicketUtil.h"
#include "Diagnostic/DiagCollector.h"
#include "FareCalc/CalcTotals.h"

#include <vector>

namespace tse
{
class TravelSeg;
class Itin;

class Diag676Collector : public DiagCollector
{
public:
  void printHeader() override;
  void printAllItinAnalysis();
  void displayItin(Itin& itin, int& a);
  void displayStopoverSegmentHeader();
  void displaySegment(TravelSeg& tvl, bool a = false);
  void displayTotalCountStopover(size_t& a);
  void displaySubItinStartBuild(Itin& itin);
  void displayProposedMultiItins(std::vector<TravelSeg*>& t1, std::vector<TravelSeg*>& t2);
  void displayCheckSubItinsMsg();
  void displayRemoveArunkAtFirstItins(std::vector<TravelSeg*>& t1, TravelSeg* tvl);
  void displayRemoveArunkAtSecondItins(std::vector<TravelSeg*>& t2, TravelSeg* tvl);
  void displayForcedSideTrip(TravelSeg* tvl1, TravelSeg* tvl2);
  void displaySubItinsCreated(Itin& itin, Itin& itin2);
  void displaySubItinNotBuildMsg();
  void printItinFooter();
  void printAllItinFooter();
  void printFooter();
  void printHeaderForTotal();
  void printFooterForTotal();
  void displayItinTravelSeg(Itin& itin);
  void printSingleTicketHeader();
  void printMultiTicketHeader();
  void displayFinalAmount(Money singleTktAmt, Money multiTktAmt);
  void displaySolution(MultiTicketUtil::TicketSolution ticketSolution);
  void displayAmount(const FarePath& fPath, const CalcTotals& calcTotals);
  void displayTotalAmount(Money amount);
  void displayMultiTicketTotalAmount(Money amount);

private:
  void displaySegments(const std::vector<TravelSeg*>& t1);
};
}

