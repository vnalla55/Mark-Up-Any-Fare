//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/Logger.h"
#include "Manager/CacheNotifyThread.h"
#include "Manager/SimpleManager.h"

namespace tse
{
typedef SimpleManager<CacheNotifyThread> CacheNotifyManager;

template <>
Logger
CacheNotifyManager::_logger("atseintl.Manager.CacheNotifyManager");

static LoadableModuleRegister<Manager, CacheNotifyManager>
_("libCacheNotifyManager.so");

} // tse
