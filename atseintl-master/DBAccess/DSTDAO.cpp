//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/DSTDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/DST.h"
#include "DBAccess/Queries/QueryGetDST.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DSTDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DSTDAO"));

DSTDAO&
DSTDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

DST*
DSTDAO::get(DeleteList& del, const DSTGrpCode& dstGroup)
{

  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DSTKey key(dstGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return ptr.get();
}

DST*
DSTDAO::create(DSTKey key)
{
  tse::DST* dst = new tse::DST;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDST query(dbAdapter->getAdapter());
    if (!query.findDST(*dst, key._a))
    {
      delete dst;
      dst = nullptr;
    }
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DSTDAO::create");
    throw;
  }

  return dst;
}

void
DSTDAO::destroy(DSTKey key, DST* dst)
{
  if (dst)
    delete dst;
}

std::string
DSTDAO::_name("DST");
std::string
DSTDAO::_cacheClass("Rules");

DAOHelper<DSTDAO>
DSTDAO::_helper(_name);

DSTDAO* DSTDAO::_instance = nullptr;

DSTKey
DSTDAO::createKey(const DST* info)
{
  return DSTKey(info->dstgroup());
}

void
DSTDAO::load()
{
  DST tempData;
  bool loadQuerySupported(false);
  Loader<StartupLoaderNoDB<DST, DSTDAO>::NoQuery, DSTDAO, DST>(tempData, loadQuerySupported);
}

} // namespace tse
