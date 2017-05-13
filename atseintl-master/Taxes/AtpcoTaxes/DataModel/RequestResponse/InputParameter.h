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

#include <string>

namespace tax
{

class InputParameter
{
public:
  std::string& name() { return _name; }

  const std::string& name() const { return _name; }

  std::string& value() { return _value; }

  const std::string& value() const { return _value; }

private:
  std::string _name;
  std::string _value;
};
}
