//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TaxSpecConfigDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxSpecConfig.h"
#include "DBAccess/TaxSpecConfigReg.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxSpecConfigDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxSpecConfigDAO"));

TaxSpecConfigDAO&
TaxSpecConfigDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}
std::vector<TaxSpecConfigReg*>&
getTaxSpecConfigData(const TaxSpecConfigName& name,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TaxSpecConfigHistoricalDAO& dao = TaxSpecConfigHistoricalDAO::instance();
    return dao.get(deleteList, name, ticketDate);
  }
  else
  {
    TaxSpecConfigDAO& dao = TaxSpecConfigDAO::instance();
    return dao.get(deleteList, name, ticketDate);
  }
}

std::vector<TaxSpecConfigReg*>&
TaxSpecConfigDAO::get(DeleteList& del, const TaxSpecConfigName& name, const DateTime& ticketDate)
{
  DAOCache::pointer_type ptr = cache().get(TaxSpecConfigKey(name));
  del.copy(ptr);
  std::vector<TaxSpecConfigReg*>* ret = new std::vector<TaxSpecConfigReg*>;
  del.adopt(ret);

  IsCurrentG<TaxSpecConfigReg> isCurrent(ticketDate);
  std::vector<TaxSpecConfigReg*>::const_iterator i = ptr->begin();
  std::vector<TaxSpecConfigReg*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (LIKELY(isCurrent(*i)))
      ret->push_back(*i);
  }
  return *ret;
}

void
TaxSpecConfigDAO::load()
{
  StartupLoader<QueryGetAllTaxSpecConfigRegs, TaxSpecConfigReg, TaxSpecConfigDAO>();
}

TaxSpecConfigKey
TaxSpecConfigDAO::createKey(TaxSpecConfigReg* info)
{
  return TaxSpecConfigKey(info->taxSpecConfigName());
}

std::vector<TaxSpecConfigReg*>*
TaxSpecConfigDAO::create(TaxSpecConfigKey key)
{
  std::vector<TaxSpecConfigReg*>* ret = new std::vector<TaxSpecConfigReg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxSpecConfig tc(dbAdapter->getAdapter());
    tc.findTaxSpecConfigReg(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxSpecConfigDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxSpecConfigDAO::destroy(TaxSpecConfigKey key, std::vector<TaxSpecConfigReg*>* recs)
{
  destroyContainer(recs);
}

std::string
TaxSpecConfigDAO::_name("TaxSpecConfig");
std::string
TaxSpecConfigDAO::_cacheClass("Taxes");

DAOHelper<TaxSpecConfigDAO>
TaxSpecConfigDAO::_helper(_name);

TaxSpecConfigDAO* TaxSpecConfigDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxSpecConfigHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TaxSpecConfigHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxSpecConfigHistoricalDAO"));

TaxSpecConfigHistoricalDAO&
TaxSpecConfigHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<TaxSpecConfigReg*>&
TaxSpecConfigHistoricalDAO::get(DeleteList& del,
                                const TaxSpecConfigName& name,
                                const DateTime& ticketDate)
{
  TaxSpecConfigHistoricalKey key(name);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TaxSpecConfigReg*>* ret = new std::vector<TaxSpecConfigReg*>;
  del.adopt(ret);

  IsCurrentH<TaxSpecConfigReg> isCurrent(ticketDate);
  std::vector<TaxSpecConfigReg*>::const_iterator i = ptr->begin();
  std::vector<TaxSpecConfigReg*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i))
      ret->push_back(*i);
  }
  return *ret;
}

std::vector<TaxSpecConfigReg*>*
TaxSpecConfigHistoricalDAO::create(TaxSpecConfigHistoricalKey key)
{
  std::vector<TaxSpecConfigReg*>* ret = new std::vector<TaxSpecConfigReg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxSpecConfigHistorical tch(dbAdapter->getAdapter());
    tch.findTaxSpecConfigRegHistorical(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxSpecConfigHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxSpecConfigHistoricalDAO::destroy(const TaxSpecConfigHistoricalKey key,
                                    std::vector<TaxSpecConfigReg*>* recs)
{
  std::vector<TaxSpecConfigReg*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TaxSpecConfigHistoricalDAO::_name("TaxSpecConfigHistorical");
std::string
TaxSpecConfigHistoricalDAO::_cacheClass("Taxes");

DAOHelper<TaxSpecConfigHistoricalDAO>
TaxSpecConfigHistoricalDAO::_helper(_name);

TaxSpecConfigHistoricalDAO* TaxSpecConfigHistoricalDAO::_instance = nullptr;

} // namespace tse
