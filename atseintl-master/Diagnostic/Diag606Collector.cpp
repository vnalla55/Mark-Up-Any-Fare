//----------------------------------------------------------------------------
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
#include "Diagnostic/Diag606Collector.h"

#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>
#include <vector>

namespace tse
{
bool Diag606Collector::_fcSBdisplayDone = false;
boost::mutex Diag606Collector::_fcSBdisplayMutex;

// ----------------------------------------------------------------------------
void
Diag606Collector::initParam(Diagnostic& root)
{
  _rootDiag = &root;

  std::map<std::string, std::string>::iterator itEnd = root.diagParamMap().end();
  std::map<std::string, std::string>::iterator it = root.diagParamMap().find("FC");
  if (it != itEnd)
  {
    _fareClass = it->second.c_str();

    it = root.diagParamMap().find("FM");
    if (it != itEnd)
    {
      std::string fareMarket = it->second.c_str();
      if (fareMarket.length() >= 6)
      {
        _origin = fareMarket.substr(0, 3);
        _destination = fareMarket.substr(3, 3);
        if (fareMarket.length() >= 8)
        {
          _carrier = fareMarket.substr(6, 2);
        }
      }
    }

    if (_carrier.length() == 0)
    {
      it = root.diagParamMap().find("CX");
      if (it != itEnd)
        _carrier = it->second.c_str();
    }
    if (_origin.length() == 0)
    {
      it = root.diagParamMap().find("OR");
      if (it != itEnd)
        _origin = it->second.c_str();
    }
    if (_destination.length() == 0)
    {
      it = root.diagParamMap().find("DS");
      if (it != itEnd)
        _destination = it->second.c_str();
    }
  }
}

void
Diag606Collector::printHeader()
{
  if (_active)
  {
    ((std::ostringstream&)*this)
        << "******************** SCOREBOARD ANALYSIS **********************\n";
  }
}

Diag606Collector&
Diag606Collector::operator << (const PricingUnit& pu )
{
  if (_active)
  {
    // lock the mutex
    {
      boost::lock_guard<boost::mutex> g(_fcSBdisplayMutex);
      if (_fcSBdisplayDone)
        return *this;

      if (_fareClass.empty())
      {
        _fcSBdisplayDone = true;
        *this << "************** NO FARE CLASS - NOTHING TO LOOK FOR ************\n";
        return *this;
      }
    }

    DiagCollector& dc = *this;

    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

    for (; fuIter != fuIterEnd; ++fuIter)
    {
      const PaxTypeFare* fare = (*fuIter)->paxTypeFare();

      if (_fareClass == fare->fareClass() &&
          (_carrier.length() == 0 || _carrier == fare->carrier()) &&
          (_origin.length() == 0 || _origin == fare->origin()) &&
          (_destination.length() == 0 || _destination == fare->destination()))
      {
        *this << "FARE CLASS: " << _fareClass << std::endl;

        // lock the mutex
        {
          boost::lock_guard<boost::mutex> g(_fcSBdisplayMutex);
          if (_fcSBdisplayDone)
            return *this;
          _fcSBdisplayDone = true;
        }

        CombinabilityRuleInfo* pCat10 = (*fuIter)->rec2Cat10();

        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << "FROM-" << std::setw(4) << fare->origin() << "TO-" << std::setw(4)
           << fare->destination() << "CARRIER-" << std::setw(4) << fare->carrier() << "RULE "
           << std::setw(4) << fare->ruleNumber() << "-" << std::setw(7) << fare->tcrRuleTariffCode()
           << "\n"
           << "FARE BASIS-" << std::setw(9) << fare->fareClass() << std::setw(2)
           << (fare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "RT" : "OW") << "-" << std::setw(5)
           << (*fuIter)->paxTypeFare()->fcaFareType() << std::setw(5)
           << (((*fuIter)->isPaxTypeFareNormal()) ? "NORMAL FARE" : "SPECIAL FARE") << "\n\n";

        if ((*fuIter)->rec2Cat10() == 0)
        {
          dc << "            ******** NO RECORD 2 CATEGORY 10 ********\n";
          break;
        }

        tools::printCat10Info(dc, pCat10);

        break;
      }

    } // for fuIter

    {
      boost::lock_guard<boost::mutex> g(_fcSBdisplayMutex);
      if (!_fcSBdisplayDone)
        *this << "FARE CLASS NOT FOUND" << std::endl;
      _fcSBdisplayDone = true;
    }
  }
  return *this;
}
}
