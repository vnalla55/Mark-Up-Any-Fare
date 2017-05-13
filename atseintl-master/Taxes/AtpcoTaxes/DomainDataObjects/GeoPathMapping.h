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

#include "DomainDataObjects/Mapping.h"

namespace tax
{
class GeoPathMapping
{
public:
  GeoPathMapping(void);
  ~GeoPathMapping(void);

  std::vector<Mapping>& mappings() { return _mappings; }

  const std::vector<Mapping>& mappings() const { return _mappings; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  std::vector<Mapping> _mappings;
};
}
