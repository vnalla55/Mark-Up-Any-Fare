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
#include <sstream>
#include <stdexcept>

namespace boost
{
void
assertion_failed(char const* expr, char const* function, char const* file, long line)
{
  std::stringstream fatalError;
  fatalError << "FATAL ERROR \"" << expr << "\""
             << " in " << file << ":" << line << "::" << function;
  throw std::runtime_error(fatalError.str());
}
}
