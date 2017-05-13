//-------------------------------------------------------------------
//  Created:     June 31, 2005
//  Authors:     Abu
//  Copyright Sabre 2007
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

#include "FareDisplay/GroupingDataRetriever.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/FareDisplaySort.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupHeader.h"

namespace tse
{
static Logger
logger("atseintl.FareDiplay.GroupingDataRetriever");

GroupingDataRetriever::GroupingDataRetriever(FareDisplayTrx& trx)
  : FDCustomerRetriever(trx), _domIntlAppl(' '), _fareDisplayType(' ')
{
  if (!trx.itin().empty())
  {
    Itin* itin = trx.itin().front();

    _domIntlAppl = ((itin->geoTravelType() == GeoTravelType::Domestic || itin->geoTravelType() == GeoTravelType::Transborder ||
                     itin->geoTravelType() == GeoTravelType::ForeignDomestic) &&
                    !trx.isSameCityPairRqst())
                       ? INDICATOR_DOMESTIC
                       : INDICATOR_INTERNATIONAL;

    _fareDisplayType = trx.isShopperRequest() ? MULTIPLE_CXR_FD_TYPE : SINGLE_CXR_FD_TYPE;
  }
}

void
GroupingDataRetriever::getGroupAndSortPref(std::vector<Group*>& groups)
{
  LOG4CXX_INFO(logger, " Entered GroupingDataRetriever::getGroupAndSortPref()");
  retrieveOne(); // one user type, may have many grouping records

  if (initializeGroups(groups))
  {
    setHeader(groups);
  }
  else
  {
    _parentTableData.clear();
    retrieveDefault();
    LOG4CXX_DEBUG(logger, " PROCESSING DEFAULTS ");
    initializeGroups(groups);
    setHeader(groups);
  }

  LOG4CXX_INFO(logger, " Leaving GroupingDataRetriever::getGroupAndSortPref()");
}

bool
GroupingDataRetriever::initializeGroups(std::vector<Group*>& groups)
{
  if (!_parentTableData.empty())
  {
    std::sort(_parentTableData.begin(), _parentTableData.end(), LessBySeqNo());
    groups.reserve(_parentTableData.size());
    std::vector<FareDisplaySort*>::iterator i(_parentTableData.begin()),
        end(_parentTableData.end());
    ComparatorFactory rVFactory(_trx);

    for (; i != end; ++i)
    {
      // lint --e{413}
      Group* group = nullptr;
      _trx.dataHandle().get(group);

      if (createGroup(**i, *group))
      {
        group->comparator() = rVFactory.getComparator(group->groupType());
        group->comparator()->group() = group;
        group->comparator()->prepare(_trx);
        groups.push_back(group);
      }
    }
    return (groups.empty() == false);
  }
  return false;
}

//----------------------------------------------------------------------------
// GroupingDataRetriever::createGroup()
//----------------------------------------------------------------------------

bool
GroupingDataRetriever::createGroup(FareDisplaySort& sortData, Group& groupData)
{
  groupData.groupType() = getGroupType(sortData, groupData);
  groupData.sortData() = &sortData;
  if ((sortData.fareDisplayType() == _fareDisplayType || sortData.fareDisplayType() == ' ') &&
      (sortData.domIntlAppl() == _domIntlAppl || sortData.domIntlAppl() == ' '))
    return true;

  return false;
}

//------------------------------------------------------
// GroupingDataRetriever::getGroupType()
//------------------------------------------------------
Group::GroupType
GroupingDataRetriever::getGroupType(const FareDisplaySort& sortOption, Group& group)
{
  if (sortOption.sortByGlobalDir() == Group::APPLY_PRIORITY_LIST)
  {
    group.sortType() = sortOption.sortByGlobalDir();

    if (group.comparator() != nullptr)
    {
      group.comparator()->prepare(_trx);
    }

    return Group::GROUP_BY_GLOBAL_DIR;
  }
  else if (sortOption.sortByRouting() == Group::ASCENDING ||
           sortOption.sortByRouting() == Group::DESCENDING)
  {
    group.sortType() = sortOption.sortByRouting();
    return Group::GROUP_BY_ROUTING;
  }
  else if (sortOption.sortByNMLSpecial() == Group::NORMAL_OVER_SPECIAL ||
           sortOption.sortByNMLSpecial() == Group::SPECIAL_OVER_NORMAL)
  {
    group.sortType() = sortOption.sortByNMLSpecial();
    return Group::GROUP_BY_NORMAL_SPECIAL;
  }
  else if (sortOption.sortByOWRT() == Group::ONE_WAY_OVER_ROUND_TRIP ||
           sortOption.sortByOWRT() == Group::ROUND_TRIP_OVER_ONE_WAY)
  {
    group.sortType() = sortOption.sortByOWRT();
    return Group::GROUP_BY_OW_RT;
  }
  else if (sortOption.sortByFareBasis() == YES)
  {
    group.sortType() = sortOption.sortByFareBasis();
    return Group::GROUP_BY_FARE_BASIS_CODE;
  }
  else if (sortOption.sortByFareBasisCharComb() == YES)
  {
    group.sortType() = sortOption.sortByFareBasisCharComb();
    return Group::GROUP_BY_FARE_BASIS_CHAR_COMB;
  }
  else if (sortOption.sortByPsgType() == Group::APPLY_PRIORITY_LIST ||
           sortOption.sortByPsgType() == Group::ASCENDING ||
           sortOption.sortByPsgType() == Group::DESCENDING)
  {
    group.sortType() = sortOption.sortByPsgType();
    return Group::GROUP_BY_PSG_TYPE;
  }
  else if (sortOption.sortByPublicPrivate() == Group::PUBLIC_OVER_PRIVATE ||
           sortOption.sortByPublicPrivate() == Group::PRIVATE_OVER_PUBLIC)
  {
    group.sortType() = sortOption.sortByPublicPrivate();
    return Group::GROUP_BY_PUBLIC_PRIVATE;
  }
  else if (sortOption.sortByAmount() == Group::ASCENDING ||
           sortOption.sortByAmount() == Group::DESCENDING)
  {
    group.sortType() = sortOption.sortByAmount();
    return Group::GROUP_BY_FARE_AMOUNT;
  }
  else if (sortOption.sortByExpireDate() == Group::ASCENDING ||
           sortOption.sortByExpireDate() == Group::DESCENDING)
  {
    group.sortType() = sortOption.sortByExpireDate();
    return Group::GROUP_BY_TRAVEL_DISCONTINUE_DATE;
  }

  return Group::GROUP_NOT_REQUIRED;
}

void
GroupingDataRetriever::setHeader(std::vector<Group*>& groups)
{
  GroupHeader header(_trx);
  header.setGroupHeaderInfo(groups);
}
bool
GroupingDataRetriever::retrieveData(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const TJRGroup& tjrGroup)
{
  const std::vector<FareDisplaySort*>& sortData = _trx.dataHandle().getFareDisplaySort(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup, _trx.travelDate());
  if (sortData.empty())
    return false;
  _parentTableData = sortData;
  return !_parentTableData.empty();
}
}
