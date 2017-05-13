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
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/Types.h"

namespace tse
{
namespace flexFares
{

class GroupsDataAnalyzer
{
public:
  static void analyzeGroupsAndPutResultsTo(const GroupsData& data, TotalAttrs& storage)
  {
    for (const GroupsData::value_type& groupAttrs : data)
    {
      analyzeGroupAndPutResultsTo(groupAttrs.first, groupAttrs.second, storage);
    }
  }

private:
  static void analyzeGroupAndPutResultsTo(const GroupId index,
                                          const GroupAttrs& groupAttrs,
                                          TotalAttrs& storage);
};

} // flexFares

} // tse

