//-------------------------------------------------------------------
//
//  File:        RequestSource.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class RequestSource
{
public:
  ClientID& clientID() { return _clientID; }
  const ClientID& clientID() const { return _clientID; }

  BrandRequestType& requestType() { return _requestType; }
  const BrandRequestType& requestType() const { return _requestType; }

private:
  ClientID _clientID;
  BrandRequestType _requestType;
};
} // tse

