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

#include <stdint.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include "DataModel/RequestResponse/InputParameter.h"

namespace tax
{

class InputDiagnosticCommand
{
public:
  InputDiagnosticCommand(void) : _number(0) {}

  uint32_t& number() { return _number; }
  const uint32_t& number() const { return _number; }

  boost::ptr_vector<InputParameter>& parameters() { return _parameters; }
  const boost::ptr_vector<InputParameter>& parameters() const { return _parameters; }

  uint32_t _number;
  boost::ptr_vector<InputParameter> _parameters;
};

} // namespace tax
