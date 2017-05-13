//----------------------------------------------------------------------------
//  File:        Diag550Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 550
//
//  Updates:
//          04/23/2004  VK - create.
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

#include "Diagnostic/Diag550Collector.h"

#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>

using namespace std;

namespace tse
{
void
Diag550Collector::printHeader()
{
  if (_active)
  {

    ((DiagCollector&)*this) << "---------------------------------------------------------\n"
                            << "PAX MARKET   FARE  VENDOR  CXR TRF  RULE SEQ     SEG TYPE\n"
                            << "TYPE                           NBR  NBR  NBR     CNT CAT \n"
                            << "---------------------------------------------------------\n";
  }
}

void
Diag550Collector::diag550Collector(const PaxTypeFare& paxFare,
                                   const GeneralFareRuleInfo* rule,
                                   const bool fareRule)
{
  if (_active)
  {

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isValid())
    {
      dc << setw(3) << paxFare.paxType()->paxType();
      dc << ":";
    }
    else
    {
      if (paxFare.isFareClassAppMissing())
      {
        dc << "-R1:";
      }
      else if (paxFare.isFareClassAppSegMissing())
      {
        dc << "-1B:";
      }
      else if (paxFare.isRoutingProcessed() && !(paxFare.isRoutingValid()))
      {
        dc << "-RT:";
      }
      else if (paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        dc << "-BK:";
      }
      else if (!paxFare.areAllCategoryValid())
      {
        dc << setw(3) << paxFare.paxType()->paxType();
        dc << "*";
      }
      else
      {
        dc << "-XX:";
      }
    }

    dc << setw(3) << paxFare.fareMarket()->origin()->loc();
    dc << setw(4) << paxFare.fareMarket()->destination()->loc();
    dc << setw(9) << paxFare.fare()->fareClass();
    dc << "R2:";
    dc << setw(5) << paxFare.vendor();
    dc << setw(3) << paxFare.fare()->carrier();
    dc << setw(5) << paxFare.fare()->tcrRuleTariff();
    dc << setw(5) << paxFare.fare()->ruleNumber();

    dc << setw(8) << rule->sequenceNumber();
    dc << setw(3) << rule->segcount();
    if (fareRule)
    {
      dc << " R";
    }
    else
      dc << " G";
    dc << setw(2) << rule->categoryNumber();
    dc << '\n';

    RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(trx());
    if ((rexTrx != nullptr) && (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE))
    {
      displayRetrievalDate(paxFare);
      dc << "\n";
    }
  }
}

void
Diag550Collector::diag550Collector(Record3ReturnTypes statusRule)
{
  if (_active)
  {

    DiagCollector& dc(*this);

    if (statusRule == PASS)
    {
      dc << "                              ---PASS--- ";
    }
    else if (statusRule == SKIP)
    {
      dc << "                              ---SKIP--- ";
    }
    else if (statusRule == SOFTPASS)
    {
      dc << "                              ---SOFT--- ";
    }
    else
    {
      dc << "                              ---FAIL--- ";
    }
    dc << "\n---------------------------------------------------------\n";
  }
}

void
Diag550Collector::diag550Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo* rule)
{
  if (_active)
  {

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isValid())
    {
      dc << setw(3) << paxFare.paxType()->paxType();
      dc << ":";
    }
    else
    {
      if (paxFare.isFareClassAppMissing())
      {
        dc << "-R1:";
      }
      else if (paxFare.isFareClassAppSegMissing())
      {
        dc << "-1B:";
      }
      else if (paxFare.isRoutingProcessed() && !(paxFare.isRoutingValid()))
      {
        dc << "-RT:";
      }
      else if (paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        dc << "-BK:";
      }
      else if (!paxFare.areAllCategoryValid())
      {
        if (paxFare.paxType())
          dc << setw(3) << paxFare.paxType()->paxType();
        else
          dc << "   ";

        dc << "*";
      }
      else
      {
        dc << "-XX:";
      }
    }

    dc << setw(3) << paxFare.fareMarket()->origin()->loc();
    dc << setw(4) << paxFare.fareMarket()->destination()->loc();
    dc << setw(9) << paxFare.fare()->fareClass();
    dc << "R2:";
    dc << setw(5) << paxFare.vendor();
    dc << setw(3) << paxFare.fare()->carrier();
    dc << setw(5) << paxFare.fare()->tcrRuleTariff();
    dc << setw(5) << rule->footNote();
    dc << setw(8) << rule->sequenceNumber();
    dc << setw(3) << rule->segcount();
    dc << " F";
    dc << setw(2) << rule->categoryNumber();
    dc << '\n';
  }
}

void
Diag550Collector::displayRelation(const PaxTypeFare& paxFare,
                                  const CategoryRuleItemInfo* rule,
                                  Record3ReturnTypes statusRule)
{
  if (_active)
  {
    string status;
    if (statusRule == PASS)
    {
      status = "PASS";
    }
    else if (statusRule == SKIP)
    {
      status = "SKIP";
    }
    else if (statusRule == SOFTPASS)
    {
      status = "SOFT";
    }
    else
    {
      status = "FAIL";
    }

    string relation;

    switch (rule->relationalInd())
    {
    case CategoryRuleItemInfo::IF:
    {
      relation = "IF";
      break;
    }
    case CategoryRuleItemInfo::THEN:
    {
      relation = "THEN";
      break;
    }
    case CategoryRuleItemInfo::OR:
    {
      relation = "OR";
      break;
    }
    case CategoryRuleItemInfo::AND:
    {
      relation = "AND";
      break;
    }
    case CategoryRuleItemInfo::ELSE:
    {
      relation = "ELSE";
      break;
    }
    default:
    {
      relation = "UNKN";
      break;
    }
    }

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "            ";
    dc << std::setw(8) << relation;
    dc << setw(2) << rule->itemcat();
    dc << " - ";
    dc << setw(8) << rule->itemNo();
    dc << std::setw(6) << status;
    dc << '\n';
  }
}
}
