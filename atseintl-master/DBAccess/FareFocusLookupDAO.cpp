#include "DBAccess/FareFocusLookupDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusLookupInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusLookup.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusLookupDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusLookupDAO"));

FareFocusLookupDAO&
FareFocusLookupDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusLookupInfo*
getFareFocusLookupData(const PseudoCityCode& pcc,
                       DeleteList& deleteList,
                       DateTime ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusLookupHistoricalDAO& dao(FareFocusLookupHistoricalDAO::instance());

    return dao.get(deleteList, pcc, ticketDate);
  }
  else
  {
    FareFocusLookupDAO& dao(FareFocusLookupDAO::instance());

    return dao.get(deleteList, pcc, ticketDate);
  }
}

const FareFocusLookupInfo*
FareFocusLookupDAO::get(DeleteList& del,
                        const PseudoCityCode& pcc,
                        DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusLookupKey key(pcc);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusLookupInfo* ret(nullptr);
  if (!ptr->empty())
  {
    ret = ptr->front();
  }
  return ret;
}

std::vector<FareFocusLookupInfo*>*
FareFocusLookupDAO::create(FareFocusLookupKey key)
{
  std::vector<FareFocusLookupInfo*>* ret(new std::vector<FareFocusLookupInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusLookup q(dbAdapter->getAdapter());
    q.findFareFocusLookup(*ret, key._a);
    /*
    std::cerr << "  !! QueryGetFareFocusLookup: " << ' ' << ret->size() << " !!" << std::endl;
    for (size_t i = 0; i < ret->size(); ++i)
    {
      FareFocusLookupInfo* info((*ret)[i]);
      std::cerr << "  !! pcc: " << ' ' << info->pcc() << " !!" << std::endl;
      std::vector<uint64_t>& ids(info->fareFocusRuleIds());
      for (size_t j = 0; j < ids.size(); ++j)
      {
        std::cerr << "  !! id: " << ' ' << ids[j] << " !!" << std::endl;
      }
    }
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusLookupDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusLookupKey
FareFocusLookupDAO::createKey(const FareFocusLookupInfo* info)
{
  return FareFocusLookupKey(info->pcc());
}

void
FareFocusLookupDAO::destroy(FareFocusLookupKey, std::vector<FareFocusLookupInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusLookupDAO::_name("FareFocusLookup");
std::string
FareFocusLookupDAO::_cacheClass("Rules");
DAOHelper<FareFocusLookupDAO>
FareFocusLookupDAO::_helper(_name);
FareFocusLookupDAO*
FareFocusLookupDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusLookupHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusLookupHistoricalDAO"));

FareFocusLookupHistoricalDAO&
FareFocusLookupHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusLookupInfo*
FareFocusLookupHistoricalDAO::get(DeleteList& del,
                                  const PseudoCityCode& pcc,
                                  DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusLookupHistoricalKey key(pcc);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  FareFocusLookupInfo* ret(nullptr);
  if (!ptr->empty())
  {
    ret = ptr->front();
  }
  return ret;
}

std::vector<FareFocusLookupInfo*>*
FareFocusLookupHistoricalDAO::create(FareFocusLookupHistoricalKey key)
{
  std::vector<FareFocusLookupInfo*>* ret(new std::vector<FareFocusLookupInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusLookupHistorical q(dbAdapter->getAdapter());
    q.findFareFocusLookup(*ret, key._a, key._b, key._c);
    /*
    std::cerr << "  !! QueryGetFareFocusLookupHistorical: " << ' ' << ret->size() << " !!" << std::endl;
    for (size_t i = 0; i < ret->size(); ++i)
    {
      FareFocusLookupInfo* info((*ret)[i]);
      std::cerr << "  !! pcc: " << ' ' << info->pcc() << " !!" << std::endl;
      std::vector<uint64_t>& ids(info->fareFocusRuleIds());
      for (size_t j = 0; j < ids.size(); ++j)
      {
        std::cerr << "  !! id: " << ' ' << ids[j] << " !!" << std::endl;
      }
    }
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusLookupHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusLookupHistoricalKey
FareFocusLookupHistoricalDAO::createKey(const FareFocusLookupInfo* info,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  return FareFocusLookupHistoricalKey(info->pcc(), startDate, endDate);
}

void
FareFocusLookupHistoricalDAO::destroy(FareFocusLookupHistoricalKey,
                                      std::vector<FareFocusLookupInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusLookupHistoricalDAO::_name("FareFocusLookupHistorical");
std::string
FareFocusLookupHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusLookupHistoricalDAO>
FareFocusLookupHistoricalDAO::_helper(_name);
FareFocusLookupHistoricalDAO*
FareFocusLookupHistoricalDAO::_instance(nullptr);

} // namespace tse
