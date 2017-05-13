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
#include <stdexcept>

namespace tax
{
class XmlParsingError : public std::runtime_error
{
public:
  XmlParsingError(const std::string& w) : runtime_error(w) {}
};
}
