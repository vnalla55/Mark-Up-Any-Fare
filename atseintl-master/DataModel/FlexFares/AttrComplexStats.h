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

#pragma once

#include "Common/CabinType.h"
#include "Common/TseConsts.h"
#include "DataModel/FlexFares/Types.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace tse
{

namespace flexFares
{
/**
 * template<typename AttrType>
 * class AttrComplexStats
 *
 * For some flex fare attribute of type AttrType
 * this class supports three operations:
 * 1) remeber that if group with given index is related to given attribute value
 * 2) give all groups related to given attribute value
 * 3) iterate through all values of some attribute
 *    ordered by decreasing number of related groups
 */
template <typename AttrType>
class AttrComplexStats
{
public:
  void insert(const AttrType& key, const flexFares::GroupId index)
  {
    GroupUsageInfoMapIterator keyIt = _groupUsageInfoMap.find(key);
    if (keyIt == _groupUsageInfoMap.end())
    {
      std::pair<GroupUsageInfoMapIterator, bool> insertResult =
          _groupUsageInfoMap.insert(GroupUsageInfoMapValue(key, GroupsIdsPtr(new GroupsIds())));

      keyIt = insertResult.first;
    }
    // At this point we have an iterator keyIt with a valid set of groups ids.

    // Now we remove existing entry in sorted set,
    // because after adding new index to it, the order may be invalid.
    GroupUsageInfoSetValue setElement(key, keyIt->second);
    GroupUsageInfoSetIterator setIterator = _groupUsageInfoSet.find(setElement);
    if (setIterator != _groupUsageInfoSet.end())
      _groupUsageInfoSet.erase(setIterator);

    keyIt->second->insert(index);
    // Insert the entry for this attribute value back.
    // This time with possibly enlarged set, so the order will be ok.
    _groupUsageInfoSet.insert(setElement);
  }

  bool areAllGroupsValid(const AttrType& key, const GroupsIds& validGroups) const
  {
    std::vector<flexFares::GroupId> notValidatedGroups;
    const GroupUsageInfoMapConstIterator relatedGroups = _groupUsageInfoMap.find(key);
    // we don't need to validate this key
    if (relatedGroups == _groupUsageInfoMap.end())
      return true;

    std::set_difference(relatedGroups->second->begin(),
                        relatedGroups->second->end(),
                        validGroups.begin(),
                        validGroups.end(),
                        notValidatedGroups.begin());

    return notValidatedGroups.empty();
  }

  void getAllGroupsIds(GroupsIds& result) const
  {
    for (const GroupUsageInfoMapValue& groupUsage : _groupUsageInfoMap)
    {
      result.insert(groupUsage.second->begin(), groupUsage.second->end());
    }
  }

  const GroupsIds& getValidGroups(const AttrType& key) const
  {
    const GroupUsageInfoMapConstIterator findIt = _groupUsageInfoMap.find(key);
    if (findIt == _groupUsageInfoMap.end())
      return _emptySet;

    return *findIt->second;
  }

private:
  typedef std::shared_ptr<GroupsIds> GroupsIdsPtr;
  typedef typename std::pair<AttrType, GroupsIdsPtr> GroupUsageInfo;

  struct GroupUsageInfoComp
  {
    bool operator()(const GroupUsageInfo& left, const GroupUsageInfo& right) const
    {
      if (!left.second || !right.second)
        return left.first > right.first;

      if (left.second->size() != right.second->size())
        return left.second->size() > right.second->size();

      return left.first > right.first;
    }
  };

  typedef std::set<GroupUsageInfo, GroupUsageInfoComp> GroupUsageInfoSet;
  typedef typename std::map<AttrType, GroupsIdsPtr> GroupUsageInfoMap;

  typedef typename GroupUsageInfoMap::iterator GroupUsageInfoMapIterator;
  typedef typename GroupUsageInfoMap::const_iterator GroupUsageInfoMapConstIterator;
  typedef typename GroupUsageInfoMap::value_type GroupUsageInfoMapValue;

  typedef typename GroupUsageInfoSet::iterator GroupUsageInfoSetIterator;
  typedef typename GroupUsageInfoSet::const_iterator GroupUsageInfoSetConstIterator;
  typedef typename GroupUsageInfoSet::value_type GroupUsageInfoSetValue;

  // In _groupUsageInfoSet we store pairs (AttrType,SetOfRelatedGroupsIds)
  // sorted by decreasing cardinality of the second element.
  // The set is represented by the shared_ptr to actual structure.
  GroupUsageInfoSet _groupUsageInfoSet;

  // In _groupUsageInfoMap we store info about ids of groups
  // related to particular attribute values.
  // This is to achieve quick response for requirement 2)
  GroupUsageInfoMap _groupUsageInfoMap;

  GroupsIds _emptySet;

public:
  typedef GroupUsageInfoSetConstIterator const_iterator;
  const_iterator begin() const { return _groupUsageInfoSet.begin(); }
  const_iterator end() const { return _groupUsageInfoSet.end(); }
  typedef GroupUsageInfoSetValue value_type;
  bool empty() const { return _groupUsageInfoMap.empty(); }
};

} // flexFares

} // tse

