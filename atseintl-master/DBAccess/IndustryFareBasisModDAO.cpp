//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/IndustryFareBasisModDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/IndustryFareBasisMod.h"
#include "DBAccess/Queries/QueryGetIndFareBasisMod.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
IndustryFareBasisModDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryFareBasisModDAO"));

IndustryFareBasisModDAO&
IndustryFareBasisModDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const IndustryFareBasisMod*>&
getIndustryFareBasisModData(const CarrierCode& carrier,
                            Indicator userApplType,
                            const UserApplCode& userAppl,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    IndustryFareBasisModHistoricalDAO& dao = IndustryFareBasisModHistoricalDAO::instance();
    return dao.get(deleteList, carrier, userApplType, userAppl, date, ticketDate);
  }
  else
  {
    IndustryFareBasisModDAO& dao = IndustryFareBasisModDAO::instance();
    return dao.get(deleteList, carrier, userApplType, userAppl, date, ticketDate);
  }
}

const std::vector<const IndustryFareBasisMod*>&
IndustryFareBasisModDAO::get(DeleteList& del,
                             const CarrierCode& carrier,
                             const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IndustryFareBasisModKey key(carrier, userApplType, userAppl);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const IndustryFareBasisMod*>* ret = new std::vector<const IndustryFareBasisMod*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<IndustryFareBasisMod>(date, ticketDate));
  if (LIKELY(carrier != ""))
  {
    key = IndustryFareBasisModKey("", userApplType, userAppl);
    ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<IndustryFareBasisMod>(date, ticketDate));
  }
  if (UNLIKELY(userAppl != ""))
  {
    key = IndustryFareBasisModKey(carrier, userApplType, "");
    ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<IndustryFareBasisMod>(date, ticketDate));
    if (carrier != "")
    {
      key = IndustryFareBasisModKey("", userApplType, "");
      ptr = cache().get(key);
      del.copy(ptr);
      remove_copy_if(ptr->begin(),
                     ptr->end(),
                     back_inserter(*ret),
                     IsNotEffectiveG<IndustryFareBasisMod>(date, ticketDate));
    }
  }
  return *ret;
}

IndustryFareBasisModKey
IndustryFareBasisModDAO::createKey(IndustryFareBasisMod* info)
{
  return IndustryFareBasisModKey(info->carrier(), info->userApplType(), info->userAppl());
}

std::vector<IndustryFareBasisMod*>*
IndustryFareBasisModDAO::create(IndustryFareBasisModKey key)
{
  std::vector<IndustryFareBasisMod*>* ret = new std::vector<IndustryFareBasisMod*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndFareBasisMod ifbm(dbAdapter->getAdapter());
    ifbm.findIndustryFareBasisMod(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    destroyContainer(ret);
    LOG4CXX_WARN(_logger, "DB exception in IndustryFareBasisModDAO::create");
    throw;
  }

  return ret;
}

void
IndustryFareBasisModDAO::destroy(IndustryFareBasisModKey key,
                                 std::vector<IndustryFareBasisMod*>* recs)
{
  destroyContainer(recs);
}

void
IndustryFareBasisModDAO::load()
{
  StartupLoader<QueryGetAllIndFareBasisMod, IndustryFareBasisMod, IndustryFareBasisModDAO>();
}

std::string
IndustryFareBasisModDAO::_name("IndustryFareBasisMod");
std::string
IndustryFareBasisModDAO::_cacheClass("Fares");

DAOHelper<IndustryFareBasisModDAO>
IndustryFareBasisModDAO::_helper(_name);

IndustryFareBasisModDAO* IndustryFareBasisModDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: IndustryFareBasisModHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
IndustryFareBasisModHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryFareBasisModHistoricalDAO"));
IndustryFareBasisModHistoricalDAO&
IndustryFareBasisModHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const IndustryFareBasisMod*>&
IndustryFareBasisModHistoricalDAO::get(DeleteList& del,
                                       const CarrierCode& carrier,
                                       const Indicator& userApplType,
                                       const UserApplCode& userAppl,
                                       const DateTime& date,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IndustryFareBasisModKey key(carrier, userApplType, userAppl);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const IndustryFareBasisMod*>* ret = new std::vector<const IndustryFareBasisMod*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<IndustryFareBasisMod>(date, ticketDate));
  if (carrier != "")
  {
    key = IndustryFareBasisModKey("", userApplType, userAppl);
    ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<IndustryFareBasisMod>(date, ticketDate));
  }
  if (userAppl != "")
  {
    key = IndustryFareBasisModKey(carrier, userApplType, "");
    ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<IndustryFareBasisMod>(date, ticketDate));
    if (carrier != "")
    {
      key = IndustryFareBasisModKey("", userApplType, "");
      ptr = cache().get(key);
      del.copy(ptr);
      remove_copy_if(ptr->begin(),
                     ptr->end(),
                     back_inserter(*ret),
                     IsNotEffectiveHist<IndustryFareBasisMod>(date, ticketDate));
    }
  }
  return *ret;
}

std::vector<IndustryFareBasisMod*>*
IndustryFareBasisModHistoricalDAO::create(IndustryFareBasisModKey key)
{
  std::vector<IndustryFareBasisMod*>* ret = new std::vector<IndustryFareBasisMod*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndFareBasisModHistorical ifbm(dbAdapter->getAdapter());
    ifbm.findIndustryFareBasisMod(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryFareBasisModHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IndustryFareBasisModHistoricalDAO::destroy(IndustryFareBasisModKey key,
                                           std::vector<IndustryFareBasisMod*>* recs)
{
  destroyContainer(recs);
}

struct IndustryFareBasisModHistoricalDAO::groupByKey
{
public:
  IndustryFareBasisModKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(IndustryFareBasisModHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<IndustryFareBasisMod*>* ptr;

  void operator()(IndustryFareBasisMod* info)
  {
    IndustryFareBasisModKey key(info->carrier(), info->userApplType(), info->userAppl());
    if (!(key == prevKey))
    {
      ptr = new std::vector<IndustryFareBasisMod*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
IndustryFareBasisModHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<IndustryFareBasisMod*> vector;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIndFareBasisModHistorical ifbm(dbAdapter->getAdapter());
    ifbm.findAllIndustryFareBasisMod(vector);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryFareBasisModHistoricalDAO::load");
    deleteVectorOfPointers(vector);
    throw;
  }

  std::for_each(vector.begin(), vector.end(), groupByKey());
}

std::string
IndustryFareBasisModHistoricalDAO::_name("IndustryFareBasisModHistorical");
std::string
IndustryFareBasisModHistoricalDAO::_cacheClass("Fares");
DAOHelper<IndustryFareBasisModHistoricalDAO>
IndustryFareBasisModHistoricalDAO::_helper(_name);

IndustryFareBasisModHistoricalDAO* IndustryFareBasisModHistoricalDAO::_instance = nullptr;

} // namespace tse
