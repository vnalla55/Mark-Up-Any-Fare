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
#include "Common/Tariff.h"
#include "DataModel/Common/CodeOps.h"

namespace tax
{

namespace
{
  bool matches(int16_t ref, int16_t val)
  {
    return ref == -1 || ref == val;
  }
}

bool matches(Tariff ref, Tariff val)
{
  return matches(ref._asNumber, val._asNumber) && matches(ref._asCode, val._asCode);
}

} // namespace tax
