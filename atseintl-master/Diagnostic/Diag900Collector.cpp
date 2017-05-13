//----------------------------------------------------------------------------
//  File:        Diag900Collector.C
//  Created:     2004-07-20
//
//  Description: Diagnostic 900 formatter
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

#include "Diagnostic/Diag900Collector.h"

#include "Common/ClassOfService.h"
#include "Common/Global.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/DataManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>

namespace tse
{
void
Diag900Collector::printHeader()
{
  *this << "*************************************************************************\n";
  *this << "DIAG 900: CARRIER LIST FOR EACH LEG AND ITS AVAILABILITY\n";
  *this << "*************************************************************************\n";
  *this << "[CUS] - special leg/SOP for custom carrier and routing\n";
  *this << "[ASO] - Across StopOver leg\n";
  *this << "-------------------------------------------------------------------------\n";
  *this << "\n";
  *this << "*************************************************************************\n";
  *this << "CABIN AND GOVERNING CARRIER LIST FOR EACH LEG\n";
  *this << "*************************************************************************\n";
}

Diag900Collector& Diag900Collector::operator<<(const ShoppingTrx& trx)
{
  if (false == _active)
  {
    return *this;
  }

  if (trx.legs().empty())
  {
    *this << "ERROR: "
          << "Legs vector is empty for current transaction.\n";
    return *this;
  }

  _trx = (PricingTrx*)&trx;

  printHeader();
  for (uint32_t n = 0; n != trx.legs().size(); n++)
  {
    const ShoppingTrx::Leg& curLeg = trx.legs()[n];

    _legIndex = n;

    if (curLeg.sop().empty())
    {
      *this << "ERROR: "
            << "Scheduling options vector for leg number " << (n + 1) << " is empty.\n";
      continue;
    }

    if (curLeg.sop().front().itin()->travelSeg().empty())
    {
      *this << "ERROR: "
            << "First itinerary in first scheduling option got empty travel segment vector.\n";
      continue;
    }
    // Retrive origin and destiantion cities for currently processed leg and
    // print it
    const LocCode& origin = curLeg.sop().front().itin()->travelSeg().front()->boardMultiCity();
    const LocCode& destination = curLeg.sop().front().itin()->travelSeg().back()->offMultiCity();

    *this << "\n"
          << "LEG " << (n + 1) << ": " << origin << "-" << destination
          << "\t CABIN: " << curLeg.preferredCabinClass().printName();
    *this << (curLeg.stopOverLegFlag() ? "     [ASO] " : "     ");
    *this << (curLeg.isCustomLeg() ? "[CUS]\n" : "\n");

    // Go thorough all carrier rows in carrier index for currently
    // processing leg
    for (const auto& elem : curLeg.carrierIndex().root())
    {
      const Itin* curItin = nullptr;

      ShoppingTrx& constTrx = const_cast<ShoppingTrx&>(trx);
      ItinIndex::ItinCell* curCell = ShoppingUtil::retrieveDirectItin(
          constTrx, n, elem.first, ItinIndex::CHECK_NOTHING);

      if (nullptr == curCell)
      {
        *this << "ERROR: "
              << "ItinCell object for direct flight is NULL.\n";
        continue;
      }
      curItin = curCell->second;

      if ((nullptr == curItin) || (curItin->fareMarket().empty()))
      {
        *this << "ERROR: "
              << "Direct flight itinerary object is NULL or fare markets vector for this itinerary "
                 "is empty.\n";
        continue;
      }

      const FareMarket* fareMarket = curItin->fareMarket().front();
      *this << "  " << fareMarket->governingCarrier() << "\n";
    }
  }
  *this << "\n";
  *this << "*************************************************************************\n";
  *this << "DETALIED INFO ABOUT EACH LEG\n";
  *this << "*************************************************************************\n";
  *this << "\n";
  for (uint32_t n = 0; n != trx.legs().size(); n++)
  {
    const ShoppingTrx::Leg& curLeg = trx.legs()[n];

    _legIndex = n;

    if (curLeg.stopOverLegFlag())
    {
      continue;
    }

    if (curLeg.sop().empty())
    {
      *this << "ERROR: "
            << "Scheduling options vector for leg number " << (n + 1) << " is empty.\n";
      continue;
    }

    if (curLeg.sop().front().itin()->travelSeg().empty())
    {
      *this << "ERROR: "
            << "First itinerary in first scheduling option got empty travel segment vector.\n";
      continue;
    }
    // Go thorough all scheduling options
    for (uint32_t sopId = 0; sopId != curLeg.sop().size(); sopId++)
    {
      const ShoppingTrx::SchedulingOption& sop = curLeg.sop()[sopId];

      if (false == ((ShoppingTrx::SchedulingOption&)sop).getDummy())
      {
        *this << sop;
      }
    }
  }
  return *this;
}

Diag900Collector& Diag900Collector::operator<<(const ShoppingTrx::SchedulingOption& sop)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  const Itin* itin = sop.itin();

