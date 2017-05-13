//-------------------------------------------------------------------
//  Copyright Sabre 2016
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
#include "DataModel/CurrencyTrx.h"

#include "Common/Logger.h"
#include "Xform/CurrencyResponseFormatter.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.CurrencyTrx");

void
CurrencyTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  _response << ere.message();
  CurrencyResponseFormatter formatter;
  response = formatter.formatResponse(*this);
}

bool
CurrencyTrx::convert(std::string& response)
{
  LOG4CXX_DEBUG(logger, "Doing CurrencyTrx response: " << xml2());
  CurrencyResponseFormatter formatter;
  response = formatter.formatResponse(*this);
  LOG4CXX_DEBUG(logger, "response: " << response);

  return true;
}
}
