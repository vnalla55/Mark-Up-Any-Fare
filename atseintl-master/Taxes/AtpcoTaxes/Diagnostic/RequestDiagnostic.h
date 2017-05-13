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
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{
class Request;
class OptionalService;

class RequestDiagnostic : public AtpcoDiagnostic
{
  typedef boost::function<bool(const OptionalService&)> CheckOptionalServiceType;

public:
  static const uint32_t NUMBER;

  RequestDiagnostic(const Request& taxRequest, const boost::ptr_vector<Parameter>& parameters);
  ~RequestDiagnostic();

private:
  virtual void runAll() override;
  virtual void printHeader() override;
  virtual void applyParameters() override;

  void printHelp();
  void printTaxPoints();
  void printFarePaths();
  void printOptionalServices();
  void printBaggage();
  void printRequest();
  void printSelectedOS(const std::string& header, const type::ProcessingGroup& processingGroup);

public:
  Printer<RequestDiagnostic, &RequestDiagnostic::printFarePaths>& farePathsPrinter()
  {
    return _farePathsPrinter;
  }

  Printer<RequestDiagnostic, &RequestDiagnostic::printTaxPoints>& taxPointsPrinter()
  {
    return _taxPointsPrinter;
  }

  Printer<RequestDiagnostic, &RequestDiagnostic::printOptionalServices>& optionalServicesPrinter()
  {
    return _optionalServicesPrinter;
  }

  Printer<RequestDiagnostic, &RequestDiagnostic::printBaggage>& baggagePrinter()
  {
    return _baggagePrinter;
  }

  Printer<RequestDiagnostic, &RequestDiagnostic::printRequest>& requestPrinter()
  {
    return _requestPrinter;
  }

private:
  const Request& _taxRequest;
  const boost::ptr_vector<Parameter>& _parameters;

  Printer<RequestDiagnostic, &RequestDiagnostic::printHelp> _helpPrinter;
  Printer<RequestDiagnostic, &RequestDiagnostic::printTaxPoints> _taxPointsPrinter;
  Printer<RequestDiagnostic, &RequestDiagnostic::printFarePaths> _farePathsPrinter;
  Printer<RequestDiagnostic, &RequestDiagnostic::printOptionalServices> _optionalServicesPrinter;
  Printer<RequestDiagnostic, &RequestDiagnostic::printBaggage> _baggagePrinter;
  Printer<RequestDiagnostic, &RequestDiagnostic::printRequest> _requestPrinter;
};
}
