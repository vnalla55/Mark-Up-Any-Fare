//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "Common/Config/ConfigurableValueUtil.h"

#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseCallableTrxTask.h"

namespace tse
{
namespace ConfigurableValueUtil
{

const Trx*
getTrxFromTLS(bool checked)
{
  Trx* trx = TseCallableTrxTask::currentTrx();
  if (checked)
    TSE_ASSERT(trx);
  return trx;
}

void
checkTrx(const Trx* trx)
{
  if (UNLIKELY(!trx))
    throw ErrorResponseException(ErrorResponseException::DYNAMIC_CONFIGURATION_RELOAD_IN_PROGRESS);
}

}
}
