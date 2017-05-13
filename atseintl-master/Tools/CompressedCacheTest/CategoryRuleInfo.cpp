//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include "CategoryRuleInfo.h"

namespace tse
{
  void CategoryRuleInfo::init()
  {
    _hasCatStopovers = init(8);
    _hasCatTransfers = init(9);
  }

  bool CategoryRuleInfo::init(unsigned int category)
  {
    std::vector<CategoryRuleItemInfoSet *>::const_iterator setIt(_categoryRuleItemInfoSet.begin()),
                                                           setItEnd(_categoryRuleItemInfoSet.end());
    for ( ; setIt != setItEnd; ++setIt)
    {
      std::vector<CategoryRuleItemInfo *>::const_iterator infoIt((*setIt)->categoryRuleItemInfo().begin()),
                                                          infoItEnd((*setIt)->categoryRuleItemInfo().end());
      for ( ; infoIt != infoItEnd; ++infoIt)
      {
        if (category == (*infoIt)->itemcat())
        {
	        return true;
	      }
      }
    }
    return false;
  }
}// tse
