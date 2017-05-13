//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/TktDesignatorExemptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTktDesignatorExempt.h"
#include "DBAccess/TktDesignatorExemptInfo.h"

namespace tse
{
log4cxx::LoggerPtr
TktDesignatorExemptDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TktDesignatorExemptDAO"));

TktDesignatorExemptDAO&
TktDesignatorExemptDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TktDesignatorExemptInfo*>&
getTktDesignatorExemptData(const CarrierCode& carrier,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    TktDesignatorExemptHistoricalDAO& dao(TktDesignatorExemptHistoricalDAO::instance());

    const std::vector<TktDesignatorExemptInfo*>& ret(dao.get(deleteList, carrier, ticketDate));

    return ret;
  }
  else
  {
    TktDesignatorExemptDAO& dao(TktDesignatorExemptDAO::instance());

    const std::vector<TktDesignatorExemptInfo*>& ret(dao.get(deleteList, carrier, ticketDate));

    return ret;
  }
}

const std::vector<TktDesignatorExemptInfo*>&
TktDesignatorExemptDAO::get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  TktDesignatorExemptKey key(carrier);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<TktDesignatorExemptInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<TktDesignatorExemptInfo*>*
TktDesignatorExemptDAO::create(TktDesignatorExemptKey key)
{
  std::vector<TktDesignatorExemptInfo*>* ret(new std::vector<TktDesignatorExemptInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetTktDesignatorExempt q(dbAdapter->getAdapter());
    q.findTktDesignatorExempt(*ret, key._a);
    /*
    std::cerr << "  !!!!!!!!!QueryGetTktDesignatorExempt: " << ' ' << ret->size() << " !!!!!!!!!!"
    << std::endl;
    std::vector<TktDesignatorExemptInfo*> *all(new std::vector<TktDesignatorExemptInfo*>);// test
    QueryGetAllTktDesignatorExempt qall(dbAdapter->getAdapter());// test
    qall.findAllTktDesignatorExempt(*all);// test
    std::cerr << "  !!!!!!! QueryGetAllTktDesignatorExempt: " << ' ' << all->size() << " !!!!!!" <<
    std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TktDesignatorExemptDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

TktDesignatorExemptKey
TktDesignatorExemptDAO::createKey(const TktDesignatorExemptInfo* info)
{
  return TktDesignatorExemptKey(info->carrier());
}

void
TktDesignatorExemptDAO::destroy(TktDesignatorExemptKey, std::vector<TktDesignatorExemptInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
TktDesignatorExemptDAO::_name("TktDesignatorExempt");
std::string
TktDesignatorExemptDAO::_cacheClass("Fares");
DAOHelper<TktDesignatorExemptDAO>
TktDesignatorExemptDAO::_helper(_name);
TktDesignatorExemptDAO*
TktDesignatorExemptDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
TktDesignatorExemptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TktDesignatorExemptHistoricalDAO"));

TktDesignatorExemptHistoricalDAO&
TktDesignatorExemptHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TktDesignatorExemptInfo*>&
TktDesignatorExemptHistoricalDAO::get(DeleteList& del,
                                      const CarrierCode& carrier,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  TktDesignatorExemptHistoricalKey key(carrier);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<TktDesignatorExemptInfo*>* ret(new std::vector<TktDesignatorExemptInfo*>);
  del.adopt(ret);
  IsNotCurrentH<TktDesignatorExemptInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<TktDesignatorExemptInfo*>*
TktDesignatorExemptHistoricalDAO::create(TktDesignatorExemptHistoricalKey key)
{
  std::vector<TktDesignatorExemptInfo*>* ret(new std::vector<TktDesignatorExemptInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetTktDesignatorExemptHistorical q(dbAdapter->getAdapter());
    q.findTktDesignatorExempt(*ret, key._a, key._b, key._c);
    // std::cerr << "!!!! QueryGetTktDesignatorExemptHistorical: " << ' ' << ret->size() <<
    // std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TktDesignatorExemptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

TktDesignatorExemptHistoricalKey
TktDesignatorExemptHistoricalDAO::createKey(const TktDesignatorExemptInfo* info,
                                            const DateTime& startDate,
                                            const DateTime& endDate)
{
  return TktDesignatorExemptHistoricalKey(info->carrier(), startDate, endDate);
}

void
TktDesignatorExemptHistoricalDAO::destroy(TktDesignatorExemptHistoricalKey,
                                          std::vector<TktDesignatorExemptInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
TktDesignatorExemptHistoricalDAO::_name("TktDesignatorExemptHistorical");
std::string
TktDesignatorExemptHistoricalDAO::_cacheClass("Fares");
DAOHelper<TktDesignatorExemptHistoricalDAO>
TktDesignatorExemptHistoricalDAO::_helper(_name);
TktDesignatorExemptHistoricalDAO*
TktDesignatorExemptHistoricalDAO::_instance(nullptr);

} // namespace tse
