//----------------------------------------------------------------------------
//  File:        Diag969Collector.C
//
//  Copyright Sabre 2007
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

#include "Diagnostic/Diag969Collector.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/PricingTrx.h"

using namespace std;
namespace tse
{
Diag969Collector& Diag969Collector::operator<<(FlightFinderTrx& flightFinderTrx)
{
  if (_active &&
      flightFinderTrx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ALTDATESPQ")
  {
    DiagCollector& dc(*this);

    dc << "***************************************************" << endl;
    dc << "Diagnostic 969 : JA response BEGIN" << endl;
    dc << "***************************************************" << endl;

    if (flightFinderTrx.getTrxType() != PricingTrx::FF_TRX)
    {
      dc << "FF logic is not active";
    }
    else if (flightFinderTrx.outboundDateflightMap().empty())
    {
      dc << "List is empty" << endl;
    }
    else
    {
      dc << "Flight List for BFF and FF:" << endl;

      FlightFinderTrx::OutBoundDateFlightMap::const_iterator itODFM =
          flightFinderTrx.outboundDateflightMap().begin();
      for (; itODFM != flightFinderTrx.outboundDateflightMap().end(); ++itODFM)
      {
        // DataTime outbound
        dc << "   OutboundDate: " << itODFM->first.date();
        if (showFareBasisCode(flightFinderTrx, true))
        {
          PaxTypeFare* outBoundFare = getFrontPaxTypeFare(itODFM->second->flightInfo);
          if (outBoundFare)
          {
            dc << " - " << outBoundFare->createFareBasis(flightFinderTrx, false);
          }

          if (flightFinderTrx.avlInS1S3Request())
          {
            dc << " - "
               << "AVL: " << (itODFM->second->flightInfo.onlyApplicabilityFound ? "F" : "T");
          }
        }
        dc << endl;

        if (showSOPs(flightFinderTrx, true))
        {
          std::vector<FlightFinderTrx::SopInfo*>::const_iterator itOBFL =
              itODFM->second->flightInfo.flightList.begin();
          for (; itOBFL != itODFM->second->flightInfo.flightList.end(); ++itOBFL)
          {
            // outBaund flights - LEG = 0 for outBaund
            dc << getSOPInfo(flightFinderTrx, 0, *itOBFL) << endl;
          }
        }
        // iBDateFlightMap
        FlightFinderTrx::InboundDateFlightMap::const_iterator itIDFM =
            itODFM->second->iBDateFlightMap.begin();
        for (; itIDFM != itODFM->second->iBDateFlightMap.end(); ++itIDFM)
        {
          dc << "     InboundDate: " << itIDFM->first.date();
          if (showFareBasisCode(flightFinderTrx, false))
          {
            PaxTypeFare* inBoundFare = getFrontPaxTypeFare(*(itIDFM->second));
            if (inBoundFare)
            {
              dc << " - " << inBoundFare->createFareBasis(flightFinderTrx, false);
            }

            if (flightFinderTrx.avlInS1S3Request())
            {
              dc << " - "
                 << "AVL: " << (itIDFM->second->onlyApplicabilityFound ? "F" : "T");
            }
          }
          dc << endl;

          if (showSOPs(flightFinderTrx, false))
          {
            // inBaund flights
            std::vector<FlightFinderTrx::SopInfo*>::const_iterator itIBFL =
                itIDFM->second->flightList.begin();
            for (; itIBFL != itIDFM->second->flightList.end(); ++itIBFL)
            {
              // inBaund flights - LEG = 1 for outBaund
              dc << getSOPInfo(flightFinderTrx, 1, *itIBFL) << endl;
            }
          }
        }
      }
    }

    dc << "***************************************************" << endl;
    dc << "Diagnostic 969 : JA response END" << endl;
    dc << "***************************************************" << endl;
    dc << endl;
  }

  return (*this);
}

PaxTypeFare*
Diag969Collector::getFrontPaxTypeFare(const FlightFinderTrx::FlightDataInfo& flightInfo)
{
  if (!flightInfo.altDatesPaxTypeFareVect.empty())
  {
    return flightInfo.altDatesPaxTypeFareVect.front();
  }
  else
  {
    if (flightInfo.flightList.empty())
    {
      return nullptr;
    }
    else
    {
      return flightInfo.flightList.front()->paxTypeFareVect.front();
    }
  }
}

std::string
Diag969Collector::getSOPInfo(FlightFinderTrx& fFTrx,
                             const uint16_t& legID,
                             FlightFinderTrx::SopInfo* sopInfo)
{
  std::vector<ShoppingTrx::SchedulingOption>& sop = fFTrx.legs()[legID].sop();
  std::ostringstream output;
  output << "       SOP " << sop[sopInfo->sopIndex].originalSopId() << "\n";

  const Itin* sopItin = sop[sopInfo->sopIndex].itin();

  std::string buffer("");
  std::vector<TravelSeg*>::const_iterator travelSegIter = sopItin->travelSeg().begin();
  for (size_t count = 1; travelSegIter != sopItin->travelSeg().end(); ++travelSegIter, ++count)
  {
    const AirSeg* curSeg = dynamic_cast<const AirSeg*>(*travelSegIter);
    if (curSeg == nullptr)
    {
      continue;
    }

    buffer = curSeg->origAirport() + curSeg->destAirport();
    const DateTime& depDT = curSeg->departureDT();
    const DateTime& arrDT = curSeg->arrivalDT();
    std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
    std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");

    output << "       SEGMENT #" << count << "  - ";

    if (curSeg->segmentType() != Arunk)
    {
      output << std::setw(4) << curSeg->carrier() << std::setw(6) << curSeg->flightNumber()
             << std::setw(4) << getAvailability(sopInfo, count) << std::setw(8) << buffer
             << std::setw(6) << depDT.dateToString(DDMMM, "") << std::setw(7) << depDTStr
             << std::setw(7) << arrDTStr << "\n";
    }
    else
    {
      output << std::setw(4) << " " << std::setw(6) << "ARUNK" << std::setw(4)
             << getAvailability(sopInfo, count) << std::setw(8) << buffer << std::setw(6)
             << depDT.dateToString(DDMMM, "") << std::setw(7) << depDTStr << std::setw(7)
             << arrDTStr << "\n";
    }
  }

  return output.str();
}

bool
Diag969Collector::showFareBasisCode(FlightFinderTrx& flightFinderTrx, bool outbound)
{
  if (!flightFinderTrx.isBffReq())
    return true;

  if (outbound)
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_1 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_2 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_5)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else // inbound
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_3 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_4 ||
        flightFinderTrx.bffStep() == FlightFinderTrx::STEP_6)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool
Diag969Collector::showSOPs(FlightFinderTrx& flightFinderTrx, bool outbound)
{
  if (!flightFinderTrx.isBffReq())
    return true;

  if (outbound)
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_5)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else // inbound
  {
    if (flightFinderTrx.bffStep() == FlightFinderTrx::STEP_6)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

std::string
Diag969Collector::getAvailability(const FlightFinderTrx::SopInfo* sopInfo, const size_t count)
{
  std::ostringstream output;

  if (sopInfo->bkgCodeDataVect.front().size() > (count - 1))
  {
    output << sopInfo->bkgCodeDataVect.front()[count - 1].numSeats;
    output << sopInfo->bkgCodeDataVect.front()[count - 1].bkgCode;
  }
  else
  {
    output << "-";
  }

  return output.str();
}
}
