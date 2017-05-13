//----------------------------------------------------------------------------
//  File:        Diag909Collector.C
//  Created:     2004-09-27
//
//  Description: Diagnostic 909 : Routing Validation against flightBitmap
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
#include "Diagnostic/Diag909Collector.h"

#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/Diag903Collector.h"
#include "Diagnostic/DiagCollector.h"

#include <iomanip>

namespace tse
{
Diag909Collector&
Diag909Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    _displayR1 = true; // display rule validation
    if (checkPaxTypeFare(paxFare) == false)
    {
      return *this;
    }

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isDiscounted())
      dc << "D ";
    else if (paxFare.isFareByRule())
      dc << "B ";
    else if (paxFare.fare()->isIndustry())
    {
      const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(paxFare.fare());
      if ((indFare != nullptr) && (indFare->isMultiLateral()))
        dc << "M ";
      else
        dc << "Y ";
    }
    else if (paxFare.fare()->isConstructed())
      dc << "C ";
    else
      dc << "P ";

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2)
       << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
       << std::setw(5) << paxFare.ruleNumber();
    //<< std::setw( 13) << paxFare.fareClass()
    std::string fareBasis = paxFare.createFareBasis(*_trx, false);
    if (fareBasis.size() > 15)
      fareBasis = fareBasis.substr(0, 15) + "*"; // Cross-of-lorraine?
    dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";

    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";

    else
      dc << " ";

    dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

    if (!paxFare.isFareClassAppMissing())
    {
      dc << std::setw(4) << paxFare.fcaFareType();
    }
    else
    {
      dc << "UNK ";
    }

    if (!paxFare.isFareClassAppSegMissing())
    {
      if (paxFare.fcasPaxType().empty())
        dc << "*** ";
      else
        dc << std::setw(4) << paxFare.fcasPaxType();
    }
    else
    {
      dc << "UNK ";
    }
    dc << " ";
    dc << std::setw(5) << paxFare.routingNumber();

    if (paxFare.flightBitmap().size() == 0)
    {
      dc << "\n";
      return (*this);
    }

    dc << std::endl << " ";

    PaxTypeFare::FlightBitmapConstIterator bitMapIter = paxFare.flightBitmap().begin();
    PaxTypeFare::FlightBitmapConstIterator bitMapEndIter = paxFare.flightBitmap().end();
    setDiagWrapMode(DiagCollector::DiagWrapSimple);
    setLineWrapLength(DiagCollector::DEFAULT_LINEWRAP_LENGTH);
    setAlignmentMark();
    setLineWrapAnchor();
    for (; bitMapIter != bitMapEndIter; ++bitMapIter)
    {
      const uint8_t& fareBit = static_cast<const uint8_t&>(bitMapIter->_flightBit);
      if (fareBit == 0)
      {
        dc << "-";
      }
      else
      {
        dc << std::setw(1) << fareBit;
      }
      // Will only wrap line if needed
      if (wrapLine())
      {
        setLineWrapAnchor();
      }
    }
    setDiagWrapMode(DiagCollector::DiagWrapNone);
    dc << " ";
    dc << '\n';
  }
  return *this;
}

Diag909Collector&
Diag909Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    // If we dont have travel segments, we count output this line
    if (fareMarket.travelSeg().size() == 0)
      return *this;

    std::vector<TravelSeg*>::const_iterator tvlSegItr;
    tvlSegItr = fareMarket.travelSeg().begin();

    std::string globalDirStr;
    globalDirectionToStr(globalDirStr, fareMarket.getGlobalDirection());

    dc << " #  " << fareMarket.getDirectionAsString() << "\n";
    dc << "MARKETING CARRIER: " << fareMarket.governingCarrier() << "\n";

    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
    std::vector<uint32_t> sops;
    if (!paxTypeCortegeVec.empty())
    {
      std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
      std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();

      while (ptcIt != ptcEnd)
      {
        const PaxTypeBucket& cortege = *ptcIt;
        const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
        if (!paxFareVec.empty())
        {
          dc << '\n';
          dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';

          dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';

          dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';
          dc << '\n';

          dc << "                                                              \n";
          dc << "                                                              \n";
          dc << "                                                              \n";
          dc << "                                                              \n";
          dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX      \n";
          dc << "                            NUM R I              TYP TYP  ROUT\n";
          dc << "- -- - ---- --------------- --- - - -------- --- --- ---  ----\n";

          std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
          std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();
          while (ptfIt != ptfEnd)
          {
            PaxTypeFare& paxFare = **ptfIt;
            dc << paxFare;
            ++ptfIt;
          }
        }
        else
        {
          dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
             << fareMarket.destination()->loc()
             << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
        }
        ++ptcIt;
      }
    }
    else
    {
      dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << '\n';
    }

    dc << '\n';
  }

  return *this;
}

