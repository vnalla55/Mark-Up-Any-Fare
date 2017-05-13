//----------------------------------------------------------------------------
//  File:        Diag676Collector.cpp
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

#include "Diagnostic/Diag676Collector.h"

#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"

using namespace std;
namespace tse
{

void
Diag676Collector::printHeader()
{
  //          1         2         3         4         5         6
  // 1234567890123456789012345678901234567890123456789012345678901234
  if (_active)
  {
    *this << "\n*****************   MULTI-TICKET ANALYSIS   *****************\n";
  }
}

void
Diag676Collector::printAllItinAnalysis()
{
  //          1         2         3         4         5         6
  // 1234567890123456789012345678901234567890123456789012345678901234
  if (_active)
  {
    *this << "\n*---------------- ALL ITINS ANALYSIS   -------------------*\n";
  }
}

void
Diag676Collector::displayItin(Itin& itin, int& a)
{
  if (_active)
  {
    *this << "\n";
    *this << "*   ITINERARY - " << a << " : ALL TRAVEL SEGMENTS\n";
    if (itin.travelSeg().empty())
      *this << "**  NO SEGMENTS FOUND\n";
    else
      displaySegments(itin.travelSeg());
  }
}

void
Diag676Collector::displayStopoverSegmentHeader()
{
  if (_active)
  {
    *this << "\n*        STOPOVER SEGMENTS\n";
  }
}

void
Diag676Collector::displayTotalCountStopover(size_t& a)
{
  if (_active)
  {
    //          1         2         3         4         5         6
    // 1234567890123456789012345678901234567890123456789012345678901234
    if (a > 0)
    {
      *this << "\n*        TOTAL NUMBER OF STOPOVERS - " << a << "\n";
      if (a > 1)
        *this << "*\n*  MULTI DESTINATION NOT VALID FOR MULTI-TICKET *\n";
    }
    else
      *this << "*\n*  NO STOPOVER FOUND - NOT VALID FOR MULTI-TICKET *\n";
  }
}

void
Diag676Collector::displaySubItinStartBuild(Itin& itin)
{
  if (_active)
    *this << "*\n*     SUB-ITINERARIES BUILD PROCESS STARTS\n";
}

void
Diag676Collector::displayProposedMultiItins(std::vector<TravelSeg*>& t1,
                                            std::vector<TravelSeg*>& t2)
{
  if (_active)
  {
    *this << "*     SUB-ITINERARY - 1 :\n";

    if (t1.size() > 0)
    {
      displaySegments(t1);
    }
    else
      *this << "*  NO SEGMENTS ARE SELECTED\n";

    if (t2.size() > 0)
    {
      *this << "*     SUB-ITINERARY - 2 :\n";
      displaySegments(t2);
    }
    else
      *this << "*  NO SEGMENTS ARE SELECTED\n";
  }
}

void
Diag676Collector::displayCheckSubItinsMsg()
{
  if (_active)
    *this << "*\n* CHECK SUB-ITINS FOR START/END ARUNK/SIDE TRIP\n";
}

void
Diag676Collector::displayRemoveArunkAtFirstItins(std::vector<TravelSeg*>& t1, TravelSeg* tvl)
{
  if (_active)
    *this << "\n*     REMOVING LAST ARUNK SEGMENT AT SUB-ITIN 1\n";
}

void
Diag676Collector::displayRemoveArunkAtSecondItins(std::vector<TravelSeg*>& t2, TravelSeg* tvl)
{
  if (_active)
    *this << "\n*     REMOVING FIRST ARUNK SEGMENT AT SUB-ITIN 2\n";
}

void
Diag676Collector::displayForcedSideTrip(TravelSeg* tvl1, TravelSeg* tvl2)
{
  if (_active)
    *this << "\n* SUB-ITINS ARE NOT CREATED\n"
          << "* SIDE TRIP NOT ALLOWED ON FIRST/LAST SEGMENT\n";
}

void
Diag676Collector::displaySubItinsCreated(Itin& itin1, Itin& itin2)
{
  if (_active)
    *this << "\n* SUB-ITINS 1 AND 2 ARE CREATED\n";
}

void
Diag676Collector::displaySubItinNotBuildMsg()
{
  if (_active)
    *this << "*\n* SUB-ITINS 1 AND 2 ARE NOT CREATED\n";
}

void
Diag676Collector::printItinFooter()
{
  if (_active)
    *this << "\n*---------------   ITINERARY ANALYSIS DONE  ---------------*\n";
}

void
Diag676Collector::printAllItinFooter()
{
  if (_active)
    *this << "\n*-------- MULTI-TICKET ALL ITIN ANALYSIS COMPLETED --------*\n";
}

void
Diag676Collector::printFooter()
{
  if (_active)
    *this << "\n*** END DIAG 676 FOR MULTI-TICKET PROCESSING ***\n";
}

void
Diag676Collector::displaySegments(const std::vector<TravelSeg*>& tvlSegs)
{
  if (_active)
  {
    std::vector<TravelSeg*>::const_iterator tvls = tvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator tvle = tvlSegs.end();
    for (; tvls != tvle; ++tvls)
    {
      displaySegment(**tvls, true);
    }
  }
}

void
Diag676Collector::displaySegment(TravelSeg& tvlSeg, bool displayAll)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&tvlSeg);
    if (airSeg == nullptr)
    {
      dc << "   ARUNK   ";
      if (&tvlSeg != nullptr)
      {
        dc << tvlSeg.origAirport() << "-" << tvlSeg.destAirport();
      }
      dc << std::endl;
      return;
    }
    dc << " " << tvlSeg.pnrSegment();
    dc << " " << airSeg->carrier();
    dc << setw(4) << airSeg->flightNumber();
    dc << setw(2) << airSeg->getBookingCode();
    dc << " " << setw(5) << airSeg->departureDT().dateToString(DDMMM, "");
    dc << " " << airSeg->origAirport() << airSeg->destAirport();
    dc << " " << setw(6) << airSeg->departureDT().timeToString(HHMM_AMPM, "");
    dc << " " << setw(6) << airSeg->arrivalDT().timeToString(HHMM_AMPM, "");
    if (!displayAll)
    {
      dc << "\n";
      return;
    }
    std::string conStop("");
    if (airSeg->stopOver() || airSeg->isForcedStopOver())
      conStop = "O-";
    else
      conStop = "X-";

    conStop = conStop + tvlSeg.destAirport();
    dc << " " << conStop;
    dc << " " << DiagnosticUtil::geoTravelTypeTo3CharString(airSeg->geoTravelType());
    dc << " " << airSeg->marriageStatus();
    dc << " \n";
  }
}

