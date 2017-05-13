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
#include <string>

namespace tax
{

class InputRequest;
class DiagnosticCommand;

struct InputRequestDiagnostic
{
  static const uint32_t NUMBER = 830;
  enum class Match { No, RequestOnly, RequestWithCache };

  static Match match(const InputRequest& request);
  static Match match(const DiagnosticCommand& diag);
  static std::string makeContent(const InputRequest& request);
  static std::string makeContent(const InputRequest& request, const std::string& cache);
};

}

