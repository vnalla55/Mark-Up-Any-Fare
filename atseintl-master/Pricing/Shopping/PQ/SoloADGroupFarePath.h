//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "Common/TseDateTimeTypes.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"

namespace tse
{
namespace shpq
{

class SoloADGroupFarePath : public SoloGroupFarePath
{
public:
  SoloADGroupFarePath(DatePair dp) : _datePair(dp) {}

  const DatePair _datePair;
};

} /* namespace shpq */
} /* namespace tse */

