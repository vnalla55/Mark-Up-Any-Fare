//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/BrandedFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BrandedFare.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBrandedFare.h"

namespace tse
{
log4cxx::LoggerPtr
BrandedFareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BrandedFareDAO"));

BrandedFareDAO&
BrandedFareDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BrandedFare*>&
getBrandedFareData(const VendorCode& vendor,
                   const CarrierCode& carrier,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    BrandedFareHistoricalDAO& dao(BrandedFareHistoricalDAO::instance());

    const std::vector<BrandedFare*>& ret(dao.get(deleteList, vendor, carrier, ticketDate));

    return ret;
  }
  else
  {
    BrandedFareDAO& dao(BrandedFareDAO::instance());

    const std::vector<BrandedFare*>& ret(dao.get(deleteList, vendor, carrier, ticketDate));

    return ret;
  }
}

const std::vector<BrandedFare*>&
BrandedFareDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  BrandedFareKey key(vendor, carrier);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<BrandedFare> isNotApplicable(ticketDate);
  return *(applyFilter(del, ptr, isNotApplicable));
}

std::vector<BrandedFare*>*
BrandedFareDAO::create(BrandedFareKey key)
{
  std::vector<BrandedFare*>* ret(new std::vector<BrandedFare*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetBrandedFare q(dbAdapter->getAdapter());
    // key._a = "ATP";// test !!!!
    // key._b = "VA";// test !!!!
    q.findBrandedFare(*ret, key._a, key._b);
    // std::cerr << "  !!!!!!!!!QueryGetBrandedFare: " << ' ' << ret->size() << " !!!!!!!!!!" <<
    // std::endl;

    // std::vector<BrandedFare*> *all(new std::vector<BrandedFare*>);// test
    // QueryGetAllBrandedFare qall(dbAdapter->getAdapter());// test
    // qall.findAllBrandedFare(*all);// test
    // std::cerr << "  !!!!!!! QueryGetAllBrandedFare: " << ' ' << all->size() << " !!!!!!" <<
    // std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandedFareDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

BrandedFareKey
BrandedFareDAO::createKey(const BrandedFare* info)
{
  return BrandedFareKey(info->vendor(), info->carrier());
}

void
BrandedFareDAO::destroy(BrandedFareKey, std::vector<BrandedFare*>* recs)
{
  destroyContainer(recs);
}

std::string
BrandedFareDAO::_name("BrandedFare");
std::string
BrandedFareDAO::_cacheClass("Fares");
DAOHelper<BrandedFareDAO>
BrandedFareDAO::_helper(_name);
BrandedFareDAO*
BrandedFareDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
BrandedFareHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BrandedFareHistoricalDAO"));

BrandedFareHistoricalDAO&
BrandedFareHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BrandedFare*>&
BrandedFareHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  BrandedFareHistoricalKey key(vendor, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<BrandedFare*>* ret(new std::vector<BrandedFare*>);
  del.adopt(ret);
  IsNotCurrentH<BrandedFare> isNotApplicable(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicable);
  return *ret;
}

std::vector<BrandedFare*>*
BrandedFareHistoricalDAO::create(BrandedFareHistoricalKey key)
{
  std::vector<BrandedFare*>* ret(new std::vector<BrandedFare*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetBrandedFareHistorical q(dbAdapter->getAdapter());
    // key._a = "ATP";// test !!!!
    // key._b = "VA";// test !!!!
    q.findBrandedFare(*ret, key._a, key._b, key._c, key._d);
    // std::cerr << "  !!!! QueryGetBrandedFareHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandedFareHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

BrandedFareHistoricalKey
BrandedFareHistoricalDAO::createKey(const BrandedFare* info,
                                    const DateTime& startDate,
                                    const DateTime& endDate)
{
  return BrandedFareHistoricalKey(info->vendor(), info->carrier(), startDate, endDate);
}

void
BrandedFareHistoricalDAO::destroy(BrandedFareHistoricalKey, std::vector<BrandedFare*>* recs)
{
  destroyContainer(recs);
}

std::string
BrandedFareHistoricalDAO::_name("BrandedFareHistorical");
std::string
BrandedFareHistoricalDAO::_cacheClass("Fares");
DAOHelper<BrandedFareHistoricalDAO>
BrandedFareHistoricalDAO::_helper(_name);
BrandedFareHistoricalDAO*
BrandedFareHistoricalDAO::_instance(nullptr);

} // namespace tse
