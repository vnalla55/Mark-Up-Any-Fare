//----------------------------------------------------------------------------
//  File:        Diag185Collector.C
//
//  Description: Diagnostic 185 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Diagnostic/Diag185Collector.h"

#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
void
Diag185Collector::printHeader() const
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "***************** START DIAG 185 ************************** " << std::endl;
  }
}

void
Diag185Collector::startDiag185(const PricingTrx& trx) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  CabinType cabin = trx.getOptions()->cabin();

  dc << "\n REQUESTED CABIN: " ;

  if (cabin.isPremiumFirstClass())
     dc << " PB - PREMIUM FIRST";
  else if (cabin.isFirstClass())
     dc << " FB - FIRST";
  else if (cabin.isPremiumBusinessClass())
     dc << " JB - PREMIUM BUSINESS";
  else if (cabin.isBusinessClass())
     dc << " BB - BUSINESS";
  else if (cabin.isPremiumEconomyClass())
     dc << " SB - PREMIUM ECONOMY";
  else if (cabin.isEconomyClass())
     dc << " YB - ECONOMY";
  else if (cabin.isAllCabin())
     dc << " AB - ALL CABINS" ;
  dc << "   CAB:"<< cabin ;
  dc << " \n ---------------------------------------------------------- " << std::endl;
}

void
Diag185Collector::printGcmSelectionHeader() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " \n STEP 1 :\n" ;
  dc << "\n SELECT TURNAROUND/FURTHEST POINT BASED ON GCM:\n" ;
}

void
Diag185Collector::printSelectedFmNotFound() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "\n* SELECTED FARE MARKET NOT FOUND, BREAK IT TO SMALLER FM *\n" ;
}

void
Diag185Collector::printRTWSelectionHeader() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " \n STEP 1 :\n" ;
  dc << "\n SELECT RW FARE MARKET:\n" ;
}

void
Diag185Collector::printSegmentSelectedHeader() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " \n SEGMENTS :    GC MILES\n" ;
}

void
Diag185Collector::printSegmentGcm(const LocCode& origin,
                                  const TravelSeg& tvlSeg,
                                  const uint32_t gcm) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(2) << tvlSeg.pnrSegment() << " "
     << origin << tvlSeg.destAirport() << "       ";
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(6) <<gcm << "\n";
}

void
Diag185Collector::printProcessSelectedFareMarkets() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " \n STEP 2 :\n" ;
  dc << " \n PROCESS SELECTED FARE MARKETS :\n" ;
}

void
Diag185Collector::printNoMarketsSelected() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "* ERROR - NO MARKETS SELECTED *\n" << std::endl;
}

void
Diag185Collector::printSelectedSegments(const std::vector<TravelSeg*>& tvlSegs)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  if(tvlSegs.empty())
  {
    dc << " \n       NO MARKET SELECTED :\n" ;
    return;
  }

  dc << " \n       MARKET SELECTED :\n" ;

  for(auto tvl : tvlSegs)
    addTravelSegment(tvl, true);
  dc << "* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n" << std::endl;
}

void
Diag185Collector::printFareMarket(const PricingTrx& trx, FareMarket& fm)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " ---------------------------------------------------------- " << std::endl;
  dc << " *** " << "FARE MARKET " 
     << fm.origin()->loc() << "-"
     << fm.destination()->loc() << "   ";
  if(trx.getOptions()->isRtw() && trx.itin().front()->tripCharacteristics().isSet(Itin::RW_SFC))
    dc << "ROUND THE WORLD" << std::endl;
  else if(trx.getOptions()->isRtw() && trx.itin().front()->tripCharacteristics().isSet(Itin::CT_SFC))
    dc << "CIRCLE TRIP" << std::endl;
  else if(fm.isDomestic())
    dc << "DOMESTIC" << std::endl;
  else if(fm.isForeignDomestic() || fm.isWithinRussianGroup())
    dc << "FOREIGN DOMESTIC" << std::endl;
  else if(fm.isTransBorder())
    dc << "TRANSBORDER" << std::endl;
  else if(LocUtil::oneOfNetherlandAntilles(*fm.origin()) &&
          LocUtil::oneOfNetherlandAntilles(*fm.destination()) &&
          isItinOutsideNetherlandAntilles(*trx.itin().front()))
    dc << "NETHERLAND ANTILLES" << std::endl;
  else if(LocUtil::isEurope(*fm.origin()) && LocUtil::isEurope(*fm.destination()) &&
          isItinOutsideEurope(*trx.itin().front()))
    dc << "IN EUROPE" << std::endl;
  else
    dc << "INTERNATIONAL" << std::endl;
  dc << " ---------------------------------------------------------- " << std::endl;
  addFareMarket(trx, fm);
  dc << " ---------------------------------------------------------- " << std::endl;
}

bool
Diag185Collector::isItinOutsideEurope(const Itin& itin)
{
  if ( itin.geoTravelType() != GeoTravelType::International)
    return false;

  for(auto tvl : itin.travelSeg())
  {
    if(!LocUtil::isEurope(*tvl->origin()) ||
       !LocUtil::isEurope(*tvl->destination()) )
      return true;
  }
  return false;
}

