// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DomainDataObjects/YqYrUsage.h"

namespace tax
{

class YqYrPath
{
public:
  YqYrPath(void);
  ~YqYrPath(void);

  std::vector<YqYrUsage>& yqYrUsages() { return _yqYrUsages; };

  std::vector<YqYrUsage> const& yqYrUsages() const { return _yqYrUsages; };

  type::MoneyAmount& totalAmount() { return _totalAmount; }

  type::MoneyAmount const& totalAmount() const { return _totalAmount; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  std::vector<YqYrUsage> _yqYrUsages;
  type::MoneyAmount _totalAmount;
};
} // namespace tax
