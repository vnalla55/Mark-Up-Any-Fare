#include "DataModel/PricingDetailTrx.h"

#include "Common/Logger.h"
#include "Xform/PricingDetailResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"

#include <utility>
namespace tse
{
static Logger
logger("atseintl.DataModel.PricingDetailTrx");

void
PricingDetailTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  _response << ere.message();
  PricingDetailResponseFormatter formatter;
  response = formatter.formatResponse(*this);
}
bool
PricingDetailTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }
  std::string xmlResponse;
  if (wpnTrx())
  {
    xmlResponse = XMLConvertUtils::formatWpnResponse(*this);
    LOG4CXX_INFO(logger, "response: " << response);
  }
  else
  {
    PricingDetailResponseFormatter pricingDetailFormatter;
    xmlResponse = pricingDetailFormatter.formatResponse(*this);
  }
  response = std::move(xmlResponse);
  LOG4CXX_INFO(logger, "response: " << xmlResponse);

  return true;
}
}