bool
Diag185Collector::isItinOutsideNetherlandAntilles(const Itin& itin)
{
  if ( itin.geoTravelType() != GeoTravelType::International)
    return false;

  for(auto tvl : itin.travelSeg())
  {
    if(!LocUtil::oneOfNetherlandAntilles(*tvl->origin()) ||
       !LocUtil::oneOfNetherlandAntilles(*tvl->destination()) )
      return true;
  }
  return false;
}

void
Diag185Collector::printInvalidFM(const FareMarket& fm, bool primary)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  if(!primary && fm.travelSeg().size() == 1)
    dc << " * INVENTORY REQUIRED FOR REPLACEMENT NOT AVAIABLE *\n";

  dc << " ---------------------------------------------------------- " << std::endl;
  dc << " * INVALID FARE MARKET "
     << fm.origin()->loc() << "-"
     << fm.destination()->loc() << std::endl;

  for (const TravelSeg* travelSeg : fm.travelSeg())
    addTravelSegment(travelSeg);

  if(!primary)
    return;

  dc << " \n* PRIMARY SECTOR COULD NOT BE REPLACED IN REQUESTED CABIN *\n" << std::endl;
  if(fm.primarySector())
    addTravelSegment(fm.primarySector());

  dc <<  "***************** END  DIAG 185 ************************** " << std::endl;
  dc.flushMsg();
}

void
Diag185Collector::printInvalidFareMarket()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " ---------------------------------------------------------- " << std::endl;
  dc << " * INVALID FARE MARKET " << std::endl;
  dc << " * PRIMARY SECTOR CABIN NOT OFFERED/JUMP DOWN ***" << std::endl;
}

void
Diag185Collector::printNoOneSegmentInCabinRequested() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << " ---------------------------------------------------------- " << std::endl;
  dc << " * ITINERARY WHOLLY WITH THE SAME NATION " << std::endl;
  dc << " * NO ONE SEGMENT IN THE CABIN REQUESTED *" << std::endl;
}

void
Diag185Collector::printNoRBDfoundForSecondarySegment(const TravelSeg* tvlSeg, const FareMarket& fm)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  printInvalidFM(fm, false);
  dc << " \n";
  dc << " * CABIN OFFERED BUT UNAVAILABLE ON SECONDARY SECTOR *" << std::endl;
  addTravelSegment(tvlSeg);
  dc <<  "***************** END  DIAG 185 ************************** " << std::endl;
  dc.flushMsg();
}

void
Diag185Collector::printFurthestPoint(const TravelSeg& tvlSeg, const uint32_t highestGcm)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "\nTURNAROUND/FURTHEST POINT:   " << tvlSeg.pnrSegment() << " "
               << tvlSeg.destAirport() << "   DISTANCE: " << highestGcm << "\n";
}

void
Diag185Collector::printFurthestPointNotSet() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "\nERROR: TURNAROUND/FURTHEST POINT NOT SET\n";
}

void
Diag185Collector::printGcmNotCalculated() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "    GCM NOT CALCULATED\n";
}

void
Diag185Collector::printNoChanges(const TravelSeg* tvlSeg)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  addTravelSegment(tvlSeg);
  dc << "    ALREADY BOOKED IN JUMP DOWN CABIN\n \n" << std::endl;
}

void
Diag185Collector::printSkippedSegment(const TravelSeg* tvlSeg, CabinType reqCabin)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  addTravelSegment(tvlSeg);
  if(!tvlSeg->isAir())
    dc << "        SURFACE\n \n" << std::endl;
  else if(tvlSeg->rbdReplaced())
    dc << "        ALREADY REPLACED\n \n" << std::endl;
  else if(tvlSeg->bookedCabin() == reqCabin)
    dc << "        BOOKED IN REQUESTED CABIN\n \n" << std::endl;
}

