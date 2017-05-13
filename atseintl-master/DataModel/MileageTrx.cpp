#include "DataModel/MileageTrx.h"

#include "Common/Logger.h"
#include "Xform/MileageResponseFormatter.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.MileageTrx");
void
MileageTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  _response << ere.message();
  MileageResponseFormatter formatter;
  response = formatter.formatResponse(*this);
}
bool
MileageTrx::convert(std::string& response)
{
  LOG4CXX_DEBUG(logger, "Doing MileageTrx response");
  MileageResponseFormatter formatter;
  response = formatter.formatResponse(*this);
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}
} // namespace tse
