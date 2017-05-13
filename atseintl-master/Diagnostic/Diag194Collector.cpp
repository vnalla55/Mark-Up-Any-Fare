//----------------------------------------------------------------------------
//  File:        Diag194Collector.C
//  Authors:     Grzegorz Cholewiak
//  Created:     Apr 22 2007
//
//  Description: Diagnostic 194 formatter
//
//  Updates:
//          date - initials - description.
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
#include "Diagnostic/Diag194Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/InternalDiagUtil.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{

namespace
{
const std::string header = "***************** START DIAG 194 ************************** \n";
const std::string footer = "***************** END   DIAG 194 ************************** \n";
const std::string divider = "*********************************************************** \n";
}

Diag194Collector&
Diag194Collector::operator << (const Itin& itn)
{
  if (_active)
  {
    std::vector<TravelSeg*>::const_iterator tsIterator = itn.travelSeg().begin();
    for (; tsIterator != itn.travelSeg().end(); ++tsIterator)
      prepareSegmentDescriptionRow(*tsIterator);
  }

  return *this;
}

void
Diag194Collector::printDate(const std::string& dateTitle,
                            const std::string& timeTitle,
                            const DateTime& dt)
{
  if (dt.isValid())
  {
    *this << dateTitle << ": " << dt.dateToString(YYYYMMDD, "-");

    if (dt.historicalIncludesTime())
      *this << " " << timeTitle << ": " << dt.timeToString(HHMM, ":");

    *this << "\n";
  }
}

void
Diag194Collector::printTransactionDates(const BaseExchangeTrx& trx)
{
  printDate("    ORIGINAL TICKET DATE D92", " TIME T92", trx.originalTktIssueDT());

  if(trx.ticketingDate().isValid())
    printDate("     CURRENT TICKET DATE D07", " TIME D54", trx.ticketingDate());
  else
    printDate("     CURRENT TICKET DATE D07", " TIME D54", trx.currentTicketingDT());
  printDate("LAST TICKET REISSUE DATE D94", " TIME T94", trx.lastTktReIssueDT());
  printDate("           PURCHASE DATE D93", "     TIME", trx.purchaseDT());

  if (trx.applyReissueExcPrimary())
    printDate("TRANS TYPE EXCHANGE DATE D95", " TIME T95", trx.previousExchangeDT());

}

//----------------------------------------------------------------------------
// operator << RexPricingTrx
// ---------------------------------------------------------------------------

Diag194Collector&
Diag194Collector::operator << (const RexPricingTrx& trx)
{
  if (_active)
  {
    _reshopTrx = (trx.getTrxType() == PricingTrx::RESHOP_TRX);
    const RexExchangeTrx* rexExcTrx = dynamic_cast<const RexExchangeTrx*>(&trx);

    DiagCollector& dc = (DiagCollector&)*this;
    dc << header;

    printTransactionDates(trx);

    ExcItin* excItin = const_cast<ExcItin*>(trx.exchangeItin().front());
    std::vector<Itin*>::const_iterator itinIt = trx.newItin().begin();
    for (size_t index = 0; itinIt != trx.newItin().end(); ++itinIt, index++)
    {
      const Itin* newItin = *itinIt;
      (const_cast<RexPricingTrx*>(&trx))->setItinIndex(index);

      dc << "*** PREVIOUSLY TICKETED ITINERARY ***" << std::endl;
      if (trx.applyReissueExcPrimary())
      {
        const RexPricingOptions& rexOptions = trx.getRexOptions();
        if (!rexOptions.excTotalFareAmt().empty())
          dc << "BASE FARE AMOUNT: " << rexOptions.excTotalFareAmt() << " "
             << rexOptions.excBaseFareCurrency() << std::endl;
        dc << "FARE LEVEL: " << (rexOptions.isNetSellingIndicator() ? "NET\n" : "SELL\n");
      }

      _processingExItin = true;
      dc << *excItin;

      dc << "*** NEW  ITINERARY ";
      if (rexExcTrx)
      {
        dc << "WITH INDEX " << index << " ";
      }
      dc << "***" << std::endl;
      _processingExItin = false;
      dc << *newItin;

      if (index + 1 < trx.newItin().size())
        dc << divider;
    }

    if (trx.applyReissueExcPrimary())
      dc << " EXCHANGE REISSUE: Y\n\n";
    else
      dc << " EXCHANGE REISSUE: N\n\n";
    dc << footer;
  }
  return *this;
}

//----------------------------------------------------------------------------
// operator << ExchangePricingTrx
// ---------------------------------------------------------------------------

