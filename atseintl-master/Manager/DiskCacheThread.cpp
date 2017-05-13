//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Manager/DiskCacheThread.h"

#include "Common/Logger.h"

namespace tse
{
template <>
ManagerThreadConfig
DiskCacheThread::_configVariables("DISKCACHE_MAN");

template <>
Logger
DiskCacheThread::_logger("atseintl.Manager.DiskCacheThread");
}
