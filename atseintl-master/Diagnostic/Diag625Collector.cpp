//----------------------------------------------------------------------------
//  File:         Diag625Collector.C
//  Description:  Diagnostic Collector base class: Defines all the virtual methods
//                derived class may orverride these methods.
//
//  Authors:      Doug Boeving
//  Created:      August 2004
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
#include "Diagnostic/Diag625Collector.h"

#include "Common/Money.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleConst.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

namespace tse
{

void
Diag625Collector::printHeader()
{
  if (_active)
  {
    *this << "*****************  FARE BY RULE OVERRIDE  *********************\n";
    *this << "***************************************************************\n";
  }
}

Diag625Collector& Diag625Collector::operator<< (const PaxTypeFare& ptf)
{
  if (!_active)
  {
    return *this;
  }

  DiagCollector& dc = *this;
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);

  const FareMarket* fm = ptf.fareMarket();
  if (fm)
  {
    dc << " " << fm->getStartSegNum() << "--" << fm->getEndSegNum() << ":" << fm->boardMultiCity()
        << " " << fm->offMultiCity() << endl;
  }

  outputFareInformation(ptf);

  dc << " --------\n";
  if (ptf.isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPaxTypeFare = ptf.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPaxTypeFare)
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());
      if (!fbrPaxTypeFare->isSpecifiedFare() && fbrItemInfo && fbrItemInfo->ovrdcat10() == 'B')
      {
        dc << " FARE BY RULE OVERRIDE TAG - B\n USE BASE FARE CAT 10 DATA\n";
        dc << " --------\n";

        if (fbrPaxTypeFare->baseFare() && fbrPaxTypeFare->baseFare()->fare())
        {
          outputFareInformation(*fbrPaxTypeFare->baseFare()->fare());
        }
        else
        {
          dc << " NO BASE FARE INFORMATION\n";
        }
      }
      else
      {
        dc << " FARE BY RULE OVERRIDE TAG - X\n USE FARE BY RULE CAT 10 DATA\n";
      }
    }
    else
    {
      dc << " MISSING CAT 25 INFORMATION\n";
    }
  }
  else
  {
    dc << " NON CAT 25 FARE\n";
  }

  dc << "***************************************************************\n";
  return *this;
}

bool
Diag625Collector::outputFareInformation(const PaxTypeFare& ptf)
{
  DiagCollector& dc = *this;

  std::string fareBasis = ptf.createFareBasis(nullptr);
  if (fareBasis.size() > 12)
    fareBasis = fareBasis.substr(0, 12) + "$";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(13) << fareBasis << std::setw(5) << ptf.vendor() << std::setw(5)
     << ptf.tcrRuleTariff() << std::setw(3) << ptf.carrier() << std::setw(5) << ptf.ruleNumber();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << std::setw(8) << Money(ptf.fareAmount(), ptf.currency());

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(2) << ptf.owrt() << std::setw(2)
     << (ptf.directionality() == FROM ? "O" : ptf.directionality() == TO ? "I" : " ");

  std::string gd;
  globalDirectionToStr(gd, ptf.globalDirection());

  dc << std::setw(3) << gd;
  dc << "\n";

  return true;
}

bool
Diag625Collector::outputFareInformation(const Fare& fare)
{
  DiagCollector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(13) << fare.fareClass() << std::setw(5) << fare.vendor() << std::setw(5)
     << fare.tcrRuleTariff() << std::setw(3) << fare.carrier() << std::setw(5) << fare.ruleNumber();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << std::setw(8) << Money(fare.fareAmount(), fare.currency());

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(2) << fare.owrt() << std::setw(2)
     << (fare.directionality() == FROM ? "O" : fare.directionality() == TO ? "I" : " ");

  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  dc << std::setw(3) << gd;
  dc << "\n";

  return true;
}
}
