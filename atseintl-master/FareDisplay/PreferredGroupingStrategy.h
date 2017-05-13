//----------------------------------------------------------------------------
//   File : PreferredGroupingStrategy.cpp
//   Author: Abu
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "FareDisplay/GroupingStrategy.h"


namespace tse
{
/**
 *@class PreferredGroupByRouting.
 *Applies the GroupingStrategy logic as per user preference retrieved from FareDisplaySort Table.
 *
 */

class PreferredGroupingStrategy : public GroupingStrategy
{
  friend class PreferredGroupingStrategyTest;

public:
  PreferredGroupingStrategy();
  virtual ~PreferredGroupingStrategy();

  /**
   * Implements the apply interface for GroupingStrategy.
   */

  virtual bool apply() override;

private:
  bool hasGroup(Group::GroupType grpType);
  void reorderGroup(Group::GroupType grpType);
  bool isProcessPsgGroupingRqst();
  bool isApplyMultiTransportGrouping();

  struct EqualByGrpType : public std::unary_function<Group, bool>
  {
    EqualByGrpType(Group::GroupType type) : _type(type) {}
    ~EqualByGrpType() {}

  public:
    bool operator()(const Group* g) const
    {
      if (g == nullptr)
        return false;
      else
        return g->groupType() == _type;
    }

  private:
    Group::GroupType _type;
  };
};

} // namespace tse

