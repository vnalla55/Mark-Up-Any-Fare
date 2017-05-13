//-------------------------------------------------------------------
//
//  File:        MarketRule.h
//  Created:     Oct 2013
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class MarketRule : boost::noncopyable
{
public:
  LocCode& originLoc() { return _originLoc; }
  const LocCode& originLoc() const { return _originLoc; }

  LocTypeCode& originLocType() { return _originLocType; }
  const LocTypeCode& originLocType() const { return _originLocType; }

  LocCode& destinationLoc() { return _destinationLoc; }
  const LocCode& destinationLoc() const { return _destinationLoc; }

  LocTypeCode& destinationLocType() { return _destinationLocType; }
  const LocTypeCode& destinationLocType() const { return _destinationLocType; }

  AlphaCode& direction() { return _direction; }
  const AlphaCode direction() const { return _direction; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  GlobalDirection globalDirection() const { return _globalDirection; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

private:
  LocCode _originLoc;
  LocTypeCode _originLocType = LOCTYPE_NONE;
  LocCode _destinationLoc;
  LocTypeCode _destinationLocType = LOCTYPE_NONE;
  AlphaCode _direction;
  GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
  CarrierCode _carrier;
};
} // tse

