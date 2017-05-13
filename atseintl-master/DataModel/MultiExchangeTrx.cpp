#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "Xform/PricingResponseFormatter.h"

namespace tse
{
static const std::string ME_RESPONSE_OPEN_TAG = "<RexPricingResponse S96=\"ME\">";
static const std::string ME_RESPONSE_CLOSE_TAG = "</RexPricingResponse>";

void
MultiExchangeTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }

  PricingTrx& pricingTrx =
      (diagPricingTrx() != nullptr && diagPricingTrx()->excTrxType() == PricingTrx::ME_DIAG_TRX)
          ? *diagPricingTrx()
          : *newPricingTrx();
  response = ME_RESPONSE_OPEN_TAG;
  PricingResponseFormatter formatter;
  response += formatter.formatResponse(tmpResponse, false, pricingTrx, nullptr, ere.code());
  response += ME_RESPONSE_CLOSE_TAG;
}

bool
MultiExchangeTrx::convert(std::string& response)
{
  std::string tmpResponse;
  bool result = true;
  response = ME_RESPONSE_OPEN_TAG;
  if (!skipNewPricingTrx())
  {
    result = newPricingTrx()->convert(tmpResponse);
    if (!result)
      return false;
    response += tmpResponse;
  }
  if (!skipExcPricingTrx1())
  {
    // prepareExc0Header
    result = excPricingTrx1()->convert(tmpResponse);
    if (!result)
      return false;
    response += tmpResponse;
  }
  if (!skipExcPricingTrx2())
  {
    // prepareExc1Header
    result = excPricingTrx2()->convert(tmpResponse);
    response += tmpResponse;
  }
  response += ME_RESPONSE_CLOSE_TAG;
  return result;
}

const Billing*
MultiExchangeTrx::billing() const
{
  return _newPricingTrx->billing();
}
}
