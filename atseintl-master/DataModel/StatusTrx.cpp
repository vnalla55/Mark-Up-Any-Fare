// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "DataModel/StatusTrx.h"

#include "Common/Logger.h"
#include "Xform/XMLConvertUtils.h"

#include <sstream>
#include <utility>

namespace tse
{
static Logger
logger("atseintl.DataModel.StatusTrx");

bool
StatusTrx::convert(std::string& response)
{
  std::string tmpResponse = this->response().str();
  // Map to XML
  if (tmpResponse.empty())
  {
    tmpResponse = "RECEIVED REQUEST - NO RESPONSE DATA\n";
  }
  std::string xmlResponse;
  XMLConvertUtils::formatResponse(tmpResponse, xmlResponse);
  response = std::move(xmlResponse);

  return true;
}
}
