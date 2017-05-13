//-------------------------------------------------------------------
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
#include "DataModel/AltPricingDetailObFeesTrx.h"

#include "Common/Logger.h"
#include "Xform/XMLConvertUtils.h"
#include "Diagnostic/DiagTools.h"
#include "Xform/XMLConvertUtils.h"

#include <sstream>

namespace tse
{
static Logger
logger("atseintl.DataModel.AltPricingDetailObFeesTrx");
bool
AltPricingDetailObFeesTrx::convert(std::string& response)
{
  std::string tmpResponse;
  XMLConvertUtils::tracking(*this);
  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  Diagnostic& diag = diagnostic();
  if (_fareCalcCollector.empty())
  {
    LOG4CXX_WARN(logger, "Pricing Response Items are Missing");
  }

  tmpResponse = diag.toString();

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

  if (diagnostic().diagnosticType() == Diagnostic870)
  {
    std::string diagTxt = diagnostic().toString();
    if (diagTxt.empty())
      diagTxt = XMLConvertUtils::prepareResponseText(diagTxt);
    diagTxt = XMLConvertUtils::printDiagReturnedNoData(Diagnostic870);
    XMLConvertUtils::wrapWpnRespWitnMainTag(diagTxt);
    response = diagTxt;
  }
  else
  {
    response = XMLConvertUtils::formatWpanDetailResponse<AltPricingDetailObFeesTrx>(this);
  }

  return true;
}
}
