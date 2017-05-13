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

#include "Routing/MileageUtil.h"

// TODO refactor Mileage so that we don't use uint16_t everywhere.
uint16_t
tse::MileageUtil::getEMS(const uint16_t totalTPM, const uint16_t totalMPM)
{
  if (UNLIKELY(totalMPM == 0))
    return 0;

  if (totalTPM <= totalMPM)
    return 0;

  const double percent = double(totalTPM) / double(totalMPM);

  if (percent <= 1.05)
    return 5;
  if (percent <= 1.10)
    return 10;
  if (percent <= 1.15)
    return 15;
  if (percent <= 1.20)
    return 20;
  if (percent <= 1.25)
    return 25;

  return 30;
}
