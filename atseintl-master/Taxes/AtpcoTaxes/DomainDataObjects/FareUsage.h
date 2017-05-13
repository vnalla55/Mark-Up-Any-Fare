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

#include "DataModel/Common/Types.h"

namespace tax
{
class Fare;

class FareUsage
{
public:
  FareUsage();
  ~FareUsage();

  const type::Index& index() const { return _index; }

  type::Index& index() { return _index; }

  const Fare* fare() const { return _fare; }

  const Fare*& fare() { return _fare; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _index;
  const Fare* _fare;
};
}
