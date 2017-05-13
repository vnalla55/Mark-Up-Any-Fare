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

#include <boost/optional.hpp>

#include "DataModel/RequestResponse/OutputItins.h"
#include "DataModel/RequestResponse/OutputDiagnostic.h"
#include "DataModel/RequestResponse/OutputError.h"

namespace tax
{

class OutputResponse
{
public:
  OutputResponse() {};

  const std::string& echoToken() const { return _echoToken; }
  std::string& echoToken() { return _echoToken; }

  const boost::optional<OutputItins>& itins() const { return _itins; }
  boost::optional<OutputItins>& itins() { return _itins; }

  const boost::optional<OutputDiagnostic>& diagnostic() const { return _diagnostic; }
  boost::optional<OutputDiagnostic>& diagnostic() { return _diagnostic; }

  const boost::optional<OutputError>& error() const { return _error; }
  boost::optional<OutputError>& error() { return _error; }

private:
  OutputResponse& operator==(const OutputResponse&);

  std::string _echoToken;
  boost::optional<OutputItins> _itins;
  boost::optional<OutputDiagnostic> _diagnostic;
  boost::optional<OutputError> _error;
};

} // namespace tax
