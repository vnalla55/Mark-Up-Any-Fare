//----------------------------------------------------------------------------
//  File:         Diag603Collector.C
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

#include "Diagnostic/Diag603Collector.h"

#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{
void
Diag603Collector::printHeader()
{
  if (_active)
  {
    *this << "*******************  PRICING UNIT LIST  ***********************\n";
  }
}

Diag603Collector& Diag603Collector::operator<< (const PricingUnit& pu)
{
  if (!_active)
  {
    return *this;
  }

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  DiagCollector& dc = *this;
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);

  uint16_t count = 0;
  std::vector<FareUsage*>::const_iterator it = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator itEnd = pu.fareUsage().end();
  for (; it != itEnd; ++it)
  {
    ++count;
    FareMarket* fm = (*it)->paxTypeFare()->fareMarket();
    dc << " " << fm->getStartSegNum() << "--" << fm->getEndSegNum() << ":" << fm->boardMultiCity()
       << " " << fm->offMultiCity() << " ";

    if (count % 4 == 0)
      dc << "\n";
  }
  if (count % 4 != 0)
    dc << "\n";

  it = pu.fareUsage().begin();
  for (; it != itEnd; ++it)
  {
    PaxTypeFare* fare = (*it)->paxTypeFare();

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << std::setw(13) << DiagnosticUtil::getFareBasis(*fare) << " " << std::setw(5) << fare->vendor() << std::setw(5)
       << fare->tcrRuleTariff() << std::setw(3) << fare->carrier() << std::setw(5)
       << fare->ruleNumber();

    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc << std::setw(8) << Money(fare->fareAmount(), fare->currency());

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << std::setw(2) << fare->owrt() << std::setw(2)
       << (fare->directionality() == FROM ? "O" : fare->directionality() == TO ? "I" : " ");

    std::string gd;
    globalDirectionToStr(gd, fare->globalDirection());

    dc << std::setw(3) << gd;

    dc << std::setw(3) << (*it)->paxTypeFare()->fcaFareType();

    dc.setf(std::ios::right, std::ios::adjustfield);
    dc << " ";
    dc << std::setw(6)
       << (pu.puType() == PricingUnit::Type::OPENJAW
               ? "101/OJ"
               : (pu.puType() == PricingUnit::Type::ROUNDTRIP
                      ? "102/RT"
                      : (pu.puType() == PricingUnit::Type::CIRCLETRIP
                             ? "103/CT"
                             : (pu.puType() == PricingUnit::Type::ONEWAY ? "104/OW" : " ")))) << "\n";
  }

  if (_trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "CABIN")
  {
    dc << "CABIN COMBINATION: ";
    if(pricingTrx)
       DiagnosticUtil::displayCabinCombination(*pricingTrx, pu, dc);
    dc << std::endl;
  }

  return *this;
}
}