Diag194Collector&
Diag194Collector::operator << (const ExchangePricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << header;
    if (trx.reqType() == "CE")
    {
      dc << "*** TAG 10 EXCHANGE REQUEST TYPE ***" << std::endl;
    }

    printTransactionDates(trx);

    if (!trx.exchangeItin().empty())
    {
      dc << "*** PREVIOUSLY TICKETED ITINERARY ***" << std::endl;
      dc << *trx.exchangeItin().front();
      dc << "*** NEW  ITINERARY ***" << std::endl;
      dc << *trx.newItin().front();
      dc << "******** \n\n";

      if (trx.exchangeItin().front()->stopOverChange())
        dc << " STOPOVER CHANGE: Y\n\n";
      else
        dc << " STOPOVER CHANGE: N\n\n";
    }
    else
      dc << *trx.newItin().front();

    const BaseExchangeTrx* baseTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
    if (baseTrx != nullptr && baseTrx->applyReissueExcPrimary())
      dc << " EXCHANGE REISSUE: Y\n\n";
    else
      dc << " EXCHANGE REISSUE: N\n\n";
    dc << footer;
  }
  return *this;
}

Diag194Collector&
Diag194Collector::operator << (const RefundPricingTrx& trx)
{
  if (_active)
  {
    _refundDiagnostic = true;
    *this << header;
    printTransactionDates(trx);
    *this << "*** ORIGINALLY TICKETED ITINERARY ***" << std::endl;
    if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
    {
      *this << "FARE LEVEL: " << (trx.getRexOptions().isNetSellingIndicator() ? "NET\n" : "SELL\n");
    }
    *this << *trx.exchangeItin().front();
    *this << "*** FLOWN SEGMENTS ***" << std::endl;
    *this << *trx.newItin().front();
    *this << footer;
  }
  return *this;
}

void
Diag194Collector::prepareSegmentDescriptionRow(const TravelSeg* travelSeg)
{
  _travelSeg = travelSeg;

  if (!trx())
  {
    *this << std::setw(2) << _travelSeg->pnrSegment();

    addCarrierFlightCode();
    addDate();
    addCityPair();
  }
  else
  {
    InternalDiagUtil idu(*trx(), *this);
    idu.printSegmentInfo(*travelSeg);
  }

  *this << std::setw(3);

  if (_refundDiagnostic)
    addRefundCode();

  else if (_reshopTrx)
    addReshopCode();

  else
    addChangeStatus();

  if(!_refundDiagnostic)
    *this << "/" << (travelSeg->unflown() ? "U" : "F");

  *this << "\n";
}

void
Diag194Collector::addCarrierFlightCode()
{
  const AirSeg* tvlSegA = dynamic_cast<const AirSeg*>(_travelSeg);
  if (tvlSegA)
  {
    *this << std::setw(3) << tvlSegA->carrier();

    (tvlSegA->segmentType() == Open) ? *this << "OPEN" : *this << std::setw(4)
                                                               << tvlSegA->flightNumber();

    *this << tvlSegA->getBookingCode();
  }

  else
    *this << std::setw(5) << "ARNK"
          << "   ";
}

void
Diag194Collector::addDate()
{
  *this << std::setw(8);

  if (_travelSeg->segmentType() == Open)
  {
    std::string sDepartureDate = _travelSeg->pssDepartureDate();

    if (sDepartureDate != "")
    {
      sDepartureDate += " 12:00";
      DateTime dtDepartureDate = DateTime(sDepartureDate);
      *this << dtDepartureDate.dateToString(DDMMMYY, "");
    }
    else
      *this << "NONE";
  }
  else
    *this << _travelSeg->departureDT().dateToString(DDMMMYY, "");
}

void
Diag194Collector::addCityPair()
{
  *this << std::setw(4) << _travelSeg->origAirport() << _travelSeg->destAirport();
}

void
Diag194Collector::addRefundCode()
{
  _travelSeg->unflown() ? *this << 'U' : *this << 'F';
}

void
Diag194Collector::addReshopCode()
{
  // Don't print statuses for exc Itin
  if (_processingExItin)
    return;

  addChangeStatus();

  if (_travelSeg->isShopped())
    *this << "  SHOPPED";
}

void
Diag194Collector::addChangeStatus()
{
  switch (_travelSeg->changeStatus())
  {
  case TravelSeg::CHANGED:
    *this << 'C';
    break;
  case TravelSeg::UNCHANGED:
    *this << 'U';
    break;
  case TravelSeg::INVENTORYCHANGED:
    *this << 'I';
    break;
  case TravelSeg::CONFIRMOPENSEGMENT:
    *this << 'O';
    break;
  }
}
}