void
Diag185Collector::addFareMarket(const PricingTrx& trx, const FareMarket& fm)
{
  if (_active)
  {
    DiagCollector& dc = (Diag185Collector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);

    // Do Travel Segments
    dc << " *** TRAVEL SEGMENTS AS BOOKED RBD *** " << std::endl;
    for (const TravelSeg* travelSeg : fm.travelSeg())
      addTravelSegment(travelSeg);

    dc << " \n";

    if (!trx.getOptions()->isRtw() && 
        (fm.isDomestic() || fm.isForeignDomestic() ||
         fm.isTransBorder() || fm.isWithinRussianGroup() ||
         (LocUtil::oneOfNetherlandAntilles(*fm.origin()) &&
          LocUtil::oneOfNetherlandAntilles(*fm.destination()) &&
          isItinOutsideNetherlandAntilles(*trx.itin().front())) ||
         (LocUtil::isEurope(*fm.origin()) && LocUtil::isEurope(*fm.destination()) &&
          isItinOutsideEurope(*trx.itin().front()))))
    { // do not print primary sector for the domestic fare market
    }
    else
    {
      // Do Primary Sector
      dc << " *** PRIMARY SECTOR ***" << std::endl;
      if (fm.primarySector() == 0)
        dc << " PRIMARY SECTOR IS NULL " << std::endl;
      else
        addTravelSegment(fm.primarySector());
      dc << std::endl;

      dc << " \n";
    }
    // Do ATAE data for class of services
    dc << " *** TRAVEL SEGMENTS AVAILABILITY  ***" << std::endl;
    const std::vector<std::vector<ClassOfService*>*>& cosVecVec = fm.classOfServiceVec();
    uint16_t lenCosVecVec = fm.classOfServiceVec().size();
    uint16_t iCos = 0;
    for (const TravelSeg* travelSeg : fm.travelSeg())
    {
      addTravelSegment(travelSeg);

      if (!travelSeg->isAir())
      {
        dc << " \n";
        continue;
      }
      std::vector<ClassOfService*>* cosVec = getJourneyAvailability(travelSeg, trx.itin().front());

      dc << "   FLOW AVAIL: ";

      if(!cosVec)
      {
        if (iCos < lenCosVecVec)
        {
          addCOS(cosVecVec[iCos++]);
        }
      }
      else
        addCOS(cosVec);

      dc << " \n";
      dc << "   SOLO AVAIL: ";
      addCOS(&(travelSeg->classOfService()));
      dc << " \n";
    }
  }
  return;
}

std::vector<ClassOfService*>*
Diag185Collector::getJourneyAvailability(const TravelSeg* travelSeg, Itin* itin)
{
  return JourneyUtil::availability(travelSeg, itin);
}

void
Diag185Collector::addTravelSegment(const TravelSeg* tvlSeg, bool shift)
{
  if (_active)
  {
    DiagCollector& dc = (Diag185Collector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);

    const AirSeg* airSeg(nullptr);
    if (tvlSeg->isAir())
    {
      airSeg = static_cast<const AirSeg*>(tvlSeg);
    }

    if(shift)
      dc << "       ";
    if(!airSeg)
    {
      dc << " " << tvlSeg->pnrSegment();
      dc << " ARUNK          ";
      dc << tvlSeg->origAirport() << "-" << tvlSeg->destAirport() << std::endl;
      return;
    }

    dc << " " << tvlSeg->pnrSegment();
    dc << " " << airSeg->carrier();
    dc << setw(4) << airSeg->flightNumber();
    dc << setw(2) << airSeg->getBookingCode();
    dc << " " << setw(5) << airSeg->departureDT().dateToString(DDMMM, "");
    dc << " " << airSeg->origAirport() << "-" << airSeg->destAirport();
    dc << " CAB:" << airSeg->bookedCabin() << " \n";
  }
  return;
}

void
Diag185Collector::addCOS(const std::vector<ClassOfService*>* cosVec)
{
  if (_active)
  {
    DiagCollector& dc = (Diag185Collector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    uint16_t adj = 0;
    for (const ClassOfService* classOfService : *cosVec)
    {
      dc << classOfService->bookingCode() << classOfService->numSeats() << " ";

       // adj will keep an account if we should keep displaying the booking codes
       // in the same line or should we start a new line
      if (classOfService->numSeats() > 99)
      {
        adj = adj + 5;
      }
      else
      {
        if (classOfService->numSeats() < 10)
          adj = adj + 3;
        else
          adj = adj + 4;
      }
      // only display 40 characters in one line
      if (adj > 40)
      {
        dc << " \n";
        dc << "               ";
        adj = 0;
      }
    }
  }
  return;
}

void
Diag185Collector::displayChangedHeader() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " \n" ;
  dc <<  " *** TRAVEL SEGMENTS CHANGED RBD ***" << std::endl;
}

void
Diag185Collector::displayChangedInfo(const ClassOfService* cos, bool flow) const
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    if (flow)
    {
      dc << "     CHECK FLOW AVAILAILITY: FOUND RBD " << cos->bookingCode();
    }
    else
      dc << "     CHECK SOLO AVAILAILITY: FOUND RBD " << cos->bookingCode();
    dc << " WITH " << cos->numSeats() << " SEATS " << std::endl;
    dc << " \n";
  }
  return;
}

void
Diag185Collector::displayChangesDiag185(const ClassOfService* cos, const TravelSeg* tvlSeg, bool avType)
{
  if (!_active)
   return;

  displayChangedHeader();
  addTravelSegment(tvlSeg);
  displayChangedInfo(cos, avType);
}

void
Diag185Collector::displayAllTravelSeg(const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << " *** ALL TRAVEL SEGMENTS ***" << std::endl;
    for (const TravelSeg* travelSeg : trx.travelSeg())
      addTravelSegment(travelSeg);
    dc << " \n";
  }
  return;
}

void
Diag185Collector::finishDiag185(const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    displayAllTravelSeg(trx);  //Print all Travel Segments after RBD changes

    dc <<  "***************** END  DIAG 185 ************************** " << std::endl;
    dc.flushMsg();
  }
  return;
}

} // name space
