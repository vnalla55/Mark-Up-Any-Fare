//----------------------------------------------------------------------------
//  File:        Diag502Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 502
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

#include "Diagnostic/Diag502Collector.h"

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>

using namespace std;

namespace tse
{
const string Diag502Collector::SHORT_OUTPUT = "PH";
const string Diag502Collector::PREVALIDATION = "PREV";
const string Diag502Collector::NORMAL_OR_FARECOMP_VALIDATION = "FCV";
const string Diag502Collector::REVALIDATION = "REV";
const string Diag502Collector::DYNAMIC_VALIDATION = "DV";
const string Diag502Collector::SPECIFIC_CATEGORY = "RL";

void
Diag502Collector::printRulePhase(const CategoryPhase phase)
{
  if (_active)
  {
    ((DiagCollector&)*this) << "RULE PHASE: ";
    if (phase == NormalValidation)
      ((DiagCollector&)*this) << "FARE COMPONENT VALIDATION\n";
    else if (phase == PreValidation)
      ((DiagCollector&)*this) << "PRE-VALIDATION\n";
    else if (phase == PreCombinabilityValidation)
      ((DiagCollector&)*this) << "PRE-COMBINABILITY VALIDATION\n";
    else
        if (phase == PURuleValidation)
      ((DiagCollector&)*this) << "PU FACTORY VALIDATION\n";
    else if (phase == FPRuleValidation)
      ((DiagCollector&)*this) << "FAREPATH FACTORY VALIDATION\n";
    else if (phase == DynamicValidation)
      ((DiagCollector&)*this) << "DYNAMIC-VALIDATION\n";
    else if (phase == ShoppingComponentValidation)
      ((DiagCollector&)*this) << "SHOPPING-COMPONENT-VALIDATION\n";
    else if (phase == ShoppingComponentWithFlightsValidation)
      ((DiagCollector&)*this) << "SHOPPING-COMPONENT-WITH-FLIGHTS-VALIDATION\n";
  }
}

void
Diag502Collector::diag502Collector(const PaxTypeFare& paxFare, const GeneralFareRuleInfo& rule)
{
  if (!_active)
  {
    return;
  }

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "---------------------------------------------------------------\n";
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
      if (paxFare.paxType()) // to fix core dump
      {
        dc << setw(3) << paxFare.paxType()->paxType();
      }
      else
      {
        dc << setw(3) << "   ";
      }
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

  if (paxFare.directionality() == FROM)
    dc << setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << setw(2) << "I";
  else
    dc << setw(2) << "-";

  dc << "R2:";
  dc << setw(5) << rule.vendorCode();
  dc << setw(3) << rule.carrierCode();
  dc << setw(5) << rule.tariffNumber();
  dc << setw(5) << rule.ruleNumber();

  dc << setw(8) << rule.sequenceNumber();
  dc << setw(3) << rule.segcount();

  dc << setw(2) << rule.categoryNumber();
  dc << '\n';

  if (paxFare.isConstructed())
  {
    dc << "CONSTRUCTED FN1:" << paxFare.footNote1() << " FN2:" << paxFare.footNote2()
       << " ADDON ORIG:" << paxFare.origAddonTariff() << " FN1:" << paxFare.origAddonFootNote1()
       << " FN2:" << paxFare.origAddonFootNote2() << "\n";
    dc << " ADDON DEST:" << paxFare.destAddonTariff() << " FN1:" << paxFare.destAddonFootNote1()
       << " FN2:" << paxFare.destAddonFootNote2() << "\n";
  }

  dc << "             GEN FARE RULE        PAX TYPE FARE\n";
  dc << "INHIBIT:      " << setw(20) << rule.inhibit() << paxFare.inhibit() << '\n';
  dc << "OW/RT:        " << setw(20) << rule.owrt() << paxFare.owrt() << '\n';
  dc << "ROUTING   APPL:" << rule.routingAppl() << " :" << setw(16) << rule.routing()
     << paxFare.routingNumber() << '\n';
  dc << "FARE TYPE:    " << setw(20) << rule.fareType() << paxFare.fcaFareType() << '\n';
  dc << "SEASON TYPE:  " << setw(20) << rule.seasonType() << paxFare.fcaSeasonType() << '\n';
  dc << "DOW:          " << setw(20) << rule.dowType() << paxFare.fcaDowType() << '\n';

  dc << "FOOT NOTE:   " << setw(8) << rule.footNote1() << setw(13) << rule.footNote2() << setw(8)
     << paxFare.footNote1() << paxFare.footNote2() << '\n';

  dc << "JOINT CXR: ";
  if (rule.jointCarrierTblItemNo())
  {
    dc << "TBLNO " << setw(17) << rule.jointCarrierTblItemNo();
    const std::vector<TravelSeg*>& tvlSegs = paxFare.fareMarket()->travelSeg();

    std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

    for (; tvlSegI != tvlSegEndI; tvlSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegI);
      if (airSeg != 0)
      {
        dc << airSeg->carrier() << " ";
      }
    }
  }
  dc << '\n';

