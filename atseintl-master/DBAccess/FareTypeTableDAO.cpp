//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/FareTypeTableDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareTypeTable.h"
#include "DBAccess/Queries/QueryGetFareTypeTable.h"

namespace tse
{
log4cxx::LoggerPtr
FareTypeTableDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeTableDAO"));

FareTypeTableDAO&
FareTypeTableDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const std::vector<FareTypeTable*>&
getFareTypeTableData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     const DateTime& applDate)
{
  if (isHistorical)
  {
    FareTypeTableHistoricalDAO& dao = FareTypeTableHistoricalDAO::instance();

    if (applDate != DateTime::emptyDate())
    {
      std::vector<FareTypeTable*>& ret = dao.get(deleteList, vendor, itemNo, applDate);
      if (ret.size() > 0)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }

  FareTypeTableDAO& dao = FareTypeTableDAO::instance();
  return dao.get(deleteList, vendor, itemNo, ticketDate);
}

std::vector<FareTypeTable*>&
FareTypeTableDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      int itemNo,
                      const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  FareTypeTableKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<FareTypeTable*>* ret = new std::vector<FareTypeTable*>;
  del.adopt(ret);

  IsCurrentG<FareTypeTable> isCurrent(ticketDate);
  std::vector<FareTypeTable*>::const_iterator i = ptr->begin();
  std::vector<FareTypeTable*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<FareTypeTable>(*i))
      ret->push_back(*i);
  }
  return *ret;
}

FareTypeTableKey
FareTypeTableDAO::createKey(FareTypeTable* info)
{
  return FareTypeTableKey(info->vendor(), info->itemNo());
}

void
FareTypeTableDAO::load()
{
  StartupLoader<QueryGetAllFareTypeTable, FareTypeTable, FareTypeTableDAO>();
}

std::vector<FareTypeTable*>*
FareTypeTableDAO::create(FareTypeTableKey key)
{
  std::vector<FareTypeTable*>* ret = new std::vector<FareTypeTable*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetFareTypeTable tc(dbAdapter->getAdapter());
    tc.findFareTypeTable(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeTableDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareTypeTableDAO::destroy(FareTypeTableKey key, std::vector<FareTypeTable*>* recs)
{
  std::vector<FareTypeTable*>::iterator i;
  for (i = recs->begin(); i != recs->end(); ++i)
    delete *i;
  delete recs;
}

std::string
FareTypeTableDAO::_name("FareTypeTable");
std::string
FareTypeTableDAO::_cacheClass("Rules");
DAOHelper<FareTypeTableDAO>
FareTypeTableDAO::_helper(_name);
FareTypeTableDAO* FareTypeTableDAO::_instance = nullptr;

log4cxx::LoggerPtr
FareTypeTableHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeTableHistoricalDAO"));
FareTypeTableHistoricalDAO&
FareTypeTableHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

std::vector<FareTypeTable*>&
FareTypeTableHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  FareTypeTableKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<FareTypeTable*>* ret = new std::vector<FareTypeTable*>;
  del.adopt(ret);

  IsCurrentH<FareTypeTable> isCurrent(ticketDate);
  std::vector<FareTypeTable*>::const_iterator i = ptr->begin();
  std::vector<FareTypeTable*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<FareTypeTable>(*i))
      ret->push_back(*i);
  }
  return *ret;
}

std::vector<FareTypeTable*>*
FareTypeTableHistoricalDAO::create(FareTypeTableKey key)
{
  std::vector<FareTypeTable*>* ret = new std::vector<FareTypeTable*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeTableHistorical query(dbAdapter->getAdapter());
    query.findFareTypeTable(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeTableHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareTypeTableHistoricalDAO::destroy(FareTypeTableKey key, std::vector<FareTypeTable*>* recs)
{
  std::vector<FareTypeTable*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
FareTypeTableHistoricalDAO::_name("FareTypeTableHistorical");
std::string
FareTypeTableHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareTypeTableHistoricalDAO>
FareTypeTableHistoricalDAO::_helper(_name);
FareTypeTableHistoricalDAO* FareTypeTableHistoricalDAO::_instance = nullptr;
}
