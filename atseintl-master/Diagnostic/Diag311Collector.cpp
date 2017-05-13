//----------------------------------------------------------------------------
//  File:        Diag311Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 311 to display BlackoutDates data
//
//  Updates:
//          05/21/2004  TB - create.
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

#include "Diagnostic/Diag311Collector.h"

#include "Common/FareDisplayUtil.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/BlackoutDates.h"
#include "Rules/RuleItem.h"

using namespace std;

namespace tse
{
void
Diag311Collector::diag311Collector(const PaxTypeFare& paxFare,
                                   const CategoryRuleInfo& ruleInfo,
                                   const BlackoutDates& blackoutDates,
                                   const Record3ReturnTypes status,
                                   const int phase,
                                   PricingTrx& trx,
                                   bool isInbound,
                                   const PricingUnit* pricingUnit)
{
  if (!_active)
    return;

  DiagCollector& dc = *this; // Just a shorthand

  dc << "CATEGORY 11 BLACKOUTS DATES APPLICATION DIAGNOSTICS" << endl << endl;

  dc << "PHASE:";
  dc << setw(18) << RuleItem::getRulePhaseString(phase);

  dc << "     R3 ITEM NUMBER:";
  char fillChar = dc.fill();
  int width = dc.width();
  dc << setw(10) << setfill('0') << blackoutDates.info().itemNo() << endl;
  dc.width(width);
  dc.fill(fillChar);

  // Segments
  // MIA SJU HKXE3N    R2:FARERULE   :  ATP AA 867     5485
  dc << paxFare.fareMarket()->origin()->loc() << " " << paxFare.fareMarket()->destination()->loc()
     << " " << setw(7) << paxFare.fareClass() << "    R2:FARERULE   : " << setw(4)
     << paxFare.vendor() << " " << setw(width) << paxFare.carrier() << " " << setw(4)
     << paxFare.fareTariff() << " " << setw(9) << ruleInfo.ruleNumber() << setw(width) << endl;

  if (blackoutDates.info().geoTblItemNoBetween() != 0)
  {
    dc << "BLACKOUTS : GEO TABLE ITEM NO. - " << setw(6)
       << blackoutDates.info().geoTblItemNoBetween();

    if (blackoutDates.info().geoTblItemNoAnd() != 0)
      dc << " " << blackoutDates.info().geoTblItemNoAnd();

    dc << setw(width) << endl;

    if (_geoMatched)
      dc << " BLACKOUTS : MATCHED ON GEO DATA REQUIREMENT" << endl;
  }

  dc << "BLACKOUTS START DATE : ";
  if (blackoutDates.info().tvlStartYear() < 0)
  {
    dc << "  ";
  }
  else
  {
    dc << setw(2) << setfill('0') << blackoutDates.info().tvlStartYear();
  }
  dc << "-" << setw(2) << blackoutDates.info().tvlStartMonth() << "-" << setw(2)
     << blackoutDates.info().tvlStartDay() << endl;

  dc << "BLACKOUTS STOP DATE : ";
  if (blackoutDates.info().tvlStopYear() < 0)
  {
    dc << "  ";
  }
  else
  {
    dc << setw(2) << setfill('0') << blackoutDates.info().tvlStopYear();
  }
  dc << "-" << setw(2) << blackoutDates.info().tvlStopMonth() << "-"
     << blackoutDates.info().tvlStopDay() << endl;

  dc << setw(width) << setfill(fillChar);

  FareDisplayTrx* fdTrx;
  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx))
  {
    DateTime depDate = paxFare.fareMarket()->travelSeg().front()->departureDT();
    NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
    if (noPnrTrx != nullptr)
    {
      // NoPNR processing  - departure date may have to be updated for display
      if (trx.itin().front()->dateType() == Itin::NoDate)
      {
        // for itineraries w/o date, always use current during validation
        depDate = DateTime::localTime();
      }
      else if (pricingUnit != nullptr)
      {
        depDate = pricingUnit->travelSeg().front()->departureDT();
        noPnrTrx->updateOpenDateIfNeccesary(pricingUnit->travelSeg().front(), depDate);
      }
      else if (paxFare.fareMarket()->travelSeg().front()->hasEmptyDate())
      {
        depDate = DateTime::emptyDate();
      }
    }

    {
      PricingTrx* pTrx = dynamic_cast<PricingTrx*>(&trx);
      if (!pTrx || paxFare.fareMarket()->travelSeg().size() == 1)
      {
        dc << "DEPARTURE DATE : " << depDate.dateToIsoExtendedString() << endl;
      }
      else
      {
        dc << "DEPARTURE DATES\n";
        std::vector<TravelSeg*>::const_iterator tvlSegI = paxFare.fareMarket()->travelSeg().begin();
        std::vector<TravelSeg*>::const_iterator tvlSegEndI =
            paxFare.fareMarket()->travelSeg().end();
        for (; tvlSegI != tvlSegEndI; ++tvlSegI)
        {
          dc << "       " << (*tvlSegI)->origAirport() << "-" << (*tvlSegI)->destAirport() << " : "
             << (*tvlSegI)->departureDT().dateToIsoExtendedString() << std::endl;
        }
      }
    }
  }
  else
  {
    if (isInbound)
    {
      dc << "RETURN    DATE : " << fdTrx->inboundFareMarket()
                                       ->travelSeg()[0]
                                       ->earliestDepartureDT()
                                       .dateToIsoExtendedString() << endl;
    }
    else
    {
      dc << "DEPARTURE DATE : "
         << paxFare.fareMarket()->travelSeg()[0]->departureDT().dateToIsoExtendedString() << endl;
    }
  }

  dc << "BLACKOUT: ";
  if (status == PASS)
    dc << "PASSED";
  else if (status == SKIP)
    dc << "SKIPPED";
  else if (status == SOFTPASS)
    dc << "SOFTPASSED";
  else if (status == FAIL)
    dc << "FAILED";
  else
    dc << status;
  dc << endl;
  dc << "***************************************************************" << endl;
}

} // tse
