//----------------------------------------------------------------------------
//  File:        Diag512Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 512 (Total Surcharges collection)
//
//  Updates:
//          09/09/2004  VK - create.
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

#include "Diagnostic/Diag512Collector.h"

#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Rules/RuleConst.h"
#include "Rules/SecSurchargeAppl.h"

#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{
void
Diag512Collector::writeFPHeader(MoneyAmount& amt, const FarePath& fareP)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "************************************************************\n";
    dc << "*   ATSE     FINAL SURCHARGE COLLECTION DIAGNOSTIC         *\n";
    dc << "************************************************************\n";
    dc << '\n';
    dc << setw(10) << "PAXTYPE -";
    dc << setw(7) << fareP.paxType()->paxType();
    dc << setw(30) << "SURCHARGE PLUSUP NUC AMOUNT -";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << std::setw(8) << amt;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << '\n';
    dc << '\n';
  }
}

void
Diag512Collector::writePUHeader(const PricingUnit& pu)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "************************************************************\n";

    dc << '\n';
    dc << setw(14) << "*   PU TYPE   - ";
    dc << setw(11) << DiagnosticUtil::pricingUnitTypeToString(pu.puType());

    dc << '\n';
  }
}

void
Diag512Collector::writeFUHeader(const FareUsage& fu)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << '\n';
    dc << "*   FARE USAGE: \n";

    dc << setw(3) << fu.paxTypeFare()->fareMarket()->origin()->loc();
    dc << "-";
    dc << setw(3) << fu.paxTypeFare()->fare()->carrier();
    dc << "-";
    dc << setw(5) << fu.paxTypeFare()->fareMarket()->destination()->loc();

    dc << setw(10) << fu.paxTypeFare()->fare()->fareClass();

    dc << std::setw(8) << Money(fu.paxTypeFare()->fareAmount(), fu.paxTypeFare()->currency())
       << " ";
    dc << "     ";
    dc << setw(5) << fu.paxTypeFare()->vendor();
    dc << setw(5) << fu.paxTypeFare()->fare()->tcrRuleTariff();
    dc << setw(5) << fu.paxTypeFare()->fare()->ruleNumber();
    dc << '\n';
    dc << " " << '\n';

    if (!fu.surchargeData().empty())
    {
      dc << "    SURCHARGES: " << '\n';
      dc << "------------------------------------------------------------\n";
      dc << "TYPE     TOTAL AMT CUR   NUCAMT  TRAVEL PORTION BRD-OFF APPL " << '\n';
      dc << " " << '\n';
    }
    else
    {

      dc << "    NO SURCHARGES   \n";
      dc << " " << '\n';
    }
  }
}

void
Diag512Collector::displaySurchargeData(const SurchargeData& sd, const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);

    string tvl;
    Indicator i = sd.travelPortion();
    if (i == RuleConst::BLANK || i == RuleConst::ONEWAY)
      tvl = "ONEWAY";
    else if (i == RuleConst::ROUNDTRIP)
      tvl = "ROUNDTRIP";
    else if (i == RuleConst::PERTRANSFER)
      tvl = "PER TRANSFER";
    else if (i == RuleConst::PERTICKET)
      tvl = "PER TICKET";
    else if (i == RuleConst::PERCOUPON)
      tvl = "PER COUPON";
    else if (i == RuleConst::PERDIRECTION)
      tvl = "PER DIRECTION";
    else
      tvl = "UNKNOWN";

    string type;
    i = sd.surchargeType();
    if (i == RuleConst::AIRPORT)
      type = "AIRPORT";
    else if (i == RuleConst::BUSINESCLASS)
      type = "BUSINESS";
    else if (i == RuleConst::SUPERSONIC)
      type = "SUPERSC";
    else if (i == RuleConst::PEAKTRAVEL)
      type = "PEAKTRAVEL";
    else if (i == RuleConst::EQUIPMENT)
      type = "EQUIPMENT";
    else if (i == RuleConst::FUEL)
      type = "FUEL";
    else if (i == RuleConst::PEAK)
      type = "PEAK";
    else if (i == RuleConst::HOLIDAY)
      type = "HOLIDAY";
    else if (i == RuleConst::SIDETRIP)
      type = "SIDETRIP";
    else if (i == RuleConst::SEASONAL)
      type = "SEASONAL";
    else if (i == RuleConst::WEEKEND)
      type = "WEEKEND";
    else if (i == RuleConst::SLEEPERETTE)
      type = "SLEEP";
    else if (i == RuleConst::WAIVERADVRES)
      type = "WAIVERADV";
    else if (i == RuleConst::NAVIGATION)
      type = "NAVIGATION";
    else if (i == RuleConst::SECURITY)
    {
      if (SecSurchargeAppl::SURCHARGE_DESC == sd.surchargeDesc())
      {
        type = "SECTOR";
      }
      else
      {
        type = "SECURITY";
      }
    }
    else if (i == RuleConst::WAIVERMAXSTAY)
      type = "WAIVERMAX";
    else if (i == RuleConst::SURFACE)
      type = "SURFACE";
    else if (i == RuleConst::RBD)
      type = "SVC CLASS";
    else if (i == RuleConst::OTHER)
      type = "OTHER";
    else
      type = "UNKNOWN";
    MoneyAmount totalAmt = sd.amountSelected() * sd.itinItemCount();
    MoneyAmount totalNUCAmt = sd.amountNuc() * sd.itinItemCount();

    if (type == "UNKNOWN")
      dc << setw(10) << sd.surchargeType();
    else
      dc << setw(10) << type;
    dc << setw(8) << Money(totalAmt, sd.currSelected());
    dc << " ";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << setw(8) << totalNUCAmt;
    dc << " ";
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " ";
    dc << setw(15) << tvl;
    dc << setw(3) << sd.brdAirport();
    dc << "-";
    dc << setw(7) << sd.offAirport();

    if ((sd.travelPortion() != RuleConst::PERDIRECTION &&
         sd.travelPortion() != RuleConst::PERTICKET) ||
        sd.selectedTkt())
    {
      dc << setw(3) << "Y";
    }
    dc << '\n';
  }
}

void
Diag512Collector::diag512Collector(MoneyAmount& amt, FarePath& fareP, const PricingTrx& trx)
{
  if (_active)
  {
    writeFPHeader(amt, fareP);

    std::vector<PricingUnit*>::iterator puIt = fareP.pricingUnit().begin();
    std::vector<PricingUnit*>::iterator puItEnd = fareP.pricingUnit().end();

    for (; puIt != puItEnd; ++puIt)
    {
      PricingUnit& pricingUnit = **puIt;

      writePUHeader(pricingUnit);

      // go throuth the Fare Usages
      std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
      std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

      for (; fareUsageI != fareUsageEnd; ++fareUsageI)
      {
        writeFUHeader(**fareUsageI);

        std::vector<SurchargeData*>::iterator sI = (*fareUsageI)->surchargeData().begin();
        std::vector<SurchargeData*>::iterator sEnd = (*fareUsageI)->surchargeData().end();

        for (; sI != sEnd; ++sI)
        {
          displaySurchargeData(**sI, trx);
        } // sd
      } // fu
    } // pu
  }
}
}
