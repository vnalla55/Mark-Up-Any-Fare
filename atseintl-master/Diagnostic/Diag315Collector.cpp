//----------------------------------------------------------------------------
//  File:        Diag315Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 315 (Sales Restriction Rule)
//
//  Updates:
//          06/15/2004  VK - create.
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

#include "Diagnostic/Diag315Collector.h"

#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleConst.h"

#include <iomanip>

using namespace std;

namespace tse
{
void
Diag315Collector::writeHeader(const PaxTypeFare& paxTypeFare)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "************************************************************\n";
    dc << "*            ATSE CATEGORY 15 SALES RESTRICTIONS           *\n";
    dc << "************************************************************\n";

    dc << setw(3) << paxTypeFare.fareMarket()->origin()->loc();
    dc << "-";
    dc << setw(3) << paxTypeFare.fare()->carrier();
    dc << "-";
    dc << setw(5) << paxTypeFare.fareMarket()->destination()->loc();
    dc << setw(10) << paxTypeFare.fare()->fareClass();

    dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";
    dc << "     ";
    dc << setw(5) << paxTypeFare.vendor();
    dc << setw(5) << paxTypeFare.fare()->tcrRuleTariff();
    dc << setw(5) << paxTypeFare.fare()->ruleNumber();
    dc << '\n';

    if (paxTypeFare.selectedFareNetRemit())
    {
      dc << " SELECTED TICKETING CAT35 NET REMIT FARE \n";
    }
    else if (!paxTypeFare.isFareByRule() && paxTypeFare.cat25Fare())
    {
      dc << " BASE FARE OF CAT25 FARE - ";
      dc << paxTypeFare.cat25Fare()->fareClass();
      dc << '\n';
    }
    dc << "  " << '\n';
  }
}


void
Diag315Collector::displayLocales(const SalesRestriction* srr)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);

    if (srr->locales().size() != 0) // Any Locale item?
    {
      dc << " LOCALE RESTRICTIONS: " << '\n';

      std::vector<Locale*>::const_iterator iterB = srr->locales().begin();
      std::vector<Locale*>::const_iterator iterE = srr->locales().end();

      while (iterB != iterE)
      {
        string appl;
        string type;

        // Application
        switch ((*iterB)->locAppl())
        {
        case 'Y': // TKTS_MAY_ONLY_BE_SOLD
        {
          appl = " MAY ONLY BE SOLD BY        ";
          break;
        }
        case 'N': // TKTS_MAY_NOT_BE_SOLD
        {
          appl = " MAY NOT BE SOLD BY         ";
          break;
        }
        case 'T': // TKTS_MAY_ONLY_BE_ISSUED
        {
          appl = " TKTS MAY ONLY BE ISSUED BY ";
          break;
        }
        case 'X': // TKTS_MAY_NOT_BE_ISSUED
        {
          appl = " TKTS MAY NOT BE ISSUED BY  ";
          break;
        }
        default:
        {
          appl = " UNKNOWN               ";
          break;
        }
        }

        // Location type
        switch ((*iterB)->loc1().locType())
        {
        case 'A': // Area
        {
          type = "AREA";
          break;
        }
        case 'C': // City
        {
          type = "CITY";
          break;
        }
        case 'H': // Home IATA Travel Agency Number
        {
          type = "HOME IATA NO.";
          break;
        }
        case 'I': // IATA Travel Agency Number
        {
          type = "IATA TVL NO.";
          break;
        }
        case 'N': // Country
        {
          type = "COUNTRY";
          break;
        }
        case 'P': // Airport
        {
          type = "AIRPORT";
          break;
        }
        case 'S': // Country/State
        {
          type = "COUNTRY/STATE";
          break;
        }
        case 'T': // Travel Agency
        {
          type = "TRAVEL AGENCY";
          break;
        }
        case 'U': // Home Travel Agency
        {
          type = "HOME TVL AGENCY";
          break;
        }
        case 'V': // Function Code
        {
          type = "FUNCTION CODE";
          break;
        }
        case 'X': // Duty Code
        {
          type = "DUTY CODE";
          break;
        }
        case 'Z': // Zone
        {
          type = "ZONE";
          break;
        }
        default:
        {
          type = "UNKNOWN";
          break;
        }
        }
        dc << setw(30) << appl;
        dc << setw(16) << type;

        dc << setw(8) << (*iterB)->loc1().loc();
        dc << setw(7) << (*iterB)->loc2().loc() << '\n';
        ++iterB;
      }
    }
    else
    {
      dc << " NO LOCALE RESTRICTIONS " << '\n';
    }
  }
}
}
