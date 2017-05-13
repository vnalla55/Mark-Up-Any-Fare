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

#include <boost/ptr_container/ptr_vector.hpp>
#include <stdint.h>
#include "DomainDataObjects/Parameter.h"

namespace tax
{

class DiagnosticCommand
{
public:
  DiagnosticCommand(void);

  uint32_t& number() { return _number; }
  const uint32_t& number() const { return _number; }

  boost::ptr_vector<Parameter>& parameters() { return _parameters; }
  const boost::ptr_vector<Parameter>& parameters() const { return _parameters; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  uint32_t _number;
  boost::ptr_vector<Parameter> _parameters;
};

} // namespace tax
