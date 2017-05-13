// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#pragma once

#include "DataModel/RequestResponse/InputLoc.h"
#include "DataModel/RequestResponse/InputFlightUsage.h"

namespace tax
{

struct InputGeo
{
  InputGeo() : _id(0), _unticketedTransfer(type::UnticketedTransfer::No) {}

  type::Index _id;
  InputLoc _loc;
  type::UnticketedTransfer _unticketedTransfer;
};

} // namespace tax
