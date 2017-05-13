//----------------------------------------------------------------------------
//  File:        Diag954Collector.C
//  Created:     2008-01-29
//
//  Description: Diagnostic 954 formatter
//
//  Updates:
//
//  Copyright Sabre 2008
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
#include "Diagnostic/Diag954Collector.h"

#include "Common/BookingCodeUtil.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/TravelSeg.h"

#include <iomanip>
#include <iostream>

namespace tse
{
Diag954Collector& Diag954Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************\n"
     << "954 : SCHEDULING OPTIONS WITH CHEAPEST FARES COMBINATIONS\n"
     << "**********************************************************\n";
  dc << std::endl;

  _trx = (PricingTrx*)&shoppingTrx;

  if (shoppingTrx.legs().empty())
  {
    dc << "ERROR: "
       << "Legs vector is empty for current shopping transaction." << std::endl;

    return (*this);
  }

  if ("Y" == dc.rootDiag()->diagParamMapItem("PRINTRULES"))
  {
    _printRules = true;
  }

  // Read carrier parameters
  const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");

  // Go thorough all legs
  for (uint32_t legId = 0; legId != shoppingTrx.legs().size(); legId++)
  {
    const ShoppingTrx::Leg& leg = shoppingTrx.legs()[legId];

    _legIndex = legId;

    // Go thorough all scheduling options
    for (uint32_t sopId = 0; sopId != leg.sop().size(); sopId++)
    {
      const ShoppingTrx::SchedulingOption& sop = leg.sop()[sopId];

      if (false == ((ShoppingTrx::SchedulingOption&)sop).getDummy())
      {
        _sopIndex = sopId;

        const Itin* itin = sop.itin();

        // Check if itinerary object is not NULL
        if (nullptr == itin)
        {
          dc << "ERROR: Itinerary object is NULL." << std::endl;

          return (*this);
        }

        // Check if diagnostic parameters specify to exclude this
        // governing carrier
        if ((filterCxr.empty() == false) && (filterCxr != sop.governingCarrier()))
        {
          continue;
        }

        // Print information about leg
        const LocCode& origin = itin->travelSeg().front()->origin()->loc();
        const LocCode& destination = itin->travelSeg().back()->destination()->loc();

        dc << "**********************************************************" << std::endl;
        dc << "LEG: " << (_legIndex + 1) << " OF "
           << (dynamic_cast<ShoppingTrx*>(_trx))->legs().size() << ": " << origin << "-"
           << destination << std::endl;

        // Print scheduling option index number
        dc << "SCHEDULING OPTION ID: " << sop.originalSopId() << std::endl;

        // Display travel segments information
        dc << "TRAVEL SEGMENTS: ";

        // Go thorough all travel segments
        std::vector<TravelSeg*>::const_iterator segIter;

        for (segIter = itin->travelSeg().begin(); segIter != itin->travelSeg().end(); segIter++)
        {
          const TravelSeg* travelSeg = (*segIter);

          // Check if travel segment object is not NULL
          if (nullptr == travelSeg)
          {
            dc << "ERROR: Travel segment object is NULL." << std::endl;

            return (*this);
          }

          const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

          dc << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc() << " ("
             << airSegment.carrier() << ") ";
        }

        // print JCB flag
        if (PricingTrx::ESV_TRX == _trx->getTrxType())
        {
          dc << std::endl;
          if (itin->isJcb())
          {
            dc << "JCB FLAG: TRUE";
          }
          else
          {
            dc << "JCB FLAG: FALSE";
          }
        }

        dc << std::endl << "**********************************************************" << std::endl
           << std::endl;

        // Print diagnostics for each scheduling option
        (*this) << sop;
      }
    }
  }

  return (*this);
}

