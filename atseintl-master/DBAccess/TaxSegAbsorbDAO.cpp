//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TaxSegAbsorbDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxSegAbsorb.h"
#include "DBAccess/TaxSegAbsorb.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxSegAbsorbDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxSegAbsorbDAO"));

TaxSegAbsorbDAO&
TaxSegAbsorbDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxSegAbsorb*>&
getTaxSegAbsorbData(const CarrierCode& carrier,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    TaxSegAbsorbHistoricalDAO& dao = TaxSegAbsorbHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    TaxSegAbsorbDAO& dao = TaxSegAbsorbDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<TaxSegAbsorb*>&
TaxSegAbsorbDAO::get(DeleteList& del,
                     const CarrierCode& key,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  std::vector<TaxSegAbsorb*>* ret = new std::vector<TaxSegAbsorb*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<TaxSegAbsorb>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<TaxSegAbsorb>), ret->end());
  return *ret;
}

CarrierKey
TaxSegAbsorbDAO::createKey(TaxSegAbsorb* info)
{
  return CarrierKey(info->carrier());
}

std::vector<TaxSegAbsorb*>*
TaxSegAbsorbDAO::create(CarrierKey key)
{
  std::vector<TaxSegAbsorb*>* ret = new std::vector<TaxSegAbsorb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxSegAbsorb tsa(dbAdapter->getAdapter());
    tsa.findTaxSegAbsorb(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxSegAbsorbDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxSegAbsorbDAO::destroy(CarrierKey key, std::vector<TaxSegAbsorb*>* recs)
{
  destroyContainer(recs);
}

void
TaxSegAbsorbDAO::load()
{
  StartupLoader<QueryGetAllTaxSegAbsorb, TaxSegAbsorb, TaxSegAbsorbDAO>();
}

std::string
TaxSegAbsorbDAO::_name("TaxSegAbsorb");
std::string
TaxSegAbsorbDAO::_cacheClass("Taxes");

DAOHelper<TaxSegAbsorbDAO>
TaxSegAbsorbDAO::_helper(_name);

TaxSegAbsorbDAO* TaxSegAbsorbDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxSegAbsorbHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TaxSegAbsorbHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxSegAbsorbHistoricalDAO"));
TaxSegAbsorbHistoricalDAO&
TaxSegAbsorbHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxSegAbsorb*>&
TaxSegAbsorbHistoricalDAO::get(DeleteList& del,
                               const CarrierCode& key,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxSegAbsorbHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<TaxSegAbsorb*>* ret = new std::vector<TaxSegAbsorb*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TaxSegAbsorb>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<TaxSegAbsorb>), ret->end());
  return *ret;
}

std::vector<TaxSegAbsorb*>*
TaxSegAbsorbHistoricalDAO::create(TaxSegAbsorbHistoricalKey key)
{
  std::vector<TaxSegAbsorb*>* ret = new std::vector<TaxSegAbsorb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxSegAbsorbHistorical tsa(dbAdapter->getAdapter());
    tsa.findTaxSegAbsorb(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxSegAbsorbHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxSegAbsorbHistoricalDAO::destroy(TaxSegAbsorbHistoricalKey key, std::vector<TaxSegAbsorb*>* recs)
{
  destroyContainer(recs);
}

std::string
TaxSegAbsorbHistoricalDAO::_name("TaxSegAbsorbHistorical");
std::string
TaxSegAbsorbHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxSegAbsorbHistoricalDAO>
TaxSegAbsorbHistoricalDAO::_helper(_name);

TaxSegAbsorbHistoricalDAO* TaxSegAbsorbHistoricalDAO::_instance = nullptr;

} // namespace tse
