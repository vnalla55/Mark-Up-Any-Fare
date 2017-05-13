#include "DataModel/AltPricingDetailTrx.h"

#include "Common/Logger.h"
#include "Diagnostic/DiagTools.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.AltPricingDetailTrx");
bool
AltPricingDetailTrx::convert(std::string& response)
{
  std::string tmpResponse;
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

  response = XMLConvertUtils::formatWpanDetailResponse<AltPricingDetailTrx>(this);

  return true;
}
}
