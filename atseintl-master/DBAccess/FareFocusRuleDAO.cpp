#include "DBAccess/FareFocusRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusRule.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRuleDAO"));

FareFocusRuleDAO&
FareFocusRuleDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusRuleInfo*
getFareFocusRuleData(uint64_t fareFocusRuleId,
                     DateTime adjustedTicketDate,
                     DeleteList& deleteList,
                     DateTime ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusRuleHistoricalDAO& dao(FareFocusRuleHistoricalDAO::instance());

    return dao.get(deleteList, fareFocusRuleId, ticketDate);
  }
  else
  {
    FareFocusRuleDAO& dao(FareFocusRuleDAO::instance());

    return dao.get(deleteList, fareFocusRuleId, adjustedTicketDate, ticketDate);
  }
}

const FareFocusRuleInfo*
FareFocusRuleDAO::get(DeleteList& del,
                      uint64_t fareFocusRuleId,
                      DateTime adjustedTicketDate,
                      DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusRuleKey key(fareFocusRuleId);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusRuleInfo* ret(nullptr);

  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveNow<FareFocusRuleInfo>(adjustedTicketDate, ticketDate));
  if (i != ptr->end())
    ret = *i;

  return ret;
}

std::vector<FareFocusRuleInfo*>*
FareFocusRuleDAO::create(FareFocusRuleKey key)
{
  std::vector<FareFocusRuleInfo*>* ret(new std::vector<FareFocusRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusRule q(dbAdapter->getAdapter());
    q.findFareFocusRule(*ret, key._a);
    /*
    std::cerr << "  !!!!!!!!!QueryGetFareFocusRule: " << ' ' << ret->size() << " !!!!!!!!!!" << std::endl;
    std::vector<FareFocusRuleInfo*> *all(new std::vector<FareFocusRuleInfo*>);// test
    QueryGetAllFareFocusRule qall(dbAdapter->getAdapter());// test
    qall.findAllFareFocusRule(*all);// test
    std::cerr << "  !!!!!!! QueryGetAllFareFocusRule: " << ' ' << all->size() << " !!!!!!" << std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRuleDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRuleKey
FareFocusRuleDAO::createKey(const FareFocusRuleInfo* info)
{
  return FareFocusRuleKey(info->fareFocusRuleId());
}

void
FareFocusRuleDAO::destroy(FareFocusRuleKey, std::vector<FareFocusRuleInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
FareFocusRuleDAO::compress(const std::vector<FareFocusRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareFocusRuleInfo*>*
FareFocusRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareFocusRuleInfo>(compressed);
}

std::string
FareFocusRuleDAO::_name("FareFocusRule");
std::string
FareFocusRuleDAO::_cacheClass("Rules");
DAOHelper<FareFocusRuleDAO>
FareFocusRuleDAO::_helper(_name);
FareFocusRuleDAO*
FareFocusRuleDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRuleHistoricalDAO"));

FareFocusRuleHistoricalDAO&
FareFocusRuleHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusRuleInfo*
FareFocusRuleHistoricalDAO::get(DeleteList& del,
                                uint64_t fareFocusRuleId,
                                DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusRuleHistoricalKey key(fareFocusRuleId);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusRuleInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveDH<FareFocusRuleInfo>(key._b, key._c, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusRuleInfo*>*
FareFocusRuleHistoricalDAO::create(FareFocusRuleHistoricalKey key)
{
  std::vector<FareFocusRuleInfo*>* ret(new std::vector<FareFocusRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusRuleHistorical q(dbAdapter->getAdapter());
    q.findFareFocusRule(*ret, key._a, key._b, key._c);
    //std::cerr << "!!!! QueryGetFareFocusRuleHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRuleHistoricalKey
FareFocusRuleHistoricalDAO::createKey(const FareFocusRuleInfo* info,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  return FareFocusRuleHistoricalKey(info->fareFocusRuleId(), startDate, endDate);
}

void
FareFocusRuleHistoricalDAO::destroy(FareFocusRuleHistoricalKey,
                                    std::vector<FareFocusRuleInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
FareFocusRuleHistoricalDAO::compress(const std::vector<FareFocusRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareFocusRuleInfo*>*
FareFocusRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareFocusRuleInfo>(compressed);
}

std::string
FareFocusRuleHistoricalDAO::_name("FareFocusRuleHistorical");
std::string
FareFocusRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusRuleHistoricalDAO>
FareFocusRuleHistoricalDAO::_helper(_name);
FareFocusRuleHistoricalDAO*
FareFocusRuleHistoricalDAO::_instance(nullptr);

} // namespace tse
