#include "DBAccess/FareFocusDaytimeApplDAO.h"

#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusDaytimeApplInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusDaytimeAppl.h"

#include <log4cxx/logger.h>

namespace tse
{

log4cxx::LoggerPtr
FareFocusDaytimeApplDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusDaytimeApplDAO"));

FareFocusDaytimeApplDAO&
FareFocusDaytimeApplDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusDaytimeApplInfo*
getFareFocusDaytimeApplData(uint64_t dayTimeApplItemNo,
                         DateTime createDate,
                         DeleteList& deleteList,
                         DateTime expireDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusDaytimeApplHistoricalDAO& dao(FareFocusDaytimeApplHistoricalDAO::instance());

    return dao.get(deleteList, dayTimeApplItemNo, expireDate);
  }
  else
  {
    FareFocusDaytimeApplDAO& dao(FareFocusDaytimeApplDAO::instance());

    return dao.get(deleteList, dayTimeApplItemNo, createDate, expireDate);
  }
}

const FareFocusDaytimeApplInfo*
FareFocusDaytimeApplDAO::get(DeleteList& del,
                           uint64_t dayTimeApplItemNo,
                           DateTime createDate,
                           DateTime expireDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusDaytimeApplKey key(dayTimeApplItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusDaytimeApplInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusDaytimeApplInfo>(expireDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusDaytimeApplInfo*>*
FareFocusDaytimeApplDAO::create(FareFocusDaytimeApplKey key)
{
  std::vector<FareFocusDaytimeApplInfo*>* ret(new std::vector<FareFocusDaytimeApplInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusDaytimeAppl q(dbAdapter->getAdapter());
    q.findFareFocusDaytimeAppl(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusDaytimeApplDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusDaytimeApplKey
FareFocusDaytimeApplDAO::createKey(const FareFocusDaytimeApplInfo* info)
{
  return FareFocusDaytimeApplKey(info->dayTimeApplItemNo());
}

void
FareFocusDaytimeApplDAO::destroy(FareFocusDaytimeApplKey, std::vector<FareFocusDaytimeApplInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusDaytimeApplDAO::_name("FareFocusDaytimeAppl");
std::string
FareFocusDaytimeApplDAO::_cacheClass("Rules");
DAOHelper<FareFocusDaytimeApplDAO>
FareFocusDaytimeApplDAO::_helper(_name);
FareFocusDaytimeApplDAO*
FareFocusDaytimeApplDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusDaytimeApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusDaytimeApplHistoricalDAO"));

FareFocusDaytimeApplHistoricalDAO&
FareFocusDaytimeApplHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusDaytimeApplInfo*
FareFocusDaytimeApplHistoricalDAO::get(DeleteList& del,
                                     uint64_t dayTimeApplItemNo,
                                     DateTime expireDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusDaytimeApplHistoricalKey key(dayTimeApplItemNo);
  DAOUtils::getDateRange(expireDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusDaytimeApplInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusDaytimeApplInfo>(expireDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusDaytimeApplInfo*>*
FareFocusDaytimeApplHistoricalDAO::create(FareFocusDaytimeApplHistoricalKey key)
{
  std::vector<FareFocusDaytimeApplInfo*>* ret(new std::vector<FareFocusDaytimeApplInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusDaytimeApplHistorical q(dbAdapter->getAdapter());
    q.findFareFocusDaytimeAppl(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusDaytimeApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusDaytimeApplHistoricalKey
FareFocusDaytimeApplHistoricalDAO::createKey(const FareFocusDaytimeApplInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusDaytimeApplHistoricalKey(info->dayTimeApplItemNo(), startDate, endDate);
}

void
FareFocusDaytimeApplHistoricalDAO::destroy(FareFocusDaytimeApplHistoricalKey,
                                         std::vector<FareFocusDaytimeApplInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusDaytimeApplHistoricalDAO::_name("FareFocusDaytimeApplHistorical");
std::string
FareFocusDaytimeApplHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusDaytimeApplHistoricalDAO>
FareFocusDaytimeApplHistoricalDAO::_helper(_name);
FareFocusDaytimeApplHistoricalDAO*
FareFocusDaytimeApplHistoricalDAO::_instance(nullptr);

} // namespace tse
