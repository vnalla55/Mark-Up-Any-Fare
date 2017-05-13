// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "FallbackServiceServer.h"

namespace tax
{

bool
FallbackServiceServer::isSet(const std::function<bool(const tse::Trx*)>& /*fallbackFunc*/) const
{
  return false; // for now, until established how fallbacks should work on test server
}

bool
FallbackServiceServer::isSet(const std::function<bool()>& /*fallbackFunc*/) const
{
  return false; // for now, until established how fallbacks should work on test server
}

} // namespace tax
