//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Pricing/GroupsFilter.h"

#include "DataModel/FlexFares/AttrComplexStats.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "Common/FallbackUtil.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackFFGAcctCodeCorpIDFilterFix);
namespace flexFares
{

template <>
void
GroupsFilter::filterOut<flexFares::ACC_CODES>(const ValidationStatusPtr status, GroupsIds& result)
    const
{
  if (result.empty())
    return;
  filterOutComplexAttr<flexFares::ACC_CODES>(status, result);
}

template <>
void
GroupsFilter::filterOut<flexFares::CORP_IDS>(const ValidationStatusPtr status, GroupsIds& result)
    const
{
  if (result.empty())
    return;
  filterOutComplexAttr<flexFares::CORP_IDS>(status, result);
}

// Default behavior, not applicable to CORP_IDS and ACC_CODES
template <Attribute attribute>
void
GroupsFilter::filterOut(const ValidationStatusPtr status, GroupsIds& result) const
{
  if (result.empty())
    return;
  if (!isValid(status->getStatusForAttribute<attribute>()))
    removeInvalidGroups(_totalAttrs.getAllGroups<attribute>(), result);
}

template <Attribute attribute>
void
GroupsFilter::filterOutComplexAttr(const ValidationStatusPtr status, GroupsIds& result) const
{
  const flexFares::AttrComplexStats<std::string>& groups = _totalAttrs.getAllGroups<attribute>();

  GroupsIds neededIds;
  groups.getAllGroupsIds(neededIds); // all groups that need attribute validation

  GroupsIds validIds;

  // collect valid ids, i.e. groups for which at least one attrVal is matched
  for (const std::string& attrVal : status->getStatusForAttribute<attribute>())
  {
    const GroupsIds& validGroups = groups.getValidGroups(attrVal);
    validIds.insert(validGroups.begin(), validGroups.end());
  }

  // A little bit of set theory :)
  // Let:
  //   N - set of ids of groups that need matching(neededIds in code)
  //   V - set of valid groups(validIds)
  //   R - result
  //   X - set of all possible group ids (e.g. all non-negative integers)
  //   I - invalid groups, i.e. N\V (needed minus valid)
  //
  // we want to filter out invalid groups from R, it means
  // R := R\I
  //    = R\(N\V)
  //    = R\(N intersect (X\V))
  //    = R\N union R\(X\V)
  //    = R\N union (R intersect V)

  // in result vector, each element of initial result may occur at most 2 times
  std::vector<GroupId> resultVec(2 * result.size());
  std::vector<GroupId>::iterator it1, it2;

  // put R\N into resultVec
  it1 = std::set_difference(
      result.begin(), result.end(), neededIds.begin(), neededIds.end(), resultVec.begin());

  // add (R intersect V)
  it2 = std::set_intersection(result.begin(), result.end(), validIds.begin(), validIds.end(), it1);

  // put resultVec into the set;
  // note that if there are some duplicates in resultVec, they will be added only once,
  // since we insert them into set
  result.clear();
  result.insert(resultVec.begin(), it2);
}

void
GroupsFilter::removeInvalidGroups(const GroupsIds& invalidGroups, GroupsIds& result) const
{
  std::vector<GroupId> validGroups(result.size());
  std::vector<GroupId>::iterator diffEnd = std::set_difference(result.begin(),
                                                               result.end(),
                                                               invalidGroups.begin(),
                                                               invalidGroups.end(),
                                                               validGroups.begin());

  result.clear();
  result.insert(validGroups.begin(), diffEnd);
}

void
GroupsFilter::filterOutInvalidGroups(const ValidationStatusPtr status, GroupsIds& result) const
{
  filterOut<flexFares::CORP_IDS>(status, result);
  filterOut<flexFares::ACC_CODES>(status, result);
  filterOut<flexFares::PUBLIC_FARES>(status, result);
  filterOut<flexFares::PRIVATE_FARES>(status, result);
  filterOut<flexFares::NO_ADVANCE_PURCHASE>(status, result);
  filterOut<flexFares::NO_PENALTIES>(status, result);
  filterOut<flexFares::NO_MIN_MAX_STAY>(status, result);
}

template <>
bool
GroupsFilter::filterOut<flexFares::ACC_CODES>(const ValidationStatusPtr status, GroupId& result)
{
  return filterOutComplexAttr<flexFares::ACC_CODES>(status, result);
}

template <>
bool
GroupsFilter::filterOut<flexFares::CORP_IDS>(const ValidationStatusPtr status, GroupId& result)
{
  return filterOutComplexAttr<flexFares::CORP_IDS>(status, result);
}

// Default behavior, not applicable to CORP_IDS and ACC_CODES
template <Attribute attribute>
bool
GroupsFilter::filterOut(const ValidationStatusPtr status, GroupId& result)
{
  if (!isValidNew(status->getStatusForAttribute<attribute>()))
  {
    const GroupsIds& validGroups = _totalAttrs.getAllGroups<attribute>();

    return (validGroups.find(result) == validGroups.end());
  }

  return true;
}

template <Attribute attribute>
bool
GroupsFilter::filterOutComplexAttr(const ValidationStatusPtr status, GroupId& result)
{
  const flexFares::AttrComplexStats<std::string>& groups = _totalAttrs.getAllGroups<attribute>();
  bool noAccCodeCorpId = true;

  // collect valid ids, i.e. groups for which at least one attrVal is matched
  for (const std::string& attrVal : status->getStatusForAttribute<attribute>())
  {
    const GroupsIds& validGroups = groups.getValidGroups(attrVal);
    noAccCodeCorpId = false;

    if (validGroups.find(result) != validGroups.end())
      return true;
  }

  return noAccCodeCorpId;
}

bool
GroupsFilter::filterOutInvalidGroups(const ValidationStatusPtr status, GroupId& result)
{
  if (fallback::fixed::fallbackFFGAcctCodeCorpIDFilterFix())
    return filterOut<flexFares::CORP_IDS>(status, result) &&
           filterOut<flexFares::ACC_CODES>(status, result) &&
           filterOut<flexFares::PUBLIC_FARES>(status, result) &&
           filterOut<flexFares::PRIVATE_FARES>(status, result) &&
           filterOut<flexFares::NO_ADVANCE_PURCHASE>(status, result) &&
           filterOut<flexFares::NO_PENALTIES>(status, result) &&
           filterOut<flexFares::NO_MIN_MAX_STAY>(status, result);
  else
    return (filterOut<flexFares::CORP_IDS>(status, result) ||
           filterOut<flexFares::ACC_CODES>(status, result)) &&
           filterOut<flexFares::PUBLIC_FARES>(status, result) &&
           filterOut<flexFares::PRIVATE_FARES>(status, result) &&
           filterOut<flexFares::NO_ADVANCE_PURCHASE>(status, result) &&
           filterOut<flexFares::NO_PENALTIES>(status, result) &&
           filterOut<flexFares::NO_MIN_MAX_STAY>(status, result);
}

}
}
