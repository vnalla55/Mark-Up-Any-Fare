//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MarkupSecFilterDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarkupSecFilter.h"
#include "DBAccess/Queries/QueryGetMarkupSecFilter.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MarkupSecFilterDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MarkupSecFilterDAO"));

MarkupSecFilterDAO&
MarkupSecFilterDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MarkupSecFilter*>&
getMarkupSecFilterData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MarkupSecFilterHistoricalDAO& dao = MarkupSecFilterHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, ticketDate);
  }
  else
  {
    MarkupSecFilterDAO& dao = MarkupSecFilterDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, ticketDate);
  }
}

const std::vector<MarkupSecFilter*>&
MarkupSecFilterDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const TariffNumber& ruleTariff,
                        const RuleNumber& rule,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MarkupSecFilterKey key(vendor, carrier, ruleTariff, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotCurrentG<MarkupSecFilter>(ticketDate)));
}

std::vector<MarkupSecFilter*>*
MarkupSecFilterDAO::create(MarkupSecFilterKey key)
{
  std::vector<MarkupSecFilter*>* ret = new std::vector<MarkupSecFilter*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMarkupSecFilter msf(dbAdapter->getAdapter());
    msf.findMarkupSecFilter(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarkupSecFilterDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MarkupSecFilterDAO::destroy(MarkupSecFilterKey key, std::vector<MarkupSecFilter*>* recs)
{
  std::vector<MarkupSecFilter*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MarkupSecFilterKey
MarkupSecFilterDAO::createKey(MarkupSecFilter* info)
{
  return MarkupSecFilterKey(info->vendor(), info->carrier(), info->ruleTariff(), info->rule());
}

void
MarkupSecFilterDAO::load()
{
  StartupLoaderNoDB<MarkupSecFilter, MarkupSecFilterDAO>();
}

sfc::CompressedData*
MarkupSecFilterDAO::compress(const std::vector<MarkupSecFilter*>* vect) const
{
  return compressVector(vect);
}

std::vector<MarkupSecFilter*>*
MarkupSecFilterDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MarkupSecFilter>(compressed);
}

std::string
MarkupSecFilterDAO::_name("MarkupSecFilter");
std::string
MarkupSecFilterDAO::_cacheClass("Rules");
DAOHelper<MarkupSecFilterDAO>
MarkupSecFilterDAO::_helper(_name);
MarkupSecFilterDAO* MarkupSecFilterDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MarkupSecFilterHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MarkupSecFilterHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MarkupSecFilterHistoricalDAO"));
MarkupSecFilterHistoricalDAO&
MarkupSecFilterHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MarkupSecFilter*>&
MarkupSecFilterHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MarkupSecFilterHistoricalKey key(vendor, carrier, ruleTariff, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<MarkupSecFilter*>* ret = new std::vector<MarkupSecFilter*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MarkupSecFilter>(ticketDate));
  return *ret;
}

std::vector<MarkupSecFilter*>*
MarkupSecFilterHistoricalDAO::create(MarkupSecFilterHistoricalKey key)
{
  std::vector<MarkupSecFilter*>* ret = new std::vector<MarkupSecFilter*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetMarkupSecFilterHistorical msf(dbAdapter->getAdapter());
    msf.findMarkupSecFilter(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarkupSecFilterHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MarkupSecFilterHistoricalDAO::destroy(MarkupSecFilterHistoricalKey key,
                                      std::vector<MarkupSecFilter*>* recs)
{
  std::vector<MarkupSecFilter*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MarkupSecFilterHistoricalDAO::compress(const std::vector<MarkupSecFilter*>* vect) const
{
  return compressVector(vect);
}

std::vector<MarkupSecFilter*>*
MarkupSecFilterHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MarkupSecFilter>(compressed);
}

std::string
MarkupSecFilterHistoricalDAO::_name("MarkupSecFilterHistorical");
std::string
MarkupSecFilterHistoricalDAO::_cacheClass("Rules");
DAOHelper<MarkupSecFilterHistoricalDAO>
MarkupSecFilterHistoricalDAO::_helper(_name);
MarkupSecFilterHistoricalDAO* MarkupSecFilterHistoricalDAO::_instance = nullptr;

} // namespace tse
