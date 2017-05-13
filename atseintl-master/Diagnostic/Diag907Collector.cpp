//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Diagnostic/Diag907Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagCollector.h"

#include <iomanip>

namespace tse
{
void
Diag907Collector::outputFareBitDesc(const PaxTypeFare& paxFare,
                                    double point,
                                    int numberOfClassesToAdd,
                                    int requestedNumberOfSeats)
{
  if (_active)
  {
    _displayR1 = true; // display rule validation
    DiagCollector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);

    ItinIndex* curLegIdx = const_cast<ItinIndex*>(_curLegCarrierIndex);
    uint32_t legId = _legIndex - 1;

    ShoppingTrx& shoppingTrx = getShoppingTrx();

    ItinIndex::ItinIndexIterator iRCIter =
        (_curLeg->stopOverLegFlag())
            ? curLegIdx->beginAcrossStopOverRow(shoppingTrx, legId, *_curItinRowKey)
            : curLegIdx->beginRow(*_curItinRowKey);

    ItinIndex::ItinIndexIterator iRCEIter =
        (_curLeg->stopOverLegFlag()) ? curLegIdx->endAcrossStopOverRow() : curLegIdx->endRow();

    const SOPUsages* sopUsages = nullptr;
    if (paxFare.fareMarket() && paxFare.fareMarket()->getApplicableSOPs())
    {
      const FareMarket& fareMarket = *paxFare.fareMarket();
      ApplicableSOP::const_iterator applicableSOPIt =
          fareMarket.getApplicableSOPs()->find(*_curItinRowKey);
      if (applicableSOPIt != fareMarket.getApplicableSOPs()->end())
      {
        sopUsages = &applicableSOPIt->second;
      }
    }

    uint32_t bitId = 0;
    for (; iRCIter != iRCEIter; ++iRCIter, ++bitId)
    {
      dc << std::setw(5) << bitId << " ";
      const ItinIndex::ItinCell& curCell = *iRCIter;
      const ItinIndex::ItinCellInfo& curCellInfo = curCell.first;
      const Itin* curCellItin = curCell.second;
      const TravelSeg* primarySector = curCellInfo.getPrimarySector();

      if (iRCIter.currentSopSet().empty() == false)
      {
        for (const auto sopId : iRCIter.currentSopSet())
        {
          if (!shoppingTrx.legs()[_legIndex - 1].stopOverLegFlag())
          {
            dc << std::setw(9) << ShoppingUtil::findSopId(shoppingTrx, _legIndex - 1, sopId) << " ";
            dc << std::setw(4)
               << (shoppingTrx.legs()[_legIndex - 1].sop()[sopId].isHighTPM() ? "T" : "F");
            dc << std::setw(9)
               << (shoppingTrx.legs()[_legIndex - 1].sop()[sopId].isCustomSop() ? "[CUS]" : " - ");
          }
        }
      }
      else
      {
        if (!shoppingTrx.legs()[_legIndex - 1].stopOverLegFlag())
        {
          dc << std::setw(9)
             << ShoppingUtil::findSopId(shoppingTrx, _legIndex - 1, curCellInfo.sopIndex());
          dc << std::setw(4)
             << (shoppingTrx.legs()[_legIndex - 1].sop()[curCellInfo.sopIndex()].isHighTPM() ? "T" : "F");
          dc << std::setw(9)
             << (shoppingTrx.legs()[_legIndex - 1].sop()[curCellInfo.sopIndex()].isCustomSop()
                     ? "[CUS]"
                     : " - ");
        }
      }
      if (numberOfClassesToAdd > 0)
      {
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(5) << ItinUtil::calculateEconomyAvailabilityScore(
                                  curCellItin, point, numberOfClassesToAdd, requestedNumberOfSeats);
      }

      dc << "\n\n";

      if ((shoppingTrx.getTrxType() == PricingTrx::FF_TRX) &&
          (shoppingTrx.isValidatingCxrGsaApplicable()))
      {
        dc << "VALIDATING CARRIERS :";
        std::vector<CarrierCode> participatingCarriers;
        ValidatingCxrUtil::getParticipatingCxrs(shoppingTrx, *curCellItin, participatingCarriers);
        for (const auto participatingCarrier : participatingCarriers)
        {
          dc << participatingCarrier << " ";
        }
        dc << "\n";
      }

      const std::vector<TravelSeg*>& travelSegments = curCellItin->travelSeg();
      std::vector<TravelSeg*>::const_iterator travelSegIt = travelSegments.begin();
      std::vector<TravelSeg*>::const_iterator travelSegEndIt = travelSegments.end();

      const SOPUsage* sopUsage = sopUsages ? &sopUsages->at(iRCIter.bitIndex()) : nullptr;
      if (sopUsage && !sopUsage->applicable_)
      {
        dc << "- NOT APPLICABLE\n";
        continue;
      }

      if (sopUsage && sopUsage->startSegment_ != -1 && sopUsage->endSegment_ != -1)
      {
        travelSegIt = travelSegments.begin() + sopUsage->startSegment_;
        travelSegEndIt = travelSegments.begin() + sopUsage->endSegment_ + 1;
      }

      for (uint32_t i = 0; travelSegIt != travelSegEndIt; ++travelSegIt, ++i)
      {
        if (const AirSeg* airSeg = static_cast<AirSeg*>(*travelSegIt))
        {
          const LocCode& origin = airSeg->origin()->loc();
          const LocCode& destin = airSeg->destination()->loc();
          const CarrierCode& cxr = airSeg->carrier();
          const FlightNumber& fNum = airSeg->flightNumber();
          const DateTime& depDT = airSeg->departureDT();
          dc << "- SEGMENT " << std::setw(2) << i << ": " << std::setw(4) << origin << "("
             << airSeg->boardMultiCity() << ") "
             << "- " << std::setw(4) << destin << "(" << airSeg->offMultiCity() << ")"
             << " " << std::setw(3) << cxr << " " << std::setw(5) << fNum << " " << std::setw(5)
             << depDT.dateToString(DDMMM, "");

          if (primarySector == airSeg)
          {
            dc << " *"; // this is governing sector
          }
        }
        else
        {
          dc << "- ARUNK";
        }

        dc << "\n";
      }
    }
  }
}

