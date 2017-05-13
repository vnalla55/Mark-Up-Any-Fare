// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "TestServer/Facades/ActivationInfoServiceServer.h"
#include "Rules/RequestLogicError.h"
#include "Rules/MathUtils.h"

namespace tax
{

ActivationInfoServiceServer::ActivationInfoServiceServer()
{
}

bool
ActivationInfoServiceServer::isAtpcoDefaultRoundingActive() const
{
  return false;
}

}
