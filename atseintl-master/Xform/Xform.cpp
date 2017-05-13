//----------------------------------------------------------------------------
//
//  File:               Xform.C
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Xform/Xform.h"

#include "Common/ErrorResponseException.h"
#include "DataModel/Trx.h"

#include <string>

namespace tse
{
bool
Xform::convert(ErrorResponseException& ere, std::string& response)
{
  response = ere.what();
  return true;
}

bool
Xform::throttle(std::string& /*request*/, std::string& response)
{
  ErrorResponseException ere(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);
  return convert(ere, response);
}

bool
Xform::initialize()
{
  return initialize(0, nullptr);
}
}
