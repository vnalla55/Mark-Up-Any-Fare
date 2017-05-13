//----------------------------------------------------------------------------
//          File:           Diag988Collector.cpp
//          Description:    Diag988Collector
//          Created:        11/10/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "Diagnostic/Diag988Collector.h"

#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Pricing/FPPQItem.h"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <sstream>

namespace tse
{

void
Diag988Collector::outputHeader()
{
  *this << "DIAGNOSTIC 988 VALIDATE INTERLINE TICKETING AGREEMENT\n";
}

void
Diag988Collector::outputVITAData(const PricingTrx& trx,
                                 const Itin& itin,
                                 bool intTicketValidationResult,
                                 const std::string& validationMessage)
{
  *this << "\n FLIGHTS: \n";

  for (const auto travelSeg : itin.travelSeg())
  {
    *this << " ";
    *this << std::setw(3) << travelSeg->origin()->loc();
    *this << "-" << std::setw(3) << travelSeg->destination()->loc();
    *this << "  ";

    AirSeg* airSegPtr = travelSeg->toAirSeg();
    if (!airSegPtr)
    {
      *this << "ARUNK\n";
      continue;
    }
    else
    {
      const AirSeg& aSeg = *airSegPtr;
      *this << aSeg.marketingCarrierCode() << aSeg.flightNumber();

      const int FlightNumberWidth = 6;
      int numberWidth = boost::lexical_cast<std::string>(aSeg.flightNumber()).size();
      for (; numberWidth < FlightNumberWidth; ++numberWidth)
      {
        *this << " ";
      }
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();
      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
      *this << std::setw(6) << depDTStr;
      *this << " ";
      *this << std::setw(6) << arrDTStr;

      if (!aSeg.operatingCarrierCode().empty() &&
          aSeg.operatingCarrierCode() != aSeg.marketingCarrierCode())
      {
        *this << " OPERATED BY " << aSeg.operatingCarrierCode();
      }

      *this << "\n";
    }
  }

  *this << "  VALIDATING CARRIER: " << itin.validatingCarrier() << "\n";
  *this << "  INTERLINETICKETINGAGREEMENT " << (intTicketValidationResult ? "PASSED" : "FAILED");
  if (!intTicketValidationResult && !validationMessage.empty())
  {
    *this << " " << validationMessage;
  }

  *this << "\n";
}

} // namespace tse
