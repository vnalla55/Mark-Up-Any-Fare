//-------------------------------------------------------------------
//
//  File:         Response.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{

class Response
{
public:
  virtual ~Response() = default;

  std::string& output() { return _output; }
  const std::string& output() const { return _output; }

private:
  std::string _output;
};
} // tse namespace
