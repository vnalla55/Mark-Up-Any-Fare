// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/Response/ResponseFormatter.h"

namespace tax
{
namespace display
{

class View
{
public:
  virtual bool header() { return true; }
  virtual bool body() { return true; }
  virtual bool footer() { return true; }

  virtual ~View() = default;

protected:
  View(ResponseFormatter& responseFormatter)
    : _formatter(responseFormatter) {}

  ResponseFormatter& _formatter;
};

} // namespace display
} // namespace tax
