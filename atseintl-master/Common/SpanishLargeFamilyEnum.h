//-------------------------------------------------------------------
//
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

#pragma once

#include <cstdint>

namespace tse
{

namespace SLFUtil
{

enum class DiscountLevel : uint8_t
{
  NO_DISCOUNT = 0,
  LEVEL_1 = 1,
  LEVEL_2 = 2
};

} // namespace SLFUtil

} // namespace tse
