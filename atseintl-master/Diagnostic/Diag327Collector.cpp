//-----------------------------------------------------------------------------
//
//  File:     Diag327Collector.cpp
//
//  Author :  Konrad Koch
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#include "Diagnostic/Diag327Collector.h"

#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Tours.h"

namespace tse
{
void
Diag327Collector::collect(RuleItem::ProcessingPhase phase,
                          const PaxTypeFare& paxTypeFare,
                          const CategoryRuleInfo& ruleInfo,
                          const Tours& tours)
{
  if (_active)
  {
    displayHeader(phase, paxTypeFare, ruleInfo);
    displayRecord2Info(ruleInfo);
    displayRecord2SegmentsInfo(ruleInfo);
    displayRecord3Info(tours);
  }
}

void
Diag327Collector::displayHeader(RuleItem::ProcessingPhase phase,
                                const PaxTypeFare& paxTypeFare,
                                const CategoryRuleInfo& catRuleInfo)
{
  Diag327Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc.fill(' ');

  dc << "***************************************************************\n";
  dc << "*               ATSE CATEGORY 27 TOURS DIAGNOSTICS            *\n";
  dc << "***************************************************************\n";

  dc << "PHASE: " << RuleItem::getRulePhaseString(phase) << "\n";

  dc << std::setw(3) << paxTypeFare.fareMarket()->origin()->loc();
  dc << " ";
  dc << std::setw(3) << paxTypeFare.fareMarket()->destination()->loc();
  dc << " ";
  dc << std::setw(10) << paxTypeFare.fare()->fareClass();
  dc << "R2: FARERULE : ";
  dc << std::setw(5) << catRuleInfo.vendorCode();
  dc << std::setw(5) << catRuleInfo.tariffNumber();
  dc << std::setw(3) << catRuleInfo.carrierCode();
  dc << std::setw(5) << catRuleInfo.ruleNumber() << "\n";
}

void
Diag327Collector::displayRecord2Info(const CategoryRuleInfo& catRuleInfo)
{
  Diag327Collector& dc = *this;

  dc << "R2: SEQ NBR ";
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.fill('0');
  dc << std::setw(9) << catRuleInfo.sequenceNumber() << "\n";
}

void
Diag327Collector::displayRecord2SegmentsInfo(const CategoryRuleInfo& catRuleItemInfo)
{
  for (const auto& infoSet: catRuleItemInfo.categoryRuleItemInfoSet())
  {
    for (const auto& itemInfo: *infoSet)
    {
      displayRecord2SegmentInfo(itemInfo);
    }
  }
}

void
Diag327Collector::displayRecord2SegmentInfo(const CategoryRuleItemInfo& catRuleItemInfo)
{
  Diag327Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc.fill(' ');
  dc << "R2S: ";
  dc << std::setw(4) << getRelationString(catRuleItemInfo.relationalInd());
  dc << " ";
  dc << std::setw(9) << catRuleItemInfo.itemNo();
  dc << "    ";
  dc << "DIR-" << std::setw(1) << catRuleItemInfo.directionality();
  dc << "  ";
  dc << "I/O-" << std::setw(1) << catRuleItemInfo.inOutInd() << "\n";
}

void
Diag327Collector::displayRecord3Info(const Tours& tours)
{
  Diag327Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc.fill(' ');
  dc << "R3 : ";
  dc << std::setw(9) << tours.itemNo();
  dc << "         ";
  dc << "T994-";
  dc << std::setw(9) << tours.overrideDateTblItemNo();
  dc << "   ";
  dc << "TOUR CODE:";
  dc << std::setw(8) << tours.tourNo() << "\n";
}

void
Diag327Collector::displayStatus(Record3ReturnTypes result)
{
  if (_active)
  {
    Diag327Collector& dc = *this;

    dc << "STATUS: " << (result == PASS ? "MATCH" : "NOT MATCH") << "\n";
  }
}
}
