// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/Types.h"

namespace tax
{
class Geo;

struct TaxableData
{
  TaxableData() :
    _taxPointLoc2(nullptr),
    _taxPointLoc3(nullptr),
    _taxPointEnd(nullptr),
    _taxableAmount(0)
  {
  }

  const Geo* _taxPointLoc2;
  const Geo* _taxPointLoc3;
  const Geo* _taxPointEnd;
  type::MoneyAmount _taxableAmount;
};
}

