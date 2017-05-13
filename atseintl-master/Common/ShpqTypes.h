// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Util/NullTermVector.h"

#include <vector>

namespace tse
{
namespace shpq
{

enum SolutionType
{ NONE,
  OW, // One Way
  HRT, // Half Round Trip
  OW_OW, // OneWay - OneWay
  OW_HRT, // OneWay - Half Round Trip
  HRT_OW, // Half Round Trip - One Way
  HRT_HRT // Half Round Trip - Half Round Trip
};

enum
{ UNKNOWN_MONEY_AMOUNT = 999999999999 };

typedef std::vector<uint32_t> CxrKeys;
typedef NullTermVector<uint32_t, SOL_MAX_LEGS> CxrKeyPerLeg;
typedef NullTermVector<CxrKeys, SOL_MAX_LEGS, EmptyContainerPolicy> CxrKeysPerLeg;

}
} // namespace tse::shpq

