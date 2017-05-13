//----------------------------------------------------------------------------
//  File:        Diag304Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 304 to display Flight Application data
//
//  Updates:
//          05/06/2004  TB - create.
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

#include "Diagnostic/Diag304Collector.h"

#include "Common/FareMarketUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/Loc.h"

#include <iomanip>

using namespace std;

namespace tse
{
void
Diag304Collector::diag304Collector(const PaxTypeFare& paxFare,
                                   const FlightAppRule& flightAppRule,
                                   const PricingRequest& request)
{
  if (!_active)
    return;

  DiagCollector& dc = *this; // Just a shorthand

  dc.printLine();
  dc << "  FLIGHT APPLICATION VALIDATION FOR R3 - " << paxFare.fareClass() << " " << setw(6)
     << flightAppRule.vendor() << " " << flightAppRule.itemNo() << endl;

  // First line
  // LGA  /NYC  -AA  705 2004-06-10 -DFW  /DFW
  // origin airport/city, -cxr, flt#, departure date, -dest airport/city

  dc << "FLIGHT INFO: ";

  vector<TravelSeg*>::const_iterator it;
  it = paxFare.fareMarket()->travelSeg().begin();
  for (; it != paxFare.fareMarket()->travelSeg().end(); ++it)
  {

    dc << setw(3) << (*it)->origAirport() << " /" << setw(3)
       << FareMarketUtil::getBoardMultiCity(*paxFare.fareMarket(), **it) << "/";

    AirSeg* seg = dynamic_cast<AirSeg*>(*it);
    if (seg)
    {
      dc << "  -" << seg->carrier() << " " << setw(4) << seg->flightNumber() << " ";
      if (!seg->operatingCarrierCode().empty())
        dc << seg->operatingCarrierCode() << " ";
    }
    else
      dc << "  ARUNK SEG ";

    dc << setw(3) << (*it)->destAirport() << "  /" << setw(3)
       << FareMarketUtil::getOffMultiCity(*paxFare.fareMarket(), **it) << "/" << endl;
  }

  // NYC   DFW   ATP  245 AA  7399 ONTP3SON
  // fare origin, fare dest, rule vendor, rule tariff number, rule carrier,
  //   rule number, fare class code
  dc << "MARKET INFO: ";

  dc << setfill(' ') << paxFare.fareMarket()->origin()->loc() << setw(6)
     << paxFare.fareMarket()->destination()->loc();
  dc << " " << setw(4) << setfill('0')
     << paxFare.fareMarket()->travelSeg()[0]->departureDT().date().year() << "-" << setw(2)
     << paxFare.fareMarket()->travelSeg()[0]->departureDT().date().month() << "-" << setw(2)
     << paxFare.fareMarket()->travelSeg()[0]->departureDT().date().day() << endl;

  // TICKETING DATE - 2004-04-08
  // ticketing date (_ticketingDT from Request class)
  dc << "TICKETING DATE - " << setw(4) << setfill('0') << request.ticketingDT().date().year() << "-"
     << setw(2) << request.ticketingDT().date().month() << "-" << setw(2)
     << request.ticketingDT().date().day() << endl;

#if 0
       dc << setfill(' ') << "R2 DATA: " ;
       dc << setw(6) << ruleInfo.vendorCode() << " "
       << setw(3) << ruleInfo.carrierCode() << " "
       << setw(5) << ruleInfo.tariffNumber() << " "
       << setw(3) << ruleInfo.ruleNumber() << " "
       << paxFare.fareClass()
       << endl;
#endif

  dc << "R3 FLIGHT " << endl;
  dc << " FLT APPL: " << flightAppRule.fltAppl() << " FLT1: " << flightAppRule.carrier1() << setw(5)
     << flightAppRule.flt1() << " : " << flightAppRule.fltRelational1()
     << " FLT2: " << flightAppRule.carrier2() << setw(5) << flightAppRule.flt2() << " : "
     << flightAppRule.fltRelational2() << " FLT3: " << flightAppRule.carrier3() << setw(5)
     << flightAppRule.flt3() << endl;

  dc << " TBL986-1: " << flightAppRule.carrierFltTblItemNo1()
     << " TBL986-2: " << flightAppRule.carrierFltTblItemNo2() << " DOW: " << flightAppRule.dow()
     << endl;

  dc << "R3 TRAVEL RESTR " << endl;
  dc << " INOUT: " << flightAppRule.inOutInd() << " GEO APPL: " << flightAppRule.geoAppl()
     << " VIA APPL: " << flightAppRule.viaInd() << " LOC APPL: " << flightAppRule.locAppl() << endl;

  dc << " TBL995 BTWVIA: " << flightAppRule.geoTblItemNoBetwVia()
     << " ANDVIA: " << flightAppRule.geoTblItemNoAndVia()
     << " VIA: " << flightAppRule.geoTblItemNoVia() << endl;

  dc << " HIDDEN: " << flightAppRule.hidden() << " NONSTOP: " << flightAppRule.fltNonStop()
     << " DIRECT: " << flightAppRule.fltDirect() << " MULTI STOP : " << flightAppRule.fltMultiStop()
     << " ONE STOP: " << flightAppRule.fltOneStop() << endl;

  dc << " ONLINE: " << flightAppRule.fltOnline() << " INTERLINE: " << flightAppRule.fltInterline()
     << " SAME: " << flightAppRule.fltSame() << " EQP APPL: " << flightAppRule.equipAppl()
     << " EQP TYPE: " << flightAppRule.equipType() << endl;

  dc.printLine();
}
}