void
Diag676Collector::printHeaderForTotal()
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
        //          1         2         3         4         5         6
        // 1234567890123456789012345678901234567890123456789012345678901234
    dc << "***************************************************************\n";
    dc << "*        MULTI-TICKET SOLUTION ANALYSIS DIAGNOSTIC 676        *\n";
    dc << "***************************************************************\n";
  }
}

void
Diag676Collector::printFooterForTotal()
{
  if (_active)
    *this << "******************   END OF DIAGNOCTIC 676   ******************\n";
}

void
Diag676Collector::displayItinTravelSeg(Itin& itin)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    if (itin.getMultiTktItinOrderNum() == 1 ||
        itin.getMultiTktItinOrderNum() == 2)
    {
      if (itin.getMultiTktItinOrderNum() == 2)
        dc << "---------------------------------------------------------------\n";
      dc << "TKT" << itin.getMultiTktItinOrderNum() << "\n";
    }
    if (itin.travelSeg().empty())
      dc << "**  NO SEGMENTS FOUND";
    else
      displaySegments(itin.travelSeg());
    dc << std::endl;
  }
}

void
Diag676Collector::printSingleTicketHeader()
{
  if (_active)
    *this << "SINGLE TICKET PROCESSING:" << std::endl;
}

void
Diag676Collector::printMultiTicketHeader()
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "***************************************************************\n";
    dc << "MULTIPLE TICKET PROCESSING:\n";
  }
}

