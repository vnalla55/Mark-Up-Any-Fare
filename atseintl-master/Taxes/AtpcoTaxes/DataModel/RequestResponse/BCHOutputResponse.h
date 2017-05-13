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

#include "DataModel/RequestResponse/BCHOutputItin.h"
#include "DataModel/RequestResponse/BCHOutputTaxDetail.h"
#include "DataModel/RequestResponse/OutputDiagnostic.h"
#include "DataModel/RequestResponse/OutputError.h"

#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace tax
{

class BCHOutputResponse
{
public:
  BCHOutputResponse() {};

  BCHOutputResponse& operator=(const BCHOutputResponse&) = delete;

  boost::ptr_vector<BCHOutputTaxDetail>& mutableTaxDetails() { return _taxDetail; }
  const boost::ptr_vector<BCHOutputTaxDetail>& constTaxDetails() const { return _taxDetail; }

  boost::ptr_vector<BCHOutputItin>& mutableItins() { return _itins; }
  const boost::ptr_vector<BCHOutputItin>& constItins() const { return _itins; }

  boost::optional<OutputDiagnostic>& mutableDiagnostic() { return _diagnostic; }
  const boost::optional<OutputDiagnostic>& constDiagnostic() const { return _diagnostic; }

  boost::optional<OutputError>& mutableError() { return _error; }
  const boost::optional<OutputError>& constError() const { return _error; }

private:
  boost::ptr_vector<BCHOutputTaxDetail> _taxDetail;
  boost::ptr_vector<BCHOutputItin> _itins;
  boost::optional<OutputDiagnostic> _diagnostic;
  boost::optional<OutputError> _error;
};

} // namespace tax
