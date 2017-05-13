//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/RuleCategoryDescDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRuleCatDesc.h"
#include "DBAccess/RuleCategoryDescInfo.h"

namespace tse
{
log4cxx::LoggerPtr
RuleCategoryDescDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RuleCategoryDescDAO"));

RuleCategoryDescDAO&
RuleCategoryDescDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const RuleCategoryDescInfo*
getRuleCategoryDescData(const CatNumber& key, DeleteList& deleteList)
{
  return RuleCategoryDescDAO::instance().get(deleteList, key);
}

const RuleCategoryDescInfo*
RuleCategoryDescDAO::get(DeleteList& del, const CatNumber& key)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CatKey(key));
  del.copy(ptr);
  return ptr.get();
}

RuleCategoryDescInfo*
RuleCategoryDescDAO::create(CatKey key)
{
  RuleCategoryDescInfo* ret = nullptr;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRuleCatDesc rcd(dbAdapter->getAdapter());
    rcd.findRuleCategoryDesc(ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleCategoryDescDAO::create");
    throw;
  }

  return ret;
}

void
RuleCategoryDescDAO::destroy(CatKey key, RuleCategoryDescInfo* rec)
{
  delete rec;
}

struct RuleCategoryDescDAO::groupByKey
{
public:
  DAOCache& cache;

  groupByKey() : cache(RuleCategoryDescDAO::instance().cache()) {}

  void operator()(RuleCategoryDescInfo* info) { cache.put(info->category(), info); }
};

void
RuleCategoryDescDAO::load()
{
  std::vector<RuleCategoryDescInfo*> recs;
  Loader<QueryGetAllRuleCatDescs, RuleCategoryDescDAO, std::vector<RuleCategoryDescInfo*> > loader(
      recs);
  if (loader.successful() && (!loader.gotFromLDC()))
  {
    std::for_each(recs.begin(), recs.end(), groupByKey());
  }
}

CatKey
RuleCategoryDescDAO::createKey(RuleCategoryDescInfo* info)
{
  return CatKey(info->category());
}

std::string
RuleCategoryDescDAO::_name("RuleCategoryDesc");
std::string
RuleCategoryDescDAO::_cacheClass("Rules");
DAOHelper<RuleCategoryDescDAO>
RuleCategoryDescDAO::_helper(_name);
RuleCategoryDescDAO* RuleCategoryDescDAO::_instance = nullptr;

} // namespace tse
