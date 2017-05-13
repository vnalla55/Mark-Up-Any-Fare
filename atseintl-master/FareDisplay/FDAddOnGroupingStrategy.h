//----------------------------------------------------------------------------
//  Author: Abu, Partha
//	Copyright Sabre 2005
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

#include "DBAccess/FDAddOnFareInfo.h"
#include "FareDisplay/GroupingStrategy.h"


#include <vector>

namespace tse
{
/**
 *@class GroupByRouting.
 * Applies the GroupingStrategy logic for a FareDisplayTrx.
 *
 */

class FDAddOnGroupingStrategy : public GroupingStrategy
{
  friend class FDAddOnGroupingStrategyTest;

public:
  FDAddOnGroupingStrategy(std::vector<FDAddOnFareInfo*>& addonFares);

  virtual bool apply() override;
  std::map<GlobalDirection, uint16_t> _globalGroup;

private:
  std::vector<FDAddOnFareInfo*>& _addonFares;

  //--------------------------------------------------------------
  // LessByGlobal
  //-------------------------------------------------------------

  class LessByGlobal : public std::binary_function<FDAddOnFareInfo, FDAddOnFareInfo, bool>
  {
  public:
    LessByGlobal(std::map<GlobalDirection, uint16_t>& priorityMap) : _priorityMap(priorityMap) {}
    bool operator()(const FDAddOnFareInfo* l, const FDAddOnFareInfo* r) const
    {
      if (l == nullptr)
        return true;
      if (r == nullptr)
        return false;

      if (globalPriority(l->globalDir()) < globalPriority(r->globalDir()))
        return true;

      if (globalPriority(l->globalDir()) > globalPriority(r->globalDir()))
        return false;

      if (l->routing() < r->routing())
        return true;

      return false;
    }

  private:
    std::map<GlobalDirection, uint16_t>& _priorityMap;
    uint16_t globalPriority(const GlobalDirection& gd) const
    {
      std::map<GlobalDirection, uint16_t>::iterator itr(_priorityMap.end());
      itr = _priorityMap.find(gd);
      if (itr != _priorityMap.end())
      {
        return itr->second;
      }
      return (_priorityMap.size() + 1);
    }
  };
};
} // namespace tse

