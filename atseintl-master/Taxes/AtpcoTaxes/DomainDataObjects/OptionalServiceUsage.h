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

#include "DataModel/Common/Types.h"

namespace tax
{
class OptionalService;

class OptionalServiceUsage
{
public:
  OptionalServiceUsage();
  ~OptionalServiceUsage();

  const type::Index& index() const { return _index; }

  type::Index& index() { return _index; }

  const OptionalService* optionalService() const { return _optionalService; }

  OptionalService*& optionalService() { return _optionalService; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _index;
  OptionalService* _optionalService;
};
} // namespace tax
