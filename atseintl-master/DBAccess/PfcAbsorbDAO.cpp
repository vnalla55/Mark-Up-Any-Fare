//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcAbsorbDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcAbsorb.h"
#include "DBAccess/Queries/QueryGetPfcAbsorb.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcAbsorbDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcAbsorbDAO"));

PfcAbsorbDAO&
PfcAbsorbDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct PfcAbsorbDAO::isEffective : public std::unary_function<const PfcAbsorb*, bool>
{
  const DateTime _date;

  isEffective(const DateTime& date) : _date(date) {}

  bool operator()(const PfcAbsorb* rec) const
  {
    return (rec->effDate() <= _date) && _date <= rec->expireDate();
  }
};

const std::vector<PfcAbsorb*>&
getPfcAbsorbData(const LocCode& pfcAirport,
                 const CarrierCode& localCarrier,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    PfcAbsorbHistoricalDAO& dao = PfcAbsorbHistoricalDAO::instance();
    return dao.get(deleteList, pfcAirport, localCarrier, date, ticketDate);
  }
  else
  {
    PfcAbsorbDAO& dao = PfcAbsorbDAO::instance();
    return dao.get(deleteList, pfcAirport, localCarrier, date, ticketDate);
  }
}

const std::vector<PfcAbsorb*>&
PfcAbsorbDAO::get(DeleteList& del,
                  const LocCode& pfcAirport,
                  const CarrierCode& localCarrier,
                  const DateTime& date,
                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcAbsorbKey key(pfcAirport, localCarrier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<PfcAbsorb*>* ret = new std::vector<PfcAbsorb*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<PfcAbsorb>(date,
  ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<PfcAbsorb>),
             ret->end());
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<PfcAbsorb>(date, ticketDate)));
}

const std::vector<PfcAbsorb*>&
getAllPfcAbsorbData(DeleteList& del)
{

  PfcAbsorbDAO& dao = PfcAbsorbDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcAbsorb*>&
PfcAbsorbDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::shared_ptr<std::vector<PfcAbsorbKey>> keys = cache().keys();

  std::vector<PfcAbsorb*>* ret = new std::vector<PfcAbsorb*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}
// dao fix
size_t
PfcAbsorbDAO::clear()
{
  size_t result(cache().clear());
  //DISKCACHE.remove(time(NULL), true, _name, DISKCACHE_ALL);
  load();
  return result;
}

size_t
PfcAbsorbDAO::invalidate(const ObjectKey& objectKey)
{
  return clear();
}

void
PfcAbsorbDAO::load()
{
  StartupLoader<QueryGetAllPfcAbsorb, PfcAbsorb, PfcAbsorbDAO>();
}

PfcAbsorbKey
PfcAbsorbDAO::createKey(PfcAbsorb* info)
{
  return PfcAbsorbKey(info->pfcAirport(), info->localCarrier());
}

std::vector<PfcAbsorb*>*
PfcAbsorbDAO::create(PfcAbsorbKey key)
{
  std::vector<PfcAbsorb*>* ret = new std::vector<PfcAbsorb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcAbsorb pa(dbAdapter->getAdapter());
    pa.findPfcAbsorb(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcAbsorbDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcAbsorbDAO::destroy(PfcAbsorbKey key, std::vector<PfcAbsorb*>* recs)
{
  std::vector<PfcAbsorb*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
PfcAbsorbDAO::compress(const std::vector<PfcAbsorb*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcAbsorb*>*
PfcAbsorbDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcAbsorb>(compressed);
}

std::string
PfcAbsorbDAO::_name("PfcAbsorb");
std::string
PfcAbsorbDAO::_cacheClass("Taxes");

DAOHelper<PfcAbsorbDAO>
PfcAbsorbDAO::_helper(_name);

PfcAbsorbDAO* PfcAbsorbDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcAbsorbHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcAbsorbHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcAbsorbHistoricalDAO"));

PfcAbsorbHistoricalDAO&
PfcAbsorbHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct PfcAbsorbHistoricalDAO::isEffective : public std::unary_function<const PfcAbsorb*, bool>
{
  const DateTime _date;

  isEffective(const DateTime& date) : _date(date) {}

  bool operator()(const PfcAbsorb* rec) const
  {
    return (rec->effDate() <= _date) && _date <= rec->expireDate();
  }
};

const std::vector<PfcAbsorb*>&
PfcAbsorbHistoricalDAO::get(DeleteList& del,
                            const LocCode& pfcAirport,
                            const CarrierCode& localCarrier,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcAbsorbHistoricalKey cacheKey(pfcAirport, localCarrier);
  DAOUtils::getDateRange(ticketDate, cacheKey._c, cacheKey._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<PfcAbsorb*>* ret = new std::vector<PfcAbsorb*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<PfcAbsorb>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<PfcAbsorb>), ret->end());

  return *ret;
}

std::vector<PfcAbsorb*>*
PfcAbsorbHistoricalDAO::create(PfcAbsorbHistoricalKey key)
{
  std::vector<PfcAbsorb*>* ret = new std::vector<PfcAbsorb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetPfcAbsorbHistorical pah(dbAdapter->getAdapter());
    pah.findPfcAbsorbHistorical(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcAbsorbHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcAbsorbHistoricalDAO::destroy(const PfcAbsorbHistoricalKey, std::vector<PfcAbsorb*>* recs)
{
  std::vector<PfcAbsorb*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
PfcAbsorbHistoricalDAO::compress(const std::vector<PfcAbsorb*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcAbsorb*>*
PfcAbsorbHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcAbsorb>(compressed);
}

std::string
PfcAbsorbHistoricalDAO::_name("PfcAbsorbHistorical");
std::string
PfcAbsorbHistoricalDAO::_cacheClass("Taxes");

DAOHelper<PfcAbsorbHistoricalDAO>
PfcAbsorbHistoricalDAO::_helper(_name);

PfcAbsorbHistoricalDAO* PfcAbsorbHistoricalDAO::_instance = nullptr;

} // namespace tse
