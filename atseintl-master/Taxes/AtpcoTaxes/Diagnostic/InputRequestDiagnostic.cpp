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
#include "AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "AtpcoTaxes/DomainDataObjects/DiagnosticCommand.h"
#include "DomainDataObjects/DiagnosticResponse.h"
#include "Diagnostic/InputRequestDiagnostic.h"
#include "Diagnostic/AtpcoDiagnostic.h"
#include "TestServer/Server/InputRequestWithCache.h"
#include "TestServer/Xform/XmlWriter.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"

namespace tax
{

const uint32_t InputRequestDiagnostic::NUMBER;

InputRequestDiagnostic::Match InputRequestDiagnostic::match(const InputRequest& request)
{
  const InputDiagnosticCommand& diag = request.diagnostic();
  if (diag.number() == NUMBER)
  {
    for (const InputParameter& param : diag.parameters())
    {
      if (param.name() == "XM")
      {
        if (param.value() == "L")
          return Match::RequestOnly;
        else if (param.value() == "LC")
          return Match::RequestWithCache;
      }
    }
  }

  return Match::No;
}

InputRequestDiagnostic::Match InputRequestDiagnostic::match(const DiagnosticCommand& diag)
{
  if (diag.number() == NUMBER)
  {
    for (const Parameter& param : diag.parameters())
    {
      if (param.name() == "XM")
      {
        if (param.value() == "L")
          return Match::RequestOnly;
        else if (param.value() == "LC")
          return Match::RequestWithCache;
      }
    }
  }

  return Match::No;
}

std::string InputRequestDiagnostic::makeContent(const InputRequest& request)
{
  std::ostringstream here;
  writeXmlRequest(here, request, NaturalXmlTagsList());
  return here.str();
}

std::string InputRequestDiagnostic::makeContent(const InputRequest& request, const std::string& cache)
{
  std::ostringstream here;
  writeXmlRequest(here, request, cache, NaturalXmlTagsList());
  return here.str();
}


} // namespace tax
