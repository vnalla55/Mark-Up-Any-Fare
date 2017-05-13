//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ValueCodeAlgorithmDAO.h"

#include "DBAccess/Queries/QueryGetValueCodeAlgorithm.h"
#include "DBAccess/ValueCodeAlgorithm.h"

namespace tse
{
log4cxx::LoggerPtr
ValueCodeAlgorithmDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ValueCodeAlgorithmDAO"));

std::string
ValueCodeAlgorithmDAO::_name("ValueCodeAlgorithm");
std::string
ValueCodeAlgorithmDAO::_cacheClass("Rules");
DAOHelper<ValueCodeAlgorithmDAO>
ValueCodeAlgorithmDAO::_helper(_name);
ValueCodeAlgorithmDAO* ValueCodeAlgorithmDAO::_instance = nullptr;

ValueCodeAlgorithmDAO&
ValueCodeAlgorithmDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ValueCodeAlgorithm*
getValueCodeAlgorithmData(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const std::string& name,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    ValueCodeAlgorithmHistoricalDAO& dao = ValueCodeAlgorithmHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, name, date, ticketDate);
  }
  else
  {
    ValueCodeAlgorithmDAO& dao = ValueCodeAlgorithmDAO::instance();
    return dao.get(deleteList, vendor, carrier, name, date, ticketDate);
  }
}

const ValueCodeAlgorithm*
ValueCodeAlgorithmDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const std::string& name,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ValueCodeAlgorithmKey key(vendor, carrier, name);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsEffectiveG<ValueCodeAlgorithm> isEff(date, ticketDate);
  for (const auto elem : *ptr)
  {
    if (isEff(elem))
      return elem;
  }
  return nullptr;
}

std::vector<ValueCodeAlgorithm*>*
ValueCodeAlgorithmDAO::create(ValueCodeAlgorithmKey key)
{
  std::vector<ValueCodeAlgorithm*>* ret = new std::vector<ValueCodeAlgorithm*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetValueCodeAlgorithm fnc(dbAdapter->getAdapter());
    fnc.findValueCodeAlgorithm(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ValueCodeAlgorithmDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ValueCodeAlgorithmDAO::destroy(ValueCodeAlgorithmKey key, std::vector<ValueCodeAlgorithm*>* recs)
{
  destroyContainer(recs);
}

ValueCodeAlgorithmKey
ValueCodeAlgorithmDAO::createKey(ValueCodeAlgorithm* info)
{
  return ValueCodeAlgorithmKey(info->vendor(), info->carrier(), info->algorithmName());
}

void
ValueCodeAlgorithmDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<ValueCodeAlgorithm, ValueCodeAlgorithmDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
ValueCodeAlgorithmHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ValueCodeAlgorithmHistoricalDAO"));

std::string
ValueCodeAlgorithmHistoricalDAO::_name("ValueCodeAlgorithmHistorical");
std::string
ValueCodeAlgorithmHistoricalDAO::_cacheClass("Rules");
DAOHelper<ValueCodeAlgorithmHistoricalDAO>
ValueCodeAlgorithmHistoricalDAO::_helper(_name);
ValueCodeAlgorithmHistoricalDAO* ValueCodeAlgorithmHistoricalDAO::_instance = nullptr;

ValueCodeAlgorithmHistoricalDAO&
ValueCodeAlgorithmHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ValueCodeAlgorithm*
ValueCodeAlgorithmHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     const CarrierCode& carrier,
                                     const std::string& name,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ValueCodeAlgorithmHistoricalKey key(vendor, carrier, name);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsEffectiveH<ValueCodeAlgorithm> isEff(date, ticketDate);
  for (const auto elem : *ptr)
  {
    if (isEff(elem))
      return elem;
  }
  return nullptr;
}

std::vector<ValueCodeAlgorithm*>*
ValueCodeAlgorithmHistoricalDAO::create(ValueCodeAlgorithmHistoricalKey key)
{
  std::vector<ValueCodeAlgorithm*>* ret = new std::vector<ValueCodeAlgorithm*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetValueCodeAlgorithmHistorical fnc(dbAdapter->getAdapter());
    fnc.findValueCodeAlgorithm(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ValueCodeAlgorithmHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ValueCodeAlgorithmHistoricalDAO::destroy(ValueCodeAlgorithmHistoricalKey key,
                                         std::vector<ValueCodeAlgorithm*>* recs)
{
  std::vector<ValueCodeAlgorithm*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
} // tse
