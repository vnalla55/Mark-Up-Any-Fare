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

#include "DataModel/FlexFares/TotalAttrs.h"

#include "Common/Assert.h"
#include "Common/CabinType.h"
#include "Common/TseConsts.h"
#include "DataModel/FlexFares/Types.h"

namespace tse
{
namespace flexFares
{

template <>
void
TotalAttrs::addGroup<CORP_IDS>(const GroupId index)
{
  TSE_ASSERT(false);
}

template <>
void
TotalAttrs::addGroup<ACC_CODES>(const GroupId index)
{
  TSE_ASSERT(false);
}

template <>
bool
TotalAttrs::areAllGroupsValid<CORP_IDS>(const GroupsIds& validGroups) const
{
  TSE_ASSERT(false);
  return false;
}

template <>
bool
TotalAttrs::areAllGroupsValid<ACC_CODES>(const GroupsIds& validGroups) const
{
  TSE_ASSERT(false);
  return false;
}

} // flexFares
} // tse
