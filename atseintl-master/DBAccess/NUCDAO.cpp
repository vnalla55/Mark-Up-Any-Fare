//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/NUCDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/Queries/QueryGetNuc.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NUCDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NUCDAO"));

NUCDAO&
NUCDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NUCInfo*>&
NUCDAO::get(DeleteList& del,
            const CurrencyCode& currency,
            const CarrierCode& carrier,
            const DateTime& date,
            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCKey key(currency, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<NUCInfo>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<NUCInfo>(date, ticketDate)));
}

NUCInfo*
getNUCFirstData(const CurrencyCode& currency,
                const CarrierCode& carrier,
                const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    NUCHistoricalDAO& hdao = NUCHistoricalDAO::instance();
    return hdao.getFirst(deleteList, currency, carrier, date, ticketDate);
  }

  NUCDAO& dao = NUCDAO::instance();
  return dao.getFirst(deleteList, currency, carrier, date, ticketDate);
}

NUCInfo*
NUCDAO::getFirst(DeleteList& del,
                 const CurrencyCode& currency,
                 const CarrierCode& carrier,
                 const DateTime& date,
                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCKey key(currency, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotEffectiveG<NUCInfo> notEffective(date, ticketDate);
  std::vector<NUCInfo*>::const_iterator i1 = ptr->begin();
  std::vector<NUCInfo*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (LIKELY(!notEffective(*i1)))
      return *i1;
  }
  return nullptr;
}

NUCKey
NUCDAO::createKey(NUCInfo* info)
{
  return NUCKey(info->_cur, info->_carrier);
}

void
NUCDAO::load()
{
  StartupLoader<QueryGetAllNucs, NUCInfo, NUCDAO>();
}

std::vector<NUCInfo*>*
NUCDAO::create(NUCKey key)
{
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneNuc nuc(dbAdapter->getAdapter());
    nuc.findNUC(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NUCDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NUCDAO::destroy(NUCKey key, std::vector<NUCInfo*>* recs)
{
  std::vector<NUCInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NUCDAO::_name("NUC");
std::string
NUCDAO::_cacheClass("Currency");

DAOHelper<NUCDAO>
NUCDAO::_helper(_name);

NUCDAO* NUCDAO::_instance = nullptr;

//
// Historical DAO Object
//

log4cxx::LoggerPtr
NUCHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NUCHistoricalDAO"));

NUCHistoricalDAO&
NUCHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NUCInfo*>&
NUCHistoricalDAO::get(DeleteList& del,
                      const CurrencyCode& currency,
                      const CarrierCode& carrier,
                      const DateTime& ticketDate,
                      const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCHistoricalKey key(currency, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveHist<NUCInfo>(date, ticketDate));
  return *ret;
}

NUCInfo*
NUCHistoricalDAO::getFirst(DeleteList& del,
                           const CurrencyCode& currency,
                           const CarrierCode& carrier,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NUCHistoricalKey key(currency, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotEffectiveHist<NUCInfo> notEffective(date, ticketDate);
  std::vector<NUCInfo*>::const_iterator i1 = ptr->begin();
  std::vector<NUCInfo*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notEffective(*i1))
      return *i1;
  }
  return nullptr;
}

std::vector<NUCInfo*>*
NUCHistoricalDAO::create(NUCHistoricalKey key)
{
  std::vector<NUCInfo*>* ret = new std::vector<NUCInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetNucHistorical nuc(dbAdapter->getAdapter());
    nuc.findNUC(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NUCHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NUCHistoricalDAO::destroy(NUCHistoricalKey key, std::vector<NUCInfo*>* recs)
{
  std::vector<NUCInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NUCHistoricalDAO::_name("NUCHistorical");
std::string
NUCHistoricalDAO::_cacheClass("Currency");

DAOHelper<NUCHistoricalDAO>
NUCHistoricalDAO::_helper(_name);

NUCHistoricalDAO* NUCHistoricalDAO::_instance = nullptr;

} // namespace tse
