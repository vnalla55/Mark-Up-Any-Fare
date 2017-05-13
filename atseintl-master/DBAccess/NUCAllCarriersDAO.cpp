//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NUCAllCarriersDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/Queries/QueryGetNucAllCarriers.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NUCAllCarriersDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NUCAllCarriersDAO"));

NUCAllCarriersDAO&
NUCAllCarriersDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct IsNotEffectiveInRange
{
  IsNotEffectiveInRange(const DateTime& from_, const DateTime& to_) : from(from_), to(to_) {}

  const DateTime& from;
  const DateTime& to;

  bool operator()(NUCInfo*& nuc) { return !(nuc->effDate() <= to && nuc->expireDate() >= from); }
};

const std::vector<NUCInfo*>&
getNUCAllCarriersData(const CurrencyCode& currency,
                      const DateTime& from_date,
                      const DateTime& to_date,
                      DeleteList& deleteList)
{
  DateTime current = DateTime::localTime();
  if (Global::allowHistorical() && from_date < current)
  {
    NUCAllCarriersHistoricalDAO& hdao = NUCAllCarriersHistoricalDAO::instance();
    return hdao.get(deleteList, currency, from_date, to_date);
  }

  NUCAllCarriersDAO& dao = NUCAllCarriersDAO::instance();
  return dao.get(deleteList, currency, from_date, to_date);
}

const std::vector<NUCInfo*>&
NUCAllCarriersDAO::get(DeleteList& del,
                       const CurrencyCode& currency,
                       const DateTime& start_date,
                       const DateTime& to_date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCAllCarriersKey key(currency);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;
  del.adopt(ret);

  IsNotEffectiveInRange predicate(start_date,to_date);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), predicate);

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveInRange(start_date, to_date)));
}

NUCAllCarriersKey
NUCAllCarriersDAO::createKey(NUCInfo* info)
{
  return NUCAllCarriersKey(info->_cur);
}

void
NUCAllCarriersDAO::load()
{
  StartupLoader<QueryGetAllNucsAllCarriers, NUCInfo, NUCAllCarriersDAO>();
}

std::vector<NUCInfo*>*
NUCAllCarriersDAO::create(NUCAllCarriersKey key)
{
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneNucAllCarriers nuc(dbAdapter->getAdapter());
    nuc.findNUC(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NUCAllCarriersDAO::create");
    throw;
  }

  return ret;
}

void
NUCAllCarriersDAO::destroy(NUCAllCarriersKey key, std::vector<NUCInfo*>* recs)
{
  std::vector<NUCInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NUCAllCarriersDAO::_name("NUCALLCARRIERS");
std::string
NUCAllCarriersDAO::_cacheClass("Currency");

DAOHelper<NUCAllCarriersDAO>
NUCAllCarriersDAO::_helper(_name);

NUCAllCarriersDAO* NUCAllCarriersDAO::_instance = nullptr;

/////////////////////////// Historical DAO Object ///////////////////////////
log4cxx::LoggerPtr
NUCAllCarriersHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NUCAllCarriersHistoricalDAO"));

NUCAllCarriersHistoricalDAO&
NUCAllCarriersHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

/*
struct IsNotEffectiveInRange
{
    IsNotEffectiveInRange(const DateTime& from_,const DateTime& to_) : from(from_),to(to_)
    {
    }

    const DateTime& from ;
    const DateTime& to ;

    bool operator()(NUCInfo*& nuc)
    {
        return ! ( nuc->effDate() <= to && nuc->expireDate() >= from  ) ;
    }
};
*/
const std::vector<NUCInfo*>&
NUCAllCarriersHistoricalDAO::get(DeleteList& del,
                                 const CurrencyCode& currency,
                                 const DateTime& from_date,
                                 const DateTime& to_date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCAllCarriersHistoricalKey key(currency);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;

  del.adopt(ret);
  IsNotEffectiveInRange predicate(from_date, to_date);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), predicate);
  return *ret;
}

std::vector<NUCInfo*>*
NUCAllCarriersHistoricalDAO::create(NUCAllCarriersHistoricalKey key)
{
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetNucAllCarriersHistorical nuc(dbAdapter->getAdapter());
    nuc.findNUC(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NUCAllCarriersHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NUCAllCarriersHistoricalDAO::destroy(NUCAllCarriersHistoricalKey key, std::vector<NUCInfo*>* recs)
{
  std::vector<NUCInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NUCAllCarriersHistoricalDAO::_name("NUCAllCarriersHistorical");
std::string
NUCAllCarriersHistoricalDAO::_cacheClass("Currency");
DAOHelper<NUCAllCarriersHistoricalDAO>
NUCAllCarriersHistoricalDAO::_helper(_name);

NUCAllCarriersHistoricalDAO* NUCAllCarriersHistoricalDAO::_instance = nullptr;

} // namespace tse