Diag954Collector& Diag954Collector::operator<<(const ShoppingTrx::SchedulingOption& sop)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  // Adjust output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);

  const Itin* itin = sop.itin();

  // Check if itinerary object is not NULL
  if (nullptr == itin)
  {
    dc << "ERROR: Itinerary object is NULL." << std::endl;

    return (*this);
  }

  // Go thorough all pasenger types
  std::vector<PaxType*>::iterator paxTypeIter;

  for (paxTypeIter = ((PricingTrx*)_trx)->paxType().begin();
       paxTypeIter != ((PricingTrx*)_trx)->paxType().end();
       paxTypeIter++)
  {
    PaxType* paxType = (*paxTypeIter);

    // Check if passenger type object is not NULL
    if (nullptr == paxType)
    {
      dc << "ERROR: Passenger type object is NULL." << std::endl;

      return (*this);
    }

    dc << "PASSENGER TYPE: " << paxType->paxType() << std::endl;

    dc << std::endl;
    dc << "  GI V RULE   FARE BASIS    TRF RTG  O O SAME CARRIER     AMT CUR FAR PAX RULE  \n"
       << "                            NUM NUM  R I 102 103 104              TYP TYP FAILED\n"
       << "- -- - ---- --------------- --- ---- - - --- --- --- -------- --- --- --- ------\n";

    if (1 == itin->paxTypeSOPFareListMap().count(paxType))
    {
      std::map<PaxType*, SOPFareList>::const_iterator sopFareListMapIter =
          itin->paxTypeSOPFareListMap().find(paxType);
      const SOPFareList& sopFareList = sopFareListMapIter->second;

      bool validPathFound = false;

      // Print One Way ESV values
      std::vector<SOPFarePath*>::const_iterator sopFarePathIter;

      for (sopFarePathIter = sopFareList.owSopFarePaths().begin();
           sopFarePathIter != sopFareList.owSopFarePaths().end();
           sopFarePathIter++)
      {
        const SOPFarePath* sopFarePath = (*sopFarePathIter);
        printSopFarePath(sopFarePath, "OW");

        validPathFound = true;
      }

      // If it's not One Way trip print also Round Trip fares
      if ((dynamic_cast<ShoppingTrx*>(_trx))->legs().size() != 1)
      {
        // Print Round Trip ESV values
        for (sopFarePathIter = sopFareList.rtSopFarePaths().begin();
             sopFarePathIter != sopFareList.rtSopFarePaths().end();
             sopFarePathIter++)
        {
          const SOPFarePath* sopFarePath = (*sopFarePathIter);
          printSopFarePath(sopFarePath, "RT");

          validPathFound = true;
        }

        // Print Open Jaw
        for (sopFarePathIter = sopFareList.ojSopFarePaths().begin();
             sopFarePathIter != sopFareList.ojSopFarePaths().end();
             sopFarePathIter++)
        {
          const SOPFarePath* sopFarePath = (*sopFarePathIter);
          printSopFarePath(sopFarePath, "OJ");

          validPathFound = true;
        }

        // Print Circle Trip ESV values
        for (sopFarePathIter = sopFareList.ctSopFarePaths().begin();
             sopFarePathIter != sopFareList.ctSopFarePaths().end();
             sopFarePathIter++)
        {
          const SOPFarePath* sopFarePath = (*sopFarePathIter);
          printSopFarePath(sopFarePath, "CT");

          validPathFound = true;
        }
      }

      if (false == validPathFound)
      {
        dc << "    NO VALID FARE PATHS FOUND FOR THIS PAX TYPE" << std::endl;
      }
    }
    else
    {
      dc << "    NO VALID FARE PATHS FOUND FOR THIS PAX TYPE" << std::endl;
    }
  }

  dc << std::endl;

  return (*this);
}

Diag954Collector& Diag954Collector::operator<<(const PaxTypeFare& paxTypeFare)
{
  Diag953Collector::operator<<(paxTypeFare);

  return (*this);
}

void
Diag954Collector::printSopFarePath(const SOPFarePath* sopFarePath, std::string pathType)
{
  DiagCollector& dc(*this);

  dc << std::endl;

  std::string esvValueName = "";

  esvValueName = pathType;

  std::string combType = "";

  switch (sopFarePath->combinationType())
  {
  case SOPFarePath::NOT_INITIALIZED:
    combType = "(NA)";
    break;

  case SOPFarePath::R:
    combType = "(R)";
    break;

  case SOPFarePath::X:
    combType = "(X)";
    break;

  case SOPFarePath::RR:
    combType = "(RR)";
    break;

  case SOPFarePath::RX:
    combType = "(RX)";
    break;

  case SOPFarePath::XR:
    combType = "(XR)";
    break;

  case SOPFarePath::XX:
    combType = "(XX)";
    break;

  default:
    combType = "(ERROR)";
    break;
  }

  esvValueName += combType + " ESV";

  if (true == sopFarePath->haveCat10Restrictions())
  {
    esvValueName += "(SAME CARRIER)";
  }

  dc << std::setw(32) << esvValueName;

  // Print fare market path info
  dc << "FARE MARKET PATH: ";

  std::vector<FareMarket*>::const_iterator fareMarketIter;

  for (fareMarketIter = sopFarePath->fareMarketPath().begin();
       fareMarketIter != sopFarePath->fareMarketPath().end();
       fareMarketIter++)
  {
    const FareMarket* fareMarket = (*fareMarketIter);

    dc << fareMarket->origin()->loc() << "-" << fareMarket->destination()->loc() << " ";
  }

  dc << std::endl;

  // Go thorough all fares
  std::vector<PaxTypeFare*>::const_iterator paxTypeFareIter;

  for (paxTypeFareIter = sopFarePath->paxTypeFareVec().begin();
       paxTypeFareIter != sopFarePath->paxTypeFareVec().end();
       paxTypeFareIter++)
  {
    const PaxTypeFare* paxTypeFare = (*paxTypeFareIter);

    if (nullptr == paxTypeFare)
    {
      dc << "ERROR: PaxTypeFare object is NULL." << std::endl;

      continue;
    }

    // Print diagnostics for each pax type fare
    (*this) << (*paxTypeFare);

    printSelectedBookingCodes(paxTypeFare);
  }

  dc << "TOTAL AMOUNT:                                        " << std::setw(8)
     << Money(sopFarePath->totalAmount(), "NUC") << std::endl;
}

void
Diag954Collector::printSelectedBookingCodes(const PaxTypeFare* paxTypeFare)
{
  DiagCollector& dc(*this);

  dc << "BKK (";

  VecMap<uint32_t, PaxTypeFare::FlightBit>::const_iterator iter =
      paxTypeFare->flightBitmapESV().find(_sopIndex);

  if (iter != paxTypeFare->flightBitmapESV().end())
  {
    const PaxTypeFare::SegmentStatusVec& segmentStatusVec = (iter->second)._segmentStatus;

    for (const auto& elem : segmentStatusVec)
    {
      dc << elem._bkgCodeReBook << ",";
    }
  }

  dc << ")" << std::endl;
}
}
