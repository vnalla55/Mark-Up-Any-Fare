//-------------------------------------------------------------------
//  File   : FareGroupingMgr.cpp
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FQFareGroupingMgr.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Grouping");

bool
FQFareGroupingMgr::groupFares(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FQFareGroupingMgr::gropFares() ");

  if (trx.allPaxTypeFare().empty())
  {
    LOG4CXX_INFO(logger, "Leaving FQFareGroupingMgr::gropFares() -- No Fares for the Market ");
    return true;
  }

  processGrouping(trx);

  LOG4CXX_INFO(logger, "Leaving FQFareGroupingMgr::gropFares() ");
  return true;
}
}
