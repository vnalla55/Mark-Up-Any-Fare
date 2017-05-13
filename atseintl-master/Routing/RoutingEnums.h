//----------------------------------------------------------------------------
//
//  File:           RoutingEnums.h
//  Description:    Enums those are used in only routing.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

namespace tse
{
enum RoutingFareType
{
  MILEAGE_FARE = 0,
  ROUTING_FARE,
  PSR_HIP_EXEMPT_FARE,
  UNKNOWN_ROUTING_FARE_TYPE
};
}