  if (nullptr == itin)
  {
    dc << "ERROR: "
       << "Itinerary object is NULL." << std::endl;

    return (*this);
  }

  // Print information about leg
  const LocCode& origin = itin->travelSeg().front()->origin()->loc();
  const LocCode& destination = itin->travelSeg().back()->destination()->loc();

  dc << std::endl;
  dc << "*************************************************************************" << std::endl;
  dc << "LEG: " << (_legIndex + 1) << " OF " << (dynamic_cast<ShoppingTrx*>(_trx))->legs().size()
     << ": " << origin << "-" << destination << std::endl;

  // Print scheduling option id
  dc << "SCHEDULING OPTION ID: " << sop.originalSopId() << ((sop.isCustomSop()) ? "     [CUS]" : "")
     << " " << std::endl << std::endl;

  // Display travel segments information
  dc << "TRAVEL SEGMENTS: " << std::endl;

  // Go thorough all travel segments
  std::vector<TravelSeg*>::const_iterator segIter;

  for (segIter = itin->travelSeg().begin(); segIter != itin->travelSeg().end(); ++segIter)
  {
    const TravelSeg* travelSeg = (*segIter);

    // Check if travel segment object is not NULL
    if (nullptr == travelSeg)
    {
      dc << "ERROR: "
         << "Travel segment object is NULL." << std::endl;

      return (*this);
    }

    const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

    dc << "  " << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc() << " "
       << airSegment.carrier() << " " << airSegment.flightNumber() << " ("
       << airSegment.operatingCarrierCode() << ")" << std::endl;
  }

  dc << std::endl;

  // Check if fare markets vector is not empty
  if (itin->fareMarket().empty())
  {
    dc << "ERROR: "
       << "Fare markets vector is empty." << std::endl;

    return (*this);
  }

  // Print fare markets for currently processing itinerary
  dc << "FARE MARKETS:" << std::endl << std::endl;

  dc << "  CITIES  MULTI     TRAVEL      CXR DIR" << std::endl << "          CITIES    DATE"
     << std::endl << "  ------- --------- ----------- --- ---" << std::endl;

  std::vector<FareMarket*>::const_iterator fareMarketIter;

  for (fareMarketIter = itin->fareMarket().begin(); fareMarketIter != itin->fareMarket().end();
       ++fareMarketIter)
  {
    const FareMarket* fareMarket = (*fareMarketIter);

    if(fareMarket->isHighTPMGoverningCxr())
      dc << "  ADDITIONAL HIGHEST MILEAGE FARE MARKET (HIGHEST TPM):" << std::endl;

    dc << "  " << std::setw(3) << fareMarket->origin()->loc() << "-" << std::setw(3)
       << fareMarket->destination()->loc();

    dc << " (" << std::setw(3) << fareMarket->boardMultiCity() << "-" << std::setw(3)
       << fareMarket->offMultiCity();

    dc << ") " << std::setw(11) << fareMarket->travelDate().date();

    dc << std::setw(1) << " " << std::setw(3) << fareMarket->governingCarrier() << " "
       << std::setw(3) << (char)(fareMarket->direction()) << std::endl;

    if (PricingTrx::ESV_TRX == _trx->getTrxType())
    {
      ShoppingUtil::setupFareMarketTravelSegESV((FareMarket*)fareMarket, (Itin*)itin);
    }

    AvailabilityMap::const_iterator availMapIter;

    availMapIter = (dynamic_cast<ShoppingTrx*>(_trx))->availabilityMap().find(
        ShoppingUtil::buildAvlKey(fareMarket->travelSeg()));

    if (availMapIter != (dynamic_cast<ShoppingTrx*>(_trx))->availabilityMap().end())
    {
      const std::vector<ClassOfServiceList>& availabilityList =
          *(availMapIter->second);

      int segNo = 0;
      std::vector<TravelSeg*>::const_iterator segIter;

      for (segIter = fareMarket->travelSeg().begin(); segIter != fareMarket->travelSeg().end();
           segIter++, segNo++)
      {
        const TravelSeg* travelSeg = (*segIter);

        const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

        dc << "    " << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc() << " "
           << airSegment.carrier() << " " << airSegment.flightNumber() << " ("
           << airSegment.operatingCarrierCode() << ")" << std::endl;

        std::vector<ClassOfService*>::const_iterator cosIter;

        dc << "    ";

        bool availabilityFound = false;

        for (cosIter = (availabilityList[segNo]).begin();
             cosIter != (availabilityList[segNo]).end();
             ++cosIter)
        {
          if (nullptr != (*cosIter))
          {
            dc << (*cosIter)->bookingCode() << (*cosIter)->numSeats() << "(" << (*cosIter)->cabin()
               << ")"
               << "|";

            availabilityFound = true;
          }
        }

        if (false == availabilityFound)
        {
          dc << "No Availability";
        }

        dc << std::endl;
      }
    }
    else
    {
      dc << "ERROR: "
         << "No apropriate entry in availability map." << std::endl;

      return (*this);
    }
  }

  return (*this);
}
}
