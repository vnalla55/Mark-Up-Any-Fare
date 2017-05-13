//-------------------------------------------------------------------
//
//  File: DefaultGroupingStrategy.cpp
//  Author:Abu
//
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

#include "FareDisplay/DefaultGroupingStrategy.h"

#include "Common/Logger.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "FareDisplay/GroupHeader.h"

#include <algorithm>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDisplay.DefaultGroupingStrategy");

const Indicator DefaultGroupingStrategy::DESCENDING = 'D';
const Indicator DefaultGroupingStrategy::ASCENDING = 'A';

bool
DefaultGroupingStrategy::apply()
{
  if (_trx == nullptr)
  {
    LOG4CXX_INFO(logger, "BYPASSING DEFAULT GROUPING DUE TO NULL TRX");
    return false;
  }

  Indicator sortType = _trx->getOptions()->sortDescending() ? DefaultGroupingStrategy::DESCENDING
                                                            : DefaultGroupingStrategy::ASCENDING;
  createGroup(Group::GROUP_BY_FARE_AMOUNT, sortType);
  if(!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
     _trx->getRequest()->multiInclusionCodes())
    createGroup(Group::GROUP_BY_CABIN);

  LOG4CXX_INFO(logger, "PROCESSING DEFAULT GROUPING DUE TO SORT OVERRIDE");
  groupAndSort();

  GroupHeader header(*_trx);
  header.setGroupHeaderInfo(_groups);

  LOG4CXX_INFO(logger, "LEAVING DEFAULT GROUPING DUE TO SORT OVERRIDE");

  return true;
}
}
