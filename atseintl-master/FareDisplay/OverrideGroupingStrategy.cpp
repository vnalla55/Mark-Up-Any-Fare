//-------------------------------------------------------------------
//
//  File: OverrideGroupingStrategy.cpp
//  Author:Abu
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

#include "FareDisplay/OverrideGroupingStrategy.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/GroupHeader.h"
#include "FareDisplay/GroupingAlgorithm.h"

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDisplay.OverrideGroupingStrategy");

bool
OverrideGroupingStrategy::apply()
{
  LOG4CXX_INFO(logger, "Entered OverrideGroupingStrategy ::apply() ");

  initialize();

  groupAndSort();

  return true;
}

bool
OverrideGroupingStrategy::initialize()
{
  LOG4CXX_DEBUG(logger, "Entered ProcessPsgGroupingRqst::apply() ");

  createGroup(Group::GROUP_BY_FARE_AMOUNT);
  createGroup(Group::GROUP_BY_PSG_TYPE);
  if(!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
     _trx->getRequest()->multiInclusionCodes())
    createGroup(Group::GROUP_BY_CABIN);
  GroupHeader header(*_trx);
  header.setGroupHeaderInfo(_groups);
  LOG4CXX_DEBUG(logger, "Leaving ProcessPsgGroupingRqst::apply() ");
  return true;
}
}
