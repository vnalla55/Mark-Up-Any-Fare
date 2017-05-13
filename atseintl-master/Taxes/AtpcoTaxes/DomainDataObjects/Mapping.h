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

#include "DomainDataObjects/Map.h"

namespace tax
{
class Mapping
{
public:
  Mapping(void);
  ~Mapping(void);

  std::vector<Map>& maps() { return _maps; }

  const std::vector<Map>& maps() const { return _maps; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  std::vector<Map> _maps;
};

} // namespace tax
