//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcEquipTypeExemptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcEquipTypeExempt.h"
#include "DBAccess/Queries/QueryGetPfcEquipTypeExempt.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcEquipTypeExemptDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcEquipTypeExemptDAO"));

PfcEquipTypeExemptDAO&
PfcEquipTypeExemptDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PfcEquipTypeExempt*
getPfcEquipTypeExemptData(const EquipmentType& equip,
                          const StateCode& state,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    PfcEquipTypeExemptHistoricalDAO& dao = PfcEquipTypeExemptHistoricalDAO::instance();
    return dao.get(deleteList, equip, state, date, ticketDate);
  }
  else
  {
    PfcEquipTypeExemptDAO& dao = PfcEquipTypeExemptDAO::instance();
    return dao.get(deleteList, equip, state, date, ticketDate);
  }
}

const std::vector<PfcEquipTypeExempt*>&
getAllPfcEquipTypeExemptData(DeleteList& del)
{
  PfcEquipTypeExemptDAO& dao = PfcEquipTypeExemptDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcEquipTypeExempt*>&
PfcEquipTypeExemptDAO::getAll(DeleteList& del)
{
  std::vector<PfcEquipTypeExempt*>* pfexmpV = new std::vector<PfcEquipTypeExempt*>;
  del.adopt(pfexmpV);

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  QueryGetAllPfcEquipTypeExempt pfc(dbAdapter->getAdapter());
  pfc.findAllPfcEquipTypeExempt(*pfexmpV);

  return *pfexmpV;
}

const PfcEquipTypeExempt*
PfcEquipTypeExemptDAO::get(DeleteList& del,
                           const EquipmentType& equip,
                           const StateCode& state,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcEquipTypeExemptKey key(equip, state);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  PfcEquipTypeExempt* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveG, PfcEquipTypeExempt> isApplicable(date, ticketDate);
  i = std::find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

void
PfcEquipTypeExemptDAO::load()
{
  StartupLoader<QueryGetAllPfcEquipTypeExempt, PfcEquipTypeExempt, PfcEquipTypeExemptDAO>();
}

PfcEquipTypeExemptKey
PfcEquipTypeExemptDAO::createKey(PfcEquipTypeExempt* info)
{
  return PfcEquipTypeExemptKey(info->equip(), info->state());
}

std::vector<PfcEquipTypeExempt*>*
PfcEquipTypeExemptDAO::create(PfcEquipTypeExemptKey key)
{
  std::vector<PfcEquipTypeExempt*>* ret = new std::vector<PfcEquipTypeExempt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcEquipTypeExempt pete(dbAdapter->getAdapter());
    pete.findPfcEquipTypeExempt(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcEquipTypeExemptDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcEquipTypeExemptDAO::destroy(PfcEquipTypeExemptKey key, std::vector<PfcEquipTypeExempt*>* recs)
{
  std::vector<PfcEquipTypeExempt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
PfcEquipTypeExemptDAO::_name("PfcEquipTypeExempt");
std::string
PfcEquipTypeExemptDAO::_cacheClass("Taxes");

DAOHelper<PfcEquipTypeExemptDAO>
PfcEquipTypeExemptDAO::_helper(_name);

PfcEquipTypeExemptDAO* PfcEquipTypeExemptDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcEquipTypeExemptHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcEquipTypeExemptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcEquipTypeExemptHistoricalDAO"));
PfcEquipTypeExemptHistoricalDAO&
PfcEquipTypeExemptHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PfcEquipTypeExempt*
PfcEquipTypeExemptHistoricalDAO::get(DeleteList& del,
                                     const EquipmentType& equip,
                                     const StateCode& state,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcEquipTypeExemptHistoricalKey cacheKey(equip, state);
  DAOUtils::getDateRange(ticketDate, cacheKey._c, cacheKey._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  PfcEquipTypeExempt* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveHist, PfcEquipTypeExempt> isApplicable(date, ticketDate);
  i = find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<PfcEquipTypeExempt*>*
PfcEquipTypeExemptHistoricalDAO::create(PfcEquipTypeExemptHistoricalKey key)
{
  std::vector<PfcEquipTypeExempt*>* ret = new std::vector<PfcEquipTypeExempt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcEquipTypeExemptHistorical pete(dbAdapter->getAdapter());
    pete.findPfcEquipTypeExempt(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcEquipTypeExemptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcEquipTypeExemptHistoricalDAO::destroy(PfcEquipTypeExemptHistoricalKey key,
                                         std::vector<PfcEquipTypeExempt*>* recs)
{
  std::vector<PfcEquipTypeExempt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
PfcEquipTypeExemptHistoricalDAO::_name("PfcEquipTypeExemptHistorical");
std::string
PfcEquipTypeExemptHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PfcEquipTypeExemptHistoricalDAO>
PfcEquipTypeExemptHistoricalDAO::_helper(_name);
PfcEquipTypeExemptHistoricalDAO* PfcEquipTypeExemptHistoricalDAO::_instance = nullptr;

} // namespace tse
