//----------------------------------------------------------------------------
//  File:         Diag605Collector.C
//  Description:  Diagnostic Collector base class: Defines all the virtual methods
//                derived class may orverride these methods.
//
//  Authors:      Mohammad Hossan
//  Created:      April 2004
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
#include "Diagnostic/Diag605Collector.h"

#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagnosticUtil.h"


#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{
void
Diag605Collector::printHeader()
{
  if (_active)
  {
    *this << "*******************  PRICING UNIT LIST  ***********************\n";
  }
}

Diag605Collector& Diag605Collector::operator<< (const PricingUnit& pu)
{
  if (!_active)
    return *this;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  DiagCollector& dc = *this;
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);

  if (pu.taxAmount() > 0)
    dc << "TAX AND SURCHARGE " << pu.taxAmount() << " WITH " << pu.travelSeg().size() << " FLIGHTS"
       << std::endl;

  uint16_t count = 0;
  for (const FareUsage* fareUsage : pu.fareUsage())
  {
    ++count;
    const FareMarket* fm = fareUsage->paxTypeFare()->fareMarket();
    dc << " " << fm->getStartSegNum() << "--" << fm->getEndSegNum() << ":" << fm->boardMultiCity()
       << " " << fm->offMultiCity() << " ";

    if (count % 4 == 0)
      dc << "\n";
  }

  if (count % 4 != 0)
    dc << "\n";

  for (const FareUsage* fareUsage : pu.fareUsage())
  {
    const PaxTypeFare* fare = fareUsage->paxTypeFare();
    if (!fare)
      continue;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << std::setw(13) << DiagnosticUtil::getFareBasis(*fare) << " " << std::setw(4) << fare->vendor()
       << std::setw(5) << fare->tcrRuleTariff() << std::setw(3) << fare->carrier() << std::setw(5)
       << fare->ruleNumber();

    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc << std::setw(8) << Money(fare->fareAmount(), fare->currency());

    if (pricingTrx && pricingTrx->awardRequest())
    {
      dc.setf(std::ios::right);
      dc << std::setw(8) << " " << fare->mileage() << " MIL";
    }

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << std::setw(1) << fare->owrt() << std::setw(2)
       << (fare->directionality() == FROM ? "O" : fare->directionality() == TO ? "I" : " ");

    std::string gd;
    globalDirectionToStr(gd, fare->globalDirection());

    dc << std::setw(3) << gd;

    dc << std::setw(3) << fareUsage->paxTypeFare()->fcaFareType();

    dc.setf(std::ios::right, std::ios::adjustfield);
    if (fareUsage->rec2Cat10() != nullptr)
      dc << std::setw(7) << fareUsage->rec2Cat10()->sequenceNumber();
    else
      dc << "       ";

    dc << " ";
    dc << std::setw(2) << DiagnosticUtil::pricingUnitTypeToShortString(pu.puType()) << "\n";
    displayRetrievalDate(*(fareUsage->paxTypeFare()));
    dc << "\n";
  }

  if (pricingTrx &&
      pricingTrx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "CABIN")
  {
    dc << "CABIN COMBINATION: ";
    DiagnosticUtil::displayCabinCombination(*pricingTrx, pu, dc);
    dc << std::endl;
  }

  return *this;
}
}
