//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/SeasonalityDOWDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSeasonalityDOW.h"
#include "DBAccess/SeasonalityDOW.h"

namespace tse
{
log4cxx::LoggerPtr
SeasonalityDOWDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SeasonalityDOWDAO"));

SeasonalityDOWDAO&
SeasonalityDOWDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const SeasonalityDOW*
getSeasonalityDOWData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      const DateTime& applDate)
{
  if (isHistorical)
  {
    SeasonalityDOWHistoricalDAO& dao = SeasonalityDOWHistoricalDAO::instance();

    if (applDate != DateTime::emptyDate())
    {
      const SeasonalityDOW* ret = dao.get(deleteList, vendor, itemNo, applDate);
      if (ret)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }

  SeasonalityDOWDAO& dao = SeasonalityDOWDAO::instance();
  return dao.get(deleteList, vendor, itemNo, ticketDate);
}

const SeasonalityDOW*
SeasonalityDOWDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  SeasonalityDOWKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsCurrentG<SeasonalityDOW> isCurrent(ticketDate);
  std::vector<SeasonalityDOW*>::const_iterator i = ptr->begin();
  std::vector<SeasonalityDOW*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<SeasonalityDOW>(*i))
      return *i;
  }
  return nullptr;
}

SeasonalityDOWKey
SeasonalityDOWDAO::createKey(SeasonalityDOW* info)
{
  return SeasonalityDOWKey(info->vendor(), info->itemNo());
}

void
SeasonalityDOWDAO::load()
{
  StartupLoader<QueryGetAllSeasonalityDOW, SeasonalityDOW, SeasonalityDOWDAO>();
}

std::vector<SeasonalityDOW*>*
SeasonalityDOWDAO::create(SeasonalityDOWKey key)
{
  std::vector<SeasonalityDOW*>* ret = new std::vector<SeasonalityDOW*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetSeasonalityDOW tc(dbAdapter->getAdapter());
    tc.findSeasonalityDOW(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeasonalityDOWDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SeasonalityDOWDAO::destroy(SeasonalityDOWKey key, std::vector<SeasonalityDOW*>* recs)
{
  std::vector<SeasonalityDOW*>::iterator i;
  for (i = recs->begin(); i != recs->end(); ++i)
    delete *i;
  delete recs;
}

std::string
SeasonalityDOWDAO::_name("SeasonalityDOW");
std::string
SeasonalityDOWDAO::_cacheClass("Rules");
DAOHelper<SeasonalityDOWDAO>
SeasonalityDOWDAO::_helper(_name);
SeasonalityDOWDAO* SeasonalityDOWDAO::_instance = nullptr;

log4cxx::LoggerPtr
SeasonalityDOWHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SeasonalityDOWHistoricalDAO"));
SeasonalityDOWHistoricalDAO&
SeasonalityDOWHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const SeasonalityDOW*
SeasonalityDOWHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  SeasonalityDOWKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsCurrentH<SeasonalityDOW> isCurrent(ticketDate);
  std::vector<SeasonalityDOW*>::const_iterator i = ptr->begin();
  std::vector<SeasonalityDOW*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<SeasonalityDOW>(*i))
      return *i;
  }
  return nullptr;
}

std::vector<SeasonalityDOW*>*
SeasonalityDOWHistoricalDAO::create(SeasonalityDOWKey key)
{
  std::vector<SeasonalityDOW*>* ret = new std::vector<SeasonalityDOW*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSeasonalityDOWHistorical query(dbAdapter->getAdapter());
    query.findSeasonalityDOW(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeasonalityDOWHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SeasonalityDOWHistoricalDAO::destroy(SeasonalityDOWKey key, std::vector<SeasonalityDOW*>* recs)
{
  std::vector<SeasonalityDOW*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SeasonalityDOWHistoricalDAO::_name("SeasonalityDOWHistorical");
std::string
SeasonalityDOWHistoricalDAO::_cacheClass("Rules");
DAOHelper<SeasonalityDOWHistoricalDAO>
SeasonalityDOWHistoricalDAO::_helper(_name);
SeasonalityDOWHistoricalDAO* SeasonalityDOWHistoricalDAO::_instance = nullptr;
}
