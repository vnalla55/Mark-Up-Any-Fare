//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "Util/BranchPrediction.h"

namespace tse
{
namespace MileageUtil
{
uint16_t getEMS(const uint16_t totalTPM, const uint16_t totalMPM);
}
}

