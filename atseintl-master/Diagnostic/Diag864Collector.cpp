//----------------------------------------------------------------------------
//  File:        Diag864Collector.C
//  Authors:     Marco Cartolano / Adrian Tovar
//  Created:     August 27, 2007
//
//  Description: Diagnostic 864 formatter - Consolidator Plus Up Analysis
//
//  Updates:
//          date - initials - description.
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
#include "Diagnostic/Diag864Collector.h"

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{
void
Diag864Collector::printHeader()
{
  if (_active)
  {
    *this << "*************** CONSOLIDATOR PLUS UP - ANALYSIS ***************\n";
  }
}

Diag864Collector & Diag864Collector::operator<< (const  Agent& agent)
{
  if (_active)
  {
    *this << "AGENT PCC: " << agent.tvlAgencyPCC() << "/" << agent.agentLocation()->nation()
          << std::endl;
  }

  return *this;
}

Diag864Collector & Diag864Collector::operator<< (const PaxTypeFare& ptFare)
{
  if (!_active)
  {
    return *this;
  }

  Diag864Collector& dc = *this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "FARE CLASS   : " << std::setw(9) << ptFare.fareClass() << std::endl
     << "VENDOR       : " << std::setw(5) << ptFare.vendor() << std::endl
     << "CARRIER      : " << std::setw(3) << ptFare.carrier() << std::endl
     << "RULE TARIFF  : " << std::setw(4) << ptFare.tcrRuleTariff() << "-" << std::setw(8)
     << ptFare.tcrRuleTariffCode() << std::endl << "RULE NUMBER  : " << std::setw(5)
     << ptFare.ruleNumber() << std::endl << "DISPLAY TYPE : " << std::setw(2)
     << ptFare.fcaDisplayCatType() << std::endl << "AMOUNT/CURR  : " << std::setw(8)
     << Money(ptFare.fareAmount(), ptFare.currency()) << std::endl;

  return *this;
}

void
Diag864Collector::printPriceDeviationResults(PricingTrx& pt)
{
  Diag864Collector& dc = *this;
  for (const Itin* it : pt.itin())
    for (const FarePath* fp : it->farePath())
    {
      const CurrencyCode calcCurr = it->calculationCurrency();

      dc << "***************************************************************" << std::endl;
      if (pt.isMip())
        dc << "ITIN: " << it->itinNum() << " ";
      dc << "REQUESTED PAXTYPE: " << fp->paxType()->paxType() << std::endl
         << "  TOTAL AMOUNT: " << fp->getTotalNUCAmount() << calcCurr << std::endl
         << "  TOTAL SCORE: " << fp->getNUCAmountScore() << calcCurr << std::endl
         << "  TOTAL PRICE DEVIATION: " << fp->getDynamicPriceDeviationAmount() << calcCurr
         << std::endl;
      dc << "  PU FU FM         FAREBASIS" << std::endl;

      for (size_t puInd = 0; puInd < fp->pricingUnit().size(); ++puInd)
      {
        const PricingUnit* pu = fp->pricingUnit()[puInd];

        for (size_t fuInd = 0; fuInd < pu->fareUsage().size(); ++fuInd)
        {
          const FareUsage& fu = *pu->fareUsage()[fuInd];
          const PaxTypeFare& ptf = *fu.paxTypeFare();

          dc << "  " << std::setw(3) << std::left << (puInd + 1) << std::setw(3) << (fuInd + 1);
          std::stringstream fmStr;
          fmStr << ptf.fareMarket()->boardMultiCity() << "-" << ptf.fareMarket()->governingCarrier()
                << "-" << ptf.fareMarket()->offMultiCity();

          auto priceDeviation = fu.getDiscAmount() != 0 ? -fu.getDiscAmount() : fu.getDiscAmount();

          dc << std::setw(11) << std::left << fmStr.str() << std::setw(9) << ptf.createFareBasis(pt)
             << std::endl;
          dc << "    FARE AMOUNT: " << ptf.totalFareAmount() << calcCurr << std::endl
             << "    MIN FARE PLUS UP: " << fu.minFarePlusUpAmt() << calcCurr << std::endl
             << "    DIFFERENTIAL: " << fu.differentialAmt() << calcCurr << std::endl
             << "    SURCHARGE: " << fu.surchargeAmt() << calcCurr << std::endl
             << "    TRANSFER SURCHARGE: " << fu.transferAmt() << calcCurr << std::endl
             << "    STOPOVER SURCHARGE: " << fu.stopOverAmt() << calcCurr << std::endl
             << "    PRICE DEVIATION: " << priceDeviation << calcCurr << std::endl;
        }
      }
    }
  dc << "***************************************************************" << std::endl;
}
}
