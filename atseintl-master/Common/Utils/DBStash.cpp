//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------


#include "Common/Utils/DBStash.h"
#include "Common/Utils/ShadowVector.h"
#include "DBAccess/Record2Types.h"


namespace tse
{

namespace tools
{

template<> SharingSet<CategoryRuleItemInfoSet>
  GetStash<CategoryRuleItemInfoSet>::_stash{};
template<> SharingSet<CombinabilityRuleItemInfoSet>
  GetStash<CombinabilityRuleItemInfoSet>::_stash{};
template<> SharingSet<ShadowVector<CategoryRuleItemInfoSet>>
  GetStash<ShadowVector<CategoryRuleItemInfoSet>>::_stash{};
template<> SharingSet<ShadowVector<CombinabilityRuleItemInfoSet>>
  GetStash<ShadowVector<CombinabilityRuleItemInfoSet>>::_stash{};

} // namespace tools

} // namespace tse
