//----------------------------------------------------------------------------
//  File:        Diag902Collector.C
//  Created:     2004-08-17
//
//  Description: Diagnostic 902 formatter
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
#include "Diagnostic/Diag902Collector.h"

#include "Common/ClassOfService.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagCollector.h"

#include <fstream>
#include <iomanip>
#include <iostream>

namespace tse
{
Diag902Collector& Diag902Collector::operator<<(const Itin& itin)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    // Return from this method if the itinerary has no
    // travel segments
    if (itin.travelSeg().empty())
    {
      return (*this);
    }

    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "\n";

    // Get travel seg iterator
    std::vector<TravelSeg*>::const_iterator travelSegIter = itin.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSegEndIter = itin.travelSeg().end();

    int count = 1;
    std::string buffer("");
    for (; travelSegIter != travelSegEndIter; ++travelSegIter)
    {
      const AirSeg* curSeg = dynamic_cast<const AirSeg*>(*travelSegIter);
      if (curSeg == nullptr)
      {
        continue;
      }

      buffer = curSeg->origAirport() + curSeg->destAirport();
      const DateTime& depDT = curSeg->departureDT();
      const DateTime& arrDT = curSeg->arrivalDT();
      const std::string& depDTStr = depDT.timeToString(HHMM_AMPM, "");
      const std::string& arrDTStr = arrDT.timeToString(HHMM_AMPM, "");

      dc << "- SEGMENT #" << count << "  - ";

      if (curSeg->segmentType() != Arunk)
      {
        dc << std::setw(4) << curSeg->carrier() << std::setw(6) << curSeg->flightNumber();
      }
      else
      {
        dc << std::setw(4) << " " << std::setw(6) << "ARUNK";
      }

      dc << std::setw(8) << buffer << std::setw(6) << depDT.dateToString(DDMMM, "") << std::setw(7)
         << depDTStr << std::setw(6) << arrDTStr;
      if (*travelSegIter == _primarySector)
        dc << " *";
      dc << "\n";

      // Increase the count
      count++;
    }
  }
  return (*this);
}

