//-------------------------------------------------------------------
//
//  File: PreferredGroupingStrategy.cpp
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

#include "FareDisplay/PreferredGroupingStrategy.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
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
logger("atseintl.FareDisplay.PreferredGroupingStrategy");

PreferredGroupingStrategy::~PreferredGroupingStrategy() {}

PreferredGroupingStrategy::PreferredGroupingStrategy() {}

bool
PreferredGroupingStrategy::apply()
{
  LOG4CXX_INFO(logger, "Entered PreferredGroupingStrategy ::apply() ");

  if (!isProcessPsgGroupingRqst() && isApplyMultiTransportGrouping())
  {
    createGroup(Group::GROUP_BY_MULTITRANSPORT);
    GroupHeader header(*_trx);
    header.setMultiTransportHeader(_groups, _trx->fdResponse()->groupHeaders());
  }

  if(!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
     _trx->getRequest()->multiInclusionCodes())
  {
    createGroup(Group::GROUP_BY_CABIN);

    bool found = false;
    for (const auto& groupHdr : _trx->fdResponse()->groupHeaders())
    {
      if( Group::GROUP_BY_CABIN == groupHdr )
        found = true;
    }
    if(!found)
    {
      GroupHeader header(*_trx);
      header.setCabinHeader();
    }
 }

  if (hasGroup(Group::GROUP_BY_BRAND))
  {
    reorderGroup(Group::GROUP_BY_BRAND);
  }

  if (hasGroup(Group::GROUP_BY_S8BRAND))
  {
    reorderGroup(Group::GROUP_BY_S8BRAND);
  }

  if (hasGroup(Group::GROUP_BY_TRAVEL_DISCONTINUE_DATE))
  {
    _trx->fdResponse()->setGroupedByTravelDiscDate();
  }

  groupAndSort();

  LOG4CXX_INFO(logger, "Leaving PreferredGroupingStrategy ::apply() ");
  return true;
}

bool
PreferredGroupingStrategy::isProcessPsgGroupingRqst()
{
  LOG4CXX_DEBUG(logger, "Entered ProcessPsgGroupingRqst::apply() ");

  if (!_trx->getRequest()->isPaxTypeRequested())
  {
    return false;
  }

  if (hasGroup(Group::GROUP_BY_PSG_TYPE))
  {
    reorderGroup(Group::GROUP_BY_PSG_TYPE);
    return true;
  }
  else
    createGroup(Group::GROUP_BY_PSG_TYPE);

  LOG4CXX_DEBUG(logger, "Leaving ProcessPsgGroupingRqst::apply() ");
  return true;
}

bool
PreferredGroupingStrategy::hasGroup(Group::GroupType grpType)
{
  std::vector<Group*>::iterator i(_groups.end());
  i = std::find_if(_groups.begin(), _groups.end(), EqualByGrpType(grpType));
  return (i != _groups.end());
}

void
PreferredGroupingStrategy::reorderGroup(Group::GroupType grpType)
{
  std::vector<Group*>::iterator i(_groups.end());
  i = std::find_if(_groups.begin(), _groups.end(), EqualByGrpType(grpType));
  if (i != _groups.end())
  {
    if (i == _groups.begin())
    {
      return;
    }
    else
    {
      Group* group = *i;
      _groups.erase(i);
      _groups.insert(_groups.begin(), group);
    }
  }
}

bool
PreferredGroupingStrategy::isApplyMultiTransportGrouping()
{
  static int16_t multiTransportGrouping = -1;

  if (multiTransportGrouping == -1)
  {
    // Get the config value the first time through. We'll reuse the flag
    // subsequent times for efficiency.
    std::string configVal("N");
    if (!(Global::config()).getValue("GROUP_MULTITRANSPORT_FARES", configVal, "FAREDISPLAY_SVC"))
    {
      CONFIG_MAN_LOG_KEY_ERROR(logger, "GROUP_MULTITRANSPORT_FARES", "FAREDISPLAY_SVC");
    }

    if (configVal == "Y" || configVal == "y" || configVal == "1")
      multiTransportGrouping = 1;
    else
      multiTransportGrouping = 0;
  }
  return (multiTransportGrouping > 0 ? true : false);
}
}
