//----------------------------------------------------------------------------
//  File:        Diag323Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 323 (MiscFareTag Rule)
//
//  Updates:
//          04/25/2005  VK - create.
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

#include "Diagnostic/Diag323Collector.h"

#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MiscFareTag.h"
#include "Rules/RuleConst.h"

#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
void
Diag323Collector::writeHeader(const PaxTypeFare& paxTypeFare, const MiscFareTag& mft)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "************************************************************\n";
    dc << "*            ATSE CATEGORY 23 MISCELLANEOUS FARE TAGS RULE *\n";
    dc << "************************************************************\n";
    dc << setw(3) << paxTypeFare.fareMarket()->origin()->loc();
    dc << "-";
    dc << setw(6) << paxTypeFare.fareMarket()->destination()->loc();
    dc << setw(5) << paxTypeFare.fare()->carrier();

    dc << setw(11) << paxTypeFare.fare()->fareClass();

    dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";

    if (paxTypeFare.isConstructed())
    {
      dc << setw(15) << " CONSTRUCTED";
    }
    else
    {
      dc << setw(19) << " NOT CONSTRUCTED";
    }

    dc << '\n';

    dc << setw(9) << " VENDOR- ";
    dc << setw(5) << paxTypeFare.vendor();
    dc << setw(5) << paxTypeFare.tcrRuleTariff();
    dc << setw(5) << paxTypeFare.ruleNumber();

    dc << std::setw(10) << "    REC3-";
    dc << setw(8) << mft.itemNo();
    dc << '\n';
  }
}

void
Diag323Collector::diag323Collector(const PaxTypeFare& paxTypeFare, const MiscFareTag* mft)
{
  if (_active)
  {
    writeHeader(paxTypeFare, *mft);

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "CATEGORY 23 RULE DATA:";
    dc << '\n';
    dc << '\n';

    if (mft->unavailtag() == RuleConst::DATA_UNAVAILABLE) // dataUnavailable
    {
      dc << " DATA UNAVAILABLE" << '\n';
    }
    else if (mft->unavailtag() == RuleConst::TEXT_ONLY) // textOnly
    {
      dc << " TEXT DATA ONLY" << '\n';
    }

    dc << " OVERRIDE TABLE 994 NUMBER: ";
    dc << setw(7) << mft->overrideDateTblItemNo() << '\n';

    dc << " CONSTRUCTION - ";
    dc << setw(2) << mft->constInd() << '\n';

    dc << " PRORATE      - ";
    dc << std::setw(2) << mft->prorateInd() << '\n';

    dc << " DIFF CALC    - ";
    dc << std::setw(2) << mft->diffcalcInd() << '\n';

    dc << " REFUND CALC  - ";
    dc << std::setw(2) << mft->refundcalcInd() << '\n';

    dc << " PROPORT FARE - ";
    dc << std::setw(2) << mft->proportionalInd() << '\n';

    dc << " CURRENCY ADJ - ";
    dc << std::setw(2) << mft->curradjustInd() << '\n';

    dc << " GEO TABLE NO - ";
    dc << setw(7) << mft->geoTblItemNo() << '\n';

    dc << " FARE 1 TYPE  - ";
    if (mft->fareClassType1Ind() == RuleConst::BLANK)
    {
      dc << "NOT APPLICABLE" << '\n';
    }
    else
    {
      dc << setw(2) << mft->fareClassType1Ind();
      dc << "FARE CLASS FARE/TYPE - " << setw(9) << mft->fareClassType1() << '\n';
    }

    dc << " FARE 2 TYPE  - ";
    if (mft->fareClassType2Ind() == RuleConst::BLANK)
    {
      dc << "NOT APPLICABLE" << '\n';
    }
    else
    {
      dc << setw(2) << mft->fareClassType2Ind();
      dc << "FARE CLASS FARE/TYPE - " << setw(9) << mft->fareClassType2() << '\n';
    }

    dc << "************************************************************\n";
    dc << '\n';
  }
}
}
