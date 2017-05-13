#include "Diagnostic/Diag457Collector.h"

#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Fares/RoutingController.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"

#include <iomanip>
#include <map>

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag457Collector
//
// Description:  Display the results of Routing Validation
//
// @param
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag457Collector::buildHeader()
{
  if (!_active)
  {
    return;
  }

  ((DiagCollector&)*this) << "\n****** ROUTING MAP AND ROUTING LIST DEBUGGING DISPLAY *********\n";
}

// ----------------------------------------------------------------------------
void
Diag457Collector::displayRoutingListItem(const Routing* routing, int16_t listItemNumber)
{
  ((DiagCollector&)*this)
      << "\n"
      << "      ROUTING LIST ITEM " << listItemNumber << "  VENDOR          " << routing->vendor()
      << std::endl << "                           ROUTING TARIFF  " << routing->routingTariff()
      << std::endl << "                           CARRIER         " << routing->carrier()
      << std::endl << "                           ROUTING NUMBER  " << routing->routing()
      << std::endl
      //<<  "                           DRV INDICATOR   "  << routing->domRtgvalInd() << std::endl
      << "                           RESTRICTIONS    " << routing->noofRestrictions() << std::endl;

  if (!(routing->rmaps().empty()))
  {
    ((DiagCollector&)*this) << "                           ROUTE MAP EXISTS" << std::endl;
  }
  else
  {
    ((DiagCollector&)*this) << "                           NO ROUTE MAP" << std::endl;
  }

  ((DiagCollector&)*this) << " \n";
}

// ----------------------------------------------------------------------------
void
Diag457Collector::displayRoutingMapItem(std::map<RtgKey, bool>::const_iterator rMap,
                                        int16_t mapItemNumber)
{
  RtgKey rKey = rMap->first;

  ((DiagCollector&)*this)
      << "      ROUTE MAP ITEM " << mapItemNumber << "     VENDOR           " << rKey.vendor()
      << std::endl << "                           ROUTING TARIFF 1 " << rKey.routingTariff()
      << std::endl << "                           CARRIER          " << rKey.carrier() << std::endl
      << "                           ROUTING NUMBER   " << rKey.routingNumber() << std::endl
      << "                           ROUTE STATUS     ";

  if (rMap->second)
  {
    ((DiagCollector&)*this) << "TRUE" << std::endl << " \n";
  }
  else
  {
    ((DiagCollector&)*this) << "FALSE" << std::endl << " \n";
  }
}
}
