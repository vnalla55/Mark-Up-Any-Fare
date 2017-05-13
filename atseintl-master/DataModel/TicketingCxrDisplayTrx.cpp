#include "Common/Logger.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "Xform/STLTicketingCxrDisplayResponseFormatter.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.TicketingCxrDisplayTrx");

void
TicketingCxrDisplayTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  LOG4CXX_DEBUG(logger, "TicketingCxrDisplayTrx response: " << xml2());
  STLTicketingCxrDisplayResponseFormatter responseFormatter;
  response = responseFormatter.formatResponse(
      (ere.code() > 0 && ere.message().empty()) ? "UNKNOWN EXCEPTION" : ere.message(),
      *this,
      ere.code());
}

bool
TicketingCxrDisplayTrx::convert(std::string& response)
{
  std::string tmpResponse;
  LOG4CXX_DEBUG(logger, "TicketingCxrDisplayTrx response: " << xml2());
  STLTicketingCxrDisplayResponseFormatter responseFormatter;
  response = responseFormatter.formatResponse(tmpResponse, *this);
  LOG4CXX_DEBUG(logger, "Response: " << response);

  return true;
}
}