void
Diag676Collector::displayFinalAmount(Money singleTicketTotal,
                                     Money multiTicketTotal)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "***************************************************************\n";
    if (singleTicketTotal.code() != INVALID_CURRENCYCODE ||
        multiTicketTotal.code() != INVALID_CURRENCYCODE)
    {
      dc << "FINAL AMOUNT COMPARISON:\n";
      if (singleTicketTotal.code() == INVALID_CURRENCYCODE)
        dc << "SINGLE TICKET TOTAL NOT AVAILABLE\n";
      else
        dc << "SINGLE TICKET TOTAL " << Money(singleTicketTotal) << "\n";
      if (multiTicketTotal.code() != INVALID_CURRENCYCODE)
        dc << "MULTIPLE TICKET TOTAL " << Money(multiTicketTotal) << "\n";
    }
    dc << std::endl;
  }
}

void
Diag676Collector::displaySolution(MultiTicketUtil::TicketSolution ticketSolution)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "***************************************************************\n";
    dc << "SOLUTION:\n";
    switch (ticketSolution)
    {
    case MultiTicketUtil::MULTITKT_NOT_APPLICABLE:
      dc << "MULTI-TICKET FARE NOT APPLICABLE - SINGLE TICKET FARE OFFERED";
      break;
    case MultiTicketUtil::MULTITKT_NOT_FOUND:
      dc << "MULTI-TICKET FARE NOT FOUND - SINGLE TICKET FARE OFFERED";
      break;
    case MultiTicketUtil::NO_SOLUTION_FOUND:
      dc << "NO SOLUTION FOUND";
      break;
    case MultiTicketUtil::SINGLETKT_NOT_APPLICABLE:
      dc << "MULTI-TICKET FARE OFFERED - SINGLE TICKET NOT AVAILABLE";
      break;
    case MultiTicketUtil::SINGLETKT_LOWER_THAN_MULTITKT:
      dc << "SINGLE TICKET FARE OFFERED AS LOWER THAN MULTI-TICKET FARE";
      break;
    case MultiTicketUtil::SINGLETKT_SAME_AS_MULTITKT:
      dc << "SINGLE TICKET FARE OFFERED AS SAME AS MULTI-TICKET FARE";
      break;
    case MultiTicketUtil::MULTITKT_LOWER_THAN_SINGLETKT:
      dc << "MULTI-TICKET FARE OFFERED AS LOWER THAN SINGLE TICKET FARE";
      break;
    default:
      dc << "UNKNOWN SOLUTION";
    }
    dc << std::endl;
  }
}

void
Diag676Collector::displayAmount(const FarePath& fPath, const CalcTotals& calcTotal)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << " \n";
    dc << "REQUESTED PAXTYPE " << fPath.paxType()->paxType() << "-"
       << std::setw(2) << std::setfill('0') << std::right << calcTotal.farePath->paxType()->number() << "\n";
    dc << "BASE FARE/EQUIVALENT AMOUNT " << Money(calcTotal.equivFareAmount, calcTotal.equivCurrencyCode) << "\n";
    dc << "TAXES/FEES/CHARGES " << Money(calcTotal.taxAmount(), calcTotal.equivCurrencyCode) << "\n";
    dc << "TOTAL AMOUNT " << Money(calcTotal.equivFareAmount + calcTotal.taxAmount(), calcTotal.equivCurrencyCode) << "\n";
  }
}

void
Diag676Collector::displayTotalAmount(Money amount)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << " \n";
    dc << "TOTAL AMOUNT FOR ALL PAXTYPES " << Money(amount) << "\n";
  }
}

void
Diag676Collector::displayMultiTicketTotalAmount(Money amount)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "---------------------------------------------------------------\n";
    dc << "MULTIPLE TICKET TOTAL AMOUNT " << Money(amount) << "\n";
  }
}
}
