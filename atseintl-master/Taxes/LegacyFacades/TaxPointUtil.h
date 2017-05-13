// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Taxes/Common/AtpcoTypes.h"

namespace tse
{
class Itin;

class TaxPointUtil
{
public:
  static void
  setTravelSegIndices(tax::type::Index taxPointBegin,
                      tax::type::Index taxPointEnd,
                      const tse::Itin& itin,
                      uint16_t& travelSegStartIndex,
                      uint16_t& travelSegEndIndex);
};
}
