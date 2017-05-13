//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcTktDesigExceptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcTktDesigExcept.h"
#include "DBAccess/Queries/QueryGetPfcTktDesigExcept.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcTktDesigExceptDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcTktDesigExceptDAO"));

PfcTktDesigExceptDAO&
PfcTktDesigExceptDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct PfcTktDesigExceptDAO::isEffective
    : public std::unary_function<const PfcTktDesigExcept*, bool>
{
  const DateTime _date;

  isEffective(const DateTime& date) : _date(date) {}

  bool operator()(const PfcTktDesigExcept* rec) const
  {
    return (rec->effDate() <= _date) && _date <= rec->expireDate();
  }
};

const std::vector<const PfcTktDesigExcept*>&
getPfcTktDesigExceptData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    PfcTktDesigExceptHistoricalDAO& dao = PfcTktDesigExceptHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    PfcTktDesigExceptDAO& dao = PfcTktDesigExceptDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<const PfcTktDesigExcept*>&
PfcTktDesigExceptDAO::get(DeleteList& del,
                          const CarrierCode& carrier,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcTktDesigExceptKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<const PfcTktDesigExcept*>* ret = new std::vector<const PfcTktDesigExcept*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotEffectiveG<PfcTktDesigExcept>(date, ticketDate));

// Right now there is no Inhibit field, expect it to change
//    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<PfcTktDesigExcept>),ret->end());

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<PfcTktDesigExcept>(date, ticketDate)));
}

std::vector<const PfcTktDesigExcept*>*
PfcTktDesigExceptDAO::create(PfcTktDesigExceptKey key)
{
  std::vector<const PfcTktDesigExcept*>* ret = new std::vector<const PfcTktDesigExcept*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcTktDesigExcept tde(dbAdapter->getAdapter());
    tde.findPfcTktDesigExcept(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcTktDesigExceptDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcTktDesigExceptDAO::destroy(PfcTktDesigExceptKey key, std::vector<const PfcTktDesigExcept*>* recs)
{
  std::vector<const PfcTktDesigExcept*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
PfcTktDesigExceptDAO::_name("PfcTktDesigExcept");
std::string
PfcTktDesigExceptDAO::_cacheClass("Rules");
DAOHelper<PfcTktDesigExceptDAO>
PfcTktDesigExceptDAO::_helper(_name);
PfcTktDesigExceptDAO* PfcTktDesigExceptDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcTktDesigExceptHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcTktDesigExceptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcTktDesigExceptHistoricalDAO"));
PfcTktDesigExceptHistoricalDAO&
PfcTktDesigExceptHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct PfcTktDesigExceptHistoricalDAO::isEffective
    : public std::unary_function<const PfcTktDesigExcept*, bool>
{
  const DateTime _date;

  isEffective(const DateTime& date) : _date(date) {}

  bool operator()(const PfcTktDesigExcept* rec) const
  {
    return (rec->effDate() <= _date) && _date <= rec->expireDate();
  }
};

const std::vector<const PfcTktDesigExcept*>&
PfcTktDesigExceptHistoricalDAO::get(DeleteList& del,
                                    const CarrierCode& carrier,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcTktDesigExceptHistoricalKey key(carrier);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const PfcTktDesigExcept*>* ret = new std::vector<const PfcTktDesigExcept*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<PfcTktDesigExcept>(date, ticketDate));

  // Right now there is no Inhibit field, expect it to change
  //    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<PfcTktDesigExcept>),ret->end());

  return *ret;
}

std::vector<const PfcTktDesigExcept*>*
PfcTktDesigExceptHistoricalDAO::create(PfcTktDesigExceptHistoricalKey key)
{
  std::vector<const PfcTktDesigExcept*>* ret = new std::vector<const PfcTktDesigExcept*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcTktDesigExceptHistorical tde(dbAdapter->getAdapter());
    tde.findPfcTktDesigExcept(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcTktDesigExceptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcTktDesigExceptHistoricalDAO::destroy(PfcTktDesigExceptHistoricalKey key,
                                        std::vector<const PfcTktDesigExcept*>* recs)
{
  std::vector<const PfcTktDesigExcept*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
PfcTktDesigExceptHistoricalDAO::_name("PfcTktDesigExceptHistorical");
std::string
PfcTktDesigExceptHistoricalDAO::_cacheClass("Rules");
DAOHelper<PfcTktDesigExceptHistoricalDAO>
PfcTktDesigExceptHistoricalDAO::_helper(_name);
PfcTktDesigExceptHistoricalDAO* PfcTktDesigExceptHistoricalDAO::_instance = nullptr;

} // namespace tse
