//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#pragma once

#include <cstdint>

namespace tse
{
struct FreqFlyerStatus;

struct FreqFlyerStatusData
{
  explicit FreqFlyerStatusData(const FreqFlyerStatus* data) : _dbData(data) {}
  const FreqFlyerStatus* _dbData;
  uint16_t _maxPassengersTotal = 0;
};
}