Diag902Collector& Diag902Collector::operator<<(const ItinIndex& itinIndex)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    if (itinIndex.root().empty())
    {
      return (*this);
    }

    double point = 0.0;
    int numberOfClassesToAdd = 0;
    int requestedNumberOfSeats = PaxTypeUtil::totalNumSeats(*_shoppingTrx);

    if (_shoppingTrx->getRequestedNumOfEstimatedSolutions() > 0)
    {
      ItinUtil::readConfigDataForSOPScore(point, numberOfClassesToAdd);
    }

    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "\n";

    // Create the iterators for the itin matrix
    ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
    ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();

    const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");
    const std::string& filterFM = dc.rootDiag()->diagParamMapItem("FM");
    std::string filterFMOrigin, filterFMDest;
    if (filterFM.size() == 6)
    {
      filterFMOrigin = filterFM.substr(0, 3);
      filterFMDest = filterFM.substr(3, 3);
    }
    const std::string& dd = dc.rootDiag()->diagParamMapItem("DD");
    bool bookingCodeAvailDisplay = (dd == "AVAILABILITY" || dd == "MERGEDAVAILABILITY");

    int count = 1;

    // Loop through the itin matrix
    for (; iGIter != iGEndIter; ++iGIter)
    {
      // Get the row from the itin matrix iterator
      const ItinIndex::ItinMatrixPair& curPair = (*iGIter);
      const ItinIndex::ItinRow& iRow = curPair.second;

      // Create the necessary iterators to produce the data
      ItinIndex::ItinRowConstIterator iRowIter = iRow.begin();
      ItinIndex::ItinRowConstIterator iRowEndIter = iRow.end();

      // Find the governing carrier from the first fare market of the carrier grouping
      const ItinIndex::ItinCell* topCell;
      topCell = itinIndex.retrieveTopItinCell(curPair.first, ItinIndex::CHECK_FAKEDIRECTFLIGHT);
      if (topCell == nullptr)
      {
        continue;
      }

      const Itin* carrierItin = topCell->second;
      const CarrierCode& curCarrier = carrierItin->fareMarket().front()->governingCarrier();

      const LocCode& origin = carrierItin->travelSeg().front()->boardMultiCity();
      const LocCode& dest = carrierItin->travelSeg().back()->offMultiCity();

      // see if parameters to the diagnostic specify to exclude this
      // fare market
      if ((filterCxr.empty() == false && filterCxr != curCarrier) ||
          (filterFMOrigin.empty() == false && filterFMOrigin != origin) ||
          (filterFMDest.empty() == false && filterFMDest != dest))
      {
        continue;
      }

      dc << "LEG " << _legIndex << " " << origin << "-" << dest << " "
         << "GOVERNING CARRIER " << count++ << " OF " << itinIndex.root().size() << ": "
         << curCarrier << "\n";

      // Loop through the itin row
      for (; iRowIter != iRowEndIter; ++iRowIter)
      {
        const ItinIndex::ItinRowPair& curRowPair = (*iRowIter);
        const ItinIndex::ItinColumn& curColumn = curRowPair.second;

        // Create the necessary iterators to produce the column itin data
        ItinIndex::ItinColumnConstIterator iColumnIter = curColumn.begin();
        ItinIndex::ItinColumnConstIterator iColumnEndIter = curColumn.end();

        for (; iColumnIter != iColumnEndIter; ++iColumnIter)
        {
          const ItinIndex::ItinCell& curCell = (*iColumnIter);
          const Itin* curItin = curCell.second;
          const ItinIndex::ItinCellInfo& itinInfo = curCell.first;
          _primarySector = itinInfo.getPrimarySector();
          if (itinInfo.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT)
          {
            continue;
          }
          if (!_acrossStopOverLeg) // using  external SOP
          {
            if (_shoppingTrx->getTrxType() == PricingTrx::ESV_TRX)
            {
              dc << std::setw(3) << "- SOP "
                 << _shoppingTrx->legs()[_legIndex - 1].sop()[itinInfo.sopIndex()].originalSopId();
            }
            else
            {
              if (curItin->isDummy())
              {
                dc << std::setw(3) << "- SOP "
                   << "DUMMY FLIGHT";
              }
              else
              {
                dc << std::setw(3) << "- SOP "
                   << ShoppingUtil::findSopId(*_shoppingTrx, _legIndex - 1, itinInfo.sopIndex());
                dc << (_shoppingTrx->legs()[_legIndex - 1].sop()[itinInfo.sopIndex()].isCustomSop()
                           ? "     [CUS]"
                           : "");
              }
            }
          }
          else
          {
            dc << std::setw(3) << "- SOP " << itinInfo.sopIndex();
            dc << (_shoppingTrx->legs()[_legIndex - 1].sop()[itinInfo.sopIndex()].isCustomSop()
                       ? "     [CUS]"
                       : "");
          }

          if (numberOfClassesToAdd > 0)
          {
            double score = ItinUtil::calculateEconomyAvailabilityScore(
                curItin, point, numberOfClassesToAdd, requestedNumberOfSeats);
            dc.setf(std::ios::fixed, std::ios::floatfield);
            dc.precision(2);
            dc << "  SCORE " << score;
          }
          if (_shoppingTrx->diversity().isEnabled())
          {
            dc << " FLIGHT DURATION " << curItin->getFlightTimeMinutes() << " TOD "
               << curItin->getTODBucket(_shoppingTrx->diversity().getTODRanges()) << " TPM "
               << curItin->getMileage();
          }

          if (_shoppingTrx->legs()[_legIndex - 1].sop()[itinInfo.sopIndex()].isLngCnxSop())
          {
            dc << " LNGCNX";
          }

          dc << "\n";
          dc << *curItin;
          dc << "\n";
          // display booking code and availability for normal leg only
          if ((!_acrossStopOverLeg) && bookingCodeAvailDisplay)
          {
            displayBookingCodeAvail(*curItin);
          }

          if (_shoppingTrx->getTrxType() == PricingTrx::ESV_TRX)
          {
            const DateTime& reqDepDateTime = _shoppingTrx->getRequest()->requestedDepartureDT();
            const int stopPenalty = (_shoppingTrx->esvOptions().perStopPenalty() * 100);
            const int travDurPenalty = _shoppingTrx->esvOptions().travelDurationPenalty();
            const int depDevPenalty = _shoppingTrx->esvOptions().departureTimeDeviationPenalty();
            displaySOPPenalties(
                *curItin, stopPenalty, travDurPenalty, depDevPenalty, reqDepDateTime);
          }
        }
      }
    }
  }

  return (*this);
}

