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

#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/DiagnosticResponse.h"
#include "DomainDataObjects/ErrorMessage.h"

namespace tax
{

struct Response
{
  bool
  isDiagnosticResponse() const
  {
    return (_diagnosticResponse && !_diagnosticResponse->_messages.empty());
  }

  std::string _echoToken;
  boost::optional<ItinsPayments> _itinsPayments;
  boost::optional<DiagnosticResponse> _diagnosticResponse;
  boost::optional<ErrorMessage> _error;
};

} // namespace tax
