//----------------------------------------------------------------------------
//  File:        Diag451Collector.C
//  Authors:     Rahib Roy
//  Created:     Mar 2005
//
//  Description: Diagnostic 451 formatter
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

#include "Diagnostic/Diag451Collector.h"

#include "Common/Money.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"

#include <iomanip>

namespace tse
{
void
Diag451Collector::displayRoutingValidationResults(TravelRoute& tvlRoute,
                                                  const std::vector<RoutingKeyInfo*>& rtgKeyInfoV)
{
  if (!_active)
  {
    return;
  }
  Diag451Collector& diag = dynamic_cast<Diag451Collector&>(*this);

  diag.buildHeader();
  diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());

  std::vector<RoutingKeyInfo*>::const_iterator rtgKeyInfoVIter = rtgKeyInfoV.begin();
  std::string gd;

  for (; rtgKeyInfoVIter != rtgKeyInfoV.end(); rtgKeyInfoVIter++)
  {
    globalDirectionToStr(gd, tvlRoute.globalDir());

    *this << "    " << gd << "  " << (*rtgKeyInfoVIter)->routing() << " TRF-" << std::setw(4)
          << (*rtgKeyInfoVIter)->routingTariff() << "  " << std::setw(7) << std::setfill(' ')
          << (*rtgKeyInfoVIter)->routingCode()
          << std::endl;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag451Collector
//
// Description:  Display the results of Routing Validation
//
// @param
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag451Collector::buildHeader()
{
  if (!_active)
  {
    return;
  }
  *this << "\n******************** ROUTING DATA INFO ************************\n";
}
}
