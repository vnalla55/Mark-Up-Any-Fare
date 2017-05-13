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

#include "Common/FallbackUtil.h"
#include "DataModel/Trx.h"
#include "Taxes/LegacyFacades/FallbackServiceV2.h"

namespace tse
{

bool
FallbackServiceV2::isSet(const std::function<bool(const tse::Trx*)>& fallbackFunc) const
{
  return fallbackFunc(&_trx);
}

bool
FallbackServiceV2::isSet(const std::function<bool()>& fallbackFunc) const
{
  return fallbackFunc();
}

} // namespace tse
