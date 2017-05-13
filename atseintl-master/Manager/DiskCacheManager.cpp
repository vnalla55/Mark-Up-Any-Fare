//----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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
#include "Manager/DiskCacheThread.h"
#include "Manager/SimpleManager.h"

namespace tse
{
typedef SimpleManager<DiskCacheThread> DiskCacheManager;

template <>
Logger
DiskCacheManager::_logger("atseintl.Manager.DiskCacheManager");

static LoadableModuleRegister<Manager, DiskCacheManager>
_("libDiskCacheManager.so");

} // tse
