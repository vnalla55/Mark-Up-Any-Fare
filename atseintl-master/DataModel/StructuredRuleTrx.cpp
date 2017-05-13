//-------------------------------------------------------------------
//
//  Authors:     Kacper Stapor
//
//  Copyright Sabre 2015
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

#include "DataModel/StructuredRuleTrx.h"

#include "Common/Logger.h"
#include "Diagnostic/DiagTools.h"
#include "Service/Service.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/StructuredRulesResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

#include <memory>
namespace tse
{

static Logger
logger("atseintl.DataModel.StructuredRuleTrx");

bool
StructuredRuleTrx::process(Service& srv)
{
  return srv.process(*this);
}

void
StructuredRuleTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }
  StructuredRulesResponseFormatter::formatResponse(ere, response);
}

bool
StructuredRuleTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");

  Diagnostic& diag = diagnostic();
  if (_fareCalcCollector.empty())
  {
    LOG4CXX_WARN(logger, "Pricing Response Items are Missing");
  }

  FareCalcCollector* fareCalcCollector = nullptr;
  if ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
      !_fareCalcCollector.empty())
  {
    fareCalcCollector = _fareCalcCollector.front();
  }

  std::string tmpResponse = diag.toString();

  if (diag.diagnosticType() == Diagnostic854 &&
      (diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS) ||
       diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS)))
  {
    tmpResponse += "\n\n" + utils::getMetricsInfo(*this);
  }

  if (diag.diagnosticType() != DiagnosticNone && tmpResponse.length() == 0)
  {
    char tmpBuf[512];
    sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
    tmpResponse.insert(0, tmpBuf);
  }

  StructuredRulesResponseFormatter formatter;
  response = formatter.formatResponse(tmpResponse, displayOnly(), *this, fareCalcCollector);

  return true;
}

void
StructuredRuleTrx::setupFootNotePrevalidation()
{
  _footNotePrevalidationAllowed = false; // Do not prevalidate for Structured Fare Rules
}

bool
StructuredRuleTrx::isMultiPassengerSFRRequestType() const
{
  return _isMultiPassengerSFRRequest;
}

} // tse
