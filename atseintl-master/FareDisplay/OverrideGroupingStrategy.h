//----------------------------------------------------------------------------
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
 *@class OverrideGroupByRouting.
 *Applies the GroupingStrategy logic as per user preference retrieved from FareDisplaySort Table.
 *
 */

class OverrideGroupingStrategy : public GroupingStrategy
{
  friend class OverrideGroupingStrategyTest;

public:
  virtual bool apply() override;

private:
  bool initialize();
};

} // namespace tse
