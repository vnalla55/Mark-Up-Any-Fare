#include "DataModel/TaxTrx.h"

#include "Common/Logger.h"
#include "Xform/XMLConvertUtils.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.TaxTrx");

const TaxRequestType OTA_REQUEST = "OTA";
const TaxRequestType DISPLAY_REQUEST = "DISPLAY";
const TaxRequestType PFC_DISPLAY_REQUEST = "PFC_DISPLAY";
const TaxRequestType ATPCO_DISPLAY_REQUEST = "ATPCO_DISPLAY";
const TaxRequestType NON_OTA_REQUEST = "NON_OTA";
const TaxRequestType TAX_INFO_REQUEST = "TAX_INFO";
const TaxRequestType NEW_OTA_REQUEST = "NEW_OTA";

bool
TaxTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  LOG4CXX_DEBUG(logger, "Doing TaxTrx response");
  response = _response.str();
  LOG4CXX_INFO(logger, "response: " << response);

  LOG4CXX_WARN(logger, "Transaction not supported!");
  return false;
}
}
