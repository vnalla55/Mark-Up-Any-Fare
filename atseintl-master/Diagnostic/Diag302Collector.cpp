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

#include "Diagnostic/Diag302Collector.h"

#include "Common/Money.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/DayTimeApplication.h"

#include <iomanip>
#include <ios>

using namespace std;

namespace tse
{
void
Diag302Collector::diag302Collector(const PaxTypeFare& paxTypeFare,
                                   const PricingUnit& pu,
                                   const Record3ReturnTypes status,
                                   const string& phase,
                                   const CategoryRuleInfo& cri,
                                   const DayTimeAppInfo& ruleInfo)
{
  static const char* weekDays[] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };

  if (!_active)
    return;

  DiagCollector& dc = *this; // Just a shorthand

  //------------------------------------------------------------------------
  // Format Fare Component Heading of the Cat 2 diagnostic.
  //------------------------------------------------------------------------
  dc << "***************************************************************" << endl;
  dc << "             ATSE CATEGORY 02 DAY/TIME APPLICATION             " << endl;

  dc << "***************************************************************" << endl;

  //------------------------------------------------------------------------
  // BOS-DFW  2001-06-14  ADT  AA   QLE14TN 12345.12  PRICING UNIT
  //------------------------------------------------------------------------
  const TravelSeg* primarySeg;
  vector<TravelSeg*> partOfTravel;
  dc << setw(3);
  if (phase == "PRICING UNIT")
  {
    dc << paxTypeFare.fareMarket()->origin()->loc() << "-" << setw(3)
       << paxTypeFare.fareMarket()->destination()->loc() << " ";
    dc << " PU ";
    dc << (*(pu.travelSeg().begin()))->origin()->loc() << "-"
       << (*(pu.travelSeg().end() - 1))->destination()->loc() << " ";
  }
  else
  {
    dc << paxTypeFare.fareMarket()->origin()->loc() << "-" << setw(3)
       << paxTypeFare.fareMarket()->destination()->loc() << " ";
    primarySeg = paxTypeFare.fareMarket()->primarySector();
    // to fix core dump now, may update when knowing how to get primarySeg
    if (primarySeg == 0)
    {
      primarySeg = *(paxTypeFare.fareMarket()->travelSeg().begin());
    }
    dc << primarySeg->departureDT().toSimpleString() << " ";
  }

  dc << setw(3) << paxTypeFare.fcasPaxType() << " " << setfill(' ') << setw(3)
     << paxTypeFare.carrier() << " " << setfill(' ') << setw(7) << paxTypeFare.fareClass() << " "
     << setfill(' ') << setw(10) //<< setprecision(paxTypeFare.numDecimal())
     << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " " << phase << endl << endl;

  dc << "R3 ITEM NUMBER: " << setw(7) << setfill(' ') << std::left << ruleInfo.itemNo()
     << std::right << setw(4) << paxTypeFare.vendor() << setw(4) << paxTypeFare.fareTariff()
     << setw(4) << paxTypeFare.carrier() << " " << paxTypeFare.ruleNumber() << endl;

  //------------------------------------------------------------------------
  //   CATEGORY 2 RULE DATA
  //------------------------------------------------------------------------
  dc << "CATEGORY 2 RULE DATA:" << endl << endl;

  //--------------------------
  // Unavailable Tag
  //--------------------------
  if (ruleInfo.unavailtag() != ' ')
  {
    if (ruleInfo.unavailtag() == DayTimeApplication::dataUnavailable)
      dc << " UNAVAILABLE DATA";
    else
      dc << " TEXT DATA ONLY";

    dc << endl;

    //--------------------------------------------------------------------
    //    End of Display
    //--------------------------------------------------------------------
    dc << "***************************************************************" << endl;
    return;
  }

  //--------------------------
  // Override Date Table 994
  //--------------------------
  dc << " OVERRIDE DATE TABLE 994           : " << setfill(' ') << setw(6)
     << ruleInfo.overrideDateTblItemNo() << endl;

  //--------------------------------------
  // Geographic Specification Table 995
  //--------------------------------------
  dc << " GEOGRAPHIC SPECIFICATION TABLE 995: " << setfill(' ') << setw(6)
     << ruleInfo.geoTblItemNo() << endl << endl;

  //----------------------------
  // Start/Stop Times and Days
  //----------------------------
  bool anyRestriction = false;
  string firstDOW;
  string lastDOW;

