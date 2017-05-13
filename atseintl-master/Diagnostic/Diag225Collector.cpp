//----------------------------------------------------------------------------
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

#include "Diagnostic/Diag225Collector.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>
#include <iostream>

namespace tse
{
const char*
Diag225Collector::R2_PASS("");
const char*
Diag225Collector::R2_FAIL_GEO("GEO");
const char*
Diag225Collector::R2_FAIL_EFF_DISC_DATE("EFF/DISC DATE");

void
Diag225Collector::printHeader()
{
  if (_active)
  {
    *this << "***********  FARE BY RULE CATEGORY CONTROL RECORDS  ***********" << std::endl;
  }
}

void
Diag225Collector::writeSeparator(SeparatorType st)
{
  if (_active)
  {
    if (st == RULE_HEADER)
    {
      ((DiagCollector&)*this) << "VENDOR CXR TRF  RULE  SEQ     SEG    FAIL  " << std::endl
                              << "           NBR  NBR   NBR     CNT    REASON" << std::endl;
    }
  }
}

Diag225Collector&
Diag225Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << std::endl;
    dc << "***************************************************************";

    dc << std::endl;
    dc << fareMarket.boardMultiCity();
    dc << "-";
    dc << fareMarket.governingCarrier();
    dc << "-";
    dc << fareMarket.offMultiCity();
    dc << std::endl;

    writeSeparator(RULE_HEADER);
  }

  return *this;
}

Diag225Collector&
Diag225Collector::operator<<(const PaxType& paxType)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "REQUESTED PAXTYPE : ";
    dc << paxType.paxType();
    dc << std::endl;
  }

  return *this;
}

void
Diag225Collector::diag225Collector(const FareByRuleCtrlInfo* rule,
                                   const char* failCode,
                                   FareMarket& fareMarket)
{
  if (_active)
  {

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "---------------------------------------------------------------";
    dc << std::endl;

    dc << std::setw(7) << rule->vendorCode();
    dc << std::setw(4) << rule->carrierCode();
    dc << std::setw(5) << rule->tariffNumber();
    dc << std::setw(6) << rule->ruleNumber();
    dc << std::setw(8) << rule->sequenceNumber();
    dc << std::setw(7) << rule->segCnt();

    dc << failCode << std::endl;
  }
}
}
