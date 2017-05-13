//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/IndustryPricingApplDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Queries/QueryGetIndPriceAppl.h"

namespace tse
{
log4cxx::LoggerPtr
IndustryPricingApplDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryPricingApplDAO"));

IndustryPricingApplDAO&
IndustryPricingApplDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct IndustryPricingApplDAO::IsNotGlobalDir
{
  const GlobalDirection _globalDir;
  IsNotEffectiveG<IndustryPricingAppl> dateCheck;

  IsNotGlobalDir(const GlobalDirection& globalDir, const DateTime& date, const DateTime& ticketDate)
    : _globalDir(globalDir), dateCheck(date, ticketDate)
  {
  }

  bool operator()(const IndustryPricingAppl* rec) const
  {
    if (rec->globalDir() != GlobalDirection::ZZ && rec->globalDir() != _globalDir)
      return true;
    return dateCheck(rec);
  }
};

const std::vector<const IndustryPricingAppl*>&
getIndustryPricingApplData(const CarrierCode& carrier,
                           const GlobalDirection& globalDir,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    IndustryPricingApplHistoricalDAO& dao = IndustryPricingApplHistoricalDAO::instance();
    return dao.get(deleteList, carrier, globalDir, date, ticketDate);
  }
  else
  {
    IndustryPricingApplDAO& dao = IndustryPricingApplDAO::instance();
    return dao.get(deleteList, carrier, globalDir, date, ticketDate);
  }
}

const std::vector<const IndustryPricingAppl*>&
IndustryPricingApplDAO::get(DeleteList& del,
                            const CarrierCode& carrier,
                            const GlobalDirection& globalDir,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  const std::vector<const IndustryPricingAppl*>* ret;
  IsNotGlobalDir globalDirCheck(globalDir, date, ticketDate);

  DAOCache::pointer_type ptr = cache().get(CarrierKey(carrier));
  ret = applyFilter(del, ptr, globalDirCheck);

  if (ret->empty() && !carrier.empty())
  {
    ptr = cache().get(CarrierKey(""));
    ret = applyFilter(del, ptr, globalDirCheck);
  }
  return *ret;
}

std::vector<const IndustryPricingAppl*>*
IndustryPricingApplDAO::create(CarrierKey key)
{
  std::vector<const IndustryPricingAppl*>* ret = new std::vector<const IndustryPricingAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndPriceAppl ipa(dbAdapter->getAdapter());
    ipa.findIndustryPricingAppl(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryPricingApplDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IndustryPricingApplDAO::destroy(CarrierKey key, std::vector<const IndustryPricingAppl*>* recs)
{
  destroyContainer(recs);
}

CarrierKey
IndustryPricingApplDAO::createKey(const IndustryPricingAppl* info)
{
  return CarrierKey(info->carrier());
}

void
IndustryPricingApplDAO::load()
{
  StartupLoader<QueryGetAllIndPriceAppl, IndustryPricingAppl, IndustryPricingApplDAO>();
}

std::string
IndustryPricingApplDAO::_name("IndustryPricingAppl");
std::string
IndustryPricingApplDAO::_cacheClass("Fares");
DAOHelper<IndustryPricingApplDAO>
IndustryPricingApplDAO::_helper(_name);
IndustryPricingApplDAO* IndustryPricingApplDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: IndustryPricingApplHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
IndustryPricingApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryPricingApplHistoricalDAO"));
IndustryPricingApplHistoricalDAO&
IndustryPricingApplHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct IndustryPricingApplHistoricalDAO::IsNotGlobalDir
{
  const GlobalDirection _globalDir;

  IsNotGlobalDir(const GlobalDirection& globalDir) : _globalDir(globalDir) {}

  bool operator()(const IndustryPricingAppl* rec) const
  {
    return rec->globalDir() != GlobalDirection::ZZ && rec->globalDir() != _globalDir;
  }
};

const std::vector<const IndustryPricingAppl*>&
IndustryPricingApplHistoricalDAO::get(DeleteList& del,
                                      const CarrierCode& carrier,
                                      const GlobalDirection& globalDir,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsNotEffectiveH<IndustryPricingAppl> dateCheck(date, ticketDate);
  std::vector<const IndustryPricingAppl*>* ret = new std::vector<const IndustryPricingAppl*>;
  del.adopt(ret);

  DAOCache::pointer_type ptr = cache().get(carrier);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), dateCheck);
  ret->erase(std::remove_if(ret->begin(), ret->end(), IsNotGlobalDir(globalDir)), ret->end());

  if (ret->empty() && !carrier.empty())
  {
    ptr = cache().get("");
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), dateCheck);
    ret->erase(std::remove_if(ret->begin(), ret->end(), IsNotGlobalDir(globalDir)), ret->end());
  }

  if (!ret->empty())
    del.copy(ptr);
  return *ret;
}

std::vector<IndustryPricingAppl*>*
IndustryPricingApplHistoricalDAO::create(CarrierCode key)
{
  std::vector<IndustryPricingAppl*>* ret = new std::vector<IndustryPricingAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndPriceApplHistorical ipa(dbAdapter->getAdapter());
    ipa.findIndustryPricingAppl(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryPricingApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IndustryPricingApplHistoricalDAO::destroy(CarrierCode key, std::vector<IndustryPricingAppl*>* recs)
{
  destroyContainer(recs);
}

struct IndustryPricingApplHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(IndustryPricingApplHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<IndustryPricingAppl*>* ptr;

  void operator()(IndustryPricingAppl* info)
  {
    CarrierCode key(info->carrier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<IndustryPricingAppl*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
IndustryPricingApplHistoricalDAO::load()
{
  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<IndustryPricingAppl*> vector;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIndPriceApplHistorical ipa(dbAdapter->getAdapter());
    ipa.findAllIndustryPricingAppl(vector);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryPricingApplHistoricalDAO::load");
    deleteVectorOfPointers(vector);
    throw;
  }

  std::for_each(vector.begin(), vector.end(), groupByKey());
}

std::string
IndustryPricingApplHistoricalDAO::_name("IndustryPricingApplHistorical");
std::string
IndustryPricingApplHistoricalDAO::_cacheClass("Fares");
DAOHelper<IndustryPricingApplHistoricalDAO>
IndustryPricingApplHistoricalDAO::_helper(_name);
IndustryPricingApplHistoricalDAO* IndustryPricingApplHistoricalDAO::_instance = nullptr;

} // namespace tse
