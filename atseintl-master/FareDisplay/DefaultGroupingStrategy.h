//----------------------------------------------------------------------------
//   File : DefaultGroupingStrategy.cpp
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

#include "Common/TsePrimitiveTypes.h"
#include "FareDisplay/GroupingStrategy.h"


#include <set>

namespace tse
{
/**
 *@class DefaultGroupingStrategy.
 * Applies the GroupingStrategy logic for a FareDisplayTrx. Implements grouping only by FareAmount
 *
 */

class DefaultGroupingStrategy : public GroupingStrategy
{
  friend class DefaultGroupingStrategyTest;

public:
  virtual bool apply() override;

private:
  static const Indicator DESCENDING;
  static const Indicator ASCENDING;
};
} // namespace tse

