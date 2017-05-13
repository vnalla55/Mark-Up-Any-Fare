//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TaxNationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxNation.h"
#include "DBAccess/TaxNation.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxNationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxNationDAO"));

TaxNationDAO&
TaxNationDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TaxNation*
getTaxNationData(const NationCode& key,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TaxNationHistoricalDAO& dao = TaxNationHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    TaxNationDAO& dao = TaxNationDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

TaxNation*
TaxNationDAO::get(DeleteList& del,
                  const NationCode& key,
                  const DateTime& date,
                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(NationKey(key));
  del.copy(ptr);
  TaxNation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<TaxNation>(date, ticketDate));
  if (LIKELY(i != ptr->end()))
    ret = *i;
  return ret;
}

std::vector<TaxNation*>*
TaxNationDAO::create(NationKey key)
{
  std::vector<TaxNation*>* ret = new std::vector<TaxNation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxNation tn(dbAdapter->getAdapter());
    tn.findTaxNation(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxNationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxNationDAO::destroy(NationKey key, std::vector<TaxNation*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
TaxNationDAO::compress(const std::vector<TaxNation*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxNation*>*
TaxNationDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxNation>(compressed);
}

std::string
TaxNationDAO::_name("TaxNation");
std::string
TaxNationDAO::_cacheClass("Taxes");

DAOHelper<TaxNationDAO>
TaxNationDAO::_helper(_name);

TaxNationDAO* TaxNationDAO::_instance = nullptr;

NationKey
TaxNationDAO::createKey(const TaxNation* info)
{
  return NationKey(info->nation());
}

void
TaxNationDAO::load()
{
  StartupLoaderNoDB<TaxNation, TaxNationDAO>();
}

// --------------------------------------------------
// Historical DAO: TaxNationHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TaxNationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxNationHistoricalDAO"));
TaxNationHistoricalDAO&
TaxNationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

TaxNation*
TaxNationHistoricalDAO::get(DeleteList& del,
                            const NationCode& code,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxNationHistoricalKey key(code);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  TaxNation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<TaxNation>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<TaxNation*>*
TaxNationHistoricalDAO::create(TaxNationHistoricalKey key)
{
  std::vector<TaxNation*>* ret = new std::vector<TaxNation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxNationHistorical tn(dbAdapter->getAdapter());
    tn.findTaxNation(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxNationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxNationHistoricalDAO::destroy(TaxNationHistoricalKey key, std::vector<TaxNation*>* recs)
{
  destroyContainer(recs);
}
sfc::CompressedData*
TaxNationHistoricalDAO::compress(const std::vector<TaxNation*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxNation*>*
TaxNationHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxNation>(compressed);
}

std::string
TaxNationHistoricalDAO::_name("TaxNationHistorical");
std::string
TaxNationHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxNationHistoricalDAO>
TaxNationHistoricalDAO::_helper(_name);

TaxNationHistoricalDAO* TaxNationHistoricalDAO::_instance = nullptr;

} // namespace tse