Diag907Collector&
Diag907Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = *this;

    // If we dont have travel segments, we count output this line
    if (fareMarket.travelSeg().size() == 0)
      return *this;

    dc << "--------------------------------------------------------\n";
    dc << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc();
    if (_shoppingTrx->isSumOfLocalsProcessingEnabled() &&
        fareMarket.getFmTypeSol() == FareMarket::SOL_FM_LOCAL)
    {
      dc << " # LOCAL FM";
    }

    dc << " #  " << fareMarket.getDirectionAsString() << "\n";
    dc << "MARKETING CARRIER: " << fareMarket.governingCarrier() << "\n";

    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
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
          int requestedNumberOfSeats = PaxTypeUtil::totalNumSeats(*_shoppingTrx);
          double point = 0.0;
          int numberOfClassesToAdd = 0;

          ItinUtil::readConfigDataForSOPScore(point, numberOfClassesToAdd);

          if (numberOfClassesToAdd > 0)
          {
            dc << "\nBIT   SOP     TPM SOP      SOP   FLIGHT\n";
            dc << "ID    ID      T/F TYPE     SCORE INFO\n";
            dc << "----- ------- --- -------- ----- ---------------------------------------------\n";

            outputFareBitDesc(
                *paxFareVec.front(), point, numberOfClassesToAdd, requestedNumberOfSeats);
          }
          else
          {
            dc << "\nBIT   SOP     TPM SOP      FLIGHT\n";
            dc << "ID    ID      T/F TYPE     INFO\n";
            dc << "----- ------- --- -------- ---------------------------------------------\n";
            outputFareBitDesc(*paxFareVec.front());
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

Diag907Collector&
Diag907Collector::operator<<(const ItinIndex& itinIndex)
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

    dc << "\n";

    ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
    ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();
    const ItinIndex::ItinCell* curCell; // Cell instance

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

      dc << "--------------------------------------------------------"
         << "\n";
      dc << "LEG " << _legIndex << " GOVCXR " << count << " OF " << itinIndex.root().size() << " : "
         << curCarrier << "\n";

      // Get the leaf
      curCell =
          ShoppingUtil::retrieveDirectItin(itinIndex, iGIter->first, ItinIndex::CHECK_NOTHING);

      if (!curCell)
      {
        continue;
      }

      const Itin* topItin = curCell->second;

      // Set the cur itin row
      _curItinRowKey = &(iGIter->first);
      _curLegCarrierIndex = &itinIndex;

      const std::vector<FareMarket*>& fareMarkets = topItin->fareMarket();
      for (const auto fm : fareMarkets)
      {
        const FareMarket& fareMarket = *fm;
        dc << fareMarket;

        if (!_shoppingTrx->isSumOfLocalsProcessingEnabled() &&
           (!_shoppingTrx->isIataFareSelectionApplicable()))
          break;
      }
      count++;
    }
    dc << "--------------------------------------------------------"
       << "\n";
  }

  return (*this);
}

Diag907Collector&
Diag907Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  _shoppingTrx = &shoppingTrx;

  if (_active)
  {
    DiagCollector& dc = *this;

    if (PricingTrx::ESV_TRX == shoppingTrx.getTrxType())
    {
      dc << "NOT VIS DIAGNOSTIC" << std::endl;

      return (*this);
    }

    const std::vector<ShoppingTrx::Leg>& sLV = shoppingTrx.legs();
    if (shoppingTrx.legs().empty())
    {
      return (*this);
    }

    std::vector<ShoppingTrx::Leg>::const_iterator sGLIter = sLV.begin();
    std::vector<ShoppingTrx::Leg>::const_iterator sGLEndIter = sLV.end();

    dc << "***************************************************"
       << "\n";
    dc << "907: FLIGHT SCHEDULES IN INTERNAL PROCESSING ORDER\n";
    dc << "***************************************************"
       << "\n";
    dc << "[CUS] - special leg/SOP for custom carrier and routing\n";
    dc << "***************************************************"
       << "\n";
    dc << "\n";

    int count = 1;
    for (; sGLIter != sGLEndIter; ++sGLIter)
    {
      const ShoppingTrx::Leg& curLeg = *sGLIter;
      dc << "THRU AND LOCAL FARE MARKET"
         << "\n";

      dc << "LEG " << count << " OF " << sLV.size() << "\n";
      if (curLeg.stopOverLegFlag())
      {
        dc << "#-ACROSS STOPOVER-#"
           << "\n";
      }
      if (curLeg.isCustomLeg())
      {
        dc << "#-[CUS]-#"
           << "\n";
      }
      _legIndex = count;
      _curLeg = static_cast<const ShoppingTrx::Leg*>(&curLeg);
      dc << curLeg.carrierIndex();
      count++;
    }
  }

  return (*this);
}

} // namespace tse
