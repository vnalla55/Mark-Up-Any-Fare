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
#include "Rules/RuleValidationChancelor.h"

namespace tse
{
RuleValidationChancelor::~RuleValidationChancelor()
{
  for (const auto& p : _policies)
    delete p.second;
}
}
