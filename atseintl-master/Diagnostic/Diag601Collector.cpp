//----------------------------------------------------------------------------
//  File:         Diag601Collector.C
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

#include "Diagnostic/Diag601Collector.h"

#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
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
Diag601Collector::printHeader()
{
  if (_active)
  {
    *this << "*******************  FARE SELECTION FOR PU ********************\n";
  }
}

void
Diag601Collector::displayCat10RuleData(const CombinabilityRuleInfo* pCat10, const PaxTypeFare& fare)
{
  if (pCat10 == nullptr)
    return;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!_active || (pricingTrx && DiagnosticUtil::filter(*pricingTrx, fare)))
  {
    return;
  }

  DiagCollector& dc = *this;

  dc << std::setw(9) << pCat10->sequenceNumber();

  std::map<std::string, std::string>::iterator itEnd = _rootDiag->diagParamMap().end();
  std::map<std::string, std::string>::iterator it = _rootDiag->diagParamMap().find("MX");
  if (it == itEnd)
  {
    return;
  }
  std::string fareClass;
  it = _rootDiag->diagParamMap().find("FC");
  if (it != itEnd)
  {
    fareClass = it->second;
    if (fare.fareClass() != fareClass && fare.createFareBasis(nullptr) != fareClass)
    {
      return;
    }
  }

  dc << "\n-----------\n";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "FROM-" << std::setw(4) << fare.origin() << "TO-" << std::setw(4) << fare.destination()
     << "CARRIER-" << std::setw(4) << fare.carrier() << "RULE " << std::setw(4) << fare.ruleNumber()
     << "-" << std::setw(7) << fare.tcrRuleTariffCode() << "\n"
     << "FARE BASIS-" << std::setw(13) << fare.createFareBasis(nullptr) << std::setw(2)
     << (fare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "RT" : "OW") << "FARE DIR:"
     << (fare.directionality() == BOTH ? "NONE" : (fare.directionality() == TO ? "T" : "F")) << "-"
     << std::setw(5) << fare.fcaFareType() << std::setw(5)
     << ((fare.isNormal()) ? "NORMAL FARE" : "SPECIAL FARE") << "\n\n";

  tools::printCat10Info(dc, pCat10);

  dc << "\n-----------\n";
  return;
}

Diag601Collector & Diag601Collector::operator<< (const  FareUsage& fu)
{
  const PaxTypeFare* paxTypeFare = fu.paxTypeFare();

  displayFareInfo(*paxTypeFare);

  const CombinabilityRuleInfo* rec2Cat10 = fu.rec2Cat10();

  displayCat10RuleData(rec2Cat10, *paxTypeFare);

  return *this;
}

void
Diag601Collector::displayFareInfo(const PaxTypeFare& fare)
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);

  if (!_active || (pricingTrx && DiagnosticUtil::filter(*pricingTrx, fare)))
  {
    return;
  }

  displayPtfItins(fare);

  DiagCollector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(13) << DiagnosticUtil::getFareBasis(fare) << " " << std::setw(4) << fare.vendor() << std::setw(5)
     << fare.tcrRuleTariff() << std::setw(3) << fare.carrier() << std::setw(5) << fare.ruleNumber();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << std::setw(8) << Money(fare.fareAmount(), fare.currency());

  if (pricingTrx)
    if (pricingTrx->awardRequest())
    {
      dc.setf(std::ios::right);
      dc << std::setw(8) << " " << fare.mileage() << " MIL";
    }

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(1) << fare.owrt() << std::setw(2)
     << (fare.directionality() == FROM ? "O" : (fare.directionality() == TO ? "I" : " "));

  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  dc << std::setw(3) << gd;

  return;
}

Diag601Collector & Diag601Collector::operator<< (const  PaxTypeFare& fare)
{

  displayFareInfo(fare);

  const CombinabilityRuleInfo* pCat10 = fare.rec2Cat10();

  displayCat10RuleData(pCat10, fare);

  return *this;
}
}