Diag902Collector& Diag902Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    _shoppingTrx = &shoppingTrx;

    const FlightFinderTrx* ffTrx = dynamic_cast<const FlightFinderTrx*>(_shoppingTrx);
    bool skipFirstLeg = false;
    if (ffTrx != nullptr && (ffTrx->bffStep() == FlightFinderTrx::STEP_4 ||
                       ffTrx->bffStep() == FlightFinderTrx::STEP_6))
    {
      skipFirstLeg = true;
    }

    const std::vector<ShoppingTrx::Leg>& sLV = shoppingTrx.legs();
    if (shoppingTrx.legs().empty())
    {
      return (*this);
    }
    std::vector<ShoppingTrx::Leg>::const_iterator sGLIter = sLV.begin();
    std::vector<ShoppingTrx::Leg>::const_iterator sGLEndIter = sLV.end();

    dc << "**********************************************"
       << "\n";
    dc << "SCHEDULES BY GOVERNING CARRIER AND CONNECTIONS"
       << "\n";
    dc << "**********************************************"
       << "\n";
    dc << "[CUS] - special leg/SOP for custom carrier and routing\n";
    dc << "**********************************************"
       << "\n";
    if (shoppingTrx.diversity().isEnabled())
    {
      dc << "TOD RANGES";
      const std::vector<std::pair<uint16_t, uint16_t> >& todRanges =
          shoppingTrx.diversity().getTODRanges();
      for (size_t i = 0; i < todRanges.size(); ++i)
        dc << " " << i << ": " << std::setfill('0') << std::setw(2) << todRanges[i].first / 60
           << std::setw(2) << todRanges[i].first % 60 << "-" << std::setw(2)
           << todRanges[i].second / 60 << std::setw(2) << todRanges[i].second % 60
           << std::setfill(' ') << std::setw(0);
      dc << "\n";
    }

    int count = 1;
    if (skipFirstLeg)
    {
      ++count;
      ++sGLIter;
    }

    for (; sGLIter != sGLEndIter; ++sGLIter)
    {
      const ShoppingTrx::Leg& curLeg = *sGLIter;
      const ShoppingTrx::SchedulingOption& minFlightDurationSOP =
          curLeg.sop()[curLeg.getMinDurationSopIdx()];

      dc << "********************************************"
         << "\n";
      dc << "THRU-FARE MARKET"
         << "\n";
      dc << "LEG " << count << " OF " << sLV.size() << "\n";
      _acrossStopOverLeg = curLeg.stopOverLegFlag();
      if (_acrossStopOverLeg)
      {
        dc << "#-ACROSS STOPOVER-#"
           << "\n";
      }
      if (curLeg.isCustomLeg())
      {
        dc << "#-[CUS]-#\n";
      }
      if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
      {
        dc << "MINIMAL FLIGHT DURATION IN LEG: "
           << minFlightDurationSOP.itin()->getFlightTimeMinutes() << std::endl;
      }
      dc << "********************************************"
         << "\n";

      _legIndex = count;
      dc << curLeg.carrierIndex();
      count++;
    }
    dc << "**********************************************"
       << "\n";
  }

  return (*this);
}
//--------------------------------------------------------------------------
//  To display booking code and number of seat when DDBKCAVAIL is requested
//--------------------------------------------------------------------------
void
Diag902Collector::displayBookingCodeAvail(const Itin& itin)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    // Return from this method if the itinerary has no
    // travel segments
    if (itin.travelSeg().empty())
    {
      return;
    }
    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);
    // Get travel seg iterator
    std::vector<TravelSeg*>::const_iterator travelSegIter = itin.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSegEndIter = itin.travelSeg().end();
    std::vector<TravelSeg*>::const_iterator tvlIter;
    std::vector<TravelSeg*> travelSegs;
    const std::vector<ClassOfServiceList>* cosVecList = nullptr;
    //-------------------------------------------------------------------------//
    //  1st loop travel seg and display booking code for local availability
    //  2nd loop add more travel each time to display thru market availability
    //-------------------------------------------------------------------------//
    for (; travelSegIter != travelSegEndIter; ++travelSegIter)
    {
      const AirSeg* curSeg = dynamic_cast<const AirSeg*>(*travelSegIter);
      if (curSeg == nullptr)
      {
        continue;
      }
      // keep all travel seg for each thru market availability
      travelSegs.clear();
      travelSegs.push_back(*travelSegIter);
      cosVecList = nullptr;
      cosVecList = &ShoppingUtil::getClassOfService(*_shoppingTrx, travelSegs);
      if (!cosVecList)
      {
        continue;
      }

      dc << curSeg->origAirport() << "-" << curSeg->destAirport() << "\n";

      dc << std::setw(4) << curSeg->carrier() << std::setw(6) << curSeg->flightNumber() << "  ";

      // always 1 set of cos
      std::vector<ClassOfService*>::const_iterator cosItEnd = ((*cosVecList)[0]).end();
      std::vector<ClassOfService*>::const_iterator cosIt = ((*cosVecList)[0]).begin();
      std::ostringstream bookingCode;
      for (; cosIt != cosItEnd; ++cosIt)
      {
        bookingCode << (*cosIt)->bookingCode();
        bookingCode << (*cosIt)->numSeats();
      }
      dc << std::setw(52) << bookingCode.str();
      dc << "\n";
      for (tvlIter = travelSegIter + 1; tvlIter != travelSegEndIter; ++tvlIter)
      {
        travelSegs.push_back(*tvlIter);
        const AirSeg* lastSeg = dynamic_cast<const AirSeg*>(*tvlIter);
        if (lastSeg == nullptr)
        {
          continue;
        }

        dc << curSeg->origAirport() << "-" << lastSeg->destAirport() << "\n";
        cosVecList = nullptr;
        cosVecList = &ShoppingUtil::getClassOfService(*_shoppingTrx, travelSegs);
        if (!cosVecList)
        {
          continue;
        }
        for (uint16_t index = 0; index < cosVecList->size(); ++index)
        {
          curSeg = dynamic_cast<const AirSeg*>(travelSegs[index]);
          if (curSeg == nullptr)
          {
            continue;
          }
          dc << std::setw(4) << curSeg->carrier() << std::setw(6) << curSeg->flightNumber() << "  ";
          std::vector<ClassOfService*>::const_iterator cosItEnd = ((*cosVecList)[index]).end();
          std::vector<ClassOfService*>::const_iterator cosIt = ((*cosVecList)[index]).begin();
          std::ostringstream bookingCodeList;
          for (; cosIt != cosItEnd; ++cosIt)
          {
            bookingCodeList << (*cosIt)->bookingCode();
            bookingCodeList << (*cosIt)->numSeats();
          }
          dc << std::setw(52) << bookingCodeList.str() << "\n";
        }
      }
    }
    dc << "\n";
  }
  return;
}
void
Diag902Collector::displaySOPPenalties(const Itin& itin,
                                      const int& stopPenalty,
                                      const int& travDurPenalty,
                                      const int& depDevPenalty,
                                      const DateTime& reqDepDateTime)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    // Return from this method if the itinerary has no
    // travel segments
    if (itin.travelSeg().empty())
    {
      return;
    }

    MoneyAmount totalStopPenalty =
        ((float)(ShoppingUtil::totalStopPenalty(&itin, stopPenalty)) / 100);
    MoneyAmount totalTravDurPenalty =
        ((float)(ShoppingUtil::totalTravDurPenalty(&itin, travDurPenalty)) / 100);
    MoneyAmount totalDepTimeDevPenalty =
        ((float)(ShoppingUtil::totalDepTimeDevPenalty(&itin, depDevPenalty, reqDepDateTime)) / 100);

    MoneyAmount totalPenalty = totalStopPenalty + totalDepTimeDevPenalty + totalTravDurPenalty;

    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "PENALTIES: " << std::endl;
    dc << std::setw(20) << "STOP: " << std::setw(8) << Money(totalStopPenalty, "NUC") << std::endl;
    dc << std::setw(20) << "DEPTIMEDEV: " << std::setw(8) << Money(totalDepTimeDevPenalty, "NUC")
       << std::endl;
    dc << std::setw(20) << "TRAVDUR: " << std::setw(8) << Money(totalTravDurPenalty, "NUC")
       << std::endl;
    dc << std::setw(20) << "TOTAL: " << std::setw(8) << Money(totalPenalty, "NUC") << std::endl;
    dc << std::endl;
  }
  return;
}
}
