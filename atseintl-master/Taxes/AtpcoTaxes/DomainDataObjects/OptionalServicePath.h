// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <vector>

#include "DomainDataObjects/OptionalServiceUsage.h"

namespace tax
{

class OptionalServicePath
{
public:
  OptionalServicePath(void);
  ~OptionalServicePath(void);

  std::vector<OptionalServiceUsage>& optionalServiceUsages()
  {
    return _optionalServiceUsages;
  };

  std::vector<OptionalServiceUsage> const& optionalServiceUsages() const
  {
    return _optionalServiceUsages;
  };

  type::Index& index() { return _index; }

  type::Index const& index() const { return _index; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  std::vector<OptionalServiceUsage> _optionalServiceUsages;
  type::Index _index;
};
} // namespace tax
