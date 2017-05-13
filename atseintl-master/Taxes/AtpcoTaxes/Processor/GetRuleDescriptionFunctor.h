// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

namespace tax
{

template <class Rule>
class GetRuleDescriptionFunctor
{
public:
  static bool apply(const Rule& rule,
                    Services& services,
                    std::vector<std::string>& result)
  {
    result.push_back(rule.getDescription(services));

    return true;
  }
};

} /* namespace tax */
