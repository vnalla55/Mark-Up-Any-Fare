//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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


#ifndef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/DynamicConfigurableValue.h"

#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "DataModel/Trx.h"

namespace tse
{
const Trx*
DynamicConfigurableValue::getTrx(const Trx* trx)
{
  if (LIKELY(trx))
    return trx;

  return TseCallableTrxTask::currentTrx();
}

void
DynamicConfigurableValue::checkTrx(const Trx* trx)
{
  if (UNLIKELY(!trx))
    throw ErrorResponseException(ErrorResponseException::DYNAMIC_CONFIGURATION_RELOAD_IN_PROGRESS);
}

bool
DynamicConfigurableValue::shouldReadFromConfig(const Trx*& trx)
{
  trx = getTrx(trx);

  if (UNLIKELY(Global::configUpdateInProgress()))
  {
    checkTrx(trx);
    return true;
  }

  if (UNLIKELY(trx && trx->isDynamicCfgOverriden()))
    return true;

  return false;
}

const ConfigMan&
DynamicConfigurableValue::configureGetConfigSource(const std::string& section,
                                                   const std::string& name)
{
  if (Global::configUpdateInProgress())
    return *Global::newDynamicCfg();

  return *Global::dynamicCfg();
}

} // tse

#endif