  // Days of Week
  if (!ruleInfo.dow().empty())
  {
    if (ruleInfo.dayTimeNeg() == ' ')
      dc << " DAYS TRAVEL IS PERMITTED: ";
    else
      dc << " DAYS TRAVEL IS NOT PERMITTED: ";

    int32_t dowSize = (int32_t)ruleInfo.dow().size();

    for (int32_t i = 0; i < dowSize; ++i)
    {
      // Convert record 3 cat 2 dow
      int32_t ruleDOW = ruleInfo.dow()[i] - '0';

      if (ruleDOW < 1 || ruleDOW > 7) // End of valid days of week
        break;

      if (i == 0)
      {
        dc << setw(4) << weekDays[ruleDOW - 1];
        firstDOW = weekDays[ruleDOW - 1];
      }
      else
      {
        dc << setw(4) << weekDays[ruleDOW - 1];
        lastDOW = weekDays[ruleDOW - 1];
      }
    }
    dc << endl;

    /// @todo if (startTime.isNotValid() && stopTime.isNotValid())
    //         {
    //             dc << " *APPLY AT ANY TIME OF DAY" << endl;
    //         }

    anyRestriction = true;
  }

  // Start/Stop Times
  // if (startTime.isValid() || stopTime.isValid())
  {
    if (ruleInfo.dayTimeNeg() == ' ')
      dc << " TIME TRAVEL IS PERMITTED: ";
    else
      dc << " TIME TRAVEL IS NOT PERMITTED: ";

    if (ruleInfo.todAppl() != DayTimeApplication::range)
    {
      dc << "FROM " << setw(2) << setfill('0') << ruleInfo.startTime() / 60 << ':' << setw(2)
         << setfill('0') << ruleInfo.startTime() % 60 << " THRU " << setw(2) << setfill('0')
         << ruleInfo.stopTime() / 60 << ':' << setw(2) << setfill('0') << ruleInfo.stopTime() % 60
         << endl;
    }
    else
    {
      dc << "FROM " << setw(2) << setfill('0') << ruleInfo.startTime() / 60 << ':' << setw(2)
         << setfill('0') << ruleInfo.startTime() % 60 << " " << setw(3) << firstDOW << " THRU "
         << setw(2) << setfill('0') << ruleInfo.stopTime() / 60 << ':' << setw(2) << setfill('0')
         << ruleInfo.stopTime() % 60 << " " << setw(3) << lastDOW << endl;
    }

    if (ruleInfo.dow().empty())
      dc << " *APPLY TO EVERY DAY OF WEEK" << endl;

    anyRestriction = true;
  }

  if (anyRestriction)
  {
    if (ruleInfo.dayTimeAppl() == ' ')
      dc << " *RESTRICTIONS APPLY TO DEPARTURE FROM ORIGIN OF FARE COMPONENT" << endl;
    else
      dc << " *RESTRICTIONS APPLY TO DEPARTURE FROM ORIGIN OF PRICING UNIT" << endl;

    dc << "  UNLESS FURTHER MODIFIED BY A GEOGRAPHIC SPECIFICATION TABLE" << endl;
  }

  //---------------------
  // Day Of Week - Same
  //---------------------

  if (ruleInfo.dowSame() != ' ')
  {
    dc << "\n ALL FARE COMPONENTS WITHIN THE PRICING UNIT MUST DEPART ON" << endl
       << "  THE SAME DAY OF WEEK AS THE FARE COMPONENT BEING PROCESSED" << endl;
  }

  //--------------------------
  // Day Of Week - Occurence
  //--------------------------

  if (ruleInfo.dowOccur() != 0)
  {
    dc << "\n THE ORIGINATING FARE COMPONENT OF THE PRICING UNIT MUST MEET" << endl;

    dc << "  THE DAYS RESTRICTION, AND ALL SUBSEQUENT FARE COMPONENTS" << endl;

    dc << "  IN THE PRICING UNIT MUST DEPART ON THE " << setw(4) << getOrdinal(ruleInfo.dowOccur())
       << " OCCURRENCE OF" << endl;

    dc << "  THE DAYS INDICATED" << endl;
  }

  //------------------------------------------------------------------------
  //    End of Display
  //------------------------------------------------------------------------
  //    dc << "***************************************************************"
  //       << endl;
} // End of Diagnostics Function for Cat 02

/** Return string containing ordinal number.
 * 1 -> 1ST
 * 2 -> 2ND
 * 3 -> 3RD
 * x -> xTH
 */
string
Diag302Collector::getOrdinal(int input) const
{
  stringstream s;
  s << input;

  if (input % 10 == 1)
    s << "ST";
  else if (input % 10 == 2)
    s << "ND";
  else if (input % 10 == 3)
    s << "RD";
  else
    s << "TH";

  return s.str();
}
}