  dc << "LOC           " << rule.loc1().locType() << " " << setw(5) << rule.loc1().loc()
     << rule.loc2().locType() << " " << setw(11) << rule.loc2().loc();
  if (paxFare.isReversed())
  {
    dc << setw(3) << paxFare.fare()->market2();
    dc << setw(4) << paxFare.fare()->market1() << "\n";
  }
  else
  {
    dc << setw(3) << paxFare.fare()->market1();
    dc << setw(4) << paxFare.fare()->market2() << "\n";
  }

  dc << "FARE CLASS:   " << setw(20) << rule.fareClass() << paxFare.fare()->fareClass() << "\n";
}

void
Diag502Collector::diag502Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo& rule)
{
  if (!_active)
  {
    return;
  }

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "----------------------------------------------------------------\n";
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
      if (paxFare.paxType()) // to fix core dump
      {
        dc << setw(3) << paxFare.paxType()->paxType();
      }
      else
      {
        dc << setw(3) << "   ";
      }
      dc << "*";
    }
    else
    {
      dc << "-XX:";
    }
  }

  if (paxFare.isReversed())
  {
    dc << setw(3) << paxFare.fare()->market2();
    dc << setw(4) << paxFare.fare()->market1();
  }
  else
  {
    dc << setw(3) << paxFare.fare()->market1();
    dc << setw(4) << paxFare.fare()->market2();
  }
  dc << setw(9) << paxFare.fare()->fareClass();

  if (paxFare.directionality() == FROM)
    dc << setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << setw(2) << "I";
  else
    dc << setw(2) << "-";

  dc << "R2:";
  dc << setw(5) << paxFare.vendor();
  dc << setw(3) << paxFare.fare()->carrier();
  dc << setw(5) << rule.fareTariff();
  dc << setw(5) << rule.footNote();
  dc << setw(8) << rule.sequenceNumber();
  dc << setw(3) << rule.segcount();
  dc << " F";
  dc << setw(2) << rule.categoryNumber();
  dc << '\n';

  dc << "TARIFF NUMBER: " << setw(6) << rule.tariffNumber();
  dc << "RULE NUMBER: " << setw(6) << rule.ruleNumber();
  dc << '\n';

  dc << "             FOOTNOTE RULE        PAX TYPE FARE\n";
  dc << "INHIBIT:      " << setw(20) << rule.inhibit() << paxFare.inhibit() << '\n';
  dc << "OW/RT:        " << setw(20) << rule.owrt() << paxFare.owrt() << '\n';
  dc << "ROUTING   APPL:" << rule.routingAppl() << " :" << setw(16) << rule.routing()
     << paxFare.routingNumber() << '\n';
  dc << "JOINT CXR: ";
  if (rule.jointCarrierTblItemNo())
  {
    dc << "TBLNO " << setw(17) << rule.jointCarrierTblItemNo();
    const std::vector<TravelSeg*>& tvlSegs = paxFare.fareMarket()->travelSeg();

    std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

    for (; tvlSegI != tvlSegEndI; tvlSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegI);
      if (airSeg != 0)
      {
        dc << airSeg->carrier() << " ";
      }
    }
  }
  dc << '\n';

  dc << "LOC           " << rule.loc1().locType() << " " << setw(5) << rule.loc1().loc()
     << rule.loc2().locType() << " " << setw(11) << rule.loc2().loc() << setw(3)
     << paxFare.fareMarket()->origin()->loc() << setw(4)
     << paxFare.fareMarket()->destination()->loc() << "\n";

  dc << "FARE CLASS:   " << setw(20) << rule.fareClass() << paxFare.fare()->fareClass() << "\n";
}