Diag909Collector&
Diag909Collector::operator<<(const ItinIndex& itinIndex)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    if (itinIndex.root().empty())
    {
      return (*this);
    }

    // Adjust dc output to left justified
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::endl;

    ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
    ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();
    const ItinIndex::ItinCell* curCell; // Cell instance

    const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");
    const std::string& filterFM = dc.rootDiag()->diagParamMapItem("FM");

    std::string filterFMOrigin, filterFMDest;
    if (filterFM.size() == 6)
    {
      filterFMOrigin = filterFM.substr(0, 3);
      filterFMDest = filterFM.substr(3, 3);
    }

    int count = 1;
    for (; iGIter != iGEndIter; ++iGIter)
    {
      // Get the leaf
      curCell = ShoppingUtil::retrieveDirectItin(
          itinIndex, iGIter->first, ItinIndex::CHECK_FAKEDIRECTFLIGHT);

      // If the leaf retrieval failed, go to the next itinerary
      if (!curCell)
      {
        continue;
      }

      // Retrieve the itinerary
      const Itin* curItin = curCell->second;

      // Find the governing carrier from the first fare market of the carrier grouping
      const CarrierCode& curCarrier = curItin->fareMarket().front()->governingCarrier();

      const LocCode& origin = curItin->travelSeg().front()->boardMultiCity();
      const LocCode& dest = curItin->travelSeg().back()->offMultiCity();

      // see if parameters to the diagnostic specify to exclude this
      // fare market
      if ((filterCxr.empty() == false && filterCxr != curCarrier) ||
          (filterFMOrigin.empty() == false && filterFMOrigin != origin) ||
          (filterFMDest.empty() == false && filterFMDest != dest))
      {
        continue;
      }

      dc << "LEG " << _legIndex << " GOVCXR " << count << " OF " << itinIndex.root().size() << " : "
         << curCarrier << std::endl;

      // Get the leaf
      curCell =
          ShoppingUtil::retrieveDirectItin(itinIndex, iGIter->first, ItinIndex::CHECK_NOTHING);

      if (!curCell)
      {
        continue;
      }

      const Itin* topItin = curCell->second;

      // Set the cur itin row
      curItinRow = static_cast<const ItinIndex::ItinRow*>(&(iGIter->second));
      const std::vector<FareMarket*>& fareMarkets(topItin->fareMarket());
      ShoppingTrx& shoppingTrx = getShoppingTrx();
      size_t fmIdx(1);

      for (FareMarket* fm : fareMarkets)
      {
        dc << "--------------------------------------------------------\n";
        if (shoppingTrx.isIataFareSelectionApplicable())
        {
          dc << "MARKET #" << fmIdx++ << ":" << std::endl;
        }
        dc << curItin->travelSeg().front()->boardMultiCity() << "-"
           << curItin->travelSeg().back()->offMultiCity();
        dc << *fm;

        if (!shoppingTrx.isIataFareSelectionApplicable())
          break;
      }

      count++;
    }
    dc << "--------------------------------------------------------" << std::endl;
  }

  return (*this);
}

Diag909Collector&
Diag909Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    const std::vector<ShoppingTrx::Leg>& sLV = shoppingTrx.legs();
    if (shoppingTrx.legs().empty())
    {
      return (*this);
    }

    std::vector<ShoppingTrx::Leg>::const_iterator sGLIter = sLV.begin();
    std::vector<ShoppingTrx::Leg>::const_iterator sGLEndIter = sLV.end();
    int count = 1;
    for (; sGLIter != sGLEndIter; ++sGLIter)
    {
      const ShoppingTrx::Leg& curLeg = *sGLIter;
      _legIndex = count;
      dc << curLeg.carrierIndex();
      count++;
    }
  }

  return (*this);
}
}
