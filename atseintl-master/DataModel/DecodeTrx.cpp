#include "Common/Logger.h"
#include "DataModel/DecodeTrx.h"
#include "Xform/DecodeResponseFormatter.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.DecodeTrx");

void
DecodeTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  LOG4CXX_DEBUG(logger, "DecodeTrx response: " << ere.message());
  this->addToResponse(ere.message());
  DecodeResponseFormatter formatter(*this);
  response = formatter.formatResponse('E');
}

bool
DecodeTrx::convert(std::string& response)
{
  LOG4CXX_DEBUG(logger, "Generating DecodeTrx response");
  DecodeResponseFormatter formatter(*this);
  response = formatter.formatResponse();
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}
}