void
Diag502Collector::diag502Collector(const PaxTypeFare& paxFare, const CombinabilityRuleInfo& rule)
{
  if (!_active)
  {
    return;
  }

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "---------------------------------------------------------------\n";
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
      if (paxFare.paxType()) // to fix core dump
      {
        dc << setw(3) << paxFare.paxType()->paxType();
      }
      else
      {
        dc << setw(3) << "   ";
      }
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

  if (paxFare.directionality() == FROM)
    dc << setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << setw(2) << "I";
  else
    dc << setw(2) << "-";

  dc << "R2:";
  dc << setw(5) << rule.vendorCode();
  dc << setw(3) << rule.carrierCode();
  dc << setw(5) << rule.tariffNumber();
  dc << setw(5) << rule.ruleNumber();

  dc << setw(8) << rule.sequenceNumber();
  dc << setw(3) << rule.segCnt();

  dc << setw(2) << rule.categoryNumber();
  dc << '\n';

  if (paxFare.isConstructed())
  {
    dc << "CONSTRUCTED FN1:" << paxFare.footNote1() << " FN2:" << paxFare.footNote2()
       << " ADDON ORIG:" << paxFare.origAddonTariff() << " FN1:" << paxFare.origAddonFootNote1()
       << " FN2:" << paxFare.origAddonFootNote2() << "\n";
    dc << " ADDON DEST:" << paxFare.destAddonTariff() << " FN1:" << paxFare.destAddonFootNote1()
       << " FN2:" << paxFare.destAddonFootNote2() << "\n";
  }

  dc << "        COMBINABILIRY FARE RULE        PAX TYPE FARE\n";
  dc << "INHIBIT:      " << setw(20) << rule.inhibit() << paxFare.inhibit() << '\n';
  dc << "OW/RT:        " << setw(20) << rule.owrt() << paxFare.owrt() << '\n';
  dc << "ROUTING   APPL:" << rule.routingAppl() << " :" << setw(16) << rule.routing()
     << paxFare.routingNumber() << '\n';
  dc << "FARE TYPE:    " << setw(20) << rule.fareType() << paxFare.fcaFareType() << '\n';
  dc << "SEASON TYPE:  " << setw(20) << rule.seasonType() << paxFare.fcaSeasonType() << '\n';
  dc << "DOW:          " << setw(20) << rule.dowType() << paxFare.fcaDowType() << '\n';

  dc << "FOOT NOTE:   " << setw(8) << rule.footNote1() << setw(13) << rule.footNote2() << setw(8)
     << paxFare.footNote1() << paxFare.footNote2() << '\n';

  dc << "JOINT CXR: ";
  if (rule.jointCarrierTblItemNo())
  {
    dc << "TBLNO " << setw(17) << rule.jointCarrierTblItemNo();
    const std::vector<TravelSeg*>& tvlSegs = paxFare.fareMarket()->travelSeg();

    std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
    std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

    for (; tvlSegI != tvlSegEndI; tvlSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegI);
      if (airSeg != 0)
      {
        dc << airSeg->carrier() << " ";
      }
    }
  }
  dc << '\n';

  dc << "LOC           " << rule.loc1().locType() << " " << setw(5) << rule.loc1().loc()
     << rule.loc2().locType() << " " << setw(11) << rule.loc2().loc() << setw(3)
     << paxFare.fareMarket()->origin()->loc() << setw(4)
     << paxFare.fareMarket()->destination()->loc() << "\n";

  dc << "FARE CLASS:   " << setw(20) << rule.fareClass() << paxFare.fare()->fareClass() << "\n";
}

bool
Diag502Collector::isDiagNeeded(PricingTrx& trx,
                               const PaxTypeFare& paxFare,
                               const std::vector<CategoryRuleItemInfoSet*>& ruleSet)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic502))
    return false;

  if (!trx.diagnostic().shouldDisplay(paxFare))
    return false;

  // if there is filter on display for a category only
  map<string, string>::iterator end = trx.diagnostic().diagParamMap().end();
  map<string, string>::iterator begin = trx.diagnostic().diagParamMap().find(SPECIFIC_CATEGORY);

  if (begin == end)
  {
    // no category specified, display all
    return true;
  }

  uint16_t categoryNumber = 0;
  string catNum = (*begin).second;
  if (!catNum.empty())
  {
    categoryNumber = atoi(catNum.c_str());
  }

  if (categoryNumber == 0)
  {
    // invalid filter
    return true;
  }

  for (const auto& criSetPtr: ruleSet)
  {
    for (const auto& info: *criSetPtr)
    {
      if (info.itemcat() == categoryNumber)
      {
        return true;
      }
    }
  }
  return false;
}
}
