//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/DifferentialsDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Differentials.h"
#include "DBAccess/Queries/QueryGetDifferentials.h"

namespace tse
{
log4cxx::LoggerPtr
DifferentialsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DifferentialsDAO"));
DifferentialsDAO&
DifferentialsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct DifferentialsDAO::isCalc : public std::unary_function<const Differentials*, bool>
{
  bool operator()(const Differentials* rec) const { return (rec->calculationInd() != 'N'); }
};

const std::vector<Differentials*>&
getDifferentialsData(const CarrierCode& key,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    DifferentialsHistoricalDAO& dao = DifferentialsHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    DifferentialsDAO& dao = DifferentialsDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<Differentials*>&
DifferentialsDAO::get(DeleteList& del,
                      const CarrierCode& key,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  DAOCache::pointer_type ptrYY = cache().get(CarrierKey(""));
  del.copy(ptrYY);
  std::vector<Differentials*>* ret = new std::vector<Differentials*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isCalc());
  remove_copy_if(ptrYY->begin(), ptrYY->end(), back_inserter(*ret), isCalc());
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isCalc()));
  remove_copy_if(ptrYY->begin(), ptrYY->end(), back_inserter(*ret), not1(isCalc()));
  ret->erase(
      std::remove_if(ret->begin(), ret->end(), IsNotEffectiveG<Differentials>(date, ticketDate)),
      ret->end());
  return *ret;
}

CarrierKey
DifferentialsDAO::createKey(Differentials* info)
{
  return CarrierKey(info->carrier());
}

std::vector<Differentials*>*
DifferentialsDAO::create(CarrierKey key)
{
  std::vector<Differentials*>* ret = new std::vector<Differentials*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDifferentials diff(dbAdapter->getAdapter());
    diff.findDifferentials(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DifferentialsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DifferentialsDAO::destroy(CarrierKey key, std::vector<Differentials*>* recs)
{
  destroyContainer(recs);
}

void
DifferentialsDAO::load()
{
  StartupLoader<QueryGetAllDifferentials, Differentials, DifferentialsDAO>();
}

std::string
DifferentialsDAO::_name("Differentials");
std::string
DifferentialsDAO::_cacheClass("BookingCode");
DAOHelper<DifferentialsDAO>
DifferentialsDAO::_helper(_name);
DifferentialsDAO* DifferentialsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: DifferentialsHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
DifferentialsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DifferentialsHistoricalDAO"));
DifferentialsHistoricalDAO&
DifferentialsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct DifferentialsHistoricalDAO::isCalc : public std::unary_function<const Differentials*, bool>
{
  bool operator()(const Differentials* rec) const { return (rec->calculationInd() != 'N'); }
};

const std::vector<Differentials*>&
DifferentialsHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& key,
                                const DateTime& date,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::pointer_type ptrYY = cache().get("");
  del.copy(ptrYY);
  std::vector<Differentials*>* ret = new std::vector<Differentials*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isCalc());
  remove_copy_if(ptrYY->begin(), ptrYY->end(), back_inserter(*ret), isCalc());
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isCalc()));
  remove_copy_if(ptrYY->begin(), ptrYY->end(), back_inserter(*ret), not1(isCalc()));
  ret->erase(
      std::remove_if(ret->begin(), ret->end(), IsNotEffectiveHist<Differentials>(date, ticketDate)),
      ret->end());
  return *ret;
}

std::vector<Differentials*>*
DifferentialsHistoricalDAO::create(CarrierCode key)
{
  std::vector<Differentials*>* ret = new std::vector<Differentials*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDifferentialsHistorical diff(dbAdapter->getAdapter());
    diff.findDifferentials(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DifferentialsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DifferentialsHistoricalDAO::destroy(CarrierCode key, std::vector<Differentials*>* recs)
{
  destroyContainer(recs);
}

struct DifferentialsHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;
  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE), cache(DifferentialsHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<Differentials*>* ptr;

  void operator()(Differentials* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<Differentials*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
DifferentialsHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<Differentials*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllDifferentialsHistorical diff(dbAdapter->getAdapter());
    diff.findAllDifferentials(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DifferentialsHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
DifferentialsHistoricalDAO::_name("DifferentialsHistorical");
std::string
DifferentialsHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<DifferentialsHistoricalDAO>
DifferentialsHistoricalDAO::_helper(_name);
DifferentialsHistoricalDAO* DifferentialsHistoricalDAO::_instance = nullptr;

} // namespace tse
