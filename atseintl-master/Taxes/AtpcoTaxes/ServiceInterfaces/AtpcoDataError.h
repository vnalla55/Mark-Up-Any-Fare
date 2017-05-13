// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include <stdexcept>

namespace tax
{

class AtpcoDataError : public std::runtime_error
{
  typedef std::runtime_error Super;

public:
  AtpcoDataError(const std::string& cause) : Super(cause) {}
  AtpcoDataError(const char* cause) : Super(cause) {}
};

} // namespace tax

