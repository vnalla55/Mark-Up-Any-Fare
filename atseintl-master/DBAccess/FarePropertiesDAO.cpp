//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FarePropertiesDAO.h"

#include "DBAccess/FareProperties.h"
#include "DBAccess/Queries/QueryGetFareProperties.h"

namespace tse
{
log4cxx::LoggerPtr
FarePropertiesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FarePropertiesDAO"));

std::string
FarePropertiesDAO::_name("FareProperties");
std::string
FarePropertiesDAO::_cacheClass("Fares");
DAOHelper<FarePropertiesDAO>
FarePropertiesDAO::_helper(_name);
FarePropertiesDAO* FarePropertiesDAO::_instance = nullptr;

FarePropertiesDAO&
FarePropertiesDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FareProperties*
getFarePropertiesData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariff,
                      const RuleNumber& rule,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    FarePropertiesHistoricalDAO& dao = FarePropertiesHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, tariff, rule, ticketDate);
  }
  else
  {
    FarePropertiesDAO& dao = FarePropertiesDAO::instance();
    return dao.get(deleteList, vendor, carrier, tariff, rule, ticketDate);
  }
}

const FareProperties*
FarePropertiesDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariff,
                       const RuleNumber& rule,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FarePropertiesKey key(vendor, carrier, tariff, rule);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsCurrentG<FareProperties> isCurrent(ticketDate);
  for (const auto elem : *ptr)
  {
    if (isCurrent(elem))
      return elem;
  }
  return nullptr;
}

std::vector<FareProperties*>*
FarePropertiesDAO::create(FarePropertiesKey key)
{
  std::vector<FareProperties*>* ret = new std::vector<FareProperties*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareProperties fnc(dbAdapter->getAdapter());
    fnc.findFareProperties(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FarePropertiesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FarePropertiesDAO::destroy(FarePropertiesKey key, std::vector<FareProperties*>* recs)
{
  destroyContainer(recs);
}

FarePropertiesKey
FarePropertiesDAO::createKey(FareProperties* info)
{
  return FarePropertiesKey(info->vendor(), info->carrier(), info->ruleTariff(), info->rule());
}

void
FarePropertiesDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<FareProperties, FarePropertiesDAO>();
}

sfc::CompressedData*
FarePropertiesDAO::compress(const std::vector<FareProperties*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareProperties*>*
FarePropertiesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareProperties>(compressed);
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
FarePropertiesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FarePropertiesHistoricalDAO"));

std::string
FarePropertiesHistoricalDAO::_name("FarePropertiesHistorical");
std::string
FarePropertiesHistoricalDAO::_cacheClass("Fares");
DAOHelper<FarePropertiesHistoricalDAO>
FarePropertiesHistoricalDAO::_helper(_name);
FarePropertiesHistoricalDAO* FarePropertiesHistoricalDAO::_instance = nullptr;

FarePropertiesHistoricalDAO&
FarePropertiesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FareProperties*
FarePropertiesHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariff,
                                 const RuleNumber& rule,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FarePropertiesHistoricalKey key(vendor, carrier, tariff, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;
  del.copy(ptr);

  IsCurrentH<FareProperties> isCurrent(ticketDate);
  for (const auto elem : *ptr)
  {
    if (isCurrent(elem))
      return elem;
  }
  return nullptr;
}

std::vector<FareProperties*>*
FarePropertiesHistoricalDAO::create(FarePropertiesHistoricalKey key)
{
  std::vector<FareProperties*>* ret = new std::vector<FareProperties*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFarePropertiesHistorical fnc(dbAdapter->getAdapter());
    fnc.findFareProperties(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FarePropertiesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FarePropertiesHistoricalDAO::destroy(FarePropertiesHistoricalKey key,
                                     std::vector<FareProperties*>* recs)
{
  std::vector<FareProperties*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
FarePropertiesHistoricalDAO::compress(const std::vector<FareProperties*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareProperties*>*
FarePropertiesHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareProperties>(compressed);
}

} // tse
