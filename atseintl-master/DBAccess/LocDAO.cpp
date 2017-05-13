//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/LocDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Queries/QueryGetDomIntFares.h"
#include "DBAccess/Queries/QueryGetMarket.h"

namespace tse
{
log4cxx::LoggerPtr
LocDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.LocDAO"));

LocDAO&
LocDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const Loc*
getLocData(const LocCode& locCode,
           const DateTime& date,
           DeleteList& deleteList,
           const DateTime& ticketDate,
           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    LocHistoricalDAO& dao = LocHistoricalDAO::instance();
    const Loc* result = dao.get(deleteList, locCode, date, ticketDate);

    if (result == nullptr)
    {
      LocDAO& dao = LocDAO::instance();
      return dao.get(deleteList, locCode, DateTime::localTime(), DateTime::localTime());
    }
    else
    {
      return result;
    }
  }
  else
  {
    LocDAO& dao = LocDAO::instance();
    return dao.get(deleteList, locCode, date, ticketDate);
  }
}

NationCode
getLocNationData(const LocCode& loc,
                 const DateTime& date,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    LocHistoricalDAO& dao = LocHistoricalDAO::instance();
    return dao.getLocNation(loc, date, ticketDate);
  }
  else
  {
    LocDAO& dao = LocDAO::instance();
    return dao.getLocNation(loc, date, ticketDate);
  }
}

const Loc*
LocDAO::get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  Loc* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<Loc>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

NationCode
LocDAO::getLocNation(const LocCode& key, const DateTime& date, const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<Loc>(date, ticketDate));
  if (UNLIKELY(i == ptr->end()))
    return INVALID_NATIONCODE;
  return (*i)->nation();
}

std::vector<Loc*>*
LocDAO::create(LocCodeKey key)
{
  std::vector<Loc*>* ret = new std::vector<Loc*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMkt mkt(dbAdapter->getAdapter());
    mkt.findLoc(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in LocDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

LocCodeKey
LocDAO::createKey(Loc* info)
{
  return LocCodeKey(info->loc());
}

void
LocDAO::load()
{
  StartupLoader<QueryGetAllMkt, Loc, LocDAO>();
}

size_t
LocDAO::clear()
{
  size_t result(cache().clear());
  QueryGetDomFares::clearDomMkts();
  return result;
}

void
LocDAO::destroy(LocCodeKey key, std::vector<Loc*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
LocDAO::compress(const std::vector<Loc*>* vect) const
{
  return compressVector(vect);
}

std::vector<Loc*>*
LocDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Loc>(compressed);
}

std::string
LocDAO::_name("Loc");
std::string
LocDAO::_cacheClass("Common");
DAOHelper<LocDAO>
LocDAO::_helper(_name);
LocDAO* LocDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: LocHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
LocHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.LocHistoricalDAO"));
LocHistoricalDAO&
LocHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Loc*
LocHistoricalDAO::get(DeleteList& del,
                      const LocCode& loc,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  LocHistoricalKey key(loc);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  Loc* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveH<Loc>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

NationCode
LocHistoricalDAO::getLocNation(const LocCode& loc, const DateTime& date, const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  LocHistoricalKey key(loc);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveH<Loc>(date, ticketDate));
  if (i == ptr->end())
    return INVALID_NATIONCODE;
  return (*i)->nation();
}

size_t
LocHistoricalDAO::clear()
{
  size_t result(cache().clear());
  QueryGetDomFaresHistorical::clearDomMkts();
  return result;
}

std::vector<Loc*>*
LocHistoricalDAO::create(LocHistoricalKey key)
{
  std::vector<Loc*>* ret = new std::vector<Loc*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMktHistorical mkt(dbAdapter->getAdapter());
    mkt.findLoc(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in LocHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
LocHistoricalDAO::destroy(LocHistoricalKey key, std::vector<Loc*>* recs)
{
  std::vector<Loc*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
LocHistoricalDAO::compress(const std::vector<Loc*>* vect) const
{
  return compressVector(vect);
}

std::vector<Loc*>*
LocHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Loc>(compressed);
}

std::string
LocHistoricalDAO::_name("LocHistorical");
std::string
LocHistoricalDAO::_cacheClass("Common");
DAOHelper<LocHistoricalDAO>
LocHistoricalDAO::_helper(_name);
LocHistoricalDAO* LocHistoricalDAO::_instance = nullptr;

} // namespace tse
