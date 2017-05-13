//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/IndustryFareApplDAO.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/Queries/QueryGetIndFareAppl.h"

namespace tse
{
log4cxx::LoggerPtr
IndustryFareApplDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryFareApplDAO"));

IndustryFareApplDAO&
IndustryFareApplDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const IndustryFareAppl*>&
getIndustryFareApplData(Indicator selectionType,
                        const CarrierCode& carrier,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  return getIndustryFareApplData(
      selectionType, carrier, date, date, deleteList, ticketDate, isHistorical);
}

const std::vector<const IndustryFareAppl*>&
getIndustryFareApplData(Indicator selectionType,
                        const CarrierCode& carrier,
                        const DateTime& startDate,
                        const DateTime& endDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    IndustryFareApplHistoricalDAO& dao = IndustryFareApplHistoricalDAO::instance();
    return dao.get(deleteList, selectionType, carrier, startDate, ticketDate);
  }
  else
  {
    IndustryFareApplDAO& dao = IndustryFareApplDAO::instance();
    return dao.get(deleteList, selectionType, carrier, startDate, endDate, ticketDate);
  }
}

const std::vector<const IndustryFareAppl*>&
IndustryFareApplDAO::get(DeleteList& del,
                         const Indicator& indicator,
                         const CarrierCode& carrier,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  return get(del, indicator, carrier, date, date, ticketDate);
}

const std::vector<const IndustryFareAppl*>&
IndustryFareApplDAO::get(DeleteList& del,
                         const Indicator& indicator,
                         const CarrierCode& carrier,
                         const DateTime& startDate,
                         const DateTime& endDate,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IndustryFareApplKey key(indicator, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const IndustryFareAppl*>* ret = new std::vector<const IndustryFareAppl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<const IndustryFareAppl>(startDate, endDate, ticketDate));
  key = IndustryFareApplKey(indicator, "");
  ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<const IndustryFareAppl>(startDate, endDate, ticketDate));
  return *ret;
}

IndustryFareApplKey
IndustryFareApplDAO::createKey(IndustryFareAppl* info)
{
  return IndustryFareApplKey(info->selectionType(), info->carrier());
}

std::vector<IndustryFareAppl*>*
IndustryFareApplDAO::create(IndustryFareApplKey key)
{
  std::vector<IndustryFareAppl*>* ret = new std::vector<IndustryFareAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndFareAppl ifa(dbAdapter->getAdapter());
    ifa.findIndustryFareAppl(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryFareApplDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IndustryFareApplDAO::destroy(IndustryFareApplKey key, std::vector<IndustryFareAppl*>* recs)
{
  destroyContainer(recs);
}

void
IndustryFareApplDAO::load()
{
  StartupLoader<QueryGetAllIndFareAppl, IndustryFareAppl, IndustryFareApplDAO>();
}

sfc::CompressedData*
IndustryFareApplDAO::compress(const std::vector<IndustryFareAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<IndustryFareAppl*>*
IndustryFareApplDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<IndustryFareAppl>(compressed);
}

std::string
IndustryFareApplDAO::_name("IndustryFareAppl");
std::string
IndustryFareApplDAO::_cacheClass("Fares");
DAOHelper<IndustryFareApplDAO>
IndustryFareApplDAO::_helper(_name);
IndustryFareApplDAO* IndustryFareApplDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: IndustryFareApplHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
IndustryFareApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IndustryFareApplHistoricalDAO"));
IndustryFareApplHistoricalDAO&
IndustryFareApplHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const IndustryFareAppl*>&
IndustryFareApplHistoricalDAO::get(DeleteList& del,
                                   const Indicator& indicator,
                                   const CarrierCode& carrier,
                                   const DateTime& date,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IndustryFareApplHistoricalKey key(indicator, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const IndustryFareAppl*>* ret = new std::vector<const IndustryFareAppl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<const IndustryFareAppl>(date, ticketDate));
  return *ret;
}

std::vector<IndustryFareAppl*>*
IndustryFareApplHistoricalDAO::create(IndustryFareApplHistoricalKey key)
{
  std::vector<IndustryFareAppl*>* ret = new std::vector<IndustryFareAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetIndFareApplHistorical ifa(dbAdapter->getAdapter());
    ifa.findIndustryFareAppl(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IndustryFareApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IndustryFareApplHistoricalDAO::destroy(IndustryFareApplHistoricalKey key,
                                       std::vector<IndustryFareAppl*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
IndustryFareApplHistoricalDAO::compress(const std::vector<IndustryFareAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<IndustryFareAppl*>*
IndustryFareApplHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<IndustryFareAppl>(compressed);
}

std::string
IndustryFareApplHistoricalDAO::_name("IndustryFareApplHistorical");
std::string
IndustryFareApplHistoricalDAO::_cacheClass("Fares");
DAOHelper<IndustryFareApplHistoricalDAO>
IndustryFareApplHistoricalDAO::_helper(_name);
IndustryFareApplHistoricalDAO* IndustryFareApplHistoricalDAO::_instance = nullptr;

} // namespace tse
